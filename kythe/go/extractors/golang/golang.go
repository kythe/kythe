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

// Package golang produces Kythe compilation units for each Go import path
// specified.  Compilations are extracted incrementally, so that partial
// results are available to the caller.
//
// Usage:
//   var c golang.Extractor
//   if _, err := c.Locate("fmt"); err != nil {
//     log.Fatalf(`Unable to locate package "fmt": %v`, err)
//   }
//   c.Extract()
//   for _, pkg := range c.Packages {
//     if pkg.Err != nil {
//       log.Printf("Error extracting %q: %v", pkg.Path, pkg.Err)
//     } else {
//       writeOutput(pkg)
//     }
//   }
//
package golang

import (
	"fmt"
	"go/build"
	"os"
	"path/filepath"
	"strings"

	"kythe.io/kythe/go/extractors/govname"
	"kythe.io/kythe/go/platform/indexpack"
	"kythe.io/kythe/go/platform/vfs"

	"golang.org/x/net/context"

	apb "kythe.io/kythe/proto/analysis_proto"
	spb "kythe.io/kythe/proto/storage_proto"
)

var workingDir string

func init() {
	if wd, err := os.Getwd(); err == nil {
		workingDir = wd
	}
}

// An Extractor contains the state needed to extract Go compilations from build
// information.  The zero value is ready for use with default settings.
type Extractor struct {
	// The build configuration to use for extraction.
	BuildContext build.Context

	// The packages that have been extracted so far (initially empty).
	Packages []*Package

	// The name of the corpus that should be attributed to packages whose
	// corpus is not specified and cannot be inferred (e.g., local imports).
	Corpus string

	// The local path against which relative imports should be resolved.
	LocalPath string

	// An alternative installation path for compiled packages.  If this is set,
	// and a compiled package cannot be found in the normal location, the
	// extractor will try in this location.
	AltInstallPath string

	// A function to generate a vname from a package's import path.  If nil,
	// the extractor will use govname.ForPackage.
	PackageVName func(corpus string, bp *build.Package) *spb.VName

	// A function to convert a directory path to an import path.  If nil, the
	// path is made relative to the first matching element of the build
	// context's GOROOT or GOPATH or the current working directory.
	DirToImport func(path string) (string, error)

	pmap map[string]*build.Package // Map of import path to build package
	fmap map[string]string         // Map of file path to content digest
}

// addPackage imports the specified package, if it has not already been
// imported, and returns its package value.
func (e *Extractor) addPackage(importPath string) (*build.Package, error) {
	if bp := e.pmap[importPath]; bp != nil {
		return bp, nil
	}
	bp, err := e.BuildContext.Import(importPath, e.LocalPath, build.AllowBinary)
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

// fetchAndStore reads the contents of path and stores them into a, returning
// the digest of the contents.  The path to digest mapping is cached so that
// repeated uses of the same file will avoid redundant work.
func (e *Extractor) fetchAndStore(ctx context.Context, path string, a *indexpack.Archive) (string, error) {
	if digest, ok := e.fmap[path]; ok {
		return digest, nil
	}
	data, err := vfs.ReadFile(ctx, path)
	if err != nil {
		// If there's an alternative installation path, and this is a path that
		// could potentially be there, try that.
		if i := strings.Index(path, "/pkg/"); i >= 0 && e.AltInstallPath != "" {
			alt := e.AltInstallPath + path[i:]
			data, err = vfs.ReadFile(ctx, alt)
			// fall through to the recheck below
		}
	}
	if err != nil {
		return "", err
	}
	name, err := a.WriteFile(ctx, data)
	if err != nil {
		return "", err
	}
	digest := strings.TrimSuffix(name, filepath.Ext(name))
	if e.fmap == nil {
		e.fmap = map[string]string{path: digest}
	}
	return digest, err
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

// vnameFor returns a vname for the specified package, handling the default.
func (e *Extractor) vnameFor(bp *build.Package) *spb.VName {
	if e.PackageVName != nil {
		return e.PackageVName(e.Corpus, bp)
	}
	return govname.ForPackage(e.Corpus, bp)
}

// dirToImport converts a directory name to an import path, if possible.
func (e *Extractor) dirToImport(dir string) (string, error) {
	if conv := e.DirToImport; conv != nil {
		return conv(dir)
	}
	if !filepath.IsAbs(dir) {
		return dir, nil
	}
	for _, path := range e.BuildContext.SrcDirs() {
		if rel, err := filepath.Rel(path, dir); err == nil {
			return rel, nil
		}
	}
	return filepath.Rel(workingDir, dir)
}

// Locate attempts to locate the specified import path in the build context.
// If the package has already been located, its existing package is returned.
// Otherwise, if importing succeeds, a new *Package value is returned, and also
// appended to the Packages field.
func (e *Extractor) Locate(importPath string) (*Package, error) {
	if pkg := e.findPackage(importPath); pkg != nil {
		return pkg, nil
	}
	bp, err := e.addPackage(importPath)
	if err != nil {
		return nil, err
	}
	pkg := &Package{
		ext:          e,
		Path:         importPath,
		BuildPackage: bp,
	}
	e.Packages = append(e.Packages, pkg)
	return pkg, nil
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
		pkg.Err = pkg.Extract()
		if pkg.Err != nil && err == nil {
			err = pkg.Err
		}
	}
	return err
}

// Package represents a single Go package extracted from local files.
type Package struct {
	ext *Extractor // pointer back to the extractor that generated this package

	Path         string                 // Import or directory path
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
	cu := &apb.CompilationUnit{
		VName:    p.VName,
		Argument: []string{"go", "build"},
	}
	bc := p.ext.BuildContext
	p.addEnv(cu, "GOPATH", bc.GOPATH)
	p.addEnv(cu, "GOOS", bc.GOOS)
	p.addEnv(cu, "GOARCH", bc.GOARCH)

	// Add required inputs from this package (source files of various kinds).
	bp := p.BuildPackage
	srcBase := filepath.Join(bp.SrcRoot, bp.ImportPath)
	p.addSource(cu, bp.Root, srcBase, bp.GoFiles)
	p.addFiles(cu, bp.Root, srcBase, bp.CgoFiles)
	p.addFiles(cu, bp.Root, srcBase, bp.CFiles)
	p.addFiles(cu, bp.Root, srcBase, bp.CXXFiles)
	p.addFiles(cu, bp.Root, srcBase, bp.HFiles)
	p.addSource(cu, bp.Root, srcBase, bp.TestGoFiles)

	// TODO(fromberger): Treat tests that are not in the same package as a
	// separate compilation, e.g.,
	// p.addSource(cu, bp.Root, srcBase, bp.XTestGoFiles)
	// missing = append(missing, p.addDeps(cu, bp.XTestImports)...)

	// Add the outputs of all the dependencies as required inputs.
	//
	// TODO(fromberger): Consider making a transitive option, to flatten out
	// the source requirements for tools like the oracle.
	missing := p.addDeps(cu, bp.Imports)
	missing = append(missing, p.addDeps(cu, bp.TestImports)...)

	// Add command-line arguments.
	// TODO(fromberger): Figure out what to do with cgo compiler flags.
	// Also, whether we should emit separate compilations for cgo actions.
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

// Store writes the compilation units of p to the specified archive and returns
// its unit file names.  This has the side-effect of updating the required
// inputs of the compilations so that they contain the proper digest values.
func (p *Package) Store(ctx context.Context, a *indexpack.Archive) ([]string, error) {
	const formatKey = "kythe"

	var unitFiles []string
	for _, cu := range p.Units {
		// Pack the required inputs into the archive.
		for _, ri := range cu.RequiredInput {
			// Check whether we already did this, so Store can be idempotent.
			//
			// When addFiles first adds the required input to the record, we
			// know its path but have not yet fetched its contents -- that step
			// is deferred until we are ready to store them for output (i.e.,
			// now).  Once we have fetched the file contents, we'll update the
			// field with the correct digest value.  We only want to do this
			// once, per input, however.
			path := ri.Info.Digest
			if !strings.Contains(path, "/") {
				continue
			}

			// Fetch the file and store it into the archive.  We may get a
			// cache hit here, handled by fetchAndStore.
			digest, err := p.ext.fetchAndStore(ctx, path, a)
			if err != nil {
				return nil, err
			}
			ri.Info.Digest = digest
		}

		// Pack the compilation unit into the archive.
		fn, err := a.WriteUnit(ctx, formatKey, cu)
		if err != nil {
			return nil, err
		}
		unitFiles = append(unitFiles, fn)
	}
	return unitFiles, nil
}

// addFiles adds a required input to cu for each file whose basename or path is
// given in names.  If base != "", it is prejoined to each name.
// The path of the input will have root/ trimmed from the beginning.
// The digest will be the complete path as written -- this will be replaced
// with the content digest in the fetcher.
func (*Package) addFiles(cu *apb.CompilationUnit, root, base string, names []string) {
	for _, name := range names {
		path := name
		if base != "" {
			path = filepath.Join(base, name)
		}
		cu.RequiredInput = append(cu.RequiredInput, &apb.CompilationUnit_FileInput{
			Info: &apb.FileInfo{
				Path:   strings.TrimPrefix(path, root+"/"),
				Digest: path,
			},
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
	p.addFiles(cu, bp.Root, "", []string{bp.PkgObj})

	// Populate the vname for the input based on the corpus of the package.
	fi := cu.RequiredInput[len(cu.RequiredInput)-1]
	fi.VName = p.ext.vnameFor(bp)
}

// addEnv adds an environment variable to cu.
func (*Package) addEnv(cu *apb.CompilationUnit, name, value string) {
	cu.Environment = append(cu.Environment, &apb.CompilationUnit_Env{
		Name:  name,
		Value: value,
	})
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
func (p *Package) addDeps(cu *apb.CompilationUnit, importPaths []string) []string {
	var missing []string
	for _, ip := range importPaths {
		dep, err := p.ext.addPackage(ip)
		if err != nil {
			missing = append(missing, ip)
		} else if ip != "unsafe" { // package unsafe is intrinsic
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
