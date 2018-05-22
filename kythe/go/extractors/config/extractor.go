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

package config

import (
	"fmt"
	"io/ioutil"
	"log"
	"os"
	"os/exec"
	"path/filepath"
	"strings"
)

// kytheConfigFileName The name of the Kythe extraction config
const kytheExtractionConfigFile = ".kythe-extraction-config"

// Repo is a container of input/output parameters for doing extraction on remote
// repositories.
type Repo struct {
	// The remote location of the repo itself.
	URI string
	// Where to write from an extraction.
	OutputPath string
	// An optional path to a file containing a
	// kythe.proto.ExtractionConfiguration encoded as JSON that details how
	// to perform extraction.
	ConfigPath string
}

// Extractor is the interface for handling kindex generation on repos.
//
// ExtractRepo takes an input repo, output path to a directory, and optional
// kythe.proto.ExtractionConfiguration file path, and performs kythe extraction
// on the repo, depositing results in the output directory path.
type Extractor interface {
	ExtractRepo(repo Repo) error
}

// DefaultExtractor provides an extractor that can perform extraction on
// remote git repos using git commandline tool.
//
// By default, it uses a top level extraction config file
// .kythe-extraction-config, though this can be overridden by passing a specific
// ConfigPath to ExtractRepo.
type DefaultExtractor struct{}

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
func (d DefaultExtractor) ExtractRepo(repo Repo) error {
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
	cmd := exec.Command("git", "clone", repo.URI, repoDir)
	err = cmd.Run()
	if err != nil {
		return fmt.Errorf("cloning repo: %v", err)
	}

	// if a config was passed as a arg, use the specified config
	if repo.ConfigPath == "" {
		// otherwise, use a Kythe config within the repo (if it exists)
		repo.ConfigPath = filepath.Join(repoDir, kytheExtractionConfigFile)
	}

	log.Printf("Using configuration file: %q\n", repo.ConfigPath)
	extractionDockerFile, err := ioutil.TempFile(tmpOutDir, "extractionDockerFile")
	if err != nil {
		return fmt.Errorf("creating tmp Dockerfile: %v", err)
	}

	// generate an extraction image from the config
	configFile, err := os.Open(repo.ConfigPath)
	if err != nil {
		return fmt.Errorf("opening config file: %v", err)
	}

	extractionConfig, err := Load(configFile)
	if err != nil {
		return fmt.Errorf("loading extraction config: %v", err)
	}

	err = CreateImage(extractionDockerFile.Name(), extractionConfig)
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
	output, err = exec.Command("docker", "run", "--rm", "-v", fmt.Sprintf("%s:%s", repoDir, DefaultRepoVolume), "-v", fmt.Sprintf("%s:%s", repo.OutputPath, DefaultOutputVolume), "-t", imageTag).CombinedOutput()
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
