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

// Binary extractrepo provides a tool to run a Kythe extraction on a
// specified repository.  This binary requires both Git and Docker to be on the
// $PATH during execution.
//
// Usage:
//   extractrepo -repo <repo-url> -output <output-file-path> -config [config-file-path]
package main

import (
	"context"
	"flag"
	"fmt"
	"io"
	"log"
	"os"
	"path/filepath"
	"time"

	"kythe.io/kythe/go/extractors/config"
)

var (
	repoURI    = flag.String("remote", "", "URI of the repository containing the project to be extracted")
	repoPath   = flag.String("local", "", "Existing local directory copy of the repository containing the project to be extracted")
	outputPath = flag.String("output", "", "Path for output kindex file")
	configPath = flag.String("config", "", "Path for the JSON extraction configuration file")
	timeout    = flag.Duration("timeout", 2*time.Minute, "Timeout for extraction")
	// TODO(#156): Remove this flag after we get rid of docker-in-docker.
	tempRepoDir = flag.String("tmp_repo_dir", "", "Path for inner docker copy of input repo. Should be an empty directory.")
)

func init() {
	flag.Usage = func() {
		fmt.Fprintf(os.Stderr, `Usage: %s -remote <repo_uri> -output <path> -config [config_file_path]

This tool extracts compilation records from a repository utilizing a JSON
extraction configuration encoding a kythe.proto.ExtractionConfiguration.

The originating repo to extract is specified either by -remote <repo-url> and
done via a git clone, or -local <repo-path> and simply copying the repo over.
Specifying both -remote and -local is invalid input.

The configuration is specified either via the -config flag, or else within
a configuration file located at the root of the repository named:
".kythe-extraction-config". It makes a local clone of the specified repository
and then uses the config to generate a customized extraction Docker image.

Finally it executes the extraction by running the extraction image, the output
of which is a kindex file as defined here:  http://kythe.io/docs/kythe-index-pack.html

This binary requires both Git and Docker to be on the $PATH during execution.

Options:
`, filepath.Base(os.Args[0]))
		flag.PrintDefaults()
	}
}

func verifyFlags() {
	if flag.NArg() > 0 {
		log.Fatalf("Unknown arguments: %v", flag.Args())
	}

	hasError := false
	if *repoURI == "" && *repoPath == "" {
		hasError = true
		log.Println("You must provide a non-empty -remote or -local")
	}
	if *repoURI != "" && *repoPath != "" {
		hasError = true
		log.Println("You must specify only one of -remote or -local, not both")
	}

	if *outputPath == "" {
		hasError = true
		log.Println("You must provide a non-empty -output")
	}

	if *tempRepoDir != "" {
		if !empty(*tempRepoDir) {
			hasError = true
			log.Println("-tmp_repo_dir must be an empty directory.")
		}
	}

	if hasError {
		os.Exit(1)
	}
}

func empty(dir string) bool {
	stat, err := os.Stat(dir)
	if os.IsNotExist(err) {
		return os.MkdirAll(dir, 0755) == nil
	}
	if err != nil {
		log.Printf("Unexpected stat problem: %v", err)
		return false
	}
	if !stat.IsDir() {
		return false
	}
	f, err := os.Open(dir)
	if err != nil {
		log.Printf("Failed to open dir: %v", err)
		return false
	}
	defer f.Close()

	_, err = f.Readdirnames(1)
	if err == io.EOF {
		return true
	}
	return false
}

// kytheConfigFileName The name of the Kythe extraction config
const kytheExtractionConfigFile = ".kythe-extraction-config"

func main() {
	flag.Parse()
	verifyFlags()

	ctx, cancel := context.WithTimeout(context.Background(), *timeout)
	defer cancel()
	repo := config.Repo{
		Git:         *repoURI,
		Local:       *repoPath,
		OutputPath:  *outputPath,
		ConfigPath:  *configPath,
		TempRepoDir: *tempRepoDir,
	}
	if err := config.ExtractRepo(ctx, repo); err != nil {
		log.Fatalf("Failed to extract repo: %v", err)
	}
}
