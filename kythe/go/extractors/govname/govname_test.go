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

package govname

import (
	"go/build"
	"testing"

	"github.com/golang/protobuf/proto"

	"kythe.io/kythe/go/util/kytheuri"

	spb "kythe.io/kythe/proto/storage_proto"
)

func TestForPackage(t *testing.T) {
	tests := []struct {
		path   string
		ticket string
		isRoot bool
	}{
		{path: "bytes", ticket: "kythe://golang.org?lang=go?path=bytes#%3Apkg%3A", isRoot: true},
		{path: "go/types", ticket: "kythe://golang.org?lang=go?path=go/types#%3Apkg%3A", isRoot: true},
		{path: "golang.org/x/net/context", ticket: "kythe://golang.org/x/net?lang=go?path=context#%3Apkg%3A"},
		{path: "code.google.com/p/foo.bar/baz", ticket: "kythe://code.google.com/p/foo?lang=go?path=baz?root=bar#%3Apkg%3A"},
		{path: "fuzzy1.googlecode.com/alpha", ticket: "kythe://fuzzy1.googlecode.com?lang=go?path=alpha#%3Apkg%3A"},
		{path: "github.com/google/kythe/foo", ticket: "kythe://github.com/google/kythe?lang=go?path=foo#%3Apkg%3A"},
		{path: "bitbucket.org/zut/alors/non", ticket: "kythe://bitbucket.org/zut/alors?lang=go?path=non#%3Apkg%3A"},
		{path: "launchpad.net/~frood/blee/blor", ticket: "kythe://launchpad.net/~frood/blee?lang=go?path=blor#%3Apkg%3A"},

		{path: "golang.org/x/net/context", ticket: "kythe://golang.org/x/net?lang=go?path=context#%3Apkg%3A"},
	}
	for _, test := range tests {
		pkg := &build.Package{
			ImportPath: test.path,
			Goroot:     test.isRoot,
		}
		got := ForPackage("", pkg)
		gotTicket := kytheuri.ToString(got)
		if gotTicket != test.ticket {
			t.Errorf(`ForPackage("", [%s]): got %q, want %q`, test.path, gotTicket, test.ticket)
		}
	}
}

func TestIsStandardLib(t *testing.T) {
	test := &spb.VName{Corpus: "golang.org", Language: "go", Signature: "whatever"}
	if ok := IsStandardLibrary(test); !ok {
		t.Errorf("IsStandardLibrary(%+v): got %v, want true", test, ok)
	}
}

func TestNotStandardLib(t *testing.T) {
	tests := []*spb.VName{
		nil,
		{Corpus: "foo", Language: "go"},
		{Corpus: "golang.org", Language: "c++"},
		{Corpus: "golang.org/x/net", Language: "go", Signature: ":pkg:"},
		{Language: "go"},
		{Corpus: "golang.org", Language: "python", Path: "p", Root: "R", Signature: "Σ"},
	}
	for _, test := range tests {
		if ok := IsStandardLibrary(test); ok {
			t.Errorf("IsStandardLibrary(%+v): got %v, want false", test, ok)
		}
	}
}

func TestForBuiltin(t *testing.T) {
	const signature = "blah"
	want := &spb.VName{
		Corpus:    golangCorpus,
		Language:  Language,
		Root:      "ref/spec",
		Signature: signature,
	}
	got := ForBuiltin("blah")
	if !proto.Equal(got, want) {
		t.Errorf("ForBuiltin(%q): got %+v, want %+v", signature, got, want)
	}
}
