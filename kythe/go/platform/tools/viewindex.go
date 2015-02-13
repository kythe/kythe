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

// Binary viewindex prints a .kindex as JSON to stdout.
//
// Example:
//   viewindex compilation.kindex | jq .
package main

import (
	"encoding/json"
	"flag"
	"fmt"
	"log"
	"os"
	"path/filepath"

	"kythe/go/platform/kindex"
)

func init() {
	flag.Usage = func() {
		fmt.Fprintf(os.Stderr, "usage: %s <kindex-file>\n", filepath.Base(os.Args[0]))
		flag.PrintDefaults()
	}
}

var printFiles = flag.Bool("files", false, "Print file contents as well as the compilation")

func main() {
	flag.Parse()

	if len(flag.Args()) != 1 {
		flag.Usage()
		os.Exit(1)
	}

	path := flag.Arg(0)
	idx, err := kindex.Open(path)
	if err != nil {
		log.Fatalf("Error reading %q: %v", path, err)
	}

	en := json.NewEncoder(os.Stdout)
	if *printFiles {
		if err := en.Encode(idx); err != nil {
			log.Fatalf("Error encoding JSON: %v", err)
		}
	} else {
		if err := en.Encode(idx.Proto); err != nil {
			log.Fatalf("Error encoding JSON compilation: %v", err)
		}
	}
}
