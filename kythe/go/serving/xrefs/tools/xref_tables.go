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

// Binary xref_tables produces sorted JSON lookup tables from a GraphStore for
// cached xrefs.Service calls.
package main

import (
	"flag"
	"log"
	"strings"

	"kythe/go/services/graphstore"
	srvx "kythe/go/serving/xrefs"
	"kythe/go/storage/gsutil"
	sxrefs "kythe/go/storage/xrefs"
)

var (
	outDir            = flag.String("out_dir", "tables", "Output dir for tables")
	dumpText          = flag.Bool("dump_text", false, "Dump each table to a text file")
	skipPreprocessing = flag.Bool("skip_preprocessing", false, "Skip GraphStore preprocessing")

	gs graphstore.Service
)

func init() {
	gsutil.Flag(&gs, "graphstore", "GraphStore for which to build xrefs tables")
}

func main() {
	flag.Parse()

	if gs != nil && !*skipPreprocessing {
		fatalOnErr("Failed to add reverse edges: %v", sxrefs.AddReverseEdges(gs))
	}

	tbls, err := srvx.Open(*outDir)
	fatalOnErr("Failed to open xrefs tables: %v", err)
	if gs == nil {
		log.Println("WARNING: no --graphstore given to fill tables.")
	} else {
		fatalOnErr("Failed to fill tables: %v", tbls.Fill(gs))
	}

	if *dumpText {
		log.Println("Dumping text tables")
		fatalOnErr("Failed to dump text tables: %v", tbls.DumpToText())
	}
}

func fatalOnErr(msg string, err error) {
	if !strings.Contains(msg, "%v") {
		panic("fatalOnErr message must contain %v format specifier")
	}
	if err != nil {
		log.Fatalf(msg, err)
	}
}
