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

// Package rdf implements encoding of RDF triples, as described in
// http://www.w3.org/TR/2014/REC-n-triples-20140225/.
package rdf // import "kythe.io/kythe/go/util/encoding/rdf"

import (
	"bytes"
	"fmt"
	"io"
	"unicode"
)

// A Triple represents a single RDF triple.
type Triple struct {
	Subject, Predicate, Object string
}

// encodeTo appends the encoding of t to buf.
func (t *Triple) encodeTo(buf *bytes.Buffer) {
	quoteTo(buf, t.Subject)
	buf.WriteByte(' ')
	quoteTo(buf, t.Predicate)
	buf.WriteByte(' ')
	quoteTo(buf, t.Object)
	buf.WriteString(" .")
}

// Encode writes a string encoding of t as an RDF triple to w.
func (t *Triple) Encode(w io.Writer) error {
	var buf bytes.Buffer
	t.encodeTo(&buf)
	_, err := w.Write(buf.Bytes())
	return err
}

// String returns a string encoding of t as an RDF triple.
func (t *Triple) String() string {
	var buf bytes.Buffer
	t.encodeTo(&buf)
	return buf.String()
}

// ctrlMap gives shortcut escape sequences for common control characters.
var ctrlMap = map[rune]string{
	'\t': `\t`,
	'\b': `\b`,
	'\n': `\n`,
	'\r': `\r`,
	'\f': `\f`,
}

// Quote produces a quote-bounded string from s, following the TURTLE escaping
// rules http://www.w3.org/TR/2014/REC-turtle-20140225/#sec-escapes.
func Quote(s string) string {
	var buf bytes.Buffer
	quoteTo(&buf, s)
	return buf.String()
}

func quoteTo(buf *bytes.Buffer, s string) {
	buf.Grow(2 + len(s))
	buf.WriteByte('"')
	for i, c := range s {
		switch {
		case unicode.IsControl(c):
			if s, ok := ctrlMap[c]; ok {
				buf.WriteString(s)
			} else {
				fmt.Fprintf(buf, "\\u%04x", c)
			}
		case c == '"', c == '\\', c == '\'':
			buf.WriteByte('\\')
			buf.WriteRune(c)
		case c <= unicode.MaxASCII:
			buf.WriteRune(c)
		case c == unicode.ReplacementChar:
			// In correctly-encoded UTF-8, we should never see a replacement
			// char.  Some text in the wild has valid Unicode characters that
			// aren't UTF-8, and this case lets us be more forgiving of those.
			fmt.Fprintf(buf, "\\u%04x", s[i])
		case c <= 0xffff:
			fmt.Fprintf(buf, "\\u%04x", c)
		default:
			fmt.Fprintf(buf, "\\U%08x", c)
		}
	}
	buf.WriteByte('"')
}
