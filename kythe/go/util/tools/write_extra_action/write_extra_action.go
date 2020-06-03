/*
 * Copyright 2019 The Kythe Authors. All rights reserved.
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

// Binary write_extra_action reads a textproto ExtraActionInfo, wire-encodes it,
// and writes it to another file.
//
// Usage:
//   write_extra_action <text_proto_file> <output_path>
package main

import (
	"context"
	"flag"
	"fmt"
	"io/ioutil"
	"log"
	"os"
	"path/filepath"

	"kythe.io/kythe/go/platform/vfs"

	"google.golang.org/protobuf/encoding/prototext"
	"google.golang.org/protobuf/proto"

	xapb "kythe.io/third_party/bazel/extra_actions_base_go_proto"
)

func main() {
	flag.Parse()

	if flag.NArg() != 2 {
		fmt.Printf("usage: %s <text_proto_file> <output_path>\n", filepath.Base(os.Args[0]))
		os.Exit(2)
	}

	ctx := context.Background()
	textFile := flag.Arg(0)
	outFile := flag.Arg(1)

	in, err := vfs.Open(ctx, textFile)
	if err != nil {
		log.Fatal(err)
	}

	txt, err := ioutil.ReadAll(in)
	in.Close()
	if err != nil {
		log.Fatal(err)
	}

	var xa xapb.ExtraActionInfo
	if err := prototext.Unmarshal(txt, &xa); err != nil {
		log.Fatal(err)
	}

	rec, err := proto.Marshal(&xa)
	if err != nil {
		log.Fatal(err)
	}

	f, err := vfs.Create(ctx, outFile)
	if err != nil {
		log.Fatal(err)
	} else if _, err := f.Write(rec); err != nil {
		log.Fatal(err)
	} else if err := f.Close(); err != nil {
		log.Fatal(err)
	}
}
