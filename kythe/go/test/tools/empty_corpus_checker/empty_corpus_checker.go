/*
 * Copyright 2021 The Kythe Authors. All rights reserved.
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

// Binary empty_corpus_checker reads an entrystream (in delimited binary proto
// form) from stdin and checks for any node vnames that have an empty corpus.
// Nodes with an empty corpus are logged to stderr and cause the binary to
// return a non-zero exit code.
package main

import (
	"bufio"
	"flag"
	"log"
	"os"

	"kythe.io/kythe/go/storage/entryset"
	"kythe.io/kythe/go/storage/stream"
	"kythe.io/kythe/go/util/flagutil"
	"kythe.io/kythe/go/util/kytheuri"

	intpb "kythe.io/kythe/proto/internal_go_proto"
	spb "kythe.io/kythe/proto/storage_go_proto"
)

func init() {
	flag.Usage = flagutil.SimpleUsage("Checks a stream of Entry protos via stdin for empty vname.corpus")
}

func main() {
	flag.Parse()

	in := bufio.NewReaderSize(os.Stdin, 2*4096)
	rd := stream.NewReader(in)

	set := entryset.New(nil)
	err := rd(func(entry *spb.Entry) error {
		return set.Add(entry)
	})
	if err != nil {
		log.Fatalf("Failed to read entrystream: %v", err)
	}
	set.Canonicalize()

	emptyCorpusCount := 0
	set.Sources(func(src *intpb.Source) bool {
		r, err := kytheuri.ParseRaw(src.GetTicket())
		if err != nil {
			log.Fatalf("Error parsing ticket: %q, %v", src.GetTicket(), err)
		}

		if r.URI.Corpus == "" {
			log.Printf("Found source with empty corpus: %v", src)
			emptyCorpusCount++
		}

		return true
	})
	if emptyCorpusCount != 0 {
		log.Fatalf("FAILURE: found %d sources with empty corpus", emptyCorpusCount)
	}
}
