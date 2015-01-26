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

// Binary indexpack is a utility to transfer compilations units in .kindex files
// to/from indexpack archives.
//
// Usages:
//   indexpack --from_archive <root> [dir]
//   indexpack --to_archive   <root> <kindex-file>...
package main

import (
	"crypto/sha256"
	"encoding/hex"
	"encoding/json"
	"flag"
	"fmt"
	"log"
	"os"
	"path/filepath"
	"strings"

	"kythe/go/platform/indexinfo"
	"kythe/go/platform/indexpack"

	apb "kythe/proto/analysis_proto"

	"golang.org/x/net/context"
)

const formatKey = "kythe"

var (
	toArchive   = flag.String("to_archive", "", "Move kindex files into the given indexpack archive")
	fromArchive = flag.String("from_archive", "", "Move the compilation units from the given archive into separate kindex files")

	quiet = flag.Bool("quiet", false, "Suppress normal log output")
)

func init() {
	flag.Usage = func() {
		fmt.Fprintln(os.Stderr, `Usage: indexpack --to_archive <root> [kindex-paths...]
       indexpack --from_archive <root> [dir]`)
		flag.PrintDefaults()
		os.Exit(1)
	}
}

func main() {
	flag.Parse()

	if *toArchive != "" && *fromArchive != "" {
		fmt.Fprintln(os.Stderr, "ERROR: --to_archive and --from_archive are mutually exclusive")
		flag.Usage()
	} else if *toArchive == "" && *fromArchive == "" {
		fmt.Fprintln(os.Stderr, "ERROR: Either --to_archive or --from_archive must be specified")
		flag.Usage()
	}

	archiveRoot := *toArchive
	if archiveRoot == "" {
		archiveRoot = *fromArchive
	}

	ctx := context.Background()
	pack, err := indexpack.CreateOrOpen(ctx, archiveRoot, indexpack.UnitType(apb.CompilationUnit{}))
	if err != nil {
		log.Fatalf("Error opening indexpack at %q: %v", archiveRoot, err)
	}

	if *toArchive != "" {
		if len(flag.Args()) == 0 {
			log.Println("WARNING: no kindex file paths given")
		}
		for _, path := range flag.Args() {
			kindex, err := indexinfo.Open(path)
			if err != nil {
				log.Fatalf("Error opening kindex at %q: %v", path, err)
			}
			if err := packIndex(ctx, pack, kindex); err != nil {
				log.Fatalf("Error packing kindex at %q into %q: %v", path, pack.Root(), err)
			}
		}
	} else {
		var dir string
		if len(flag.Args()) > 1 {
			fmt.Fprintf(os.Stderr, "ERROR: Too many positional arguments for --from_archive: %v\n", flag.Args())
			flag.Usage()
		} else if len(flag.Args()) == 1 {
			dir = flag.Arg(0)
			if err := os.MkdirAll(dir, os.ModePerm); err != nil {
				log.Fatalf("Error creating directory %q: %v", dir, err)
			}
		}
		if err := unpackIndex(ctx, pack, dir); err != nil {
			log.Fatalf("Error unpacking compilation units at %q: %v", pack.Root(), err)
		}
	}
}

func packIndex(ctx context.Context, pack *indexpack.Archive, kindex *indexinfo.IndexInfo) error {
	for _, data := range kindex.Files {
		if _, err := pack.WriteFile(ctx, data.Content); err != nil {
			return fmt.Errorf("error writing file %v: %v", data.Info, err)
		}
	}

	path, err := pack.WriteUnit(ctx, formatKey, kindex.Compilation)
	if err != nil {
		return fmt.Errorf("error writing compilation unit: %v", err)
	}
	if !*quiet {
		fmt.Println(strings.TrimSuffix(path, ".unit"))
	}
	return nil
}

func unpackIndex(ctx context.Context, pack *indexpack.Archive, dir string) error {
	return pack.ReadUnits(ctx, formatKey, func(u interface{}) error {
		unit, ok := u.(*apb.CompilationUnit)
		if !ok {
			return fmt.Errorf("%T is not a CompilationUnit", u)
		}
		kindex := &indexinfo.IndexInfo{Compilation: unit}
		for _, input := range unit.RequiredInput {
			if !*quiet {
				log.Println("Reading file", input.GetInfo().GetDigest())
			}
			data, err := pack.ReadFile(ctx, input.GetInfo().GetDigest())
			if err != nil {
				return fmt.Errorf("error reading required input (%v): %v", input, err)
			}
			kindex.Files = append(kindex.Files, &apb.FileData{
				Info:    input.Info,
				Content: data,
			})
		}
		path := kindexPath(dir, kindex)
		if !*quiet {
			log.Println("Writing compilation unit to", path)
		}
		f, err := os.Create(path)
		if err != nil {
			return fmt.Errorf("error creating kindex file: %v", err)
		}
		if _, err := kindex.WriteTo(f); err != nil {
			f.Close() // try to close file before returning
			return fmt.Errorf("error writing to kindex file: %v", err)
		}
		return f.Close()
	})
}

func kindexPath(dir string, kindex *indexinfo.IndexInfo) string {
	name := kindex.Compilation.VName.GetSignature()
	if name == "" {
		h := sha256.New()
		data, err := json.Marshal(kindex.Compilation)
		if err != nil {
			panic(err)
		}
		h.Write(data)
		name = hex.EncodeToString(h.Sum(nil))
	}
	name = strings.Trim(strings.Replace(name, "/", "_", -1), "/")
	return filepath.Join(dir, name+indexinfo.FileExt)
}
