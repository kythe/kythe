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

// Package govname supports the creation of VName protobuf messages for Go
// packages and other entities.
package govname

import (
	"go/build"
	"regexp"
	"strings"

	"kythe.io/kythe/go/storage/vnameutil"

	spb "kythe.io/kythe/proto/storage_proto"
)

const (
	pathTail   = `(?:/(?P<path>.+))?$`
	packageSig = ":pkg:${path}"
)

// VCSRules defines rewriting rules for Go import paths, using rules loosely
// borrowed from $GOROOT/src/cmd/go/vcs.go.  These rules are used to extract
// corpus and root information from an import path.
var VCSRules = vnameutil.Rules{{
	// Google code, new syntax
	regexp.MustCompile(`^(?i)(?P<corpus>code\.google\.com/p/[-a-z0-9]+)(?:\.(?P<subrepo>\w+))?` + pathTail),
	&spb.VName{Corpus: "${corpus}", Signature: packageSig, Root: "${subrepo}"},
}, {
	// Google code, old syntax
	regexp.MustCompile(`^(?i)(?P<corpus>[-._a-z0-9]+\.googlecode\.com)` + pathTail),
	&spb.VName{Corpus: "${corpus}", Signature: packageSig},
}, {
	// GitHub
	regexp.MustCompile(`^(?P<corpus>github\.com/(?:[-.\w]+){2})` + pathTail),
	&spb.VName{Corpus: "${corpus}", Signature: packageSig},
}, {
	// Bitbucket
	regexp.MustCompile(`^(?P<corpus>bitbucket\.org(?:/[-.\w]+){2})` + pathTail),
	&spb.VName{Corpus: "${corpus}", Signature: packageSig},
}, {
	// Launchpad
	regexp.MustCompile(`^(?P<corpus>launchpad\.net/(?:[-.\w]+|~[-.\w]+/[-.\w]+))` + pathTail),
	&spb.VName{Corpus: "${corpus}", Signature: packageSig},
}, {
	// Go extension repositories
	regexp.MustCompile(`(?P<corpus>golang\.org(?:/x/\w+))` + pathTail),
	&spb.VName{Corpus: "${corpus}", Signature: packageSig},
},
}

// Language is the language string to use for Go VNames.
const Language = "go"

// ForPackage returns a VName for a Go package.
//
// The implicit default rule is to take the first path component of the path as
// the corpus name, except for packages under GOROOT.
func ForPackage(corpus string, pkg *build.Package) *spb.VName {
	ip := pkg.ImportPath
	v, ok := VCSRules.Apply(ip)
	if !ok {
		v = &spb.VName{Signature: ":pkg:" + ip}
		if pkg.Goroot {
			// This is a Go standard library package; the corpus is implicit.
			v.Corpus = "golang.org"
		} else if strings.HasPrefix(ip, ".") {
			// Local import; no corpus
		} else if i := strings.Index(ip, "/"); i > 0 {
			// Take the first slash-delimited component to be the corpus.
			// e.g., import "foo/bar/baz" ⇒ corpus "foo", signature "bar/baz".
			v.Corpus = ip[:i]
			v.Signature = ":pkg:" + ip[i+1:]
		} else if corpus != "" {
			// Default: Assume the package is in "this" corpus, if defined.
			v.Corpus = corpus
		}
	}
	if v != nil {
		v.Language = Language
	}
	return v
}
