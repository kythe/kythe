/*
 * Copyright 2018 Google Inc. All rights reserved.
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
	"log"
	"os"
	"path/filepath"
	"regexp"
	"strings"
	"testing"

	"github.com/golang/protobuf/jsonpb"
	"github.com/google/go-cmp/cmp"

	cp "kythe.io/kythe/go/extractors/config/config_parser"
	ecp "kythe.io/kythe/proto/extraction_config_go_proto"
)

const testDataDir = "$TEST_SRCDIR/testdata"

func openTestDataFile(fileName string) (*os.File, error) {
	return os.Open(os.ExpandEnv(filepath.Join(testDataDir, fileName)))
}

func mustLoadConfig(fileName string) *ecp.ExtractionConfiguration {
	testConfigFile, err := openTestDataFile(fileName)
	if err != nil {
		log.Panicf("Failed to load test config file: %v", err)
	}

	testConfig := ecp.ExtractionConfiguration{}
	if err := jsonpb.Unmarshal(testConfigFile, &testConfig); err != nil {
		log.Panicf("Failed to unmarshal test config: %v", err)
	}

	return &testConfig
}

var multipleNewLines = regexp.MustCompile("\n{2,}")

func imagesEqual(got, want []byte) (bool, string) {
	// remove superfluous whitespace
	gotStr := strings.Trim(string(got[:]), " \n")
	wantStr := strings.Trim(string(want[:]), " \n")
	gotStr = multipleNewLines.ReplaceAllString(gotStr, "\n")
	wantStr = multipleNewLines.ReplaceAllString(wantStr, "\n")

	// diff want vs got
	diff := cmp.Diff(gotStr, wantStr)
	if diff != "" {
		return false, diff
	}

	return true, ""
}

func TestNewExtractionImageGeneratesExpectedDockerFiles(t *testing.T) {
	testConfigFiles, err := filepath.Glob(fmt.Sprintf("%s/*.json", testDataDir))
	if err != nil {
		t.Fatalf("\nFailed to glob for test config files: %v\n", err)
	}

	for _, testConfigFile := range testConfigFiles {
		testConfig := mustLoadConfig(filepath.Base(testConfigFile))

		extractionImageContent, err := cp.NewExtractionImage(testConfig)
		if err != nil {
			t.Fatalf("\nFailed to parse test config: %v\n", err)
		}

		expectedDockerFileName := fmt.Sprintf("expected_%s.Dockerfile", strings.Replace(filepath.Base(testConfigFile), ".json", "", 1))
		expectedImageFile, err := openTestDataFile(expectedDockerFileName)
		if err != nil {
			t.Fatalf("\nFailed to open expected test data file: %v\n", err)
		}

		expectedImageContent, err := ioutil.ReadAll(expectedImageFile)
		if err != nil {
			t.Fatalf("\nFailed to load test expected data: %v\n", err)
		}

		if eq, diff := imagesEqual(extractionImageContent, expectedImageContent); !eq {

			t.Fatalf("[Failed]: Images were not equal, diff:\n%s", diff)
		}
	}
}
