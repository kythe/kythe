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

package inmemory

import (
	"context"
	"io"
	"sort"
	"testing"

	"kythe.io/kythe/go/storage/keyvalue"

	"github.com/google/go-cmp/cmp"
)

var ctx = context.Background()

func TestKeyValueDB_get(t *testing.T) {
	db := NewKeyValueDB()

	if val, err := db.Get(ctx, []byte("nonExistent"), nil); err == nil {
		t.Errorf("Found nonExistent value: %q", val)
	} else if err != io.EOF {
		t.Errorf("Unexpected error: %v", err)
	}

	write(t, db, "nonExistent", "val")

	if val, err := db.Get(ctx, []byte("nonExistent"), nil); err != nil {
		t.Errorf("Unexpected error: %v", err)
	} else if found := string(val); found != "val" {
		t.Errorf("Expected %q; found %q", "val", found)
	}

	if err := db.Close(ctx); err != nil {
		t.Fatalf("DB close error: %v", err)
	}
}

func TestKeyValueDB_overwrite(t *testing.T) {
	db := NewKeyValueDB()

	write(t, db, "key", "val")

	if val, err := db.Get(ctx, []byte("key"), nil); err != nil {
		t.Errorf("Unexpected error: %v", err)
	} else if found := string(val); found != "val" {
		t.Errorf("Expected %q; found %q", "val", found)
	}

	write(t, db, "key", "val2")

	if val, err := db.Get(ctx, []byte("key"), nil); err != nil {
		t.Errorf("Unexpected error: %v", err)
	} else if found := string(val); found != "val2" {
		t.Errorf("Expected %q; found %q", "val2", found)
	}

	if err := db.Close(ctx); err != nil {
		t.Fatalf("DB close error: %v", err)
	}
}

type entry struct{ Key, Value string }

func TestKeyValueDB_scanPrefix(t *testing.T) {
	db := NewKeyValueDB()

	entries := []entry{
		{"k1", "val1"},
		{"k4", "val4"},
		{"k3", "val3"},
		{"k0", "val0"},
		{"k2", "val2"},
		{"k", "val"},
	}
	writeEntries(t, db, entries)
	writeEntries(t, db, []entry{
		{"j1", "val1"},
		{"j0", "val0"},
	})

	it, err := db.ScanPrefix(ctx, []byte("k"), nil)
	if err != nil {
		t.Fatalf("ScanPrefix error: %v", err)
	}

	var found []entry
	for {
		k, v, err := it.Next()
		if err == io.EOF {
			break
		}
		found = append(found, entry{string(k), string(v)})
	}

	if err := it.Close(); err != nil {
		t.Fatalf("Iterator close error: %v", err)
	}

	sort.Slice(entries, func(i, j int) bool { return entries[i].Key < entries[j].Key })
	if diff := cmp.Diff(entries, found); diff != "" {
		t.Fatalf("Found entry differences: (- expected; + found)\n%s", diff)
	}

	if err := db.Close(ctx); err != nil {
		t.Fatalf("DB close error: %v", err)
	}
}

func TestKeyValueDB_scanRange(t *testing.T) {
	db := NewKeyValueDB()

	entries := []entry{
		{"k1", "val1"},
		{"k0", "val0"},
		{"k2", "val2"},
	}
	writeEntries(t, db, entries)
	writeEntries(t, db, []entry{
		{"k3", "val3"},
		{"k", "val"},
		{"j1", "val1"},
		{"k4", "val4"},
		{"j0", "val0"},
	})

	it, err := db.ScanRange(ctx, &keyvalue.Range{
		Start: []byte("k0"),
		End:   []byte("k3"),
	}, nil)
	if err != nil {
		t.Fatalf("ScanRange error: %v", err)
	}

	var found []entry
	for {
		k, v, err := it.Next()
		if err == io.EOF {
			break
		}
		found = append(found, entry{string(k), string(v)})
	}

	if err := it.Close(); err != nil {
		t.Fatalf("Iterator close error: %v", err)
	}

	sort.Slice(entries, func(i, j int) bool { return entries[i].Key < entries[j].Key })
	if diff := cmp.Diff(entries, found); diff != "" {
		t.Fatalf("Found entry differences: (- expected; + found)\n%s", diff)
	}

	if err := db.Close(ctx); err != nil {
		t.Fatalf("DB close error: %v", err)
	}
}

func TestKeyValueDB_scanPrefixSeek(t *testing.T) {
	db := NewKeyValueDB()

	entries := []entry{
		{"k4", "val4"},
		{"k3", "val3"},
		{"k2", "val2"},
	}
	writeEntries(t, db, entries)
	writeEntries(t, db, []entry{
		{"k", "val"},
		{"k0", "val0"},
		{"k1", "val1"},
		{"j1", "val1"},
		{"j0", "val0"},
	})

	it, err := db.ScanPrefix(ctx, []byte("k"), nil)
	if err != nil {
		t.Fatalf("ScanPrefix error: %v", err)
	}

	// Seek past k1 key
	if err := it.Seek([]byte("k10")); err != nil {
		t.Fatalf("Seek error: %v", err)
	}

	var found []entry
	for {
		k, v, err := it.Next()
		if err == io.EOF {
			break
		}
		found = append(found, entry{string(k), string(v)})
	}

	if err := it.Close(); err != nil {
		t.Fatalf("Iterator close error: %v", err)
	}

	sort.Slice(entries, func(i, j int) bool { return entries[i].Key < entries[j].Key })
	if diff := cmp.Diff(entries, found); diff != "" {
		t.Fatalf("Found entry differences: (- expected; + found)\n%s", diff)
	}

	if err := db.Close(ctx); err != nil {
		t.Fatalf("DB close error: %v", err)
	}
}

func TestKeyValueDB_scanRangeSeek(t *testing.T) {
	db := NewKeyValueDB()

	entries := []entry{
		{"k1", "val1"},
		{"k2", "val2"},
	}
	writeEntries(t, db, entries)
	writeEntries(t, db, []entry{
		{"k0", "val0"},
		{"k3", "val3"},
		{"k", "val"},
		{"j1", "val1"},
		{"k4", "val4"},
		{"j0", "val0"},
	})

	it, err := db.ScanRange(ctx, &keyvalue.Range{
		Start: []byte("k0"),
		End:   []byte("k3"),
	}, nil)
	if err != nil {
		t.Fatalf("ScanRange error: %v", err)
	}

	// Seek past k0 key
	if err := it.Seek([]byte("k1")); err != nil {
		t.Fatalf("Seek error: %v", err)
	}

	var found []entry
	for {
		k, v, err := it.Next()
		if err == io.EOF {
			break
		}
		found = append(found, entry{string(k), string(v)})
	}

	if err := it.Close(); err != nil {
		t.Fatalf("Iterator close error: %v", err)
	}

	sort.Slice(entries, func(i, j int) bool { return entries[i].Key < entries[j].Key })
	if diff := cmp.Diff(entries, found); diff != "" {
		t.Fatalf("Found entry differences: (- expected; + found)\n%s", diff)
	}

	if err := db.Close(ctx); err != nil {
		t.Fatalf("DB close error: %v", err)
	}
}

func writeEntries(t *testing.T, db *KeyValueDB, entries []entry) {
	for _, e := range entries {
		write(t, db, e.Key, e.Value)
	}
}

func write(t *testing.T, db *KeyValueDB, key, val string) {
	w, err := db.Writer(ctx)
	if err != nil {
		t.Fatalf("Writer error: %v", err)
	}

	if err := w.Write([]byte(key), []byte(val)); err != nil {
		t.Fatalf("Write error: %v", err)
	} else if err := w.Close(); err != nil {
		t.Fatalf("Write close error: %v", err)
	}
}
