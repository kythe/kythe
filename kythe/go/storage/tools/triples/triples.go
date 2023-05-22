/*
 * Copyright 2014 The Kythe Authors. All rights reserved.
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

// Binary triples implements a converter from an Entry stream to a stream of triples.
//
// Examples:
//
//	triples < entries > triples.nq
//	triples entries > triples.nq.gz
//	triples --graphstore path/to/gs > triples.nq.gz
//	triples entries triples.nq
//
// Reference: http://en.wikipedia.org/wiki/N-Triples
package main

import (
	"context"
	"encoding/base64"
	"flag"
	"fmt"
	"io"
	"os"

	"kythe.io/kythe/go/platform/vfs"
	"kythe.io/kythe/go/services/graphstore"
	"kythe.io/kythe/go/storage/gsutil"
	"kythe.io/kythe/go/storage/stream"
	"kythe.io/kythe/go/util/encoding/rdf"
	"kythe.io/kythe/go/util/flagutil"
	"kythe.io/kythe/go/util/kytheuri"
	"kythe.io/kythe/go/util/log"
	"kythe.io/kythe/go/util/schema/edges"
	"kythe.io/kythe/go/util/schema/facts"

	spb "kythe.io/kythe/proto/storage_go_proto"

	_ "kythe.io/kythe/go/services/graphstore/proxy"
	_ "kythe.io/kythe/go/storage/leveldb"
)

var (
	keepReverseEdges = flag.Bool("keep_reverse_edges", false, "Do not filter reverse edges from triples output")
	quiet            = flag.Bool("quiet", false, "Do not emit logging messages")

	gs graphstore.Service
)

func init() {
	gsutil.Flag(&gs, "graphstore", "Path to GraphStore to convert to triples (instead of an entry stream)")
	flag.Usage = flagutil.SimpleUsage("Converts an Entry stream to a stream of triples",
		"[(--graphstore path | entries_file) [triples_out]]")
}

func main() {
	flag.Parse()

	if len(flag.Args()) > 2 || (gs != nil && len(flag.Args()) > 1) {
		fmt.Fprintf(os.Stderr, "ERROR: too many arguments %v\n", flag.Args())
		flag.Usage()
		os.Exit(1)
	}

	if gs != nil {
		defer gsutil.LogClose(context.Background(), gs)
	}

	var in io.ReadCloser = os.Stdin
	if gs == nil && len(flag.Args()) > 0 {
		file, err := vfs.Open(context.Background(), flag.Arg(0))
		if err != nil {
			log.Fatalf("Failed to open input file %q: %v", flag.Arg(0), err)
		}
		defer file.Close()
		in = file
	}

	outIdx := 1
	if gs != nil {
		outIdx = 0
	}

	var out io.WriteCloser = os.Stdout
	if len(flag.Args()) > outIdx {
		file, err := vfs.Create(context.Background(), flag.Arg(outIdx))
		if err != nil {
			log.Fatalf("Failed to create output file %q: %v", flag.Arg(outIdx), err)
		}
		defer file.Close()
		out = file
	}

	var (
		entries      <-chan *spb.Entry
		reverseEdges int
		triples      int
	)

	if gs == nil {
		entries = stream.ReadEntries(in)
	} else {
		ch := make(chan *spb.Entry)
		entries = ch
		go func() {
			defer close(ch)
			if err := gs.Scan(context.Background(), &spb.ScanRequest{}, func(e *spb.Entry) error {
				ch <- e
				return nil
			}); err != nil {
				log.Fatalf("Error scanning graphstore: %v", err)
			}
		}()
	}

	for entry := range entries {
		if edges.IsReverse(entry.EdgeKind) && !*keepReverseEdges {
			reverseEdges++
			continue
		}

		t, err := toTriple(entry)
		if err != nil {
			log.Fatal(err)
		}
		fmt.Fprintln(out, t)
		triples++
	}

	if !*quiet {
		if !*keepReverseEdges {
			log.Infof("Skipped %d reverse edges", reverseEdges)
		}
		log.Infof("Wrote %d triples", triples)
	}
}

// toTriple converts an Entry to the triple file format. Returns an error if
// the entry is not valid.
func toTriple(entry *spb.Entry) (*rdf.Triple, error) {
	if err := graphstore.ValidEntry(entry); err != nil {
		return nil, fmt.Errorf("invalid entry {%+v}: %v", entry, err)
	}

	t := &rdf.Triple{
		Subject: kytheuri.FromVName(entry.Source).String(),
	}
	if graphstore.IsEdge(entry) {
		t.Predicate = entry.EdgeKind
		t.Object = kytheuri.FromVName(entry.Target).String()
	} else if entry.FactName == facts.Code {
		t.Predicate = entry.FactName
		t.Object = base64.StdEncoding.EncodeToString(entry.FactValue)
	} else {
		t.Predicate = entry.FactName
		t.Object = string(entry.FactValue)
	}
	return t, nil
}
