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

// Binary dedup_stream reads a delimited stream from stdin and writes a delimited stream to stdout.
// Each record in the stream will be hashed, and if that hash value has already been seen, the
// record will not be emitted.
package main

import (
	"flag"
	"os"

	"kythe.io/kythe/go/platform/delimited"
	"kythe.io/kythe/go/platform/delimited/dedup"
	"kythe.io/kythe/go/util/datasize"
	"kythe.io/kythe/go/util/flagutil"
	"kythe.io/kythe/go/util/log"
)

func init() {
	flag.Usage = flagutil.SimpleUsage("Remove duplicate records from a delimited stream")
}

var cacheSize = datasize.Flag("cache_size", "3GiB", `Maximum size of the cache of known record hashes (e.g. "10B", "12KB", "3GiB", etc.)`)

func main() {
	flag.Parse()
	if flag.NArg() != 0 {
		flagutil.UsageErrorf("unknown arguments: %v", flag.Args())
	}

	rd, err := dedup.NewReader(os.Stdin, int(cacheSize.Bytes()))
	if err != nil {
		log.Fatalf("Error creating UniqReader: %v", err)
	}
	wr := delimited.NewWriter(os.Stdout)
	if err := delimited.Copy(wr, rd); err != nil {
		log.Fatal(err)
	}
	log.Infof("dedup_stream: skipped %d records", rd.Skipped())
}
