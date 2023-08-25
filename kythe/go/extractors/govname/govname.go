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
package govname // import "kythe.io/kythe/go/extractors/govname"

import (
	"go/build"
	"path/filepath"
	"regexp"
	"strings"

	"kythe.io/kythe/go/util/log"
	"kythe.io/kythe/go/util/vnameutil"

	"golang.org/x/tools/go/vcs"

	spb "kythe.io/kythe/proto/storage_go_proto"
)

const (
	pathTail   = `(?:/(?P<path>.+))?$`
	packageSig = "package"
	// GolangCorpus is the corpus used for the go std library
	GolangCorpus = "golang.org"
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

	// Rules optionally provides a list of rules to apply to go package and file
	// paths to customize output vnames. See the vnameutil package for details.
	Rules vnameutil.Rules

	// If set, file and package paths are made relative to this directory before
	// applying vname rules (if any). If unset, the module root (if using
	// modules) or the gopath directory is used instead.
	RootDirectory string

	// UseDefaultCorpusForStdLib tells the extractor to assign the DefaultCorpus
	// to go stdlib files rather than the default of 'golang.org'.
	UseDefaultCorpusForStdLib bool

	// UseDefaultCorpusForDeps tells the extractor to set the vname corpus for
	// imported modules to DefaultCorpus and to use the module's import path as
	// the vname's root.
	UseDefaultCorpusForDeps bool
}

// ForPackage returns a VName for a Go package.
//
// A package VName has the fixed signature "package", and the VName path holds
// the import path relative to the corpus root.  If the package's VCS root
// cannot be determined for the package's corpus, then by default the first path
// component of the import path is used as the corpus name, except for packages
// under GOROOT which are attributed to the special corpus "golang.org".
//
// If a set of vname rules is provided, they are applied first and used if
// applicable. Note that vname rules are ignored for go stdlib packages.
//
// Examples:
//
//	  ForPackage(<kythe.io/kythe/go/util/schema>, &{CanonicalizePackageCorpus: false}) => {
//	    Corpus: "kythe.io",
//	    Path: "kythe/go/util/schema",
//			 Language: "go",
//			 Signature: "package",
//	  }
//
//	  ForPackage( <kythe.io/kythe/go/util/schema>, &{CanonicalizePackageCorpus: true}) => {
//	    Corpus: "github.com/kythe/kythe",
//	    Path: "kythe/go/util/schema",
//			 Language: "go",
//			 Signature: "package",
//	  }
//
//	  ForPackage(<github.com/kythe/kythe/kythe/go/util/schema>, &{CanonicalizePackageCorpus: false}) => {
//	    Corpus: "github.com/kythe/kythe",
//	    Path: "kythe/go/util/schema",
//			 Language: "go",
//			 Signature: "package",
//	  }
//
//	  ForPackage(<github.com/kythe/kythe/kythe/go/util/schema>, &{CanonicalizePackageCorpus: true}) => {
//	    Corpus: "github.com/kythe/kythe",
//	    Path: "kythe/go/util/schema",
//			 Language: "go",
//			 Signature: "package",
//	  }
func ForPackage(pkg *build.Package, opts *PackageVNameOptions) *spb.VName {
	if !pkg.Goroot && opts != nil && opts.Rules != nil {
		root := pkg.Root
		if opts.RootDirectory != "" {
			root = opts.RootDirectory
		}

		relpath, err := filepath.Rel(root, pkg.Dir)
		if err != nil {
			log.Fatalf("relativizing path %q against dir %q: %v", pkg.Dir, root, err)
		}
		if relpath == "." {
			relpath = ""
		}

		v2, ok := opts.Rules.Apply(relpath)
		if ok {
			v2.Language = Language
			v2.Signature = packageSig
			return v2
		}
	}

	ip := pkg.ImportPath
	v := &spb.VName{Language: Language, Signature: packageSig}

	// Attempt to resolve the package's repository root as its corpus.
	if r, err := RepoRoot(ip); err == nil {
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

		if opts.UseDefaultCorpusForDeps && v.Corpus != opts.DefaultCorpus {
			v.Root = v.Corpus
			v.Corpus = opts.DefaultCorpus
		}

		return v
	}

	v.Path = ip
	if pkg.Goroot {
		// This is a Go standard library package. By default the corpus is
		// implied to be "golang.org", but can be configured to use the default
		// corpus instead.
		if opts.UseDefaultCorpusForStdLib {
			v.Corpus = opts.DefaultCorpus
		} else {
			v.Corpus = GolangCorpus
		}
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

// ForStandardLibrary returns a VName for a standard library package with the
// given import path.
func ForStandardLibrary(importPath string) *spb.VName {
	return &spb.VName{
		Corpus:    GolangCorpus,
		Language:  Language,
		Path:      importPath,
		Signature: "package",
	}
}

// IsStandardLibrary reports whether v names part of the Go standard library.
// This includes the "golang.org" corpus but excludes the "golang.org/x/..."
// extension repositories.  If v == nil, the answer is false.
func IsStandardLibrary(v *spb.VName) bool {
	return v != nil && (v.Language == "go" || v.Language == "") && v.Corpus == GolangCorpus
}

var archiveExt = regexp.MustCompile(`\.[xa]$`)

// ImportPath returns the putative Go import path corresponding to v.  The
// resulting string corresponds to the string literal appearing in source at the
// import site for the package so named.
func ImportPath(v *spb.VName, goRoot string) string {
	if IsStandardLibrary(v) || (goRoot != "" && v.Root == goRoot) {
		return v.Path
	}

	trimmed := archiveExt.ReplaceAllString(v.Path, "")
	if tail, ok := rootRelative(goRoot, trimmed); ok {
		// Paths under a nonempty GOROOT are treated as if they were standard
		// library packages even if they are not labelled as "golang.org", so
		// that nonstandard install locations will work sensibly.
		return tail
	}
	return filepath.Join(v.Corpus, trimmed)
}

// rootRelative reports whether path has the form
//
//	root[/pkg/os_arch/]tail
//
// and if so, returns the tail. It returns path, false if path does not have
// this form.
func rootRelative(root, path string) (string, bool) {
	trimmed := strings.TrimPrefix(path, root+"/")
	if root == "" || trimmed == path {
		return path, false
	}
	if tail := strings.TrimPrefix(trimmed, "pkg/"); tail != trimmed {
		parts := strings.SplitN(tail, "/", 2)
		if len(parts) == 2 && strings.Contains(parts[0], "_") {
			return parts[1], true
		}
	}
	return trimmed, true
}

// RepoRoot analyzes importPath to determine it's vcs.RepoRoot.
func RepoRoot(importPath string) (*vcs.RepoRoot, error) {
	if root := repoRootCache.lookup(strings.Split(importPath, "/")); root != nil {
		return root, nil
	}
	r, err := vcs.RepoRootForImportPath(importPath, false)
	if err != nil {
		return nil, err
	}
	repoRootCache.add(strings.Split(r.Root, "/"), r)
	return r, nil
}

var repoRootCache repoRootCacheNode

// repoRootCacheNode is a prefix search tree node for *vcs.RepoRoots by their
// root import path.
type repoRootCacheNode struct {
	root *vcs.RepoRoot

	children map[string]*repoRootCacheNode
}

// add puts the given *vcs.RepoRoot into the prefix tree for the given path
// components.
func (n *repoRootCacheNode) add(components []string, r *vcs.RepoRoot) {
	if len(components) == 0 {
		n.root = r
		return
	}

	if n.children == nil {
		n.children = make(map[string]*repoRootCacheNode)
	}
	p := components[0]
	c := n.children[p]
	if c == nil {
		c = &repoRootCacheNode{}
		n.children[p] = c
	}
	c.add(components[1:], r)
}

// lookup returns the first known *vcs.RepoRoot for any prefix of the given path
// components.  Returns nil if none match.
func (n *repoRootCacheNode) lookup(components []string) *vcs.RepoRoot {
	if n == nil {
		return nil
	} else if n.root != nil || len(components) == 0 {
		return n.root
	}
	return n.children[components[0]].lookup(components[1:])
}
