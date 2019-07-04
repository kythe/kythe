/*
 * Copyright 2017 The Kythe Authors. All rights reserved.
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

// Package pathmap provides utilities for matching and generating paths
// based on a pattern string.
package pathmap // import "kythe.io/kythe/go/languageserver/pathmap"

import (
	"fmt"
	"net/url"
	"path/filepath"
	"regexp"
	"strings"
)

// Mapper provides path parsing and generating with named segments
// See NewMapper for details on construction
type Mapper struct {
	// Regexp used for parsing strings to variables
	re *regexp.Regexp
	// Array of segments used for generating strings from variables
	seg []segment
}

// NewMapper produces a Mapper object from a pattern string.
// Patterns strings are paths that have named segments that are extracted during
// parsing and populated during generation.
// Example:
// 		m := NewMapper("/dir/:segment/home/:rest*")
//  	s, err := m.Parse("/dir/foo/home/hello/world")	// {"segment": "foo", "rest": "hello/world"}, nil
//		p := m.Generate(s) 								// /dir/foo/home/hello/world
func NewMapper(pat string) (*Mapper, error) {
	// All handling below assumes unix-style '/' separated paths
	pat = filepath.ToSlash(pat)

	// This is a sanity check to ensure that the path is not malformed.
	// By prepending the / we stop url from getting confused if there's
	// a leading colon
	u, err := url.Parse("/" + pat)
	if err != nil {
		return nil, fmt.Errorf("pattern (%s) is not a valid path: '%v'", pat, err)
	}
	if u.EscapedPath() != u.Path {
		return nil, fmt.Errorf("pattern (%s) is requires escaping", pat)
	}

	var (
		segments []segment
		patReg   = "^"
	)

	for i, seg := range strings.Split(pat, "/") {
		// A slash must precede any segment after the first
		if i != 0 {
			patReg += "/"
			segments = append(segments, staticsegment("/"))
		}

		// If the segment is empty there's nothing else to do
		if seg == "" {
			continue
		}

		// If the segment starts with a ':' then it is a named segment
		if seg[0] == ':' {
			var segname string
			if seg[len(seg)-1] == '*' {
				segname = seg[1 : len(seg)-1]
				patReg += fmt.Sprintf("(?P<%s>(?:[^/]+/)*(?:[^/]+))", segname)
			} else {
				segname = seg[1:]
				patReg += fmt.Sprintf("(?P<%s>(?:[^/]+))", segname)
			}
			segments = append(segments, varsegment(segname))
		} else {
			segments = append(segments, staticsegment(seg))
			patReg += regexp.QuoteMeta(seg)
		}
	}
	patReg += "$"

	re, err := regexp.Compile(patReg)
	if err != nil {
		return nil, fmt.Errorf("error compiling regex for pattern (%s):\nRegex: %s\nError: %v", pat, patReg, err)
	}

	return &Mapper{
		re:  re,
		seg: segments,
	}, nil
}

// Parse extracts named segments from the path provided
// All segments must appear in the path
func (m Mapper) Parse(path string) (map[string]string, error) {
	// The regex is based on unix-style paths so we need to convert
	path = filepath.ToSlash(path)

	match := m.re.FindStringSubmatch(path)
	if len(match) == 0 {
		return nil, fmt.Errorf("path (%s) did not match regex (%v)", path, m.re)
	}

	out := make(map[string]string)
	// The first SubexpName is "" so we skip it
	for i, v := range m.re.SubexpNames()[1:] {
		// The first match is the full string so add 1
		out[v] = match[i+1]
	}
	return out, nil
}

// Generate produces a path from a map of segment values.
// All required values must be present
func (m Mapper) Generate(vars map[string]string) (string, error) {
	var gen string
	for _, s := range m.seg {
		seg, err := s.str(vars)
		if err != nil {
			return "", err
		}
		gen += seg
	}
	// Convert back to local path format at the end
	return filepath.FromSlash(gen), nil
}

type segment interface {
	str(map[string]string) (string, error)
}

type staticsegment string

func (s staticsegment) str(map[string]string) (string, error) {
	return string(s), nil
}

type varsegment string

func (v varsegment) str(p map[string]string) (string, error) {
	if s, ok := p[string(v)]; ok {
		return s, nil
	}
	return "", fmt.Errorf("path generation failure. Missing key: %s", v)
}
