/*
 * Copyright 2014 The Kythe Authors. All rights reserved.
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

// Package kytheuri provides a type to represent Kythe URIs.  This package
// supports parsing a Kythe URI from a string, and converting back and forth
// between a Kythe URI and a Kythe VName protobuf message.
package kytheuri // import "kythe.io/kythe/go/util/kytheuri"

import (
	"errors"
	"fmt"
	"path"
	"strings"

	cpb "kythe.io/kythe/proto/common_go_proto"
	spb "kythe.io/kythe/proto/storage_go_proto"
)

// Scheme is the URI scheme label for Kythe.
const Scheme = "kythe:"

// A URI represents a parsed, unescaped Kythe URI.  A zero-valued URI is ready
// for use, representing the empty URI.
type URI struct {
	Signature string
	Corpus    string
	Root      string
	Path      string
	Language  string
}

// VName converts the URI to an equivalent Kythe VName protobuf message.
func (u *URI) VName() *spb.VName {
	if u == nil {
		return new(spb.VName)
	}
	return &spb.VName{
		Signature: u.Signature,
		Corpus:    u.Corpus,
		Root:      u.Root,
		Path:      cleanPath(u.Path),
		Language:  u.Language,
	}
}

// String renders the Kythe URI into the standard URI string format.
//
// The resulting string is in canonical ordering, so if the URI was created by
// parsing a string, this may return a different string from that.  However,
// parsing this string will always give back the same URI.  If u == nil, it is
// treated as an empty URI.
func (u *URI) String() string { return u.Encode().String() }

// Equal reports whether u is equal to v.
func (u *URI) Equal(v *URI) bool { return u.String() == v.String() }

// Encode returns an escaped "raw" Kythe URI equivalent to u.
func (u *URI) Encode() *Raw {
	if u == nil {
		return nil
	}
	return &Raw{
		URI: URI{
			Signature: all.escape(u.Signature),
			Corpus:    paths.escape(u.Corpus),
			Root:      paths.escape(u.Root),
			Path:      paths.escape(cleanPath(u.Path)),
			Language:  all.escape(u.Language),
		},
	}
}

// A Raw represents a parsed, "raw" Kythe URI whose field values are escaped.
// Use the Decode method to convert a *Raw to a plain *URI.
type Raw struct{ URI URI }

// Decode returns a *URI equivalent to r but with its field values unescaped.
func (r *Raw) Decode() (*URI, error) {
	u := r.URI // copy
	buf := make([]byte, len(u.Signature)+len(u.Corpus)+len(u.Root)+len(u.Path)+len(u.Language))
	return decode(&u, buf)
}

// String renders r into the standard URI string format.
//
// The resulting string is in canonical ordering, so if the URI was created by
// parsing a string, this may return a different string from that.  However,
// parsing this string will always give back the same URI.  If r == nil, it is
// treated as an empty URI.
func (r *Raw) String() string {
	if r == nil {
		return Scheme
	}
	var buf strings.Builder
	buf.Grow(len(Scheme) +
		2 + len(r.URI.Corpus) + // "//" + corpus
		6 + len(r.URI.Language) + // "?lang=" + string
		6 + len(r.URI.Path) + // "?path=" + string
		6 + len(r.URI.Root) + // "?root=" + string
		1 + len(r.URI.Signature), // "#" + string
	)
	buf.WriteString(Scheme)
	if c := r.URI.Corpus; c != "" {
		buf.WriteString("//")
		buf.WriteString(c)
	}

	// Pack up the query arguments. Order matters here, so that we can preserve
	// a canonical string format.
	if s := r.URI.Language; s != "" {
		buf.WriteString("?lang=")
		buf.WriteString(s)
	}
	if s := r.URI.Path; s != "" {
		buf.WriteString("?path=")
		buf.WriteString(s)
	}
	if s := r.URI.Root; s != "" {
		buf.WriteString("?root=")
		buf.WriteString(s)
	}

	// If there is a signature, add that in as well.
	if s := r.URI.Signature; s != "" {
		buf.WriteByte('#')
		buf.WriteString(s)
	}
	return buf.String()
}

// FromVName returns a Kythe URI for the given Kythe VName protobuf message.
func FromVName(v *spb.VName) *URI {
	if v == nil {
		return &URI{}
	}
	return &URI{
		Signature: v.Signature,
		Corpus:    v.Corpus,
		Root:      v.Root,
		Path:      v.Path,
		Language:  v.Language,
	}
}

// FromCorpusPath returns a Kythe URI for the given Kythe VName protobuf message.
func FromCorpusPath(cp *cpb.CorpusPath) *URI {
	if cp == nil {
		return &URI{}
	}
	return &URI{
		Corpus: cp.Corpus,
		Root:   cp.Root,
		Path:   cp.Path,
	}
}

// cleanPath is as path.Clean, but leaves "" alone.
func cleanPath(s string) string {
	if s == "" {
		return s
	}
	return path.Clean(s)
}

// Partition s around the first occurrence of mark, if any.
// If s has the form p mark q, returns p, q; otherwise returns s, "".
func split(s string, mark byte) (prefix, suffix string) {
	if i := strings.IndexByte(s, mark); i >= 0 {
		return s[:i], s[i+1:]
	}
	return s, ""
}

// ParseRaw parses a Kythe URI from s, but does not unescape its fields.  Use
// Parse to fully parse and unescape a URI, or call the Decode method of the
// returned value.
func ParseRaw(s string) (*Raw, error) {
	if s == "" {
		return new(Raw), nil
	}

	// Split off the signature from the fragment tail, if defined.
	head, fragment := split(s, '#')

	// Check for a scheme label.  This may be empty; but if present, it must be
	// our expected scheme.
	if tail := strings.TrimPrefix(head, Scheme); tail != head {
		head = tail // found and removed our scheme marker
	}

	// Check for a bundle of attribute values.  This may be empty.
	head, attrs := split(head, '?')
	if tail := strings.TrimPrefix(head, "//"); tail != head {
		head = tail
	} else if head != "" {
		return nil, errors.New("invalid URI scheme")
	}

	r := &Raw{
		URI: URI{
			Signature: fragment,
			Corpus:    head,
		},
	}

	// If there are any attributes, parse them.  We allow valid attributes to
	// occur in any order, even if it is not canonical.
	if attrs != "" {
		if err := splitByte(attrs, '?', func(attr string) error {
			name, value := split(attr, '=')
			if value == "" {
				return fmt.Errorf("invalid attribute: %q", attr)
			}
			switch name {
			case "lang":
				r.URI.Language = value
			case "root":
				r.URI.Root = value
			case "path":
				r.URI.Path = value
			default:
				return fmt.Errorf("invalid attribute: %q", name)
			}
			return nil
		}); err != nil {
			return nil, err
		}
	}
	return r, nil
}

// splitByte calls f with each partition of s delimited by b or the end of the
// string.  If f reports an error, the split is aborted and that error is
// returned to the caller of splitByte.
func splitByte(s string, b byte, f func(string) error) error {
	pos := 0
	for pos < len(s) {
		tail := s[pos:]
		i := strings.IndexByte(tail, b)
		if i < 0 {
			return f(tail)
		} else if err := f(tail[:i]); err != nil {
			return err
		}
		pos += i + 1
	}
	return nil
}

// Parse parses and unescapes a Kythe URI from s. If s omits a scheme label,
// the "kythe" scheme is assumed.
func Parse(s string) (*URI, error) {
	r, err := ParseRaw(s)
	if err != nil {
		return nil, err
	}
	return decode(&r.URI, make([]byte, len(s)))
}

// decode decodes u in-place using buf as an intermediate buffer.  The caller
// must ensure len(buf) is sufficient to hold the longest field.  Preallocation
// reduces allocation for unescaping and saves ~200 ns/op in benchmarks.
func decode(u *URI, buf []byte) (*URI, error) {
	if err := unescape(&u.Signature, buf); err != nil {
		return nil, fmt.Errorf("invalid signature: %v", err)
	} else if err := unescape(&u.Corpus, buf); err != nil {
		return nil, fmt.Errorf("invalid corpus label: %v", err)
	} else if err := unescape(&u.Language, buf); err != nil {
		return nil, fmt.Errorf("invalid language: %v", err)
	} else if err := unescape(&u.Path, buf); err != nil {
		return nil, fmt.Errorf("invalid path: %v", err)
	} else if err := unescape(&u.Root, buf); err != nil {
		return nil, fmt.Errorf("invalid root: %v", err)
	}
	return u, nil
}

// ToString renders the given VName into the standard string uri format.
func ToString(v *spb.VName) string { return FromVName(v).String() }

// ToVName parses the given string as a URI and returns an equivalent VName.
func ToVName(s string) (*spb.VName, error) {
	uri, err := Parse(s)
	if err != nil {
		return nil, err
	}
	return uri.VName(), nil
}

// MustParse returns the URI from parsing s, or panics in case of error.
func MustParse(s string) *URI {
	u, err := Parse(s)
	if err != nil {
		panic(fmt.Sprintf("Parse %q: %v", s, err))
	}
	return u
}

// Fix returns the canonical form of the given Kythe URI, if possible.
func Fix(s string) (string, error) {
	u, err := Parse(s)
	if err != nil {
		return "", err
	}
	return u.String(), nil
}

// Equal reports whether the two Kythe URI strings are equal in canonical form.
// If either URI is invalid, Equal returns false.
func Equal(u1, u2 string) bool {
	f1, err := Fix(u1)
	if err != nil {
		return false
	}
	f2, err := Fix(u2)
	if err != nil {
		return false
	}
	return f1 == f2
}
