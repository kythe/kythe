/*
 * Copyright 2014 The Kythe Authors. All rights reserved.
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

// Binary viewindex prints a .kindex/.kzip as JSON to stdout.
//
// Example:
//   viewindex compilation.kzip | jq .
package main

import (
	"context"
	"encoding/json"
	"flag"
	"fmt"
	"log"
	"os"
	"strings"

	"kythe.io/kythe/go/platform/kindex"
	"kythe.io/kythe/go/platform/kzip"
	"kythe.io/kythe/go/util/flagutil"

	"github.com/golang/protobuf/jsonpb"

	apb "kythe.io/kythe/proto/analysis_go_proto"

	_ "kythe.io/kythe/proto/buildinfo_go_proto"
	_ "kythe.io/kythe/proto/cxx_go_proto"
	_ "kythe.io/kythe/proto/go_go_proto"
	_ "kythe.io/kythe/proto/java_go_proto"
)

func init() {
	flag.Usage = flagutil.SimpleUsage("Print a .kzip/.kindex archive as JSON to stdout",
		"[--files] <kzip-file/kindex-file>")
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
		flagutil.UsageError("missing kzip/kindex file path")
	} else if len(flag.Args()) > 1 {
		flagutil.UsageErrorf("unknown arguments: %v", flag.Args()[1:])
	}

	path := flag.Arg(0)
	if !strings.HasSuffix(path, ".kzip") {
		// Assume deprecated .kindex, by default
		viewKIndex(path)
		return
	}

	f, err := os.Open(path)
	if err != nil {
		log.Fatalf("Error opening %q: %v", path, err)
	}
	defer f.Close()

	info, err := f.Stat()
	if err != nil {
		log.Fatalf("Error getting size of %q: %v", path, err)
	}

	rd, err := kzip.NewReader(f, info.Size())
	if err != nil {
		log.Fatalf("Error opening kzip reader %q: %v", path, err)
	}

	if *printFile != "" {
		data, err := rd.ReadAll(*printFile)
		if err != nil {
			log.Fatalf("Error reading file with digest %q: %v", *printFile, err)
		} else if _, err := os.Stdout.Write(data); err != nil {
			log.Fatal(err)
		}
		return
	}

	out := os.Stdout
	en := json.NewEncoder(out)
	if err := rd.Scan(func(u *kzip.Unit) error {
		if !*printFiles {
			return m.Marshal(out, u.Proto)
		}

		idx := &kindex.Compilation{Proto: u.Proto}
		for _, f := range u.Proto.RequiredInput {
			data, err := rd.ReadAll(f.Info.GetDigest())
			if err != nil {
				return fmt.Errorf("reading file digest %q: %v", f.Info.GetDigest(), err)
			}
			idx.Files = append(idx.Files, &apb.FileData{Content: data, Info: f.Info})
		}

		return en.Encode(idx)
	}); err != nil {
		log.Fatalf("Error scanning kzip %q: %v", path, err)
	}
}

func viewKIndex(path string) {
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
		log.Fatalf("File digest %q not found", *printFile)
	} else {
		if err := m.Marshal(out, idx.Proto); err != nil {
			log.Fatalf("Error encoding JSON compilation: %v", err)
		}
	}
}
