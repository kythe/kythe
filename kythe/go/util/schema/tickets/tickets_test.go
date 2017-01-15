/*
 * Copyright 2017 Google Inc. All rights reserved.
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

package tickets

import "testing"

func TestAnchorFile(t *testing.T) {
	tests := []struct {
		anchor, file string
	}{
		{"kythe://foo?path=bar?lang=c++", "kythe://foo?path=bar"},
		{"//foo?lang=go?path=bar#siggy", "kythe://foo?path=bar"},
		{"kythe:?root=apple?path=pear?lang=plum#cherry", "kythe:?path=pear?root=apple"},
	}
	for _, test := range tests {
		got, err := AnchorFile(test.anchor)
		if err != nil {
			t.Errorf("AnchorFile(%q): unexpected error: %v", test.anchor, err)
		} else if got != test.file {
			t.Errorf("AnchorFile(%q): got %q, want %q", test.anchor, got, test.file)
		}
	}
}

func TestBadAnchor(t *testing.T) {
	const input = "bogus"
	if got, err := AnchorFile(input); err == nil {
		t.Errorf("AnchorFile(%q): got %q, expected error", input, got)
	}
}
