/*
 * Copyright 2015 The Kythe Authors. All rights reserved.
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

// Package filetree defines the filetree Service interface and a simple
// in-memory implementation.
package filetree // import "kythe.io/kythe/go/services/filetree"

import (
	"context"
	"fmt"
	"net/http"
	"path"
	"path/filepath"
	"strings"
	"time"

	"kythe.io/kythe/go/services/graphstore"
	"kythe.io/kythe/go/services/web"
	"kythe.io/kythe/go/util/log"
	"kythe.io/kythe/go/util/schema/facts"
	"kythe.io/kythe/go/util/schema/nodes"

	"google.golang.org/protobuf/proto"

	ftpb "kythe.io/kythe/proto/filetree_go_proto"
	spb "kythe.io/kythe/proto/storage_go_proto"
)

// Service provides an interface to explore a tree of VName files.
type Service interface {
	// Directory returns the contents of the directory at the given corpus/root/path.
	Directory(context.Context, *ftpb.DirectoryRequest) (*ftpb.DirectoryReply, error)

	// CorpusRoots returns a map from corpus to known roots.
	CorpusRoots(context.Context, *ftpb.CorpusRootsRequest) (*ftpb.CorpusRootsReply, error)
}

// CleanDirPath returns a clean, corpus root relative equivalent to path.
func CleanDirPath(path string) string {
	const sep = string(filepath.Separator)
	return strings.TrimPrefix(filepath.Join(sep, path), sep)
}

// Map is a FileTree backed by an in-memory map.
type Map struct {
	// corpus -> root -> dirPath -> DirectoryReply
	M map[string]map[string]map[string]*ftpb.DirectoryReply
}

// NewMap returns an empty filetree map.
func NewMap() *Map {
	return &Map{make(map[string]map[string]map[string]*ftpb.DirectoryReply)}
}

// Populate adds each file node in gs to m.
func (m *Map) Populate(ctx context.Context, gs graphstore.Service) error {
	start := time.Now()
	log.Info("Populating in-memory file tree")
	var total int
	if err := gs.Scan(ctx, &spb.ScanRequest{FactPrefix: facts.NodeKind},
		func(entry *spb.Entry) error {
			if entry.FactName == facts.NodeKind && string(entry.FactValue) == nodes.File {
				m.AddFile(entry.Source)
				total++
			}
			return nil
		}); err != nil {
		return fmt.Errorf("failed to Scan GraphStore for directory structure: %v", err)
	}
	log.Infof("Indexed %d files in %s", total, time.Since(start))
	return nil
}

// AddFile adds the given file VName to m.
func (m *Map) AddFile(file *spb.VName) {
	dirPath := CleanDirPath(path.Dir(file.Path))
	dir := m.ensureDir(file.Corpus, file.Root, dirPath)
	dir.Entry = addEntry(dir.Entry, &ftpb.DirectoryReply_Entry{
		Kind:      ftpb.DirectoryReply_FILE,
		Name:      filepath.Base(file.Path),
		Generated: file.GetRoot() != "",
	})
}

// CorpusRoots implements part of the filetree.Service interface.
func (m *Map) CorpusRoots(ctx context.Context, req *ftpb.CorpusRootsRequest) (*ftpb.CorpusRootsReply, error) {
	cr := &ftpb.CorpusRootsReply{}
	for corpus, rootDirs := range m.M {
		var roots []string
		for root := range rootDirs {
			roots = append(roots, root)
		}
		cr.Corpus = append(cr.Corpus, &ftpb.CorpusRootsReply_Corpus{
			Name: corpus,
			Root: roots,
		})
	}
	return cr, nil
}

// Directory implements part of the filetree.Service interface.
func (m *Map) Directory(ctx context.Context, req *ftpb.DirectoryRequest) (*ftpb.DirectoryReply, error) {
	roots := m.M[req.Corpus]
	if roots == nil {
		return &ftpb.DirectoryReply{}, nil
	}
	dirs := roots[req.Root]
	if dirs == nil {
		return &ftpb.DirectoryReply{}, nil
	}
	d := dirs[req.Path]
	if d == nil {
		return &ftpb.DirectoryReply{}, nil
	}
	return d, nil
}

func (m *Map) ensureCorpusRoot(corpus, root string) map[string]*ftpb.DirectoryReply {
	roots := m.M[corpus]
	if roots == nil {
		roots = make(map[string]map[string]*ftpb.DirectoryReply)
		m.M[corpus] = roots
	}

	dirs := roots[root]
	if dirs == nil {
		dirs = make(map[string]*ftpb.DirectoryReply)
		roots[root] = dirs
	}
	return dirs
}

func (m *Map) ensureDir(corpus, root, path string) *ftpb.DirectoryReply {
	if path == "." {
		path = ""
	}
	dirs := m.ensureCorpusRoot(corpus, root)
	dir := dirs[path]
	if dir == nil {
		dir = &ftpb.DirectoryReply{
			Corpus: corpus,
			Root:   root,
			Path:   path,
		}
		dirs[path] = dir

		if path != "" {
			parent := m.ensureDir(corpus, root, filepath.Dir(path))
			parent.Entry = addEntry(parent.Entry, &ftpb.DirectoryReply_Entry{
				Kind:      ftpb.DirectoryReply_DIRECTORY,
				Name:      filepath.Base(path),
				Generated: root != "",
			})
		}
	}
	return dir
}

func addEntry(entries []*ftpb.DirectoryReply_Entry, e *ftpb.DirectoryReply_Entry) []*ftpb.DirectoryReply_Entry {
	for _, x := range entries {
		if proto.Equal(x, e) {
			return entries
		}
	}
	return append(entries, e)
}

type webClient struct{ addr string }

// CorpusRoots implements part of the Service interface.
func (w *webClient) CorpusRoots(ctx context.Context, req *ftpb.CorpusRootsRequest) (*ftpb.CorpusRootsReply, error) {
	var reply ftpb.CorpusRootsReply
	return &reply, web.Call(w.addr, "corpusRoots", req, &reply)
}

// Directory implements part of the Service interface.
func (w *webClient) Directory(ctx context.Context, req *ftpb.DirectoryRequest) (*ftpb.DirectoryReply, error) {
	var reply ftpb.DirectoryReply
	return &reply, web.Call(w.addr, "dir", req, &reply)
}

// WebClient returns an filetree Service based on a remote web server.
func WebClient(addr string) Service { return &webClient{addr} }

// RegisterHTTPHandlers registers JSON HTTP handlers with mux using the given
// filetree Service.  The following methods with be exposed:
//
//	GET /corpusRoots
//	  Response: JSON encoded filetree.CorpusRootsReply
//	GET /dir
//	  Request: JSON encoded filetree.DirectoryRequest
//	  Response: JSON encoded filetree.DirectoryReply
//
// Note: /corpusRoots and /dir will return their responses as serialized
// protobufs if the "proto" query parameter is set.
func RegisterHTTPHandlers(ctx context.Context, ft Service, mux *http.ServeMux) {
	mux.HandleFunc("/corpusRoots", func(w http.ResponseWriter, r *http.Request) {
		start := time.Now()
		defer func() {
			log.Infof("filetree.CorpusRoots:\t%s", time.Since(start))
		}()

		var req ftpb.CorpusRootsRequest
		if err := web.ReadJSONBody(r, &req); err != nil {
			http.Error(w, err.Error(), http.StatusBadRequest)
			return
		}
		cr, err := ft.CorpusRoots(ctx, &req)
		if err != nil {
			http.Error(w, err.Error(), http.StatusInternalServerError)
			return
		}
		if err := web.WriteResponse(w, r, cr); err != nil {
			log.Info(err)
		}
	})
	mux.HandleFunc("/dir", func(w http.ResponseWriter, r *http.Request) {
		start := time.Now()
		defer func() {
			log.Infof("filetree.Dir:\t%s", time.Since(start))
		}()

		var req ftpb.DirectoryRequest
		if err := web.ReadJSONBody(r, &req); err != nil {
			http.Error(w, err.Error(), http.StatusBadRequest)
			return
		}
		reply, err := ft.Directory(ctx, &req)
		if err != nil {
			http.Error(w, err.Error(), http.StatusInternalServerError)
			return
		}
		if err := web.WriteResponse(w, r, reply); err != nil {
			log.Info(err)
		}
	})
}
