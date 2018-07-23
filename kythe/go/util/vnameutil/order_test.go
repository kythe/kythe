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

package vnameutil

import (
	"testing"

	spb "kythe.io/kythe/proto/storage_go_proto"
)

func TestOrder(t *testing.T) {
	tests := []struct {
		a, b VString
		want int
	}{
		{"_____", "_____", 0},
		{"AB_DE", "AB_DE", 0},
		{"_____", "A____", -1},
		{"ABCDE", "ABCDF", -1},
		{"ABCDE", "ABCDD", 1},
		{"B____", "A____", 1},
		{"AAAA_", "AAAAA", -1},
		{"_ABCD", "_ACCD", -1},
	}

	for _, test := range tests {
		got := Compare(test.a.vname(), test.b.vname())
		if got != test.want {
			t.Errorf("Compare(%q, %q): got %d, want %d", test.a, test.b, got, test.want)
		}
	}
}

// VString is a compact representation of a vname for test construction.
//
// Valid values have 1 character per vname field: corpus, lang, path, root,
// sig.  The field value is that character, except underscore (_) which is
// turned into an empty string. Missing letters on the right are blanks.
type VString string

func (v VString) vname() *spb.VName {
	var out spb.VName
	ptrs := [...]*string{&out.Corpus, &out.Language, &out.Path, &out.Root, &out.Signature}

	for i, ch := range v {
		ptr := ptrs[i]
		if ch == '_' {
			*ptr = ""
		} else {
			*ptr = string(ch)
		}
	}
	return &out
}
