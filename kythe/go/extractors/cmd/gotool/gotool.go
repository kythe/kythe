/*
 * Copyright 2015 Google Inc. All rights reserved.
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
// into an index pack directory.
package main

import (
	"flag"
	"fmt"
	"go/build"
	"io/ioutil"
	"log"
	"os"
	"path/filepath"
	"strings"

	"kythe.io/kythe/go/extractors/golang"
	"kythe.io/kythe/go/platform/indexpack"

	"golang.org/x/net/context"

	apb "kythe.io/kythe/proto/analysis_proto"
)

var (
	bc = build.Default // A shallow copy of the default build settings

	campfireMode = flag.Bool("campfire", false, "Enable support for the campfire build system")
	corpus       = flag.String("corpus", "", "Default corpus name to use")
	localPath    = flag.String("local_path", "", "Directory where relative imports are resolved")
	outputDir    = flag.String("output_dir", "", "Directory where output should be written")
	byDir        = flag.Bool("bydir", false, "Import by directory rather than import path")
	keepGoing    = flag.Bool("continue", false, "Continue past errors")
	verbose      = flag.Bool("v", false, "Enable verbose logging")
)

func init() {
	flag.Usage = func() {
		fmt.Fprintf(os.Stderr, "Usage: %s [options] <import-path>...\n", filepath.Base(os.Args[0]))
		fmt.Fprintln(os.Stderr, `
Extract Kythe compilation records from Go import paths specified on the command line.
Outputs are written to an index pack directory.

If the -campfire flag is set, the extractor assumes the working directory is
the root of the Kythe repository, and sets up extra paths to handle that.

Options:`)
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

	// TODO(fromberger): Attach flags to the build and release tags (maybe).
}

func maybeFatal(msg string, args ...interface{}) {
	log.Printf(msg, args...)
	if !*keepGoing {
		os.Exit(1)
	}
}

func maybeLog(msg string, args ...interface{}) {
	if *verbose {
		log.Printf(msg, args...)
	}
}

// setupCampfire modifies the build configurations to support the Kythe
// campfire build environment.  It adds entries to GOPATH for generated files
// in campfire-out and third_party packages, and the working directory so that
// "kythe/..." import paths will work.
//
// On success, returns a function to clean up leftover state, which the caller
// should defer so it occurs before exit.
func setupCampfire(ext *golang.Extractor) (func(), error) {
	if !*campfireMode {
		return func() {}, nil
	}

	maybeLog("Enabling campfire compatibility mode")
	tp, err := ioutil.ReadDir("third_party/go")
	if err != nil {
		return nil, err
	}
	goPath := os.ExpandEnv(":$PWD:campfire-out/gen/go")
	for _, elt := range tp {
		if elt.IsDir() {
			goPath += filepath.Join(":third_party/go", elt.Name())
		}
	}
	ext.BuildContext.GOPATH += goPath
	ext.AltInstallPath = "campfire-out/go"
	if err := os.Symlink(".", "src"); err != nil {
		if os.IsExist(err) {
			return func() {}, nil
		}
		return nil, err
	}
	return func() { os.Remove("src") }, nil
}

func main() {
	flag.Parse()

	if *outputDir == "" {
		log.Fatal("You must provide a non-empty --output_dir")
	}

	ext := golang.Extractor{
		BuildContext: bc,
		Corpus:       *corpus,
		LocalPath:    *localPath,
	}
	if cleanup, err := setupCampfire(&ext); err != nil {
		log.Fatalf("Error enabling campfire support: %v", err)
	} else {
		defer cleanup()
	}
	locate := ext.Locate
	if *byDir {
		locate = ext.ImportDir
	}
	for _, path := range flag.Args() {
		pkg, err := locate(path)
		if err == nil {
			maybeLog("Found %q in %s", pkg.Path, pkg.BuildPackage.Dir)
		} else {
			maybeFatal("Error locating %q: %v", path, err)
		}
	}

	if err := ext.Extract(); err != nil {
		maybeFatal("Error in extraction: %v", err)
	}

	pack, err := indexpack.CreateOrOpen(context.Background(), *outputDir,
		indexpack.UnitType((*apb.CompilationUnit)(nil)))
	if err != nil {
		log.Fatalf("Unable to open %q: %v", *outputDir, err)
	}

	maybeLog("Writing %d package(s) to %q", len(ext.Packages), pack.Root())
	for _, pkg := range ext.Packages {
		maybeLog("Package %q:\n\t// %s", pkg.Path, pkg.BuildPackage.Doc)
		uf, err := pkg.Store(context.Background(), pack)
		if err != nil {
			maybeFatal("Error writing %q: %v", pkg.Path, err)
		} else {
			maybeLog("Output: %v\n", strings.Join(uf, ", "))
		}
	}
}
