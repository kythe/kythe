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
	"testing"

	"kythe.io/kythe/go/test/testutil"
)

const testDataDir = "testdata"

func TestPreProcess(t *testing.T) {
	testcases := []struct {
		inputFile          string
		expectedOutputFile string
	}{
		{"modified-pom.xml", "modified-pom.xml"},
		{"plain-pom.xml", "modified-pom.xml"},
		{"other-pom.xml", "modified-pom.xml"},
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
			if err := PreProcessPomXML(tfName); err != nil {
				t.Fatalf("modifying pom.xml %s: %v", tcase.inputFile, err)
			}

			// Compare results.
			eq, diff := testutil.TrimmedEqual(mustReadBytes(t, tfName), mustReadBytes(t, getPath(t, tcase.expectedOutputFile)))
			if !eq {
				t.Errorf("Expected input file %s to be %s, but got diff %s", tcase.inputFile, tcase.expectedOutputFile, diff)
				t.Errorf("input: %s", mustReadBytes(t, tfName))
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
