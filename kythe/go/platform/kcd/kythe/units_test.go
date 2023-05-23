/*
 * Copyright 2016 The Kythe Authors. All rights reserved.
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

package kythe

import (
	"io/ioutil"
	"path/filepath"
	"reflect"
	"sort"
	"testing"

	"kythe.io/kythe/go/test/testutil"
	"kythe.io/kythe/go/util/compare"
	"kythe.io/kythe/go/util/ptypes"

	"google.golang.org/protobuf/encoding/prototext"

	anypb "github.com/golang/protobuf/ptypes/any"
	apb "kythe.io/kythe/proto/analysis_go_proto"
	bipb "kythe.io/kythe/proto/buildinfo_go_proto"
	spb "kythe.io/kythe/proto/storage_go_proto"
)

func TestIndexing(t *testing.T) {
	buildInfo, err := ptypes.MarshalAny(&bipb.BuildDetails{
		BuildTarget: "T",
	})
	if err != nil {
		t.Fatalf("Error marshaling build details: %v", err)
	}
	unit := Unit{Proto: &apb.CompilationUnit{
		VName: &spb.VName{
			Signature: "false target",
			Language:  "c++",
		},
		RequiredInput: []*apb.CompilationUnit_FileInput{{
			VName: &spb.VName{Path: "p1"},
			Info:  &apb.FileInfo{Path: "../p1", Digest: "d1"},
		}, {
			Info: &apb.FileInfo{Path: "p2", Digest: "d2"},
		}},
		SourceFile: []string{"S"},
		OutputKey:  "O",
		Details:    []*anypb.Any{buildInfo},
	}}

	idx := unit.Index()
	sort.Strings(idx.Inputs)
	if got, want := idx.Language, "c++"; got != want {
		t.Errorf("%T Language: got %q, want %q", unit, got, want)
	}
	if got, want := idx.Output, "O"; got != want {
		t.Errorf("%T Output: got %q, want %q", unit, got, want)
	}
	if got, want := idx.Inputs, []string{"d1", "d2"}; !reflect.DeepEqual(got, want) {
		t.Errorf("%T Inputs: got %+q, want %+q", unit, got, want)
	}
	if got, want := idx.Sources, []string{"S"}; !reflect.DeepEqual(got, want) {
		t.Errorf("%T Sources: got %+q, want %+q", unit, got, want)
	}
	if got, want := idx.Target, "T"; got != want {
		t.Errorf("%T Target: got %q, want %q", unit, got, want)
	}
}

func TestCanonicalization(t *testing.T) {
	unit := Unit{Proto: &apb.CompilationUnit{
		RequiredInput: []*apb.CompilationUnit_FileInput{
			{Info: &apb.FileInfo{Digest: "B"}},
			{Info: &apb.FileInfo{Digest: "C"}},
			{Info: &apb.FileInfo{Digest: "A"}},
			{Info: &apb.FileInfo{Digest: "C"}},
			{Info: &apb.FileInfo{Digest: "A"}},
		},
		SourceFile: []string{"C", "A", "B"},
		Environment: []*apb.CompilationUnit_Env{
			{Name: "B"}, {Name: "A"}, {Name: "C"},
		},
		Details: []*anypb.Any{
			{TypeUrl: "C"}, {TypeUrl: "B"}, {TypeUrl: "A"},
		},
	}}
	unit.Canonicalize()
	tests := []struct {
		name  string
		value any
	}{
		{"required inputs", unit.Proto.RequiredInput},
		{"source files", unit.Proto.SourceFile},
		{"environment variables", unit.Proto.Environment},
		{"details", unit.Proto.Details},
	}
	want := []string{"A", "B", "C"}
	for _, test := range tests {
		got := keys(test.value)
		if !reflect.DeepEqual(got, want) {
			t.Errorf("Keys for %s: got %+q, want %+q", test.name, got, want)
		}
	}
}

func TestLookupVName(t *testing.T) {
	unit := Unit{Proto: &apb.CompilationUnit{
		VName: &spb.VName{Corpus: "DefaultCorpus", Root: "DefaultRoot"},
		RequiredInput: []*apb.CompilationUnit_FileInput{
			{Info: &apb.FileInfo{Path: "/absolute/path/file.go"},
				VName: &spb.VName{Corpus: "Corpus", Root: "Root", Path: "abspath/file.go"},
			},
			{Info: &apb.FileInfo{Path: "/build/absolute/path/file.go"},
				VName: &spb.VName{Corpus: "Corpus", Root: "Root", Path: "buildrelpath/file.go"},
			},
			{Info: &apb.FileInfo{Path: "relative/file.go"},
				VName: &spb.VName{Corpus: "Corpus", Root: "Root", Path: "relpath/file.go"},
			},
			{Info: &apb.FileInfo{Path: "missing/vname/corpus/file.go"},
				VName: &spb.VName{Path: "missing/corpus/file.go"},
			},
			{Info: &apb.FileInfo{Path: "missing/vname/path/file.go"},
				VName: &spb.VName{Corpus: "Corpus", Root: "Root"},
			},
		},
		WorkingDirectory: "/build",
	}}
	tests := []struct {
		path string
		want *spb.VName
	}{
		{"missing/file.go", nil},
		{"/absolute/path/file.go", &spb.VName{Corpus: "Corpus", Root: "Root", Path: "abspath/file.go"}},
		{"/build/absolute/path/file.go", &spb.VName{Corpus: "Corpus", Root: "Root", Path: "buildrelpath/file.go"}},
		{"relative/file.go", &spb.VName{Corpus: "Corpus", Root: "Root", Path: "relpath/file.go"}},
		{"./relative/file.go", &spb.VName{Corpus: "Corpus", Root: "Root", Path: "relpath/file.go"}},
		{"/build/relative/file.go", &spb.VName{Corpus: "Corpus", Root: "Root", Path: "relpath/file.go"}},
		{"missing/vname/corpus/file.go", &spb.VName{Corpus: "DefaultCorpus", Root: "DefaultRoot", Path: "missing/corpus/file.go"}},
		{"missing/vname/path/file.go", &spb.VName{Corpus: "Corpus", Root: "Root", Path: "missing/vname/path/file.go"}},
	}
	for _, test := range tests {
		got := unit.LookupVName(test.path)
		if diff := compare.ProtoDiff(test.want, got); diff != "" {
			t.Errorf("(-expected; +found):\n%s", diff)
		}
	}
}

func keys(v any) (keys []string) {
	switch t := v.(type) {
	case []*apb.CompilationUnit_FileInput:
		for _, input := range t {
			keys = append(keys, input.Info.GetDigest())
		}
	case []string:
		return t
	case []*apb.CompilationUnit_Env:
		for _, env := range t {
			keys = append(keys, env.Name)
		}
	case []*anypb.Any:
		for _, any := range t {
			keys = append(keys, any.TypeUrl)
		}
	}
	return
}

const testDataDir = "../../../../testdata/platform"

func TestDigest(t *testing.T) {
	tests := []string{
		"56bf5044e1b5c4c1cc7c4b131ac2fb979d288460e63352b10eef80ca35bd0a7b",
		"e9e170dcfca53c8126755bbc8b703994dedd3af32584291e01fba164ab5d3f32",
		"bb761979683e7c268e967eb5bcdedaa7fa5d1d472b0826b00b69acafbaad7ee6",
	}

	for _, test := range tests {
		b, err := ioutil.ReadFile(testutil.TestFilePath(t, filepath.Join(testDataDir, test+".pbtxt")))
		if err != nil {
			t.Fatalf("error reading test proto: %v", err)
		}
		var unit apb.CompilationUnit
		if err := prototext.Unmarshal(b, &unit); err != nil {
			t.Fatalf("error unmarshaling proto: %v", err)
		}
		got := Unit{Proto: &unit}.Digest()
		if got != test {

			t.Errorf("Digest: got %q, want %q\nInput: %+v", got, test, unit)
		}
	}
}
