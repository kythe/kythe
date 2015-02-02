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

// Binary xrefs_server is an HTTP frontend to an xrefs.Service and a
// filetree.FileTree based on a given --graphstore or set of --tables.  The
// given GraphStore will be first pre-processed to add reverse edges and store a
// static in-memory directory structure.  The --serving_path will be served from
// the root URL along with the handlers in kythe/go/serving/web.
package main

import (
	"flag"
	"log"
	"net/http"
	"path/filepath"
	"time"

	"kythe/go/services/filetree"
	"kythe/go/services/graphstore"
	"kythe/go/services/xrefs"
	srvx "kythe/go/serving/xrefs"
	"kythe/go/storage/gsutil"
	sxrefs "kythe/go/storage/xrefs"
)

var (
	listeningAddr     = flag.String("listen", "localhost:8080", "HTTP serving address")
	servingDir        = flag.String("serving_path", "kythe/web/ui/resources/public", "Path to public serving directory")
	skipPreprocessing = flag.Bool("skip_preprocessing", false, "Skip GraphStore preprocessing")

	indirectNameNodes = flag.Bool("indirect_names", false, "For xrefs calls, indirect through name nodes to get a complete set of references")

	tablesPath = flag.String("tables", "", "Path to xrefs tables to serve")
	gs         graphstore.Service
)

func init() {
	gsutil.Flag(&gs, "graphstore", "GraphStore to which to write the entry stream")
}

func main() {
	flag.Parse()
	if *listeningAddr == "" {
		log.Fatal("Missing --listen address")
	}

	if gs != nil && *tablesPath != "" {
		log.Fatal("--graphstore and --tables are mutually exclusive")
	}

	var (
		tree filetree.Service
		xs   xrefs.Service
	)

	if gs != nil {
		if !*skipPreprocessing {
			if err := sxrefs.AddReverseEdges(gs); err != nil {
				log.Fatalf("Failed to add reverse edges: %v", err)
			}
		}
		var err error
		tree, err = createFileTree(gs)
		if err != nil {
			log.Fatalf("Failed to create GraphStore file tree: %v", err)
		}
		xs = sxrefs.NewGraphStoreService(gs)
	} else {
		tbls, err := srvx.Open(*tablesPath)
		if err != nil {
			log.Fatalf("Error opening tables at %q: %v", *tablesPath, err)
		}
		xs = tbls.XRefs()
		tree = tbls.FileTree()
	}

	// Add HTTP handlers
	xrefs.RegisterHTTPHandlers("", xs, http.DefaultServeMux)
	filetree.RegisterHTTPHandlers("", tree, http.DefaultServeMux)
	http.HandleFunc("/", func(w http.ResponseWriter, r *http.Request) {
		http.ServeFile(w, r, filepath.Join(*servingDir, filepath.Clean(r.URL.Path)))
	})

	log.Printf("xrefs browser launching on %q", *listeningAddr)
	log.Fatal(http.ListenAndServe(*listeningAddr, nil))
}

// createFileTree times the population of a filetree.Map with a given
// GraphStore.
func createFileTree(gs graphstore.Service) (filetree.Service, error) {
	log.Println("Creating GraphStore file tree")
	startTime := time.Now()
	defer func() {
		log.Printf("Tree populated in %v", time.Since(startTime))
	}()
	t := filetree.NewMap()
	return t, t.Populate(gs)
}
