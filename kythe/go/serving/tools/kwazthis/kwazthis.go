/*
 * Copyright 2015 Google Inc. All rights reserved.
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

// Binary kwazthis (K, what's this?) determines what references are located at a
// particular offset within a file.  All results are printed as JSON.
//
// By default, kwazthis will search for a .kythe configuration file in a
// directory above the given --path (if it exists locally relative to the
// current working directory).  If found, --path will be made relative to this
// directory and --root before making any Kythe service requests.  If not found,
// --path will be passed unchanged.  --ignore_local_repo will turn off this
// behavior.
//
// Usage:
//   kwazthis --path kythe/cxx/tools/kindex_tool_main.cc --offset 2660
//   kwazthis --path kythe/java/com/google/devtools/kythe/analyzers/base/EntrySet.java --offset 2815
package main

import (
	"encoding/json"
	"flag"
	"fmt"
	"io/ioutil"
	"log"
	"os"
	"path/filepath"
	"strconv"
	"strings"

	"kythe.io/kythe/go/services/search"
	"kythe.io/kythe/go/services/xrefs"
	"kythe.io/kythe/go/util/kytheuri"
	"kythe.io/kythe/go/util/schema"

	"golang.org/x/net/context"
	"google.golang.org/grpc"

	spb "kythe.io/kythe/proto/storage_proto"
	xpb "kythe.io/kythe/proto/xref_proto"
)

func init() {
	binary := filepath.Base(os.Args[0])
	flag.Usage = func() {
		fmt.Fprintf(os.Stderr, `Determine what references are located at a particular offset within a file.

Usage: %s --offset int (--path p | --signature s) [--corpus c] [--root r] [--language l]
       %`+strconv.Itoa(len(binary))+`s [--ignore_local_repo] [--dirty_buffer path]

By default, kwazthis will search for a .kythe configuration file in a directory
above the given --path (if it exists locally relative to the current working
directory).  If found, --path will be made relative to this directory and --root
before making any Kythe service requests.  If not found, --path will be passed
unchanged.  --ignore_local_repo will turn off this behavior.

Defaults flag values:
`, binary, "")
		flag.PrintDefaults()
	}
}

var (
	remoteAPI = flag.String("api", "https://xrefs-dot-kythe-repo.appspot.com", "Remote API server")

	ignoreLocalRepo = flag.Bool("ignore_local_repo", false, "Ignore local repository .kythe configuration")

	dirtyBuffer = flag.String("dirty_buffer", "", "Path to file with dirty buffer contents (optional)")

	path      = flag.String("path", "", "Path of file (optional if --signature is given)")
	signature = flag.String("signature", "", "Signature of file VName (optional if --path is given)")
	corpus    = flag.String("corpus", "", "Corpus of file VName (optional)")
	root      = flag.String("root", "", "Root of file VName (optional)")
	language  = flag.String("language", "", "Language of file VName (optional)")

	offset = flag.Int("offset", -1, "Non-negative offset in file to list references")
)

var (
	xs  xrefs.Service
	idx search.Service

	fileFacts = []*spb.SearchRequest_Fact{
		{Name: schema.NodeKindFact, Value: []byte(schema.FileKind)},
	}
)

type reference struct {
	Span struct {
		Start int    `json:"start"`
		End   int    `json:"end"`
		Text  string `json:"text,omitempty"`
	} `json:"span"`
	Kind string `json:"kind"`

	Node struct {
		Ticket  string   `json:"ticket"`
		Names   []string `json:"names,omitempty"`
		Kind    string   `json:"kind,omitempty"`
		Subkind string   `json:"subkind,omitempty"`
	} `json:"node"`
}

func main() {
	flag.Parse()
	if *offset < 0 {
		log.Fatal("ERROR: non-negative --offset required")
	} else if *signature == "" && *path == "" {
		log.Fatal("ERROR: must provide at least -path or --signature")
	}

	if strings.HasPrefix(*remoteAPI, "http://") || strings.HasPrefix(*remoteAPI, "https://") {
		xs = xrefs.WebClient(*remoteAPI)
		idx = search.WebClient(*remoteAPI)
	} else {
		conn, err := grpc.Dial(*remoteAPI)
		if err != nil {
			log.Fatalf("Error connecting to remote API %q: %v", *remoteAPI, err)
		}
		defer conn.Close()
		ctx := context.Background()
		xs = xrefs.GRPC(ctx, xpb.NewXRefServiceClient(conn))
		idx = search.GRPC(ctx, spb.NewSearchServiceClient(conn))
	}

	relPath := *path
	if !*ignoreLocalRepo {
		if _, err := os.Stat(relPath); err == nil {
			absPath, err := filepath.Abs(relPath)
			if err != nil {
				log.Fatal(err)
			}
			kytheRoot := findKytheRoot(filepath.Dir(absPath))
			if kytheRoot != "" {
				relPath, err = filepath.Rel(filepath.Join(kytheRoot, *root), absPath)
				if err != nil {
					log.Fatal(err)
				}
			}
		}
	}
	partialFile := &spb.VName{
		Signature: *signature,
		Corpus:    *corpus,
		Root:      *root,
		Path:      relPath,
		Language:  *language,
	}
	reply, err := idx.Search(&spb.SearchRequest{
		Partial: partialFile,
		Fact:    fileFacts,
	})
	if err != nil {
		log.Fatalf("Error locating file {%v}: %v", partialFile, err)
	}
	if len(reply.Ticket) == 0 {
		log.Fatalf("Could not locate file {%v}", partialFile)
	} else if len(reply.Ticket) > 1 {
		log.Fatalf("Ambiguous file {%v}; multiple results: %v", partialFile, reply.Ticket)
	}

	fileTicket := reply.Ticket[0]
	decor, err := xs.Decorations(&xpb.DecorationsRequest{
		// TODO(schroederc): limit Location to a SPAN around *offset
		Location:    &xpb.Location{Ticket: fileTicket},
		References:  true,
		SourceText:  true,
		DirtyBuffer: readDirtyBuffer(),
	})
	if err != nil {
		log.Fatal(err)
	}
	nodes := xrefs.NodesMap(decor.Node)

	en := json.NewEncoder(os.Stdout)
	for _, ref := range decor.Reference {
		start, _ := strconv.Atoi(string(nodes[ref.SourceTicket][schema.AnchorStartFact]))
		end, _ := strconv.Atoi(string(nodes[ref.SourceTicket][schema.AnchorEndFact]))

		if start <= *offset && *offset < end {
			var r reference
			r.Span.Start = start
			r.Span.End = end
			r.Span.Text = string(decor.SourceText[start:end])
			r.Kind = strings.TrimPrefix(ref.Kind, schema.EdgePrefix)
			r.Node.Ticket = ref.TargetTicket

			node := nodes[ref.TargetTicket]
			r.Node.Kind = string(node[schema.NodeKindFact])
			r.Node.Subkind = string(node[schema.SubkindFact])

			if eReply, err := xs.Edges(&xpb.EdgesRequest{
				Ticket: []string{ref.TargetTicket},
				Kind:   []string{schema.NamedEdge},
			}); err != nil {
				log.Printf("WARNING: error getting edges for %q: %v", ref.TargetTicket, err)
			} else {
				for _, name := range xrefs.EdgesMap(eReply.EdgeSet)[ref.TargetTicket][schema.NamedEdge] {
					if uri, err := kytheuri.Parse(name); err != nil {
						log.Printf("WARNING: named node ticket (%q) could not be parsed: %v", name, err)
					} else {
						r.Node.Names = append(r.Node.Names, uri.Signature)
					}
				}
			}

			if err := en.Encode(r); err != nil {
				log.Fatal(err)
			}
		}
	}
}

func readDirtyBuffer() []byte {
	if *dirtyBuffer == "" {
		return nil
	}

	f, err := os.Open(*dirtyBuffer)
	if err != nil {
		log.Fatalf("ERROR: could not open dirty buffer at %q: %v", *dirtyBuffer, err)
	}
	defer f.Close()
	data, err := ioutil.ReadAll(f)
	if err != nil {
		log.Fatalf("ERROR: could read dirty buffer at %q: %v", *dirtyBuffer, err)
	}
	return data
}

func findKytheRoot(dir string) string {
	for {
		if fi, err := os.Stat(filepath.Join(dir, ".kythe")); err == nil && fi.Mode().IsRegular() {
			return dir
		}
		if dir == "/" {
			break
		}
		dir = filepath.Dir(dir)
	}
	return ""
}
