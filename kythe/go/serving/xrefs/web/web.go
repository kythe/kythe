/*
 * Copyright 2014 Google Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// Package web contains HTTP handlers for a frontend to an xrefs.Service and a
// filetree.FileTree.  The following methods with be exposed:
//
//   GET /corpusRoots
//     Returns a JSON map from corpus to []root names (map[string][]string)
//   GET /dir/<path>?corpus=<corpus>&root=<root>[&recursive]
//     Returns the JSON equivalent of a filetree.Directory describing the given
//     directory's contents (sub-directories and files).
//   GET /file/<path>?corpus=<corpus>&root=<root>&language=<lang>&signature=<sig>
//     Returns the JSON equivalent of an xrefs.DecorationsReply for the
//     described file.  References and source-text will be supplied in the
//     reply.
//   GET /xrefs?ticket=<ticket>
//     Returns a JSON map from edgeKind to a set of anchor/file locations that
//     attach to the given node.
package web

import (
	"encoding/json"
	"fmt"
	"log"
	"net/http"
	"path/filepath"
	"strings"
	"time"

	"kythe/go/serving/xrefs"
	"kythe/go/serving/xrefs/tools"
	"kythe/go/storage/filetree"
	"kythe/go/util/httpencoding"
	"kythe/go/util/kytheuri"
	"kythe/go/util/schema"

	spb "kythe/proto/storage_proto"
	xpb "kythe/proto/xref_proto"

	"code.google.com/p/goprotobuf/proto"
)

// Handlers contains the service handlers necessary for the web interface.
type Handlers struct {
	XRefs    xrefs.Service
	FileTree filetree.FileTree
}

// AddXRefHandlers adds handlers to mux for the /corpusRoots, /dir/, /file/, and
// /xrefs/ patterns using the given handlers (which will may be changed after
// being passed).
func AddXRefHandlers(prefix string, handlers *Handlers, mux *http.ServeMux, indirectNameNodes bool) {
	mux.Handle(prefix+"/corpusRoots", http.StripPrefix(prefix, corpusRootsHandler(handlers)))
	mux.Handle(prefix+"/dir/", http.StripPrefix(prefix, dirHandler(handlers)))
	mux.Handle(prefix+"/file/", http.StripPrefix(prefix, fileHandler(handlers)))
	mux.Handle(prefix+"/xrefs", http.StripPrefix(prefix, xrefsHandler(handlers, indirectNameNodes)))
}

// corpusRootsHandler replies with a JSON map of corpus roots
func corpusRootsHandler(h *Handlers) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		roots, err := h.FileTree.CorporaRoots()
		if err != nil {
			http.Error(w, err.Error(), http.StatusInternalServerError)
			return
		}
		writeJSON(w, r, roots)
	}
}

// fileHandler parses a file VName from the Request URL's Path/Query and replies
// with a JSON object equivalent to a xpb.DecorationsReply with its SourceText
// and Reference fields populated
func fileHandler(h *Handlers) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		if r.Method != "GET" {
			http.Error(w, "Only GET requests allowed", http.StatusMethodNotAllowed)
			return
		}
		path := trimPath(r.URL.Path, "/file/")
		if path == "/file" {
			http.Error(w, "Bad Request: no file path given", http.StatusBadRequest)
			return
		}

		args := r.URL.Query()
		fileVName := &spb.VName{
			Signature: firstOrNil(args["signature"]),
			Language:  firstOrNil(args["language"]),
			Corpus:    firstOrNil(args["corpus"]),
			Root:      firstOrNil(args["root"]),
			Path:      proto.String(path),
		}

		startTime := time.Now()
		reply, err := h.XRefs.Decorations(&xpb.DecorationsRequest{
			Location:   &xpb.Location{Ticket: proto.String(kytheuri.FromVName(fileVName).String())},
			SourceText: proto.Bool(true),
			References: proto.Bool(true),
		})
		if err != nil {
			code := http.StatusInternalServerError
			if strings.Contains(err.Error(), "file not found") {
				code = http.StatusNotFound
			}
			http.Error(w, err.Error(), code)
			return
		}
		log.Printf("Decorations [%v]", time.Since(startTime))

		if err := writeJSON(w, r, reply); err != nil {
			log.Printf("Error encoding file response: %v", err)
		}
	}
}

// dirHandler parses a corpus/root/path from the Request URL's Path/Query and
// replies with a JSON object describing the directories sub-directories and
// files
func dirHandler(h *Handlers) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		path := trimPath(r.URL.Path, "/dir")
		if path == "" {
			path = "/"
		}
		args := r.URL.Query()
		corpus, root := firstOrEmpty(args["corpus"]), firstOrEmpty(args["root"])

		dir, err := h.FileTree.Dir(corpus, root, path)
		if err != nil {
			http.Error(w, err.Error(), http.StatusInternalServerError)
			return
		}

		if err := writeJSON(w, r, dir); err != nil {
			log.Printf("Failed to encode directory: %v", err)
		}
	}
}

var (
	anchorFilters  = []string{schema.NodeKindFact, "/kythe/loc/*"}
	revDefinesEdge = schema.MirrorEdge(schema.DefinesEdge)
)

// xrefsHandler returns a map from edge kind to set of anchorLocations for a
// given ticket.
func xrefsHandler(h *Handlers, indirectNameNodes bool) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		startTime := time.Now()
		ticket := firstOrEmpty(r.URL.Query()["ticket"])
		if ticket == "" {
			http.Error(w, "Bad Request: missing target parameter", http.StatusBadRequest)
			return
		}

		refs, total, err := tools.XRefs(h.XRefs, ticket, indirectNameNodes)
		if err != nil {
			http.Error(w, err.Error(), http.StatusInternalServerError)
			return
		} else if len(refs) == 0 {
			http.Error(w, fmt.Sprintf("No references found for ticket %q", ticket), http.StatusNotFound)
			return
		}

		log.Printf("XRefs [%v]\t%d", time.Since(startTime), total)
		if err := writeJSON(w, r, refs); err != nil {
			log.Println(err)
		}
	}
}

func writeJSON(w http.ResponseWriter, r *http.Request, v interface{}) error {
	w.Header().Set("Content-Type", "application/json; charset=utf-8")
	cw := httpencoding.CompressData(w, r)
	defer cw.Close()
	return json.NewEncoder(cw).Encode(v)
}

func firstOrEmpty(strs []string) string {
	if str := firstOrNil(strs); str != nil {
		return *str
	}
	return ""
}

func firstOrNil(strs []string) *string {
	if len(strs) == 0 {
		return nil
	}
	s := strs[0]
	return &s
}

func trimPath(path, prefix string) string {
	return strings.TrimPrefix(filepath.Clean(path), prefix)
}
