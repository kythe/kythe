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
	"go/ast"
	"go/types"
	"log"

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

	// Emit facts for all the source files claimed by this package.
	for path, text := range pi.SourceText {
		vname := pi.FileVName(path)
		e.writeFact(vname, facts.NodeKind, nodes.File)
		e.writeFact(vname, facts.Text, text)
		// All Go source files are encoded as UTF-8, which is the default.
	}

	// Traverse the AST of each file in the package for xref entries.
	for _, file := range pi.Files {
		ast.Walk(newASTVisitor(func(node ast.Node, parent parentFunc) bool {
			switch n := node.(type) {
			case *ast.Ident:
				e.visitIdent(n, parent)
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
	}

	// TODO(fromberger): If there is an enclosing context, blame the reference
	// on it.
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
