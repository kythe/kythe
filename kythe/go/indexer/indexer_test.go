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

package indexer

import (
	"context"
	"crypto/sha256"
	"encoding/base64"
	"encoding/hex"
	"fmt"
	"go/ast"
	"go/token"
	"io/ioutil"
	"os"
	"testing"

	"kythe.io/kythe/go/test/testutil"
	"kythe.io/kythe/go/util/log"
	"kythe.io/kythe/go/util/metadata"
	"kythe.io/kythe/go/util/ptypes"
	"kythe.io/kythe/go/util/schema/edges"

	"github.com/golang/protobuf/proto"

	apb "kythe.io/kythe/proto/analysis_go_proto"
	gopb "kythe.io/kythe/proto/go_go_proto"
	mpb "kythe.io/kythe/proto/metadata_go_proto"
	spb "kythe.io/kythe/proto/storage_go_proto"
)

type memFetcher map[string]string // :: digest → content

func (m memFetcher) Fetch(path, digest string) ([]byte, error) {
	if s, ok := m[digest]; ok {
		return []byte(s), nil
	}
	return nil, os.ErrNotExist
}

func readTestFile(t *testing.T, path string) ([]byte, error) {
	return ioutil.ReadFile(testutil.TestFilePath(t, path))
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
		VName: &spb.VName{Language: "go", Corpus: "test", Path: pkg, Signature: "package"},
		RequiredInput: []*apb.CompilationUnit_FileInput{{
			VName: &spb.VName{Corpus: "test", Path: path},
			Info:  &apb.FileInfo{Path: path, Digest: digest},
		}},
		SourceFile: []string{path},
	}, digest
}

func TestBuildTags(t *testing.T) {
	// Make sure build tags are being respected. Synthesize a compilation with
	// two trivial files, one tagged and the other not. After resolving, there
	// should only be one file.

	const keepFile = "// +build keepme\n\npackage foo"
	const dropFile = "// +build ignore\n\npackage foo"

	// Cobble together the data from two compilations into one.
	u1, keepDigest := oneFileCompilation("keep.go", "foo", keepFile)
	u2, dropDigest := oneFileCompilation("drop.go", "foo", dropFile)
	u1.RequiredInput = append(u1.RequiredInput, u2.RequiredInput...)
	u1.SourceFile = append(u1.SourceFile, u2.SourceFile...)

	fetcher := memFetcher{
		keepDigest: keepFile,
		dropDigest: dropFile,
	}

	// Attach details with the build tags we care about.
	info, err := ptypes.MarshalAny(&gopb.GoDetails{
		BuildTags: []string{"keepme"},
	})
	if err != nil {
		t.Fatalf("Marshaling Go details failed: %v", err)
	}
	u1.Details = append(u1.Details, info)

	pi, err := Resolve(u1, fetcher, nil)
	if err != nil {
		t.Fatalf("Resolving compilation failed: %v", err)
	}

	// Make sure the files are what we think we want.
	if n := len(pi.SourceText); n != 1 {
		t.Errorf("Wrong number of source files: got %d, want 1", n)
	}
	for _, got := range pi.SourceText {
		if got != keepFile {
			t.Errorf("Wrong source:\n got: %#q\nwant: %#q", got, keepFile)
		}
	}
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
	foo, err := readTestFile(t, "testdata/foo.a")
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
		VName: &spb.VName{Language: "go", Corpus: "test", Path: "foo", Signature: "package"},
		Info:  &apb.FileInfo{Path: "testdata/foo.a", Digest: hexDigest(foo)},
	})

	pi, err := Resolve(unit, fetcher, nil)
	if err != nil {
		t.Fatalf("Resolve failed: %v\nInput unit:\n%s", err, proto.MarshalTextString(unit))
	}
	if got, want := pi.Name, "bar"; got != want {
		t.Errorf("Package name: got %q, want %q", got, want)
	}
	if got, want := pi.VName, unit.VName; !proto.Equal(got, want) {
		t.Errorf("Base vname: got %+v, want %+v", got, want)
	}
	if got, want := pi.ImportPath, "test/bar"; got != want {
		t.Errorf("Import path: got %q, want %q", got, want)
	}
	if dep, ok := pi.Dependencies["test/foo"]; !ok {
		t.Errorf("Missing dependency for test/foo in %+v", pi.Dependencies)
	} else if pi.PackageVName[dep] == nil {
		t.Errorf("Missing VName for test/foo in %+v", pi.PackageVName)
	}
	if got, want := len(pi.Files), len(unit.SourceFile); got != want {
		t.Errorf("Source files: got %d, want %d", got, want)
	}
	for _, err := range pi.Errors {
		t.Errorf("Unexpected resolution error: %v", err)
	}
}

func base64EncodeInfo(t testing.TB, msg string) string {
	var gci mpb.GeneratedCodeInfo
	if err := proto.UnmarshalText(msg, &gci); err != nil {
		t.Fatal(err)
	}
	rec, err := proto.Marshal(&gci)
	if err != nil {
		t.Fatal(err)
	}
	return base64.StdEncoding.EncodeToString(rec)
}

func TestResolveInlineMetadata(t *testing.T) {
	const gci = `
meta: <
  edge: "%/kythe/edge/generates"
  vname: <
    signature: "IDENTIFIER:Primary"
    corpus: "default"
    path: "test/example.txt"
    language: "lang"
  >
  begin: 21
  end: 28
>
meta: <
  edge: "%/kythe/edge/generates"
  vname: <
    signature: "IDENTIFIER:Primary.my_param"
    corpus: "default"
    path: "test/example.txt"
    language: "lang"
  >
  begin: 41
  end: 48
>
	`
	encoded := base64EncodeInfo(t, gci)
	log.Infof("Encoded GeneratedCodeInfo: %s", encoded) // for testdata/basic/inline.go
	subject := "package subject\n\nvar Primary = struct {\n\tMyParam bool\n}{\n\tMyParam: true,\n}\n\n//gokythe-inline-metadata:" + encoded
	unit, digest := oneFileCompilation("testdata/subject.go", "subject", subject)
	fetcher := memFetcher{
		digest: subject,
	}

	featureRule := metadata.Rule{
		EdgeIn:  edges.DefinesBinding,
		EdgeOut: edges.Generates,
		VName: &spb.VName{
			Corpus:    "default",
			Language:  "lang",
			Signature: "IDENTIFIER:Primary",
			Path:      "test/example.txt",
		},
		Reverse: true,
		Begin:   21,
		End:     28,
	}

	flagRule := metadata.Rule{
		EdgeIn:  edges.DefinesBinding,
		EdgeOut: edges.Generates,
		VName: &spb.VName{
			Corpus:    "default",
			Language:  "lang",
			Signature: "IDENTIFIER:Primary.my_param",
			Path:      "test/example.txt",
		},
		Reverse: true,
		Begin:   41,
		End:     48,
	}
	wantRules := metadata.Rules{featureRule, flagRule}

	pi, err := Resolve(unit, fetcher, nil)
	if err != nil {
		t.Fatalf("Resolve failed: %v\nInput unit:\n%s", err, proto.MarshalTextString(unit))
	}

	gotRules := pi.Rules[pi.Files[0]]

	if len(pi.Rules) != 1 {
		t.Errorf("Resolve failed to load package rules in %v", unit.SourceFile)
	}
	if err := testutil.DeepEqual(wantRules, gotRules); err != nil {
		t.Errorf("Rules diff %s", err)
	}
}

func TestResolveErrors(t *testing.T) {
	unit, _ := oneFileCompilation("blah.a", "bogus", "package blah")
	unit.SourceFile = nil
	pkg, err := Resolve(unit, make(memFetcher), nil)
	if err == nil {
		t.Errorf("Resolving 0-source package: got %+v, wanted error", pkg)
	} else {
		t.Logf("Got expected error for 0-source package: %v", err)
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
		_, pos, end := pi.Span(node)
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
	them := &spb.VName{Language: "peeps", Signature: "him"}
	they := &spb.VName{Language: "peeps", Signature: "her"}
	ctx := context.Background()
	sink.writeFact(ctx, them, "/name", "Alex")
	sink.writeEdge(ctx, them, they, "/friendof")
	sink.writeFact(ctx, they, "/name", "Jordan")
	sink.writeEdge(ctx, them, them, "/loves")
	sink.writeEdge(ctx, they, them, "/suspiciousof")
	sink.writeFact(ctx, them, "/name/full", "Alex Q. Public")
	sink.writeFact(ctx, they, "/name/full", "Jordan M. Q. Contrary")

	for _, want := range []struct {
		who         *spb.VName
		name, value string
	}{
		{them, "/name", "Alex"},
		{them, "/name/full", "Alex Q. Public"},
		{they, "/name", "Jordan"},
		{they, "/name/full", "Jordan M. Q. Contrary"},
	} {
		if !hasFact(want.who, want.name, want.value) {
			t.Errorf("Missing fact %q=%q for %+v", want.name, want.value, want.who)
		}
	}

	for _, want := range []struct {
		src, tgt *spb.VName
		kind     string
	}{
		{them, they, "/friendof"},
		{they, them, "/suspiciousof"},
		{them, them, "/loves"},
	} {

		if !hasEdge(want.src, want.tgt, want.kind) {
			t.Errorf("Missing edge %+v ―%s→ %+v", want.src, want.kind, want.tgt)
		}
	}
}

func TestComments(t *testing.T) {
	// Verify that comment text is correctly escaped when translated into
	// documentation nodes.
	const input = `// Comment [escape] tests \t all the things.
package pkg

/*
  Comment [escape] tests \t all the things.
*/
var z int
`
	unit, digest := oneFileCompilation("testfile/comment.go", "pkg", input)
	pi, err := Resolve(unit, memFetcher{digest: input}, &ResolveOptions{Info: XRefTypeInfo()})
	if err != nil {
		t.Fatalf("Resolve failed: %v\nInput unit:\n%s", err, proto.MarshalTextString(unit))
	}

	var single, multi string
	if err := pi.Emit(context.Background(), func(_ context.Context, e *spb.Entry) error {
		if e.FactName != "/kythe/text" {
			return nil
		}
		if e.Source.Signature == "package doc" {
			if single != "" {
				return fmt.Errorf("multiple package docs (%q, %q)", single, string(e.FactValue))
			}
			single = string(e.FactValue)
		} else if e.Source.Signature == "var z doc" {
			if multi != "" {
				return fmt.Errorf("multiple variable docs (%q, %q)", multi, string(e.FactValue))
			}
			multi = string(e.FactValue)
		}
		return nil
	}, nil); err != nil {
		t.Fatalf("Emit unexpectedly failed: %v", err)
	}

	const want = `Comment \[escape\] tests \\t all the things.`
	if single != want {
		t.Errorf("Incorrect single-line comment escaping:\ngot  %#q\nwant %#q", single, want)
	}
	if multi != want {
		t.Errorf("Incorrect multi-line comment escaping:\ngot  %#q\nwant %#q", multi, want)
	}
}

func TestRules(t *testing.T) {
	const input = "package main\n"
	unit, digest := oneFileCompilation("main.go", "main", input)
	unit.RequiredInput = append(unit.RequiredInput, &apb.CompilationUnit_FileInput{
		VName: &spb.VName{Signature: "hey ho let's go"},
		Info:  &apb.FileInfo{Path: "meta"},
	})
	fetcher := memFetcher{digest: input}

	// Resolve the compilation with a rule checker that recognizes the special
	// input we added and emits a rule for the main file. This verifies we get
	// the right mapping from paths back to source inputs.
	pi, err := Resolve(unit, fetcher, &ResolveOptions{
		CheckRules: func(ri *apb.CompilationUnit_FileInput, _ Fetcher) (*Ruleset, error) {
			if ri.Info.Path == "meta" {
				return &Ruleset{
					Path: "main.go", // associate these rules to the main source
					Rules: metadata.Rules{{
						Begin: 1,
						End:   2,
						VName: ri.VName,
					}},
				}, nil
			}
			return nil, nil
		},
	})
	if err != nil {
		t.Fatalf("Resolve failed: %v", err)
	}

	// The rules should have an entry for the primary source file, and it
	// should contain the rule we generated.
	rs, ok := pi.Rules[pi.Files[0]]
	if !ok {
		t.Fatal("Missing primary source file")
	}
	want := metadata.Rules{{
		Begin: 1,
		End:   2,
		VName: &spb.VName{Signature: "hey ho let's go"},
	}}
	if err := testutil.DeepEqual(want, rs); err != nil {
		t.Errorf("Wrong rules: %v", err)
	}
}

// isEdge reports whether e represents an edge.
func isEdge(e *spb.Entry) bool { return e.Target != nil && e.EdgeKind != "" }
