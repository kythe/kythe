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
	"strings"
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

func mustParse(s string) Size {
	sz, err := Parse(s)
	if err != nil {
		panic(err)
	}
	return sz
}

func TestRoundtrip(t *testing.T) {
	// Roundtripping through String() works for values when at most 2 decimal
	// place precision is needed.
	tests := []Size{
		0,
		1023,
		mustParse("1.02KiB"),
		mustParse("1.12MiB"),
	}

	for _, unit := range allUnits {
		tests = append(tests, unit)
	}

	for _, size := range tests {
		str := size.String()
		if found, err := Parse(str); err != nil {
			t.Errorf("Parse(%q.String()): error: %v", size, err)
		} else if found != size {
			t.Errorf("Parse(Size(%d).String()): expected: %d; found: %d", size, size, found)
		}
	}
}

func TestUnits(t *testing.T) {
	for i := 0; i < len(allUnits); i++ {
		if expected := unitSuffix(allUnits[i]); !strings.HasSuffix(allUnits[i].String(), expected) {
			t.Errorf("expected suffix: %s; found: %s", expected, allUnits[i])
		}
		if i != len(allUnits)-1 && allUnits[i] <= allUnits[i+1] {
			t.Errorf("%s >= %s", allUnits[i], allUnits[i+1])
		}
	}
}

func TestFloor(t *testing.T) {
	type test struct {
		sz       Size
		expected Size
	}
	tests := []test{
		{0, 0},
		{10, 10},

		{mustParse("1.02KiB"), Kibibyte},
		{mustParse("1.45MiB"), Mebibyte},
		{mustParse("1.32GiB"), Gibibyte},
		{mustParse("1.6PiB"), Pebibyte},

		{mustParse("1.02KB"), Kilobyte},
		{mustParse("1.02MB"), Megabyte},
		{mustParse("1.02GB"), Gigabyte},
		{mustParse("1.02PB"), Petabyte},
	}

	for _, unit := range allUnits {
		tests = append(tests, test{unit, unit})
	}

	for _, test := range tests {
		found := test.sz.Floor()
		if found != test.expected {
			t.Errorf("%d.Floor(): expected: %s; found: %s", test.sz, test.expected, found)
		}
		if strings.Contains(found.String(), ".") {
			t.Errorf("%d.Floor() == %s: contains decimal", test.sz, found)
		}
	}
}

func TestRound(t *testing.T) {
	type test struct {
		sz       Size
		expected Size
	}
	tests := []test{
		{0, 0},
		{mustParse("1.02KiB"), Kibibyte},
		{mustParse("1.9KiB"), 2 * Kibibyte},
		{mustParse("1.45MiB"), Mebibyte},
		{mustParse("4.32GiB"), 4 * Gibibyte},
		{mustParse("4.72GiB"), 5 * Gibibyte},
		{mustParse("2.62GiB"), 3 * Gibibyte},
		{mustParse("1.4PiB"), Pebibyte},
		{mustParse("1.6PiB"), 2 * Pebibyte},
	}

	for _, unit := range allUnits {
		tests = append(tests, test{unit, unit})
	}

	for _, test := range tests {
		found := test.sz.Round()
		if found != test.expected {
			t.Errorf("%d.Round(): expected: %s; found: %s", test.sz, test.expected, found)
		}
		if strings.Contains(found.String(), ".") {
			t.Errorf("%d.Round() == %s: contains decimal", test.sz, found)
		}
	}
}
