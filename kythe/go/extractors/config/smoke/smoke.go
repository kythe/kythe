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

// Package smoke is a basic harness for testing the validity of
// config.ExtractRepo output.
package smoke

import (
	"context"
	"fmt"
	"io/ioutil"
	"log"
	"os"
	"os/exec"
	"path/filepath"
	"strings"

	"kythe.io/kythe/go/extractors/config"
	"kythe.io/kythe/go/platform/kindex"
)

// Tester checks the validity of config.ExtractRepo.
//
// It does this in a generic way by simply trying to determine expected output
// automatically. The current implementation is to simply check the fraction of
// files covered by extractor output.
//
// TODO(danielmoy): hook up an indexing step and check actual semantic output
// instead of simple files.
//
// TODO(danielmoy): support more than just java.
type Tester interface {
	TestRepo(ctx context.Context, repo string) (Result, error)
}

// Fetcher is a thin wrapper over something which fetches a given repo and
// writes it to an output directory. Note that the ConfigPath parameter from
// config.Repo does not affect Fetch at all.
type Fetcher interface {
	Fetch(ctx context.Context, repo config.Repo) error
}

type gitCommandlineFetcher struct{}

func (g gitCommandlineFetcher) Fetch(ctx context.Context, repo config.Repo) error {
	// TODO(danielmoy): strongly consider go-git instead of os.exec
	return exec.CommandContext(ctx, "git", "clone", repo.URI, repo.OutputPath).Run()
}

type harness struct {
	extractor   config.Extractor
	configPath  string
	repoFetcher Fetcher
}

// NewGitTestingHarness creates a simple Tester which uses
// config.DefaultExtractor to perform repo extraction, and a simple git clone
// command to fetch files used to determine expected output.
//
// An extraction config can be optionally read from a specified file.  The
// format follows kythe.proto.ExtractionConfiguration.
func NewGitTestingHarness(configPath string) Tester {
	return harness{
		extractor:   config.DefaultExtractor{},
		configPath:  configPath,
		repoFetcher: gitCommandlineFetcher{},
	}
}

// Result is a simple container for the results of a single repo test.  It may
// contain useful information about whether or not the repo was accessible,
// extracted at all, or the extent to which we got good file coverage from the
// extraction.
//
// TODO(danielmoy): consider better metrics here. For example consider having
// the smoke test harness try to run a kythe indexer in addition to an
// extraction and see how much symbol coverage we have.  This might be out of
// scope for a simple smoke test harness though.
type Result struct {
	// Whether the repo was successfully downloaded or extracted.
	Downloaded, Extracted bool
	// The number of downloaded and extracted files.
	DownloadCount, ExtractCount int
	// The percentage of files in the repo that are covered by extraction.
	// Should be in range [0.0, 1.0]
	FileCoverage float64
}

func (g harness) TestRepo(ctx context.Context, repo string) (Result, error) {
	fromRepo, err := g.filenamesFromRepo(ctx, repo)
	if err != nil {
		log.Printf("Failed to read repo from remote: %v", err)
		return Result{false, false, len(fromRepo), 0, 0.0}, nil
	}

	fromExtraction, err := g.filenamesFromExtraction(ctx, repo)
	if err != nil {
		log.Printf("Failed to extract repo: %v", err)
		// TODO(danielmoy): consider handling errors independently and
		// returning separate false results if either err != nil.
		return Result{true, false, len(fromRepo), len(fromExtraction), 0.0}, nil
	}

	var coverageTotal int32
	var coverageCount int32
	for k := range fromRepo {
		coverageTotal = coverageTotal + 1
		if _, ok := fromExtraction[k]; ok {
			coverageCount = coverageCount + 1
		}
	}

	var coverage float64
	if coverageTotal > 0 {
		coverage = float64(coverageCount) / float64(coverageTotal)
	}
	return Result{
		Downloaded:    true,
		Extracted:     true,
		DownloadCount: len(fromRepo),
		ExtractCount:  len(fromExtraction),
		FileCoverage:  coverage,
	}, nil
}

func (g harness) filenamesFromRepo(ctx context.Context, repoURI string) (map[string]bool, error) {
	repoName := pathTail(repoURI)

	repoDir, err := ioutil.TempDir("", repoName)
	if err != nil {
		return nil, fmt.Errorf("failed to create temp dir for repo %s: %v", repoURI, err)
	}
	defer os.RemoveAll(repoDir)

	if err = g.repoFetcher.Fetch(ctx, config.Repo{
		URI:        repoURI,
		OutputPath: repoDir,
	}); err != nil {
		return nil, err
	}

	ret := map[string]bool{}
	err = filepath.Walk(repoDir, func(path string, info os.FileInfo, err error) error {
		// TODO(danielmoy): make this parameterized based on the
		// extractor, e.g. supporting other languages.
		if err == nil && filepath.Ext(path) == ".java" {
			// We are only interested in the repo-relative path.
			rp, err := filepath.Rel(repoDir, path)
			if err != nil {
				return err
			}
			ret[rp] = true
		}
		return err
	})
	return ret, err
}

func (g harness) filenamesFromExtraction(ctx context.Context, repoURI string) (map[string]bool, error) {
	repoName := pathTail(repoURI)
	tmpOutDir, err := ioutil.TempDir("", repoName)
	if err != nil {
		return nil, fmt.Errorf("failed to create temp dir for repo %s: %v", repoURI, err)
	}
	defer os.RemoveAll(tmpOutDir)

	if err := g.extractor.ExtractRepo(ctx, config.Repo{
		URI:        repoURI,
		OutputPath: tmpOutDir,
		ConfigPath: g.configPath,
	}); err != nil {
		return nil, err
	}
	ret := map[string]bool{}
	err = filepath.Walk(tmpOutDir, func(path string, info os.FileInfo, err error) error {
		if err == nil && filepath.Ext(path) == ".kindex" {
			cu, err := kindex.Open(ctx, path)
			if err != nil {
				return err
			}
			if cu.Proto != nil {
				for _, v := range cu.Proto.SourceFile {
					if strings.HasSuffix(v, ".java") {
						ret[v] = true
					}
				}
			}
		}
		return err
	})

	return ret, err
}

func pathTail(path string) string {
	return path[strings.LastIndex(path, "/")+1:]
}
