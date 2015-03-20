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

// Package kytheuri provides a type to represent Kythe URIs.  This package
// supports parsing a Kythe URI from a string, and converting back and forth
// between a Kythe URI and a Kythe VName protobuf message.
package kytheuri

import (
	"bytes"
	"errors"
	"fmt"
	"path"
	"strings"

	spb "kythe.io/kythe/proto/storage_proto"
)

// Scheme is the URI scheme label for Kythe.
const Scheme = "kythe"

// A URI represents a parsed Kythe URI.  A zero-valued URI is ready for use,
// representing the empty URI.
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
		Path:      u.Path,
		Language:  u.Language,
	}
}

// String renders the Kythe URI into the standard URI string format.
//
// The resulting string is in canonical ordering, so if the URI was created by
// parsing a string, this may return a different string from that.  However,
// parsing this string will always give back the same URI.  If u == nil, it is
// treated as an empty URI.
func (u *URI) String() string {
	const empty = Scheme + ":"
	if u == nil {
		return empty
	}
	var buf bytes.Buffer
	buf.WriteString(empty)
	if c := u.Corpus; c != "" {
		buf.WriteString("//")
		// If the corpus has a path-like tail, separate that into the path
		// component of the URL.
		if i := strings.Index(c, "/"); i > 0 {
			c = path.Join(c[:i], path.Clean(c[i:]))
		}
		buf.WriteString(paths.escape(c))
	}

	// Pack up the query arguments.  Order matters here, so that we can
	// preserve a canonical string format.
	var query []string
	if s := u.Language; s != "" {
		query = append(query, "lang="+all.escape(s))
	}
	if s := u.Path; s != "" {
		query = append(query, "path="+paths.escape(s))
	}
	if s := u.Root; s != "" {
		query = append(query, "root="+paths.escape(s))
	}
	if len(query) != 0 {
		buf.WriteByte('?')
		buf.WriteString(strings.Join(query, "?"))
	}

	// If there is a signature, add that in as well.
	if s := u.Signature; s != "" {
		buf.WriteByte('#')
		buf.WriteString(all.escape(s))
	}
	return buf.String()
}

// Equal reports whether u is equal to v.
func (u *URI) Equal(v *URI) bool { return u.String() == v.String() }

// FromVName returns a Kythe URI for the given Kythe VName protobuf message.
func FromVName(v *spb.VName) *URI {
	return &URI{
		Signature: v.Signature,
		Corpus:    v.Corpus,
		Root:      v.Root,
		Path:      v.Path,
		Language:  v.Language,
	}
}

// cleanPath is as path.Clean, but leaves "" alone.
func cleanPath(s string) string {
	if s == "" {
		return ""
	}
	return path.Clean(s)
}

// Partition s around the first occurrence of mark, if any.
// If s has the form p mark q, returns p, q; otherwise returns s, "".
func split(s, mark string) (prefix, suffix string) {
	if i := strings.Index(s, mark); i >= 0 {
		return s[:i], s[i+len(mark):]
	}
	return s, ""
}

// scheme returns the scheme marker from the beginning of s, if it has one.
func scheme(s string) (scheme, tail string) {
	for i := 0; i < len(s); i++ {
		switch c := s[i]; {
		case 'a' <= c && c <= 'z' || 'A' <= c && c <= 'Z':
			// keep going
		case '0' <= c && c <= '9' || c == '+' || c == '-' || c == '.':
			if i == 0 {
				return "", s
			}
		case c == ':':
			return s[:i], s[i:]
		}
	}
	return "", s
}

// Parse parses a Kythe URI from s, if possible.  If s omits a scheme label,
// the "kythe" scheme is assumed.
func Parse(s string) (*URI, error) {
	// Split off the signature from the fragment tail, if defined.
	head, fragment := split(s, "#")

	// Check for a scheme label.  This may be empty; but if present, it must be
	// our expected scheme.
	scheme, head := scheme(head)
	if scheme == "" {
		if strings.HasPrefix(head, ":") { // e.g., "://foo/bar"
			return nil, errors.New("empty scheme")
		}
	} else if scheme != Scheme {
		return nil, fmt.Errorf("invalid scheme: %q", scheme)
	} else {
		head = head[1:] // drop the ":" following the scheme
	}

	// Check for a bundle of attribute values.  This may be empty.
	head, attrs := split(head, "?")
	if head != "" && !strings.HasPrefix(head, "//") {
		return nil, fmt.Errorf("invalid corpus label: %q", head)
	}
	sig, err := unescape(fragment)
	if err != nil {
		return nil, fmt.Errorf("invalid signature: %q", fragment)
	}
	corpus, err := unescape(strings.TrimLeft(head, "//"))
	if err != nil {
		return nil, fmt.Errorf("invalid corpus label: %q", corpus)
	}
	u := &URI{
		Signature: sig,
		Corpus:    cleanPath(corpus),
	}

	// If there are any attributes, parse them.  We allow valid attributes to
	// occur in any order, even if it is not canonical.
	if attrs != "" {
		for _, attr := range strings.Split(attrs, "?") {
			i := strings.Index(attr, "=")
			if i < 0 {
				return nil, fmt.Errorf("invalid attribute name: %q", attr)
			}
			name := attr[:i]
			value, err := unescape(attr[i+1:])
			if err != nil {
				return nil, fmt.Errorf("invalid attribute value %q: %v", value, err)
			} else if value == "" {
				return nil, fmt.Errorf("empty attribute value for %q", name)
			}

			switch name {
			case "lang":
				u.Language = value
			case "root":
				u.Root = value
			case "path":
				u.Path = value
			default:
				return nil, fmt.Errorf("invalid attribute: %q", name)
			}
		}
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
