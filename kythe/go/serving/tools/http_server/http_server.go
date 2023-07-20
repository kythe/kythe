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

// Binary http_server exposes HTTP interfaces for the xrefs and filetree
// services backed by a combined serving table.
package main

import (
	"context"
	"flag"
	"net/http"
	"os"
	"path/filepath"

	"kythe.io/kythe/go/services/filetree"
	"kythe.io/kythe/go/services/graph"
	"kythe.io/kythe/go/services/xrefs"
	ftsrv "kythe.io/kythe/go/serving/filetree"
	gsrv "kythe.io/kythe/go/serving/graph"
	"kythe.io/kythe/go/serving/identifiers"
	xsrv "kythe.io/kythe/go/serving/xrefs"
	"kythe.io/kythe/go/storage/leveldb"
	"kythe.io/kythe/go/storage/table"
	"kythe.io/kythe/go/util/flagutil"
	"kythe.io/kythe/go/util/log"

	"golang.org/x/net/http2"

	_ "kythe.io/kythe/go/services/graphstore/proxy"
)

var (
	servingTable = flag.String("serving_table", "", "LevelDB serving table")

	httpListeningAddr = flag.String("listen", "localhost:8080", "Listening address for HTTP server (\":<port>\" allows access from any machine)")
	httpAllowOrigin   = flag.String("http_allow_origin", "", "If set, each HTTP response will contain a Access-Control-Allow-Origin header with the given value")
	publicResources   = flag.String("public_resources", "", "Path to directory of static resources to serve")

	tlsListeningAddr = flag.String("tls_listen", "", "Listening address for TLS HTTP server")
	tlsCertFile      = flag.String("tls_cert_file", "", "Path to file with concatenation of TLS certificates")
	tlsKeyFile       = flag.String("tls_key_file", "", "Path to file with TLS private key")

	maxTicketsPerRequest = flag.Int("max_tickets_per_request", 20, "Maximum number of tickets allowed per request")
)

func init() {
	flag.Usage = flagutil.SimpleUsage("Exposes HTTP interfaces for the xrefs and filetree services",
		"(--graphstore spec | --serving_table path) [--listen addr] [--public_resources dir]")
}

func main() {
	flag.Parse()
	if *servingTable == "" {
		flagutil.UsageError("missing --serving_table")
	} else if *httpListeningAddr == "" && *tlsListeningAddr == "" {
		flagutil.UsageError("missing either --listen or --tls_listen argument")
	} else if *tlsListeningAddr != "" && (*tlsCertFile == "" || *tlsKeyFile == "") {
		flagutil.UsageError("--tls_cert_file and --tls_key_file are required if given --tls_listen")
	} else if flag.NArg() > 0 {
		flagutil.UsageErrorf("unknown non-flag arguments given: %v", flag.Args())
	}

	var (
		xs xrefs.Service
		gs graph.Service
		it identifiers.Service
		ft filetree.Service
	)

	ctx := context.Background()
	db, err := leveldb.Open(*servingTable, &leveldb.Options{MustExist: true})
	if err != nil {
		log.Fatalf("Error opening db at %q: %v", *servingTable, err)
	}
	defer db.Close(ctx)
	xs = xsrv.NewService(ctx, db)
	gs = gsrv.NewService(ctx, db)
	if *maxTicketsPerRequest > 0 {
		xs = xrefs.BoundedRequests{
			Service:    xs,
			MaxTickets: *maxTicketsPerRequest,
		}
		gs = graph.BoundedRequests{
			Service:    gs,
			MaxTickets: *maxTicketsPerRequest,
		}
	}
	tbl := &table.KVProto{db}
	ft = &ftsrv.Table{Proto: tbl, PrefixedKeys: true}
	it = &identifiers.Table{tbl}

	if *httpListeningAddr != "" || *tlsListeningAddr != "" {
		apiMux := http.NewServeMux()
		http.HandleFunc("/", func(w http.ResponseWriter, r *http.Request) {
			if *httpAllowOrigin != "" {
				w.Header().Set("Access-Control-Allow-Origin", *httpAllowOrigin)
			}
			apiMux.ServeHTTP(w, r)
		})

		xrefs.RegisterHTTPHandlers(ctx, xs, apiMux)
		graph.RegisterHTTPHandlers(ctx, gs, apiMux)
		identifiers.RegisterHTTPHandlers(ctx, it, apiMux)
		filetree.RegisterHTTPHandlers(ctx, ft, apiMux)
		if *publicResources != "" {
			log.Info("Serving public resources at", *publicResources)
			if s, err := os.Stat(*publicResources); err != nil {
				log.Fatalf("ERROR: could not get FileInfo for %q: %v", *publicResources, err)
			} else if !s.IsDir() {
				log.Fatalf("ERROR: %q is not a directory", *publicResources)
			}
			apiMux.HandleFunc("/", func(w http.ResponseWriter, r *http.Request) {
				http.ServeFile(w, r, filepath.Join(*publicResources, filepath.Clean(r.URL.Path)))
			})
		}
	}
	if *httpListeningAddr != "" {
		go startHTTP()
	}
	if *tlsListeningAddr != "" {
		go startTLS()
	}

	select {} // block forever
}

func startHTTP() {
	log.Infof("HTTP server listening on %q", *httpListeningAddr)
	log.Fatal(http.ListenAndServe(*httpListeningAddr, nil))
}

func startTLS() {
	srv := &http.Server{Addr: *tlsListeningAddr}
	http2.ConfigureServer(srv, nil)

	log.Infof("TLS HTTP2 server listening on %q", *tlsListeningAddr)
	log.Fatal(srv.ListenAndServeTLS(*tlsCertFile, *tlsKeyFile))
}
