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
	"io/ioutil"
	"os"
	"path/filepath"
	"testing"

	"github.com/golang/protobuf/proto"

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
	fullPath := filepath.Join(os.Getenv("TEST_SRCDIR"), filepath.FromSlash(path))
	return ioutil.ReadFile(fullPath)
}

func hexDigest(data []byte) string {
	h := sha256.New()
	h.Write(data)
	return hex.EncodeToString(h.Sum(nil))
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
	fetcher := memFetcher{
		hexDigest(foo):         string(foo),
		hexDigest([]byte(bar)): bar,
	}
	unit := &apb.CompilationUnit{
		VName: &spb.VName{Language: "go", Corpus: "test", Path: "bar", Signature: ":pkg:"},
		RequiredInput: []*apb.CompilationUnit_FileInput{{
			VName: &spb.VName{Language: "go", Corpus: "test", Path: "foo", Signature: ":pkg:"},
			Info:  &apb.FileInfo{Path: "testdata/foo.a", Digest: hexDigest(foo)},
		}, {
			Info: &apb.FileInfo{Path: "testdata/bar.go", Digest: hexDigest([]byte(bar))},
		}},
		SourceFile: []string{"testdata/bar.go"},
	}
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
