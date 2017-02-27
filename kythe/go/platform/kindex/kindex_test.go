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

package kindex

import (
	"bytes"
	"log"
	"os"
	"strings"
	"testing"

	"github.com/golang/protobuf/proto"

	apb "kythe.io/kythe/proto/analysis_proto"
	spb "kythe.io/kythe/proto/storage_proto"
)

// A fakeFetcher maps file "paths" to their contents.
// The digest is ignored.
type fakeFetcher map[string]string

func (f fakeFetcher) Fetch(path, digest string) ([]byte, error) {
	data, ok := f[path]
	if !ok {
		return nil, os.ErrNotExist
	}
	return []byte(data), nil
}

func TestRoundTrip(t *testing.T) {
	data := fakeFetcher{
		"input 1": "Yesterday, upon the stair",
		"input 2": "I saw a man who wasn't there",
		"input 3": "He wasn't there again today",
		"input 4": "Oh, how I wish he'd go away",
	}
	unit := &apb.CompilationUnit{
		VName: &spb.VName{
			Corpus:    "test",
			Path:      "magic/test/unit",
			Signature: "成功",
		},
		Revision:         "1",
		WorkingDirectory: "/usr/local/src",
		RequiredInput: []*apb.CompilationUnit_FileInput{
			{Info: &apb.FileInfo{
				Path: "input 1",
			}},
			{Info: &apb.FileInfo{
				Path: "input 2",
			}},
			{Info: &apb.FileInfo{
				Path: "input 3",
			}},
		},
		Argument:   []string{"go", "ask", "your", "mother"},
		SourceFile: []string{"input 2"},
	}

	// Pack a unit and its data into a Compilation record.
	before, err := FromUnit(unit, data)
	if err != nil {
		t.Fatalf("FromUnit failed: %v\ninput: %+v", err, unit)
	}

	// Write the record out in wire format.
	var buf bytes.Buffer
	if _, err := before.WriteTo(&buf); err != nil {
		t.Fatalf("before.WriteTo failed: %v\ninput: %+v", err, before)
	}

	// Read the wire format back in.
	after, err := New(&buf)
	if err != nil {
		t.Fatalf("Read(buf) failed: %v\ninput: %q", err, buf.String())
	}

	// Verify that we got an equivalent proto message.
	if !proto.Equal(before.Proto, after.Proto) {
		t.Errorf("Unit protos do not match\nbefore: %+v\nafter:  %+v", before.Proto, after.Proto)
	}

	// Verify that we got the expected file contents.
	for _, ri := range after.Files {
		path := ri.GetInfo().Path
		got := string(ri.Content)
		want, ok := data[path]
		if !ok {
			t.Errorf("Unexpected input for path %q: %q", path, got)
		} else if got != want {
			t.Errorf("Wrong data for path %q: got %q, want %q", path, got, want)
		}
	}

	// Verify that we got everything we put in.
	if got, want := len(after.Files), len(before.Files); got != want {
		t.Errorf("Wrong number of files: got %d, want %d", got, want)
	}
}

func mustFD(path, data string) *apb.FileData {
	fd, err := FileData(path, strings.NewReader(data))
	if err != nil {
		log.Panicf("Unable to construct FileData for %q: %v", path, err)
	}
	return fd
}

func TestFetch(t *testing.T) {
	const magic = "xyzzy"
	const magicDigest = "184858a00fd7971f810848266ebcecee5e8b69972c5ffaed622f5ee078671aed"
	idx := &Compilation{
		Files: []*apb.FileData{
			mustFD("A", "A is for Amy, who fell down the stairs"),
			mustFD("B", "B is for Basil, assaulted by bears"),
			mustFD("C", "C is for Clara, who wasted away"),
			mustFD("D", "D is for Desmond, thrown out of a sleigh"),
			mustFD("E", "E is for Ernest, who choked on a peach"),
			mustFD("X", magic),
		},
	}
	numGood := len(idx.Files)

	// Basic path resolution should work, ignoring the digest.
	for i, path := range []string{"A", "B", "C", "D", "E", "X", "F", "G"} {
		got, err := idx.Fetch(path, "")
		if err != nil && i < numGood {
			t.Errorf("Fetch %q: unexpected error: %v", path, err)
		} else if err == nil && i >= numGood {
			t.Errorf("Fetch %q: unexpectedly succeeded with %q", path, got)
		}
	}

	// A path lookup with an incorrect digest should fail.
	if got, err := idx.Fetch("A", "bogusdigestvalue"); err == nil {
		t.Errorf("Fetch A with bogus digest: got %q, wanted error", got)
	}

	// A digest lookup on its own should work.
	if raw, err := idx.Fetch("", magicDigest); err != nil {
		t.Errorf("Fetch %q: unexpected error: %v", magicDigest, err)
	} else if got := string(raw); got != magic {
		t.Errorf("Fetch %q: got %q, want %q", magicDigest, got, magic)
	}
}
