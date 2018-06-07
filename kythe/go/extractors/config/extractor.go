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
	"context"
	"fmt"
	"io"
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
	// Clone extracts a copy of the repo to the specified output Directory.
	Clone func(ctx context.Context, outputDir string) error
	// Where to write from an extraction.
	OutputPath string
	// An optional path to a file containing a
	// kythe.proto.ExtractionConfiguration encoded as JSON that details how
	// to perform extraction.
	ConfigPath string
}

// GitCopier returns a function that clones a repository via git command line.
func GitCopier(repoURI string) func(ctx context.Context, outputDir string) error {
	return func(ctx context.Context, outputDir string) error {
		// TODO(danielmoy): strongly consider go-git instead of os.exec
		return exec.CommandContext(ctx, "git", "clone", repoURI, outputDir).Run()
	}
}

// LocalCopier returns a function that copys a local repository.
func LocalCopier(repoPath string) func(ctx context.Context, outputDir string) error {
	return func(ctx context.Context, outputDir string) error {
		gitDir := filepath.Join(repoPath, ".git")
		// TODO(danielmoy): consider extracting all or part of this
		// to a more common place.
		return filepath.Walk(repoPath, func(path string, info os.FileInfo, err error) error {
			if err != nil {
				return err
			}
			if repoPath == path {
				// Intentionally do nothing for base dir.
				return nil
			}
			if filepath.HasPrefix(path, gitDir) {
				// Also skip all that .git stuff.
				return nil
			}
			rel, err := filepath.Rel(repoPath, path)
			if err != nil {
				return err
			}
			outPath := filepath.Join(outputDir, rel)
			if info.IsDir() {
				if _, err := os.Stat(outPath); err != nil {
					if err := os.Mkdir(outPath, 0777); err != nil {
						return err
					}
				}
				return nil
			}
			if !info.Mode().IsRegular() {
				// Notably in here are any links or other odd things.
				return fmt.Errorf("Can't handle the truth")
			}
			inf, err := os.Open(path)
			if err != nil {
				return err
			}
			defer inf.Close()
			of, err := os.Create(outPath)
			if err != nil {
				return err
			}
			if _, err := io.Copy(of, inf); err != nil {
				of.Close()
				return err
			}
			return of.Close()
		})
	}
}

// Extractor is the interface for handling kindex generation on repos.
//
// ExtractRepo takes an input repo, output path to a directory, and optional
// kythe.proto.ExtractionConfiguration file path, and performs kythe extraction
// on the repo, depositing results in the output directory path.
type Extractor func(ctx context.Context, repo Repo) error

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
func ExtractRepo(ctx context.Context, repo Repo) error {
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

	// copy the repo into our temp directory, so we can mutate its
	// build config without affecting the original source.
	if err := repo.Clone(ctx, repoDir); err != nil {
		return fmt.Errorf("copying repo: %v", err)
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
	output, err := exec.CommandContext(ctx, "docker", "build", "-f", extractionDockerFile.Name(), "-t", imageTag, tmpOutDir).CombinedOutput()
	defer mustCleanUpImage(ctx, imageTag)
	if err != nil {
		return fmt.Errorf("building docker image: %v\nCommand output %s", err, string(output))
	}

	// run the extraction
	output, err = exec.CommandContext(ctx, "docker", "run", "--rm", "-v", fmt.Sprintf("%s:%s", repoDir, DefaultRepoVolume), "-v", fmt.Sprintf("%s:%s", repo.OutputPath, DefaultOutputVolume), "-t", imageTag).CombinedOutput()
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

func mustCleanUpImage(ctx context.Context, tmpImageTag string) {
	cmd := exec.CommandContext(ctx, "docker", "image", "rm", tmpImageTag)
	err := cmd.Run()
	if err != nil {
		log.Printf("Failed to clean up docker image: %v", err)
	}
}
