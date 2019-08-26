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

// Binary repotester tests the high level validity of repo extraction. It tries
// to run similar logic to sibling binary extractrepo on a specified target
// repositories, and reports results in csv format.
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
	"encoding/csv"
	"flag"
	"fmt"
	"log"
	"os"
	"path/filepath"
	"strconv"
	"strings"
	"time"

	"kythe.io/kythe/go/extractors/config/smoke"
)

var (
	repos      = flag.String("repos", "", "A comma delimited list of repos to test")
	reposFile  = flag.String("repo_list_file", "", "A file that contains a newline delimited list of repos to test")
	configPath = flag.String("config", "", "An optional config file to specify kythe.proto.ExtractionConfiguration logic")
	index      = flag.Bool("index", false, "Whether to attempt to index a repo after successful extraction")
	timeout    = flag.Duration("timeout", 2*time.Minute, "Timeout for testing an individual repo.")
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

	w := csv.NewWriter(os.Stdout)
	w.Write([]string{"downloads", "extracts", "downloadfilecount", "extractfilecount", "coverage", "repo"})
	w.Flush()

	repos, err := getRepos()
	if err != nil {
		log.Fatalf("Failed to get repos to read: %v", err)
	}

	h := &smoke.Harness{
		ConfigPath: *configPath,
	}

	if *index {
		h.Indexer = smoke.EmptyIndexer
	}

	for _, repo := range repos {
		ctx, cancel := context.WithTimeout(context.Background(), *timeout)
		defer cancel()
		res, err := h.TestRepo(ctx, repo)
		if err != nil {
			log.Printf("Failed to test repo: %s", err)
		} else {
			w.Write([]string{
				strconv.FormatBool(res.Downloaded),
				strconv.FormatBool(res.Extracted),
				strconv.Itoa(res.DownloadCount),
				strconv.Itoa(res.ExtractCount),
				strconv.FormatFloat(res.FileCoverage, 'f', 2, 64),
				repo,
			})
			w.Flush()
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
		return nil, fmt.Errorf("invalid state - need a source of repos")
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
	// will become more necessary once we have more customized configs.
	for scanner.Scan() {
		ret = append(ret, scanner.Text())
	}
	if err := scanner.Err(); err != nil {
		return nil, fmt.Errorf("error reading repo file: %v", err)
	}
	return ret, nil
}
