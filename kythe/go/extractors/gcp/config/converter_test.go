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

package config

import (
	"fmt"
	"io/ioutil"
	"path/filepath"
	"testing"

	"kythe.io/kythe/go/test/testutil"
)

const testDataDir = "testdata"

func TestYAMLConversion(t *testing.T) {
	testcases := []testcase{
		{"mvn"},
		{"gradle"},
	}

	for _, tcase := range testcases {
		t.Run(tcase.basename, func(t *testing.T) {
			actual, err := KytheToYAML(tcase.getJSONFile(t))
			if err != nil {
				t.Fatalf("reading config %s: %v", tcase.basename, err)
			}
			expected, err := tcase.getExpectedYAML(t)
			if err != nil {
				t.Fatalf("reading expected testdata %s: %v", tcase.basename, err)
			}

			if err := testutil.YAMLEqual(actual, expected); err != nil {
				t.Errorf("Expected config %s to be equal: %v", tcase.basename, err)
			}
		})
	}
}

type testcase struct {
	basename string
}

func (tc testcase) getJSONFile(t *testing.T) string {
	return getPath(t, fmt.Sprintf("%s.json", tc.basename))
}

func (tc testcase) getExpectedYAML(t *testing.T) ([]byte, error) {
	fp := getPath(t, fmt.Sprintf("%s.yaml", tc.basename))
	return ioutil.ReadFile(fp)
}

func getPath(t *testing.T, f string) string {
	return testutil.TestFilePath(t, filepath.Join(testDataDir, f))
}
