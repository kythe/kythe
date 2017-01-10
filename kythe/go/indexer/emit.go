/*
 * Copyright 2016 Google Inc. All rights reserved.
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
	"log"
	"strconv"
	"strings"

	"github.com/golang/protobuf/proto"

	"kythe.io/kythe/go/extractors/govname"
	"kythe.io/kythe/go/util/schema/edges"
	"kythe.io/kythe/go/util/schema/facts"
	"kythe.io/kythe/go/util/schema/nodes"

	spb "kythe.io/kythe/proto/storage_proto"
)

// Emit generates Kythe facts and edges to represent pi, and writes them to
// sink. In case of errors, processing continues as far as possible before the
// first error encountered is reported.
func (pi *PackageInfo) Emit(ctx context.Context, sink Sink) error {
	e := &emitter{
		ctx:  ctx,
		pi:   pi,
		sink: sink,
	}

	// Emit a node to represent the package as a whole.
	e.writeFact(pi.VName, facts.NodeKind, nodes.Package)

	// Emit facts for all the source files claimed by this package.
	for path, text := range pi.SourceText {
		vname := pi.FileVName(path)
		e.writeFact(vname, facts.NodeKind, nodes.File)
		e.writeFact(vname, facts.Text, text)
		// All Go source files are encoded as UTF-8, which is the default.

		// TODO(fromberger): Update the schema to record that files are childof
		// the package that contains them.
		e.writeEdge(vname, pi.VName, edges.ChildOf)
	}

	// Traverse the AST of each file in the package for xref entries.
	for _, file := range pi.Files {
		e.writeDoc(file.Doc, pi.VName) // capture package comments
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
			}
			return true
		}), file)
	}

	// TODO(fromberger): Add diagnostics for type-checker errors.
	for _, err := range pi.Errors {
		log.Printf("WARNING: Type resolution error: %v", err)
	}
	return e.firstErr
}

type emitter struct {
	ctx      context.Context
	pi       *PackageInfo
	sink     Sink
	firstErr error
}

// visitIdent handles referring identifiers. Declaring identifiers are handled
// as part of their parent syntax.
func (e *emitter) visitIdent(id *ast.Ident, stack stackFunc) {
	obj := e.pi.Info.Uses[id]
	if obj == nil {
		// Defining identifiers are handled by their parent nodes.
		return
	}

	target := e.pi.ObjectVName(obj)
	e.writeRef(id, target, edges.Ref)
	if call, ok := isCall(id, obj, stack); ok {
		callAnchor := e.writeRef(call, target, edges.RefCall)

		// Paint an edge to the function blamed for the call, or if there is
		// none then to the package initializer.
		e.writeEdge(callAnchor, e.callContext(stack).vname, edges.ChildOf)
	}
}

// visitFuncDecl handles function and method declarations and their parameters.
func (e *emitter) visitFuncDecl(decl *ast.FuncDecl, stack stackFunc) {
	info := new(funcInfo)
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

	info.vname = e.writeBinding(decl.Name, nodes.Function, nil)
	e.writeDef(decl, info.vname)
	e.writeDoc(decl.Doc, info.vname)

	// For concrete methods: Emit the receiver if named, and connect the method
	// to its declaring type.
	sig := obj.Type().(*types.Signature)
	if sig.Recv() != nil {
		// The receiver is treated as parameter 0.
		if names := decl.Recv.List[0].Names; names != nil {
			recv := e.writeBinding(names[0], nodes.Variable, info.vname)
			e.writeEdge(info.vname, recv, edges.ParamIndex(0))
		}

		// The method should be a child of its (named) enclosing type.
		if named, _ := deref(sig.Recv().Type()).(*types.Named); named != nil {
			base := e.pi.ObjectVName(named.Obj())
			e.writeEdge(info.vname, base, edges.ChildOf)
		}
	}
	e.emitParameters(decl.Type, sig, info)
}

// visitFuncLit handles function literals and their parameters.  The signature
// for a function literal is named relative to the signature of its parent
// function, or the file scope if the literal is at the top level.
func (e *emitter) visitFuncLit(flit *ast.FuncLit, stack stackFunc) {
	fi := e.callContext(stack)
	if fi == nil {
		log.Panic("Function literal without a context: ", flit)
	}

	fi.numAnons++
	info := &funcInfo{vname: proto.Clone(fi.vname).(*spb.VName)}
	info.vname.Language = govname.Language
	info.vname.Signature += fmt.Sprintf("$%d", fi.numAnons)
	e.pi.function[flit] = info
	e.writeDef(flit, info.vname)

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
		if e.pi.Info.Defs[id] == nil {
			continue // type error (reported elsewhere)
		}
		target := e.writeBinding(id, kind, e.nameContext(stack))
		e.writeDoc(doc, target)
	}
}

// visitTypeSpec handles type declarations, including the bindings for fields
// of struct types and methods of interfaces.
func (e *emitter) visitTypeSpec(spec *ast.TypeSpec, stack stackFunc) {
	obj, _ := e.pi.Info.Defs[spec.Name]
	if obj == nil {
		return // type error
	}
	target := e.writeBinding(spec.Name, "", e.nameContext(stack))
	e.writeDef(spec, target)
	e.writeDoc(specComment(spec, stack), target)

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
		mapFields(spec.Type.(*ast.StructType).Fields, func(_ int, id *ast.Ident) {
			e.writeBinding(id, nodes.Variable, nil)
		})
		// TODO(fromberger): Add bindings for anonymous fields. This will need
		// to account for pointers and qualified identifiers.

	case *types.Interface:
		e.writeFact(target, facts.NodeKind, nodes.Interface)
		// Add parent edges for all methods, including inherited ones.
		for i, n := 0, t.NumMethods(); i < n; i++ {
			e.writeEdge(e.pi.ObjectVName(t.Method(i)), target, edges.ChildOf)
		}
		// Mark the interface as an extension of any embedded interfaces.
		for i, n := 0, t.NumEmbeddeds(); i < n; i++ {
			e.writeEdge(target, e.pi.ObjectVName(t.Embedded(i).Obj()), edges.Extends)
		}

		// Add bindings for the explicitly-named methods in this declaration.
		// Parent edges were already added, so skip them here.
		mapFields(spec.Type.(*ast.InterfaceType).Methods, func(_ int, id *ast.Ident) {
			e.writeBinding(id, nodes.Function, nil)
		})

	default:
		e.writeFact(target, facts.NodeKind, nodes.TApp)
		// TODO(fromberger): Handle pointer types, newtype forms.
	}
}

// visitImportSpec handles references to imported packages.
func (e *emitter) visitImportSpec(spec *ast.ImportSpec, stack stackFunc) {
	var (
		ipath, _ = strconv.Unquote(spec.Path.Value)
		pkg      = e.pi.Dependencies[ipath]
		target   = e.pi.PackageVName[pkg]
	)
	if target == nil {
		log.Printf("Unable to resolving import path %q", ipath)
		return
	}

	e.writeRef(spec.Path, target, edges.RefImports)
	// TODO(fromberger): Lazily emit nodes for standard library packages.
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
				e.writeBinding(id, nodes.Variable, up)
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
			param := e.writeBinding(id, nodes.Variable, info.vname)
			e.writeEdge(info.vname, param, edges.ParamIndex(paramIndex))
		}
		paramIndex++
	})
	// Emit bindings for any named result variables.
	// Results are not considered parameters.
	mapFields(ftype.Results, func(i int, id *ast.Ident) {
		e.writeBinding(id, nodes.Variable, info.vname)
	})
}

func (e *emitter) check(err error) {
	if err != nil && e.firstErr == nil {
		e.firstErr = err
		log.Printf("ERROR indexing %q: %v", e.pi.ImportPath, err)
	}
}

func (e *emitter) writeFact(src *spb.VName, name, value string) {
	e.check(e.sink.writeFact(e.ctx, src, name, value))
}

func (e *emitter) writeEdge(src, tgt *spb.VName, kind string) {
	e.check(e.sink.writeEdge(e.ctx, src, tgt, kind))
}

func (e *emitter) writeAnchor(src *spb.VName, start, end int) {
	e.check(e.sink.writeAnchor(e.ctx, src, start, end))
}

// writeRef emits an anchor spanning origin and referring to target with an
// edge of the given kind. The vname of the anchor is returned.
func (e *emitter) writeRef(origin ast.Node, target *spb.VName, kind string) *spb.VName {
	anchor, start, end := e.pi.Anchor(origin)
	e.writeAnchor(anchor, start, end)
	e.writeEdge(anchor, target, kind)
	return anchor
}

// writeBinding emits a node of the specified kind for the target of id.  If
// the identifier is not "_", an anchor for a binding definition of the target
// is also emitted at id. If parent != nil, the target is also recorded as its
// child. The target vname is returned.
func (e *emitter) writeBinding(id *ast.Ident, kind string, parent *spb.VName) *spb.VName {
	target := e.pi.ObjectVName(e.pi.Info.Defs[id])
	if kind != "" {
		e.writeFact(target, facts.NodeKind, kind)
	}
	if id.Name != "_" {
		e.writeRef(id, target, edges.DefinesBinding)
	}
	if parent != nil {
		e.writeEdge(target, parent, edges.ChildOf)
	}
	return target
}

// writeDef emits a spanning anchor and defines edge for the specified node.
// This function does not create the target node.
func (e *emitter) writeDef(node ast.Node, target *spb.VName) { e.writeRef(node, target, edges.Defines) }

// writeDoc adds associations between comment groups and a documented node.
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
	e.writeFact(docNode, facts.Text, strings.Join(lines, "\n"))
	e.writeEdge(docNode, target, edges.Documents)
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
		case nil:
			if e.pi.packageInit == nil {
				// Lazily emit a virtual node to represent the static
				// initializer for top-level expressions in the package.  We
				// only do this if there are expressions that need to be
				// initialized.
				vname := proto.Clone(e.pi.VName).(*spb.VName)
				vname.Signature += ".<init>"
				e.pi.packageInit = &funcInfo{vname: vname}
				e.writeFact(vname, facts.NodeKind, nodes.Function)
				e.writeEdge(vname, e.pi.VName, edges.ChildOf)
			}
			return e.pi.packageInit
		}
	}
}

// nameContext returns the vname for the nearest enclosing parent node, not
// including the node itself, or the enclosing package vname if the node is at
// the top level.
func (e *emitter) nameContext(stack stackFunc) *spb.VName {
	if fi := e.callContext(stack); fi != e.pi.packageInit {
		return fi.vname
	}
	return e.pi.VName
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
		comment = t.Doc
	case *ast.ValueSpec:
		comment = t.Doc
	case *ast.ImportSpec:
		comment = t.Doc
	}
	if comment == nil {
		if t, ok := stack(1).(*ast.GenDecl); ok {
			return t.Doc
		}
	}
	return comment
}
