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

package smoke

import (
	"context"
	"fmt"
	"io/ioutil"
	"math"
	"os"
	"path/filepath"
	"testing"

	"kythe.io/kythe/go/extractors/config"
	"kythe.io/kythe/go/platform/kindex"

	apb "kythe.io/kythe/proto/analysis_go_proto"
	spb "kythe.io/kythe/proto/storage_go_proto"
)

type testExtractor struct {
	repoFiles       map[string][]byte
	extractionFiles []string
	separateKindex  bool
}

type container struct {
	requiredInput []string
	kindexName    string
}

func (e testExtractor) ExtractRepo(ctx context.Context, repo config.Repo) error {
	af := []container{}
	if e.separateKindex {
		for i, f := range e.extractionFiles {
			af = append(af, container{
				requiredInput: []string{f},
				kindexName:    fmt.Sprintf("fake%d.kindex", i),
			})
		}
	} else {
		af = append(af, container{
			requiredInput: e.extractionFiles,
			kindexName:    "fake.kindex",
		})

	}
	return writeKindex(af, repo.OutputPath)
}

func writeKindex(allFiles []container, outputPath string) error {
	for _, files := range allFiles {
		cu := &kindex.Compilation{
			Proto: &apb.CompilationUnit{
				VName: &spb.VName{
					Corpus: "test",
					Path:   "fakepath",
				},
				SourceFile: files.requiredInput,
			},
		}

		err := writeSingleKindex(filepath.Join(outputPath, files.kindexName), cu)
		if err != nil {
			return err
		}
	}
	return nil
}

func writeSingleKindex(path string, unit *kindex.Compilation) error {
	f, err := os.Create(path)
	if err != nil {
		return err
	} else if _, err := unit.WriteTo(f); err != nil {
		f.Close()
		return err
	}
	return f.Close()
}

func (e testExtractor) Fetch(ctx context.Context, _, outputPath string) error {
	for k, v := range e.repoFiles {
		if err := ioutil.WriteFile(filepath.Join(outputPath, k), v, 0444); err != nil {
			return err
		}
	}
	return nil
}

func TestBasicExtraction(t *testing.T) {
	rf := map[string][]byte{
		"foo.java": []byte("stuff"),
		"bar.java": []byte("stuff"),
	}
	ef := []string{"foo.java", "bar.java"}

	te := testExtractor{
		repoFiles:       rf,
		extractionFiles: ef,
	}

	verifyRepoCoverage(t, te, 2, 2, 1.0)
}

func TestMissingRepoFiles(t *testing.T) {
	rf := map[string][]byte{
		"foo.java": []byte("stuff"),
		"bar.java": []byte("stuff"),
	}
	ef := []string{"foo.java"}

	te := testExtractor{
		repoFiles:       rf,
		extractionFiles: ef,
	}

	verifyRepoCoverage(t, te, 2, 1, 0.5)
}

func TestMultipleKindexFiles(t *testing.T) {
	rf := map[string][]byte{
		"foo.java": []byte("stuff"),
		"bar.java": []byte("stuff"),
	}
	ef := []string{"foo.java", "bar.java"}

	te := testExtractor{
		repoFiles:       rf,
		extractionFiles: ef,
		separateKindex:  true,
	}

	verifyRepoCoverage(t, te, 2, 2, 1.0)
}

// verifyRepoCoverage tests that a given testExtractor yields a specific
// number of files downloaded, extracted, and percentage coverage. The passed
// testExtractor should be set up with all of the relevant input repo and
// extraction files.
func verifyRepoCoverage(t *testing.T, te testExtractor, expectedDownloadCount, expectedExtractCount int, expectedCoverage float64) {
	t.Helper()
	h := Harness{
		Fetcher:    te.Fetch,
		Extractor:  te.ExtractRepo,
		ConfigPath: "",
		Indexer:    nil,
	}

	r, err := h.TestRepo(context.Background(), "foo")
	if err != nil {
		t.Fatalf("failed to test repo: %v", err)
	}
	if !r.Downloaded {
		t.Error("failed to download")
	}
	if !r.Extracted {
		t.Error("failed to extract")
	}
	if expectedDownloadCount != r.DownloadCount {
		t.Errorf("expected %d downloaded files, got %d", expectedDownloadCount, r.DownloadCount)
	}
	if expectedExtractCount != r.ExtractCount {
		t.Errorf("expected %d extracted files, got %d", expectedExtractCount, r.ExtractCount)
	}
	if !floatEquals(r.FileCoverage, expectedCoverage) {
		t.Errorf("expected %.3f expectedCoverage, got %.3f", expectedCoverage, r.FileCoverage)
	}
}

const eps float64 = 0.0001

func floatEquals(a, b float64) bool { return math.Abs(a-b) < eps }
