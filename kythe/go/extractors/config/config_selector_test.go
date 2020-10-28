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

	"kythe.io/kythe/go/test/testutil"

	"google.golang.org/protobuf/encoding/protojson"
	"google.golang.org/protobuf/proto"

	ecpb "kythe.io/kythe/proto/extraction_config_go_proto"
)

type testcase struct {
	name          string
	expectedError bool
	configFile    string
	buildFile     string
	expectedProto *ecpb.ExtractionConfiguration
}

func TestFindConfig(t *testing.T) {
	tCases := []testcase{
		testcase{
			name:          "maven-default",
			buildFile:     "pom.xml",
			expectedProto: loadProto("base/testdata/mvn_config.json", t),
		},
		// Note that if we're passed a config, technically we don't need to see
		// the config at this step in the process.  If we were being thorough,
		// the makeTestDir would populate a inner/nested/pom.xml, but I'm being
		// lazy.
		testcase{
			name:          "maven-passed",
			configFile:    "base/testdata/mvn2_config.json",
			expectedProto: loadProto("base/testdata/mvn2_config.json", t),
		},
		testcase{
			name:          "maven-missing-pom",
			expectedError: true,
		},
		testcase{
			name:          "maven-missing-config",
			expectedError: true,
			configFile:    "base/testdata/sir_not_appearing_in_this_repo.json",
		},
		// Even if there's a build.gradle file, if the config is directly
		// passed in, then trust the user.
		testcase{
			name:          "passed-overrides-default",
			buildFile:     "gradle.build",
			configFile:    "base/testdata/mvn2_config.json",
			expectedProto: loadProto("base/testdata/mvn2_config.json", t),
		},
		testcase{
			name:          "gradle-default",
			buildFile:     "build.gradle",
			expectedProto: loadProto("base/testdata/gradle_config.json", t),
		},
	}
	for _, tc := range tCases {
		t.Run(tc.name, func(t *testing.T) {
			td := makeTestDir(tc.buildFile, t)
			defer os.RemoveAll(td)
			p, err := findConfig(tc.configFile, td)
			if err != nil && !tc.expectedError {
				t.Fatalf("Unexpected error finding config %v", err)
			} else if err == nil && tc.expectedError {
				t.Fatalf("Expected error finding config %s, but didn't get one!", tc.name)
			} else if !tc.expectedError {
				if tc.expectedProto == nil {
					t.Fatalf("Test specification failure - should have an expected proto for case %s", tc.name)
				}

				if !proto.Equal(tc.expectedProto, p) {
					t.Fatalf("Expected and actual protos differ:\n%s\n\n%s", tc.expectedProto, p)
				}
			}
		})
	}
}

// makeTestDir creates a temporary directory, optionally with a build file.  The
// caller is responsible for cleaning up the temp directory.
func makeTestDir(buildFile string, t *testing.T) string {
	t.Helper()
	td, err := ioutil.TempDir("", "testrepo")
	if err != nil {
		t.Fatalf("failed to create test dir %s: %v", buildFile, err)
	}
	if buildFile != "" {
		tf := filepath.Join(td, buildFile)
		if err := ioutil.WriteFile(tf, []byte("not a real build file"), 0644); err != nil {
			t.Fatalf("failed to create test build file %s: %v", buildFile, err)
		}
	}
	return td
}

func loadProto(path string, t *testing.T) *ecpb.ExtractionConfiguration {
	t.Helper()
	tf := testutil.TestFilePath(t, path)
	f, err := os.Open(tf)
	if err != nil {
		t.Fatalf("Test specfication failure: failed to load config %s: %v", path, err)
	}
	defer f.Close()
	rec, err := ioutil.ReadAll(f)
	if err != nil {
		t.Fatalf("Test specification failure: failed to read config %s: %v", path, err)
	}
	extractionConfig := &ecpb.ExtractionConfiguration{}
	if err := protojson.Unmarshal(rec, extractionConfig); err != nil {
		t.Fatalf("Test specification failure: failed to parse config %s: %v", path, err)
	}

	return extractionConfig
}
