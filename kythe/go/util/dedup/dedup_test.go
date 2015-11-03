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

package dedup

import (
	"testing"

	"kythe.io/kythe/go/test/testutil"
)

func TestNewErrors(t *testing.T) {
	for i := -1; i < HashSize*2; i++ {
		_, err := New(i)
		if err == nil {
			t.Fatalf("Unexpected success of New(%d)", i)
		}
	}
}

func TestIsUnique(t *testing.T) {
	d, err := New(HashSize * 2)
	testutil.FatalOnErrT(t, "Error creating Deduper: %v", err)

	tests := []struct {
		val  string
		uniq bool
	}{
		{"a", true},
		{"a", false},
		{"a", false},
		{"b", true},
		{"a", false},
		{"b", false},
		{"c", true},
		// cache size exceeded (impl-specific behavior below)
		{"c", false},
		{"b", false},
		{"a", true},
		// cache size exceeded
		{"c", false},
		{"b", true},
	}

	var unique, duplicates uint64
	for _, test := range tests {
		uniq := d.IsUnique([]byte(test.val))
		if uniq != test.uniq {
			t.Fatalf("Expected IsUnique(%v) to be %s; found it wasn't", test.uniq, test.val)
		}
		if uniq {
			unique++
		} else {
			duplicates++
		}
		if found := d.Unique(); unique != found {
			t.Fatalf("Expected Unique() == %d; found %d", unique, found)
		}
		if found := d.Duplicates(); duplicates != found {
			t.Fatalf("Expected Duplicates() == %d; found %d", unique, found)
		}
	}
}
