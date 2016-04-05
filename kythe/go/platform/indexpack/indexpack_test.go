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

package indexpack

import (
	"archive/zip"
	"encoding/json"
	"flag"
	"fmt"
	"io"
	"io/ioutil"
	"os"
	"path/filepath"
	"reflect"
	"sort"
	"strings"
	"sync"
	"testing"

	cpb "kythe.io/kythe/proto/analysis_proto"
	spb "kythe.io/kythe/proto/storage_proto"

	"github.com/golang/protobuf/proto"
	"golang.org/x/net/context"
)

var (
	keepOutput = flag.Bool("keep_output", false, "Keep test output at exit?")

	tempDir     string   // Location for test index packs; set up in init
	testArchive *Archive // Archive created for testing

	// Fake input files.
	testFiles = map[string]string{
		"files/addresses/gettysburg.txt": "Four score and seven years ago...",
		"files/lists/audience.txt":       "1. Friends\n2. Romans\n3. Countrymen\n",
		"source/go/main.go":              "package main\nfunc main() {}\n",
		"source/go/lib.go":               "package lib\nfunc True() bool { return true }\n",
		"source/go/libfalse.go":          "package lib\nfunc False() bool { return false }\n",
		"source/java/Foo.java":           "package foo;\npublic class Foo {}\n",
	}

	// Test compilations, see below.
	testUnits = []*cpb.CompilationUnit{{
		VName: &spb.VName{
			Signature: "//root/source/java:foo",
			Language:  "java",
		},
	}, {
		VName: &spb.VName{
			Signature: "//root/source/go:main",
			Language:  "go",
		},
	}, {
		VName: &spb.VName{
			Signature: "//root/source/go:lib",
			Language:  "go",
		},
	}, {
		VName: &spb.VName{
			Signature: "//lonely/little/python",
			Language:  "python",
		},
	}}
)

// Some of the tests will be run with concurrent goroutines, to give tsan an
// opportunity to look for race conditions.
const concurrentWorkers = 4

func init() {
	dir, err := ioutil.TempDir("", "index_pack")
	if err != nil {
		panic(fmt.Sprintf("unable to create temp directory: %v", err))
	}
	tempDir = dir

	// Populate required inputs and source files for the test compilations.
	for _, unit := range testUnits {
		for path, data := range testFiles {
			unit.RequiredInput = append(unit.RequiredInput, &cpb.CompilationUnit_FileInput{
				Info: &cpb.FileInfo{
					Path:   path,
					Digest: hexDigest([]byte(data)),
				},
			})
			switch unit.VName.Language {
			case "go":
				if strings.HasSuffix(path, ".go") {
					unit.SourceFile = append(unit.SourceFile, path)
				}
			case "java":
				if strings.HasSuffix(path, ".java") {
					unit.SourceFile = append(unit.SourceFile, path)
				}
			}
		}
	}
}

// The order of the tests below is significant, specifically the first one
// creates an empty index pack that the subsequent tests use.

func TestCreateAndOpen(t *testing.T) {
	// Verify that we can successfully create a fresh index pack.
	path := filepath.Join(tempDir, "TestIndexPack")
	pack, err := Create(context.Background(), path, UnitType((*cpb.CompilationUnit)(nil)))
	if err != nil {
		t.Fatalf("Unable to create index pack %q: %v", path, err)
	}
	t.Logf("Created index pack: %#v", pack)
	testArchive = pack

	// Creating over an existing directory doesn't work.
	if alt, err := Create(context.Background(), path); err == nil {
		t.Errorf("Create should fail for existing path %q, but returned %v", path, alt)
	}

	// Opening a non-existent pack doesn't work.
	if alt, err := Open(context.Background(), path+"NoneSuch"); err == nil {
		t.Errorf("Open should fail for %q, but returned %v", path+"NoneSuch", alt)
	}

	// Opening the index pack we created for the test should work.
	if _, err := Open(context.Background(), path); err != nil {
		t.Errorf("Error opening existing path %q: %v", path, err)
	}
}

func TestOpenBogusArchive(t *testing.T) {
	path := filepath.Join(tempDir, "BogusIndexPack")
	if err := os.MkdirAll(path, 0700); err != nil {
		t.Fatalf("Unable to create bogus index pack (mkdir): %v", err)
	}
	f, err := os.Create(filepath.Join(path, "units"))
	if err != nil {
		t.Fatalf("Unable to create bogus index pack (touch): %v", err)
	}
	f.Close()
	if pack, err := Open(context.Background(), path); err == nil {
		t.Errorf("Open should have failed for bogus index pack, but returned %v", pack)
	}
}

func TestCompilationsRoundTrip(t *testing.T) {
	if testArchive == nil {
		t.Fatal("No test archive is present; test cannot proceed")
	}

	// Write all the compilations into the test archive.
	var wg sync.WaitGroup
	wg.Add(concurrentWorkers)
	for i := 0; i < concurrentWorkers; i++ {
		go func() {
			defer wg.Done()
			for _, unit := range testUnits {
				name, err := testArchive.WriteUnit(context.Background(), "kythe", unit)
				if err != nil {
					t.Errorf("Error writing compilation: %v\ninput was: %v", err, unit)
				} else {
					t.Logf("Wrote compilation unit to %q", name)
				}
			}
		}()
	}
	wg.Wait()

	// Verify that we can read the same data back out.
	sort.Sort(byTarget(testUnits))
	var gotUnits []*cpb.CompilationUnit
	err := testArchive.ReadUnits(context.Background(), "kythe", func(_ string, unit interface{}) error {
		gotUnits = append(gotUnits, unit.(*cpb.CompilationUnit))
		return nil
	})
	if err != nil {
		t.Errorf("Reading stored compilations failed: %v", err)
	}
	sort.Sort(byTarget(gotUnits))
	if !unitsEqual(gotUnits, testUnits) {
		t.Errorf("Compilation units did not round-trip:\ngot:  %+v\nwant: %+v", gotUnits, testUnits)
	}
}

func TestDefaultUnitType(t *testing.T) {
	if testArchive == nil {
		t.Fatal("No test archive is present; test cannot proceed")
	}

	// Test plan: Open the existing pack without a unit type set, and read out
	// the compilations.  Each should be delivered with the default type.
	ctx := context.Background()
	path := testArchive.Root()
	pack, err := Open(ctx, path) // no unit type specified
	if err != nil {
		t.Fatalf("Unable to create index pack %q: %v", path, err)
	}
	t.Logf("Opened index pack: %#v", pack)

	if err := pack.ReadUnits(ctx, "kythe", func(digest string, unit interface{}) error {
		if _, ok := unit.(*json.RawMessage); !ok {
			return fmt.Errorf("wrong value type for %q: got %T, want *json.RawMessage", digest, unit)
		}
		return nil
	}); err != nil {
		t.Errorf("Reading stored compilations failed: %v", err)
	}
}

func TestFilesRoundTrip(t *testing.T) {
	if testArchive == nil {
		t.Fatal("No test archive is present; test cannot proceed")
	}

	// Keep track of all the digests of the files we wrote successfully, and
	// map them back to the original file paths for verification.
	var μ sync.Mutex
	keys := make(map[string]string)

	// Write all the file contents into the index pack.
	var wg sync.WaitGroup
	wg.Add(concurrentWorkers)
	for i := 0; i < concurrentWorkers; i++ {
		go func() {
			defer wg.Done()
			for path, data := range testFiles {
				name, err := testArchive.WriteFile(context.Background(), []byte(data))
				if err != nil {
					t.Errorf("Error writing file contents for %q: %v", path, err)
				} else {
					μ.Lock()
					keys[strings.TrimRight(name, filepath.Ext(name))] = path
					μ.Unlock()
				}
			}
		}()
	}
	wg.Wait()

	// Verify that we can read the same data back out, and that we got all the
	// files we originally contracted for.

	found := make(map[string]bool) // So we can check for anything missing, below
	for digest, path := range keys {
		data, err := testArchive.ReadFile(context.Background(), digest)
		if err != nil {
			t.Errorf("Unable to read data for %q: %v", digest, err)
		} else if got, want := string(data), testFiles[path]; got != want {
			t.Errorf("Incorrect data for %q: got %q, want %q", digest, got, want)
		} else {
			t.Logf("Found correct data for %q: %q", path, got)
			found[path] = true
		}
	}
	for path := range testFiles {
		if !found[path] {
			t.Errorf("Missing data for file %q", path)
		}
	}
}

func TestFilter(t *testing.T) {
	ctx := context.Background()
	path := filepath.Join(tempDir, "FilterIndexPack")
	pack, err := Create(ctx, path, UnitType((*cpb.CompilationUnit)(nil)))
	if err != nil {
		t.Fatalf("Unable to create index pack %q: %v", path, err)
	}
	t.Logf("Created index pack: %#v", pack)

	tests := map[string]string{ // target -> key
		"A1": "alpha",
		"A2": "alpha",
		"B":  "bravo",
	}

	for target, key := range tests {
		if _, err := pack.WriteUnit(ctx, key, &cpb.CompilationUnit{
			VName: &spb.VName{Signature: target},
		}); err != nil {
			t.Fatalf("WriteUnit key=%q target=%q failed: %v", key, target, err)
		}
	}

	// Read all the units with key "alpha".
	got := make(map[string]string)
	if err := pack.ReadUnits(ctx, "alpha", func(_ string, unit interface{}) error {
		got[unit.(*cpb.CompilationUnit).GetVName().Signature] = "alpha"
		return nil
	}); err != nil {
		t.Errorf("ReadUnits key=alpha failed: %v", err)
	}

	delete(tests, "B")
	if !reflect.DeepEqual(got, tests) {
		t.Errorf("ReadUnits key=alpha: got %v, want %v", got, tests)
	}
}

func TestCreateOrOpen(t *testing.T) {
	ctx := context.Background()

	// The "BogusIndexPack" and "TestIndexPack" directories were created by
	// previous tests.  The "EmptyIndexPack" is created afresh here.

	// Opening an existing, invalid pack.
	if a, err := CreateOrOpen(ctx, filepath.Join(tempDir, "BogusIndexPack")); err == nil {
		t.Errorf("CreateOrOpen BogusIndexPack: got %v, wanted error", a)
	}

	// Opening an existing, valid pack.
	if _, err := CreateOrOpen(ctx, filepath.Join(tempDir, "TestIndexPack")); err != nil {
		t.Errorf("CreateOrOpen TestIndexPack: unexpected error: %v", err)
	}

	// Creating a new empty pack.
	if _, err := CreateOrOpen(ctx, filepath.Join(tempDir, "EmptyIndexPack")); err != nil {
		t.Errorf("CreateOrOpen EmptyIndexPack: unexpected error: %v", err)
	}
}

func TestZipReader(t *testing.T) {
	// Create a zip file from the "TestIndexPack" directory.
	root := testArchive.Root()
	zipPath := filepath.Join(tempDir, "testpack.zip")
	if err := zipDir(root, tempDir, zipPath); err != nil {
		t.Fatalf("Unable to create pack zip: %v", err)
	}

	ctx := context.Background()
	f, err := os.Open(zipPath)
	if err != nil {
		t.Fatalf("Error opening zip file: %v", err)
	}
	defer f.Close()
	fi, err := f.Stat()
	if err != nil {
		t.Fatalf("Error getting zip file size: %v", err)
	}

	pack, err := OpenZip(ctx, f, fi.Size(), UnitType((*cpb.CompilationUnit)(nil)))
	if err != nil {
		t.Fatalf("Error opening pack %q: %v", root, err)
	}

	var numUnits int
	digests := make(map[string]bool)
	if err := pack.ReadUnits(ctx, "kythe", func(_ string, v interface{}) error {
		numUnits++
		for _, ri := range v.(*cpb.CompilationUnit).RequiredInput {
			digests[ri.GetInfo().Digest] = true
		}
		return nil
	}); err != nil {
		t.Errorf("Error reading pack: %v", err)
	}
	if numUnits != len(testUnits) {
		t.Errorf("Read compilations: got %d compilations, want %d", numUnits, len(testUnits))
	}
	if len(digests) != len(testFiles) {
		t.Errorf("Read files: got %d, want %d", len(digests), len(testFiles))
	}
	t.Logf("Found %d compilations and %d unique file digests", numUnits, len(digests))
	for digest := range digests {
		data, err := pack.ReadFile(ctx, digest)
		if err != nil {
			t.Errorf("Reading digest %q failed: %v", digest, err)
		} else {
			t.Logf("Read %d bytes for digest %q", len(data), digest)
		}
	}
}

func TestZipErrors(t *testing.T) {
	ctx := context.Background()

	// Opening an empty archive should report an error.
	empty := strings.NewReader("")
	if pack, err := OpenZip(ctx, empty, 0); err == nil {
		t.Errorf("Opening empty zip: got %+v, wanted error", pack)
	}

	root := testArchive.Root()
	zipPath := filepath.Join(tempDir, "testpack.zip")
	f, err := os.Open(zipPath)
	if err != nil {
		t.Fatalf("Error opening zip file: %v", err)
	}
	defer f.Close()
	fi, err := f.Stat()
	if err != nil {
		t.Fatalf("Error getting zip file size: %v", err)
	}

	pack, err := OpenZip(ctx, f, fi.Size())
	if err != nil {
		t.Fatalf("Error opening pack %q: %v", root, err)
	}

	// Writes to a zip archive are expected to fail gracefully.  This reuses
	// the zip from the previous test.
	if id, err := pack.WriteFile(ctx, []byte("zut alors!")); err == nil {
		t.Errorf("WriteFile: got id %q, wanted error", id)
	}
	if id, err := pack.WriteUnit(ctx, "foo", []string{"x"}); err == nil {
		t.Errorf("WriteUnit: got id %q, wanted error", id)
	}
}

// This test does not actually test anything, it's just here to clean up after
// the other test cases at the end.  This should remain last in the file.
func TestCleanup(t *testing.T) {
	if *keepOutput {
		t.Logf("Keeping test output in %q", tempDir)
		return
	}
	if err := os.RemoveAll(tempDir); err != nil {
		t.Logf("Unable to clean up %q: %v", tempDir, err)
	}
}

// Order compilation units by their analysis target.
type byTarget []*cpb.CompilationUnit

func (b byTarget) Len() int      { return len(b) }
func (b byTarget) Swap(i, j int) { b[i], b[j] = b[j], b[i] }
func (b byTarget) Less(i, j int) bool {
	return b[i].GetVName().Signature < b[j].GetVName().Signature
}

// Reports whether a and b are equal, like reflect.DeepEqual but with correct
// comparison of proto messages.
func unitsEqual(a, b []*cpb.CompilationUnit) bool {
	if len(a) != len(b) {
		return false
	}
	for i, lhs := range a {
		if !proto.Equal(lhs, b[i]) {
			return false
		}
	}
	return true
}

// zipDir creates a zip file at zipPath from the recursive contents of root.
// The names in the archive have prefix trimmed from them.
func zipDir(root, prefix, zipPath string) error {
	f, err := os.Create(zipPath)
	if err != nil {
		return fmt.Errorf("creating zip file: %v", err)
	}

	w := zip.NewWriter(f)
	if err := filepath.Walk(root, func(path string, fi os.FileInfo, err error) error {
		if err != nil {
			return err
		}
		trimmed := path[len(prefix)+1:] // remove prefix/ prefix
		fh, err := zip.FileInfoHeader(fi)
		if err != nil {
			return err
		}
		fh.Name = trimmed
		if fi.IsDir() {
			// The "zip" command-line tool appends a separator to directory names.
			fh.Name += string(filepath.Separator)
		}
		f, err := w.CreateHeader(fh)
		if err != nil {
			return err
		}
		if !fi.IsDir() { // for non-directories, copy the file contents
			r, err := os.Open(path)
			if err != nil {
				return err
			}
			_, err = io.Copy(f, r)
			r.Close()
			if err != nil {
				return err
			}
		}
		return nil
	}); err != nil {
		f.Close()
		return fmt.Errorf("writing zip file: %v", err)
	}
	if err := w.Close(); err != nil {
		return fmt.Errorf("closing zip writer: %v", err)
	}
	return f.Close()
}
