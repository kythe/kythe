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

// Binary extractrepo provides a tool to run a Kythe extraction on a
// specified repository.  This binary requires both Git and Docker to be on the
// $PATH during execution.
//
// Usage:
//   extractrepo -remote <repo-url> -output <output-file-path> -config [config-file-path]
package main

import (
	"context"
	"flag"
	"fmt"
	"io/ioutil"
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
	// TODO(#156): Remove these flags after we get rid of docker-in-docker.
	tempRepoDir = flag.String("tmp_repo_dir", "", "Path for inner docker copy of input repo. Should be an empty directory.")
	tempOutDir  = flag.String("tmp_out_dir", "", "Path for inner docker copy of output. Should be an empty directory.")
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

func checkFlags() {
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

	if *tempRepoDir != "" && !isEmptyDir(*tempRepoDir) {
		hasError = true
		log.Printf("Error: -tmp_repo_dir %q should be an empty directory", *tempRepoDir)
	}

	if *tempOutDir != "" && !isEmptyDir(*tempOutDir) {
		hasError = true
		log.Printf("Error: -tmp_out_dir %q should be an empty directory", *tempOutDir)
	}

	if hasError {
		os.Exit(1)
	}
}

// isEmptyDir reports whether dir is an empty directory.  If dir does not exist,
// an empty directory is created at that path.
func isEmptyDir(dir string) bool {
	fi, err := ioutil.ReadDir(dir)
	if os.IsNotExist(err) {
		return os.MkdirAll(dir, 0755) == nil
	} else if err != nil {
		log.Printf("Error checking directory: %v", err)
	}
	return err == nil && len(fi) == 0
}

func main() {
	flag.Parse()
	checkFlags()

	ctx, cancel := context.WithTimeout(context.Background(), *timeout)
	defer cancel()
	repo := config.Repo{
		Git:         *repoURI,
		Local:       *repoPath,
		OutputPath:  *outputPath,
		ConfigPath:  *configPath,
		TempRepoDir: *tempRepoDir,
		TempOutDir:  *tempOutDir,
	}
	if err := config.ExtractRepo(ctx, repo); err != nil {
		log.Fatalf("Failed to extract repo: %v", err)
	}
}
