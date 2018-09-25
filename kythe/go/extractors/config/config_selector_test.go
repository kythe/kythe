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
	"os"
	"path/filepath"
	"testing"

	"kythe.io/kythe/go/extractors/config/default/gradle"
	"kythe.io/kythe/go/extractors/config/default/mvn"
	"kythe.io/kythe/go/test/testutil"
)

type testBuildType struct {
	buildFile, expectedConfig string
	builder                   builderType
}

func TestSelectBuilder(t *testing.T) {
	tCases := []testBuildType{
		testBuildType{
			buildFile:      "pom.xml",
			expectedConfig: "testdata/mvn_config.json",
			builder:        &mvn.Mvn{},
		},
		testBuildType{
			buildFile:      "build.gradle",
			expectedConfig: "testdata/gradle_config.json",
			builder:        &gradle.Gradle{},
		},
	}
	for _, tc := range tCases {
		t.Run(tc.builder.Name(), func(t *testing.T) {
			td := makeTestDir(tc.buildFile, t)
			defer os.RemoveAll(td)
			r, ok := testBuilder(tc.builder, td)
			if !ok {
				t.Fatalf("Did not recognize %s file as a %s repo.", tc.buildFile, tc.builder.Name())
			}

			// Let's also make sure that foo_config.json stays in sync with foo.go,
			// because I've already forgotten to do that twice now.
			fn := testutil.TestFilePath(t, tc.expectedConfig)
			expected, err := ioutil.ReadFile(fn)
			if err != nil {
				t.Fatalf("Failed to open config file: %s", fn)
			}
			actual, err := ioutil.ReadAll(r)
			if err != nil {
				t.Fatalf("Failed to read default config")
			}
			if eq, diff := testutil.TrimmedEqual(expected, actual); !eq {
				t.Fatalf("Images were not equal, diff:\n%s", diff)
			}
		})
	}
}

// makeTestDir creates a temporary directory with a build file in it.  The
// caller is responsible for cleaning up the temp directory.
func makeTestDir(buildFile string, t *testing.T) string {
	t.Helper()
	td, err := ioutil.TempDir("", "testrepo")
	if err != nil {
		t.Fatalf("failed to create test dir %s: %v", buildFile, err)
	}
	tf := filepath.Join(td, buildFile)
	if err := ioutil.WriteFile(tf, []byte("not a real build file"), 0644); err != nil {
		t.Fatalf("failed to create test build file %s: %v", buildFile, err)
	}
	return td
}
