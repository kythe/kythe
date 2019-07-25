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

// Package server exposes an HTTP interface to the xrefs and filetree
// services.
package main

import (
	"flag"
	"fmt"
	"log"
	"net/http"
	"path/filepath"
	"time"

	"kythe.io/kythe/go/services/filetree"
	"kythe.io/kythe/go/services/xrefs"
	ftsrv "kythe.io/kythe/go/serving/filetree"
	xsrv "kythe.io/kythe/go/serving/xrefs"
	"kythe.io/kythe/go/storage/leveldb"
	"kythe.io/kythe/go/storage/table"

	"golang.org/x/net/context"
)

var (
	servingTable    = flag.String("serving_table", "/srv/table", "LevelDB serving table path")
	publicResources = flag.String("public_resources", "/srv/public", "Path to directory of static resources to serve")
	listeningAddr   = flag.String("listen", ":8080", "Listening address for HTTP server (localhost:<port> allows access from only this machine)")
)

func main() {
	flag.Parse()

	http.HandleFunc("/_ah/health", func(w http.ResponseWriter, r *http.Request) {
		fmt.Fprint(w, "ok")
	})
	http.HandleFunc("/_ah/start", func(w http.ResponseWriter, r *http.Request) {
		fmt.Fprintln(w, "sure, we'll start!")
	})
	http.HandleFunc("/liveness_check", func(w http.ResponseWriter, r *http.Request) {
		fmt.Fprint(w, "ok")
	})

	log.Printf("Server listening on %q", *listeningAddr)
	go func() { log.Fatal(http.ListenAndServe(*listeningAddr, nil)) }()

	start := time.Now()
	ctx := context.Background()
	db, err := leveldb.Open(*servingTable, nil)
	if err != nil {
		log.Fatalf("Error opening db at %q: %v", *servingTable, err)
	}
	defer db.Close(ctx)
	tbl := &table.KVProto{db}
	log.Printf("Serving data opened in %s", time.Since(start))

	http.HandleFunc("/", func(w http.ResponseWriter, r *http.Request) {
		http.ServeFile(w, r, filepath.Join(*publicResources, filepath.Clean(r.URL.Path)))
	})
	xrefs.RegisterHTTPHandlers(ctx, xsrv.NewCombinedTable(tbl), http.DefaultServeMux)
	filetree.RegisterHTTPHandlers(ctx, &ftsrv.Table{tbl, true}, http.DefaultServeMux)

	http.HandleFunc("/readiness_check", func(w http.ResponseWriter, r *http.Request) {
		fmt.Fprint(w, "ok")
	})
	log.Print("Server ready for traffic")

	select {} // block forever
}
