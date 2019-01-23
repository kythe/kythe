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

// Package govname supports the creation of VName protobuf messages for Go
// packages and other entities.
package govname

import (
	"go/build"
	"strings"

	"golang.org/x/tools/go/vcs"

	spb "kythe.io/kythe/proto/storage_go_proto"
)

const (
	pathTail     = `(?:/(?P<path>.+))?$`
	packageSig   = "package"
	golangCorpus = "golang.org"
)

// Language is the language string to use for Go VNames.
const Language = "go"

// PackageVNameOptions controls the construction of VNames for Go packages.
type PackageVNameOptions struct {
	// DefaultCorpus optionally provides a fallback corpus name if a package
	// cannot otherwise be resolved to a corpus.
	DefaultCorpus string

	// CanonicalizePackageCorpus determines whether a package's corpus name is
	// canonicalized as its VCS repository root URL rather than the Go import path
	// corresponding to the VCS repository root.
	CanonicalizePackageCorpus bool
}

// ForPackage returns a VName for a Go package.
//
// A package VName has the fixed signature "package", and the VName path holds
// the import path relative to the corpus root.  If the package's VCS root
// cannot be determined for the package's corpus, then by default the first path
// component of the import path is used as the corpus name, except for packages
// under GOROOT which are attributed to the special corpus "golang.org".
//
//
// Examples:
//   ForPackage(<kythe.io/kythe/go/util/schema>, &{CanonicalizePackageCorpus: false}) => {
//     Corpus: "kythe.io",
//     Path: "kythe/go/util/schema",
//		 Language: "go",
//		 Signature: "package",
//   }
//
//   ForPackage( <kythe.io/kythe/go/util/schema>, &{CanonicalizePackageCorpus: true}) => {
//     Corpus: "github.com/kythe/kythe",
//     Path: "kythe/go/util/schema",
//		 Language: "go",
//		 Signature: "package",
//   }
//
//   ForPackage(<github.com/kythe/kythe/kythe/go/util/schema>, &{CanonicalizePackageCorpus: false}) => {
//     Corpus: "github.com/kythe/kythe",
//     Path: "kythe/go/util/schema",
//		 Language: "go",
//		 Signature: "package",
//   }
//
//   ForPackage(<github.com/kythe/kythe/kythe/go/util/schema>, &{CanonicalizePackageCorpus: true}) => {
//     Corpus: "github.com/kythe/kythe",
//     Path: "kythe/go/util/schema",
//		 Language: "go",
//		 Signature: "package",
//   }
func ForPackage(pkg *build.Package, opts *PackageVNameOptions) *spb.VName {
	ip := pkg.ImportPath
	v := &spb.VName{Language: Language, Signature: packageSig}

	// Attempt to resolve the package's repository root as its corpus.
	if r, err := vcs.RepoRootForImportPath(ip, false); err == nil {
		// TODO cache by root
		v.Path = strings.TrimPrefix(strings.TrimPrefix(ip, r.Root), "/")
		if opts != nil && opts.CanonicalizePackageCorpus {
			// Use the canonical repository URL as the corpus.
			v.Corpus = r.Repo
			if i := strings.Index(v.Corpus, "://"); i >= 0 {
				// Remove URL scheme from corpus
				v.Corpus = v.Corpus[i+3:]
			}
			v.Corpus = strings.TrimSuffix(v.Corpus, "."+r.VCS.Cmd)
		} else {
			v.Corpus = r.Root
		}
		return v
	}

	v.Path = ip
	if pkg.Goroot {
		// This is a Go standard library package; the corpus is implicit.
		v.Corpus = golangCorpus
	} else if strings.HasPrefix(ip, ".") {
		// Local import; no corpus
	} else if i := strings.Index(ip, "/"); i > 0 {
		// Take the first slash-delimited component to be the corpus.
		// e.g., import "foo/bar/baz" ⇒ corpus "foo", signature "bar/baz".
		v.Corpus = ip[:i]
		v.Path = ip[i+1:]
	} else if opts != nil && opts.DefaultCorpus != "" {
		// Default: Assume the package is in "this" corpus, if defined.
		v.Corpus = opts.DefaultCorpus
	}
	return v
}

// ForBuiltin returns a VName for a Go built-in with the given signature.
func ForBuiltin(signature string) *spb.VName {
	return &spb.VName{
		Corpus:    golangCorpus,
		Language:  Language,
		Root:      "ref/spec",
		Signature: signature,
	}
}

// ForStandardLibrary returns a VName for a standard library package with the
// given import path.
func ForStandardLibrary(importPath string) *spb.VName {
	return &spb.VName{
		Corpus:    golangCorpus,
		Language:  Language,
		Path:      importPath,
		Signature: "package",
	}
}

// IsStandardLibrary reports whether v names part of the Go standard library.
// This includes the "golang.org" corpus but excludes the "golang.org/x/..."
// extension repositories.  If v == nil, the answer is false.
func IsStandardLibrary(v *spb.VName) bool {
	return v != nil && (v.Language == "go" || v.Language == "") && v.Corpus == golangCorpus
}
