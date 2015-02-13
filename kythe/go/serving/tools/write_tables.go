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

// Binary write_tables creates a combined xrefs/filetree serving table based on
// a given graphstore.
package main

import (
	"flag"
	"log"

	"kythe/go/services/graphstore"
	"kythe/go/serving/pipeline"
	"kythe/go/storage/gsutil"
	"kythe/go/storage/leveldb"
)

var (
	gs graphstore.Service

	tablePath = flag.String("out", "", "Directory path to output serving table")
)

func init() {
	gsutil.Flag(&gs, "graphstore", "GraphStore to read")
}
func main() {
	flag.Parse()
	if gs == nil {
		log.Fatal("Missing required --graphstore argument")
	}

	db, err := leveldb.Open(*tablePath, nil)
	if err != nil {
		log.Fatal(err)
	}
	defer db.Close()

	if err := pipeline.Run(gs, db); err != nil {
		log.Fatal(err)
	}
}
