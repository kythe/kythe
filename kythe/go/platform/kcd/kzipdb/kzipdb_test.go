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

package kzipdb

import (
	"bytes"
	"context"
	"sort"
	"strings"
	"sync"
	"testing"

	"kythe.io/kythe/go/platform/kcd"
	"kythe.io/kythe/go/platform/kcd/kythe"
	"kythe.io/kythe/go/platform/kzip"

	"github.com/google/go-cmp/cmp"

	apb "kythe.io/kythe/proto/analysis_go_proto"
	spb "kythe.io/kythe/proto/storage_go_proto"
)

var testInput struct {
	sync.Once

	data  []byte
	units []string
	files []string
}

func newUnit(key, lang, corpus string) *apb.CompilationUnit {
	return &apb.CompilationUnit{
		VName: &spb.VName{
			Corpus:   corpus,
			Language: lang,
		},
		OutputKey: key,
	}
}

func newIndex(revs ...string) *apb.IndexedCompilation_Index {
	return &apb.IndexedCompilation_Index{Revisions: revs}
}

func setup(t *testing.T) {
	t.Helper()
	testInput.Do(func() {

		buf := bytes.NewBuffer(nil)
		w, err := kzip.NewWriter(buf)
		if err != nil {
			t.Fatalf("NewWriter: %v", err)
		}
		for _, ic := range []*apb.IndexedCompilation{
			{Unit: newUnit("A", "go", "kythe"), Index: newIndex("123")},
			{Unit: newUnit("B", "c++", "kythe"), Index: newIndex("456", "789")},
			{Unit: newUnit("C", "go", "chromium"), Index: newIndex("123")},
			{Unit: newUnit("D", "protobuf", "kythe"), Index: newIndex("789")},
			{Unit: newUnit("E", "java", "boiler.plate"), Index: newIndex("666")},
			{Unit: newUnit("F", "java", "kythe")},
		} {
			digest, err := w.AddUnit(ic.Unit, ic.Index)
			if err != nil {
				t.Fatalf("AddUnit %v: %v", ic.Unit, err)
			}
			testInput.units = append(testInput.units, digest)
		}
		for _, src := range []string{
			"apple", "pear", "plum", "cherry",
		} {
			digest, err := w.AddFile(strings.NewReader(src))
			if err != nil {
				t.Fatalf("AddFile %q: %v", src, err)
			}
			testInput.files = append(testInput.files, digest)
		}
		if err := w.Close(); err != nil {
			t.Fatalf("Close: %v", err)
		}

		testInput.data = buf.Bytes()
	})
}

func TestReader(t *testing.T) {
	setup(t)
	r, err := kzip.NewReader(bytes.NewReader(testInput.data), int64(len(testInput.data)))
	if err != nil {
		t.Fatalf("NewReader failed: %v", err)
	}
	db := DB{Reader: r}
	ctx := context.Background()

	t.Run("NoMatchingRevs", func(t *testing.T) {
		db.Revisions(ctx, &kcd.RevisionsFilter{Corpus: "bogus"}, func(rev kcd.Revision) error {
			t.Errorf("Unexpected revision found: %v", rev)
			return nil
		})
	})
	t.Run("MatchSomeRevs", func(t *testing.T) {
		want := []kcd.Revision{
			{Revision: "123", Corpus: "kythe"},
			{Revision: "456", Corpus: "kythe"},
			{Revision: "789", Corpus: "kythe"},
		}
		var got []kcd.Revision
		db.Revisions(ctx, &kcd.RevisionsFilter{Corpus: "kythe"}, func(rev kcd.Revision) error {
			got = append(got, rev)
			return nil
		})
		sort.Slice(got, func(i, j int) bool {
			if c := strings.Compare(got[i].Revision, got[j].Revision); c == 0 {
				return got[i].Corpus < got[j].Corpus
			} else {
				return c < 0
			}
		})
		if diff := cmp.Diff(got, want); diff != "" {
			t.Errorf("Revisions did not match: diff is\n%s", diff)
		}
	})
	t.Run("FindGo", func(t *testing.T) {
		var numFound int
		db.Find(ctx, &kcd.FindFilter{Languages: []string{"go"}}, func(digest string) error {
			unit, err := r.Lookup(digest)
			if err != nil {
				t.Errorf("Lookup %q failed: %v", digest, err)
			} else if lang := unit.Proto.GetVName().GetLanguage(); lang != "go" {
				t.Errorf("Language for %q: got %q, wanted go", digest, lang)
			} else {
				numFound++
			}
			return nil
		})
		if numFound != 2 {
			t.Errorf("Find: got %d compilations, want 2", numFound)
		}
	})
	t.Run("ReadUnits", func(t *testing.T) {
		i := 0
		db.Units(ctx, testInput.units, func(digest, key string, data []byte) error {
			if want := testInput.units[i]; digest != want {
				t.Errorf("Unit %d digest: got %q, want %q", i, digest, want)
			}
			if key != kythe.Format {
				t.Errorf("Unit %d format: got %q, want %q", i, key, kythe.Format)
			}
			i++
			return nil
		})
	})
	t.Run("FailUnits", func(t *testing.T) {
		db.Units(ctx, testInput.files, func(digest, key string, data []byte) error {
			t.Errorf("Unexpected unit digest %q key %q data %q", digest, key, string(data))
			return nil
		})
	})
	t.Run("ReadFiles", func(t *testing.T) {
		i := 0
		db.Files(ctx, testInput.files, func(digest string, data []byte) error {
			if want := testInput.files[i]; digest != want {
				t.Errorf("File %d digest: got %q, want %q", i, digest, want)
			}
			i++
			return nil
		})
		i = 0
		db.FilesExist(ctx, testInput.files, func(digest string) error {
			if want := testInput.files[i]; digest != want {
				t.Errorf("File %d digest: got %q, want %q", i, digest, want)
			}
			i++
			return nil
		})
	})
	t.Run("FailFiles", func(t *testing.T) {
		db.Files(ctx, testInput.units, func(digest string, data []byte) error {
			t.Errorf("Unexpected file digest %q data %q", digest, string(data))
			return nil
		})
		db.FilesExist(ctx, testInput.units, func(digest string) error {
			t.Errorf("Unexpected file digest %q", digest)
			return nil
		})
	})
}
