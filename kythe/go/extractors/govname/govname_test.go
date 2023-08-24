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

package govname

import (
	"go/build"
	"strings"
	"testing"

	"kythe.io/kythe/go/util/kytheuri"
	"kythe.io/kythe/go/util/vnameutil"

	"golang.org/x/tools/go/vcs"
	"google.golang.org/protobuf/proto"

	spb "kythe.io/kythe/proto/storage_go_proto"
)

func TestForPackage(t *testing.T) {
	var exampleRules = `[{
		"pattern": "(.*)",
		"vname": {
			"corpus": "rule_corpus",
			"path": "rule_path/@1@"
		}}]`

	tests := []struct {
		path                    string // import path
		dir                     string // on-disk directory that contains this package
		root                    string // GOPATH if set, otherwise working directory from which extractor was invoked
		ticket                  string
		canonical               string
		isRoot                  bool
		rulesJSON               string
		defaultCorpus           string
		useDefaultCorpusForDeps bool
	}{
		{path: "bytes", ticket: "kythe://golang.org?lang=go?path=bytes#package", isRoot: true},
		// Rules should have no effect on go stdlib packages.
		{path: "bytes", ticket: "kythe://golang.org?lang=go?path=bytes#package", isRoot: true, rulesJSON: exampleRules},
		{path: "go/types", ticket: "kythe://golang.org?lang=go?path=go/types#package", isRoot: true},
		{path: "golang.org/x/net/context", ticket: "kythe://golang.org/x/net?lang=go?path=context#package",
			canonical: "kythe://go.googlesource.com/net?lang=go?path=context#package"},
		{path: "kythe.io/kythe/go/util/kytheuri", ticket: "kythe://kythe.io?lang=go?path=kythe/go/util/kytheuri#package",
			canonical: "kythe://github.com/kythe/kythe?lang=go?path=kythe/go/util/kytheuri#package"},
		{path: "github.com/kythe/kythe/kythe/go/util/kytheuri", ticket: "kythe://github.com/kythe/kythe?lang=go?path=kythe/go/util/kytheuri#package"},
		{path: "fuzzy1.googlecode.com/alpha", ticket: "kythe://fuzzy1.googlecode.com?lang=go?path=alpha#package"},
		{path: "github.com/kythe/kythe/foo", ticket: "kythe://github.com/kythe/kythe?lang=go?path=foo#package"},
		{root: "/workspace", dir: "/workspace/kythe/foo", path: "github.com/kythe/kythe/foo",
			ticket: "kythe://rule_corpus?lang=go?path=rule_path/kythe/foo#package", rulesJSON: exampleRules},
		{path: "bitbucket.org/creachadair/stringset/makeset", ticket: "kythe://bitbucket.org/creachadair/stringset?lang=go?path=makeset#package"},
		{path: "launchpad.net/~frood/blee/blor", ticket: "kythe://launchpad.net/~frood/blee/blor?lang=go#package"},
		{path: "launchpad.net/~frood/blee/blor/baz", ticket: "kythe://launchpad.net/~frood/blee/blor?lang=go?path=baz#package"},
		{path: "bitbucket.org/creachadair/stringset/makeset", defaultCorpus: "github.com/kythe/kythe", useDefaultCorpusForDeps: true, ticket: "kythe://github.com/kythe/kythe?lang=go?path=makeset?root=bitbucket.org/creachadair/stringset#package"},
	}
	for _, test := range tests {
		var rules vnameutil.Rules
		if test.rulesJSON != "" {
			var err error
			rules, err = vnameutil.ParseRules([]byte(test.rulesJSON))
			if err != nil {
				t.Errorf("parsing vname rules: %v", err)
			}
		}

		pkg := &build.Package{
			ImportPath: test.path,
			Goroot:     test.isRoot,
			Dir:        test.dir,
			Root:       test.root,
		}
		got := ForPackage(pkg, &PackageVNameOptions{
			DefaultCorpus:           test.defaultCorpus,
			UseDefaultCorpusForDeps: test.useDefaultCorpusForDeps,
			Rules:                   rules,
		})
		gotTicket := kytheuri.ToString(got)
		if gotTicket != test.ticket {
			t.Errorf(`ForPackage([%s], nil): got %q, want %q`, test.path, gotTicket, test.ticket)
		}

		canonical := test.ticket
		if test.canonical != "" {
			canonical = test.canonical
		}
		canonicalOpts := &PackageVNameOptions{
			DefaultCorpus:             test.defaultCorpus,
			UseDefaultCorpusForDeps:   test.useDefaultCorpusForDeps,
			CanonicalizePackageCorpus: true,
			Rules:                     rules,
		}
		if got := kytheuri.ToString(ForPackage(pkg, canonicalOpts)); got != canonical {
			t.Errorf(`ForPackage([%s], %#v): got %q, want canonicalized %q`, test.path, canonicalOpts, got, canonical)
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

func TestRepoRootCache(t *testing.T) {
	var root repoRootCacheNode

	if got := root.lookup(nil); got != nil {
		t.Errorf("Expected nil; found %+v", got)
	}
	if got := root.lookup([]string{"anything"}); got != nil {
		t.Errorf("Expected nil; found %+v", got)
	}

	root1 := &vcs.RepoRoot{Root: "anything/root"}
	root.add(strings.Split(root1.Root, "/"), root1)

	if got := root.lookup(nil); got != nil {
		t.Errorf("Expected nil; found %+v", got)
	}
	if got := root.lookup([]string{"anything"}); got != nil {
		t.Errorf("Expected nil; found %+v", got)
	}

	if got := root.lookup([]string{"anything", "root"}); got != root1 {
		t.Errorf("Expected %+v; found %+v", root1, got)
	}
	if got := root.lookup([]string{"anything", "root", "path"}); got != root1 {
		t.Errorf("Expected %+v; found %+v", root1, got)
	}
}

func TestImportPath(t *testing.T) {
	tests := []struct {
		vname       *spb.VName
		root, ipath string
	}{
		// A vname in the standard library corpus, with or without GOROOT set.
		{&spb.VName{Corpus: "golang.org", Path: "foo/bar", Language: "go"}, "", "foo/bar"},
		{&spb.VName{Corpus: "golang.org", Path: "foo/bar", Language: "go"}, "foo", "foo/bar"},
		// A vname in some other corpus with the default GOROOT.
		{&spb.VName{Corpus: "whatever.io", Path: "alpha/bravo.a"}, "", "whatever.io/alpha/bravo"},
		// A vname in a nonstandard GOROOT of another corpus.
		{&spb.VName{Corpus: "foo.com", Path: "odd/duck/pkg/linux_amd64/io/ioutil.a"}, "odd/duck", "io/ioutil"},
	}
	for _, test := range tests {
		if got := ImportPath(test.vname, test.root); got != test.ipath {
			t.Errorf("ImportPath(%v, %q): got %q, want %q", test.vname, test.root, got, test.ipath)
		}
	}
}
