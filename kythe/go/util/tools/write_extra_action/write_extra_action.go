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
//
//	write_extra_action <text_proto_file> <output_path>
package main

import (
	"context"
	"flag"
	"fmt"
	"io/ioutil"
	"os"
	"path/filepath"

	"kythe.io/kythe/go/platform/vfs"
	"kythe.io/kythe/go/util/log"

	"google.golang.org/protobuf/encoding/prototext"
	"google.golang.org/protobuf/proto"

	xapb "kythe.io/third_party/bazel/extra_actions_base_go_proto"
)

func main() {
	flag.Parse()

	if flag.NArg() < 2 || (flag.NArg() > 2 && flag.Arg(2) != "--") {
		fmt.Printf("usage: %s <text_proto_file> <output_path> [-- <arg>...]\n", filepath.Base(os.Args[0]))
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

	// C++ command line arguments are not available in Starlark, so the extension cannot
	// be completed populated there and must delegate to an external binary.
	if cc := proto.GetExtension(&xa, xapb.E_CppCompileInfo_CppCompileInfo).(*xapb.CppCompileInfo); cc != nil {
		populateCppCompileInfo(cc)
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

func populateCppCompileInfo(cc *xapb.CppCompileInfo) {
	if flag.NArg() <= 3 || flag.Arg(2) != "--" {
		return // No additional flags, end early.
	}
	cc.Tool = proto.String(flag.Arg(3))
	cc.CompilerOption = flag.Args()[4:flag.NArg()]
	for i, arg := range cc.CompilerOption {
		if arg == "-c" && i+1 < len(cc.CompilerOption) {
			cc.SourceFile = proto.String(cc.CompilerOption[i+1])
			break
		}
	}
}
