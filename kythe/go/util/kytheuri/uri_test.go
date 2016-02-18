/*
 * Copyright 2014 Google Inc. All rights reserved.
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

package kytheuri

import (
	"reflect"
	"testing"

	"github.com/golang/protobuf/proto"

	spb "kythe.io/kythe/proto/storage_proto"
)

func TestParse(t *testing.T) {
	tests := []struct {
		input string
		want  *URI
	}{
		// Empty URIs.
		{"", new(URI)},
		{"kythe:", new(URI)},
		{"kythe://", new(URI)},

		// Individual components.
		{"#sig", &URI{Signature: "sig"}},
		{"kythe:#sig", &URI{Signature: "sig"}},
		{"kythe://corpus", &URI{Corpus: "corpus"}},
		{"kythe://corpus/", &URI{Corpus: "corpus"}},
		{"kythe://corpus/with/path", &URI{Corpus: "corpus/with/path"}},
		{"//corpus/with/path", &URI{Corpus: "corpus/with/path"}},
		{"kythe:?root=R", &URI{Root: "R"}},
		{"kythe:?path=P", &URI{Path: "P"}},
		{"kythe:?lang=L", &URI{Language: "L"}},

		// Multiple attributes, with permutation of order.
		{"kythe:?lang=L?root=R", &URI{Root: "R", Language: "L"}},
		{"kythe:?lang=L?path=P?root=R", &URI{Root: "R", Language: "L", Path: "P"}},
		{"kythe:?root=R?path=P?lang=L", &URI{Root: "R", Language: "L", Path: "P"}},

		// Everything.
		{"kythe://bitbucket.org/creachadair/stringset?path=stringset.go?lang=go?root=blah#sig",
			&URI{"sig", "bitbucket.org/creachadair/stringset", "blah", "stringset.go", "go"}},

		// Regression: Escape sequences in the corpus specification.
		{"kythe://libstdc%2B%2B?lang=c%2B%2B?path=bits/basic_string.h?root=/usr/include/c%2B%2B/4.8",
			&URI{Corpus: "libstdc++", Path: "bits/basic_string.h", Root: "/usr/include/c++/4.8", Language: "c++"}},
	}
	for _, test := range tests {
		got, err := Parse(test.input)
		if err != nil {
			t.Errorf("Parse %q failed: %v", test.input, err)
			continue
		}
		if !reflect.DeepEqual(got, test.want) {
			t.Errorf("Parse %q:\ngot  %#v\nwant %#v", test.input, got, test.want)
		}
	}
}

func TestParseErrors(t *testing.T) {
	tests := []string{
		"invalid corpus",
		"http://unsupported-scheme",
		"?huh=bogus+attribute+key",
		"?path=",  // empty query value
		"?root=?", // empty query value
		"//a/%x/bad-escaping",
		"kythe:///invalid-corpus?blah",
		"/another-invalid-corpus",
		"random/opaque/failure",
	}
	for _, bad := range tests {
		got, err := Parse(bad)
		if err == nil {
			t.Errorf("Parse %q: got %#v, want error", bad, got)
		} else {
			t.Logf("Parse %q gave expected error: %v", bad, err)
		}
	}
}

func TestEqual(t *testing.T) {
	eq := []struct {
		a, b string
	}{
		// Various empty equivalencies.
		{"", ""},
		{"", "kythe:"},
		{"kythe://", ""},
		{"kythe://", "kythe:"},

		// Order of attributes is normalized.
		{"kythe:?root=R?path=P", "kythe://?path=P?root=R"},
		{"kythe:?root=R?path=P?lang=L", "kythe://?path=P?lang=L?root=R"},

		// Escaping is respected.
		{"kythe:?path=%50", "kythe://?path=P"},
		{"kythe:?lang=%4c?path=%50", "kythe://?lang=L?path=P"},

		// Paths are cleaned.
		{"kythe://a/b/../c#sig", "kythe://a/c#sig"},
		{"kythe://a/b/../d/./e/../../c#sig", "kythe://a/c#sig"},
		{"//a/b/c/../d?lang=%67%6F", "kythe://a/b/d?lang=go"},
	}
	for _, test := range eq {
		if !Equal(test.a, test.b) {
			t.Errorf("Equal incorrectly reported %q ≠ %q", test.a, test.b)
		}
		a := MustParse(test.a)
		b := MustParse(test.b)
		if !a.Equal(b) {
			t.Errorf("Equal incorrectly reported %q ≠ %q", a, b)
		}
	}
	neq := []struct {
		a, b string
	}{
		{"kythe://a", "kythe://a?path=P"},
		{"bogus", "bogus"},
		{"bogus", "kythe://good"},
		{"kythe://good", "bogus"},
	}
	for _, test := range neq {
		if Equal(test.a, test.b) {
			t.Errorf("Equal incorrectly reported %q = %q", test.a, test.b)
		}
	}

	// Corner cases
	var a, b *URI
	if !a.Equal(b) {
		t.Error("Equal failed to report nil == nil")
	}
	b = MustParse("kythe://")
	if !a.Equal(b) {
		t.Errorf("Equal incorrectly reported %#v ≠ %#v", a, b)
	}
}

func TestRoundTripURI(t *testing.T) {
	// Test that converting a Kythe URI to a VName and then back preserves
	// equivalence.
	u := &URI{
		Signature: "magic carpet ride",
		Corpus:    "code.google.com/p/go.tools",
		Path:      "cmd/godoc/doc.go",
		Language:  "go",
	}
	v := u.VName()
	t.Logf(" URL is %q\nVName is %v", u.String(), v)
	if s := v.Signature; s != u.Signature {
		t.Errorf("Signature: got %q, want %q", s, u.Signature)
	}
	if s := v.Corpus; s != u.Corpus {
		t.Errorf("Corpus: got %q, want %q", s, u.Corpus)
	}
	if s := v.Path; s != u.Path {
		t.Errorf("Path: got %q, want %q", s, u.Path)
	}
	if s := v.Root; s != u.Root {
		t.Errorf("Root: got %q, want %q", s, u.Root)
	}
	if s := v.Language; s != u.Language {
		t.Errorf("Language: got %q, want %q", s, u.Language)
	}
	w := FromVName(v)
	if got, want := w.String(), u.String(); got != want {
		t.Errorf("URI did not round-trip: got %q, want %q", got, want)
	}
}

func TestRoundTripVName(t *testing.T) {
	// Verify that converting a VName to a Kythe URI and then back preserves
	// equivalence.
	tests := []*spb.VName{
		{}, // empty
		{Corpus: "//Users/foo", Path: "/Users/foo/bar", Language: "go", Signature: "∴"},
		{Corpus: "//////", Root: "←", Language: "c++"},
	}
	for _, test := range tests {
		uri := FromVName(test)
		t.Logf("VName: %+v\nURI:   %#q", test, uri)
		got := uri.VName()
		if !proto.Equal(got, test) {
			t.Errorf("VName did not round-trip: got %+v, want %+v", got, test)
		}
	}
}

func TestString(t *testing.T) {
	const empty = "kythe:"
	const canonical = "kythe:?lang=L?path=P?root=R"
	const cleaned = "kythe://a/c#sig"
	tests := []struct {
		input, want string
	}{
		// Empty forms
		{"", empty},
		{"kythe:", empty},
		{"kythe://", empty},
		{"kythe:#", empty},
		{"kythe://#", empty},

		// Check ordering
		{"kythe:?root=R?path=P?lang=L", canonical},
		{"kythe:?root=R?lang=L?path=P", canonical},
		{"kythe:?lang=L?path=P?root=R", canonical},
		{"kythe://?lang=L?path=P?root=R#", canonical},

		// Check escaping
		{"kythe://?path=%50", "kythe:?path=P"},
		{"kythe://?path=%2B", "kythe:?path=%2B"},
		{"kythe://?path=a+b", "kythe:?path=a%2Bb"},
		{"kythe://?path=%20", "kythe:?path=%20"},
		{"kythe://?path=a/b", "kythe:?path=a/b"},

		// Path cleaning
		{"kythe://a/b/../c#sig", cleaned},
		{"kythe://a/./d/.././c#sig", cleaned},

		// Regression: Escape sequences in the corpus specification.
		{"kythe://libstdc%2B%2B?path=bits/basic_string.h?lang=c%2B%2B?root=/usr/include/c%2B%2B/4.8",
			"kythe://libstdc%2B%2B?lang=c%2B%2B?path=bits/basic_string.h?root=/usr/include/c%2B%2B/4.8"},
	}
	for _, test := range tests {
		u := MustParse(test.input)
		if got := u.String(); got != test.want {
			t.Errorf("String %#v:\ngot  %q\nwant %q", u, got, test.want)
		}
	}
}
