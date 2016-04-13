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

package indexer

import (
	"crypto/sha256"
	"encoding/hex"
	"go/ast"
	"go/token"
	"io/ioutil"
	"os"
	"path/filepath"
	"testing"

	"github.com/golang/protobuf/proto"
	"golang.org/x/net/context"

	apb "kythe.io/kythe/proto/analysis_proto"
	spb "kythe.io/kythe/proto/storage_proto"
)

var _ spb.VName
var _ apb.CompilationUnit

type memFetcher map[string]string // :: digest → content

func (m memFetcher) Fetch(path, digest string) ([]byte, error) {
	if s, ok := m[digest]; ok {
		return []byte(s), nil
	}
	return nil, os.ErrNotExist
}

func readTestFile(path string) ([]byte, error) {
	pwd, err := os.Getwd()
	if err != nil {
		return nil, err
	}
	fullPath := filepath.Join(pwd, filepath.FromSlash(path))
	return ioutil.ReadFile(fullPath)
}

func hexDigest(data []byte) string {
	h := sha256.New()
	h.Write(data)
	return hex.EncodeToString(h.Sum(nil))
}

// oneFileCompilation constructs a compilation unit with a single source file
// attributed to path and package pkg, whose content is given. The compilation
// is returned along with the digest of the file's content.
func oneFileCompilation(path, pkg, content string) (*apb.CompilationUnit, string) {
	digest := hexDigest([]byte(content))
	return &apb.CompilationUnit{
		VName: &spb.VName{Language: "go", Corpus: "test", Path: pkg, Signature: ":pkg:"},
		RequiredInput: []*apb.CompilationUnit_FileInput{{
			Info: &apb.FileInfo{Path: path, Digest: digest},
		}},
		SourceFile: []string{path},
	}, digest
}

func TestResolve(t *testing.T) { // are you function enough not to back down?
	// Test resolution on a simple two-package system:
	//
	// Package foo is compiled as test data from the source
	//    package foo
	//    func Foo() int { return 0 }
	//
	// Package bar is specified as source and imports foo.
	// TODO(fromberger): Compile foo as part of the build.
	foo, err := readTestFile("kythe/go/indexer/testdata/foo.a")
	if err != nil {
		t.Fatalf("Unable to read foo.a: %v", err)
	}
	const bar = `package bar

import "test/foo"

func init() { println(foo.Foo()) }
`
	unit, digest := oneFileCompilation("testdata/bar.go", "bar", bar)
	fetcher := memFetcher{
		hexDigest(foo): string(foo),
		digest:         bar,
	}
	unit.RequiredInput = append(unit.RequiredInput, &apb.CompilationUnit_FileInput{
		VName: &spb.VName{Language: "go", Corpus: "test", Path: "foo", Signature: ":pkg:"},
		Info:  &apb.FileInfo{Path: "testdata/foo.a", Digest: hexDigest(foo)},
	})

	pi, err := Resolve(unit, fetcher, nil)
	if err != nil {
		t.Fatalf("Resolve failed: %v\nInput unit:\n%s", err, proto.MarshalTextString(unit))
	}
	if got, want := pi.Name, "bar"; got != want {
		t.Errorf("Package name: got %q, want %q", got, want)
	}
	if got, want := pi.ImportPath, "test/bar"; got != want {
		t.Errorf("Import path: got %q, want %q", got, want)
	}
	if dep, ok := pi.Dependencies["test/foo"]; !ok {
		t.Errorf("Missing dependency for test/foo in %+v", pi.Dependencies)
	} else if pi.VNames[dep] == nil {
		t.Errorf("Missing VName for test/foo in %+v", pi.VNames)
	}
	if got, want := len(pi.Files), len(unit.SourceFile); got != want {
		t.Errorf("Source files: got %d, want %d", got, want)
	}
	for _, err := range pi.Errors {
		t.Errorf("Unexpected resolution error: %v", err)
	}
}

func TestSpan(t *testing.T) {
	const input = `package main

import "fmt"
func main() { fmt.Println("Hello, world") }`

	unit, digest := oneFileCompilation("main.go", "main", input)
	fetcher := memFetcher{digest: input}
	pi, err := Resolve(unit, fetcher, nil)
	if err != nil {
		t.Fatalf("Resolve failed: %v\nInput unit:\n%s", err, proto.MarshalTextString(unit))
	}

	tests := []struct {
		key      func(*ast.File) ast.Node // return a node to compute a span for
		pos, end int                      // the expected span for the node
	}{
		{func(*ast.File) ast.Node { return nil }, -1, -1},                  // invalid node
		{func(*ast.File) ast.Node { return fakeNode{0, 2} }, -1, -1},       // invalid pos
		{func(*ast.File) ast.Node { return fakeNode{5, 0} }, 4, 4},         // invalid end
		{func(f *ast.File) ast.Node { return f.Name }, 8, 12},              // main
		{func(f *ast.File) ast.Node { return f.Imports[0].Path }, 21, 26},  // "fmt"
		{func(f *ast.File) ast.Node { return f.Decls[0] }, 14, 26},         // import "fmt"
		{func(f *ast.File) ast.Node { return f.Decls[1] }, 27, len(input)}, // func main() { ... }
	}
	for _, test := range tests {
		node := test.key(pi.Files[0])
		pos, end := pi.Span(node)
		if pos != test.pos || end != test.end {
			t.Errorf("Span(%v): got pos=%d, end=%v; want pos=%d, end=%d", node, pos, end, test.pos, test.end)
		}
	}
}

type fakeNode struct{ pos, end token.Pos }

func (f fakeNode) Pos() token.Pos { return f.pos }
func (f fakeNode) End() token.Pos { return f.end }

func TestSink(t *testing.T) {
	var facts, edges []*spb.Entry

	sink := Sink(func(_ context.Context, e *spb.Entry) error {
		if isEdge(e) {
			edges = append(edges, e)
		} else {
			facts = append(facts, e)
		}
		return nil
	})

	hasFact := func(who *spb.VName, name, value string) bool {
		for _, fact := range facts {
			if proto.Equal(fact.Source, who) && fact.FactName == name && string(fact.FactValue) == value {
				return true
			}
		}
		return false
	}
	hasEdge := func(src, tgt *spb.VName, kind string) bool {
		for _, edge := range edges {
			if proto.Equal(edge.Source, src) && proto.Equal(edge.Target, tgt) && edge.EdgeKind == kind {
				return true
			}
		}
		return false
	}

	// Verify that the entries we push into the sink are preserved in encoding.
	him := &spb.VName{Language: "peeps", Signature: "him"}
	her := &spb.VName{Language: "peeps", Signature: "her"}
	ctx := context.Background()
	sink.writeFact(ctx, him, "/name", "John")
	sink.writeEdge(ctx, him, her, "/friendof")
	sink.writeFact(ctx, her, "/name", "Mary")
	sink.writeEdge(ctx, him, him, "/loves")
	sink.writeEdge(ctx, her, him, "/suspiciousof")
	sink.writeFact(ctx, him, "/name/full", "Jonathan Q. Public")
	sink.writeFact(ctx, her, "/name/full", "Mary M. Q. Contrary")

	for _, want := range []struct {
		who         *spb.VName
		name, value string
	}{
		{him, "/name", "John"},
		{him, "/name/full", "Jonathan Q. Public"},
		{her, "/name", "Mary"},
		{her, "/name/full", "Mary M. Q. Contrary"},
	} {
		if !hasFact(want.who, want.name, want.value) {
			t.Errorf("Missing fact %q=%q for %+v", want.name, want.value, want.who)
		}
	}

	for _, want := range []struct {
		src, tgt *spb.VName
		kind     string
	}{
		{him, her, "/friendof"},
		{her, him, "/suspiciousof"},
		{him, him, "/loves"},
	} {

		if !hasEdge(want.src, want.tgt, want.kind) {
			t.Errorf("Missing edge %+v ―%s→ %+v", want.src, want.kind, want.tgt)
		}
	}
}

// isEdge reports whether e represents an edge.
func isEdge(e *spb.Entry) bool { return e.Target != nil && e.EdgeKind != "" }
