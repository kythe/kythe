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

package edges

import (
	"testing"

	"kythe.io/kythe/go/test/testutil"
)

func TestParseOrdinal(t *testing.T) {
	type ordinalTest struct { // fields exported for the comparator
		Input, Kind string
		Ordinal     int
		HasOrdinal  bool
	}
	tests := []ordinalTest{
		{"/kythe/edge/defines", "/kythe/edge/defines", 0, false},
		{"/kythe/edge/kind.here", "/kythe/edge/kind.here", 0, false},
		{"/kythe/edge/kind1", "/kythe/edge/kind1", 0, false},
		{"kind.-1", "kind.-1", 0, false},

		{"kind.3", "kind", 3, true},
		{"/kythe/edge/param.0", "/kythe/edge/param", 0, true},
		{"/kythe/edge/param.1", "/kythe/edge/param", 1, true},
		{"%/kythe/edge/param.1", "%/kythe/edge/param", 1, true},
		{"/kythe/edge/kind.1930", "/kythe/edge/kind", 1930, true},
	}

	for _, test := range tests {
		kind, ord, ok := ParseOrdinal(test.Input)
		if err := testutil.DeepEqual(test, ordinalTest{
			Input:      test.Input,
			Kind:       kind,
			Ordinal:    ord,
			HasOrdinal: ok,
		}); err != nil {
			t.Errorf("ParseOrdinal(%q): %v", test.Input, err)
		}
	}
}

func TestCanonical(t *testing.T) {
	tests := []struct {
		input, want string
	}{
		{"", ""},
		{"/kythe/edge/childof", "/kythe/edge/childof"},
		{"%/kythe/edge/defines/binding", "/kythe/edge/defines/binding"},
	}
	for _, test := range tests {
		if got := Canonical(test.input); got != test.want {
			t.Errorf("Canonical(%q): got %q, want %q", test.input, got, test.want)
		}
	}
}

func TestDirections(t *testing.T) {
	tests := []struct {
		input     string
		isForward bool
	}{
		{"/kythe/edge/defines", true},
		{"%/kythe/edge/defines", false},
	}
	for _, test := range tests {
		if got := IsForward(test.input); got != test.isForward {
			t.Errorf("IsForward(%q): got %v, want %v", test.input, got, test.isForward)
		}
		if got := IsReverse(test.input); got == test.isForward {
			t.Errorf("IsReverse(%q): got %v, want %v", test.input, got, !test.isForward)
		}
	}
}

func TestMirroring(t *testing.T) {
	tests := []struct {
		input, want string
	}{
		{"/kythe/edge/a", "%/kythe/edge/a"},
		{"%/kythe/edge/b", "/kythe/edge/b"},
	}
	for _, test := range tests {
		got := Mirror(test.input)
		if got != test.want {
			t.Errorf("Mirror(%q): got %q, want %q", test.input, got, test.want)
		}
		if rev := Mirror(got); rev != test.input {
			t.Errorf("Mirror(Mirror(%q)): got %q, want %q", test.input, rev, test.input)
		}
	}
}

func TestIsVariant(t *testing.T) {
	tests := []struct {
		x, y string
		want bool
	}{
		{"/kythe/edge/a", "/kythe/edge/a", true},
		{"/kythe/edge/a", "/kythe/edge/b", false},
		{"/kythe/edge/a/b", "/kythe/edge/a", true},
		{"/kythe/edge/a/c", "/kythe/edge/a/b", false},
		{"%/kythe/edge/fwd", "%/kythe/edge/fwd", true},
		{"%/kythe/edge/fwd/sub", "%/kythe/edge/fwd", true},
		{"%/kythe/edge/fwd/sub/more", "%/kythe/edge/fwd", true},
	}
	for _, test := range tests {
		if got := IsVariant(test.x, test.y); got != test.want {
			t.Errorf("IsVariant(%q, %q): got %v, want %v", test.x, test.y, got, test.want)
		}
		if got := IsVariant(Mirror(test.x), Mirror(test.y)); got != test.want {
			t.Errorf("IsVariant(!%q, !%q): got %v, want %v", test.x, test.y, got, test.want)
		}
	}
}

func TestIsSimilarToReference(t *testing.T) {
	tests = []struct {
		kind string
		want bool
	}{
		{"/kythe/edge/provides", true},
		{"something/else", false},
	}

	for _, tc := range tests {
		got := IsSimilarToReference(tc.kind)
		if got != tc.want {
			t.Errorf("IsSimilartoReference(%q) = %v, want: %v", tc.kind, got, tc.want)
		}
	}
}

func TestParamIndex(t *testing.T) {
	tests := []string{"param.0", "param.1", "param.2", "param.3"}
	for i, test := range tests {
		want := "/kythe/edge/" + test
		if got := ParamIndex(i); got != want {
			t.Errorf("ParamIndex(%d): got %q, want %q", i, got, want)
		}
	}
}
