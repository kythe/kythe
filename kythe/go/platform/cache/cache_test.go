/*
 * Copyright 2014 Google Inc. All rights reserved.
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

package cache

import (
	goflag "flag"
	"testing"
)

const fileData = "expected file contents"

type mockFetcher struct {
	path, digest string
}

func (m *mockFetcher) Fetch(path, digest string) ([]byte, error) {
	m.path, m.digest = path, digest
	return []byte(fileData), nil
}

func TestDelegation(t *testing.T) {
	// Verify that the compilation wrapper correctly delegates to the
	// underlying implementation of the Compilation interface.

	const filePath = "file/to/fetch"
	const fileDigest = "0123456789abcdef9876543210fedcba"
	var mock mockFetcher
	test := Fetcher(&mock, New(128))

	data, err := test.Fetch(filePath, fileDigest)
	if err != nil {
		t.Errorf("Fetch %q: unexpected error: %s", filePath, err)
	}
	if s := string(data); s != fileData {
		t.Errorf("Fetch %q: got %q, want %q", filePath, s, fileData)
	}
	if mock.path != filePath || mock.digest != fileDigest {
		t.Errorf("Fetch %q: delegate was not invoked correctly", filePath)
	}

	// Verify that the Fetch caching worked, i.e., we don't call the delegate a
	// second time given that the results do fit in the cache.

	mock.path, mock.digest = "", "" // Reset the delegate
	data, err = test.Fetch(filePath, fileDigest)
	if err != nil {
		t.Errorf("Fetch %q: unexpected error: %s", filePath, err)
	}
	if s := string(data); s != fileData {
		t.Errorf("Fetch %q: got %q, want %q", filePath, s, fileData)
	}
	if mock.path != "" {
		t.Errorf("Fetch %q: delegate was invoked again (%q)", filePath, mock.path)
	}
}

func TestCacheBounds(t *testing.T) {
	testData := map[string]string{
		"k1":   "abcd",
		"k2":   "123456",
		"k3":   "planet10",
		"last": "pdq",
	}

	// Construct a cache with a little less space than will fit all the keys in
	// the test data, so that the last key entered will force an eviction.
	totalSize := 0
	for _, v := range testData {
		totalSize += len(v)
	}
	totalSize -= len(testData["last"])

	// Put everything but the last key into the cache; as a result, the cache
	// should be at capacity.
	c := New(totalSize)
	for _, key := range []string{"k1", "k2", "k3"} {
		c.Put(key, []byte(testData[key]))
		t.Logf("Put %q, size now: %d bytes", key, c.curBytes)
	}
	if c.curBytes != c.maxBytes {
		t.Errorf("cache size: got %d, want %d", c.curBytes, c.maxBytes)
	}

	// Simulate some lookups.  After this, k3 should be the least frequently used,
	// and should be the first evicted key when we insert.
	for _, key := range []string{"k2", "x", "k1", "y", "k1", "k1", "z", "k2"} {
		v := c.Get(key)
		if key[0] == 'k' {
			if s := string(v); s != testData[key] {
				t.Errorf("Get %q: got %q, want %q", key, s, testData[key])
			}
		} else if v != nil {
			t.Errorf("Get %q: got %q, should be missing", key, string(v))
		}
	}

	for k, v := range c.data {
		t.Logf("Key %q entry %+v", k, v)
	}

	// Adding the last key should evict k3, and leave everything else alone,
	// including the one that was just added.
	c.Put("last", []byte(testData["last"]))
	if v := c.Get("k3"); v != nil {
		t.Errorf("Get %q: got %q, should be missing", "k3", string(v))
	}
	for _, key := range []string{"k1", "k2", "last"} {
		if v := c.Get(key); v == nil {
			t.Errorf("Get %q: is missing, want %q", key, testData[key])
		} else if s := string(v); s != testData[key] {
			t.Errorf("Get %q: got %q, want %q", key, s, testData[key])
		}
	}
}

func TestCacheCorners(t *testing.T) {
	if c := New(0); c != nil {
		t.Errorf("New 0: got %+v, want nil", c)
	}
	if c := New(-1); c != nil {
		t.Errorf("New -1: got %+v, want nil", c)
	}

	c := New(10)
	c.Put("a", []byte("abc"))
	c.Put("b", []byte("def"))
	c.Put("c", []byte("g"))

	// Trying to insert something too big to ever fit in the cache should not
	// evict any of the stuff that's already there.
	c.Put("huge", []byte("0123456789ABCDEF"))

	for _, key := range []string{"a", "b", "c"} {
		if c.Get(key) == nil {
			t.Errorf("Get %q: unexpectedly missing", key)
		}
	}
	if v := c.Get("huge"); v != nil {
		t.Errorf("Get %q: got %q, should be missing", "huge", string(v))
	}

	// Inserting something that just fits should evict everything else.
	c.Put("snug", []byte("0123456789"))
	if c.Get("snug") == nil {
		t.Errorf("Get %q: unexpectedly missing", "snug")
	}
	for _, key := range []string{"a", "b", "c"} {
		if v := c.Get(key); v != nil {
			t.Errorf("Get %q: got %q, should be missing", key, string(v))
		}
	}
}

// Verify that ParseByteSize works as intended.
func TestParseByteSize(t *testing.T) {
	// Enforce that *ByteSize implements the flag.Value interface for the Go flag package.
	var _ goflag.Value = (*ByteSize)(nil)

	tests := []struct {
		input string
		want  int
	}{
		// Valid responses
		{"0", 0},
		{"0G", 0},
		{"1", 1},
		{"5b", 5},
		{"101B", 101},
		{"1K", 1024},
		{"1.3k", 1331},
		{"2m", 2097152},
		{"2.5M", 2621440},
		{".25g", 268435456},
		{"0.125T", 137438953472},

		// Error conditions
		{"", -1},
		{"-523g", -1},
		{"11blah", -1},
	}
	for _, test := range tests {
		got, err := ParseByteSize(test.input)
		if err != nil && test.want >= 0 {
			t.Errorf("parse %q: unexpected error: %s", test.input, err)
		} else if got != test.want {
			t.Errorf("parse %q: got %d, want %d", test.input, got, test.want)
		}
	}
}
