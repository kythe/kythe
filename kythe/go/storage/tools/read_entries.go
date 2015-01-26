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

// Binary read_entries scans the entries from a specified GraphStore and emits
// them to stdout as a delimited stream.
package main

import (
	"flag"
	"fmt"
	"log"
	"os"
	"path/filepath"

	"kythe/go/platform/delimited"
	"kythe/go/storage"
	"kythe/go/storage/gsutil"

	spb "kythe/proto/storage_proto"
)

var gs storage.GraphStore

func init() {
	gsutil.Flag(&gs, "graphstore", "GraphStore to read")
	flag.Usage = func() {
		fmt.Fprintf(os.Stderr, `%s - scans the entries from a GraphStore, emitting a delimited entry stream to stdout
usage: %[1]s --graphstore spec
`, filepath.Base(os.Args[0]))
		flag.PrintDefaults()
		os.Exit(1)
	}
}

func main() {
	flag.Parse()
	if gs == nil {
		log.Fatal("Missing --graphstore")
	}

	wr := delimited.NewWriter(os.Stdout)
	if err := storage.EachScanEntry(gs, nil, func(entry *spb.Entry) error {
		return wr.PutProto(entry)
	}); err != nil {
		log.Fatalf("GraphStore Scan error: %v", err)
	}
}
