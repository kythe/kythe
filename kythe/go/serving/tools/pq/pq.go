/*
 * Copyright 2016 Google Inc. All rights reserved.
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

// Binary pq is an experimental tool to populate a Postgres database with Kythe
// serving data and serve it through Kythe's standard HTTP interface.
package main

import (
	"flag"
	"log"
	"net/http"
	"os"
	"path/filepath"

	"kythe.io/kythe/go/platform/vfs"
	"kythe.io/kythe/go/services/filetree"
	"kythe.io/kythe/go/services/graphstore"
	"kythe.io/kythe/go/services/xrefs"
	"kythe.io/kythe/go/serving/pq"
	"kythe.io/kythe/go/storage/gsutil"
	"kythe.io/kythe/go/storage/stream"
	"kythe.io/kythe/go/util/flagutil"
	"kythe.io/kythe/go/util/profile"

	"golang.org/x/net/context"

	spb "kythe.io/kythe/proto/storage_proto"
)

func init() {
	gsutil.Flag(&gs, "copy_graphstore", "GraphStore from which to copy into the Postgres --db (mutually exclusive with --copy_entries)")
	flag.Usage = flagutil.SimpleUsage("Experimental tools to populate a Postgres database with Kythe serving data and serve it",
		"--db connection-string [--copy_graphstore spec | --copy_entries path] [--public_resources dir] [--listen addr]")
}

var (
	gs graphstore.Service

	entriesFile = flag.String("copy_entries", "", "Path to GraphStore-ordered entries file (mutually exclusive with --copy_graphstore)")

	dbSpec = flag.String("db", "", "Postgres connection specification (see https://godoc.org/github.com/lib/pq#hdr-Connection_String_Parameters for details)")

	httpListeningAddr = flag.String("listen", "localhost:8080", "Listening address to launch xrefs service backed by Postgres")
	publicResources   = flag.String("public_resources", "", "Path to directory of static resources to serve")
)

func main() {
	flag.Parse()
	if flag.NArg() > 0 {
		flagutil.UsageErrorf("unexpected non-flag arguments: %v", flag.Args())
	} else if *dbSpec == "" {
		flagutil.UsageError("missing required --db connection string")
	} else if *entriesFile != "" && gs != nil {
		flagutil.UsageError("flags --copy_graphstore and --copy_entries are mutually exclusive")
	}

	log.Println("Connecting to db...")
	db, err := pq.Open(*dbSpec)
	fatalOnErr("error opening db: %v", err)
	defer func() {
		fatalOnErr("error closing db: %v", db.Close())
	}()
	log.Println("Connected!")

	ctx := context.Background()
	if err := profile.Start(ctx); err != nil {
		log.Fatal(err)
	}
	defer profile.Stop()

	var entries <-chan *spb.Entry
	if gs != nil {
		ch := make(chan *spb.Entry)
		go func() {
			defer close(ch)
			defer gs.Close(ctx)
			log.Println("Scanning GraphStore")
			fatalOnErr("error scanning GraphStore: %v",
				gs.Scan(ctx, &spb.ScanRequest{}, func(e *spb.Entry) error {
					ch <- e
					return nil
				}))
		}()
		entries = ch
	} else if *entriesFile != "" {
		f, err := vfs.Open(ctx, *entriesFile)
		if err != nil {
			log.Fatalf("Error opening %q: %v", *entriesFile, err)
		}
		defer f.Close()
		log.Println("Reading entries from", *entriesFile)
		entries = stream.ReadEntries(f)
	}

	if entries != nil {
		fatalOnErr("error copying entries: %v", db.CopyEntries(entries))
	}

	if *httpListeningAddr != "" {
		xrefs.RegisterHTTPHandlers(ctx, db, http.DefaultServeMux)
		filetree.RegisterHTTPHandlers(ctx, db, http.DefaultServeMux)
		if *publicResources != "" {
			log.Println("Serving public resources at", *publicResources)
			if s, err := os.Stat(*publicResources); err != nil {
				log.Fatalf("ERROR: could not get FileInfo for %q: %v", *publicResources, err)
			} else if !s.IsDir() {
				log.Fatalf("ERROR: %q is not a directory", *publicResources)
			}
			http.HandleFunc("/", func(w http.ResponseWriter, r *http.Request) {
				http.ServeFile(w, r, filepath.Join(*publicResources, filepath.Clean(r.URL.Path)))
			})
		}

		log.Printf("HTTP server listening on %q", *httpListeningAddr)
		log.Fatal(http.ListenAndServe(*httpListeningAddr, nil))
	}
}

func fatalOnErr(msg string, err error, args ...interface{}) {
	args = append([]interface{}{err}, args...)
	if err != nil {
		log.Fatalf(msg, args...)
	}
}
