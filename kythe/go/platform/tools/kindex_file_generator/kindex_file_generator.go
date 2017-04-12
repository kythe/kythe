/*
 * Copyright 2017 Google Inc. All rights reserved.
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

// Binary kindex_file_generator generates a kindex index file for a given source
// input file.
//
// Example:
//  kindex_file_generator -uri <uri> -output <path> -source <path>
package main

import (
	"bufio"
	"flag"
	"fmt"
	"log"
	"os"
	"path/filepath"

	"kythe.io/kythe/go/platform/kindex"
	"kythe.io/kythe/go/util/kytheuri"

	spb "kythe.io/kythe/proto/storage_proto"
)

var (
	unitURI    = flag.String("uri", "", "A Kythe URI naming the compilation unit")
	outputPath = flag.String("output", "", "Path for output kindex file")
	sourcePath = flag.String("source", "", "Path for input source file")
)

func init() {
	flag.Usage = func() {
		fmt.Fprintf(os.Stderr, `Usage: %s -uri <uri> -output <path> -source <path>

Construct a kindex file consisting of a single input file name by -source.
The resulting file is written to -output, and the vname of the compilation
will be attributed to the values specified within -uri.

Options:
`, filepath.Base(os.Args[0]))
		flag.PrintDefaults()
	}
}

func verifyFlags() {
	if len(flag.Args()) > 0 {
		log.Fatal("Unknown arguments: %v", flag.Args())
	}

	switch {
	case *unitURI == "":
		log.Fatal("You must provide a non-empty -uri")
	case *outputPath == "":
		log.Fatal("You must provide a non-empty -output")
	case *sourcePath == "":
		log.Fatal("You must provide a non-empty -source")
	}
}

func main() {
	flag.Parse()
	verifyFlags()

	// Create a new compilation populating its VName with the values specified
	// within the specified Kythe URI.
	uri, err := kytheuri.Parse(*unitURI)
	if err != nil {
		log.Fatalf("Error parsing -uri: %v", err)
	}
	var compilation kindex.Compilation
	unit := compilation.Unit()
	unit.VName = &spb.VName{
		Corpus:    uri.Corpus,
		Language:  uri.Language,
		Signature: uri.Signature,
		Root:      uri.Root,
		Path:      uri.Path,
	}

	// add the input soure file to the compilation
	src, err := os.Open(*sourcePath)
	if err != nil {
		log.Fatalf("Error opening -source: %v", err)
	}
	defer src.Close()
	if err = compilation.AddFile(*sourcePath, bufio.NewReader(src), &spb.VName{
		Corpus:   uri.Corpus,
		Language: uri.Language,
		Root:     uri.Root,
		Path:     *sourcePath,
	}); err != nil {
		log.Fatalf("Error adding -source to CU: %v", err)
	}

	// write the compilation to the kindex output file
	out, err := os.Create(*outputPath)
	if err != nil {
		log.Fatalf("Error opening -output: %v", err)
	}
	if _, err := compilation.WriteTo(out); err != nil {
		log.Fatalf("Error writing compilation to output: %v", err)
	} else if err := out.Close(); err != nil {
		log.Fatalf("Error closing output file: %v", err)
	}
}
