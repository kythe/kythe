/*
 * Copyright 2025 The Kythe Authors. All rights reserved.
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

package main

import (
	"context"
	"errors"
	"fmt"
	"io"
	"os"
	"path/filepath"
	"reflect"
	"slices"
	"sort"
	"strings"
	"testing"
	"time"

	"github.com/google/go-cmp/cmp"
	"github.com/google/go-cmp/cmp/cmpopts"
	"google.golang.org/protobuf/proto"
	"google.golang.org/protobuf/testing/protocmp"
	apb "kythe.io/kythe/proto/analysis_go_proto"
	spb "kythe.io/kythe/proto/storage_go_proto"
)

func TestGetTransitiveDependencies(t *testing.T) {
	// Helper to create a simple Crate for testing
	makeCrate := func(id crateId) crate {
		return crate{CrateId: id}
	}

	tests := []struct {
		name       string
		startCrate crate
		crateDeps  map[crateId][]crateId
		want       []crateId
	}{{
		name:       "No dependencies",
		startCrate: makeCrate(1),
		crateDeps: map[crateId][]crateId{
			1: {},
		},
		want: []crateId{},
	},
		{
			name:       "Simple direct dependencies",
			startCrate: makeCrate(1),
			crateDeps: map[crateId][]crateId{
				1: {2, 3},
				2: {},
				3: {},
			},
			want: []crateId{2, 3},
		},
		{
			name:       "Simple transitive dependencies (A -> B -> C)",
			startCrate: makeCrate(1),
			crateDeps: map[crateId][]crateId{
				1: {2},
				2: {3},
				3: {},
			},
			want: []crateId{2, 3},
		},
		{
			name:       "Multiple paths to same dependency (A -> B, A -> C, B -> C)",
			startCrate: makeCrate(1),
			crateDeps: map[crateId][]crateId{
				1: {2, 3},
				2: {3},
				3: {},
			},
			want: []crateId{2, 3},
		},
		{
			name:       "Diamond dependency (A -> B, A -> C, B -> D, C -> D)",
			startCrate: makeCrate(1),
			crateDeps: map[crateId][]crateId{
				1: {2, 3},
				2: {4},
				3: {4},
				4: {},
			},
			want: []crateId{2, 3, 4},
		},
		{
			name:       "Cyclic dependency (A -> B -> C -> A)",
			startCrate: makeCrate(1),
			crateDeps: map[crateId][]crateId{
				1: {2},
				2: {3},
				3: {1}, // Cycle back to A
			},
			want: []crateId{1, 2, 3},
		},
		{
			name:       "Deeper cycle (A -> B -> C -> D -> B)",
			startCrate: makeCrate(1),
			crateDeps: map[crateId][]crateId{
				1: {2},
				2: {3},
				3: {4},
				4: {2}, // Cycle back to B
			},
			want: []crateId{2, 3, 4},
		},
		{
			name:       "Disconnected graph (A -> B, C -> D)",
			startCrate: makeCrate(1),
			crateDeps: map[crateId][]crateId{
				1: {2},
				2: {},
				3: {4},
				4: {},
			},
			want: []crateId{2}, // Only B is reachable from A
		},
		{
			name:       "Empty dependency graph",
			startCrate: makeCrate(1),
			crateDeps:  map[crateId][]crateId{},
			want:       []crateId{},
		},
		{
			name:       "Start crate not in graph keys",
			startCrate: makeCrate(5), // Crate 5 is not a key in crateDeps
			crateDeps: map[crateId][]crateId{
				1: {2},
				2: {3},
			},
			want: []crateId{},
		},
		{
			name:       "Dependencies not defined elsewhere",
			startCrate: makeCrate(1),
			crateDeps: map[crateId][]crateId{
				1: {2, 3}, // Crate 2 and 3 don't have entries in the map
			},
			want: []crateId{2, 3},
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			got := transitiveDepsForCrate(tt.startCrate, tt.crateDeps)
			// Sort the expected slice just in case (though they should be defined sorted)
			sort.Slice(tt.want, func(i, j int) bool { return tt.want[i] < tt.want[j] })

			if diff := cmp.Diff(tt.want, got); diff != "" {
				t.Errorf("getTransitiveDependencies() diff: %v", diff)
			}
		})
	}
}

// Test specifically for sorting output
func TestGetTransitiveDependencies_Sorting(t *testing.T) {
	makeCrate := func(id crateId) crate {
		return crate{CrateId: id}
	}
	crateDeps := map[crateId][]crateId{
		1: {5, 2, 4, 3}, // Unsorted direct dependencies
		2: {6},
		3: {},
		4: {},
		5: {},
		6: {},
	} // Unsorted direct dependencies
	startCrate := makeCrate(1)
	want := []crateId{2, 3, 4, 5, 6} // Expected sorted output

	got := transitiveDepsForCrate(startCrate, crateDeps)

	if !reflect.DeepEqual(got, want) {
		t.Errorf("getTransitiveDependencies() sorting failed = %v, want %v", got, want)
	}
}

func TestRelativizeProjectJson(t *testing.T) {
	projectRoot := "/home/user/myproject"
	projectRootWithSlash := projectRoot + "/"

	join := func(elem ...string) string {
		// Filter out empty strings which Join usually ignores
		nonEmpty := []string{}
		for _, e := range elem {
			if e != "" {
				nonEmpty = append(nonEmpty, e)
			}
		}
		if len(nonEmpty) == 0 {
			return ""
		}
		// Use filepath.Clean to handle separators and relative elements correctly
		return filepath.Clean(strings.Join(nonEmpty, "/"))
	}

	tests := []struct {
		name        string
		projectRoot string
		input       rustProject
		want        rustProject
	}{
		{
			name:        "Empty project",
			projectRoot: projectRoot,
			input:       rustProject{Crates: []crate{}},
			want:        rustProject{Crates: []crate{}},
		},
		{
			name:        "Single crate with absolute paths inside root",
			projectRoot: projectRoot,
			input: rustProject{
				Crates: []crate{
					{
						CrateId:    0,
						RootModule: join(projectRoot, "src", "main.rs"),
						Source: source{
							IncludeDirs: []string{join(projectRoot, "src"), join(projectRoot, "vendor", "dep1")},
							ExcludeDirs: []string{join(projectRoot, "target"), join(projectRoot, "src", "tests")},
						},
					},
				},
			},
			want: rustProject{
				Crates: []crate{
					{
						CrateId:    0,
						RootModule: join("src", "main.rs"),
						Source: source{
							IncludeDirs: []string{join("src"), join("vendor", "dep1")},
							ExcludeDirs: []string{join("target"), join("src", "tests")},
						},
					},
				},
			},
		},
		{
			name:        "Single crate with trailing slash in root",
			projectRoot: projectRootWithSlash, // Use root with trailing slash
			input: rustProject{
				Crates: []crate{
					{
						CrateId:    0,
						RootModule: join(projectRoot, "src", "main.rs"),
						Source: source{
							IncludeDirs: []string{join(projectRoot, "src")},
							ExcludeDirs: []string{join(projectRoot, "target")},
						},
					},
				},
			},
			want: rustProject{
				Crates: []crate{
					{
						CrateId:    0,
						RootModule: join("src", "main.rs"),
						Source: source{
							IncludeDirs: []string{join("src")},
							ExcludeDirs: []string{join("target")},
						},
					},
				},
			},
		},
		{
			name:        "Multiple crates",
			projectRoot: projectRoot,
			input: rustProject{
				Crates: []crate{
					{
						CrateId:    0,
						RootModule: join(projectRoot, "crate0", "lib.rs"),
						Source: source{
							IncludeDirs: []string{join(projectRoot, "crate0")},
							ExcludeDirs: []string{},
						},
					},
					{
						CrateId:    1,
						RootModule: join(projectRoot, "crate1", "main.rs"),
						Source: source{
							IncludeDirs: []string{join(projectRoot, "crate1", "src")},
							ExcludeDirs: []string{join(projectRoot, "crate1", "target")},
						},
					},
				},
			},
			want: rustProject{
				Crates: []crate{
					{
						CrateId:    0,
						RootModule: join("crate0", "lib.rs"),
						Source: source{
							IncludeDirs: []string{join("crate0")},
							ExcludeDirs: []string{},
						},
					},
					{
						CrateId:    1,
						RootModule: join("crate1", "main.rs"),
						Source: source{
							IncludeDirs: []string{join("crate1", "src")},
							ExcludeDirs: []string{join("crate1", "target")},
						},
					},
				},
			},
		},
		{
			name:        "Paths already relative",
			projectRoot: projectRoot,
			input: rustProject{
				Crates: []crate{
					{
						CrateId:    0,
						RootModule: join("src", "main.rs"), // Already relative
						Source: source{
							IncludeDirs: []string{join("src")},
							ExcludeDirs: []string{join("target")},
						},
					},
				},
			},
			want: rustProject{ // Expect no change
				Crates: []crate{
					{
						CrateId:    0,
						RootModule: join("src", "main.rs"),
						Source: source{
							IncludeDirs: []string{join("src")},
							ExcludeDirs: []string{join("target")},
						},
					},
				},
			},
		},
		{
			name:        "Paths outside project root (should become relative with ..)",
			projectRoot: projectRoot,
			input: rustProject{
				Crates: []crate{
					{
						CrateId:    0,
						RootModule: join(filepath.Dir(projectRoot), "other_project", "main.rs"),
						Source: source{
							IncludeDirs: []string{join(filepath.Dir(projectRoot), "shared_lib")},
							ExcludeDirs: []string{join(projectRoot, "target")},
						},
					},
				},
			},
			want: rustProject{
				Crates: []crate{
					{
						CrateId:    0,
						RootModule: join("..", "other_project", "main.rs"),
						Source: source{
							IncludeDirs: []string{join("..", "shared_lib")},
							ExcludeDirs: []string{join("target")},
						},
					},
				},
			},
		},
		{
			name:        "Empty paths",
			projectRoot: projectRoot,
			input: rustProject{
				Crates: []crate{
					{
						CrateId:    0,
						RootModule: "", // Empty
						Source: source{
							IncludeDirs: []string{""}, // Empty
							ExcludeDirs: []string{""}, // Empty
						},
					},
				},
			},
			want: rustProject{
				Crates: []crate{
					{
						CrateId:    0,
						RootModule: "",
						Source: source{
							IncludeDirs: []string{""},
							ExcludeDirs: []string{""},
						},
					},
				},
			},
		},
		{
			name:        "Nil/Empty slices for dirs",
			projectRoot: projectRoot,
			input: rustProject{
				Crates: []crate{
					{
						CrateId:    0,
						RootModule: join(projectRoot, "main.rs"),
						Source: source{
							IncludeDirs: nil,        // Nil slice
							ExcludeDirs: []string{}, // Empty slice
						},
					},
				},
			},
			want: rustProject{
				Crates: []crate{
					{
						CrateId:    0,
						RootModule: "main.rs",
						Source: source{
							IncludeDirs: nil,        // Should remain nil
							ExcludeDirs: []string{}, // Should remain empty
						},
					},
				},
			},
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {

			input := tt.input
			// Execute the method
			input.relativizeProjectJson(tt.projectRoot)

			// Assert
			if diff := cmp.Diff(input, tt.want); diff != "" {
				t.Errorf("relativizeProjectJson() (-want +got):\n%s", diff)
			}
		})
	}
}

// MockKzipWriter satisfies the parts of kzip.Writer used by writeCrate
type MockKzipWriter struct {
	AddUnitFunc func(*apb.CompilationUnit, *apb.IndexedCompilation_Index) (string, error)
	// AddFile is needed by the mock getSourceFiles, not directly by writeCrate
	AddFileFunc func(r io.Reader) (string, error)
	AddedUnit   *apb.CompilationUnit // Stores the unit passed to AddUnit
}

func (m *MockKzipWriter) AddUnit(unit *apb.CompilationUnit, index *apb.IndexedCompilation_Index) (string, error) {
	m.AddedUnit = unit
	if m.AddUnitFunc != nil {
		return m.AddUnitFunc(unit, index)
	}
	return "mockDigest", nil
}

func (m *MockKzipWriter) AddFile(r io.Reader) (string, error) {
	if m.AddFileFunc != nil {
		return m.AddFileFunc(r)
	}
	return "mockFileDigest", nil
}

func (m *MockKzipWriter) Close() error {
	return nil
}

// MockCollectCrateSources provides a mock implementation for collectCrateSourcesFunc.
type MockCollectCrateSources struct {
	Results map[string]struct { // Keyed by a representative string (e.g., first include dir)
		Files  []string
		Inputs []*apb.CompilationUnit_FileInput
		Err    error
	}
	Calls []struct {
		SourceDirs  source
		ProjectRoot string
		Corpus      string
	}
}

// collectCrateSources is the method implementing the collectCrateSourcesFunc signature.
func (m *MockCollectCrateSources) collectCrateSources(ctx context.Context, e *extractor, sourceDirs source) ([]string, []*apb.CompilationUnit_FileInput, error) {
	m.Calls = append(m.Calls, struct {
		SourceDirs  source
		ProjectRoot string
		Corpus      string
	}{SourceDirs: sourceDirs, ProjectRoot: e.projectRoot, Corpus: e.corpus})

	// Use the first include dir as a simple key for the mock results.
	// This requires test setup to ensure this key is unique enough.
	key := ""
	if len(sourceDirs.IncludeDirs) > 0 {
		key = sourceDirs.IncludeDirs[0]
	}

	if res, ok := m.Results[key]; ok {
		// Simulate adding files to kzip if not erroring
		if res.Err == nil && e.kzipWriter != nil {
			for _, f := range res.Files {
				// We don't need the actual content for this mock
				_, _ = e.kzipWriter.AddFile(strings.NewReader("mock content for " + f))
			}
		}
		return res.Files, res.Inputs, res.Err
	}
	return nil, nil, fmt.Errorf("mock collectCrateSources not configured for key: %q", key)
}

func TestWriteCrate(t *testing.T) {
	ctx := context.Background()
	projectRoot := "/test/root"
	corpus := "testcorpus"

	// --- Test Data ---
	crate0 := crate{
		CrateId:    0,
		Label:      "crate0",
		RootModule: "crate0/src/lib.rs",
		Source: source{
			IncludeDirs: []string{"crate0/src"},
			ExcludeDirs: []string{},
		},
		Deps: []dep{{CrateId: 1, Name: "crate1"}},
	} // crate0 will be modified in the multi-dep test case
	crate1 := crate{
		Source: source{
			IncludeDirs: []string{"vendor/crate1/src", "out/gen"},
			ExcludeDirs: []string{},
		},
	}
	crate2 := crate{
		Source: source{
			IncludeDirs: []string{"vendor/crate2"},
			ExcludeDirs: []string{""},
		},
	}
	crate3 := crate{
		Source: source{
			IncludeDirs: []string{"vendor/crate3"},
			ExcludeDirs: []string{"out/gen"}, // the same directory INCLUDED by crate1
		},
	}

	sourceDirs := map[crateId]source{
		0: crate0.Source,
		1: crate1.Source,
		2: crate2.Source,
		3: crate3.Source,
	}
	baseTransitiveDeps := [][]crateId{
		0: {0, 1},
		1: {1},
		2: {2},
		3: {3},
	}

	// Expected FileInputs
	crate0FileInputs := []*apb.CompilationUnit_FileInput{
		{VName: &spb.VName{Corpus: corpus, Path: "crate0/src/lib.rs"}, Info: &apb.FileInfo{Path: "crate0/src/lib.rs", Digest: "digest-lib0"}},
		{VName: &spb.VName{Corpus: corpus, Path: "crate0/src/mod.rs"}, Info: &apb.FileInfo{Path: "crate0/src/mod.rs", Digest: "digest-mod0"}},
	}
	crate1FileInputs := []*apb.CompilationUnit_FileInput{
		{VName: &spb.VName{Corpus: corpus, Path: "vendor/crate1/src/lib.rs"}, Info: &apb.FileInfo{Path: "vendor/crate1/src/lib.rs", Digest: "digest-lib1"}},
		{VName: &spb.VName{Corpus: corpus, Path: "out/gen/lib.rs"}, Info: &apb.FileInfo{Path: "out/gen/lib.rs", Digest: "digest-lib1"}},
	}
	crate2FileInputs := []*apb.CompilationUnit_FileInput{
		{VName: &spb.VName{Corpus: corpus, Path: "vendor/crate2/lib.rs"}, Info: &apb.FileInfo{Path: "vendor/crate2/lib.rs", Digest: "digest-lib2"}},
	}
	crate3FileInputs := []*apb.CompilationUnit_FileInput{
		{VName: &spb.VName{Corpus: corpus, Path: "vendor/crate3/lib.rs"}, Info: &apb.FileInfo{Path: "vendor/crate3/lib.rs", Digest: "digest-lib3"}},
	}

	projectJsonFileInput := &apb.CompilationUnit_FileInput{
		VName: &spb.VName{Corpus: corpus, Path: "rust-project.json"},
		Info:  &apb.FileInfo{Path: "rust-project.json", Digest: "mockFileDigest"},
	}

	tests := []struct {
		name                string
		crateToTest         crate
		mockCollector       *MockCollectCrateSources
		mockKzipWriter      *MockKzipWriter
		expectedUnit        *apb.CompilationUnit
		expectWriteCrateErr bool
	}{
		{
			name:        "Basic crate with dependency",
			crateToTest: crate0,
			mockCollector: &MockCollectCrateSources{
				Results: map[string]struct {
					Files  []string
					Inputs []*apb.CompilationUnit_FileInput
					Err    error
				}{
					"crate0/src":        {Files: []string{"crate0/src/lib.rs", "crate0/src/mod.rs"}, Inputs: crate0FileInputs, Err: nil},
					"vendor/crate1/src": {Files: []string{"vendor/crate1/src/lib.rs"}, Inputs: crate1FileInputs, Err: nil},
				},
			},
			mockKzipWriter: &MockKzipWriter{},
			expectedUnit: &apb.CompilationUnit{
				VName:         &spb.VName{Corpus: corpus, Language: "rust"},
				RequiredInput: append(append(slices.Clone(crate1FileInputs), projectJsonFileInput), crate0FileInputs...), // Clone dep inputs + project.json
				SourceFile:    []string{"crate0/src/lib.rs", "crate0/src/mod.rs"},                                        // Only crate0 files

			},
			expectWriteCrateErr: false,
		},
		{
			name:        "Error collecting crate sources",
			crateToTest: crate0,
			mockCollector: &MockCollectCrateSources{
				Results: map[string]struct {
					Files  []string
					Inputs []*apb.CompilationUnit_FileInput
					Err    error
				}{
					"crate0/src": {Err: errors.New("failed to collect crate0")},
					// No need to configure crate1 as it won't be reached
				},
			},
			mockKzipWriter:      &MockKzipWriter{},
			expectWriteCrateErr: true, // Expect error from writeCrate itself
		},
		{
			name: "Crate with multiple dependencies",
			crateToTest: crate{ // Modify crate0 for this test
				CrateId:    0,
				Label:      "crate0_multi",
				RootModule: "crate0/src/lib.rs",
				Source: source{
					IncludeDirs: []string{"crate0/src"},
					ExcludeDirs: []string{},
				},
				Deps: []dep{{CrateId: 1, Name: "crate1"}, {CrateId: 2, Name: "crate2"}}, // Depends on 1 and 2
			},
			mockCollector: &MockCollectCrateSources{
				Results: map[string]struct {
					Files  []string
					Inputs []*apb.CompilationUnit_FileInput
					Err    error
				}{
					"crate0/src":        {Files: []string{"crate0/src/lib.rs", "crate0/src/mod.rs"}, Inputs: crate0FileInputs, Err: nil},
					"vendor/crate1/src": {Files: []string{"vendor/crate1/src/lib.rs"}, Inputs: crate1FileInputs, Err: nil},
					"vendor/crate2":     {Files: []string{"vendor/crate2/lib.rs"}, Inputs: crate2FileInputs, Err: nil}, // Add result for crate2
				},
			},
			mockKzipWriter: &MockKzipWriter{},
			expectedUnit: &apb.CompilationUnit{
				VName: &spb.VName{Corpus: corpus, Language: "rust"},
				// Required inputs should include both dependencies + project.json
				RequiredInput: append(append(append(slices.Clone(crate1FileInputs), crate2FileInputs...), crate0FileInputs...), projectJsonFileInput),
				SourceFile:    []string{"crate0/src/lib.rs", "crate0/src/mod.rs"}, // Only crate0 files
			},
			expectWriteCrateErr: false,
		},
		{
			name: "Conflicting include/exclude in dependencies",
			crateToTest: crate{ // crate0 depends on crate1 (includes shared) and crate2 (excludes shared)
				CrateId:    0,
				Label:      "crate0_conflict",
				RootModule: "crate0/src/lib.rs",
				Source: source{
					IncludeDirs: []string{"crate0/src"},
					ExcludeDirs: []string{},
				},
				Deps: []dep{{CrateId: 1, Name: "crate1"}, {CrateId: 3, Name: "crate3"}},
			},
			mockCollector: &MockCollectCrateSources{
				Results: map[string]struct {
					Files  []string
					Inputs []*apb.CompilationUnit_FileInput
					Err    error
				}{
					"crate0/src":        {Files: []string{"crate0/src/lib.rs", "crate0/src/mod.rs"}, Inputs: crate0FileInputs, Err: nil},
					"vendor/crate1/src": {Files: []string{"crate1/src/lib.rs", "out/gen/lib.rs"}, Inputs: crate1FileInputs, Err: nil}, // this includes a directory explicitly excluded by crate3.
					"vendor/crate3":     {Files: []string{"crate3/src/lib.rs"}, Inputs: crate3FileInputs, Err: nil},
				},
			},
			mockKzipWriter: &MockKzipWriter{},
			expectedUnit: &apb.CompilationUnit{
				VName:         &spb.VName{Corpus: corpus, Language: "rust"},
				RequiredInput: append(append(append(slices.Clone(crate1FileInputs), crate3FileInputs...), crate0FileInputs...), projectJsonFileInput),
				SourceFile:    []string{"crate0/src/lib.rs", "crate0/src/mod.rs"}, // Only crate0 files
			},
			expectWriteCrateErr: false,
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			cache := newRequiredInputsCache()

			mockWriter := tt.mockKzipWriter

			// Adjust transitiveDeps specifically for the multi-dep test case
			currentTransitiveDeps := baseTransitiveDeps
			switch tt.name {
			case "Crate with multiple dependencies":
				currentTransitiveDeps = [][]crateId{
					0: {0, 1, 2}, // Crate 0 depends on 1 and 2
					1: {1},
					2: {2},
					3: {3},
				}
			case "Conflicting include/exclude in dependencies":
				// Simulate: 0 -> 1, 0 -> 2. Crate 1 needs shared/src, Crate 2 does not.
				// For simplicity, let's assume crate1 depends on crate3 (shared), and crate2 does not.
				currentTransitiveDeps = [][]crateId{
					0: {0, 1, 3},
					1: {1},
					2: {2},
					3: {3},
				}
			}

			extractor := extractor{
				project: rustProject{
					Crates: []crate{crate0, crate1, crate2, crate3},
				},
				projectRoot:         projectRoot,
				corpus:              corpus,
				rules:               nil,
				requiredInputsCache: &cache,
				kzipWriter:          mockWriter,
			}

			// Pass the mock collector's method as the function argument
			err := extractor.writeCrate(ctx, tt.crateToTest, currentTransitiveDeps, sourceDirs, tt.mockCollector.collectCrateSources)

			if tt.expectWriteCrateErr {
				if err == nil {
					t.Errorf("writeCrate() expected an error, but got nil")
				}
				// Don't check unit if an error was expected during collection/writing
				return
			}
			if err != nil {
				t.Fatalf("writeCrate() unexpected error: %v", err)
			}

			// Verify the CompilationUnit passed to AddUnit
			if mockWriter.AddedUnit == nil {
				t.Fatalf("mockKzipWriter.AddUnit was not called")
			}

			fmt.Printf("added unit: %v\n", mockWriter.AddedUnit)

			// Sort slices for consistent comparison
			sort.Strings(mockWriter.AddedUnit.SourceFile)
			sort.Strings(tt.expectedUnit.SourceFile)
			sort.Slice(mockWriter.AddedUnit.RequiredInput, func(i, j int) bool {
				pathI := mockWriter.AddedUnit.RequiredInput[i].Info.Path
				pathJ := mockWriter.AddedUnit.RequiredInput[j].Info.Path
				return pathI < pathJ
			})
			sort.Slice(tt.expectedUnit.RequiredInput, func(i, j int) bool {
				pathI := tt.expectedUnit.RequiredInput[i].Info.Path
				pathJ := tt.expectedUnit.RequiredInput[j].Info.Path
				return pathI < pathJ
			})

			if diff := cmp.Diff(tt.expectedUnit.RequiredInput, mockWriter.AddedUnit.RequiredInput, cmp.Comparer(proto.Equal)); diff != "" {
				t.Errorf("writeCrate() produced unexpected RequiredInput diff (-want +got):\n%s", diff)
			}
			if diff := cmp.Diff(tt.expectedUnit.SourceFile, mockWriter.AddedUnit.SourceFile, cmp.Comparer(proto.Equal)); diff != "" {
				t.Errorf("writeCrate() produced unexpected RequiredInput diff (-want +got):\n%s", diff)
			}
		})
	}
}

// MockFileInfo implements os.FileInfo for testing.
type MockFileInfo struct {
	FName    string
	FIsDir   bool
	FMode    os.FileMode
	FSize    int64
	FModTime time.Time
	FSys     any
}

func (fi *MockFileInfo) Name() string       { return fi.FName }
func (fi *MockFileInfo) IsDir() bool        { return fi.FIsDir }
func (fi *MockFileInfo) Mode() os.FileMode  { return fi.FMode }
func (fi *MockFileInfo) Size() int64        { return fi.FSize }
func (fi *MockFileInfo) ModTime() time.Time { return fi.FModTime }
func (fi *MockFileInfo) Sys() any           { return fi.FSys }

func TestShouldIncludeFile(t *testing.T) {
	projectRoot := "/project"
	tests := []struct {
		name        string
		info        os.FileInfo
		path        string // Absolute path
		sourceDirs  source
		projectRoot string
		wantInclude bool
		wantErr     error
	}{
		{
			name:        "Include .rs file in include dir",
			info:        &MockFileInfo{FName: "main.rs", FIsDir: false},
			path:        filepath.Join(projectRoot, "src/main.rs"),
			sourceDirs:  source{IncludeDirs: []string{"src"}, ExcludeDirs: []string{}},
			projectRoot: projectRoot,
			wantInclude: true,
			wantErr:     nil,
		},
		{
			name:        "Exclude non-.rs file",
			info:        &MockFileInfo{FName: "README.md", FIsDir: false},
			path:        filepath.Join(projectRoot, "src/README.md"),
			sourceDirs:  source{IncludeDirs: []string{"src"}, ExcludeDirs: []string{}},
			projectRoot: projectRoot,
			wantInclude: false,
			wantErr:     nil,
		},
		{
			name:        "Exclude excluded dir",
			info:        &MockFileInfo{FName: "test.rs", FIsDir: false},
			path:        filepath.Join(projectRoot, "src/tests/"),
			sourceDirs:  source{IncludeDirs: []string{"src"}, ExcludeDirs: []string{"src/tests"}},
			projectRoot: projectRoot,
			wantInclude: false,
			wantErr:     nil,
		},
		{
			name:        "Handle directory not in exclude list",
			info:        &MockFileInfo{FName: "subdir", FIsDir: true},
			path:        filepath.Join(projectRoot, "src/subdir"),
			sourceDirs:  source{IncludeDirs: []string{"src"}, ExcludeDirs: []string{"target"}},
			projectRoot: projectRoot,
			wantInclude: false,
			wantErr:     nil,
		},
		{
			name:        "File without extension",
			info:        &MockFileInfo{FName: "config", FIsDir: false},
			path:        filepath.Join(projectRoot, "config"),
			sourceDirs:  source{IncludeDirs: []string{"."}, ExcludeDirs: []string{}},
			projectRoot: projectRoot,
			wantInclude: false,
			wantErr:     nil,
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			gotInclude, gotErr := shouldIncludeFile(tt.info, tt.path, tt.sourceDirs.ExcludeDirs, tt.projectRoot)
			if gotInclude != tt.wantInclude || !errors.Is(gotErr, tt.wantErr) {
				t.Errorf("shouldIncludeFile(%q) = (%v, %v), want (%v, %v)", tt.path, gotInclude, gotErr, tt.wantInclude, tt.wantErr)
			}
		})
	}
}

func makeTestCrate(id crateId, label string, deps []dep, isMember bool) crate {
	return crate{
		CrateId:           id,
		Label:             label,
		RootModule:        fmt.Sprintf("src/%s/lib.rs", label),
		Deps:              deps,
		IsWorkspaceMember: isMember,
	}
}

func TestSynthesizeProjectJson(t *testing.T) {
	crate0 := makeTestCrate(0, "crate0", []dep{{CrateId: 1, Name: "crate1"}, {CrateId: 2, Name: "crate2"}}, true)
	crate1 := makeTestCrate(1, "crate1", []dep{{CrateId: 3, Name: "crate3"}}, false) // Initially not a member
	crate2 := makeTestCrate(2, "crate2", []dep{{CrateId: 3, Name: "crate3"}}, true)
	crate3 := makeTestCrate(3, "crate3", []dep{}, false) // Initially not a member
	crate4 := makeTestCrate(4, "crate4", []dep{}, true)  // Standalone

	allCratesBase := []crate{crate0, crate1, crate2, crate3, crate4}

	tests := []struct {
		name           string
		targetId       crateId
		allCrates      []crate
		transitiveDeps [][]crateId
		wantProject    rustProject
	}{
		{
			name:      "No dependencies",
			targetId:  4,
			allCrates: allCratesBase,
			transitiveDeps: [][]crateId{
				0: {0, 1, 2, 3},
				1: {3, 1},
				2: {3, 2},
				3: {3},
				4: {4}, // Crate 4 has no deps
			},
			wantProject: rustProject{
				Crates: []crate{
					// Only crate 4, IsWorkspaceMember forced to true
					makeTestCrate(4, "crate4", []dep{}, true),
				},
			},
		},
		{
			name:      "Simple direct dependency",
			targetId:  1, // Target crate 1 depends on 3
			allCrates: allCratesBase,
			transitiveDeps: [][]crateId{
				0: {0, 1, 2, 3},
				1: {1, 3}, // Crate 1 depends on 3
				2: {2, 3},
				3: {3},
				4: {4},
			},
			wantProject: rustProject{
				Crates: []crate{
					// Crate 1 (index 0), depends on crate 3 (new index 1)
					makeTestCrate(1, "crate1", []dep{{CrateId: 1, Name: "crate3"}}, true), // Dep ID updated to 1
					// Crate 3 (index 1)
					makeTestCrate(3, "crate3", []dep{}, true),
				},
			},
		},
		{
			name:      "Multiple direct dependencies and transitive",
			targetId:  0, // Target crate 0 depends on 1, 2; 1->3, 2->3
			allCrates: allCratesBase,
			transitiveDeps: [][]crateId{
				0: {0, 1, 2, 3}, // Transitive deps of 0 are 1, 2, 3
				1: {1, 3},
				2: {2, 3},
				3: {3},
				4: {4},
			},
			wantProject: rustProject{
				// Crates sorted by original ID: 0, 1, 2, 3
				Crates: []crate{
					// Crate 0 (index 0), depends on 1 (new index 1), 2 (new index 2)
					makeTestCrate(0, "crate0", []dep{{CrateId: 1, Name: "crate1"}, {CrateId: 2, Name: "crate2"}}, true),
					// Crate 1 (index 1), depends on 3 (new index 3)
					makeTestCrate(1, "crate1", []dep{{CrateId: 3, Name: "crate3"}}, true),
					// Crate 2 (index 2), depends on 3 (new index 3)
					makeTestCrate(2, "crate2", []dep{{CrateId: 3, Name: "crate3"}}, true),
					// Crate 3 (index 3)
					makeTestCrate(3, "crate3", []dep{}, true),
				},
			},
		},
		{
			name:     "Dependency ID remapping with different order",
			targetId: 0,
			allCrates: []crate{
				makeTestCrate(0, "crate0", []dep{{CrateId: 2, Name: "crate2"}}, true),
				makeTestCrate(1, "crate1", []dep{}, false),
				makeTestCrate(2, "crate2", []dep{}, false),
			},
			transitiveDeps: [][]crateId{
				0: {0, 2}, // Depends on 1 and 2
				1: {1},
				2: {2},
			},
			wantProject: rustProject{
				Crates: []crate{
					// crate0 depends on crate2, which is now the 1st crate in the array (0-indexed)
					makeTestCrate(0, "crate0", []dep{{CrateId: 1, Name: "crate2"}}, true),
					// Crate 2
					makeTestCrate(1, "crate2", []dep{}, true),
				},
			},
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			// Need a deep copy of allCrates to avoid modification across tests
			cratesCopy := make([]crate, len(tt.allCrates))
			for i := range tt.allCrates {
				cratesCopy[i] = deepCopyCrate(tt.allCrates[i])
			}

			gotProject := synthesizeProjectJson(tt.targetId, cratesCopy, tt.transitiveDeps)

			opts := []cmp.Option{
				// ignore crateID because we're going to drop it when writing the json anyway.
				cmpopts.IgnoreFields(crate{}, "CrateId"),
				protocmp.Transform(),
			}

			if diff := cmp.Diff(tt.wantProject, gotProject, opts...); diff != "" {
				t.Errorf("synthesizeProjectJson(%d) mismatch (-want +got):\n%s", tt.targetId, diff)
			}
		})
	}
}
