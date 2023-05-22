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

package bazel

import (
	"bytes"
	"context"
	"errors"
	"io"
	"io/ioutil"
	"os"
	"path/filepath"
	"reflect"
	"regexp"
	"strings"
	"testing"

	"kythe.io/kythe/go/platform/kzip"
	"kythe.io/kythe/go/util/log"
	"kythe.io/kythe/go/util/ptypes"

	"github.com/golang/protobuf/proto"

	apb "kythe.io/kythe/proto/analysis_go_proto"
	bipb "kythe.io/kythe/proto/buildinfo_go_proto"
	spb "kythe.io/kythe/proto/storage_go_proto"
	xapb "kythe.io/third_party/bazel/extra_actions_base_go_proto"
)

const (
	// The digest of an empty input, cf. openssl sha256 /dev/null
	emptyDigest = "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855"

	testCorpus = "test/corpus"
	testTarget = "//target"
	testLang   = "foo"
	testOutput = "outfile"
	wantDigest = emptyDigest
)

func prependFolder(folder string, paths ...string) []string {
	var result []string
	for _, path := range paths {
		result = append(result, folder+"/"+path)
	}
	return result
}

func createEmptyFiles(t *testing.T, paths []string) {
	for _, path := range paths {
		if err := ioutil.WriteFile(path, nil, 0755); err != nil {
			t.Fatalf("Creating test file: %v", err)
		}
	}
}

func createActionInfo(folder string) (*ActionInfo, *xapb.SpawnInfo) {
	// Gin up an exra action record with some known fields, and make sure the
	// extractor handles them correctly.
	xa := &xapb.ExtraActionInfo{
		Owner:    proto.String(testTarget),
		Mnemonic: proto.String("SomeAction"),
	}
	si := &xapb.SpawnInfo{
		Argument:   []string{"cc", "-o", testOutput, "-c", "2.src", "4.src"},
		InputFile:  prependFolder(folder, "1.dep", "2.src", "3.dep", "1.dep", "4.src", "treeArtifact"),
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
		log.Fatalf("Error setting extension on XA: %v", err)
	}
	act, err := SpawnAction(xa)
	if err != nil {
		log.Fatalf("Error generating Spawn action: %v", err)
		return nil, si
	}
	return act, si
}

type results struct {
	// These values are set by the test hooks to verify the plumbing does the
	// expected things.
	gotInfo       *ActionInfo
	checkedInputs []string
	checkedEnv    []string
	gotUnit       *apb.CompilationUnit
}

func (r *results) newConfig() *Config {
	fixed := false // see FixUnit, below
	return &Config{
		Corpus:   testCorpus,
		Language: testLang,

		CheckAction: func(_ context.Context, info *ActionInfo) error {
			r.gotInfo = info
			return nil
		},

		CheckInput: func(path string) (string, bool) {
			r.checkedInputs = append(r.checkedInputs, path)
			return path, true
		},

		CheckEnv: func(name, value string) bool {
			r.checkedEnv = append(r.checkedEnv, name)
			return name != "BOGUS"
		},

		IsSource: func(path string) bool { return filepath.Ext(path) == ".src" },
		FixUnit: func(unit *apb.CompilationUnit) error {
			if fixed {
				return errors.New("redundant call to FixUnit")
			}
			fixed = true
			r.gotUnit = unit
			return nil
		},

		// All the files are empty, and all the children are above average.
		OpenRead: func(_ context.Context, path string) (io.ReadCloser, error) {
			return ioutil.NopCloser(strings.NewReader("")), nil
		},
	}
}

func (r *results) checkValues(t *testing.T, cu *apb.CompilationUnit, si *xapb.SpawnInfo, folder string) {
	t.Helper()
	// Verify that the info check callback was invoked.
	wantInfo := &ActionInfo{ // N.B.: Values prior to filtering!
		Target:    testTarget,
		Arguments: []string{"cc", "-o", testOutput, "-c", "2.src", "4.src"},
		Inputs:    prependFolder(folder, "1.dep", "1.dep", "2.src", "3.dep", "4.src", "treeArtifact"),
		Outputs:   []string{testOutput, "garbage"},
		Environment: map[string]string{
			"PATH":  "p1:p2",
			"BOGUS": "should not be seen",
		},
	}
	if r.gotInfo == nil {
		t.Error("SpawnInfo not reported")
	} else if !reflect.DeepEqual(r.gotInfo, wantInfo) {
		t.Errorf("Wrong SpawnInfo reported:\n got %+v\nwant %+v", r.gotInfo, wantInfo)
	}

	// Verify that the inputs were all passed to the callback.
	if want := prependFolder(folder, "1.dep", "1.dep", "2.src", "3.dep", "4.src", "treeArtifact/5.src", "treeArtifact/6.dep"); !reflect.DeepEqual(r.checkedInputs, want) {
		t.Errorf("Wrong input files checked:\n got %+q\nwant %+q", r.checkedInputs, want)
	}

	// Verify that the environment got filtered correctly.
	if got, want := len(r.checkedEnv), len(wantInfo.Environment); got != want {
		t.Errorf("Wrong number of environment values: got %d, want %d", got, want)
	}
	for _, env := range r.checkedEnv {
		if _, ok := wantInfo.Environment[env]; !ok {
			t.Errorf("Missing environment variable %q", env)
		}
	}

	// Verify that the fixup callback got passed the unit that was returned.
	if !proto.Equal(cu, r.gotUnit) {
		t.Errorf("Wrong unit passed to fixup: got %+v, want %+v", r.gotUnit, cu)
	}

	// Verify that the identified sources were correctly propagated.
	if want := prependFolder(folder, "2.src", "4.src", "treeArtifact/5.src"); !reflect.DeepEqual(cu.SourceFile, want) {
		t.Errorf("Wrong source files:\n got %+q\nwant %+q", cu.SourceFile, want)
	}

	// Verify that the argument list was correctly propagated.
	if !reflect.DeepEqual(cu.Argument, si.Argument) {
		t.Errorf("Wrong argument list:\n got %+q\nwant %+q", cu.Argument, si.Argument)
	}

	// Check that the required inputs and file data have the expected metadata.
	for i, ri := range cu.RequiredInput {
		vname := &spb.VName{Corpus: testCorpus, Path: ri.Info.Path}
		if !proto.Equal(ri.VName, vname) {
			t.Errorf("Required input %d: wrong vname: got %+v, want %+v", i+1, ri.VName, vname)
		}
		if got := ri.Info.Digest; got != wantDigest {
			t.Errorf("Required input %d: wrong digest: got %q, want %q", i+1, got, wantDigest)
		}
	}

	// Check that the build details got plumbed.
	if dets := cu.Details; len(dets) == 0 {
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

func TestExtractToFile(t *testing.T) {
	// Prepare test directory and create src/dep files there.
	// We want to test subdirectories as well so creating treeArtifact subdir.
	tmp, err := ioutil.TempDir("", "TestExtractToFile")
	if err != nil {
		t.Fatalf("Error creating temp directory: %v", err)
	}
	err = os.Mkdir(tmp+"/treeArtifact", 0755)
	if err != nil {
		t.Fatalf("Error creating treeArfifact directory: %v", err)
	}
	createEmptyFiles(t,
		prependFolder(tmp, "1.dep", "2.src", "3.dep", "4.src", "treeArtifact/5.src", "treeArtifact/6.dep"))
	defer os.RemoveAll(tmp) // best effort

	res := new(results)
	config := res.newConfig()
	ai, si := createActionInfo(tmp)

	buf := bytes.NewBuffer(nil)
	w, err := kzip.NewWriter(buf)
	if err != nil {
		t.Fatalf("Creating kzip writer: %v", err)
	}
	digest, err := config.ExtractToFile(context.Background(), ai, w)
	if err != nil {
		t.Fatalf("ExtractToFile: unexpected failure: %v", err)
	}
	t.Logf("Got unit digest %q from writing", digest)
	if err := w.Close(); err != nil {
		t.Fatalf("Closing kxip writer: %v", err)
	}

	var numUnits int
	if err := kzip.Scan(bytes.NewReader(buf.Bytes()), func(_ *kzip.Reader, unit *kzip.Unit) error {
		res.checkValues(t, unit.Proto, si, tmp)
		numUnits++
		return nil
	}); err != nil {
		t.Errorf("Scanning kzip failed: %v", err)
	}
	if numUnits != 1 {
		t.Errorf("Scan reported %d units, want 1", numUnits)
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
	var fds []*apb.FileData
	paths := []string{
		goodFile,
		filepath.Join(tmp, "bad.h"), // does not exist
	}
	err = cfg.fetchInputs(context.Background(), paths, func(i int, r io.Reader) error {
		fd, err := kzip.FileData(paths[i], r)
		if err == nil {
			fds = append(fds, fd)
		}
		return err
	})

	// There should have been an error, because one of the requested files does
	// not exist. However, despite that error we should have gotten data for
	// the other file.
	if err == nil {
		t.Error("fetchInputs was expected to report an error, but did not")
	} else if len(fds) > 1 {
		t.Fatalf("fetchInputs: got %d files, wanted ≤%d", len(fds), 1)
	}

	if len(fds) == 1 {
		// There may be 1 FileData in the result if its read was attempted before
		// the non-existent file.
		want := &apb.FileData{
			Content: nil,
			Info: &apb.FileInfo{
				Path:   goodFile,
				Digest: emptyDigest,
			},
		}
		if !proto.Equal(fds[0], want) {
			t.Errorf("FileData[0]: got %+v, want %+v", fds[0], want)
		}
	}
}

func TestFindSourceArgs(t *testing.T) {
	unit := &apb.CompilationUnit{
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
	}
	// Results:      new         extant
	want := []string{"a/b/c.go", "old"}

	r := regexp.MustCompile(`\.go$`)
	FindSourceArgs(r)(unit)
	if got := unit.SourceFile; !reflect.DeepEqual(got, want) {
		t.Errorf("FindSourceArgs: got %+q, want %+q", got, want)
	}
}
