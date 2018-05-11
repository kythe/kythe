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
	"path"
	"path/filepath"
	"regexp"
	"strings"

	"github.com/google/go-github/github"
	// TODO(danielmoy): auth!
	// "golang.org/x/oauth2"
	"kythe.io/kythe/go/extractors/config"
	"kythe.io/kythe/go/platform/kindex"
)

var (
	repos     = flag.String("repos", "", "A comma delimited list of repos to test.")
	reposFile = flag.String("repo_list_file", "", "A file that contains a newline delimited list of repos to test.")
	// TODO(danielmoy): auth!
	// githubToken = flag.String("github_token", "", "An oauth2 token to contact github with. https://help.github.com/articles/creating-a-personal-access-token-for-the-command-line/ to generate.")
	configPath = flag.String("config", "", "An optional config to specify for every repo.")
)

func init() {
	flag.Usage = func() {
		binary := filepath.Base(os.Args[0])
		fmt.Fprintf(os.Stderr, `Usage: %s -repos <comma_delimited,repo_urls>
%s -repo_list_file <file_containing_line_delimited_repo_urls>

This tool tests repo extraction. If specifying file list format, you can also
specify a config file comma separated after the repo:

https://repo.url, /file/path/to/config

Any config specified in this way overwrites the default top-level -config passed
as a binary flag.

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
	// TODO(danielmoy): auth!
	// if *githubToken == "" {
	// 	log.Fatalf("Must specify -github_token.")
	// }
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
	// TODO(danielmoy): auth!
	// src := oauth2.StaticTokenSource(&oauth2.Token{AccessToken: *githubToken})
	// httpClient := oauth.NewClient(context.Background(), src)

	owner, repoName, err := getNames(repoURL)
	if err != nil {
		return nil, err
	}

	client := github.NewClient(nil)
	rootTree, err := getRootTree(client, owner, repoName)
	if err != nil {
		return nil, err
	}
	if rootTree == nil {
		return nil, fmt.Errorf("Failed to get commit tree for repo %s/%s", owner, repoName)
	}

	// The tree will recursively contain stuff, so build up a queue-like thingy
	// and go to work.
	if rootTree.SHA == nil {
		return nil, fmt.Errorf("Failed to get any tree data for repo %s/%s", owner, repoName)
	}
	trees := []gitpath{gitpath{*rootTree.SHA, ""}}

	ret := map[string]bool{}
	// TODO(danielmoy): consider parallelism in here, within reasonable
	// bounds given rate limiting.
	for len(trees) > 0 {
		tree := trees[0]
		trees = trees[1:]
		contents, err := readTree(client, owner, repoName, tree.sha)
		if err != nil {
			return nil, err
		}
		if contents == nil {
			return nil, fmt.Errorf("failed to read repo %s/%s tree %s", owner, repoName, tree.sha)
		}
		for _, entry := range contents.Entries {
			if entry.SHA == nil || entry.Path == nil {
				return nil, fmt.Errorf("failed to read repo %s/%s tree %s", owner, repoName, tree.sha)
			}
			newpath := path.Join(tree.path, *entry.Path)
			switch *entry.Type {
			case "blob":
				appendFile(ret, newpath)
			case "tree":
				if entry.SHA != nil {
					trees = append(trees, gitpath{*entry.SHA, newpath})
				}
			default:
				log.Printf("Unknown tree entry %s", entry.Type)
			}
		}
	}

	return ret, nil
}

func getRootTree(client *github.Client, owner, repo string) (*github.Tree, error) {
	opt := &github.CommitsListOptions{ListOptions: github.ListOptions{PerPage: 1}}
	repos, _, err := client.Repositories.ListCommits(context.Background(), owner, repo, opt)

	if err != nil {
		return nil, err
	}
	if len(repos) == 0 {
		return nil, fmt.Errorf("failed to find latest commit for repo %s/%s", owner, repo)
	}
	if len(repos) > 1 {
		log.Fatalf("Somehow got more than one commit for repo %s/%s", owner, repo)
	}
	sha := repos[0].SHA
	if sha == nil {
		return nil, fmt.Errorf("failed to get commit hash for repo %s/%s commit %v", owner, repo, repos[0])
	}
	commit, _, err := client.Git.GetCommit(context.Background(), owner, repo, *sha)
	if commit == nil || err != nil {
		return nil, fmt.Errorf("failed to get commit data for repo %s/%s commit %v", owner, repo, repos[0])
	}
	return commit.Tree, nil
}

// getNames tries to extract the owner/repo from a github repo url.  If the
// passed string is not supported, returns an error to that effect.
func getNames(repo string) (string, string, error) {
	re := regexp.MustCompile(`https://github.com/(?P<Owner>\w+)/(?P<Repo>[\w-_]+)`)
	n := re.FindStringSubmatch(repo)
	// Recall that regex libraries like returning the whole matched thing as
	// the first bit, because... reasons.  Anyways just ignore it.
	if n == nil || len(n) != 3 {
		return "", "", fmt.Errorf("failed to parse repo %s", repo)
	}
	return n[1], n[2], nil
}

func readTree(client *github.Client, owner, repo, treeSHA string) (*github.Tree, error) {
	// Note we don't read recursively (, false), because it only supports
	// max of 200 files.
	tree, _, err := client.Git.GetTree(context.Background(), owner, repo, treeSHA, false)
	if tree == nil || err != nil {
		return nil, err
	}
	return tree, nil
}

// appendFile sees if this is a supported file type, and then wappends it to the
// list of known files.
func appendFile(ret map[string]bool, path string) {
	if strings.HasSuffix(path, ".java") {
		ret[path] = true
	}
	return
}

func filenamesFromExtraction(repo string) (map[string]bool, error) {
	_, repoName, err := getNames(repo)
	tmpOutDir, err := ioutil.TempDir("", repoName)
	if err != nil {
		return nil, fmt.Errorf("failed to create temp dir for repo %s: %v", repo, err)
	}
	defer os.RemoveAll(tmpOutDir)

	err = config.ExtractRepo(repo, tmpOutDir, *configPath)
	ret := map[string]bool{}
	if err != nil {
		return ret, err
	}
	walkFunc := func(path string, info os.FileInfo, err error) error {
		if err != nil {
			return err
		}
		if strings.HasSuffix(path, ".kindex") {
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
		return nil
	}

	err = filepath.Walk(tmpOutDir, walkFunc)
	return ret, err
}
