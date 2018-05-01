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
// specified repository using an extraction configuration as defined
// in kythe.proto.ExtractionConfiguration. It makes a local clone
// of the repository. If an extraction config was specified as
// an argument, will utilize said config, otherwise it searches the root
// directory of the repo for a Kythe a config file named: ".kythe-extraction-config".
// Uses the config to generate a customized extraction Docker image. Finally
// it executes the extraction by running the extraction image, the output of
// which is a kindex file as defined here: http://kythe.io/docs/kythe-index-pack.html
// This binary requires both Git and Docker to be on the $PATH during execution.
//
// Usage:
//   extractrepo -repo <repo_uri> -output <output_file_path> -config [config_file_path]
package main

import (
	"flag"
	"fmt"
	"io/ioutil"
	"log"
	"os"
	"os/exec"
	"path/filepath"
	"strings"

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

func verifyRequiredTools() {
	if _, err := exec.LookPath("git"); err != nil {
		log.Fatalf("Unable to find git executable: %v", err)
	}

	if _, err := exec.LookPath("docker"); err != nil {
		log.Fatalf("Unable to find docker executable: %v", err)
	}
}

func mustCleanUpImage(tmpImageTag string) {
	cmd := exec.Command("docker", "image", "rm", tmpImageTag)
	err := cmd.Run()
	if err != nil {
		log.Printf("Failed to clean up docker image: %v", err)
	}
}

// kytheConfigFileName The name of the Kythe extraction config
const kytheExtractionConfigFile = ".kythe-extraction-config"

func main() {
	flag.Parse()
	verifyFlags()
	verifyRequiredTools()

	// create a temporary directory for the repo clone
	repoDir, err := ioutil.TempDir("", "repoDir")
	if err != nil {
		log.Fatalf("creating tmp repo dir: %v", err)
	}
	defer os.RemoveAll(repoDir)

	// create a temporary directory for the extraction output
	tmpOutDir, err := ioutil.TempDir("", "tmpOutDir")
	if err != nil {
		log.Fatalf("creating tmp out dir: %v", err)
	}
	defer os.RemoveAll(tmpOutDir)

	// clone the repo
	cmd := exec.Command("git", "clone", *repoURI, repoDir)
	err = cmd.Run()
	if err != nil {
		log.Fatalf("cloning repo: %v", err)
	}

	// if a config was passed as a arg, use the specified config
	if *configPath == "" {
		// otherwise, search for a Kythe config within the repo
		*configPath = filepath.Join(repoDir, kytheExtractionConfigFile)
	}

	log.Printf("Using configuration file: %q\n", *configPath)
	extractionDockerFile, err := ioutil.TempFile(tmpOutDir, "extractionDockerFile")
	if err != nil {
		log.Fatalf("creating tmp Dockerfile: %v", err)
	}

	// generate an extraction image from the config
	configFile, err := os.Open(*configPath)
	if err != nil {
		log.Fatalf("opening config file: %v", err)
	}

	extractionConfig, err := config.Load(configFile)
	if err != nil {
		log.Fatalf("loading extraction config: %v", err)
	}

	err = config.CreateImage(extractionDockerFile.Name(), extractionConfig)
	if err != nil {
		log.Fatalf("creating extraction image: %v", err)
	}

	// use Docker to build the extraction image
	imageTag := strings.ToLower(filepath.Base(extractionDockerFile.Name()))
	output, err := exec.Command("docker", "build", "-f", extractionDockerFile.Name(), "-t", imageTag, tmpOutDir).CombinedOutput()
	defer mustCleanUpImage(imageTag)
	if err != nil {
		log.Fatalf("building docker image: %v\nCommand output %s", err, string(output))
	}

	// run the extraction
	output, err = exec.Command("docker", "run", "--rm", "-v", fmt.Sprintf("%s:%s", repoDir, config.DefaultRepoVolume), "-v", fmt.Sprintf("%s:%s", *outputPath, config.DefaultOutputVolume), "-t", imageTag).CombinedOutput()
	if err != nil {
		log.Fatalf("extracting repo: %v\nCommand output: %s\n", err, string(output))
	}

}
