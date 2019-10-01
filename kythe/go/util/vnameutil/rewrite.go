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

// Package vnameutil provides utilities for generating consistent VNames from
// common path-like values (e.g., filenames, import paths).
package vnameutil // import "kythe.io/kythe/go/util/vnameutil"

import (
	"encoding/json"
	"fmt"
	"io/ioutil"
	"regexp"
	"strings"

	spb "kythe.io/kythe/proto/storage_go_proto"
)

// A Rule associates a regular expression pattern with a VName template.  A
// Rule can be applied to a string to produce a VName.
type Rule struct {
	*regexp.Regexp // A pattern to match against an input string
	*spb.VName     // A template to populate with matches from the input
}

// Apply reports whether input matches the regexp associated with r.  If so, it
// returns a VName whose fields have values taken from r.VName, with submatches
// populated from the input string.
//
// Submatch replacement is done using regexp.ExpandString, so the same syntax
// is supported for specifying replacements.
func (r Rule) Apply(input string) (*spb.VName, bool) {
	m := r.FindStringSubmatchIndex(input)
	if m == nil {
		return nil, false
	}
	return &spb.VName{
		Corpus:    r.expand(m, input, r.Corpus),
		Path:      r.expand(m, input, r.Path),
		Root:      r.expand(m, input, r.Root),
		Signature: r.expand(m, input, r.Signature),
	}, true
}

func (r Rule) expand(match []int, input, template string) string {
	return string(r.ExpandString(nil, template, input, match))
}

// Rules are an ordered set of rewriting rules.  Applying a group of rules
// tries each rule in sequence, and returns the result of the first one that
// matches.
type Rules []Rule

// Apply applies each rule in to the input in sequence, returning the first
// successful match.  If no rules apply, returns (nil, false).
func (r Rules) Apply(input string) (*spb.VName, bool) {
	for _, rule := range r {
		if v, ok := rule.Apply(input); ok {
			return v, true
		}
	}
	return nil, false
}

// ApplyDefault acts as r.Apply, but returns v there is no matching rule.
func (r Rules) ApplyDefault(input string, v *spb.VName) *spb.VName {
	if hit, ok := r.Apply(input); ok {
		return hit
	}
	return v
}

func convertRule(r *spb.VNameRewriteRule) (Rule, error) {
	pattern := "^" + strings.TrimSuffix(strings.TrimPrefix(r.Pattern, "^"), "$") + "$"
	re, err := regexp.Compile(pattern)
	if err != nil {
		return Rule{}, fmt.Errorf("invalid regular expression: %v", err)
	}
	return Rule{
		Regexp: re,
		VName: &spb.VName{
			Corpus:    fixTemplate(r.Vname.GetCorpus()),
			Path:      fixTemplate(r.Vname.GetPath()),
			Root:      fixTemplate(r.Vname.GetRoot()),
			Language:  fixTemplate(r.Vname.GetLanguage()),
			Signature: fixTemplate(r.Vname.GetSignature()),
		},
	}, nil
}

var fieldRE = regexp.MustCompile(`@(\w+)@`)

// fixTemplate rewrites @x@ markers in the template to the ${x} markers used by
// the regexp.Expand function, to simplify rewriting.
func fixTemplate(s string) string {
	if s == "" {
		return ""
	}
	return fieldRE.ReplaceAllStringFunc(strings.Replace(s, "$", "$$", -1),
		func(s string) string {
			return "${" + strings.Trim(s, "@") + "}"
		})
}

// ParseRules parses Rules from JSON-encoded data in the following format:
//
//   [
//     {
//       "pattern": "re2_regex_pattern",
//       "vname": {
//         "corpus": "corpus_template",
//         "root": "root_template",
//         "path": "path_template"
//       }
//     }, ...
//   ]
//
// Each pattern is an RE2 regexp pattern.  Patterns are implicitly anchored at
// both ends.  The template strings may contain markers of the form @n@, that
// will be replaced by the n'th regexp group on a successful input match.
func ParseRules(data []byte) (Rules, error) {
	var rr []*spb.VNameRewriteRule
	if err := json.Unmarshal(data, &rr); err != nil {
		return nil, err
	}
	var rules Rules
	for _, vr := range rr {
		r, err := convertRule(vr)
		if err != nil {
			return nil, err
		}
		rules = append(rules, r)
	}
	return rules, nil
}

// LoadRules loads and parses the vname mapping rules in path.
// If path == "", this returns nil without error (no rules).
func LoadRules(path string) (Rules, error) {
	if path == "" {
		return nil, nil
	}
	data, err := ioutil.ReadFile(path)
	if err != nil {
		return nil, fmt.Errorf("reading vname rules: %v", err)
	}
	rules, err := ParseRules(data)
	if err != nil {
		return nil, fmt.Errorf("parsing vname rules: %v", err)
	}
	return rules, nil
}
