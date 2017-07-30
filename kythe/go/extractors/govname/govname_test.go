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

	"kythe.io/kythe/go/util/kytheuri"

	"github.com/golang/protobuf/proto"

	spb "kythe.io/kythe/proto/storage_proto"
)

func TestForPackage(t *testing.T) {
	tests := []struct {
		path   string
		ticket string
		isRoot bool
	}{
		{path: "bytes", ticket: "kythe://golang.org?lang=go?path=bytes#package", isRoot: true},
		{path: "go/types", ticket: "kythe://golang.org?lang=go?path=go/types#package", isRoot: true},
		{path: "golang.org/x/net/context", ticket: "kythe://golang.org/x/net?lang=go?path=context#package"},
		{path: "code.google.com/p/foo.bar/baz", ticket: "kythe://code.google.com/p/foo?lang=go?path=baz?root=bar#package"},
		{path: "fuzzy1.googlecode.com/alpha", ticket: "kythe://fuzzy1.googlecode.com?lang=go?path=alpha#package"},
		{path: "github.com/google/kythe/foo", ticket: "kythe://github.com/google/kythe?lang=go?path=foo#package"},
		{path: "bitbucket.org/zut/alors/non", ticket: "kythe://bitbucket.org/zut/alors?lang=go?path=non#package"},
		{path: "launchpad.net/~frood/blee/blor", ticket: "kythe://launchpad.net/~frood/blee?lang=go?path=blor#package"},

		{path: "golang.org/x/net/context", ticket: "kythe://golang.org/x/net?lang=go?path=context#package"},
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
	tests := []*spb.VName{
		{Corpus: "golang.org"},
		{Corpus: "golang.org", Language: "go", Signature: "whatever"},
		{Corpus: "golang.org", Language: "go", Path: "strconv"},
	}
	for _, test := range tests {
		if ok := IsStandardLibrary(test); !ok {
			t.Errorf("IsStandardLibrary(%+v): got %v, want true", test, ok)
		}
	}
}

func TestNotStandardLib(t *testing.T) {
	tests := []*spb.VName{
		nil,
		{},
		{Corpus: "foo", Language: "go"},
		{Corpus: "golang.org", Language: "c++"},
		{Corpus: "golang.org/x/net", Language: "go", Signature: "package"},
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

func TestForStandardLibrary(t *testing.T) {
	tests := []struct {
		input string
		want  *spb.VName
	}{
		{"fmt", &spb.VName{Corpus: "golang.org", Path: "fmt", Signature: "package", Language: "go"}},
		{"io/ioutil", &spb.VName{Corpus: "golang.org", Path: "io/ioutil", Signature: "package", Language: "go"}},
		{"strconv", &spb.VName{Corpus: "golang.org", Path: "strconv", Signature: "package", Language: "go"}},
	}
	for _, test := range tests {
		got := ForStandardLibrary(test.input)
		if !proto.Equal(got, test.want) {
			t.Errorf("ForStandardLibrary(%q): got %+v\nwant %+v", test.input, got, test.want)
		} else if !IsStandardLibrary(got) {
			t.Errorf("IsStandardLibrary(%+v) is unexpectedly false", got)
		}
	}
}
