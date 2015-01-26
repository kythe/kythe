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

// Program gopackages constructs compilation index files for Go source packages
// stored in a directory tree that follows the go/build conventions for package
// organization (root/pkg for compiled objects, root/src/pkg for source).
//
// Usage:
//   gopackages --output_dir gopack [--goroot /go/root/folder]
//
// One index file is written per package.  Dependencies on other packages under
// the same root are resolved by import path where possible.
package main

import (
	"bufio"
	"flag"
	"fmt"
	"go/build"
	"io"
	"io/ioutil"
	"log"
	"os"
	"path/filepath"
	"runtime"
	"strings"
	"sync"
	"time"

	"kythe/go/platform/indexinfo"
	"kythe/go/platform/local"
)

var (
	corpus = flag.String("corpus", "kythe", "Corpus of resulting CompilationUnits")
	goRoot = flag.String("goroot", build.Default.GOROOT,
		"Path to Go root directory(required)")
	packageForIndex = flag.String("package", "", "Output an index file only for specified package")
	outputDir       = flag.String("output_dir", "",
		"Directory where index files should be written (required)")
)

const languageName = "go"

func init() {
	flag.Usage = func() {
		fmt.Fprintln(os.Stderr, `Usage: gopackages [options]

Constructs a compilation index file for each Go source package found in the
specified root directory.  Each package must follow the standard Go directory
structure.

One index file is written per package, and dependencies between packages under
the given root are resolved by import path.

The tool needs write access to --output_dir.

By default, the goroot has the value of the default Go root directory.

Options:`)

		flag.PrintDefaults()
	}
}

func main() {
	flag.Parse()

	if *goRoot == "" {
		log.Fatal("You must provide a non-empty --goroot")
	}
	if *outputDir == "" {
		log.Fatal("You must provide a non-empty --output_dir")
	}

	if err := os.MkdirAll(*outputDir, 0755); err != nil {
		log.Fatalf("Unable to create %q: %v", *outputDir, err)
	} else {
		cleanOutput()
	}

	bc := build.Context{
		GOROOT:     *goRoot,
		GOOS:       runtime.GOOS,
		GOARCH:     runtime.GOARCH,
		GOPATH:     build.Default.GOPATH,
		CgoEnabled: true,
		Compiler:   "gc",
	}

	// Walk through the source directories and load any source packages found.
	pkgs := make(chan *build.Package)
	var wg sync.WaitGroup

	for _, dir := range bc.SrcDirs() {

		err := filepath.Walk(dir, func(path string, info os.FileInfo, err error) error {
			// Check if there was an error accessing the current path.
			if err != nil {
				log.Printf("Error accessing %q: %v\n", path, err)
				return filepath.SkipDir
			}

			// Filter out the non dir entries.
			if !info.IsDir() {
				return nil
			}

			log.Printf("IN %q\n", path)
			wg.Add(1)
			go func(dir string) {
				defer wg.Done()
				ipath := strings.TrimPrefix(path, dir+string(os.PathSeparator))
				pkg, e := bc.Import(ipath, dir, 0)
				if e != nil {
					log.Printf("Skipping %q: %v\n", path, e)
					return
				}
				pkgs <- pkg
			}(dir)
			return nil
		})

		if err != nil {
			log.Fatalf("Scanning %q failed: %v\n", dir, err)
		}
	}
	go func() {
		wg.Wait()
		close(pkgs)
	}()

	// Map from import paths to packages, so that we can use them to resolve
	// the dependencies among them.  Do this first so order won't matter.
	start := time.Now()
	imports := make(map[string]*build.Package)
	for pkg := range pkgs {
		if imports[pkg.ImportPath] != nil {
			panic("duplicate import path")
		}
		imports[pkg.ImportPath] = pkg
	}
	fmt.Fprintf(os.Stderr, "Found %d packages under %s (%v elapsed)\n",
		len(imports), bc.GOROOT, snapTime(start, time.Millisecond))

	// Assemble index files for each of the packages found.

	type result struct {
		path, signature string
		elapsed         time.Duration
		err             error
	}

	results := make(chan result)
	m := &maker{*goRoot, imports, &bc}

	assembleIndex := func(pkg *build.Package, wg *sync.WaitGroup, results chan<- result) {
		defer wg.Done()
		start := time.Now()
		idx, err := m.Make(pkg)
		if err != nil {
			results <- result{err: err, path: pkg.ImportPath}
		} else if err := writeToFile(idx, outputPath(pkg.ImportPath)); err != nil {
			results <- result{err: err, path: pkg.ImportPath}
		} else {
			results <- result{
				path:      pkg.ImportPath,
				signature: idx.Compilation.GetVName().GetSignature(),
				elapsed:   snapTime(start, time.Millisecond),
			}
		}
	}

	if *packageForIndex == "" {
		wg.Add(len(imports))
		for _, pkg := range imports {
			go assembleIndex(pkg, &wg, results)
		}
	} else {
		if pkg, ok := imports[*packageForIndex]; ok {
			wg.Add(1)
			go assembleIndex(pkg, &wg, results)
		} else {
			log.Printf("Package %q not found. Will not write any index.", *packageForIndex)
		}
	}

	go func() {
		wg.Wait()
		close(results)
	}()

	start = time.Now()
	var nfiles int
	for r := range results {
		if r.err != nil {
			log.Printf("Unable to construct index for %q: %v\n", r.path, r.err)
		} else {
			fmt.Printf("%-25s %-40s %v\n", r.path, r.signature, r.elapsed)
			nfiles++
		}
	}
	fmt.Fprintf(os.Stderr, "Wrote %d index files (%v elapsed)\n",
		nfiles, snapTime(start, time.Second))
}

func snapTime(start time.Time, res time.Duration) time.Duration {
	return time.Now().Truncate(res).Sub(start.Truncate(res))
}

type maker struct {
	corpusRoot string
	imports    map[string]*build.Package
	ctx        *build.Context
}

// Make constructs a compilation index from the given package.
func (m *maker) Make(pkg *build.Package) (*indexinfo.IndexInfo, error) {
	cu := local.NewCompilation()
	addFile := func(path string) (string, error) {
		rel := strings.TrimPrefix(path, m.corpusRoot)

		bits, err := ioutil.ReadFile(path)
		if err != nil {
			return rel, err
		}
		cu.AddData(rel, bits)
		return rel, nil
	}
	cu.AddDirectory(m.corpusRoot)

	signature := signature(pkg)
	cu.SetLanguage(languageName)
	cu.SetSignature(signature)
	cu.SetCorpus(*corpus)
	cu.SetOutput(strings.TrimPrefix(pkg.PkgObj, m.corpusRoot))

	// Add Go source files.
	for _, src := range pkg.GoFiles {
		path := filepath.Join(pkg.SrcRoot, pkg.ImportPath, src)
		rel, err := addFile(path)
		if err != nil {
			return nil, fmt.Errorf("unable to load source file %q: %v\n", path, err)
		}
		cu.SetSource(rel)
	}

	// Add required inputs for imported packages.
	for _, ipath := range pkg.Imports {
		pkg := m.imports[ipath]
		if pkg == nil {
			if ipath != "C" { // This one has no code, not even documentation.
				log.Printf("Missing import of %q for %q\n", ipath, signature)
			}
			continue
		}
		if _, err := addFile(pkg.PkgObj); err != nil {
			log.Printf("Unable to load required input: %v\n", err)
		}
	}

	// If there are other required inputs, try to add them.
	for _, c := range append(pkg.CgoFiles, append(pkg.CFiles, pkg.CXXFiles...)...) {
		if _, err := addFile(filepath.Join(pkg.SrcRoot, pkg.ImportPath, c)); err != nil {
			return nil, fmt.Errorf("unable to load required input: %v\n", err)
		}
	}

	// Set up fake compiler arguments to simulate enough to allow an analyzer to work.
	cu.Proto.Argument = append([]string{
		"fake-gobuild",
		"--go-compiler", "fake-go", "-I", strings.TrimRight(m.corpusRoot, "/"),
		"-p", "dummycorpus/" + pkg.ImportPath, ";",
		"--goroot", m.ctx.GOROOT,
		"--goos", m.ctx.GOOS,
		"--goarch", m.ctx.GOARCH,
		"--package-output", cu.Proto.GetOutputKey(),
		"--tags", "gc",
	}, cu.Proto.SourceFile...)

	// TODO(fromberger): Maybe do something with test files.  Perhaps they
	// should get their own compilations.  Since these compilations cannot be
	// directly built, and dependencies are loaded from .a files, it should be
	// safe to include the test files here also.

	idx, err := indexinfo.FromCompilation(cu.Proto, cu)
	if err != nil {
		panic(err) // Can't happen, everything is already in memory.
	}
	return idx, nil
}

// writeToFile writes w to a newly-created file at path.
func writeToFile(w io.WriterTo, path string) (err error) {
	f, err := os.Create(path)
	if err != nil {
		return err
	}

	defer func() {
		if e := f.Close(); err == nil {
			err = e
		}
	}()

	buf := bufio.NewWriterSize(f, 1<<20)
	if _, err := w.WriteTo(buf); err != nil {
		return err
	}
	if err := buf.Flush(); err != nil {
		return err
	}
	return nil
}

// signature constructs a putative Signature name for p.
// No such target may actually exist in the build graph.
func signature(p *build.Package) string { return "//" + p.ImportPath }

// outputPath converts a build target name into an index file path.
func outputPath(target string) string {
	return filepath.Join(*outputDir,
		strings.Replace(strings.TrimPrefix(target, "//"), "/", "_", -1)) + ".kindex"
}

// cleanOutput removes all regular files from the output directory.
func cleanOutput() error {
	files, err := filepath.Glob(filepath.Join(*outputDir, "*"))
	if err != nil {
		return nil
	}
	var lastErr error
	for _, fs := range files {
		if fm, err := os.Lstat(fs); err == nil && !fm.IsDir() {
			if err := os.Remove(fs); err != nil {
				lastErr = err
			}
		}
	}
	return lastErr
}
