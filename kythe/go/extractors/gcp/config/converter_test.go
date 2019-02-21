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
	"io/ioutil"
	"path/filepath"
	"testing"

	"kythe.io/kythe/go/test/testutil"
)

const testDataDir = "testdata"

// Also test some yaml configs checked in elsewhere.
const examplesDir = "../../../../extractors/gcp/examples"

func TestYAMLConversion(t *testing.T) {
	testcases := []testcase{
		{
			name:     "maven-simple",
			jsonFile: "mvn.json",
			yamlDir:  testDataDir,
			yamlFile: "mvn.yaml",
		},
		{
			name:     "maven-subdir",
			jsonFile: "mvn-subdir.json",
			yamlDir:  testDataDir,
			yamlFile: "mvn-subdir.yaml",
		},
		{
			name:     "maven-from-examples",
			jsonFile: "mvn.json",
			yamlDir:  examplesDir,
			yamlFile: "mvn.yaml",
		},
		{
			name:     "guava",
			jsonFile: "guava-mvn.json",
			yamlDir:  examplesDir,
			yamlFile: "guava-mvn.yaml",
		},
		{
			name:     "guava-android",
			jsonFile: "guava-android-mvn.json",
			yamlDir:  examplesDir,
			yamlFile: "guava-android-mvn.yaml",
		},
		{
			name:     "gradle-simple",
			jsonFile: "gradle.json",
			yamlDir:  testDataDir,
			yamlFile: "gradle.yaml",
		},
		{
			name:     "gradle-subdir",
			jsonFile: "gradle-subdir.json",
			yamlDir:  testDataDir,
			yamlFile: "gradle-subdir.yaml",
		},
		{
			name:     "gradle-from-examples",
			jsonFile: "gradle.json",
			yamlDir:  examplesDir,
			yamlFile: "gradle.yaml",
		},
	}

	for _, tcase := range testcases {
		t.Run(tcase.name, func(t *testing.T) {
			actual, err := KytheToYAML(tcase.getJSONFile(t))
			if err != nil {
				t.Fatalf("reading config %s: %v", tcase.jsonFile, err)
			}
			expected, err := tcase.getExpectedYAML(t)
			if err != nil {
				t.Fatalf("reading expected testdata %s/%s: %v", tcase.yamlDir, tcase.yamlFile, err)
			}

			if err := testutil.YAMLEqual(expected, actual); err != nil {
				t.Errorf("Expected config %s to be equal: %v", tcase.jsonFile, err)
			}
		})
	}
}

type testcase struct {
	name     string
	jsonFile string

	// YAML files not guaranteed to be in testdata, could be elsewhere.
	yamlDir  string
	yamlFile string

	expectedError error
}

func (tc testcase) getJSONFile(t *testing.T) string {
	return testutil.TestFilePath(t, filepath.Join(testDataDir, tc.jsonFile))
}

func (tc testcase) getExpectedYAML(t *testing.T) ([]byte, error) {
	fp := testutil.TestFilePath(t, filepath.Join(tc.yamlDir, tc.yamlFile))
	return ioutil.ReadFile(fp)
}
