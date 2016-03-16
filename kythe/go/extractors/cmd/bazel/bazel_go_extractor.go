/*
 * Copyright 2016 Google Inc. All rights reserved.
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

// bazel_go_extractor is a Bazel extra action that extracts Go compilations.
package main

import (
	"flag"
	"fmt"
	"io"
	"io/ioutil"
	"log"
	"os"
	"path/filepath"

	"github.com/golang/protobuf/proto"
	"golang.org/x/net/context"

	"kythe.io/kythe/go/extractors/bazel"
	"kythe.io/kythe/go/storage/vnameutil"

	eapb "kythe.io/third_party/bazel/extra_actions_base_proto"
)

var config = &bazel.Config{
	Corpus:   "kythe",
	Mnemonic: "GoCompile",
	Root:     os.Getenv("PWD"),
}

const baseUsage = `Usage: %[1]s <extra-action> <output-file> <vname-config>`

func init() {
	flag.Usage = func() {
		fmt.Fprintf(os.Stderr, baseUsage+`

Extract a Kythe compilation record for Go from a Bazel extra action.

Arguments:
 <extra-action> is a file containing a wire format ExtraActionInfo protobuf.
 <output-file>  is the path where the output kindex file is written.
 <vname-config> is the path of a VName configuration JSON file.

Flags:
`, filepath.Base(os.Args[0]))
		flag.PrintDefaults()
	}
}

func main() {
	flag.Parse()
	if flag.NArg() != 3 {
		log.Fatalf(baseUsage+` [run "%[1]s --help" for details]`, filepath.Base(os.Args[0]))
	}

	ctx := context.Background()
	info, err := loadExtraAction(flag.Arg(0))
	if err != nil {
		log.Fatalf("Error loading extra action: %v", err)
	}
	rules, err := loadVNameRules(flag.Arg(2))
	if err != nil {
		log.Fatalf("Error loading vname rules: %v", err)
	}
	config.Rules = rules

	cu, err := config.Extract(ctx, info)
	if err != nil {
		log.Fatalf("Extraction failed: %v", err)
	}

	// Write and flush the output to a .kindex file.
	if err := writeToFile(ctx, cu, flag.Arg(1)); err != nil {
		log.Fatalf("Writing output failed: %v", err)
	}
}

// writeToFile creates the specified output file from the contents of w.
func writeToFile(ctx context.Context, w io.WriterTo, path string) error {
	f, err := os.Create(path)
	if err != nil {
		return err
	}
	if _, err := w.WriteTo(f); err != nil {
		f.Close()
		return err
	}
	return f.Close()
}

// loadExtraAction reads the contents of the file at path and decodes it as an
// ExtraActionInfo protobuf message.
func loadExtraAction(path string) (*eapb.ExtraActionInfo, error) {
	bits, err := ioutil.ReadFile(path)
	if err != nil {
		return nil, err
	}
	var info eapb.ExtraActionInfo
	if err := proto.Unmarshal(bits, &info); err != nil {
		return nil, err
	}
	return &info, nil
}

// loadVNameRules reads the contents of the file at path and decodes it as a
// slice of vname rewriting rules. The result is empty if path == "".
func loadVNameRules(path string) (vnameutil.Rules, error) {
	if path == "" {
		return nil, nil
	}
	bits, err := ioutil.ReadFile(path)
	if err != nil {
		return nil, err
	}
	return vnameutil.ParseRules(bits)
}
