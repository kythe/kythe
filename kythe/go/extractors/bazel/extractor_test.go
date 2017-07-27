/*
 * Copyright 2017 Google Inc. All rights reserved.
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

package bazel

import (
	"context"
	"io"
	"io/ioutil"
	"os"
	"path/filepath"
	"reflect"
	"regexp"
	"strings"
	"testing"

	"github.com/golang/protobuf/proto"

	"kythe.io/kythe/go/platform/kindex"
	"kythe.io/kythe/go/util/ptypes"

	apb "kythe.io/kythe/proto/analysis_proto"
	bipb "kythe.io/kythe/proto/buildinfo_proto"
	spb "kythe.io/kythe/proto/storage_proto"
	xapb "kythe.io/third_party/bazel/extra_actions_base_proto"
)

// The digest of an empty input, cf. openssl sha256 /dev/null
const emptyDigest = "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855"

func TestExtractor(t *testing.T) {
	const (
		testCorpus = "test/corpus"
		testTarget = "//target"
		testLang   = "foo"
		testOutput = "outfile"
		wantDigest = emptyDigest
	)

	// Gin up an exra action record with some known fields, and make sure the
	// extractor handles them correctly.
	xa := &xapb.ExtraActionInfo{
		Owner:    proto.String(testTarget),
		Mnemonic: proto.String("SomeAction"),
	}
	si := &xapb.SpawnInfo{
		Argument:   []string{"cc", "-o", testOutput, "-c", "2.src", "4.src"},
		InputFile:  []string{"1.dep", "2.src", "3.dep", "4.src"},
		OutputFile: []string{testOutput, "garbage"},
		Variable: []*xapb.EnvironmentVariable{{
			Name:  proto.String("PATH"),
			Value: proto.String("p1:p2"),
		}, {
			Name:  proto.String("BOGUS"),
			Value: proto.String("should not be seen"),
		}},
	}
	if err := proto.SetExtension(xa, xapb.E_SpawnInfo_SpawnInfo, si); err != nil {
		t.Fatalf("Error setting extension on XA: %v", err)
	}

	// These values are set by the test hooks to verify the plumbing does the
	// expected things.
	var (
		gotInfo       *ActionInfo
		checkedInputs []string
		checkedEnv    []string
		gotUnit       *kindex.Compilation
	)
	config := &Config{
		Corpus:   testCorpus,
		Language: testLang,

		CheckAction: func(_ context.Context, info *ActionInfo) error {
			gotInfo = info
			return nil
		},

		CheckInput: func(path string) (string, bool) {
			checkedInputs = append(checkedInputs, path)
			return path, true
		},

		CheckEnv: func(name, value string) bool {
			checkedEnv = append(checkedEnv, name)
			return name != "BOGUS"
		},

		IsSource: func(path string) bool { return filepath.Ext(path) == ".src" },
		Fixup:    func(unit *kindex.Compilation) error { gotUnit = unit; return nil },

		// All the files are empty, and all the children are above average.
		OpenRead: func(_ context.Context, path string) (io.ReadCloser, error) {
			return ioutil.NopCloser(strings.NewReader("")), nil
		},
	}

	t.Log("Extra action info:\n", proto.MarshalTextString(xa))

	ai, err := SpawnAction(xa)
	if err != nil {
		t.Fatalf("Invalid extra action info: %v", err)
	}
	cu, err := config.Extract(context.Background(), ai)
	if err != nil {
		t.Errorf("Error in extraction: %v", err)
	}

	// Verify that the fixup callback got passed the unit that was returned.
	if cu != gotUnit {
		t.Errorf("Wrong unit passed to fixup: got %p, want %p", gotUnit, cu)
	}

	// Verify that the info check callback was invoked.
	wantInfo := &ActionInfo{ // N.B.: Values prior to filtering!
		Target:    testTarget,
		Arguments: []string{"cc", "-o", testOutput, "-c", "2.src", "4.src"},
		Inputs:    []string{"1.dep", "2.src", "3.dep", "4.src"},
		Outputs:   []string{testOutput, "garbage"},
		Environment: map[string]string{
			"PATH":  "p1:p2",
			"BOGUS": "should not be seen",
		},
	}
	if gotInfo == nil {
		t.Error("SpawnInfo not reported")
	} else if !reflect.DeepEqual(gotInfo, wantInfo) {
		t.Errorf("Wrong SpawnInfo reported:\n got %+v\nwant %+v", gotInfo, wantInfo)
	}

	// Verify that the inputs were all passed to the callback.
	if !reflect.DeepEqual(checkedInputs, si.InputFile) {
		t.Errorf("Wrong input files checked:\n got %+q\nwant %+q", checkedInputs, si.InputFile)
	}

	// Verify that the identified sources were correctly propagated.
	if want := []string{"2.src", "4.src"}; !reflect.DeepEqual(cu.Proto.SourceFile, want) {
		t.Errorf("Wrong source files:\n got %+q\nwant %+q", cu.Proto.SourceFile, want)
	}

	// Verify that the argument list was correctly propagated.
	if !reflect.DeepEqual(cu.Proto.Argument, si.Argument) {
		t.Errorf("Wrong argument list:\n got %+q\nwant %+q", cu.Proto.Argument, si.Argument)
	}

	// Check that the required inputs and file data have the expected metadata.
	for i, ri := range cu.Proto.RequiredInput {
		vname := &spb.VName{Corpus: testCorpus, Path: ri.Info.Path}
		if !proto.Equal(ri.VName, vname) {
			t.Errorf("Required input %d: wrong vname: got %+v, want %+v", i+1, ri.VName, vname)
		}
		if got := ri.Info.Digest; got != wantDigest {
			t.Errorf("Required input %d: wrong digest: got %q, want %q", i+1, got, wantDigest)
		}
	}
	for i, fd := range cu.Files {
		if got := fd.Info.Digest; got != wantDigest {
			t.Errorf("File data %d: wrong digest: got %q, want %q", i+1, got, wantDigest)
		}
		if got := string(fd.Content); got != "" {
			t.Errorf("File data %d: wrong content: got %q, want empty", i+1, got)
		}
	}
	if a, b := len(cu.Files), len(cu.Proto.RequiredInput); a != b {
		t.Errorf("File count mismatch: %d file data, %d required inputs", a, b)
	}

	// Check that the build details got plumbed.
	if dets := cu.Proto.Details; len(dets) == 0 {
		t.Error("Missing build details")
	} else {
		var got bipb.BuildDetails
		if err := ptypes.UnmarshalAny(dets[0], &got); err != nil {
			t.Errorf("Error unmarshaling build details: %v", err)
		} else if want := (&bipb.BuildDetails{BuildTarget: testTarget}); !proto.Equal(&got, want) {
			t.Errorf("Wrong build details:\n got %+v\nwant %+v", &got, want)
		}
	}
}

func TestFetchInputs(t *testing.T) {
	tmp, err := ioutil.TempDir("", "TestFetchInputs")
	if err != nil {
		t.Fatalf("Error creating temp directory: %v", err)
	}
	defer os.RemoveAll(tmp) // best effort

	goodFile := filepath.Join(tmp, "goodto.go")
	if err := ioutil.WriteFile(goodFile, nil, 0755); err != nil {
		t.Fatalf("Creating test file: %v", err)
	}

	var cfg Config
	fd, err := cfg.FetchInputs(context.Background(), []string{
		goodFile,
		filepath.Join(tmp, "bad.h"), // does not exist
	})

	// There should have been an error, because one of the requested files does
	// not exist. However, despite that error we should have gotten data for
	// the other file.
	if err == nil {
		t.Error("FetchInputs was expected to report an error, but did not")
	} else if len(fd) != 2 {
		t.Fatalf("FetchInputs: got %d files, wanted %d", len(fd), 2)
	}

	// There should be as many entries in the result as in the input; any that
	// were not successfully loaded should be nil.
	want := &apb.FileData{
		Content: nil,
		Info: &apb.FileInfo{
			Path:   goodFile,
			Digest: emptyDigest,
		},
	}
	if !proto.Equal(fd[0], want) {
		t.Errorf("FileData[0]: got %+v, want %+v", fd[0], want)
	}
	if fd[1] != nil {
		t.Errorf("FileData[1]: got %+v, want nil", fd[1])
	}
}

func TestFindSourceArgs(t *testing.T) {
	unit := &kindex.Compilation{
		Proto: &apb.CompilationUnit{
			RequiredInput: []*apb.CompilationUnit_FileInput{{
				Info: &apb.FileInfo{Path: "a/b/c.go"},
			}, {
				Info: &apb.FileInfo{Path: "d/e/f.cc"},
			}, {
				Info: &apb.FileInfo{Path: "old"},
			}},
			SourceFile: []string{"old"},
			// Matches:        no      yes, keep   yes, skip   no
			Argument: []string{"blah", "a/b/c.go", "p/d/q.go", "quux"},
		},
	}
	// Results:      new         extant
	want := []string{"a/b/c.go", "old"}

	r := regexp.MustCompile(`\.go$`)
	FindSourceArgs(r)(unit)
	if got := unit.Proto.SourceFile; !reflect.DeepEqual(got, want) {
		t.Errorf("FindSourceArgs: got %+q, want %+q", got, want)
	}
}
