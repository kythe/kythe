/*
 * Copyright 2018 Google Inc. All rights reserved.
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

// Binary pack2kzip converts an indexpack directory to a kzip file.
package main

import (
	"bytes"
	"context"
	"flag"
	"fmt"
	"log"
	"os"
	"path/filepath"
	"strings"
	"time"

	"bitbucket.org/creachadair/stringset"
	"golang.org/x/sync/errgroup"

	"kythe.io/kythe/go/platform/indexpack"
	"kythe.io/kythe/go/platform/kcd/kythe"
	"kythe.io/kythe/go/platform/kzip"

	apb "kythe.io/kythe/proto/analysis_go_proto"
)

var (
	kzipPath = flag.String("output", "", "Output kzip filename")
	packPath = flag.String("input", "", "Input indexpack directory or zip path")
	revision = flag.String("revision", "", "Add this revision marker")
)

func init() {
	flag.Usage = func() {
		fmt.Fprintf(os.Stderr, `Usage: %s -input p -output k [-revision r]

Copy the compilations and required input files stored in an indexpack directory
p (or p.zip) to a newly-created .kzip file k. If -revision is set, r is set as
a revision marker for each compilation unit copied.

Options:
`, filepath.Base(os.Args[0]))
		flag.PrintDefaults()
	}
}

func main() {
	flag.Parse()
	if *kzipPath == "" || *packPath == "" {
		log.Fatal("You must specify both an --input indexpack and an --output kzip")
	} else if _, err := os.Stat(*kzipPath); err == nil {
		log.Fatalf("Output file %q already exists", *kzipPath)
	}

	ctx := context.Background()
	pack, err := openPack(ctx, *packPath)
	if err != nil {
		log.Fatalf("Opening indexpack: %v", err)
	}
	f, err := os.Create(*kzipPath)
	if err != nil {
		log.Fatalf("Creating kzip file: %v", err)
	}
	kw, err := kzip.NewWriter(f)
	if err != nil {
		log.Fatalf("Creating kzip writer: %v", err)
	}

	var index *apb.IndexedCompilation_Index
	if *revision != "" {
		index = &apb.IndexedCompilation_Index{Revisions: []string{*revision}}
	}

	log.Printf("Begin copying from %q to %q ...", *packPath, *kzipPath)
	start := time.Now()

	// Copy all the compilation records, and gather the digests of all the
	// unique files that need to be copied.
	var g errgroup.Group

	fileDigests := stringset.New()
	numUnits := 0
	if err := pack.ReadUnits(ctx, kythe.Format, func(digest string, unit interface{}) error {
		cu := unit.(*apb.CompilationUnit)
		for _, ri := range cu.RequiredInput {
			fileDigests.Add(ri.Info.GetDigest())
		}
		g.Go(func() error {
			if _, err := kw.AddUnit(cu, index); err != nil {
				return fmt.Errorf("storing compilation for digest %q: %v", digest, err)
			}
			return nil
		})
		numUnits++
		if numUnits%500 == 0 {
			log.Printf("[...] scanned %d compilations so far [%v]", numUnits, time.Since(start))
		}
		return nil
	}); err != nil {
		log.Fatalf("Scanning indexpack: %v", err)
	}
	if err := g.Wait(); err != nil {
		log.Fatalf("Copying units failed: %v", err)
	}
	log.Printf("Copied %d compilation records [%v elapsed]", numUnits, time.Since(start))
	log.Printf("Found %d unique file digests", len(fileDigests))

	// Copy all the file contents...
	for fd := range fileDigests {
		fd := fd
		g.Go(func() error {
			data, err := pack.ReadFile(ctx, fd)
			if err != nil {
				return err
			}
			got, err := kw.AddFile(bytes.NewReader(data))
			if err != nil {
				return fmt.Errorf("adding file %q: %v", fd, err)
			} else if got != fd {
				log.Printf("WARNING: Input file digest %q written as %q", fd, got)
			}
			return nil
		})
	}
	if err := g.Wait(); err != nil {
		log.Fatalf("Copying files failed: %v", err)
	}
	log.Printf("Copied %d files [%v elapsed]", len(fileDigests), time.Since(start))
	if err := kw.Close(); err != nil {
		log.Fatalf("Closing kzip writer: %v", err)
	} else if err := f.Close(); err != nil {
		log.Fatalf("Closing kzip file: %v", err)
	}
	log.Printf("Conversion finished [%v elapsed]", time.Since(start))
}

var unitType = indexpack.UnitType((*apb.CompilationUnit)(nil))

// openPack returns an open indexpack for a directory or a ZIP file at path.
func openPack(ctx context.Context, path string) (*indexpack.Archive, error) {
	if !strings.HasSuffix(path, ".zip") {
		return indexpack.Open(ctx, path, unitType)
	}
	f, err := os.Open(path)
	if err != nil {
		return nil, err
	}
	fi, err := f.Stat()
	if err != nil {
		f.Close()
		return nil, err
	}
	return indexpack.OpenZip(ctx, f, fi.Size(), unitType)
}
