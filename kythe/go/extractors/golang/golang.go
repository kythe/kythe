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

// Package golang produces Kythe compilation units for each Go import path
// specified.  Compilations are extracted incrementally, so that partial
// results are available to the caller.
//
// Usage:
//
//	var c golang.Extractor
//	if _, err := c.Locate("fmt"); err != nil {
//	  log.Fatalf(`Unable to locate package "fmt": %v`, err)
//	}
//	c.Extract()
//	for _, pkg := range c.Packages {
//	  if pkg.Err != nil {
//	    log.Errorf("extracting %q: %v", pkg.Path, pkg.Err)
//	  } else {
//	    writeOutput(pkg)
//	  }
//	}
package golang // import "kythe.io/kythe/go/extractors/golang"

import (
	"context"
	"fmt"
	"go/build"
	"os"
	"path/filepath"
	"strings"

	"kythe.io/kythe/go/extractors/govname"
	"kythe.io/kythe/go/platform/analysis"
	"kythe.io/kythe/go/platform/kzip"
	"kythe.io/kythe/go/platform/vfs"
	"kythe.io/kythe/go/util/log"
	"kythe.io/kythe/go/util/ptypes"

	"bitbucket.org/creachadair/stringset"

	anypb "github.com/golang/protobuf/ptypes/any"

	apb "kythe.io/kythe/proto/analysis_go_proto"
	gopb "kythe.io/kythe/proto/go_go_proto"
	spb "kythe.io/kythe/proto/storage_go_proto"
)

var workingDir string

func init() {
	if wd, err := os.Getwd(); err == nil {
		workingDir = wd
	}
}

// PackageVNameOptions re-exports govname.PackageVNameOptions.
type PackageVNameOptions = govname.PackageVNameOptions

// An Extractor contains the state needed to extract Go compilations from build
// information.  The zero value is ready for use with default settings.
type Extractor struct {
	// The build configuration to use for extraction.
	BuildContext build.Context

	// The packages that have been extracted so far (initially empty).
	Packages []*Package

	// The configuration for constructing VNames for packages.
	PackageVNameOptions

	// An alternative installation path for compiled packages.  If this is set,
	// and a compiled package cannot be found in the normal location, the
	// extractor will try in this location.
	AltInstallPath string

	// Extra file paths to include in each compilation record.
	ExtraFiles []string

	// A function to convert a directory path to an import path.  If nil, the
	// path is made relative to the first matching element of the build
	// context's GOROOT or GOPATH or the current working directory.
	DirToImport func(path string) (string, error)

	pmap map[string]*build.Package // Map of import path to build package
}

// addPackage imports the specified package, if it has not already been
// imported, and returns its package value.
func (e *Extractor) addPackage(importPath, localPath string) (*build.Package, error) {
	if bp := e.pmap[importPath]; bp != nil {
		return bp, nil
	}
	bp, err := e.BuildContext.Import(importPath, localPath, build.AllowBinary)
	if err != nil {
		return nil, err
	}
	e.mapPackage(importPath, bp)
	return bp, nil
}

func (e *Extractor) mapPackage(importPath string, bp *build.Package) {
	if e.pmap == nil {
		e.pmap = map[string]*build.Package{importPath: bp}
	} else {
		e.pmap[importPath] = bp
	}
}

// findPackages returns the first *Package value in Packages having the given
// import path, or nil if none is found.
func (e *Extractor) findPackage(importPath string) *Package {
	for _, pkg := range e.Packages {
		if pkg.Path == importPath {
			return pkg
		}
	}
	return nil
}

// vnameFor returns a vname for the specified package.
func (e *Extractor) vnameFor(bp *build.Package) *spb.VName {
	v := govname.ForPackage(bp, &e.PackageVNameOptions)
	v.Signature = "" // not useful in this context
	return v
}

// dirToImport converts a directory name to an import path, if possible.
func (e *Extractor) dirToImport(dir string) (string, error) {
	if conv := e.DirToImport; conv != nil {
		return conv(dir)
	}
	for _, path := range e.BuildContext.SrcDirs() {
		if rel, err := filepath.Rel(path, dir); err == nil {
			return rel, nil
		}
	}
	if rel, err := filepath.Rel(workingDir, dir); err == nil {
		return rel, nil
	}
	return dir, nil
}

// Locate attempts to resolve and locate the specified import path in the build
// context.  If a package has already been located, its existing *Package is
// returned.  Otherwise, a new *Package value is returned and appended to the
// Packages field.
//
// Note: multiple packages may be resolved for "/..." import paths
func (e *Extractor) Locate(importPath string) ([]*Package, error) {
	listedPackages, listErr := e.listPackages(importPath)

	var pkgs []*Package
	for _, pkg := range listedPackages {
		if pkg.ForTest != "" || strings.HasSuffix(pkg.ImportPath, ".test") {
			// ignore constructed test packages
			continue
		} else if pkg.Error != nil {
			return nil, pkg.Error
		}

		importPath := pkg.ImportPath
		p := e.findPackage(importPath)
		if p == nil {
			p = &Package{
				ext:          e,
				Path:         importPath,
				DepOnly:      pkg.DepOnly,
				BuildPackage: pkg.buildPackage(),
			}
			e.Packages = append(e.Packages, p)
			e.mapPackage(importPath, p.BuildPackage)
		}
		if !pkg.DepOnly {
			pkgs = append(pkgs, p)
		}
	}
	return pkgs, listErr
}

// ImportDir attempts to import the Go package located in the given directory.
// An import path is inferred from the directory path.
func (e *Extractor) ImportDir(dir string) (*Package, error) {
	clean := filepath.Clean(dir)
	importPath, err := e.dirToImport(clean)
	if err != nil {
		return nil, err
	}
	if pkg := e.findPackage(importPath); pkg != nil {
		return pkg, nil
	}
	bp, err := e.BuildContext.ImportDir(clean, build.AllowBinary)
	if err != nil {
		return nil, err
	}
	bp.ImportPath = importPath
	e.mapPackage(importPath, bp)
	pkg := &Package{
		ext:          e,
		Path:         importPath,
		BuildPackage: bp,
	}
	e.Packages = append(e.Packages, pkg)
	return pkg, nil
}

// Extract invokes the Extract method of each package in the Packages list, and
// updates its Err field with the result.  If there were errors in extraction,
// one of them is returned.
func (e *Extractor) Extract() error {
	var err error
	for _, pkg := range e.Packages {
		if pkg.DepOnly {
			continue
		}
		pkg.Err = pkg.Extract()
		if pkg.Err != nil && err == nil {
			err = pkg.Err
		}
	}
	return err
}

// Package represents a single Go package extracted from local files.
type Package struct {
	ext  *Extractor    // pointer back to the extractor that generated this package
	seen stringset.Set // input files already added to this package

	CorpusRoot   string                 // Corpus package root path
	Path         string                 // Import or directory path
	DepOnly      bool                   // Whether the package is only seen as a dependency
	Err          error                  // Error discovered during processing
	BuildPackage *build.Package         // Package info from the go/build library
	VName        *spb.VName             // The package's Kythe vname
	Units        []*apb.CompilationUnit // Compilations generated from Package
}

// Extract populates the Units field of p, and reports an error if any occurred.
//
// After this method returns successfully, the require inputs for each of the
// Units are partially resolved, meaning we know their filesystem paths but not
// their contents.  The filesystem paths are resolved to contents and digests
// by the Store method.
func (p *Package) Extract() error {
	p.VName = p.ext.vnameFor(p.BuildPackage)
	if r, err := govname.RepoRoot(p.Path); err == nil {
		p.CorpusRoot = r.Root
	} else {
		p.CorpusRoot = p.VName.GetCorpus()
	}
	cu := &apb.CompilationUnit{
		VName:    p.VName,
		Argument: []string{"go", "build"},
	}
	bc := p.ext.BuildContext
	if info, err := ptypes.MarshalAny(&gopb.GoDetails{
		Gopath:     bc.GOPATH,
		Goos:       bc.GOOS,
		Goarch:     bc.GOARCH,
		Compiler:   bc.Compiler,
		BuildTags:  bc.BuildTags,
		CgoEnabled: bc.CgoEnabled,
	}); err == nil {
		cu.Details = append(cu.Details, info)
	}

	if govname.ImportPath(cu.VName, bc.GOROOT) != p.Path {
		// Add GoPackageInfo if constructed VName differs from actual ImportPath.
		if info, err := ptypes.MarshalAny(&gopb.GoPackageInfo{
			ImportPath: p.Path,
		}); err == nil {
			cu.Details = append(cu.Details, info)
		} else {
			log.Warningf("failed to marshal GoPackageInfo for CompilationUnit: %v", err)
		}
	}

	// Add required inputs from this package (source files of various kinds).
	bp := p.BuildPackage
	srcBase := bp.Dir
	p.addSource(cu, bp.Root, srcBase, bp.GoFiles)
	p.addFiles(cu, bp.Root, srcBase, bp.CgoFiles)
	p.addFiles(cu, bp.Root, srcBase, bp.CFiles)
	p.addFiles(cu, bp.Root, srcBase, bp.CXXFiles)
	p.addFiles(cu, bp.Root, srcBase, bp.HFiles)
	p.addSource(cu, bp.Root, srcBase, bp.TestGoFiles)

	// Add extra inputs that may be specified by the extractor.
	p.addFiles(cu, filepath.Dir(bp.SrcRoot), "", p.ext.ExtraFiles)

	// TODO(fromberger): Treat tests that are not in the same package as a
	// separate compilation, e.g.,
	// p.addSource(cu, bp.Root, srcBase, bp.XTestGoFiles)
	// missing = append(missing, p.addDeps(cu, bp.XTestImports, bp.Dir)...)

	// Add the outputs of all the dependencies as required inputs.
	//
	// TODO(fromberger): Consider making a transitive option, to flatten out
	// the source requirements for tools like the oracle.
	missing := p.addDeps(cu, bp.Imports, bp.Dir)
	missing = append(missing, p.addDeps(cu, bp.TestImports, bp.Dir)...)

	// Add command-line arguments.
	// TODO(fromberger): Figure out whether we should emit separate
	// compilations for cgo actions.
	p.addFlag(cu, "-compiler", bc.Compiler)
	if t := bp.AllTags; len(t) > 0 {
		p.addFlag(cu, "-tags", strings.Join(t, " "))
	}
	cu.Argument = append(cu.Argument, bp.ImportPath)

	p.Units = append(p.Units, cu)
	if len(missing) != 0 {
		cu.HasCompileErrors = true
		return &MissingError{p.Path, missing}
	}
	return nil
}

// mapFetcher implements analysis.Fetcher by dispatching to a preloaded map
// from digests to contents.
type mapFetcher map[string][]byte

// Fetch implements the analysis.Fetcher interface. The path argument is ignored.
func (m mapFetcher) Fetch(_, digest string) ([]byte, error) {
	if data, ok := m[digest]; ok {
		return data, nil
	}
	return nil, os.ErrNotExist
}

// EachUnit calls f with a compilation record for each unit in p.  If f reports
// an error, that error is returned by EachUnit.
func (p *Package) EachUnit(ctx context.Context, f func(cu *apb.CompilationUnit, fetcher analysis.Fetcher) error) error {
	fetcher := make(mapFetcher)
	for _, cu := range p.Units {
		// Ensure all the file contents are loaded, and update the digests.
		for _, ri := range cu.RequiredInput {
			if !strings.Contains(ri.Info.Digest, "/") {
				continue // skip those that are already complete
			}
			rc, err := vfs.Open(ctx, ri.Info.Digest)
			if err != nil {
				return fmt.Errorf("opening input: %v", err)
			}
			fd, err := kzip.FileData(ri.Info.Path, rc)
			rc.Close()
			if err != nil {
				return fmt.Errorf("reading input: %v", err)
			}
			fetcher[fd.Info.Digest] = fd.Content
			ri.Info.Digest = fd.Info.Digest
		}

		if err := f(cu, fetcher); err != nil {
			return err
		}
	}
	return nil
}

// addFiles adds a required input to cu for each file whose basename or path is
// given in names.  If base != "", it is prejoined to each name.
// The path of the input will have root/ trimmed from the beginning.
// The digest will be the complete path as written -- this will be replaced
// with the content digest in the fetcher.
func (p *Package) addFiles(cu *apb.CompilationUnit, root, base string, names []string) {
	// If a root directory is specified, use it instead of the the root from the
	// go package loader.
	if p.ext.RootDirectory != "" {
		root = p.ext.RootDirectory
	}
	if !strings.HasSuffix(root, "/") {
		root += "/"
	}

	for _, name := range names {
		path := name
		if base != "" {
			path = filepath.Join(base, name)
		}
		trimmed := strings.TrimPrefix(path, root)
		vn := &spb.VName{
			Corpus: p.ext.DefaultCorpus,
			Path:   trimmed,
		}

		var details []*anypb.Any
		if p.ext.Rules != nil {
			v2, ok := p.ext.Rules.Apply(trimmed)
			if ok {
				vn.Corpus = v2.Corpus
				vn.Root = v2.Root
				vn.Path = v2.Path

				if govname.ImportPath(vn, p.ext.BuildContext.GOROOT) != p.BuildPackage.ImportPath {
					// Add GoPackageInfo if constructed VName differs from actual ImportPath.
					if info, err := ptypes.MarshalAny(&gopb.GoPackageInfo{
						ImportPath: p.BuildPackage.ImportPath,
					}); err == nil {
						details = append(details, info)
					} else {
						log.Warningf("failed to marshal GoPackageInfo for input: %v", err)
					}
				}
			}
		}

		if vn.Corpus == "" {
			// If no default corpus is specified, use the package's corpus for each of
			// its files.  The package corpus is based on the rules in
			// kythe/go/extractors/govname and is usually the package's
			// repository root (e.g. github.com/golang/protobuf).
			vn.Corpus = p.VName.Corpus
			if components := strings.SplitN(vn.Path, string(filepath.Separator), 2); len(components) == 2 {
				vn.Path = strings.TrimPrefix(components[1], p.CorpusRoot+"/")
				if components[0] != "src" {
					vn.Root = components[0]
				}
			}
		}
		cu.RequiredInput = append(cu.RequiredInput, &apb.CompilationUnit_FileInput{
			VName: vn,
			Info: &apb.FileInfo{
				Path:   trimmed,
				Digest: path, // provisional, until the file is loaded
			},
			Details: details,
		})
	}
}

// addSource acts as addFiles, and in addition marks each trimmed path as a
// source input for the compilation.
func (p *Package) addSource(cu *apb.CompilationUnit, root, base string, names []string) {
	p.addFiles(cu, root, base, names)
	for _, in := range cu.RequiredInput[len(cu.RequiredInput)-len(names):] {
		cu.SourceFile = append(cu.SourceFile, in.Info.Path)
	}
}

// addInput acts as addFiles for the output of a package.
func (p *Package) addInput(cu *apb.CompilationUnit, bp *build.Package) {
	obj := bp.PkgObj
	if !p.seen.Contains(obj) {
		p.seen.Add(obj)
		p.addFiles(cu, bp.Root, "", []string{obj})

		// Populate the vname for the input based on the corpus of the package.
		fi := cu.RequiredInput[len(cu.RequiredInput)-1]
		fi.VName = p.ext.vnameFor(bp)
		// Because the VName has changed, a previously-added details message (if
		// any) is no longer valid.
		fi.Details = nil

		if govname.ImportPath(fi.VName, p.ext.BuildContext.GOROOT) != bp.ImportPath {
			// Add GoPackageInfo if constructed VName differs from actual ImportPath.
			if info, err := ptypes.MarshalAny(&gopb.GoPackageInfo{
				ImportPath: bp.ImportPath,
			}); err == nil {
				fi.Details = append(fi.Details, info)
			} else {
				log.Warningf("failed to marshal GoPackageInfo for input: %v", err)
			}
		}
	}
}

// addFlag adds a flag and its arguments to the command line, if len(values) != 0.
func (*Package) addFlag(cu *apb.CompilationUnit, name string, values ...string) {
	if len(values) != 0 {
		cu.Argument = append(cu.Argument, name)
		cu.Argument = append(cu.Argument, values...)
	}
}

// addDeps adds required inputs for the import paths given, returning the paths
// of any packages that could not be imported successfully.
func (p *Package) addDeps(cu *apb.CompilationUnit, importPaths []string, localPath string) []string {
	var missing []string
	for _, ip := range importPaths {
		if ip == "unsafe" {
			// package unsafe is intrinsic; nothing to do
		} else if dep, err := p.ext.addPackage(ip, localPath); err != nil || dep.PkgObj == "" {
			// Package was either literally missing or could not be built properly.
			// Note: Locate could have added a dependency package that could not be
			// built as part of its earlier analysis.
			missing = append(missing, ip)
		} else {
			p.addInput(cu, dep)
		}
	}
	return missing
}

// MissingError is the concrete type of errors about missing dependencies.
type MissingError struct {
	Path    string   // The import path of the incomplete package
	Missing []string // The import paths of the missing dependencies
}

func (m *MissingError) Error() string {
	return fmt.Sprintf("package %q is missing %d imports (%s)",
		m.Path, len(m.Missing), strings.Join(m.Missing, ", "))
}
