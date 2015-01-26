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
	"bytes"
	"errors"
)

type escaper int

const (
	all escaper = iota
	paths
)

// shouldEscape reports whether c should be %-escaped for inclusion in a Kythe
// URI string.
func (e escaper) shouldEscape(c byte) bool {
	if 'a' <= c && c <= 'z' || 'A' <= c && c <= 'Z' || '0' <= c && c <= '9' {
		return false // unreserved: ALPHA, DIGIT
	}
	switch c {
	case '-', '.', '_', '~':
		return false // unreserved: punctuation
	case '/':
		return e != paths
	}
	return true
}

const hexDigits = "0123456789ABCDEF"

// escape encodes a string for use in a Kythe URI, %-escaping values as needed.
func (e escaper) escape(s string) string {
	var numEsc int
	for i := 0; i < len(s); i++ {
		if e.shouldEscape(s[i]) {
			numEsc++
		}
	}
	if numEsc == 0 {
		return s
	}
	b := bytes.NewBuffer(make([]byte, 0, len(s)+2*numEsc))
	for i := 0; i < len(s); i++ {
		if c := s[i]; e.shouldEscape(c) {
			b.WriteByte('%')
			b.WriteByte(hexDigits[c>>4])
			b.WriteByte(hexDigits[c%16])
		} else {
			b.WriteByte(c)
		}
	}
	return b.String()
}

// dehex returns the digit value of the hex digit encoded by c, or -1 if c does
// not denote a hex digit.
func dehex(c byte) int {
	switch {
	case 'a' <= c && c <= 'f':
		return int(c - 'a' + 10)
	case 'A' <= c && c <= 'F':
		return int(c - 'A' + 10)
	case '0' <= c && c <= '9':
		return int(c - '0')
	}
	return -1
}

// unescape unencodes a %-escaped string.
func unescape(s string) (string, error) {
	var numEsc int
	for i := 0; i < len(s); i++ {
		if s[i] == '%' {
			numEsc++
		}
	}
	if numEsc == 0 {
		return s, nil
	}

	b := bytes.NewBuffer(make([]byte, 0, len(s)-2*numEsc))
	for i := 0; i < len(s); i++ {
		if c := s[i]; c != '%' {
			b.WriteByte(c)
			continue
		}
		if i+2 >= len(s) {
			return "", errors.New("invalid hex escape")
		}
		hi := dehex(s[i+1])
		lo := dehex(s[i+2])
		if hi < 0 || lo < 0 {
			return "", errors.New("invalid hex digit")
		}
		b.WriteByte(byte(hi<<4 | lo))
		i += 2 // skip the extra bytes from the escape
	}
	return b.String(), nil
}
