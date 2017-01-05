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
	"go/types"
	"log"

	"github.com/golang/protobuf/proto"

	"kythe.io/kythe/go/extractors/govname"
	"kythe.io/kythe/go/util/schema/edges"
	"kythe.io/kythe/go/util/schema/facts"
	"kythe.io/kythe/go/util/schema/nodes"

	spb "kythe.io/kythe/proto/storage_proto"
)

// TODO(fromberger): Maybe blame calls at file scope on a dummy package
// initializer function node, instead of on the file.
//
// TODO(fromberger): Should function literals be childof their containing
// function, when one exists?

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
		ast.Walk(newASTVisitor(func(node ast.Node, parent parentFunc) bool {
			switch n := node.(type) {
			case *ast.Ident:
				e.visitIdent(n, parent)
			case *ast.FuncDecl:
				e.visitFuncDecl(n, parent)
			case *ast.FuncLit:
				e.visitFuncLit(n, parent)
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
func (e *emitter) visitIdent(id *ast.Ident, parent parentFunc) {
	obj := e.pi.Info.Uses[id]
	if obj == nil {
		// Defining identifiers are handled by their parent nodes.
		return
	}

	refAnchor, start, end := e.pi.Anchor(id)
	e.writeAnchor(refAnchor, start, end)
	target := e.pi.ObjectVName(obj)
	e.writeEdge(refAnchor, target, edges.Ref)

	if call, ok := isCall(id, obj, parent); ok {
		callAnchor, start, end := e.pi.Anchor(call)
		e.writeAnchor(callAnchor, start, end)
		e.writeEdge(callAnchor, target, edges.RefCall)

		// Paint an edge to the function blamed for the call, or if there is
		// none then to the enclosing file.
		if fi := e.callContext(parent); fi != nil {
			e.writeEdge(callAnchor, fi.vname, edges.ChildOf)
		}
	}
}

// visitFuncDecl handles function and method declarations and their parameters.
func (e *emitter) visitFuncDecl(decl *ast.FuncDecl, parent parentFunc) {
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

	info.vname = e.pi.ObjectVName(obj)
	defAnchor, start, end := e.pi.Anchor(decl.Name)
	e.writeAnchor(defAnchor, start, end)
	e.writeEdge(defAnchor, info.vname, edges.DefinesBinding)
	fullAnchor, start, end := e.pi.Anchor(decl)
	e.writeAnchor(fullAnchor, start, end)
	e.writeEdge(fullAnchor, info.vname, edges.Defines)
	e.writeFact(info.vname, facts.NodeKind, nodes.Function)

	// For concrete methods: Emit the receiver if named, and connect the method
	// to its declaring type.
	sig := obj.Type().(*types.Signature)
	if sig.Recv() != nil {
		// The receiver is treated as parameter 0.
		if names := decl.Recv.List[0].Names; names != nil {
			recv := e.pi.ObjectVName(e.pi.Info.Defs[names[0]])
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
func (e *emitter) visitFuncLit(flit *ast.FuncLit, parent parentFunc) {
	fi := e.callContext(parent)
	if fi == nil {
		log.Panic("Function literal without a context: ", flit)
	}

	fi.numAnons++
	info := &funcInfo{vname: proto.Clone(fi.vname).(*spb.VName)}
	info.vname.Language = govname.Language
	info.vname.Signature += fmt.Sprintf("$%d", fi.numAnons)
	e.pi.function[flit] = info

	flitAnchor, start, end := e.pi.Anchor(flit)
	e.writeAnchor(flitAnchor, start, end)
	e.writeEdge(flitAnchor, info.vname, edges.Defines)

	sig := e.pi.Info.Types[flit].Type.(*types.Signature)
	e.emitParameters(flit.Type, sig, info)
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

	mapFields(ftype.Params, func(i int, id *ast.Ident) {
		if obj := sig.Params().At(i); obj != nil {
			param := e.pi.ObjectVName(obj)
			e.writeEdge(info.vname, param, edges.ParamIndex(paramIndex))
		}
		paramIndex++
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

// isCall reports whether id is a call to obj.  This holds if id is in call
// position ("id(...") or is the RHS of a selector in call position
// ("x.id(...)"). If so, the nearest enclosing call expression is also
// returned.
//
// This will not match if there are redundant parentheses in the expression.
func isCall(id *ast.Ident, obj types.Object, parent parentFunc) (*ast.CallExpr, bool) {
	if _, ok := obj.(*types.Func); ok {
		if call, ok := parent(1).(*ast.CallExpr); ok && call.Fun == id {
			return call, true // id(...)
		}
		if sel, ok := parent(1).(*ast.SelectorExpr); ok && sel.Sel == id {
			if call, ok := parent(2).(*ast.CallExpr); ok && call.Fun == sel {
				return call, true // x.id(...)
			}
		}
	}
	return nil, false
}

// callContext returns funcInfo for the nearest enclosing parent function, not
// including the node itself, or the enclosing package if the node is at the
// top level.
func (e *emitter) callContext(parent parentFunc) *funcInfo {
	for i := 1; ; i++ {
		switch p := parent(i).(type) {
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

// A visitFunc visits a node of the Go AST. The function can use parent to
// retrieve AST nodes on the path from the root up to and including node.  If
// the return value is true, the children of node are also visited; otherwise
// they are skipped.
type visitFunc func(node ast.Node, parent parentFunc) bool

// A parentFunc returns the ith parent of an AST node, where 0 denotes the node
// itself. If the ith parent does not exist, the function returns nil.
type parentFunc func(i int) ast.Node

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
