/*
 * Copyright 2018 The Kythe Authors. All rights reserved.
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

// Package testutils provides useful testing helpers.
package testutils

import (
	"regexp"
	"strings"

	"github.com/google/go-cmp/cmp"
)

var multipleNewLines = regexp.MustCompile("\n{2,}")

// TrimmedEqual compares two strings after collapsing irrelevant whitespace at
// the beginning or end of lines. It returns both a boolean indicating equality,
// as well as any relevant diff.
func TrimmedEqual(got, want []byte) (bool, string) {
	// remove superfluous whitespace
	gotStr := strings.Trim(string(got[:]), " \n")
	wantStr := strings.Trim(string(want[:]), " \n")
	gotStr = multipleNewLines.ReplaceAllString(gotStr, "\n")
	wantStr = multipleNewLines.ReplaceAllString(wantStr, "\n")

	// diff want vs got
	diff := cmp.Diff(gotStr, wantStr)
	if diff != "" {
		return false, diff
	}

	return true, ""
}
