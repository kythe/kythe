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

// Package extractor provides a library for running kythe extraction on a single
// repository.
package extractor

import (
	"fmt"
	"io/ioutil"
	"log"
	"os"
	"os/exec"
	"path/filepath"
	"strings"

	"kythe.io/kythe/go/extractors/config"
)

// kytheConfigFileName The name of the Kythe extraction config
const kytheExtractionConfigFile = ".kythe-extraction-config"

// ExtractRepo extracts a given code repository and outputs kindex files.
//
// It makes a local clone of the repository. It optionally uses a passed
// extraction config, otherwise it attempts to find a Kythe config named
// ".kythe-extraction-config".
//
// It builds a one-off customized Docker image for extraction, and then runs it,
// generating kindex files (format defined here:
// http://kythe.io/docs/kythe-index-pack.html).
//
// This function requires both Git and Docker to be in $PATH during execution.
func ExtractRepo(repoURI, outputPath, configPath string) error {
	if err := verifyRequiredTools(); err != nil {
		return fmt.Errorf("ExtractRepo requires git and docker to be in $PATH: %v", err)
	}

	// create a temporary directory for the repo clone
	repoDir, err := ioutil.TempDir("", "repoDir")
	if err != nil {
		return fmt.Errorf("creating tmp repo dir: %v", err)
	}
	defer os.RemoveAll(repoDir)

	// create a temporary directory for the extraction output
	tmpOutDir, err := ioutil.TempDir("", "tmpOutDir")
	if err != nil {
		return fmt.Errorf("creating tmp out dir: %v", err)
	}
	defer os.RemoveAll(tmpOutDir)

	// clone the repo
	cmd := exec.Command("git", "clone", repoURI, repoDir)
	err = cmd.Run()
	if err != nil {
		return fmt.Errorf("cloning repo: %v", err)
	}

	// if a config was passed as a arg, use the specified config
	if configPath == "" {
		// otherwise, use a Kythe config within the repo (if it exists)
		configPath = filepath.Join(repoDir, kytheExtractionConfigFile)
	}

	log.Printf("Using configuration file: %q\n", configPath)
	extractionDockerFile, err := ioutil.TempFile(tmpOutDir, "extractionDockerFile")
	if err != nil {
		return fmt.Errorf("creating tmp Dockerfile: %v", err)
	}

	// generate an extraction image from the config
	configFile, err := os.Open(configPath)
	if err != nil {
		return fmt.Errorf("opening config file: %v", err)
	}

	extractionConfig, err := config.Load(configFile)
	if err != nil {
		return fmt.Errorf("loading extraction config: %v", err)
	}

	err = config.CreateImage(extractionDockerFile.Name(), extractionConfig)
	if err != nil {
		return fmt.Errorf("creating extraction image: %v", err)
	}

	// use Docker to build the extraction image
	imageTag := strings.ToLower(filepath.Base(extractionDockerFile.Name()))
	output, err := exec.Command("docker", "build", "-f", extractionDockerFile.Name(), "-t", imageTag, tmpOutDir).CombinedOutput()
	defer mustCleanUpImage(imageTag)
	if err != nil {
		return fmt.Errorf("building docker image: %v\nCommand output %s", err, string(output))
	}

	// run the extraction
	output, err = exec.Command("docker", "run", "--rm", "-v", fmt.Sprintf("%s:%s", repoDir, config.DefaultRepoVolume), "-v", fmt.Sprintf("%s:%s", outputPath, config.DefaultOutputVolume), "-t", imageTag).CombinedOutput()
	if err != nil {
		return fmt.Errorf("extracting repo: %v\nCommand output: %s", err, string(output))
	}

	return nil
}

func verifyRequiredTools() error {
	if _, err := exec.LookPath("git"); err != nil {
		return err
	}

	if _, err := exec.LookPath("docker"); err != nil {
		return err
	}
	return nil
}

func mustCleanUpImage(tmpImageTag string) {
	cmd := exec.Command("docker", "image", "rm", tmpImageTag)
	err := cmd.Run()
	if err != nil {
		log.Printf("Failed to clean up docker image: %v", err)
	}
}
