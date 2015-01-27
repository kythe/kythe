// Package govname supports the creation of VName protobuf messages for Go
// packages and other entities.
package govname

import (
	"go/build"
	"regexp"
	"strings"

	"kythe/go/storage/vnameutil"

	"code.google.com/p/goprotobuf/proto"

	spb "kythe/proto/storage_proto"
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
	&spb.VName{Corpus: proto.String("${corpus}"), Signature: proto.String(packageSig), Root: proto.String("${subrepo}")},
}, {
	// Google code, old syntax
	regexp.MustCompile(`^(?i)(?P<corpus>[-._a-z0-9]+\.googlecode\.com)` + pathTail),
	&spb.VName{Corpus: proto.String("${corpus}"), Signature: proto.String(packageSig)},
}, {
	// GitHub
	regexp.MustCompile(`^(?P<corpus>github\.com/(?:[-.\w]+){2})` + pathTail),
	&spb.VName{Corpus: proto.String("${corpus}"), Signature: proto.String(packageSig)},
}, {
	// Bitbucket
	regexp.MustCompile(`^(?P<corpus>bitbucket\.org(?:/[-.\w]+){2})` + pathTail),
	&spb.VName{Corpus: proto.String("${corpus}"), Signature: proto.String(packageSig)},
}, {
	// Launchpad
	regexp.MustCompile(`^(?P<corpus>launchpad\.net/(?:[-.\w]+|~[-.\w]+/[-.\w]+))` + pathTail),
	&spb.VName{Corpus: proto.String("${corpus}"), Signature: proto.String(packageSig)},
}, {
	// Go extension repositories
	regexp.MustCompile(`(?P<corpus>golang\.org(?:/x/\w+))` + pathTail),
	&spb.VName{Corpus: proto.String("${corpus}"), Signature: proto.String(packageSig)},
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
		v = &spb.VName{Signature: proto.String(":pkg:" + ip)}
		if pkg.Goroot {
			// This is a Go standard library package; the corpus is implicit.
			v.Corpus = proto.String("golang.org")
		} else if strings.HasPrefix(ip, ".") {
			// Local import; no corpus
		} else if i := strings.Index(ip, "/"); i > 0 {
			// Take the first slash-delimited component to be the corpus.
			// e.g., import "foo/bar/baz" ⇒ corpus "foo", signature "bar/baz".
			v.Corpus = proto.String(ip[:i])
			v.Signature = proto.String(":pkg:" + ip[i+1:])
		} else if corpus != "" {
			// Default: Assume the package is in "this" corpus, if defined.
			v.Corpus = proto.String(corpus)
		}
	}
	if v != nil {
		v.Language = proto.String(Language)
	}
	return v
}
