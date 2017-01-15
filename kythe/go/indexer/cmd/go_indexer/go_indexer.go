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

// Program go_indexer implements a Kythe indexer for the Go language.
// Input is read from one or more index pack or .kindex paths.
package main

import (
	"context"
	"encoding/json"
	"flag"
	"fmt"
	"io"
	"log"
	"os"
	"path/filepath"

	"kythe.io/kythe/go/indexer/indexer"
	"kythe.io/kythe/go/platform/delimited"
	"kythe.io/kythe/go/platform/indexpack"
	"kythe.io/kythe/go/platform/kindex"
	"kythe.io/kythe/go/platform/vfs"

	apb "kythe.io/kythe/proto/analysis_proto"
	spb "kythe.io/kythe/proto/storage_proto"
)

var (
	doIndexPack = flag.Bool("indexpack", false, "Treat arguments as index pack directories")
	doZipPack   = flag.Bool("zip", false, "Treat arguments as zipped indexpack files (implies -indexpack)")
	doJSON      = flag.Bool("json", false, "Write output as JSON")
	doLibNodes  = flag.Bool("libnodes", false, "Emit nodes for standard library packages")

	writeEntry func(context.Context, *spb.Entry) error
)

func init() {
	flag.Usage = func() {
		fmt.Fprintf(os.Stderr, `Usage: %s [options] <path>...

Generate Kythe graph data for the compilations stored in the index pack or
.kindex files named by the path arguments. Output is written to stdout.

By default, paths are assumed to denote .kindex files; if --indexpack is set,
the paths are treated as index packs instead.  If --zip is set, the index packs
are treated as ZIP files; otherwise they must be directories.

By default, the output is a delimited stream of wire-format Kythe Entry
protobuf messages. With the --json flag, output is instead a stream of
undelimited JSON messages.

Options:
`, filepath.Base(os.Args[0]))

		flag.PrintDefaults()
	}
}

func main() {
	flag.Parse()

	if flag.NArg() == 0 {
		log.Fatal("No input paths were specified to index")
	}
	if *doJSON {
		enc := json.NewEncoder(os.Stdout)
		writeEntry = func(_ context.Context, entry *spb.Entry) error {
			return enc.Encode(entry)
		}
	} else {
		rw := delimited.NewWriter(os.Stdout)
		writeEntry = func(_ context.Context, entry *spb.Entry) error {
			return rw.PutProto(entry)
		}
	}

	ctx := context.Background()
	for _, path := range flag.Args() {
		if err := visitPath(ctx, path, indexGo); err != nil {
			log.Fatalf("Error indexing %q: %v", path, err)
		}
	}
}

// indexGo is a visitFunct that invokes the Kythe Go indexer on unit.
func indexGo(ctx context.Context, unit *apb.CompilationUnit, f indexer.Fetcher) error {
	pi, err := indexer.Resolve(unit, f, indexer.XRefTypeInfo())
	if err != nil {
		return err
	}
	log.Printf("Finished resolving compilation: %s", pi.String())
	return pi.Emit(ctx, writeEntry, &indexer.EmitOptions{
		EmitStandardLibs: *doLibNodes,
	})
}

type visitFunc func(context.Context, *apb.CompilationUnit, indexer.Fetcher) error

// visitPath invokes visit for each compilation denoted by path, which is
// either a .kindex file (with a single compilation) or an index pack.
func visitPath(ctx context.Context, path string, visit visitFunc) error {
	if *doIndexPack || *doZipPack {
		return visitIndexPack(ctx, path, visit)
	}
	idx, err := kindex.Open(ctx, path)
	if err != nil {
		return fmt.Errorf("opening kindex file: %v", err)
	}
	return visit(ctx, idx.Proto, idx)
}

// visitIndexPack invokes visit for each Kythe compilation in the index pack at
// path. Any error returned by the visitor terminates the scan.
func visitIndexPack(ctx context.Context, path string, visit visitFunc) error {
	pack, err := openPack(ctx, path)
	if err != nil {
		return fmt.Errorf("opening indexpack: %v", err)
	}
	return pack.ReadUnits(ctx, "kythe", func(_ string, msg interface{}) error {
		return visit(ctx, msg.(*apb.CompilationUnit), pack.Fetcher(ctx))
	})
}

func openPack(ctx context.Context, path string) (*indexpack.Archive, error) {
	utype := indexpack.UnitType((*apb.CompilationUnit)(nil))
	if *doZipPack {
		fi, err := vfs.Stat(ctx, path)
		if err != nil {
			return nil, err
		} else if !fi.Mode().IsRegular() {
			return nil, fmt.Errorf("invalid zip file path: %v", path, err)
		}
		f, err := vfs.Open(ctx, path)
		if err != nil {
			return nil, err
		}
		return indexpack.OpenZip(ctx, f.(io.ReaderAt), fi.Size(), utype)
	}
	return indexpack.Open(ctx, path, utype)
}
