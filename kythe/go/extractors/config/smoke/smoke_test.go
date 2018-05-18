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

package smoke

import (
	"io/ioutil"
	"math"
	"path/filepath"
	"testing"
)

type testExtractor struct {
	dummyFiles map[string][]byte
	downloaded []string
	extracted  []string
}

func (e *testExtractor) ExtractRepo(repoURI, outputPath, configPath string) error {
	for k, v := range e.dummyFiles {
		p := filepath.Join(outputPath, k)
		if err := ioutil.WriteFile(p, v, 0444); err != nil {
			return err
		}
		e.extracted = append(e.extracted, p)
	}
	return nil
}

func (e *testExtractor) Fetch(repoURL, outputPath string) error {
	j := filepath.Join(outputPath, "foo.java")
	// Write a java file
	if err := ioutil.WriteFile(j, []byte("package main;\n"), 0444); err != nil {
		return err
	}
	e.downloaded = append(e.downloaded, j)
	// Write a BUILD file
	if err := ioutil.WriteFile(filepath.Join(outputPath, "BUILD"), []byte("// a build file\n"), 0444); err != nil {
		return err
	}
	return nil
}

func TestBasicExtraction(t *testing.T) {
	m := make(map[string][]byte)
	m["foo.java"] = []byte("we don't actually verify file contents, woops")
	te := testExtractor{
		dummyFiles: m,
		downloaded: []string{},
		extracted:  []string{},
	}
	h := harness{
		extractor:   &te,
		configPath:  "",
		repoFetcher: &te,
	}

	r, err := h.TestRepo("foo")
	if err != nil {
		t.Fatalf("failed to test repo: %v", err)
	}
	if !r.Downloaded {
		t.Error("failed to download")
	}
	if !r.Extracted {
		t.Error("failed to extract")
	}
	// TODO(danielmoy): uncomment this when we fix the test harness to do
	// better file matching.
	//	if !floatEquals(r.FileCoverage, 1.0) {
	//		t.Errorf("expected 1.0 coverage, got %.3f", r.FileCoverage)
	//		t.Errorf("downloaded files: %v, extracted files: %v", te.downloaded, te.extracted)
	//	}
}

const eps float64 = 0.0001

func floatEquals(a, b float64) bool {
	if math.Abs(a-b) < eps {
		return true
	}
	return false
}
