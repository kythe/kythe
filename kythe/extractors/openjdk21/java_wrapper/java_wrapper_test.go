/*
 * Copyright 2019 The Kythe Authors. All rights reserved.
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
// Test for java_wrapper wraps the real java command to invoke the standalone java extractor
package main

import (
	"bufio"
	"strings"
	"testing"
)

func TestModuleNameFromPath(t *testing.T) {
	tests := []struct {
		path     string
		expected string
		err      string
	}{
		// No match found
		{
			"java/java.base/xyz.java \n foo/java.base/info.java",
			"",
			"no module found",
		},
		// Successful match
		{
			"foo/java.base/xyz.java \n java/java.base/module-info.java",
			"java.base",
			"",
		},
		// Multiple matches found
		{
			"foo/java.io/module-info.java \n java/java.base/module-info.java",
			"",
			"multiple modules found: java.io & java.base",
		},
	}

	for _, test := range tests {
		t.Run(test.path, func(t *testing.T) {
			actual, err := moduleNameFromPath(bufio.NewScanner(strings.NewReader(test.path)))
			if err != nil && err.Error() != test.err {
				t.Errorf("moduleNameFromPath() did not fail as expected. actual:%s, expected:%s", err.Error(), test.err)
			} else if actual != test.expected {
				t.Errorf("moduleName(%q) = %q, want %q", test.path, actual, test.expected)
			}
		})
	}
}
