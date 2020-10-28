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

package modifier

import (
	"io"
	"io/ioutil"
	"os"
	"path/filepath"
	"strings"
	"testing"

	"kythe.io/kythe/go/test/testutil"
)

const testDataDir = "testdata"

func TestBuildGradleHasKythe(t *testing.T) {
	testcases := []struct {
		fileName     string
		javacWrapper string
		// Whether kythe javac wrapper should be present.
		hasWrapper bool
		// Whether we expect an error.
		wantError bool
	}{
		{fileName: "modified-build.gradle", javacWrapper: "/tmp/javac-wrapper.sh", hasWrapper: true},
		{fileName: "plain-build.gradle", javacWrapper: "/tmp/javac-wrapper.sh"},
		{fileName: "other-build.gradle", javacWrapper: "/tmp/javac-wrapper.sh", wantError: true},
		// Look at other-build.gradle but use the correct javac wrapper.
		{fileName: "other-build.gradle", javacWrapper: "/different/javac-wrapper.sh", hasWrapper: true},
	}

	for _, tcase := range testcases {
		// This should just ignore the file and do nothing.
		hasWrapper, err := hasKytheWrapper(getPath(t, tcase.fileName), tcase.javacWrapper)
		if err != nil && !tcase.wantError {
			t.Fatalf("Failed to open test gradle file: %v", err)
		} else if err == nil && tcase.wantError {
			t.Errorf("Expected a failure for file %s, but didn't get one.", tcase.fileName)
		} else if tcase.hasWrapper != hasWrapper {
			var errstr string
			if tcase.hasWrapper {
				errstr = "should"
			} else {
				errstr = "should not"
			}
			t.Errorf("file %s %s already have kythe javac-wrapper", tcase.fileName, errstr)
		}
	}
}

func TestBuildGradle(t *testing.T) {
	testcases := []struct {
		inputFile          string
		javacWrapper       string
		expectedOutputFile string
	}{
		{"modified-build.gradle", "/tmp/javac-wrapper.sh", "modified-build.gradle"},
		{"plain-build.gradle", "/tmp/javac-wrapper.sh", "modified-build.gradle"},
		{"plain-build.gradle", "/different/javac-wrapper.sh", "other-build.gradle"},
	}

	for _, tcase := range testcases {
		t.Run(tcase.inputFile, func(t *testing.T) {
			// Copy the file into a temp file.
			tf, err := ioutil.TempFile("", tcase.inputFile)
			if err != nil {
				t.Fatalf("creating temp file: %v", err)
			}
			tfName := tf.Name()
			defer os.Remove(tfName)
			infile, err := os.Open(getPath(t, tcase.inputFile))
			if err != nil {
				t.Fatalf("opening file %s: %v", tcase.inputFile, err)
			}
			_, err = io.Copy(tf, infile)
			if err != nil {
				t.Fatalf("copying %s: %v", tcase.inputFile, err)
			}
			if err := infile.Close(); err != nil {
				t.Fatalf("closing %s: %v", tcase.inputFile, err)
			}
			if err := tf.Close(); err != nil {
				t.Fatalf("closing temp file: %v", err)
			}

			// Do the copy if necessary.
			if err := PreProcessBuildGradle(tfName, tcase.javacWrapper); err != nil {
				t.Fatalf("modifying gradle file %s: %v", tcase.inputFile, err)
			}

			// Compare results.
			eq, diff := testutil.TrimmedEqual(mustReadBytes(t, tfName), mustReadBytes(t, getPath(t, tcase.expectedOutputFile)))
			if !eq {
				t.Errorf("Expected input file %s to be %s, but got diff %s", tcase.inputFile, tcase.expectedOutputFile, diff)
			}
		})
	}
}

func TestPomXML(t *testing.T) {
	testcases := []struct {
		inputFile          string
		expectedOutputFile string

		// Specify this if there is an error expected, and include some snippet
		// of text from the expected error to make sure it's the right one.
		// If there's no expected error, just leave it as empty string.
		expectedErrorSnippet string
	}{
		{"modified-pom.xml", "modified-pom.xml", ""},
		{"child-pom.xml", "child-pom.xml", ""},
		{"plain-pom.xml", "modified-pom.xml", ""},
		{"other-pom.xml", "modified-pom.xml", ""},
		{"plugin-management-pom.xml", "plugin-management-pom.xml", ""},
		{"plugin-management-missing-version-pom.xml", "", "missing maven-compiler-plugin version"},
		{"plugin-management-wrong-plugin-version-pom.xml", "", "unsupported maven-compiler-plugin version: 2.9.0"},
		{"modified-wrong-source-version-pom.xml", "", "unsupported source version: 1.6"},
		{"modified-wrong-target-version-pom.xml", "", "unsupported target version: 1.6"},
		{"modified-invalid-version-pom.xml", "", "unsupported target version: elephant"},
	}

	for _, tcase := range testcases {
		t.Run(tcase.inputFile, func(t *testing.T) {
			// Copy the file into a temp file.
			tf, err := ioutil.TempFile("", tcase.inputFile)
			if err != nil {
				t.Fatalf("creating temp file: %v", err)
			}
			tfName := tf.Name()
			defer os.Remove(tfName)
			infile, err := os.Open(getPath(t, tcase.inputFile))
			if err != nil {
				t.Fatalf("opening file %s: %v", tcase.inputFile, err)
			}
			_, err = io.Copy(tf, infile)
			if err != nil {
				t.Fatalf("copying %s: %v", tcase.inputFile, err)
			}
			if err := infile.Close(); err != nil {
				t.Fatalf("closing %s: %v", tcase.inputFile, err)
			}
			if err := tf.Close(); err != nil {
				t.Fatalf("closing temp file: %v", err)
			}

			// Do the copy if necessary.
			procErr := PreProcessPomXML(tfName)
			if procErr != nil && tcase.expectedErrorSnippet == "" {
				t.Fatalf("modifying pom.xml %s: %v", tcase.inputFile, procErr)
			} else if tcase.expectedErrorSnippet != "" {
				if procErr == nil {
					t.Errorf("expected error with text '%s', but got no error", tcase.expectedErrorSnippet)
				} else if !strings.Contains(procErr.Error(), tcase.expectedErrorSnippet) {
					t.Errorf("expected error with text '%s', but got %q", tcase.expectedErrorSnippet, procErr)
				}
				// Expected error with correct text, passes.
			} else {
				// No expected error, compare results.
				eq, diff := testutil.TrimmedEqual(mustReadBytes(t, tfName), mustReadBytes(t, getPath(t, tcase.expectedOutputFile)))
				if !eq {
					t.Errorf("Expected input file %s to be %s, but got diff %s", tcase.inputFile, tcase.expectedOutputFile, diff)
					t.Errorf("input: %s", mustReadBytes(t, tfName))
				}
			}
		})
	}
}

// getPath returns a resolved filepath name for a file living in testdata/ directory.
func getPath(t *testing.T, f string) string {
	return testutil.TestFilePath(t, filepath.Join(testDataDir, f))
}

func mustReadBytes(t *testing.T, f string) []byte {
	t.Helper()
	b, err := ioutil.ReadFile(f)
	if err != nil {
		t.Fatalf("Failed to read %s: %v", f, err)
	}
	return b
}
