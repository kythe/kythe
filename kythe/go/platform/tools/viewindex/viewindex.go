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
	"context"
	"encoding/json"
	"flag"
	"fmt"
	"log"
	"os"

	"kythe.io/kythe/go/platform/kindex"
	"kythe.io/kythe/go/util/flagutil"

	"github.com/golang/protobuf/jsonpb"

	_ "kythe.io/kythe/proto/buildinfo_go_proto"
	_ "kythe.io/kythe/proto/cxx_go_proto"
	_ "kythe.io/kythe/proto/go_go_proto"
	_ "kythe.io/kythe/proto/java_go_proto"
)

func init() {
	flag.Usage = flagutil.SimpleUsage("Print a .kindex archive as JSON to stdout",
		"[--files] <kindex-file>")
}

var (
	printFiles = flag.Bool("files", false, "Print all file contents as well as the compilation")
	printFile  = flag.String("file", "", "Only print the file contents for the given digest")

	m = &jsonpb.Marshaler{
		OrigName: true,
	}
)

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

	out := os.Stdout
	if *printFiles {
		if err := json.NewEncoder(out).Encode(idx); err != nil {
			log.Fatalf("Error encoding JSON: %v", err)
		}
	} else if *printFile != "" {
		for _, f := range idx.Files {
			if f.Info.GetDigest() == *printFile {
				if _, err := os.Stdout.Write(f.Content); err != nil {
					log.Fatal(err)
				}
				return
			}
		}
		fmt.Fprintf(os.Stderr, "File digest %q not found\n", *printFile)
		os.Exit(1)
	} else {
		if err := m.Marshal(out, idx.Proto); err != nil {
			log.Fatalf("Error encoding JSON compilation: %v", err)
		}
	}
}
