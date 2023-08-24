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

// Package indexer implements a Kythe indexer for the Go language.
//
// Usage example: Indexing a Kythe CompilationUnit message.
//
//	// Obtain a compilation from some source, e.g., an kzip.
//	var unit *apb.CompilationUnit = ...
//
//	// Parse the sources and resolve types.
//	pi, err := indexer.Resolve(unit, pack, &indexer.ResolveOptions{
//	  Info: indexer.AllTypeInfo(),
//	})
//	if err != nil {
//	  log.Fatal("Resolving failed: %v", err)
//	}
//	// Type information from http://godoc.org/go/types is now available
//	// from pi.Info, which is a *types.Info record.
package indexer // import "kythe.io/kythe/go/indexer"

import (
	"bytes"
	"encoding/base64"
	"errors"
	"fmt"
	"go/ast"
	"go/build"
	"go/parser"
	"go/token"
	"go/types"
	"io"
	"io/ioutil"
	"path/filepath"
	"strconv"
	"strings"

	"kythe.io/kythe/go/extractors/govname"
	"kythe.io/kythe/go/util/log"
	"kythe.io/kythe/go/util/metadata"
	"kythe.io/kythe/go/util/ptypes"
	"kythe.io/kythe/go/util/schema/edges"

	"bitbucket.org/creachadair/stringset"
	"github.com/golang/protobuf/proto"
	"golang.org/x/tools/go/gcexportdata"

	apb "kythe.io/kythe/proto/analysis_go_proto"
	gopb "kythe.io/kythe/proto/go_go_proto"
	mpb "kythe.io/kythe/proto/metadata_go_proto"
	spb "kythe.io/kythe/proto/storage_go_proto"
)

// ErrNoSourceFiles is returned from Resolve if it is given a package without any sources files.
var ErrNoSourceFiles = errors.New("no source files in package")

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
	SourceText   map[*ast.File]string          // The text of the source files
	Rules        map[*ast.File]metadata.Rules  // Mapping metadata for each source file
	Vendored     map[string]string             // Mapping from package to its vendor path

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
	// functions, one per file in the package.
	packageInit map[*ast.File]*funcInfo

	// A cache of source file vnames.
	fileVName map[*ast.File]*spb.VName

	// A cache of type vnames.
	typeVName map[types.Type]*spb.VName

	// Cache storing whether a type's facts have been emitted (keyed by VName signature)
	typeEmitted stringset.Set

	// A cache of file location mappings. This lets us get back from the
	// parser's location to the vname for the enclosing file, which is in turn
	// affected by the build configuration.
	fileLoc map[*token.File]*ast.File

	// A cache of already-computed signatures.
	sigs map[types.Object]string

	// The number of package-level init declarations seen.
	numInits int

	// The Go-specific details from the compilation record.
	details *gopb.GoDetails
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

	pkgPath  string
	vendored map[string]string // map of package paths to their vendor paths
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
	fi := pi.fileMap[importPath]

	// Check all possible vendor/ paths for missing package file
	if fi == nil {
		// Starting at the current package's path, look upwards for a vendor/ root
		// with the imported package.
		vendorRoot := pi.pkgPath
		for {
			vPath := filepath.Join(vendorRoot, "vendor", importPath)
			fi = pi.fileMap[vPath]
			if fi != nil {
				pi.vendored[importPath] = vPath
				importPath = vPath
				break
			} else if vendorRoot == "" {
				return nil, fmt.Errorf("package %q not found", importPath)
			}
			vendorRoot, _ = filepath.Split(vendorRoot)
			vendorRoot = strings.TrimRight(vendorRoot, "/")
		}
	}

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

// ResolveOptions control the behaviour of the Resolve function. A nil options
// pointer provides default values.
type ResolveOptions struct {
	// Passes a value whose non-nil map fields will be filled in by the type
	// checker during resolution. The value will also be copied to the Info
	// field of the PackageInfo returned by Resolve.
	Info *types.Info

	// If set, this function is called for each required input to check whether
	// it contains metadata rules.
	//
	// Valid return are:
	//    rs, nil    -- a valid ruleset
	//    nil, nil   -- no ruleset found
	//    _, err     -- an error attempting to load a ruleset
	//
	CheckRules func(ri *apb.CompilationUnit_FileInput, f Fetcher) (*Ruleset, error)
}

func (r *ResolveOptions) info() *types.Info {
	if r != nil {
		return r.Info
	}
	return nil
}

func (r *ResolveOptions) checkRules(ri *apb.CompilationUnit_FileInput, f Fetcher) (*Ruleset, error) {
	if r == nil || r.CheckRules == nil {
		return nil, nil
	}
	return r.CheckRules(ri, f)
}

// A Ruleset represents a collection of mapping rules applicable to a source
// file in a compilation to be indexed.
type Ruleset struct {
	Path  string         // the file path this rule set applies to
	Rules metadata.Rules // the rules that apply to the path
}

// Resolve resolves the package information for unit and its dependencies.  On
// success the package corresponding to unit is located via ImportPath in the
// Packages map of the returned value.
func Resolve(unit *apb.CompilationUnit, f Fetcher, opts *ResolveOptions) (*PackageInfo, error) {
	sourceFiles := stringset.New(unit.SourceFile...)

	imap := make(map[string]*spb.VName)     // import path → vname
	srcs := make(map[*ast.File]string)      // file → text
	fmap := make(map[string]*apb.FileInfo)  // import path → file info
	smap := make(map[string]*ast.File)      // file path → file (sources)
	filev := make(map[*ast.File]*spb.VName) // file → vname
	floc := make(map[*token.File]*ast.File) // file → ast
	fset := token.NewFileSet()              // location info for the parser
	details := goDetails(unit)
	var files []*ast.File // parsed sources
	var rules []*Ruleset  // parsed linkage rules

	// Classify the required inputs as either sources, which are to be parsed,
	// or dependencies, which are to be "imported" via the type-checker's
	// import mechanism.  If successful, this populates fset and files with the
	// lexical and syntactic content of the package's own sources.
	//
	// The build context is used to check build tags.
	bc := &build.Context{
		GOOS:        details.GetGoos(),
		GOARCH:      details.GetGoarch(),
		BuildTags:   details.GetBuildTags(),
		ReleaseTags: build.Default.ReleaseTags,
		ToolTags:    build.Default.ToolTags,
	}
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
			if !matchesBuildTags(fpath, data, bc) {
				log.Infof("Skipped source file %q because build tags do not match", fpath)
				continue
			}
			vpath := ri.VName.GetPath()
			if vpath == "" {
				vpath = fpath
			}
			parsed, err := parser.ParseFile(fset, vpath, data, parser.AllErrors|parser.ParseComments)
			if err != nil {
				return nil, fmt.Errorf("parsing %q: %v", fpath, err)
			}

			// Cache file VNames based on the required input.
			files = append(files, parsed)
			vname := proto.Clone(ri.VName).(*spb.VName)
			if vname == nil {
				vname = proto.Clone(unit.VName).(*spb.VName)
				vname.Signature = ""
				vname.Language = ""
			}
			vname.Path = vpath
			filev[parsed] = vname
			srcs[parsed] = string(data)
			smap[fpath] = parsed

			// If the file has inlined encoded metadata, add to rules.
			var lastComment string
			if len(parsed.Comments) > 0 {
				lastComment = parsed.Comments[len(parsed.Comments)-1].Text()
			}
			const delimiter = "gokythe-inline-metadata:"

			if strings.HasPrefix(lastComment, delimiter) {
				encodedMetadata := strings.TrimPrefix(lastComment, delimiter)
				newRule, err := loadInlineMetadata(ri, encodedMetadata)
				if err != nil {
					log.Errorf("loading metadata in %q: %v", ri.Info.GetPath(), err)
				} else {
					rules = append(rules, newRule)
				}
			}
			continue
		}

		// Check for mapping metadata.
		if rs, err := opts.checkRules(ri, f); err != nil {
			log.Errorf("checking rules in %q: %v", fpath, err)
		} else if rs != nil {
			log.Infof("Found %d metadata rules for path %q", len(rs.Rules), rs.Path)
			rules = append(rules, rs)
			continue
		}

		// Files may be source or compiled archives with type information for
		// other packages, or may be other ancillary files like C headers to
		// support cgo.  Use the vname to determine which import path for each
		// and save that mapping for use by the importer.
		if ri.VName == nil {
			return nil, fmt.Errorf("missing vname for %q", fpath)
		}

		var ipath string
		if info := goPackageInfo(ri.Details); info != nil {
			ipath = info.ImportPath
		} else {
			ipath = govname.ImportPath(ri.VName, details.GetGoroot())
		}
		imap[ipath] = ri.VName
		fmap[ipath] = ri.Info
	}
	if len(files) == 0 {
		return nil, ErrNoSourceFiles
	}

	// Populate the location mapping. This relies on the fact that Iterate
	// reports its files in the order they were added to the set, which in turn
	// is their order in the files list.
	i := 0
	fset.Iterate(func(f *token.File) bool {
		floc[f] = files[i]
		i++
		return true
	})

	pi := &PackageInfo{
		Name:         files[0].Name.Name,
		FileSet:      fset,
		Files:        files,
		Info:         opts.info(),
		SourceText:   srcs,
		PackageVName: make(map[*types.Package]*spb.VName),
		Dependencies: make(map[string]*types.Package), // :: import path → package
		Vendored:     make(map[string]string),

		function:    make(map[ast.Node]*funcInfo),
		sigs:        make(map[types.Object]string),
		packageInit: make(map[*ast.File]*funcInfo),
		fileVName:   filev,
		typeVName:   make(map[types.Type]*spb.VName),
		typeEmitted: stringset.New(),
		fileLoc:     floc,
		details:     details,
	}
	if info := goPackageInfo(unit.Details); info != nil {
		pi.ImportPath = info.ImportPath
	} else {
		pi.ImportPath = govname.ImportPath(unit.VName, details.GetGoroot())
	}

	// If mapping rules were found, populate the corresponding field.
	if len(rules) != 0 {
		pi.Rules = make(map[*ast.File]metadata.Rules)
		for _, rs := range rules {
			f, ok := smap[rs.Path]
			if ok {
				pi.Rules[f] = rs.Rules
			}
		}
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

			pkgPath:  pi.ImportPath,
			vendored: pi.Vendored,
		},
		Error: func(err error) { pi.Errors = append(pi.Errors, err) },
	}
	pi.Package, _ = c.Check(pi.Name, pi.FileSet, pi.Files, pi.Info)

	// Fill in the mapping from packages to vnames.
	for ip, vname := range imap {
		if pkg := pi.Dependencies[ip]; pkg != nil {
			pi.PackageVName[pkg] = proto.Clone(vname).(*spb.VName)
			pi.PackageVName[pkg].Signature = "package"
			pi.PackageVName[pkg].Language = govname.Language
		}
	}
	if _, ok := pi.Dependencies["unsafe"]; ok {
		pi.PackageVName[types.Unsafe] = govname.ForStandardLibrary("unsafe")
	}

	// Set this package's own vname.
	pi.VName = proto.Clone(unit.VName).(*spb.VName)
	pi.VName.Language = govname.Language
	pi.VName.Signature = "package"
	pi.PackageVName[pi.Package] = pi.VName

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
		ownerByPos := make(map[token.Position]types.Object)
		unownedByPos := make(map[token.Position][]types.Object)
		pi.addOwners(pi.Package, ownerByPos, unownedByPos)
		for _, pkg := range pi.Dependencies {
			pi.addOwners(pkg, ownerByPos, unownedByPos)
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
	pkg := obj.Pkg()
	var vname *spb.VName
	if base := pi.PackageVName[pkg]; base != nil {
		vname = proto.Clone(base).(*spb.VName)
	} else if pkg == nil {
		return govname.Builtin(obj.Name())
	} else {
		// This is an indirect import, that is, a name imported but not
		// mentioned explicitly by the package being indexed.
		// TODO(#2383): This is a workaround, and may not be correct in all
		// cases; work out a more comprehensive solution (possibly during
		// extraction).
		vname = proto.Clone(pi.VName).(*spb.VName)
		vname.Path = strings.TrimPrefix(pkg.Path(), vname.Corpus+"/")
	}

	vname.Signature = sig
	return vname
}

// FileVName returns a VName for path relative to the package base.
func (pi *PackageInfo) FileVName(file *ast.File) *spb.VName {
	if v := pi.fileVName[file]; v != nil {
		return v
	}
	v := proto.Clone(pi.VName).(*spb.VName)
	v.Language = ""
	v.Signature = ""
	v.Path = pi.FileSet.Position(file.Pos()).Filename
	return v
}

// AnchorVName returns a VName for the given file and offsets.
func (pi *PackageInfo) AnchorVName(file *ast.File, start, end int) *spb.VName {
	vname := proto.Clone(pi.FileVName(file)).(*spb.VName)
	vname.Signature = "#" + strconv.Itoa(start) + ":" + strconv.Itoa(end)
	vname.Language = govname.Language
	return vname
}

// Span returns the containing file and 0-based offset range of the given AST
// node.  The range is half-open, including the start position but excluding
// the end.
//
// If node == nil or lacks a valid start position, Span returns nil -1, -1.  If
// the end position of node is invalid, start == end.
func (pi *PackageInfo) Span(node ast.Node) (file *ast.File, start, end int) {
	if node == nil {
		return nil, -1, -1
	}
	pos := node.Pos()
	if pos == token.NoPos {
		return nil, -1, -1
	}
	sp := pi.FileSet.Position(pos)
	file = pi.fileLoc[pi.FileSet.File(pos)]
	start = sp.Offset
	end = start
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
	tagTVar   = "tvar"
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
			return tagField, pi.anonSignature(t)
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
			// If the receiver is defined in this package, fully qualify the
			// name so references from elsewhere will work. Strictly speaking
			// this is only necessary for exported methods, but it's simpler to
			// do it for everything.
			return tagMethod, fmt.Sprintf("(%s).%s", types.TypeString(recv.Type(), func(pkg *types.Package) string {
				return pkg.Name()
			}), t.Name())
		}

	case *types.TypeName:
		if param, ok := t.Type().(*types.TypeParam); ok {
			return tagTVar, fmt.Sprintf("[%p]%s", t, param.String())
		}
		topLevelTag = tagType
		if t.Pkg() == nil {
			return isBuiltin + tagType, t.Name()
		}

	case *types.Label:
		return tagLabel, pi.anonSignature(t)

	default:
		panic(fmt.Sprintf("Unexpected object kind: %T", obj))
	}

	// At this point, we have eliminated built-in objects; everything else must
	// be defined in a package.
	if obj.Pkg() == nil {
		panic(fmt.Sprintf("Object without a package: %v", obj))
	}

	// Objects at package scope (i.e., parent scope is package scope).
	if obj.Parent() == obj.Pkg().Scope() {
		return topLevelTag, obj.Name()
	}

	// Objects in interior (local) scopes, i.e., everything else.
	return topLevelTag, pi.anonSignature(obj)
}

func (pi *PackageInfo) anonSignature(obj types.Object) string {
	// Use the object's line number and file basename to differentiate the
	// node while allowing for cross-package references (other parts of the
	// Position may differ).  This may collide if a source file isn't gofmt'd
	// and defines multiple anonymous fields with the same name on the same
	// line, but that's unlikely to happen in practice.
	pos := pi.FileSet.Position(obj.Pos())
	return fmt.Sprintf("[%s#%d].%s", filepath.Base(pos.Filename), pos.Line, obj.Name())
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
//	type T struct {X int}
//	type U T
//
//	type T U
//	type U struct {X int}
//
// Similarly:
//
//	type U struct {X int}
//	type V struct {U}
//
// TODO(adonovan): sameer@ points out a useful heuristic: in a case of struct
// or interface embedding, if one struct/interface has fewer fields/methods,
// then it must be the primary one.
//
// (2) This pass is not exhaustive: there remain objects that may be referenced
// from outside the package but for which we can't easily come up with good
// names.  Here are some examples:
//
//	// package p
//	var V1, V2 struct {X int} = ...
//	func F() struct{X int} {...}
//	type T struct {
//	        Y struct { X int }
//	}
//
//	// main
//	p.V2.X = 1
//	print(p.F().X)
//	new(p.T).Y[0].X
//
// Also note that there may be arbitrary pointer, struct, chan, map, array, and
// slice type constructors between the type of the exported package member (V2,
// F or T) and the type of its X subelement.  For now, we simply ignore such
// names.  They should be rare in readable code.
func (pi *PackageInfo) addOwners(pkg *types.Package, ownerByPos map[token.Position]types.Object, unownedByPos map[token.Position][]types.Object) {
	scope := pkg.Scope()
	addTypeParams := func(obj types.Object, params *types.TypeParamList) {
		mapTypeParams(params, func(i int, param *types.TypeParam) {
			typeName := param.Obj()
			if _, ok := pi.owner[typeName]; !ok {
				pi.owner[typeName] = obj
			}
		})
	}
	addFunc := func(obj *types.Func) {
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
		addTypeParams(obj, fsig.TypeParams())
		addTypeParams(obj, fsig.RecvTypeParams())
	}
	addMethods := func(obj types.Object, n int, method func(i int) *types.Func) {
		for i := 0; i < n; i++ {
			m := method(i)
			if _, ok := pi.owner[m]; !ok {
				pos := pi.FileSet.Position(m.Pos())
				if m.Pkg() == pkg {
					pi.owner[m] = obj
					addFunc(m)

					// Keep track of owners by position to facilitate ownership across
					// package boundaries for new type names of existing interfaces:
					//
					//    package pkg
					//    type A fmt.Stringer
					//
					//    -----
					//
					//    package other
					//    import "pkg"
					//                         \/
					//    func f(a pkg.A) { a.String() }
					//
					// Understanding that the above reference is to
					// (fmt.Stringer).String() is lost due to compiler unwrapping the
					// `type A fmt.Stringer` definition for downstream compilations.
					// However, position info remains behind for stack traces so we can
					// use it as fallback heuristic to determine "sameness" of methods.
					ownerByPos[pos] = obj
					for _, m := range unownedByPos[pos] {
						pi.owner[m] = obj
					}
					delete(unownedByPos, pos)
				} else if owner, ok := ownerByPos[pos]; ok {
					pi.owner[m] = owner
					addFunc(m)
				} else {
					unownedByPos[pos] = append(unownedByPos[pos], m)
				}
			}
		}
	}
	addNamed := func(obj types.Object, named *types.Named) {
		addTypeParams(obj, named.TypeParams())
		switch t := named.Underlying().(type) {
		case *types.Struct:
			// Inspect the fields of a struct.
			for i := 0; i < t.NumFields(); i++ {
				f := t.Field(i)
				if f.Pkg() != pkg && named.TypeArgs().Len() == 0 {
					continue // wrong package (and not an instantiated type)
				}
				if _, ok := pi.owner[f]; !ok {
					pi.owner[f] = obj
				}
			}
			addMethods(obj, named.NumMethods(), named.Method)

		case *types.Interface:
			// Inspect the declared methods of an interface.
			addMethods(obj, t.NumExplicitMethods(), t.ExplicitMethod)

		default:
			// Inspect declared methods of other named types.
			addMethods(obj, named.NumMethods(), named.Method)
		}
	}

	for _, name := range scope.Names() {
		switch obj := scope.Lookup(name).(type) {
		case *types.TypeName:
			// TODO(schroederc): support for type aliases.  For now, skip these so we
			// don't wind up emitting redundant declaration sites for the aliased
			// type.
			named, ok := obj.Type().(*types.Named)
			if !ok {
				continue
			}
			addNamed(obj, named)

		case *types.Func:
			addFunc(obj)
		}
	}

	if pkg == pi.Package {
		// Add owners for members of known known instantiations
		for _, t := range pi.Info.Types {
			if n, ok := t.Type.(*types.Named); ok && n.TypeArgs().Len() > 0 {
				addNamed(n.Obj(), n)
			}
		}
	}
}

// mapTypeParams applies f to each type parameter declared in params.  Each call
// to f is given the offset and the type parameter.
func mapTypeParams(params *types.TypeParamList, f func(i int, id *types.TypeParam)) {
	if params == nil {
		return
	}
	for i := 0; i < params.Len(); i++ {
		f(i, params.At(i))
	}
}

// findFieldName tries to resolve the identifier that names an embedded
// anonymous field declaration at expr, and reports whether successful.
func (pi *PackageInfo) findFieldName(expr ast.Expr) (id *ast.Ident, ok bool) {
	// There are three cases we care about here:
	//
	// A bare identifier (foo), which refers to a type defined in
	// this package, or a builtin type,
	//
	// A selector expression (pkg.Foo) referring to an exported
	// type defined in another package, or
	//
	// A pointer to either of these.

	switch t := expr.(type) {
	case *ast.StarExpr: // *T
		return pi.findFieldName(t.X)
	case *ast.Ident: // T declared locally
		return t, true
	case *ast.SelectorExpr: // pkg.T declared elsewhere
		return t.Sel, true
	default:
		// No idea what this is; possibly malformed code.
		return nil, false
	}
}

// importPath returns the import path of pkg.
func (pi *PackageInfo) importPath(pkg *types.Package) string {
	if v := pi.PackageVName[pkg]; v != nil {
		return govname.ImportPath(v, pi.details.GetGoroot())
	}
	return pkg.Name()
}

// isPackageInit reports whether fi belongs to a package-level init function.
func (pi *PackageInfo) isPackageInit(fi *funcInfo) bool {
	for _, v := range pi.packageInit {
		if fi == v {
			return true
		}
	}
	return false
}

// goDetails returns the GoDetails message attached to unit, if there is one;
// otherwise it returns nil.
func goDetails(unit *apb.CompilationUnit) *gopb.GoDetails {
	for _, msg := range unit.Details {
		var dets gopb.GoDetails
		if err := ptypes.UnmarshalAny(msg, &dets); err == nil {
			return &dets
		}
	}
	return nil
}

// goPackageInfo returns the GoPackageInfo message within the given slice, if
// there is one; otherwise it returns nil.
func goPackageInfo(details []*ptypes.Any) *gopb.GoPackageInfo {
	for _, msg := range details {
		var info gopb.GoPackageInfo
		if err := ptypes.UnmarshalAny(msg, &info); err == nil {
			return &info
		}
	}
	return nil
}

// loadInlineMetadata unmarshals the encoded metadata string and returns
// the corresponding metadata rules which link source definitions
// to the file fi. An error is returned if unmarshalling fails.
func loadInlineMetadata(fi *apb.CompilationUnit_FileInput, encodedMetadata string) (*Ruleset, error) {
	var gci mpb.GeneratedCodeInfo
	err := unmarshalProtoBase64(encodedMetadata, &gci)
	if err != nil {
		return nil, err
	}

	rules := make(metadata.Rules, 0, len(gci.GetMeta()))
	for _, r := range gci.GetMeta() {
		k, rev := strings.CutPrefix(r.Edge, "%")
		rules = append(rules, metadata.Rule{
			EdgeIn:  edges.DefinesBinding,
			EdgeOut: k,
			VName:   r.GetVname(),
			Reverse: rev,
			Begin:   int(r.GetBegin()),
			End:     int(r.GetEnd()),
		})
	}
	return &Ruleset{
		Path:  fi.Info.GetPath(),
		Rules: rules,
	}, nil
}

// matchesBuildTags reports whether the file at fpath, whose content is in
// data, would be matched by the settings in bc.
func matchesBuildTags(fpath string, data []byte, bc *build.Context) bool {
	dir, name := filepath.Split(fpath)
	bc.OpenFile = func(path string) (io.ReadCloser, error) {
		if path != fpath {
			return nil, errors.New("file not found")
		}
		return ioutil.NopCloser(bytes.NewReader(data)), nil
	}
	match, err := bc.MatchFile(dir, name)
	return err == nil && match
}

// unmarshalProtoBase64 parses a base64 encoded proto string into the supplied
// message, mutating msg.
func unmarshalProtoBase64(base64Str string, msg proto.Message) error {
	b, _ := base64.StdEncoding.DecodeString(base64Str)
	return proto.Unmarshal(b, msg)
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
