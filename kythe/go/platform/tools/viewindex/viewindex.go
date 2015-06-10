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
	"log"
	"os"

	"kythe.io/kythe/go/platform/kindex"
	"kythe.io/kythe/go/util/flagutil"

	"golang.org/x/net/context"
)

func init() {
	flag.Usage = flagutil.SimpleUsage("Print a .kindex archive as JSON to stdout",
		"[--files] <kindex-file>")
}

var printFiles = flag.Bool("files", false, "Print file contents as well as the compilation")

func main() {
	flag.Parse()
	if len(flag.Args()) == 0 {
		flagutil.UsageError("missing kindex-file path")
	} else if len(flag.Args()) > 1 {
		flagutil.UsageErrorf("unknown arguments: %v", flag.Args()[1:])
	}

	path := flag.Arg(0)
	idx, err := kindex.Open(context.Background(), path)
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
