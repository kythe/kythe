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

// Package indexer implements a Kythe indexer for the Go language.
//
// Usage example: Indexing a Kythe CompilationUnit message.
//
//   // Obtain a compilation from some source, e.g., an index pack.
//   var pack *indexpack.Archive = ...
//   var unit *apb.CompilationUnit = ...
//
//   // Parse the sources and resolve types.
//   pi, err := indexer.Resolve(unit, pack, indexer.AllTypeInfo())
//   if err != nil {
//     log.Fatal("Resolving failed: %v", err)
//   }
//   // Type information from http://godoc.org/go/types is now available
//   // from pi.Info, which is a *types.Info record.
//
package indexer

import (
	"bytes"
	"errors"
	"fmt"
	"go/ast"
	"go/parser"
	"go/token"
	"go/types"
	"log"
	"path/filepath"
	"strconv"
	"strings"

	"bitbucket.org/creachadair/stringset"
	"github.com/golang/protobuf/proto"
	"golang.org/x/tools/go/gcexportdata"

	"kythe.io/kythe/go/extractors/govname"

	apb "kythe.io/kythe/proto/analysis_proto"
	spb "kythe.io/kythe/proto/storage_proto"
)

// A Fetcher retrieves the contents of a file given its path and/or hex-encoded
// SHA256 digest, at least one of which must be set.
type Fetcher interface {
	Fetch(path, digest string) ([]byte, error)
}

// PackageInfo records information about the Go packages defined by a
// compilation unit and its dependencies.
type PackageInfo struct {
	Name         string                        // The (short) name of the package
	ImportPath   string                        // The nominal import path of the package
	Package      *types.Package                // The package for this compilation
	Dependencies map[string]*types.Package     // Packages imported from dependencies
	VName        *spb.VName                    // The base vname for this package
	PackageVName map[*types.Package]*spb.VName // Resolved package to vname
	FileSet      *token.FileSet                // Location info for the source files
	Files        []*ast.File                   // The parsed ASTs of the source files
	SourceText   map[string]string             // The text of the source files, by path

	Info   *types.Info // If non-nil, contains type-checker results
	Errors []error     // All errors reported by the type checker

	// A lazily-initialized mapping from an object on the RHS of a selection
	// (lhs.RHS) to the nearest enclosing named struct or interface type; or in
	// the body of a function or method to the nearest enclosing named method.
	owner map[types.Object]types.Object

	// A lazily-initialized mapping from from AST nodes to their corresponding
	// VNames. Only nodes that do not resolve directly to a type object are
	// included in this map, e.g., function literals.
	function map[ast.Node]*funcInfo

	// A lazily-initialized set of standard library package import paths for
	// which a node has been emitted.
	standardLib stringset.Set

	// A dummy function representing the undeclared package initilization
	// function.
	packageInit *funcInfo

	// A cache of already-computed signatures.
	sigs map[types.Object]string

	// The number of package-level init declarations seen.
	numInits int
}

type funcInfo struct {
	vname    *spb.VName
	numAnons int // number of anonymous functions defined inside this one
}

// packageImporter implements the types.Importer interface by fetching files
// from the required inputs of a compilation unit.
type packageImporter struct {
	deps    map[string]*types.Package // packages already loaded
	fileSet *token.FileSet            // source location information
	fileMap map[string]*apb.FileInfo  // :: import path → required input location
	fetcher Fetcher                   // access to required input contents
}

// Import satisfies the types.Importer interface using the captured data from
// the compilation unit.
func (pi *packageImporter) Import(importPath string) (*types.Package, error) {
	if pkg := pi.deps[importPath]; pkg != nil && pkg.Complete() {
		return pkg, nil
	} else if importPath == "unsafe" {
		// The "unsafe" package is special, and isn't usually added by the
		// resolver into the dependency map.
		pi.deps[importPath] = types.Unsafe
		return types.Unsafe, nil
	}

	// Fetch the required input holding the package for this import path, and
	// load its export data for use by the type resolver.
	if fi := pi.fileMap[importPath]; fi != nil {
		data, err := pi.fetcher.Fetch(fi.Path, fi.Digest)
		if err != nil {
			return nil, fmt.Errorf("fetching %q (%s): %v", fi.Path, fi.Digest, err)
		}
		r, err := gcexportdata.NewReader(bytes.NewReader(data))
		if err != nil {
			return nil, fmt.Errorf("reading export data in %q (%s): %v", fi.Path, fi.Digest, err)
		}
		return gcexportdata.Read(r, pi.fileSet, pi.deps, importPath)
	}
	return nil, fmt.Errorf("package %q not found", importPath)
}

// Resolve resolves the package information for unit and its dependencies.  On
// success the package corresponding to unit is located via ImportPath in the
// Packages map of the returned value.
//
// If info != nil, it is used to populate the Info field of the return value
// and will contain the output of the type checker in each user-provided map
// field.
func Resolve(unit *apb.CompilationUnit, f Fetcher, info *types.Info) (*PackageInfo, error) {
	sourceFiles := stringset.New(unit.SourceFile...)

	imap := make(map[string]*spb.VName)    // import path → vname
	srcs := make(map[string]string)        // file path → text
	fmap := make(map[string]*apb.FileInfo) // import path → file info
	fset := token.NewFileSet()             // location info for the parser
	goRoot := getenv("GOROOT", unit)
	var files []*ast.File // parsed sources

	// Classify the required inputs as either sources, which are to be parsed,
	// or dependencies, which are to be "imported" via the type-checker's
	// import mechanism.  If successful, this populates fset and files with the
	// lexical and syntactic content of the package's own sources.
	for _, ri := range unit.RequiredInput {
		if ri.Info == nil {
			return nil, errors.New("required input file info missing")
		}

		// Source inputs need to be parsed, so we can give their ASTs to the
		// type checker later on.
		fpath := ri.Info.Path
		if sourceFiles.Contains(fpath) {
			data, err := f.Fetch(fpath, ri.Info.Digest)
			if err != nil {
				return nil, fmt.Errorf("fetching %q (%s): %v", fpath, ri.Info.Digest, err)
			}
			srcs[fpath] = string(data)
			parsed, err := parser.ParseFile(fset, fpath, data, parser.AllErrors|parser.ParseComments)
			if err != nil {
				return nil, fmt.Errorf("parsing %q: %v", fpath, err)
			}
			files = append(files, parsed)
			continue
		}

		// Other files may be compiled archives with type information for other
		// packages, or may be other ancillary files like C headers to support
		// cgo.  Use the vname to determine which import path for each and save
		// that mapping for use by the importer.
		if ri.VName == nil {
			return nil, fmt.Errorf("missing vname for %q", fpath)
		}

		ipath := vnameToImport(ri.VName, goRoot)
		imap[ipath] = ri.VName
		fmap[ipath] = ri.Info
	}
	if len(files) == 0 {
		return nil, errors.New("no source files in package")
	}

	pi := &PackageInfo{
		Name:         files[0].Name.Name,
		ImportPath:   vnameToImport(unit.VName, goRoot),
		FileSet:      fset,
		Files:        files,
		Info:         info,
		SourceText:   srcs,
		PackageVName: make(map[*types.Package]*spb.VName),
		Dependencies: make(map[string]*types.Package), // :: import path → package

		function: make(map[ast.Node]*funcInfo),
		sigs:     make(map[types.Object]string),
	}

	// Run the type-checker and collect any errors it generates.  Errors in the
	// type checker are not returned directly; the caller can read them from
	// the Errors field.
	c := &types.Config{
		FakeImportC:              true, // so we can handle cgo
		DisableUnusedImportCheck: true, // this is not fatal to type-checking
		Importer: &packageImporter{
			deps:    pi.Dependencies,
			fileSet: pi.FileSet,
			fileMap: fmap,
			fetcher: f,
		},
		Error: func(err error) { pi.Errors = append(pi.Errors, err) },
	}
	pi.Package, _ = c.Check(pi.Name, pi.FileSet, pi.Files, pi.Info)
	pi.PackageVName[pi.Package] = unit.VName

	// Fill in the mapping from packages to vnames.
	for ip, vname := range imap {
		if pkg := pi.Dependencies[ip]; pkg != nil {
			pi.PackageVName[pkg] = proto.Clone(vname).(*spb.VName)
			pi.PackageVName[pkg].Signature = "package"
		}
	}
	if _, ok := pi.Dependencies["unsafe"]; ok {
		pi.PackageVName[types.Unsafe] = govname.ForStandardLibrary("unsafe")
	}

	// Set this package's own vname.
	pi.VName = proto.Clone(unit.VName).(*spb.VName)
	pi.VName.Language = govname.Language
	pi.VName.Signature = "package"

	return pi, nil
}

// String renders a human-readable synopsis of the package information.
func (pi *PackageInfo) String() string {
	if pi == nil {
		return "#<package-info nil>"
	}
	return fmt.Sprintf("#<package-info %q ip=%q pkg=%p #deps=%d #src=%d #errs=%d>",
		pi.Name, pi.ImportPath, pi.Package, len(pi.Dependencies), len(pi.Files), len(pi.Errors))
}

// Signature returns a signature for obj, suitable for use in a vname.
func (pi *PackageInfo) Signature(obj types.Object) string {
	if obj == nil {
		return ""
	} else if pi.owner == nil {
		pi.owner = make(map[types.Object]types.Object)
		pi.addOwners(pi.Package)
		for _, pkg := range pi.Dependencies {
			pi.addOwners(pkg)
		}
	}
	if sig, ok := pi.sigs[obj]; ok {
		return sig
	}
	tag, base := pi.newSignature(obj)
	sig := base
	if tag != "" {
		sig = tag + " " + base
	}
	pi.sigs[obj] = sig
	return sig
}

// ObjectVName returns a VName for obj relative to that of its package.
func (pi *PackageInfo) ObjectVName(obj types.Object) *spb.VName {
	if pkg, ok := obj.(*types.PkgName); ok {
		return pi.PackageVName[pkg.Imported()]
	}
	sig := pi.Signature(obj)
	base := pi.PackageVName[obj.Pkg()]
	if base == nil {
		return govname.ForBuiltin(sig)
	}
	vname := proto.Clone(base).(*spb.VName)
	vname.Signature = sig
	return vname
}

// FileVName returns a VName for path relative to the package base.
func (pi *PackageInfo) FileVName(path string) *spb.VName {
	vname := proto.Clone(pi.VName).(*spb.VName)
	vname.Language = ""
	vname.Signature = ""
	vname.Path = path
	return vname
}

// AnchorVName returns a VName for the given path and offsets.
func (pi *PackageInfo) AnchorVName(path string, start, end int) *spb.VName {
	vname := proto.Clone(pi.VName).(*spb.VName)
	vname.Signature = "#" + strconv.Itoa(start) + ":" + strconv.Itoa(end)
	vname.Path = path
	return vname
}

// Span returns the file path and 0-based offset range of the given AST node.
// The range is half-open, including the start position but excluding the end.
// If node == nil or lacks a valid start position, Span returns "", -1, -1.
// If the end position of node is invalid, start == end.
func (pi *PackageInfo) Span(node ast.Node) (path string, start, end int) {
	if node == nil {
		return "", -1, -1
	}
	pos := node.Pos()
	if pos == token.NoPos {
		return "", -1, -1
	}
	sp := pi.FileSet.Position(pos)
	path = sp.Filename
	start = sp.Offset
	end = start
	if pos := node.End(); pos != token.NoPos {
		end = pi.FileSet.Position(pos).Offset
	}
	return
}

// Anchor returns an anchor VName and offsets for the given AST node.
func (pi *PackageInfo) Anchor(node ast.Node) (vname *spb.VName, start, end int) {
	path, start, end := pi.Span(node)
	return pi.AnchorVName(path, start, end), start, end
}

const (
	isBuiltin = "builtin-"
	tagConst  = "const"
	tagField  = "field"
	tagFunc   = "func"
	tagLabel  = "label"
	tagMethod = "method"
	tagParam  = "param"
	tagType   = "type"
	tagVar    = "var"
)

// newSignature constructs and returns a tag and base signature for obj.  The
// tag represents the "kind" of signature, to disambiguate built-in types from
// user-defined names, fields from methods, and so on.  The base is a unique
// name for obj within its package, modulo the tag.
func (pi *PackageInfo) newSignature(obj types.Object) (tag, base string) {
	if obj.Name() == "" {
		return tagVar, "_"
	}
	topLevelTag := tagVar
	switch t := obj.(type) {
	case *types.Builtin:
		return isBuiltin + tagFunc, t.Name()

	case *types.Nil:
		return isBuiltin + tagConst, "nil"

	case *types.PkgName:
		return "", "package" // the vname corpus and path carry the package name

	case *types.Const:
		topLevelTag = tagConst
		if t.Pkg() == nil {
			return isBuiltin + tagConst, t.Name()
		}

	case *types.Var:
		if t.IsField() {
			if owner, ok := pi.owner[t]; ok {
				_, base := pi.newSignature(owner)
				return tagField, base + "." + t.Name()
			}
			return tagField, fmt.Sprintf("[%p].%s", t, t.Name())
		} else if owner, ok := pi.owner[t]; ok {
			_, base := pi.newSignature(owner)
			return tagParam, base + ":" + t.Name()
		}

	case *types.Func:
		topLevelTag = tagFunc
		if recv := t.Type().(*types.Signature).Recv(); recv != nil { // method
			if owner, ok := pi.owner[t]; ok {
				_, base := pi.newSignature(owner)
				return tagMethod, base + "." + t.Name()
			}
			return tagMethod, fmt.Sprintf("(%s).%s", recv.Type(), t.Name())
		}

	case *types.TypeName:
		topLevelTag = tagType
		if t.Pkg() == nil {
			return isBuiltin + tagType, t.Name()
		}

	case *types.Label:
		return tagLabel, fmt.Sprintf("[%p].%s", t, t.Name())

	default:
		log.Panicf("Unexpected object kind: %T", obj)
	}

	// At this point, we have eliminated built-in objects; everything else must
	// be defined in a package.
	if obj.Pkg() == nil {
		log.Panic("Object without a package: ", obj)
	}

	// Objects at package scope (i.e., parent scope is package scope).
	if obj.Parent() == obj.Pkg().Scope() {
		return topLevelTag, obj.Name()
	}

	// Objects in interior (local) scopes, i.e., everything else.
	return topLevelTag, fmt.Sprintf("[%p].%s", obj, obj.Name())
}

// addOwners updates pi.owner from the types in pkg, adding mapping from fields
// of package-level named struct types to the owning named struct type; from
// methods of package-level named interface types to the owning named interface
// type; and from parameters of package-level named function or method types to
// the owning named function or method.
//
// This relation is used to construct signatures for these fields/methods,
// since they may be referenced from another package and thus need
// deterministic names. An object does not expose its "owner"; indeed it may
// have several.
//
// Caveats:
//
// (1) This mapping is deterministic but not necessarily the best one according
// to the original syntax, to which, in general, we do not have access.  In
// these two examples, the type checker considers field X as belonging equally
// to types T and U, even though according the syntax, it belongs primarily to
// T in the first example and U in the second:
//
//      type T struct {X int}
//      type U T
//
//      type T U
//      type U struct {X int}
//
// Similarly:
//
//      type U struct {X int}
//      type V struct {U}
//
// TODO(adonovan): sameer@ points out a useful heuristic: in a case of struct
// or interface embedding, if one struct/interface has fewer fields/methods,
// then it must be the primary one.
//
// (2) This pass is not exhaustive: there remain objects that may be referenced
// from outside the package but for which we can't easily come up with good
// names.  Here are some examples:
//
//      // package p
//      var V1, V2 struct {X int} = ...
//      func F() struct{X int} {...}
//      type T struct {
//              Y struct { X int }
//      }
//
//      // main
//      p.V2.X = 1
//      print(p.F().X)
//      new(p.T).Y[0].X
//
// Also note that there may be arbitrary pointer, struct, chan, map, array, and
// slice type constructors between the type of the exported package member (V2,
// F or T) and the type of its X subelement.  For now, we simply ignore such
// names.  They should be rare in readable code.
func (pi *PackageInfo) addOwners(pkg *types.Package) {
	scope := pkg.Scope()
	for _, name := range scope.Names() {
		switch obj := scope.Lookup(name).(type) {
		case *types.TypeName:
			switch t := obj.Type().Underlying().(type) {
			case *types.Struct:
				// Inspect the fields of a struct.
				for i := 0; i < t.NumFields(); i++ {
					f := t.Field(i)
					if f.Pkg() != pkg {
						continue // wrong package
					}
					if _, ok := pi.owner[f]; !ok {
						pi.owner[f] = obj
					}
				}
			case *types.Interface:
				// Inspect the declared methods of an interface.
				for i := 0; i < t.NumMethods(); i++ {
					m := t.Method(i)
					if m.Pkg() != pkg {
						continue // wrong package
					}
					if _, ok := pi.owner[m]; !ok {
						pi.owner[m] = obj
					}
				}
			}

		case *types.Func:
			// Inspect the receiver, parameters, and result values.
			fsig := obj.Type().(*types.Signature)
			if recv := fsig.Recv(); recv != nil {
				pi.owner[recv] = obj
			}
			if params := fsig.Params(); params != nil {
				for i := 0; i < params.Len(); i++ {
					pi.owner[params.At(i)] = obj
				}
			}
			if res := fsig.Results(); res != nil {
				for i := 0; i < res.Len(); i++ {
					pi.owner[res.At(i)] = obj
				}
			}
		}
	}
}

// vnameToImport returns the putative Go import path corresponding to v.  The
// resulting string corresponds to the string literal appearing in source at
// the import site for the package so named.
func vnameToImport(v *spb.VName, goRoot string) string {
	if govname.IsStandardLibrary(v) || (goRoot != "" && v.Root == goRoot) {
		return v.Path
	} else if tail, ok := rootRelative(goRoot, v.Path); ok {
		// Paths under a nonempty GOROOT are treated as if they were standard
		// library packages even if they are not labelled as "golang.org", so
		// that nonstandard install locations will work sensibly.
		return strings.TrimSuffix(tail, filepath.Ext(tail))
	}
	trimmed := strings.TrimSuffix(v.Path, filepath.Ext(v.Path))
	return filepath.Join(v.Corpus, trimmed)
}

// rootRelative reports whether path has the form
//
//     root[/pkg/os_arch/]tail
//
// and if so, returns the tail. It returns path, false if path does not have
// this form.
func rootRelative(root, path string) (string, bool) {
	trimmed := strings.TrimPrefix(path, root+"/")
	if root == "" || trimmed == path {
		return path, false
	}
	if tail := strings.TrimPrefix(trimmed, "pkg/"); tail != trimmed {
		parts := strings.SplitN(tail, "/", 2)
		if len(parts) == 2 && strings.Contains(parts[0], "_") {
			return parts[1], true
		}
	}
	return trimmed, true
}

// getenv returns the value of the specified environment variable from unit.
// It returns "" if no such variable is defined.
func getenv(name string, unit *apb.CompilationUnit) string {
	for _, env := range unit.Environment {
		if env.Name == name {
			return env.Value
		}
	}
	return ""
}

// AllTypeInfo creates a new types.Info value with empty maps for each of the
// fields that can be filled in by the type-checker.
func AllTypeInfo() *types.Info {
	return &types.Info{
		Types:      make(map[ast.Expr]types.TypeAndValue),
		Defs:       make(map[*ast.Ident]types.Object),
		Uses:       make(map[*ast.Ident]types.Object),
		Implicits:  make(map[ast.Node]types.Object),
		Selections: make(map[*ast.SelectorExpr]*types.Selection),
		Scopes:     make(map[ast.Node]*types.Scope),
	}
}

// XRefTypeInfo creates a new types.Info value with empty maps for each of the
// fields needed for cross-reference indexing.
func XRefTypeInfo() *types.Info {
	return &types.Info{
		Types:     make(map[ast.Expr]types.TypeAndValue),
		Defs:      make(map[*ast.Ident]types.Object),
		Uses:      make(map[*ast.Ident]types.Object),
		Implicits: make(map[ast.Node]types.Object),
	}
}
