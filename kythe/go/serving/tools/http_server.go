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

// Binary http_server exposes an HTTP interface to the xrefs and filetree
// services backed by either a combined serving table or a bare GraphStore.
package main

import (
	"flag"
	"log"
	"net/http"
	"path/filepath"

	"kythe/go/services/filetree"
	"kythe/go/services/graphstore"
	"kythe/go/services/search"
	"kythe/go/services/xrefs"
	ftsrv "kythe/go/serving/filetree"
	srchsrv "kythe/go/serving/search"
	xsrv "kythe/go/serving/xrefs"
	"kythe/go/storage/gsutil"
	"kythe/go/storage/leveldb"
	"kythe/go/storage/table"
	xstore "kythe/go/storage/xrefs"
)

var (
	gs graphstore.Service

	listeningAddr   = flag.String("listen", "localhost:8080", "Listening address for API server")
	servingTable    = flag.String("serving_table", "", "LevelDB serving table")
	publicResources = flag.String("public_resources", "", "Path to directory of static resources to serve")
)

func init() {
	gsutil.Flag(&gs, "graphstore", "GraphStore to serve xrefs")
}

func main() {
	flag.Parse()
	if *servingTable == "" && gs == nil {
		log.Fatal("Missing either --serving_table or --graphstore")
	} else if *listeningAddr == "" {
		log.Fatal("Missing required --listen argument")
	} else if *servingTable != "" && gs != nil {
		log.Fatal("--serving_table and --graphstore are mutually exclusive")
	}

	var (
		xs xrefs.Service
		ft filetree.Service
		sr search.Service
	)

	if *servingTable != "" {
		db, err := leveldb.Open(*servingTable, nil)
		if err != nil {
			log.Fatalf("Error opening db at %q: %v", *servingTable, err)
		}
		defer db.Close()
		tbl := &table.KVProto{db}
		xs = &xsrv.Table{tbl}
		ft = &ftsrv.Table{tbl}
		sr = &srchsrv.Table{&table.KVInverted{db}}
	} else {
		log.Println("WARNING: serving directly from a GraphStore can be slow; you may want to use a --serving_table")
		if f, ok := gs.(filetree.Service); ok {
			log.Printf("Using %T directly as filetree service", gs)
			ft = f
		} else {
			m := filetree.NewMap()
			if err := m.Populate(gs); err != nil {
				log.Fatalf("Error populating file tree from GraphStore: %v", err)
			}
			ft = m
		}

		if x, ok := gs.(xrefs.Service); ok {
			log.Printf("Using %T directly as xrefs service", gs)
			xs = x
		} else {
			if err := xstore.EnsureReverseEdges(gs); err != nil {
				log.Fatalf("Error ensuring reverse edges in GraphStore: %v", err)
			}
			xs = xstore.NewGraphStoreService(gs)
		}

		if s, ok := gs.(search.Service); ok {
			log.Printf("Using %T directly as search service", gs)
			sr = s
		}
	}

	xrefs.RegisterHTTPHandlers(xs, http.DefaultServeMux)
	filetree.RegisterHTTPHandlers(ft, http.DefaultServeMux)
	if sr != nil {
		search.RegisterHTTPHandlers(sr, http.DefaultServeMux)
	} else {
		log.Println("Search API not supported")
	}

	if *publicResources != "" {
		http.HandleFunc("/", func(w http.ResponseWriter, r *http.Request) {
			http.ServeFile(w, r, filepath.Join(*publicResources, filepath.Clean(r.URL.Path)))
		})
	}

	log.Printf("Server listening on %q", *listeningAddr)
	log.Fatal(http.ListenAndServe(*listeningAddr, nil))
}
