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

	"kythe.io/kythe/go/extractors/config/default/mvn"

	ecpb "kythe.io/kythe/proto/extraction_config_go_proto"
)

// kytheConfigFileName The name of the Kythe extraction config
const kytheExtractionConfigFile = ".kythe-extraction-config"

// Repo is a container of input/output parameters for doing extraction on remote
// repositories.
type Repo struct {
	// Either GitRepo or LocalRepo should be set, not both.
	// A remote git repo, e.g. https://github.com/google/kythe.
	Git string

	// A local copy of a repository.
	Local string

	// Where to write from an extraction.
	OutputPath string

	// An optional path to a file containing a
	// kythe.proto.ExtractionConfiguration encoded as JSON that details how
	// to perform extraction. If this is unset, the extractor will first try
	// to find a config defined in the repo, or finally use a hard coded
	// default config.
	ConfigPath string

	// An optional path that dictates where temporary repo copies should go.
	// TODO(#156): this should be removed as soon as we refactor code to no
	// longer use docker-in-docker.
	TempRepoDir string
}

func (r Repo) gitClone(ctx context.Context, tmpDir string) error {
	// TODO(#154): strongly consider go-git instead of os.exec
	return GitClone(ctx, r.Git, tmpDir)
}

// GitClone is a convenience wrapper around a commandline git clone call.
func GitClone(ctx context.Context, repo, outputPath string) error {
	return exec.CommandContext(ctx, "git", "clone", repo, outputPath).Run()
}

func (r Repo) localClone(ctx context.Context, tmpDir string) error {
	gitDir := filepath.Join(r.Local, ".git")
	// TODO(danielmoy): consider extracting all or part of this
	// to a more common place.
	return filepath.Walk(r.Local, func(path string, info os.FileInfo, err error) error {
		if err != nil {
			return err
		}
		if r.Local == path {
			// Intentionally do nothing for base dir.
			return nil
		}
		if filepath.HasPrefix(path, gitDir) {
			return filepath.SkipDir
		}
		rel, err := filepath.Rel(r.Local, path)
		if err != nil {
			return err
		}
		outPath := filepath.Join(tmpDir, rel)
		if info.Mode().IsRegular() {
			if err := os.MkdirAll(filepath.Dir(outPath), 0755); err != nil {
				return fmt.Errorf("failed to make dir: %v", err)
			}
			inf, err := os.Open(path)
			if err != nil {
				return fmt.Errorf("failed to open input file from repo: %v", err)
			}
			defer inf.Close()
			of, err := os.Create(outPath)
			if err != nil {
				return fmt.Errorf("failed to open output file for repo copy: %v", err)
			}
			if _, err := io.Copy(of, inf); err != nil {
				of.Close()
				return fmt.Errorf("failed to copy repo file: %v", err)
			}
			return of.Close()
		} else if !info.IsDir() {
			// Notably in here are any links or other odd things.
			log.Printf("Unsupported file %s with mode %s\n", path, info.Mode())
		}
		return nil
	})
}

// Extractor is the interface for handling kindex generation on repos.
//
// ExtractRepo takes an input repo, output path to a directory, and optional
// kythe.proto.ExtractionConfiguration file path, and performs kythe extraction
// on the repo, depositing results in the output directory path.
type Extractor func(ctx context.Context, repo Repo) error

// Clone takes either the Git or Local repo and makes a copy of it.
func (r Repo) Clone(ctx context.Context, tmpDir string) error {
	if r.Git != "" {
		return r.gitClone(ctx, tmpDir)
	}
	return r.localClone(ctx, tmpDir)
}

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
	if (repo.Git != "" && repo.Local != "") || (repo.Git == "" && repo.Local == "") {
		return fmt.Errorf("ExtractRepo requries either Git or Local repo, but not both")
	}
	if err := verifyRequiredTools(repo); err != nil {
		return fmt.Errorf("ExtractRepo requires git and docker to be in $PATH: %v", err)
	}

	repoDir := repo.TempRepoDir
	if repoDir == "" {
		// create a temporary directory for the repo clone
		var err error
		repoDir, err = ioutil.TempDir("", "repoDir")
		if err != nil {
			return fmt.Errorf("creating tmp repo dir: %v", err)
		}
		// TODO(#156): intentionally not cleaning up repo dir when passed
		// in might be a mistake.  Long term this doesn't matter after
		// refactor, but would be good to confirm no disk leak here.
		// Because it should be inside container I think this doesn't
		// matter.
		defer os.RemoveAll(repoDir)
	}

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

	log.Printf("Using configuration file: %q", repo.ConfigPath)
	extractionDockerFile, err := ioutil.TempFile(tmpOutDir, "extractionDockerFile")
	if err != nil {
		return fmt.Errorf("creating tmp Dockerfile: %v", err)
	}

	// generate an extraction image from the config
	extractionConfig, err := findConfig(repo.ConfigPath, repoDir)
	if err != nil {
		return fmt.Errorf("reading config file: %v", err)
	}

	err = createImage(extractionConfig, imageSettings{
		RepoDir:   repo.TempRepoDir,
		OutputDir: repo.OutputPath,
	}, extractionDockerFile.Name())
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

	targetRepoDir := repo.TempRepoDir
	if targetRepoDir == "" {
		targetRepoDir = DefaultRepoVolume
	}
	// run the extraction
	commandArgs := []string{"run", "--rm", "-v", fmt.Sprintf("%s:%s", repoDir, targetRepoDir), "-v", fmt.Sprintf("%s:%s", repo.OutputPath, repo.OutputPath), "-t", imageTag}
	// Check and see if we're living inside a docker image.
	// TODO(#156): This is an undesireable smell from docker-in-docker which
	// should be removed when we refactor the inner docker away.
	if id, ok := inDockerImage(ctx); ok {
		commandArgs = append(commandArgs, "--volumes-from", id)
	}
	output, err = exec.CommandContext(ctx, "docker", commandArgs...).CombinedOutput()
	if err != nil {
		return fmt.Errorf("extracting repo: %v\nCommand output: %s", err, string(output))
	}

	return nil
}

func verifyRequiredTools(repo Repo) error {
	if repo.Git != "" {
		if _, err := exec.LookPath("git"); err != nil {
			return err
		}
	}

	if _, err := exec.LookPath("docker"); err != nil {
		return err
	}
	return nil
}

func findConfig(configPath, repoDir string) (*ecpb.ExtractionConfiguration, error) {
	// if a config was passed in, use the specified config, otherwise go
	// hunt for one in the repository.
	if configPath == "" {
		// otherwise, use a Kythe config within the repo (if it exists)
		configPath = filepath.Join(repoDir, kytheExtractionConfigFile)
	}

	f, err := os.Open(configPath)
	if os.IsNotExist(err) {
		// TODO(danielmoy): This needs to be configurable by builder, language, etc.
		return load(mvn.DefaultConfig())
	} else if err != nil {
		return nil, fmt.Errorf("opening config file: %v", err)
	}

	defer f.Close()
	return load(f)
}

func mustCleanUpImage(ctx context.Context, tmpImageTag string) {
	cmd := exec.CommandContext(ctx, "docker", "image", "rm", tmpImageTag)
	err := cmd.Run()
	if err != nil {
		log.Printf("Failed to clean up docker image: %v", err)
	}
}

func inDockerImage(ctx context.Context) (string, bool) {
	output, err := exec.CommandContext(ctx, "cat", "/proc/self/cgroup", "|", "grep", "'docker/'", "|", "tail", "-1", "|", "cut", "-d/", "-f", "3", "|", "cut", "-c", "1-12").CombinedOutput()
	return string(output), err == nil && len(output) != 0
}
