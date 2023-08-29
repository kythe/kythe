/*
 * Copyright 2016 The Kythe Authors. All rights reserved.
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

package indexer

import (
	"context"
	"fmt"
	"go/ast"
	"go/token"
	"go/types"
	"net/url"
	"path"
	"strconv"
	"strings"

	"kythe.io/kythe/go/extractors/govname"
	"kythe.io/kythe/go/util/log"
	"kythe.io/kythe/go/util/metadata"
	"kythe.io/kythe/go/util/schema/edges"
	"kythe.io/kythe/go/util/schema/facts"
	"kythe.io/kythe/go/util/schema/nodes"

	"github.com/golang/protobuf/proto"
	"golang.org/x/tools/go/types/typeutil"

	cpb "kythe.io/kythe/proto/common_go_proto"
	spb "kythe.io/kythe/proto/storage_go_proto"
)

// EmitOptions control the behaviour of the Emit function. A nil options
// pointer provides default values.
type EmitOptions struct {
	// If true, emit nodes for standard library packages when they are first
	// encountered. This is helpful if you want to index a package in isolation
	// where data for the standard library are not available.
	EmitStandardLibs bool

	// If true, emit code facts containing MarkedSource messages.
	EmitMarkedSource bool

	// If true, emit linkages specified by metadata rules.
	EmitLinkages bool

	// If true, emit childof edges for an anchor's semantic scope.
	EmitAnchorScopes bool

	// If true, use the enclosing file for top-level callsite scopes.
	UseFileAsTopLevelScope bool

	// If set, use this as the base URL for links to godoc.  The import path is
	// appended to the path of this URL to obtain the target URL to link to.
	DocBase *url.URL

	// If true, the doc/uri fact is only emitted for go std library packages.
	OnlyEmitDocURIsForStandardLibs bool

	// If enabled, all VNames emitted by the indexer are assigned the
	// compilation unit's corpus.
	UseCompilationCorpusForAll bool

	// If set, all stdlib nodes are assigned this corpus. This takes precedence
	// over UseCompilationCorpusForAll for stdlib nodes.
	OverrideStdlibCorpus string

	// EmitRefCallOverIdentifier determines whether ref/call anchors are emitted
	// over function identifiers (or the legacy behavior of over the entire
	// callsite).
	EmitRefCallOverIdentifier bool

	// Verbose determines whether verbose logging is enabled.
	Verbose bool
}

func (e *EmitOptions) emitMarkedSource() bool {
	if e == nil {
		return false
	}
	return e.EmitMarkedSource
}

func (e *EmitOptions) emitAnchorScopes() bool {
	if e == nil {
		return false
	}
	return e.EmitAnchorScopes
}

func (e *EmitOptions) emitRefCallOverIdentifier() bool {
	if e == nil {
		return false
	}
	return e.EmitRefCallOverIdentifier
}

func (e *EmitOptions) useFileAsTopLevelScope() bool {
	if e == nil {
		return false
	}
	return e.UseFileAsTopLevelScope
}

// shouldEmit reports whether the indexer should emit a node for the given
// vname.  Presently this is true if vname denotes a standard library and the
// corresponding option is enabled.
func (e *EmitOptions) shouldEmit(vname *spb.VName) bool {
	return e != nil && e.EmitStandardLibs && govname.IsStandardLibrary(vname)
}

// docURL returns a documentation URL for the specified package, if one is
// specified by the options, or "" if not.
func (e *EmitOptions) docURL(pi *PackageInfo) string {
	if e == nil || e.DocBase == nil {
		return ""
	}
	if e.OnlyEmitDocURIsForStandardLibs && !govname.IsStandardLibrary(pi.VName) {
		return ""
	}

	u := *e.DocBase
	u.Path = path.Join(u.Path, pi.ImportPath)
	return u.String()
}

func (e *EmitOptions) verbose() bool {
	if e == nil {
		return false
	}
	return e.Verbose
}

// An impl records that a type A implements an interface B.
type impl struct{ A, B types.Object }

// Emit generates Kythe facts and edges to represent pi, and writes them to
// sink. In case of errors, processing continues as far as possible before the
// first error encountered is reported.
func (pi *PackageInfo) Emit(ctx context.Context, sink Sink, opts *EmitOptions) error {
	if opts == nil {
		opts = &EmitOptions{}
	}
	e := &emitter{
		ctx:      ctx,
		pi:       pi,
		sink:     sink,
		opts:     opts,
		impl:     make(map[impl]struct{}),
		anchored: make(map[ast.Node]struct{}),
		fmeta:    make(map[*ast.File]bool),
	}

	// Emit a node to represent the package as a whole.
	e.writeFact(pi.VName, facts.NodeKind, nodes.Package)
	if url := e.opts.docURL(pi); url != "" {
		e.writeFact(pi.VName, facts.DocURI, url)
	}
	e.emitPackageMarkedSource(pi)

	// Emit facts for all the source files claimed by this package.
	for file, text := range pi.SourceText {
		vname := pi.FileVName(file)
		e.writeFact(vname, facts.NodeKind, nodes.File)
		e.writeFact(vname, facts.Text, text)
		// All Go source files are encoded as UTF-8, which is the default.

		e.writeEdge(vname, pi.VName, edges.ChildOf)
	}

	// Traverse the AST of each file in the package for xref entries.
	for _, file := range pi.Files {
		e.cmap = ast.NewCommentMap(pi.FileSet, file, file.Comments)
		e.writeDoc(file.Doc, pi.VName)                        // capture package comments
		e.writeRef(file.Name, pi.VName, edges.DefinesBinding) // define a binding for the package
		ast.Walk(newASTVisitor(func(node ast.Node, stack stackFunc) bool {
			switch n := node.(type) {
			case *ast.Ident:
				e.visitIdent(n, stack)
			case *ast.FuncDecl:
				e.visitFuncDecl(n, stack)
			case *ast.FuncLit:
				e.visitFuncLit(n, stack)
			case *ast.ValueSpec:
				e.visitValueSpec(n, stack)
			case *ast.TypeSpec:
				e.visitTypeSpec(n, stack)
			case *ast.ImportSpec:
				e.visitImportSpec(n, stack)
			case *ast.AssignStmt:
				e.visitAssignStmt(n, stack)
			case *ast.RangeStmt:
				e.visitRangeStmt(n, stack)
			case *ast.CompositeLit:
				e.visitCompositeLit(n, stack)
			case *ast.IndexExpr:
				e.visitIndexExpr(n, stack)
			case *ast.IndexListExpr:
				e.visitIndexListExpr(n, stack)
			case *ast.ArrayType:
				e.visitArrayType(n, stack)
			}
			return true
		}), file)
	}

	// Emit edges from each named type to the interface types it satisfies, for
	// those interface types that are known to this compiltion.
	e.emitSatisfactions()

	// TODO(fromberger): Add diagnostics for type-checker errors.
	if opts.verbose() {
		for _, err := range pi.Errors {
			log.Warningf("Type resolution error: %v", err)
		}
	}
	return e.firstErr
}

type emitter struct {
	ctx      context.Context
	pi       *PackageInfo
	sink     Sink
	opts     *EmitOptions
	impl     map[impl]struct{}                    // see checkImplements
	rmap     map[*ast.File]map[int]metadata.Rules // see applyRules
	fmeta    map[*ast.File]bool                   // see applyRules
	anchored map[ast.Node]struct{}                // see writeAnchor
	firstErr error
	cmap     ast.CommentMap // current file's CommentMap
}

type refKind int

const (
	readRef refKind = iota
	writeRef
	readWriteRef
)

func exprRefKind(tgt ast.Expr, stack stackFunc, depth int) refKind {
	switch parent := stack(depth + 1).(type) {
	case *ast.AssignStmt:
		// Check if identifier is being assigned; we assume this is not a definition
		// and checked by the caller.
		for _, lhs := range parent.Lhs {
			if lhs == tgt {
				switch parent.Tok {
				case token.ASSIGN, token.DEFINE:
					return writeRef
				default: // +=, etc.
					return readWriteRef
				}
			}
		}
	case *ast.IncDecStmt:
		return readWriteRef
	case *ast.SelectorExpr:
		if id, ok := tgt.(*ast.Ident); ok && id == parent.Sel {
			return exprRefKind(parent, stack, depth+1)
		}
	case *ast.KeyValueExpr:
		if tgt == parent.Key {
			if c, ok := stack(depth + 2).(*ast.CompositeLit); ok {
				if _, isMap := c.Type.(*ast.MapType); !isMap {
					return writeRef
				}
			}
		}
	}
	return readRef
}

// visitIdent handles referring identifiers. Declaring identifiers are handled
// as part of their parent syntax.
func (e *emitter) visitIdent(id *ast.Ident, stack stackFunc) {
	obj := e.pi.Info.Uses[id]
	if obj == nil {
		// Defining identifiers are handled by their parent nodes.
		return
	}

	if sig, ok := obj.Type().(*types.Signature); ok && sig.Recv() != nil && sig.RecvTypeParams().Len() > 0 {
		// Lookup the original non-instantiated method to reference.
		if n, ok := deref(sig.Recv().Type()).(*types.Named); ok {
			f, _, _ := types.LookupFieldOrMethod(n.Origin(), true, obj.Pkg(), obj.Name())
			if f != nil {
				obj = f
			}
		}
	}

	// Receiver type parameter identifiers are both usages and definitions; take
	// the opportunity to emit a binding and do not continue to emit a Ref edge.
	if def, ok := e.pi.Info.Defs[id].(*types.TypeName); ok && def == obj {
		e.writeBinding(id, nodes.TVar, nil)
		return
	}

	var target *spb.VName
	if n, ok := obj.(*types.TypeName); ok && obj.Pkg() == nil {
		// Handle type arguments in instantiated types.
		target = e.emitType(n.Type())
	} else {
		target = e.pi.ObjectVName(obj)
	}

	if target == nil {
		// This should not happen in well-formed packages, but can if the
		// extractor gets confused. Avoid emitting confusing references in such
		// cases. Note that in this case we need to emit a fresh anchor, since
		// we aren't otherwise emitting a reference.
		e.writeNodeDiagnostic(id, diagnostic{
			Message: fmt.Sprintf("Unable to identify the package for %q", id.Name),
		})
		return
	}

	var refs []*spb.VName
	refKind := exprRefKind(id, stack, 0)
	if refKind == readRef || refKind == readWriteRef {
		refs = append(refs, e.writeRef(id, target, edges.Ref))
	}
	if refKind == writeRef || refKind == readWriteRef {
		refs = append(refs, e.writeRef(id, target, edges.RefWrites))
	}

	if e.opts.emitAnchorScopes() {
		parent := e.callContext(stack).vname
		for _, ref := range refs {
			e.writeEdge(ref, parent, edges.ChildOf)
		}
	}
	if call, ok := isCall(id, obj, stack); ok {
		var callAnchor *spb.VName
		if e.opts.emitRefCallOverIdentifier() {
			callAnchor = e.writeRef(id, target, edges.RefCall)
		} else {
			callAnchor = e.writeRef(call, target, edges.RefCall)
		}

		// Paint an edge to the function blamed for the call, or if there is
		// none then to the package initializer.
		e.writeEdge(callAnchor, e.callContext(stack).vname, edges.ChildOf)
	}
}

// visitFuncDecl handles function and method declarations and their parameters.
func (e *emitter) visitFuncDecl(decl *ast.FuncDecl, stack stackFunc) {
	info := &funcInfo{vname: new(spb.VName)}
	e.pi.function[decl] = info

	// Get the type of this function, even if its name is blank.
	obj, _ := e.pi.Info.Defs[decl.Name].(*types.Func)
	if obj == nil {
		return // a redefinition, for example
	}

	// Special case: There may be multiple package-level init functions, so
	// override the normal signature generation to include a discriminator.
	if decl.Recv == nil && obj.Name() == "init" {
		e.pi.numInits++
		e.pi.sigs[obj] = fmt.Sprintf("%s#%d", e.pi.Signature(obj), e.pi.numInits)
	}

	info.vname = e.mustWriteBinding(decl.Name, nodes.Function, nil)
	e.writeDef(decl, info.vname)
	e.writeDoc(decl.Doc, info.vname)

	// For concrete methods: Emit the receiver if named, and connect the method
	// to its declaring type.
	sig := obj.Type().(*types.Signature)
	if sig.Recv() != nil {
		// The receiver is treated as parameter 0.
		if names := decl.Recv.List[0].Names; names != nil {
			if recv := e.writeBinding(names[0], nodes.Variable, info.vname); recv != nil {
				e.writeEdge(info.vname, recv, edges.ParamIndex(0))
			}
		}

		// The method should be a child of its (named) enclosing type.
		if named, _ := deref(sig.Recv().Type()).(*types.Named); named != nil {
			base := e.pi.ObjectVName(named.Obj())
			e.writeEdge(info.vname, base, edges.ChildOf)
		}
	}
	e.emitParameters(decl.Type, sig, info)
}

// rewrittenCorpusForVName returns the new corpus that should be assigned to the
// given vname based on the OverrideStdlibCorpus and UseCompilationCorpusForAll options
func (e *emitter) rewrittenCorpusForVName(v *spb.VName) string {
	if e.opts.OverrideStdlibCorpus != "" && v.GetCorpus() == govname.GolangCorpus {
		return e.opts.OverrideStdlibCorpus
	}
	if e.opts.UseCompilationCorpusForAll {
		return e.pi.VName.GetCorpus()
	}
	if v.GetCorpus() == "" {
		// If the VName doesn't specify a corpus, use the compilation unit's corpus
		return e.pi.VName.GetCorpus()
	}
	return v.GetCorpus()
}

// emitTApp emits a tapp node and returns its VName.  The new tapp is emitted
// with given constructor and parameters.  The constructor's kind is also
// emitted if this is the first time seeing it.
func (e *emitter) emitTApp(ms *cpb.MarkedSource, ctorKind string, ctor *spb.VName, params ...*spb.VName) *spb.VName {
	if ctorKind != "" && e.pi.typeEmitted.Add(ctor.Signature) {
		e.writeFact(ctor, facts.NodeKind, ctorKind)
		if ctorKind == nodes.TBuiltin {
			e.emitBuiltinMarkedSource(ctor)
		}
	}
	components := []any{ctor}
	for _, p := range params {
		components = append(components, p)
	}
	v := &spb.VName{Language: govname.Language, Signature: hashSignature(components)}
	v.Corpus = e.rewrittenCorpusForVName(v)
	if e.pi.typeEmitted.Add(v.Signature) {
		e.writeFact(v, facts.NodeKind, nodes.TApp)
		e.writeEdge(v, ctor, edges.ParamIndex(0))
		for i, p := range params {
			e.writeEdge(v, p, edges.ParamIndex(i+1))
		}
		if ms != nil && e.opts.emitMarkedSource() {
			e.emitCode(v, ms)
		}
	}
	return v
}

// emitType emits the type as a node and returns its VName.  VNames are cached
// so the type nodes are only emitted the first time they are seen.
func (e *emitter) emitType(typ types.Type) *spb.VName {
	v, ok := e.pi.typeVName[typ]
	if ok {
		return v
	}

	switch typ := typ.(type) {
	case *types.Named:
		if typ.TypeArgs().Len() == 0 {
			v = e.pi.ObjectVName(typ.Obj())
		} else {
			// Instantiated Named types produce tapps
			ctor := e.emitType(typ.Origin())
			args := typ.TypeArgs()
			var params []*spb.VName
			for i := 0; i < args.Len(); i++ {
				params = append(params, e.emitType(args.At(i)))
			}
			v = e.emitTApp(genericTAppMS, "", ctor, params...)
		}
	case *types.Basic:
		v = govname.BasicType(typ)
		if e.pi.typeEmitted.Add(v.Signature) {
			e.writeFact(v, facts.NodeKind, nodes.TBuiltin)
			e.emitBuiltinMarkedSource(v)
		}
	case *types.Array:
		v = e.emitTApp(arrayTAppMS(typ.Len()), nodes.TBuiltin, govname.ArrayConstructorType(typ.Len()), e.emitType(typ.Elem()))
	case *types.Slice:
		v = e.emitTApp(sliceTAppMS, nodes.TBuiltin, govname.SliceConstructorType(), e.emitType(typ.Elem()))
	case *types.Pointer:
		v = e.emitTApp(pointerTAppMS, nodes.TBuiltin, govname.PointerConstructorType(), e.emitType(typ.Elem()))
	case *types.Chan:
		v = e.emitTApp(chanTAppMS(typ.Dir()), nodes.TBuiltin, govname.ChanConstructorType(typ.Dir()), e.emitType(typ.Elem()))
	case *types.Map:
		v = e.emitTApp(mapTAppMS, nodes.TBuiltin, govname.MapConstructorType(), e.emitType(typ.Key()), e.emitType(typ.Elem()))
	case *types.Tuple: // function return types
		v = e.emitTApp(tupleTAppMS, nodes.TBuiltin, govname.TupleConstructorType(), e.visitTuple(typ)...)
	case *types.Signature: // function types
		ms := &cpb.MarkedSource{
			Kind: cpb.MarkedSource_TYPE,
			Child: []*cpb.MarkedSource{{
				Kind:          cpb.MarkedSource_PARAMETER_LOOKUP_BY_PARAM,
				LookupIndex:   3,
				PreText:       "func(",
				PostChildText: ", ",
				PostText:      ")",
			}},
		}

		params := e.visitTuple(typ.Params())
		if typ.Variadic() && len(params) > 0 {
			// Convert last parameter type from slice type to variadic type.
			last := len(params) - 1
			if slice, ok := typ.Params().At(last).Type().(*types.Slice); ok {
				params[last] = e.emitTApp(variadicTAppMS, nodes.TBuiltin, govname.VariadicConstructorType(), e.emitType(slice.Elem()))
			}
		}

		var ret *spb.VName
		if typ.Results().Len() == 1 {
			ret = e.emitType(typ.Results().At(0).Type())
		} else {
			ret = e.emitType(typ.Results())
		}
		if typ.Results().Len() != 0 {
			ms.Child = append(ms.Child, &cpb.MarkedSource{
				Kind:    cpb.MarkedSource_BOX,
				PreText: " ",
				Child: []*cpb.MarkedSource{{
					Kind:        cpb.MarkedSource_LOOKUP_BY_PARAM,
					LookupIndex: 1,
				}},
			})
		}

		var recv *spb.VName
		if r := typ.Recv(); r != nil {
			recv = e.emitType(r.Type())
			ms.Child = append([]*cpb.MarkedSource{{
				Kind:     cpb.MarkedSource_BOX,
				PreText:  "(",
				PostText: ") ",
				Child: []*cpb.MarkedSource{{
					Kind:        cpb.MarkedSource_LOOKUP_BY_PARAM,
					LookupIndex: 2,
				}},
			}}, ms.Child...)
		} else {
			recv = e.emitType(types.NewTuple())
		}

		v = e.emitTApp(ms, nodes.TBuiltin, govname.FunctionConstructorType(),
			append([]*spb.VName{ret, recv}, params...)...)
	case *types.Interface:
		v = &spb.VName{Language: govname.Language, Signature: hashSignature(typ)}
		if e.pi.typeEmitted.Add(v.Signature) {
			e.writeFact(v, facts.NodeKind, nodes.Interface)
			if e.opts.emitMarkedSource() {
				e.emitCode(v, &cpb.MarkedSource{
					Kind:    cpb.MarkedSource_TYPE,
					PreText: typ.String(),
				})
			}
		}
	case *types.Struct:
		v = &spb.VName{Language: govname.Language, Signature: hashSignature(typ)}
		if e.pi.typeEmitted.Add(v.Signature) {
			e.writeFact(v, facts.NodeKind, nodes.Record)
			if e.opts.emitMarkedSource() {
				e.emitCode(v, &cpb.MarkedSource{
					Kind:    cpb.MarkedSource_TYPE,
					PreText: typ.String(),
				})
			}
		}
	case *types.TypeParam:
		v = e.pi.ObjectVName(typ.Obj())
	default:
		log.Warningf("unknown type %T: %+v", typ, typ)
	}

	e.pi.typeVName[typ] = v
	return v
}

func (e *emitter) emitTypeOf(expr ast.Expr) *spb.VName { return e.emitType(e.pi.Info.TypeOf(expr)) }

func (e *emitter) visitTuple(t *types.Tuple) []*spb.VName {
	size := t.Len()
	ts := make([]*spb.VName, size)
	for i := 0; i < size; i++ {
		ts[i] = e.emitType(t.At(i).Type())
	}
	return ts
}

// visitFuncLit handles function literals and their parameters.  The signature
// for a function literal is named relative to the signature of its parent
// function, or the file scope if the literal is at the top level.
func (e *emitter) visitFuncLit(flit *ast.FuncLit, stack stackFunc) {
	fi := e.callContext(stack)
	if fi == nil {
		panic(fmt.Sprintf("Function literal without a context: %v", flit))
	}

	fi.numAnons++
	info := &funcInfo{vname: proto.Clone(fi.vname).(*spb.VName)}
	info.vname.Language = govname.Language
	info.vname.Signature += "$" + strconv.Itoa(fi.numAnons)
	e.pi.function[flit] = info
	e.writeDef(flit, info.vname)
	e.writeFact(info.vname, facts.NodeKind, nodes.Function)

	if sig, ok := e.pi.Info.Types[flit].Type.(*types.Signature); ok {
		e.emitParameters(flit.Type, sig, info)
	}
}

// visitValueSpec handles variable and constant bindings.
func (e *emitter) visitValueSpec(spec *ast.ValueSpec, stack stackFunc) {
	kind := nodes.Variable
	if stack(1).(*ast.GenDecl).Tok == token.CONST {
		kind = nodes.Constant
	}
	doc := specComment(spec, stack)
	for _, id := range spec.Names {
		target := e.writeBinding(id, kind, e.nameContext(stack))
		if target == nil {
			continue // type error (reported elsewhere)
		}
		e.writeDoc(doc, target)
	}

	// Handle members of anonymous types declared in situ.
	if spec.Type != nil {
		e.emitAnonMembers(spec.Type)
	}
	for _, v := range spec.Values {
		if lit, ok := v.(*ast.CompositeLit); ok {
			e.emitAnonMembers(lit.Type)
		}
	}
}

// visitTypeSpec handles type declarations, including the bindings for fields
// of struct types and methods of interfaces.
func (e *emitter) visitTypeSpec(spec *ast.TypeSpec, stack stackFunc) {
	obj := e.pi.Info.Defs[spec.Name]
	if obj == nil {
		return // type error
	}
	target := e.mustWriteBinding(spec.Name, "", e.nameContext(stack))
	e.writeDef(spec, target)
	e.writeDoc(specComment(spec, stack), target)

	if e.pi.ImportPath == "builtin" {
		// Ignore everything but defs/docs in special builtin package
		return
	}

	mapFields(spec.TypeParams, func(i int, id *ast.Ident) {
		v := e.writeBinding(id, nodes.TVar, nil)
		e.writeEdge(target, v, edges.TParamIndex(i))
	})

	// Emit type-specific structure.
	switch t := obj.Type().Underlying().(type) {
	case *types.Struct:
		e.writeFact(target, facts.NodeKind, nodes.Record)
		e.writeFact(target, facts.Subkind, nodes.Struct)
		// Add parent edges for all fields, including promoted ones.
		for i, n := 0, t.NumFields(); i < n; i++ {
			e.writeEdge(e.pi.ObjectVName(t.Field(i)), target, edges.ChildOf)
		}

		// Add bindings for the explicitly-named fields in this declaration.
		// Parent edges were already added, so skip them here.
		if st, ok := spec.Type.(*ast.StructType); ok {
			mapFields(st.Fields, func(i int, id *ast.Ident) {
				target := e.writeVarBinding(id, nodes.Field, nil)
				f := st.Fields.List[i]
				e.writeDoc(firstNonEmptyComment(f.Doc, f.Comment), target)
				e.emitAnonMembers(f.Type)
			})

			// Handle anonymous fields. Such fields behave as if they were
			// named by the base identifier of their type.
			for _, field := range st.Fields.List {
				if len(field.Names) != 0 {
					continue // already handled above
				}
				id, ok := e.pi.findFieldName(field.Type)
				obj := e.pi.Info.Defs[id]
				if ok && obj != nil {
					// Don't write a fresh anchor here; we already wrote one as
					// part of the ref to the type, and we don't want duplicate
					// outputs.
					anchor := e.pi.AnchorVName(e.pi.Span(id))
					target := e.pi.ObjectVName(obj)
					e.writeEdge(anchor, target, edges.DefinesBinding)
					e.writeFact(target, facts.NodeKind, nodes.Variable)
					e.writeFact(target, facts.Subkind, nodes.Field)
					e.writeDoc(firstNonEmptyComment(field.Doc, field.Comment), target)
				}
			}
		}

	case *types.Interface:
		e.writeFact(target, facts.NodeKind, nodes.Interface)
		// Add parent edges for all methods, including inherited ones.
		for i, n := 0, t.NumMethods(); i < n; i++ {
			e.writeEdge(e.pi.ObjectVName(t.Method(i)), target, edges.ChildOf)
		}
		// Mark the interface as an extension of any embedded interfaces.
		for i, n := 0, t.NumEmbeddeds(); i < n; i++ {
			if named, ok := t.EmbeddedType(i).(*types.Named); ok {
				if eobj := named.Obj(); e.checkImplements(obj, eobj) {
					e.writeEdge(target, e.pi.ObjectVName(eobj), edges.Extends)
				}
			}
		}

		// Add bindings for the explicitly-named methods in this declaration.
		// Parent edges were already added, so skip them here.
		if it, ok := spec.Type.(*ast.InterfaceType); ok {
			mapFields(it.Methods, func(_ int, id *ast.Ident) {
				e.writeBinding(id, nodes.Function, nil)
			})
		}

	default:
		// We model a newtype form whose underlying type is not already a
		// struct (e.g., "type Foo int") as if it were a record with a single
		// unexported field of the underlying type. That is not really what Go
		// does, but it is close enough for the graph model to work. Since
		// there is no actual field declaration, however, we don't emit that.
		e.writeFact(target, facts.NodeKind, nodes.Record)
		e.writeFact(target, facts.Subkind, nodes.Type)
	}
}

// visitImportSpec handles references to imported packages.
func (e *emitter) visitImportSpec(spec *ast.ImportSpec, stack stackFunc) {
	ipath, _ := strconv.Unquote(spec.Path.Value)
	if vPath, ok := e.pi.Vendored[ipath]; ok {
		ipath = vPath
	}

	pkg := e.pi.Dependencies[ipath]
	target := e.pi.PackageVName[pkg]
	if target == nil {
		log.Warningf("Unable to resolve import path %q", ipath)
		return
	}

	e.writeRef(spec.Path, target, edges.RefImports)
	if e.opts.shouldEmit(target) && !e.pi.standardLib.Contains(ipath) {
		e.writeFact(target, facts.NodeKind, nodes.Package)
		e.pi.standardLib.Add(ipath)
	}
}

// visitAssignStmt handles bindings introduced by short-declaration syntax in
// assignment statments, e.g., "x, y := 1, 2".
func (e *emitter) visitAssignStmt(stmt *ast.AssignStmt, stack stackFunc) {
	if stmt.Tok != token.DEFINE {
		return // no new bindings in this statement
	}

	// Not all the names in a short declaration assignment may be defined here.
	// We only add bindings for newly-defined ones, of which there must be at
	// least one in a well-typed program.
	up := e.nameContext(stack)
	for _, expr := range stmt.Lhs {
		if id, _ := expr.(*ast.Ident); id != nil {
			// Add a binding only if this is the definition site for the name.
			if obj := e.pi.Info.Defs[id]; obj != nil && obj.Pos() == id.Pos() {
				e.mustWriteBinding(id, nodes.Variable, up)
			}
		}
	}

	// TODO(fromberger): Add information about initializers where available.
}

// visitRangeStmt handles the bindings introduced by a for ... range statement.
func (e *emitter) visitRangeStmt(stmt *ast.RangeStmt, stack stackFunc) {
	if stmt.Tok != token.DEFINE {
		return // no new bindings in this statement
	}

	// In a well-typed program, the key and value will always be identifiers.
	up := e.nameContext(stack)
	if key, _ := stmt.Key.(*ast.Ident); key != nil {
		e.writeBinding(key, nodes.Variable, up)
	}
	if val, _ := stmt.Value.(*ast.Ident); val != nil {
		e.writeBinding(val, nodes.Variable, up)
	}
}

// visitCompositeLit handles references introduced by positional initializers
// in composite literals that construct (pointer to) struct values. Named
// initializers are handled separately.
func (e *emitter) visitCompositeLit(expr *ast.CompositeLit, stack stackFunc) {
	if len(expr.Elts) == 0 {
		return // no fields to initialize
	}

	tv, ok := e.pi.Info.Types[expr]
	if !ok {
		log.Warningf("Unable to determine composite literal type (%s)", e.pi.FileSet.Position(expr.Pos()))
		return
	}
	sv, ok := deref(tv.Type.Underlying()).(*types.Struct)
	if !ok {
		return // non-struct type, e.g. a slice; nothing to do here
	}

	if n := sv.NumFields(); n < len(expr.Elts) {
		// Embedded struct fields from an imported package may not appear in
		// the list if the import did not succeed.  To remain robust against
		// such cases, don't try to read into the fields of a struct type if
		// the counts don't line up. The information we emit will still be
		// correct, we'll just miss some initializers.
		log.Errorf("Struct has %d fields but %d initializers (skipping)", n, len(expr.Elts))
		return
	}
	for i, elt := range expr.Elts {
		// The keys for key-value initializers are handled upstream of us, so
		// we need only handle the values. But note that key-value initializers
		// may not be in order, so we have to take care to get the right field.
		// Positional fields must be in order, in well-formed code.
		switch t := elt.(type) {
		case *ast.KeyValueExpr:
			f, ok := fieldIndex(t.Key, sv)
			if !ok {
				log.Errorf("Found no field index for %v (skipping)", t.Key)
				continue
			}
			e.emitPosRef(t.Value, sv.Field(f), edges.RefInit)
		default:
			e.emitPosRef(t, sv.Field(i), edges.RefInit)
		}
	}
}

// visitIndexExpr handles references to instantiated types with a single type
// parameter.
func (e *emitter) visitIndexExpr(expr *ast.IndexExpr, stack stackFunc) {
	if n, ok := e.pi.Info.TypeOf(expr).(*types.Named); ok && n.TypeArgs().Len() > 0 {
		e.writeRef(expr, e.emitType(n), edges.Ref)
	}
}

// visitIndexListExpr handles references to instantiated types with multiple
// type parameters.
func (e *emitter) visitIndexListExpr(expr *ast.IndexListExpr, stack stackFunc) {
	if n, ok := e.pi.Info.TypeOf(expr).(*types.Named); ok && n.TypeArgs().Len() > 0 {
		e.writeRef(expr, e.emitType(n), edges.Ref)
	}
}

// visitArrayType handles references to array types.
func (e *emitter) visitArrayType(expr *ast.ArrayType, stack stackFunc) {
	e.emitAnonMembers(expr.Elt)
}

// emitPosRef emits an anchor spanning loc, pointing to obj.
func (e *emitter) emitPosRef(loc ast.Node, obj types.Object, kind string) {
	target := e.pi.ObjectVName(obj)
	file, start, end := e.pi.Span(loc)
	anchor := e.pi.AnchorVName(file, start, end)
	e.writeAnchor(loc, anchor, start, end)
	e.writeEdge(anchor, target, kind)
}

// emitParameters emits parameter edges for the parameters of a function type,
// given the type signature and info of the enclosing declaration or function
// literal.
func (e *emitter) emitParameters(ftype *ast.FuncType, sig *types.Signature, info *funcInfo) {
	paramIndex := 0

	// If there is a receiver, it is treated as param.0.
	if sig.Recv() != nil {
		paramIndex++
	}

	// Emit bindings and parameter edges for the parameters.
	mapFields(ftype.Params, func(i int, id *ast.Ident) {
		if sig.Params().At(i) != nil {
			if param := e.writeBinding(id, nodes.Variable, info.vname); param != nil {
				e.writeEdge(info.vname, param, edges.ParamIndex(paramIndex))

				field := ftype.Params.List[i]
				e.emitAnonMembers(field.Type)

				// Field object does not associate any comments with the parameter; use CommentMap to find them
				e.writeDoc(firstNonEmptyComment(e.cmap.Filter(field).Comments()...), param)
			}
		}
		paramIndex++
	})
	// Emit bindings for any named result variables.
	// Results are not considered parameters.
	mapFields(ftype.Results, func(i int, id *ast.Ident) {
		e.writeBinding(id, nodes.Variable, info.vname)
	})
	// Emit bindings for type parameters
	mapFields(ftype.TypeParams, func(i int, id *ast.Ident) {
		v := e.writeBinding(id, nodes.TVar, nil)
		e.writeEdge(info.vname, v, edges.TParamIndex(i))
	})
}

// emitAnonMembers checks whether expr denotes an anonymous struct or interface
// type, and if so emits bindings for its member fields/methods. The resulting
// members do not parent to the type, since it has no referential identity; but
// we do capture documentation in the unlikely event someone wrote any.
func (e *emitter) emitAnonMembers(expr ast.Expr) {
	if st, ok := expr.(*ast.StructType); ok {
		mapFields(st.Fields, func(i int, id *ast.Ident) {
			target := e.writeVarBinding(id, nodes.Field, nil) // no parent
			e.writeDoc(firstNonEmptyComment(st.Fields.List[i].Doc, st.Fields.List[i].Comment), target)
		})
	} else if it, ok := expr.(*ast.InterfaceType); ok {
		mapFields(it.Methods, func(i int, id *ast.Ident) {
			target := e.writeBinding(id, nodes.Function, nil) // no parent
			e.writeDoc(firstNonEmptyComment(it.Methods.List[i].Doc, it.Methods.List[i].Comment), target)
		})
	}
}

// An override represents the relationship that x overrides y.
type override struct {
	x, y types.Object
}

// overrides represents a set of override relationships we've already generated.
type overrides map[override]bool

// seen reports whether an x overrides y was already cached, and if not adds it
// to the set.
func (o overrides) seen(x, y types.Object) bool {
	ov := override{x: x, y: y}
	ok := o[ov]
	if !ok {
		o[ov] = true
	}
	return ok
}

// emitSatisfactions visits each named type known through the compilation being
// indexed, and emits edges connecting it to any known interfaces its method
// set satisfies.
func (e *emitter) emitSatisfactions() {
	// Find all the Named types mentioned in this compilation.
	var allTypes []*types.Named

	// For the current source package, use all names, even local ones.
	for _, obj := range e.pi.Info.Defs {
		if obj, ok := obj.(*types.TypeName); ok {
			if n, ok := obj.Type().(*types.Named); ok {
				allTypes = append(allTypes, n)
			}
		}
	}

	// Include instance types.
	for _, t := range e.pi.Info.Types {
		if n, ok := t.Type.(*types.Named); ok && n.TypeArgs().Len() > 0 {
			allTypes = append(allTypes, n)
		}
	}

	// For dependencies, we only have access to package-level types, not those
	// defined by inner scopes.
	for _, pkg := range e.pi.Dependencies {
		scope := pkg.Scope()
		for _, name := range scope.Names() {
			if obj, ok := scope.Lookup(name).(*types.TypeName); ok {
				// Note that the names of some "named" types that are brought
				// in from dependencies may not be known at this point -- the
				// compiled package headers omit the names if they are not
				// needed.  Skip such cases, even though they would qualify if
				// we had the source package.
				if n, ok := obj.Type().(*types.Named); ok && obj.Name() != "" {
					allTypes = append(allTypes, n)
				}
			}
		}
	}

	// Shared Context across all generic assignability checks.
	tctx := types.NewContext()

	// Cache the method set of each named type in this package.
	var msets typeutil.MethodSetCache
	// Cache the overrides we've noticed to avoid duplicate entries.
	cache := make(overrides)
	for _, x := range allTypes {
		xobj := x.Obj()
		if xobj.Pkg() != e.pi.Package {
			continue // not from this package
		}

		// Check whether x is a named type with methods; if not, skip it.
		if len(typeutil.IntuitiveMethodSet(x, &msets)) == 0 {
			continue // no methods to consider
		}

		// N.B. This implementation is quadratic in the number of visible
		// interfaces, but that's probably OK since are only considering a
		// single compilation.

		// Check the method sets of both x and pointer-to-x for overrides.
		xmset := msets.MethodSet(xobj.Type())
		pxmset := msets.MethodSet(types.NewPointer(xobj.Type()))

		for _, y := range allTypes {
			yobj := y.Obj()
			if xobj == yobj {
				continue
			}

			ymset := msets.MethodSet(yobj.Type())

			ifx, ify := isInterface(x), isInterface(y)
			switch {
			case ifx && ify && ymset.Len() > 0:
				// x and y are both interfaces. Note that extension is handled
				// elsewhere as part of the type spec for the interface.
				if assignableTo(tctx, x, y) {
					e.writeSatisfies(xobj, yobj)
				}
				if assignableTo(tctx, y, x) {
					e.writeSatisfies(yobj, xobj)
				}

			case ifx:
				// y is a concrete type
				pymset := msets.MethodSet(types.NewPointer(y))
				if assignableTo(tctx, y, x) {
					e.writeSatisfies(yobj, xobj)
					e.emitOverrides(ymset, pymset, xmset, cache)
				}

			case ify && ymset.Len() > 0:
				// x is a concrete type
				if assignableTo(tctx, x, y) {
					e.writeSatisfies(xobj, yobj)
					e.emitOverrides(xmset, pxmset, ymset, cache)
				}

			default:
				// Both x and y are concrete.
			}
		}
	}
}

// Add xm-(overrides)-ym for each concrete method xm with a corresponding
// abstract method ym.
func (e *emitter) emitOverrides(xmset, pxmset, ymset *types.MethodSet, cache overrides) {
	for i, n := 0, ymset.Len(); i < n; i++ {
		ym := ymset.At(i)
		yobj := ym.Obj()
		xm := xmset.Lookup(yobj.Pkg(), yobj.Name())
		if xm == nil {
			if pxmset != nil {
				xm = pxmset.Lookup(yobj.Pkg(), yobj.Name())
			}
			if xm == nil {
				continue // this method is not part of the interface we're probing
			}
		}

		xobj := xm.Obj()
		if cache.seen(xobj, yobj) {
			continue
		}

		xvname := e.pi.ObjectVName(xobj)
		yvname := e.pi.ObjectVName(yobj)
		if e.pi.typeEmitted.Add(xvname.Signature + "+" + yvname.Signature) {
			e.writeEdge(xvname, yvname, edges.Overrides)
		}

		xt := e.emitType(xobj.Type())
		yt := e.emitType(yobj.Type())
		if e.pi.typeEmitted.Add(xt.Signature + "+" + yt.Signature) {
			e.writeEdge(xt, yt, edges.Satisfies)
		}
	}
}

func isInterface(typ types.Type) bool { _, ok := typ.Underlying().(*types.Interface); return ok }

func (e *emitter) check(err error) {
	if err != nil && e.firstErr == nil {
		e.firstErr = err
		log.Errorf("indexing %q: %v", e.pi.ImportPath, err)
	}
}

func (e *emitter) checkImplements(src, tgt types.Object) bool {
	i := impl{A: src, B: tgt}
	if _, ok := e.impl[i]; ok {
		return false
	}
	e.impl[i] = struct{}{}
	return true
}

func (e *emitter) writeSatisfies(src, tgt types.Object) {
	if e.checkImplements(src, tgt) {
		e.writeEdge(e.pi.ObjectVName(src), e.pi.ObjectVName(tgt), edges.Satisfies)
	}
}

func (e *emitter) writeFact(src *spb.VName, name, value string) {
	if corpus := e.rewrittenCorpusForVName(src); corpus != src.GetCorpus() {
		src = proto.Clone(src).(*spb.VName)
		src.Corpus = corpus
	}
	e.check(e.sink.writeFact(e.ctx, src, name, value))
}

func (e *emitter) writeEdge(src, tgt *spb.VName, kind string) {
	if corpus := e.rewrittenCorpusForVName(src); corpus != src.GetCorpus() {
		src = proto.Clone(src).(*spb.VName)
		src.Corpus = corpus
	}
	if corpus := e.rewrittenCorpusForVName(tgt); corpus != tgt.GetCorpus() {
		tgt = proto.Clone(tgt).(*spb.VName)
		tgt.Corpus = corpus
	}
	e.check(e.sink.writeEdge(e.ctx, src, tgt, kind))
}

func (e *emitter) writeAnchor(node ast.Node, src *spb.VName, start, end int) {
	if corpus := e.rewrittenCorpusForVName(src); corpus != src.GetCorpus() {
		src = proto.Clone(src).(*spb.VName)
		src.Corpus = corpus
	}
	if _, ok := e.anchored[node]; ok {
		return // this node already has an anchor
	}
	e.anchored[node] = struct{}{}
	e.check(e.sink.writeAnchor(e.ctx, src, start, end))
}

func (e *emitter) writeDiagnostic(src *spb.VName, d diagnostic) {
	if corpus := e.rewrittenCorpusForVName(src); corpus != src.GetCorpus() {
		src = proto.Clone(src).(*spb.VName)
		src.Corpus = corpus
	}
	e.check(e.sink.writeDiagnostic(e.ctx, src, d))
}

func (e *emitter) writeNodeDiagnostic(src ast.Node, d diagnostic) {
	file, start, end := e.pi.Span(src)
	anchor := e.pi.AnchorVName(file, start, end)
	e.writeAnchor(src, anchor, start, end)
	e.writeDiagnostic(anchor, d)
}

// writeRef emits an anchor spanning origin and referring to target with an
// edge of the given kind. The vname of the anchor is returned.
func (e *emitter) writeRef(origin ast.Node, target *spb.VName, kind string) *spb.VName {
	file, start, end := e.pi.Span(origin)
	anchor := e.pi.AnchorVName(file, start, end)
	e.writeAnchor(origin, anchor, start, end)
	e.writeEdge(anchor, target, kind)

	// Check whether we are intended to emit metadata linkage edges, and if so,
	// whether there are any to process.
	e.applyRules(file, start, end, kind, func(rule metadata.Rule) {
		if rule.Reverse {
			e.writeEdge(rule.VName, target, rule.EdgeOut)
		} else {
			e.writeEdge(target, rule.VName, rule.EdgeOut)
		}
		if rule.Semantic != nil {
			e.writeFact(target, facts.SemanticGenerated, strings.ToLower(rule.Semantic.String()))
		}
		if rule.EdgeOut == edges.Generates && !e.fmeta[file] {
			e.fmeta[file] = true
			if rule.VName.Path != "" && target.Path != "" {
				ruleVName := narrowToFileVName(rule.VName)
				fileTarget := narrowToFileVName(anchor)
				if rule.Reverse {
					e.writeEdge(ruleVName, fileTarget, rule.EdgeOut)
				} else {
					e.writeEdge(fileTarget, ruleVName, rule.EdgeOut)
				}
			}
		}
	})

	return anchor
}

func narrowToFileVName(v *spb.VName) *spb.VName {
	return &spb.VName{Corpus: v.GetCorpus(), Root: v.GetRoot(), Path: v.GetPath()}
}

// mustWriteBinding is as writeBinding, but panics if id does not resolve.  Use
// this in cases where the object is known already to exist.
func (e *emitter) mustWriteBinding(id *ast.Ident, kind string, parent *spb.VName) *spb.VName {
	if target := e.writeBinding(id, kind, parent); target != nil {
		return target
	}
	panic("unresolved definition") // logged in writeBinding
}

// writeVarBinding is as writeBinding, assuming the kind is "variable".
// If subkind != "", it is also emitted as a subkind.
func (e *emitter) writeVarBinding(id *ast.Ident, subkind string, parent *spb.VName) *spb.VName {
	vname := e.writeBinding(id, nodes.Variable, parent)
	if vname != nil && subkind != "" {
		e.writeFact(vname, facts.Subkind, subkind)
	}
	return vname
}

// writeBinding emits a node of the specified kind for the target of id.  If
// the identifier is not "_", an anchor for a binding definition of the target
// is also emitted at id. If parent != nil, the target is also recorded as its
// child. The target vname is returned.
func (e *emitter) writeBinding(id *ast.Ident, kind string, parent *spb.VName) *spb.VName {
	obj := e.pi.Info.Defs[id]
	if obj == nil {
		loc := e.pi.FileSet.Position(id.Pos())
		log.Errorf("Missing definition for id %q at %s", id.Name, loc)
		return nil
	}
	target := e.pi.ObjectVName(obj)
	if e.pi.ImportPath == "builtin" && parent != nil && (parent.GetSignature() == "package" || parent.GetSignature() == "") {
		// Special-case top-level builtin bindings: https://pkg.go.dev/builtin
		target = govname.Builtin(id.String())
		kind = "tbuiltin"
	}
	if kind != "" {
		e.writeFact(target, facts.NodeKind, kind)
	}
	if id.Name != "_" {
		e.writeRef(id, target, edges.DefinesBinding)
	}
	if parent != nil {
		e.writeEdge(target, parent, edges.ChildOf)
	}
	if e.opts.emitMarkedSource() {
		e.emitCode(target, e.pi.MarkedSource(obj))
	}
	e.writeEdge(target, e.emitTypeOf(id), edges.Typed)
	return target
}

// writeDef emits a spanning anchor and defines edge for the specified node.
// This function does not create the target node.
func (e *emitter) writeDef(node ast.Node, target *spb.VName) {
	e.writeRef(node, target, edges.Defines)
}

// writeDoc adds associations between comment groups and a documented node.
// It also handles marking deprecated facts on the target.
func (e *emitter) writeDoc(comments *ast.CommentGroup, target *spb.VName) {
	if comments == nil || len(comments.List) == 0 || target == nil {
		return
	}
	var lines []string
	for _, comment := range comments.List {
		lines = append(lines, trimComment(comment.Text))
	}
	docNode := proto.Clone(target).(*spb.VName)
	docNode.Signature += " doc"
	e.writeFact(docNode, facts.NodeKind, nodes.Doc)
	e.writeFact(docNode, facts.Text, escComment.Replace(strings.Join(lines, "\n")))
	e.writeEdge(docNode, target, edges.Documents)
	e.emitDeprecation(target, lines)
}

// emitDeprecation emits a deprecated fact for the specified target if the
// comment lines indicate it is deprecated per https://github.com/golang/go/wiki/Deprecated
func (e *emitter) emitDeprecation(target *spb.VName, lines []string) {
	var deplines []string
	for _, line := range lines {
		if len(deplines) == 0 {
			if msg := strings.TrimPrefix(line, "Deprecated:"); msg != line {
				deplines = append(deplines, strings.TrimSpace(msg))
			}
		} else if line == "" {
			break
		} else {
			deplines = append(deplines, strings.TrimSpace(line))
		}
	}
	if len(deplines) > 0 {
		e.writeFact(target, facts.Deprecated, strings.Join(deplines, " "))
	}
}

// isCall reports whether id is a call to obj.  This holds if id is in call
// position ("id(...") or is the RHS of a selector in call position
// ("x.id(...)"). If so, the nearest enclosing call expression is also
// returned.
//
// This will not match if there are redundant parentheses in the expression.
func isCall(id *ast.Ident, obj types.Object, stack stackFunc) (*ast.CallExpr, bool) {
	if _, ok := obj.(*types.Func); ok {
		if call, ok := stack(1).(*ast.CallExpr); ok && call.Fun == id {
			return call, true // id(...)
		}
		if sel, ok := stack(1).(*ast.SelectorExpr); ok && sel.Sel == id {
			if call, ok := stack(2).(*ast.CallExpr); ok && call.Fun == sel {
				return call, true // x.id(...)
			}
		}
	}
	return nil, false
}

// callContext returns funcInfo for the nearest enclosing parent function, not
// including the node itself, or the enclosing package initializer if the node
// is at the top level.
func (e *emitter) callContext(stack stackFunc) *funcInfo {
	for i := 1; ; i++ {
		switch p := stack(i).(type) {
		case *ast.FuncDecl, *ast.FuncLit:
			return e.pi.function[p]
		case *ast.File:
			if e.opts.useFileAsTopLevelScope() {
				return &funcInfo{vname: e.pi.FileVName(p)}
			}
			fi := e.pi.packageInit[p]
			if fi == nil {
				// Lazily emit a virtual node to represent the static
				// initializer for top-level expressions in this file of the
				// package.  We only do this if there are expressions that need
				// to be initialized.
				vname := proto.Clone(e.pi.VName).(*spb.VName)
				vname.Signature += fmt.Sprintf(".<init>@%d", p.Package)
				fi = &funcInfo{vname: vname}
				e.pi.packageInit[p] = fi
				e.writeFact(vname, facts.NodeKind, nodes.Function)
				e.writeEdge(vname, e.pi.VName, edges.ChildOf)

				// The callgraph requires we provide the caller with a
				// definition (http://www.kythe.io/docs/schema/callgraph.html).
				// Since there is no location, attach it to the beginning of
				// the file itself.
				anchor := e.pi.AnchorVName(p, 0, 0)
				e.check(e.sink.writeAnchor(e.ctx, anchor, 0, 0))
				e.writeEdge(anchor, vname, edges.Defines)
			}
			return fi
		}
	}
}

// nameContext returns the vname for the nearest enclosing parent node, not
// including the node itself, or the enclosing package vname if the node is at
// the top level.
func (e *emitter) nameContext(stack stackFunc) *spb.VName {
	if fi := e.callContext(stack); !e.pi.isPackageInit(fi) {
		return fi.vname
	}
	return e.pi.VName
}

// applyRules calls apply for each metadata rule matching the given combination
// of location and kind.
func (e *emitter) applyRules(file *ast.File, start, end int, kind string, apply func(r metadata.Rule)) {
	if e.opts == nil || !e.opts.EmitLinkages {
		return // nothing to do
	} else if e.rmap == nil {
		e.rmap = make(map[*ast.File]map[int]metadata.Rules)
	}

	// Lazily populate a cache of file :: start :: rules mappings, so that we
	// need only scan the rules coincident on the starting point of the range
	// we care about. In almost all cases that will be just one, if any.
	rules, ok := e.rmap[file]
	if !ok {
		rules = make(map[int]metadata.Rules)
		for _, rule := range e.pi.Rules[file] {
			rules[rule.Begin] = append(rules[rule.Begin], rule)
		}
		e.rmap[file] = rules
	}

	for _, rule := range rules[start] {
		if rule.End == end && rule.EdgeIn == kind {
			apply(rule)
		}
	}
}

// A visitFunc visits a node of the Go AST. The function can use stack to
// retrieve AST nodes on the path from the node up to the root.  If the return
// value is true, the children of node are also visited; otherwise they are
// skipped.
type visitFunc func(node ast.Node, stack stackFunc) bool

// A stackFunc returns the ith stack entry above of an AST node, where 0
// denotes the node itself. If the ith entry does not exist, the function
// returns nil.
type stackFunc func(i int) ast.Node

// astVisitor implements ast.Visitor, passing each visited node to a callback
// function.
type astVisitor struct {
	stack []ast.Node
	visit visitFunc
}

func newASTVisitor(f visitFunc) ast.Visitor { return &astVisitor{visit: f} }

// Visit implements the required method of the ast.Visitor interface.
func (w *astVisitor) Visit(node ast.Node) ast.Visitor {
	if node == nil {
		w.stack = w.stack[:len(w.stack)-1] // pop
		return w
	}

	w.stack = append(w.stack, node) // push
	if !w.visit(node, w.parent) {
		return nil
	}
	return w
}

func (w *astVisitor) parent(i int) ast.Node {
	if i >= len(w.stack) {
		return nil
	}
	return w.stack[len(w.stack)-1-i]
}

// deref returns the base type of T if it is a pointer, otherwise T itself.
func deref(T types.Type) types.Type {
	if U, ok := T.Underlying().(*types.Pointer); ok {
		return U.Elem()
	}
	return T
}

// mapFields applies f to each identifier declared in fields.  Each call to f
// is given the offset and the identifier.
func mapFields(fields *ast.FieldList, f func(i int, id *ast.Ident)) {
	if fields == nil {
		return
	}
	for i, field := range fields.List {
		for _, id := range field.Names {
			f(i, id)
		}
	}
}

// fieldIndex reports whether sv has a field named by expr, which must be of
// type *ast.Ident, and returns its positional index if so.
//
// N.B. This is a linear scan, but the count of fields should almost always be
// small enough not to worry about it.
func fieldIndex(expr ast.Expr, sv *types.Struct) (int, bool) {
	want := expr.(*ast.Ident).Name
	for i := 0; i < sv.NumFields(); i++ {
		if sv.Field(i).Name() == want {
			return i, true
		}
	}
	return -1, false
}

var escComment = strings.NewReplacer("[", `\[`, "]", `\]`, `\`, `\\`)

// trimComment removes the comment delimiters from a comment.  For single-line
// comments, it also removes a single leading space, if present; for multi-line
// comments it discards leading and trailing whitespace.
func trimComment(text string) string {
	if single := strings.TrimPrefix(text, "//"); single != text {
		return strings.TrimPrefix(single, " ")
	}
	return strings.TrimSpace(strings.TrimSuffix(strings.TrimPrefix(text, "/*"), "*/"))
}

// specComment returns the innermost comment associated with spec, or nil.
func specComment(spec ast.Spec, stack stackFunc) *ast.CommentGroup {
	var comment *ast.CommentGroup
	switch t := spec.(type) {
	case *ast.TypeSpec:
		comment = firstNonEmptyComment(t.Doc, t.Comment)
	case *ast.ValueSpec:
		comment = firstNonEmptyComment(t.Doc, t.Comment)
	case *ast.ImportSpec:
		comment = firstNonEmptyComment(t.Doc, t.Comment)
	}
	if comment == nil {
		if t, ok := stack(1).(*ast.GenDecl); ok {
			return t.Doc
		}
	}
	return comment
}

func firstNonEmptyComment(cs ...*ast.CommentGroup) *ast.CommentGroup {
	for _, c := range cs {
		if c != nil && len(c.List) > 0 {
			return c
		}
	}
	return nil
}

func canBeAssignableTo(v, t types.Type) bool {
	return types.AssignableTo(v, t) || types.AssignableTo(types.NewPointer(v), t)
}

func assignableTo(tctx *types.Context, V, T types.Type) bool {
	// If V and T are not both named, or do not have matching non-empty type
	// parameter lists, fall back on types.AssignableTo.
	VN, Vnamed := V.(*types.Named)
	TN, Tnamed := T.(*types.Named)
	if !Vnamed || !Tnamed {
		return canBeAssignableTo(V, T)
	}

	vtparams := VN.TypeParams()
	ttparams := TN.TypeParams()
	if vtparams.Len() == 0 || vtparams.Len() != ttparams.Len() || VN.TypeArgs().Len() != 0 || TN.TypeArgs().Len() != 0 {
		return canBeAssignableTo(V, T)
	}

	// V and T have the same (non-zero) number of type params. Instantiate both
	// with the type parameters of V. This must always succeed for V, and will
	// succeed for T if and only if the type set of each type parameter of V is a
	// subset of the type set of the corresponding type parameter of T, meaning
	// that every instantiation of V corresponds to a valid instantiation of T.

	targs := make([]types.Type, vtparams.Len())
	for i := 0; i < vtparams.Len(); i++ {
		targs[i] = vtparams.At(i)
	}

	vinst, err := types.Instantiate(tctx, V, targs, true)
	if err != nil {
		log.Errorf("type parameters should satisfy their own constraints: %v", err)
		return false
	}

	tinst, err := types.Instantiate(tctx, T, targs, true)
	if err != nil {
		return false
	}

	return canBeAssignableTo(vinst, tinst)
}
