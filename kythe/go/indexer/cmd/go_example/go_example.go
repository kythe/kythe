/*
 * Copyright 2016 The Kythe Authors. All rights reserved.
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

// Program go_example runs the Kythe Go indexer on a single package consisting
// of files named on the command line, for use in verifying schema examples.
package main

import (
	"context"
	"flag"
	"fmt"
	"go/build"
	"io"
	"io/ioutil"
	"os"
	"path/filepath"

	"kythe.io/kythe/go/extractors/golang"
	"kythe.io/kythe/go/indexer"
	"kythe.io/kythe/go/platform/analysis"
	"kythe.io/kythe/go/platform/delimited"
	"kythe.io/kythe/go/platform/vfs"
	"kythe.io/kythe/go/util/log"

	apb "kythe.io/kythe/proto/analysis_go_proto"
	spb "kythe.io/kythe/proto/storage_go_proto"
)

var (
	importPath = flag.String("package", "example", "Package import path")

	bc = build.Default
)

func init() {
	flag.StringVar(&bc.GOROOT, "goroot", filepath.Join(filepath.Dir(os.Args[0]), "go_example.runfiles/go_sdk"),
		"Use this directory as the root for Go library packages")

	flag.Usage = func() {
		fmt.Fprintf(os.Stderr, `Usage: %s -package <ipath> <source-file>...

Generate Kythe graph data for a single compilation expressed as a sequence of
Go source files.  Because only a single package is generated, only standard
library packages may be imported.

Options:
`, filepath.Base(os.Args[0]))

		flag.PrintDefaults()
	}
}

func main() {
	flag.Parse()

	if flag.NArg() == 0 {
		log.Fatal("No input paths were specified to index")
	} else if *importPath == "" {
		log.Fatal("You must provide a non-empty --package path")
	}

	ctx := context.Background()
	base, err := ioutil.TempDir("", "go_example")
	if err != nil {
		log.Fatalf("Error creating temporary directory: %v", err)
	}
	defer os.RemoveAll(base) // best-effort
	dir := filepath.Join(base, "src", *importPath)
	if err := vfs.MkdirAll(ctx, dir, 0755); err != nil {
		log.Fatalf("Error creating output directory: %v", err)
	}
	bc.GOPATH = base

	for _, path := range flag.Args() {
		if err := copyFile(ctx, path, dir); err != nil {
			log.Fatalf("Error copying %q: %v", path, err)
		}
	}

	ext := &golang.Extractor{BuildContext: bc}
	if err := os.Chdir(base); err != nil {
		log.Fatalf("Error changing directory to %q: %v", dir, err)
	}
	pkgs, err := ext.Locate(*importPath)
	if err != nil {
		log.Fatalf("Error locating package: %v", err)
	}
	if err := ext.Extract(); err != nil {
		log.Fatalf("Error extracting package; %v", err)
	}

	rw := delimited.NewWriter(os.Stdout)
	for _, pkg := range pkgs {
		if err := pkg.EachUnit(ctx, func(unit *apb.CompilationUnit, f analysis.Fetcher) error {
			pi, err := indexer.Resolve(unit, f, &indexer.ResolveOptions{
				Info: indexer.XRefTypeInfo(),
			})
			if err != nil {
				return err
			}
			return pi.Emit(ctx, func(_ context.Context, entry *spb.Entry) error {
				return rw.PutProto(entry)
			}, nil)
		}); err != nil {
			log.Fatalf("Error indexing: %v", err)
		}
	}
}

// copyFile copies the file named by path into the directory named by dir.
func copyFile(ctx context.Context, path, dir string) error {
	outPath := filepath.Join(dir, filepath.Base(path))
	in, err := vfs.Open(ctx, path)
	if err != nil {
		return err
	}
	defer in.Close()
	out, err := vfs.Create(ctx, outPath)
	if err != nil {
		return err
	}
	_, err = io.Copy(out, in)
	cerr := out.Close()
	if err != nil {
		return err
	}
	return cerr
}
