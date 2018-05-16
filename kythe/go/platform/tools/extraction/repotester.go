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

// Binary repotester tests the high level validity of repo extraction. It tries
// to run similar logic to sibling binary extractrepo on a specified target
// repositories, and reports results.
//
// For simple extraction, the results are merely based on the fraction of java
// files in the repo that end up in the kindex files.
//
// An extraction config can be optionally read from a specified file.  The
// format follows kythe.proto.ExtractionConfiguration.
//
// Usage:
//   repotester -repos <comma_delimited,repo_urls> [-config <config_file_path>]
//   repotester -repo_list_file <file> [-config <config_file_path>]
package main

import (
	"bufio"
	"context"
	"flag"
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

var (
	repos      = flag.String("repos", "", "A comma delimited list of repos to test")
	reposFile  = flag.String("repo_list_file", "", "A file that contains a newline delimited list of repos to test")
	configPath = flag.String("config", "", "An optional config file to specify kythe.proto.ExtractionConfiguration logic")
)

func init() {
	flag.Usage = func() {
		binary := filepath.Base(os.Args[0])
		fmt.Fprintf(os.Stderr, `Usage: %s -repos <comma_delimited,repo_urls>
%s -repo_list_file <file_containing_line_delimited_repo_urls>

This tool tests repo extraction by comparing extractrepo results with the
contents of the actual repository itself.

This binary requires both Git and Docker to be on the $PATH during execution.

Options:
`, binary, binary)
		flag.PrintDefaults()
	}
}

func main() {
	flag.Parse()
	verifyFlags()

	// Print some header
	fmt.Printf("|%9s |%9s |%9s | %s\n", "download", "extract", "coverage", "repo")
	fmt.Printf("|%9s |%9s |%9s |%s\n", " ----", " ----", " ----", " ----")

	repos, err := getRepos()
	if err != nil {
		log.Fatalf("Failed to get repos to read: %v", err)
	}
	for _, repo := range repos {
		res, err := testRepo(repo)
		if err != nil {
			log.Printf("Failed to test repo: %s", err)
		} else {
			fmt.Printf("|%9t |%9t |      %2.0f%% | %s\n", res.downloaded, res.extracted, 100*res.fileCoverage, repo)
		}
	}
}

func verifyFlags() {
	if flag.NArg() > 0 {
		log.Fatalf("Unknown arguments: %v", flag.Args())
	}
	if (*repos == "" && *reposFile == "") || (*repos != "" && *reposFile != "") {
		log.Fatalf("Must specify one of -repos or -repo_list_file, but no both.")
	}
}

func getRepos() ([]string, error) {
	switch {
	case *repos != "":
		return strings.Split(*repos, ","), nil
	case *reposFile != "":
		return getReposFromFile()
	default:
		return nil, fmt.Errorf("Invalid state - need a source of repos")
	}
}

func getReposFromFile() ([]string, error) {
	file, err := os.Open(*reposFile)
	if err != nil {
		return nil, fmt.Errorf("cannot read repo file: %v", err)
	}
	defer file.Close()

	ret := []string{}
	scanner := bufio.NewScanner(file)
	// TODO(danielmoy): consider supporting separate configs per repo. This
	// will become more necessary once we have more customzied configs.
	for scanner.Scan() {
		ret = append(ret, scanner.Text())
	}
	if err := scanner.Err(); err != nil {
		return nil, fmt.Errorf("error reading repo file: %v", err)
	}
	return ret, nil
}

// result is a simple container for the results of a single repo test.  It may
// contain useful information about whether or not the repo was accessible,
// extracted at all, or the extent to which we got good file coverage from the
// extraction.
type result struct {
	// Whether the repo was successfully downloaded or extracted.
	downloaded, extracted bool
	// Should be in range [0.0, 1.0]
	fileCoverage float32
}

func testRepo(repo string) (result, error) {
	fromExtraction, err := filenamesFromExtraction(repo)
	if err != nil {
		log.Printf("Failed to extract repo: %v", err)
		// TODO(danielmoy): consider handling errors independently and
		// returning separate false results if either err != nil.
		return result{false, false, 0.0}, nil
	}
	fromRepo, err := filenamesFromRepo(repo)
	if err != nil {
		log.Printf("Failed to read repo from remote: %v", err)
		return result{false, true, 0.0}, nil
	}

	var coverageTotal int32
	var coverageCount int32
	// TODO(danielmoy): the repos won't necessarily line up properly. This
	// needs to be fixed to be more extensible. Potentially with a suffix
	// trie on successive path elements (basename and then directory
	// backwards).
	for k := range fromRepo {
		coverageTotal = coverageTotal + 1
		if _, ok := fromExtraction[k]; ok {
			coverageCount = coverageCount + 1
		}
	}

	return result{
		downloaded:   true,
		extracted:    true,
		fileCoverage: float32(coverageCount) / float32(coverageTotal),
	}, nil
}

// gitpath is a container for storing actual paths, since what github API calls
// "path" is actually just a basename, and we need the full path.
type gitpath struct {
	sha, path string
}

func filenamesFromRepo(repoURL string) (map[string]bool, error) {
	repoName := pathTail(repoURL)

	// Make a temp dir.
	repoDir, err := ioutil.TempDir("", repoName)
	if err != nil {
		return nil, fmt.Errorf("failed to create temp dir for repo %s: %v", repoURL, err)
	}
	defer os.RemoveAll(repoDir)

	// TODO(danielmoy): strongly consider go-git instead of os.exec
	if err = exec.Command("git", "clone", repoURL, repoDir).Run(); err != nil {
		return nil, fmt.Errorf("cloning repo: %v", err)
	}

	ret := map[string]bool{}
	err = filepath.Walk(repoDir, func(path string, info os.FileInfo, err error) error {
		// TODO(danielmoy): make this parameterized based on the
		// extractor, e.g. supporting other languages.
		if err == nil && filepath.Ext(path) == ".java" {
			ret[path] = true
		}
		return err
	})
	return ret, err
}

func filenamesFromExtraction(repoURL string) (map[string]bool, error) {
	repoName := pathTail(repoURL)
	tmpOutDir, err := ioutil.TempDir("", repoName)
	if err != nil {
		return nil, fmt.Errorf("failed to create temp dir for repo %s: %v", repoURL, err)
	}
	defer os.RemoveAll(tmpOutDir)

	err = config.ExtractRepo(repoURL, tmpOutDir, *configPath)
	ret := map[string]bool{}
	if err != nil {
		return ret, err
	}
	err = filepath.Walk(tmpOutDir, func(path string, info os.FileInfo, err error) error {
		if err == nil && filepath.Ext(path) == ".kindex" {
			cu, err := kindex.Open(context.Background(), path)
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
