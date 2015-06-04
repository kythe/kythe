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

// Binary write_tables creates a combined xrefs/filetree/search serving table
// based on a given GraphStore.
package main

import (
	"flag"
	"log"

	"kythe.io/kythe/go/services/graphstore"
	"kythe.io/kythe/go/serving/pipeline"
	"kythe.io/kythe/go/storage/gsutil"
	"kythe.io/kythe/go/storage/leveldb"
	"kythe.io/kythe/go/util/flagutil"

	"golang.org/x/net/context"

	_ "kythe.io/kythe/go/services/graphstore/grpc"
	_ "kythe.io/kythe/go/services/graphstore/proxy"
)

var (
	gs graphstore.Service

	tablePath = flag.String("out", "", "Directory path to output serving table")
)

func init() {
	gsutil.Flag(&gs, "graphstore", "GraphStore to read")
	flag.Usage = flagutil.SimpleUsage("Creates a combined xrefs/filetree/search serving table based on a given GraphStore",
		"--graphstore spec --out path")
}
func main() {
	flag.Parse()
	if gs == nil {
		flagutil.UsageError("missing required --graphstore flag")
	} else if *tablePath == "" {
		flagutil.UsageError("missing required --out flag")
	}

	db, err := leveldb.Open(*tablePath, nil)
	if err != nil {
		log.Fatal(err)
	}
	defer db.Close()

	if err := pipeline.Run(context.Background(), gs, db); err != nil {
		log.Fatal(err)
	}
}
