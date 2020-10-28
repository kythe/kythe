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

package config

import (
	"bufio"
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
	// Either GitRepo or LocalRepo should be set, not both.
	// A remote git repo, e.g. https://github.com/kythe/kythe.
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

	// TODO(#156): these two should be remoevd when docker-in-docker is
	// removed.  Currently necessary for docker-in-docker because regular volume
	// mapping uses the outermost context instead of correctly pushing the
	// first layer docker container's empeheral volume into the innermost
	// layer.
	//
	// When running inside a docker container, these should be set to match
	// their corresponding volumes.
	// When not running inside a docker cotnainer, these can be left unset,
	// in which case an ephemeral temp directory is used for TempRepoDir, and
	// output is written directly into OutputPath from runextractor.go.

	// An optional temporary repo path to copy the input repo to.
	TempRepoDir string

	// An optional temporary directory path to write output before copying to
	// OutputPath.
	TempOutDir string
}

func (r Repo) gitClone(ctx context.Context, tmpDir string) error {
	return GitClone(ctx, r.Git, tmpDir)
}

// GitClone is a convenience wrapper around a commandline git clone call.
func GitClone(ctx context.Context, repo, outputPath string) error {
	// TODO(#154): strongly consider go-git instead of os.exec
	return exec.CommandContext(ctx, "git", "clone", repo, outputPath).Run()
}

func (r Repo) localClone(ctx context.Context, tmpDir string) error {
	gitDir := filepath.Join(r.Local, ".git")
	// TODO(danielmoy): consider extracting all or part of this
	// to a more common place.
	err := copyDir(copyArgs{
		out:     tmpDir,
		in:      r.Local,
		skipDir: gitDir,
	})
	if err != nil {
		return fmt.Errorf("copying repo: %v", err)
	}
	return nil
}

type copyArgs struct {
	out     string
	in      string
	skipDir string
}

// copyDir copies files from in to out. If skipDir is not empty, it skips any
// directories matching that prefix.
func copyDir(args copyArgs) error {
	return filepath.Walk(args.in, func(path string, info os.FileInfo, err error) error {
		if err != nil {
			return err
		}
		if args.in == path {
			// Intentionally do nothing for base dir.
			return nil
		}
		if args.skipDir != "" && strings.HasPrefix(path, args.skipDir) {
			return filepath.SkipDir
		}
		rel, err := filepath.Rel(args.in, path)
		if err != nil {
			return err
		}
		outPath := filepath.Join(args.out, rel)
		if info.Mode().IsRegular() {
			if err := os.MkdirAll(filepath.Dir(outPath), 0755); err != nil {
				return fmt.Errorf("failed to make dir: %v", err)
			}
			inf, err := os.Open(path)
			if err != nil {
				return fmt.Errorf("failed to open input file: %v", err)
			}
			defer inf.Close()
			of, err := os.Create(outPath)
			if err != nil {
				return fmt.Errorf("failed to open output file for copy: %v", err)
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

	outPath := repo.OutputPath
	if repo.TempOutDir != "" {
		outPath = repo.TempOutDir
	}
	err = createImage(extractionConfig, imageSettings{
		RepoDir:   repo.TempRepoDir,
		OutputDir: outPath,
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
	commandArgs := []string{"run", "--rm"}
	// Check and see if we're living inside a docker image.
	// TODO(#156): This is an undesireable smell from docker-in-docker which
	// should be removed when we refactor the inner docker away.
	if id, ok := inDockerImage(ctx); ok {
		// Because we're inside an inner docker image, if we set
		// -v /input/repo:/input, it actually takes /input/repo from the
		// *outermost* docker container, which is not what we want! Instead rely
		// on --volumes-from to copy in all the correct volumes.
		// Similar logic holds for output directory mapping.
		commandArgs = append(commandArgs, "--volumes-from", id)
	} else {
		commandArgs = append(commandArgs, "-v", fmt.Sprintf("%s:%s", repo.OutputPath, outPath), "-v", fmt.Sprintf("%s:%s", repoDir, targetRepoDir))
	}
	commandArgs = append(commandArgs, "-t", imageTag)
	output, err = exec.CommandContext(ctx, "docker", commandArgs...).CombinedOutput()
	if err != nil {
		return fmt.Errorf("extracting repo: %v\nCommand output: %s", err, string(output))
	}
	if repo.TempOutDir != "" {
		// TODO(#156): after removing docker-in-docker we can ax this copy.
		err := copyDir(copyArgs{
			out: repo.OutputPath,
			in:  repo.TempOutDir,
		})
		if err != nil {
			return fmt.Errorf("copying output: %v", err)
		}
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

func mustCleanUpImage(ctx context.Context, tmpImageTag string) {
	cmd := exec.CommandContext(ctx, "docker", "image", "rm", tmpImageTag)
	err := cmd.Run()
	if err != nil {
		log.Printf("Failed to clean up docker image: %v", err)
	}
}

func inDockerImage(ctx context.Context) (string, bool) {
	f, err := os.Open("/proc/self/cgroup")
	if err != nil {
		log.Printf("Failed to open /proc/self/cgroup to check for docker: %v", err)
		return "", false
	}
	defer f.Close()

	scanner := bufio.NewScanner(f)
	for scanner.Scan() {
		// Example line for this file is:
		// 10:cpuset:/docker/5e24d567c9846f5209af443612ca2e53f7291afe6cfb31510121f82f1ac93472
		gr := strings.Split(scanner.Text(), ":")
		if len(gr) == 3 {
			res := strings.Split(gr[2], "/")
			if len(res) == 3 && res[1] == "docker" && len(res[2]) >= 12 {
				return res[2][0:12], true
			}
		}
	}

	if err := scanner.Err(); err != nil {
		log.Printf("Failed reading /rpoc/self/cgroup: %v", err)
	}
	return "", false
}
