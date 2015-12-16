/*
 * Copyright 2015 Google Inc. All rights reserved.
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

package gcs

import (
	"runtime"
	"testing"
)

func TestLiteralPrefix(t *testing.T) {
	tests := []struct {
		glob, prefix string
	}{
		{"", ""},
		{"hello/world.txt", "hello/world.txt"},
		{"*", ""},
		{"?", ""},
		{"[abc]", ""},
		{"prefix*blah", "prefix"},
		{"prefix?blah", "prefix"},
		{"prefix[abc]blah", "prefix"},
	}

	if runtime.GOOS == "windows" {
		tests = append(tests, struct{ glob, prefix string }{`some/path/a\*\?\[abc]`, `some/path/a\`})
	} else {
		tests = append(tests, struct{ glob, prefix string }{`some/path/a\*\?\[abc]`, "some/path/a*?[abc]"})
	}

	for _, test := range tests {
		if res := globLiteralPrefix(test.glob); res != test.prefix {
			t.Errorf("globLiteralPrefix(%q): expected %q; result %q", test.glob, test.prefix, res)
		}
	}
}
