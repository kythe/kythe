/*
 * Copyright 2016 The Kythe Authors. All rights reserved.
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

package kcd

import (
	"regexp"
	"testing"
	"time"
)

func TestHexDigest(t *testing.T) {
	// SHA256 test vectors from http://www.nsrl.nist.gov/testdata/
	// We're using these just to make sure we actually get SHA256.
	tests := map[string]string{
		"":    "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855",
		"abc": "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad",
		"abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq": "248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1",
	}
	for input, want := range tests {
		if got := HexDigest([]byte(input)); got != want {
			t.Errorf("HexDigest(%q): got %q, want %q", input, got, want)
		}
	}
}

func regexps(exprs ...string) (res []*regexp.Regexp) {
	for _, expr := range exprs {
		res = append(res, regexp.MustCompile(expr))
	}
	return
}

func TestIsEmpty(t *testing.T) {
	tests := []struct {
		input *FindFilter
		want  bool
	}{
		{nil, true},
		{new(FindFilter), true},
		{&FindFilter{Revisions: []string{}}, true},
		{&FindFilter{Languages: []string{""}}, false},
		{&FindFilter{Sources: regexps("foo")}, false},
		{&FindFilter{Outputs: regexps("foo")}, false},
		{&FindFilter{BuildCorpus: []string{"google3", "foo", "bar"}}, false},
	}
	for _, test := range tests {
		if got := test.input.IsEmpty(); got != test.want {
			t.Errorf("%v.IsEmpty(): got %v, want %v", test.input, got, test.want)
		}
	}
}

func TestMatcher(t *testing.T) {
	// The matcher function is an internal detail, but it provides the kernel
	// of functionality for several other pieces of the package so testing this
	// is a convenient way to cover the most important bits.
	tests := []struct {
		exprs  []string
		inputs []string
		quote  func(string) string
		want   bool
	}{
		// Fallback on empty exprs should work, with or without inputs.
		{want: true},
		{inputs: []string{"xyz"}, want: true},
		{exprs: []string{}, want: true},
		{exprs: []string{}, inputs: []string{"pdq"}, want: true},

		{ // No values at all; no match.
			exprs: []string{"a", "b", "c"}},

		{ // Some values, but no match.
			exprs:  []string{"needle"},
			inputs: []string{"hay", "stack"}},

		{ // Some values and a match.
			exprs:  []string{"needle"},
			inputs: []string{"a", "needle", "is", "here"},
			want:   true},

		{ // Make sure quoting works.
			exprs:  []string{"foo"},
			inputs: []string{"xfoox"},
			quote:  func(s string) string { return "x" + s + "x" },
			want:   true},

		{ // Make sure quoting works, negative case.
			exprs:  []string{"foo"},
			inputs: []string{"foo"},
			quote:  func(s string) string { return "bollocks" }},

		{ // Another negative case for quoting.
			exprs:  []string{"f.*g"},
			inputs: []string{"foooog"},
			quote:  regexp.QuoteMeta},

		{ // Matching with regexp operators.
			exprs:  []string{"w[aeiou]{2,}"},
			inputs: []string{"waaaaooooouuuu"},
			want:   true},
	}

	for _, test := range tests {
		m, err := matcher(test.exprs, test.quote)
		if err != nil {
			t.Errorf("Matcher for %+v: unexpected error: %v [broken test]", test, err)
			continue
		}
		if got := m(test.inputs...); got != test.want {
			t.Errorf("Match failed: got %v, want %v", got, test.want)
			t.Logf("Exprs: %+q\nInput: %+q", test.exprs, test.inputs)
		}
	}
}

func TestCombineRegexps(t *testing.T) {
	res := []*regexp.Regexp{
		regexp.MustCompile(`^(foo|bar)$`),
		regexp.MustCompile(`.*xxx.*`),
	}
	match, err := combineRegexps(res)
	if err != nil {
		t.Fatalf("combineRegexps %+v: unexpected error: %v", res, err)
	}
	for _, yes := range []string{"foo", "bar", "xxx", "axxxb", "fooxxxbar"} {
		if !match(yes) {
			t.Errorf("match(%q): got false, want true", yes)
		}
	}
	for _, no := range []string{"food", "barf", "xyxx", "xxfooxx"} {
		if match(no) {
			t.Errorf("match(%q): got true, want false", no)
		}
	}
}

func TestRevMatch(t *testing.T) {
	tests := []struct {
		// Pieces of the filter
		filterRev, filterCorpus string
		until                   time.Time

		// Input to test against the filter
		inputRev, inputCorpus string
		timestamp             time.Time

		// Should it match?
		want bool
	}{
		// An empty filter matches everything.
		{want: true},
		{inputRev: "xyzzy", want: true},
		{inputRev: "plugh", inputCorpus: "deedle", want: true},
		{inputCorpus: "kitteh!", want: true},
		{inputRev: "zardoz", timestamp: time.Unix(1940, 204), want: true},

		// Exact match on the revision string.
		{filterRev: "xyz pdq", inputRev: "xyz pdq", want: true},
		{filterRev: "保護者", inputRev: "watcher", want: false},

		// Regexp match on the revision string.
		{filterRev: `1.*`, inputRev: "12345", inputCorpus: "alpha", want: true},
		{filterRev: `\d+`, inputRev: "12345", inputCorpus: "bravo", want: true},
		{filterRev: `(?i)[aeiou]lf`, inputRev: "Elf", want: true},

		// Exact match on the corpus string.
		{filterCorpus: "alpha", inputRev: "12345", inputCorpus: "alpha", want: true},
		{filterCorpus: "alpha", inputRev: "12345", inputCorpus: "bravo", want: false},
		{filterRev: `\d+`, filterCorpus: "alpha", inputRev: "12345", inputCorpus: "alpha", want: true},
		{filterRev: `1{5}`, filterCorpus: "alpha", inputRev: "12345", want: false},

		// Corpus doesn't accept regexp matches.
		{filterCorpus: `p(ie|ea)ces?`, inputRev: "q", inputCorpus: "pieces", want: false},

		// Timestamp constraints.
		{until: time.Unix(25, 0), inputRev: "boozle", timestamp: time.Unix(25, 1), want: false},
		{until: time.Unix(25, 0), inputRev: "beezle", timestamp: time.Unix(24, 0), want: true},
	}

	for _, test := range tests {
		filter := &RevisionsFilter{
			Revision: test.filterRev,
			Corpus:   test.filterCorpus,
			Until:    test.until,
		}
		matches, err := filter.Compile()
		if err != nil {
			t.Errorf("Compile %+v failed: %v", filter, err)
			continue
		}
		rev := Revision{test.inputRev, test.inputCorpus, test.timestamp}
		if got := matches(rev); got != test.want {
			t.Errorf("Match failed: got %v, want %v\nFilter: %+v\nQuery:  %+v", got, test.want,
				filter, rev)
		}
	}
}

func TestIsValidDigest(t *testing.T) {
	tests := []struct {
		input string
		want  bool
	}{
		// String too short.
		{"", false},
		{"e3b0c4429", false},
		{"E3B0C4429", false},
		{"x y z", false},
		{"<dir>", false},

		// Invalid characters in the digest.
		{"e3b-c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855", false},
		{"e3b0c?4298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b85+", false},
		{"all your base are belong to us you have no chance to survive ha!", false},

		// Correct format, correct case.
		{"e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855", true},
		{"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa", true},
		{"0000000000000000000000000000000000000000000000000000000000000000", true},

		// Correct format, but case matters.
		{"E3B0C44298FC1C149AFBF4C8996FB92427AE41E4649B934CA495991B7852B855", false},
	}
	for _, test := range tests {
		got := IsValidDigest(test.input)
		if got != test.want {
			t.Errorf("IsValidDigest(%q): got %v, want %v", test.input, got, test.want)
		}
	}
}
