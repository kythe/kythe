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
//   extractrepo -repo <repo_uri> -output <output_file_path> -config [config_file_path]
package main

import (
	"flag"
	"fmt"
	"log"
	"os"
	"path/filepath"

	"kythe.io/kythe/go/extractors/config"
)

var (
	repoURI    = flag.String("repo", "", "URI of the repository containing the project to be extracted")
	outputPath = flag.String("output", "", "Path for output kindex file")
	configPath = flag.String("config", "", "Path for the JSON extraction configuration file")
)

func init() {
	flag.Usage = func() {
		fmt.Fprintf(os.Stderr, `Usage: %s -repo <repo_uri> -output <path> -config [config_file_path]

This tool extracts compilation records from a repository utilizing a JSON
extraction configuration encoding a kythe.proto.ExtractionConfiguration.
This configuration is specified either via the -config flag, or else within
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
	if *repoURI == "" {
		hasError = true
		log.Println("You must provide a non-empty -repo")
	}

	if *outputPath == "" {
		hasError = true
		log.Println("You must provide a non-empty -output")
	}

	if hasError {
		os.Exit(1)
	}
}

// kytheConfigFileName The name of the Kythe extraction config
const kytheExtractionConfigFile = ".kythe-extraction-config"

func main() {
	flag.Parse()
	verifyFlags()

	extractor := config.DefaultExtractor{}
	err := extractor.ExtractRepo(*repoURI, *outputPath, *configPath)
	if err != nil {
		log.Fatalf("Failed to extract repo: %v", err)
	}
}
