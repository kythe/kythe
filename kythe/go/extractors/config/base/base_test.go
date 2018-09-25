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

package base

import (
	"io/ioutil"
	"os"
	"path/filepath"
	"strings"
	"testing"

	"kythe.io/kythe/go/test/testutil"
)

func TestDefaultConfig(t *testing.T) {
	expectedConfig := map[string]string{
		"maven":  "testdata/mvn_config.json",
		"gradle": "testdata/gradle_config.json",
	}
	tCases := supportedBuilders()
	for _, tc := range tCases {
		t.Run(tc.Name, func(t *testing.T) {
			td := makeTestDir(tc.BuildFile, t)
			defer os.RemoveAll(td)
			r, ok := DefaultConfig(td)
			if !ok {
				t.Fatalf("Did not recognize %s file as a %s repo.", tc.BuildFile, tc.Name)
			}

			// Let's also make sure that foo_config.json stays in sync with foo.go,
			// because I've already forgotten to do that twice now.
			conf, ok := expectedConfig[tc.Name]
			if !ok {
				t.Fatalf("Unsupported builder %s", tc.Name)
			}
			fn := testutil.TestFilePath(t, conf)
			expected, err := ioutil.ReadFile(fn)
			if err != nil {
				t.Fatalf("Failed to open config file: %s", fn)
			}
			actual, err := ioutil.ReadAll(strings.NewReader(r))
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
