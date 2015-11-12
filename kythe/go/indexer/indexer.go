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

// TODO(fromberger): Connect the back end of the indexer.

import (
	"bufio"
	"bytes"
	"errors"
	"fmt"
	"go/ast"
	"go/parser"
	"go/token"
	"go/types"
	"log"
	"path"
	"sort"

	"github.com/golang/protobuf/proto"
	gcimporter "golang.org/x/tools/go/gcimporter15"

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
	VNames       map[*types.Package]*spb.VName // Resolved package to vname
	FileSet      *token.FileSet                // Location info for the source files
	Files        []*ast.File                   // The parsed ASTs of the source files
	SourceText   map[string]string             // The text of the source files, by path

	Info   *types.Info // If non-nil, contains type-checker results
	Errors []error     // All errors reported by the type checker

	// A lazily-initialized mapping from an object on the RHS of a selection
	// (lhs.RHS) to the nearest enclosing named struct or interface type; or in
	// the body of a function or method to the nearest enclosing named method.
	owner map[types.Object]types.Object

	// A cache of already-computed signatures.
	sigs map[types.Object]string
}

// Import satisfies the types.Importer interface using the captured data from
// the compilation unit.
func (pi *PackageInfo) Import(importPath string) (*types.Package, error) {
	if pkg := pi.Dependencies[importPath]; pkg != nil {
		return pkg, nil
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
	isSource := make(map[string]bool)
	for _, src := range unit.SourceFile {
		isSource[src] = true
	}

	deps := make(map[string]*types.Package) // import path → package
	imap := make(map[string]*spb.VName)     // import path → vname
	srcs := make(map[string]string)         // file path → text
	fset := token.NewFileSet()              // location info for the parser
	var files []*ast.File                   // parsed sources

	// Classify the required inputs as either sources, which are to be parsed,
	// or dependencies, which are to be "imported" via the type-checker's
	// import mechanism.  If successful, this populates fset and files with the
	// lexical and syntactic content of the package's own sources.
	for _, ri := range unit.RequiredInput {
		if ri.Info == nil {
			return nil, errors.New("required input file info missing")
		}

		// Fetch the contents of each required input.
		fpath := ri.Info.Path
		data, err := f.Fetch(fpath, ri.Info.Digest)
		if err != nil {
			return nil, fmt.Errorf("fetching %q (%s): %v", fpath, ri.Info.Digest, err)
		}

		// Source inputs need to be parsed, so we can give their ASTs to the
		// type checker later on.
		if isSource[fpath] {
			srcs[fpath] = string(data)
			parsed, err := parser.ParseFile(fset, fpath, data, parser.AllErrors)
			if err != nil {
				return nil, fmt.Errorf("parsing %q: %v", fpath, err)
			}
			files = append(files, parsed)
			continue
		}

		// For archives, recover the import path from the VName and read the
		// archive header for type information.  If the VName is not set, the
		// import is considered bogus.
		if ri.VName == nil {
			return nil, fmt.Errorf("missing vname for %q", fpath)
		}
		hdr := bufio.NewReader(bytes.NewReader(data))
		if _, err := gcimporter.FindExportData(hdr); err != nil {
			return nil, fmt.Errorf("scanning export data in %q: %v", fpath, err)
		}

		ipath := path.Join(ri.VName.Corpus, ri.VName.Path)
		if govname.IsStandardLibrary(ri.VName) {
			ipath = ri.VName.Path
		}
		imap[ipath] = ri.VName

		if _, err := gcimporter.ImportData(deps, fpath, ipath, hdr); err != nil {
			return nil, fmt.Errorf("importing %q: %v", ipath, err)
		}
	}

	// Fill in the mapping from packages to vnames.
	vmap := make(map[*types.Package]*spb.VName)
	for ip, vname := range imap {
		if pkg := deps[ip]; pkg != nil {
			vmap[pkg] = vname
		}
	}
	pi := &PackageInfo{
		Name:         files[0].Name.Name,
		ImportPath:   path.Join(unit.VName.Corpus, unit.VName.Path),
		VNames:       vmap,
		FileSet:      fset,
		Files:        files,
		SourceText:   srcs,
		Dependencies: deps,
		Info:         info,

		sigs: make(map[types.Object]string),
	}

	// Run the type-checker and collect any errors it generates.  Errors in the
	// type checker are not returned directly; the caller can read them from
	// the Errors field.
	c := &types.Config{
		FakeImportC:              true, // so we can handle cgo
		DisableUnusedImportCheck: true, // this is not fatal to type-checking
		Importer:                 pi,
		Error: func(err error) {
			pi.Errors = append(pi.Errors, err)
		},
	}
	if pkg, _ := c.Check(pi.Name, pi.FileSet, pi.Files, pi.Info); pkg != nil {
		pi.Package = pkg
		vmap[pkg] = unit.VName
	}
	return pi, nil
}

// String renders a human-readable synopsis of the package information.
func (pi *PackageInfo) String() string {
	if pi == nil {
		return "#<package-info nil>"
	}
	var deps []string
	for ip := range pi.Dependencies {
		deps = append(deps, ip)
	}
	sort.Strings(deps)
	return fmt.Sprintf("#<package-info %q ip=%q pkg=%p deps=%+s src=%d errs=%d>",
		pi.Name, pi.ImportPath, pi.Package, deps, len(pi.Files), len(pi.Errors))
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

// VName returns a VName for obj relative to that of its package.
func (pi *PackageInfo) VName(obj types.Object) *spb.VName {
	sig := pi.Signature(obj)
	base := pi.VNames[obj.Pkg()]
	if base == nil {
		return govname.ForBuiltin(sig)
	}
	vname := proto.Clone(base).(*spb.VName)
	vname.Signature = sig
	return vname
}

// Span returns the 0-based offset range of the given AST node.
// The range is half-open, including the start position but excluding the end.
// If node == nil or lacks a valid start position, Span returns -1, -1.
// If the end position of node is invalid, start == end.
func (pi *PackageInfo) Span(node ast.Node) (start, end int) {
	if node == nil {
		return -1, -1
	} else if pos := node.Pos(); pos == token.NoPos {
		return -1, -1
	} else {
		start = pi.FileSet.Position(pos).Offset
		end = start
	}
	if pos := node.End(); pos != token.NoPos {
		end = pi.FileSet.Position(pos).Offset
	}
	return
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
		return "", ":pkg:" // the vname corpus and path carry the package name

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
// deterministic names. An object does expose its "owner"; indeed, it may have
// several.
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
