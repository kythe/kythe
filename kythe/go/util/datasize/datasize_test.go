/*
 * Copyright 2015 The Kythe Authors. All rights reserved.
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

package datasize

import (
	"testing"

	"kythe.io/kythe/go/test/testutil"
)

func TestParse(t *testing.T) {
	tests := []struct {
		str string
		sz  Size
	}{
		{"0", 0},
		{"1024", 1024 * Byte},
		{"100", 100 * Byte},
		{"0tb", 0},
		{"1b", Byte},
		{"1kb", Kilobyte},
		{"1mb", Megabyte},
		{"1Gb", Gigabyte},
		{"1tb", Terabyte},
		{"1Pb", Petabyte},
		{"1KiB", Kibibyte},
		{"1mib", Mebibyte},
		{"1gib", Gibibyte},
		{"1tib", Tebibyte},
		{"1PiB", Pebibyte},
		{"4TB", 4 * Terabyte},
		{"43.5MB", Size(43.5 * float64(Megabyte))},
	}

	for _, test := range tests {
		found, err := Parse(test.str)
		testutil.FatalOnErrT(t, "Unexpected error: %v", err)
		if found != test.sz {
			t.Errorf("Parse(%q): expected: %s; found: %s", test.str, test.sz, found)
		}
	}
}

func TestString(t *testing.T) {
	tests := []struct {
		str string
		sz  Size
	}{
		{"0B", 0},
		{"1B", Byte},
		{"256B", 256 * Byte},
		{"256.50KiB", Size(256.5 * float64(Kibibyte))},

		{"1KiB", Kibibyte},
		{"1MiB", Mebibyte},
		{"1GiB", Gibibyte},
		{"1TiB", Tebibyte},
		{"1PiB", Pebibyte},

		{"1kB", Kilobyte},
		{"1MB", Megabyte},
		{"1GB", Gigabyte},
		{"1TB", Terabyte},
		{"1PB", Petabyte},
		{"2PB", 2 * Petabyte},
	}

	for _, test := range tests {
		if found := test.sz.String(); found != test.str {
			t.Errorf("%d.String(): expected: %s; found: %s", test.sz, test.str, found)
		}
	}
}
