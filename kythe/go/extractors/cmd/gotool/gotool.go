/*
 * Copyright 2015 The Kythe Authors. All rights reserved.
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

// Binary gotool extracts Kythe compilation information for Go packages named
// by import path on the command line.  The output compilations are written
// into a kzip.
package main

import (
	"bytes"
	"context"
	"flag"
	"fmt"
	"go/build"
	"os"
	"path/filepath"
	"strings"

	"kythe.io/kythe/go/extractors/golang"
	"kythe.io/kythe/go/platform/analysis"
	"kythe.io/kythe/go/platform/kzip"
	"kythe.io/kythe/go/platform/vfs"
	"kythe.io/kythe/go/util/flagutil"
	"kythe.io/kythe/go/util/log"
	"kythe.io/kythe/go/util/vnameutil"
	apb "kythe.io/kythe/proto/analysis_go_proto"
)

var (
	bc = build.Default // A shallow copy of the default build settings

	corpus     = flag.String("corpus", "", "Default corpus name to use")
	rulesFile  = flag.String("rules", "", "Path to vnames.json file that maps file paths to output corpus, root, and path.")
	outputPath = flag.String("output", "", "KZip output path")
	extraFiles = flag.String("extra_files", "", "Additional files to include in each compilation (CSV)")
	byDir      = flag.Bool("bydir", false, "Import by directory rather than import path")
	keepGoing  = flag.Bool("continue", false, "Continue past errors")
	verbose    = flag.Bool("v", false, "Enable verbose logging")

	canonicalizePackageCorpus = flag.Bool("canonicalize_package_corpus", false, "Whether to use a package's canonical repository root URL as their corpus")
	useDefaultCorpusForStdLib = flag.Bool("use_default_corpus_for_stdlib", false, "By default, go stdlib files are given the 'golang.org' corpus. If this flag is enabled, they will instead be assigned the corpus from the --corpus flag.")
	useDefaultCorpusForDeps   = flag.Bool("use_default_corpus_for_deps", false, "By default, imported modules are assigned a corpus based on their import path. If this flag is enabled, they will instead be assigned the corpus from the --corpus flag and a root corresponding the their import path.")

	buildTags flagutil.StringList
)

func init() {
	flag.Usage = func() {
		fmt.Fprintf(os.Stderr, `Usage: %s [options] <import-path>...
Extract Kythe compilation records from Go import paths specified on the command line.
Output is written to a .kzip file specified by --output.

Options:
`, filepath.Base(os.Args[0]))
		flag.PrintDefaults()
	}

	// Attach flags to the various parts of the go/build context we are using.
	// These will override the system defaults from the environment.
	flag.StringVar(&bc.GOARCH, "goarch", bc.GOARCH, "Go system architecture tag")
	flag.StringVar(&bc.GOOS, "goos", bc.GOOS, "Go operating system tag")
	flag.StringVar(&bc.GOPATH, "gopath", bc.GOPATH, "Go library path")
	flag.StringVar(&bc.GOROOT, "goroot", bc.GOROOT, "Go system root")
	flag.BoolVar(&bc.CgoEnabled, "gocgo", bc.CgoEnabled, "Whether to allow cgo")
	flag.StringVar(&bc.Compiler, "gocompiler", bc.Compiler, "Which Go compiler to use")
	flag.Var(&buildTags, "buildtags", "Comma-separated list of Go +build tags to enable during extraction.")

	// TODO(fromberger): Attach flags to the build and release tags (maybe).
}

func maybeFatal(msg string, args ...any) {
	log.Errorf(msg, args...)
	if !*keepGoing {
		os.Exit(1)
	}
}

func maybeLog(msg string, args ...any) {
	if *verbose {
		log.Infof(msg, args...)
	}
}

func main() {
	flag.Parse()

	bc.BuildTags = buildTags

	if *outputPath == "" {
		log.Fatal("You must provide a non-empty --output path")
	}

	// Rules for rewriting package and file VNames.
	var rules vnameutil.Rules
	if *rulesFile != "" {
		var err error
		rules, err = vnameutil.LoadRules(*rulesFile)
		if err != nil {
			log.Fatalf("loading rules file: %v", err)
		}
	}

	ctx := context.Background()
	ext := &golang.Extractor{
		BuildContext: bc,

		PackageVNameOptions: golang.PackageVNameOptions{
			DefaultCorpus:             *corpus,
			Rules:                     rules,
			CanonicalizePackageCorpus: *canonicalizePackageCorpus,
			RootDirectory:             os.Getenv("KYTHE_ROOT_DIRECTORY"),
			UseDefaultCorpusForStdLib: *useDefaultCorpusForStdLib,
			UseDefaultCorpusForDeps:   *useDefaultCorpusForDeps,
		},
	}
	if *extraFiles != "" {
		ext.ExtraFiles = strings.Split(*extraFiles, ",")
		for i, path := range ext.ExtraFiles {
			var err error
			ext.ExtraFiles[i], err = filepath.Abs(path)
			if err != nil {
				log.Fatalf("Error finding absolute path of %s: %v", path, err)
			}
		}
	}

	locate := ext.Locate
	if *byDir {
		locate = func(path string) ([]*golang.Package, error) {
			pkg, err := ext.ImportDir(path)
			if err != nil {
				return nil, err
			}
			return []*golang.Package{pkg}, nil
		}
	}
	for _, path := range flag.Args() {
		pkgs, err := locate(path)
		if err != nil {
			maybeFatal("Error locating %q: %v", path, err)
		}
		for _, pkg := range pkgs {
			maybeLog("Found %q in %s", pkg.Path, pkg.BuildPackage.Dir)
		}
	}

	if err := ext.Extract(); err != nil {
		maybeFatal("Error in extraction: %v", err)
	}

	maybeLog("Writing %d package(s) to %q", len(ext.Packages), *outputPath)
	w, err := kzipWriter(ctx, *outputPath)
	if err != nil {
		maybeFatal("Error creating kzip writer: %v", err)
	}
	for _, pkg := range ext.Packages {
		maybeLog("Package %q:\n\t// %s", pkg.Path, pkg.BuildPackage.Doc)
		if err := pkg.EachUnit(ctx, func(cu *apb.CompilationUnit, fetcher analysis.Fetcher) error {
			if _, err := w.AddUnit(cu, nil); err != nil {
				return err
			}
			for _, ri := range cu.RequiredInput {
				fd, err := fetcher.Fetch(ri.Info.Path, ri.Info.Digest)
				if err != nil {
					return err
				}
				if _, err := w.AddFile(bytes.NewReader(fd)); err != nil {
					return err
				}
			}
			return nil
		}); err != nil {
			maybeFatal("Error writing %q: %v", pkg.Path, err)
		}
	}
	if err := w.Close(); err != nil {
		maybeFatal("Error closing output: %v", err)
	}
}

func kzipWriter(ctx context.Context, path string) (*kzip.Writer, error) {
	if err := vfs.MkdirAll(ctx, filepath.Dir(path), 0755); err != nil {
		log.Fatalf("Unable to create output directory: %v", err)
	}
	f, err := vfs.Create(ctx, path)
	if err != nil {
		log.Fatalf("Unable to create output file: %v", err)
	}
	return kzip.NewWriteCloser(f)
}
