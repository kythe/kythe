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

// Package cmakecmd extracts from a CMake-based repository.
package cmakecmd

import (
	"context"
	"encoding/json"
	"flag"
	"fmt"
	"io/ioutil"
	"log"
	"os"
	"os/exec"
	"path/filepath"
	"strings"

	"kythe.io/kythe/go/util/cmdutil"

	"bitbucket.org/creachadair/shell"
	"github.com/google/subcommands"
	"github.com/pborman/uuid"
	"golang.org/x/sync/errgroup"
	"golang.org/x/sync/semaphore"
)

type cmakeCommand struct {
	cmdutil.Info

	extractor string
	buildDir  string
	sourceDir string
}

type compileCommand struct {
	Command   string
	Directory string
}

func New() subcommands.Command {
	return &cmakeCommand{
		Info: cmdutil.NewInfo("cmake", "extract a repo build with CMake", `documight`),
	}
}

func (c *cmakeCommand) SetFlags(fs *flag.FlagSet) {
	fs.StringVar(&c.extractor, "extractor", "", "A required path to the extractor binary to use.")
	fs.StringVar(&c.sourceDir, "sourcedir", ".", "A required path to the repository root. Defaults to the current directory.")
	fs.StringVar(&c.buildDir, "builddir", "", "An optional path to the directory in which to build. If empty, defaults to a unique subdirectory of sourcedir.")
}

func (c *cmakeCommand) verifyFlags() error {
	for _, key := range []string{"KYTHE_CORPUS", "KYTHE_ROOT_DIRECTORY", "KYTHE_OUTPUT_DIRECTORY"} {
		if os.Getenv(key) == "" {
			return fmt.Errorf("required %s not set", key)
		}
	}
	if c.extractor == "" {
		return fmt.Errorf("required -extractor not set")
	}
	if c.sourceDir == "" {
		return fmt.Errorf("required -sourcedir not set")
	}
	return nil
}

func (c *cmakeCommand) Execute(ctx context.Context, fs *flag.FlagSet, args ...interface{}) subcommands.ExitStatus {
	if err := c.verifyFlags(); err != nil {
		return c.Fail("incorrect flags: %v", err)
	}
	// Since we have to change our working directory, resolve all of our paths early.
	extractor, err := filepath.Abs(c.extractor)
	if err != nil {
		return c.Fail("unable to resolve path to extractor: %v", err)
	}
	sourceDir, err := filepath.Abs(c.sourceDir)
	if err != nil {
		return c.Fail("unable to resolve source directory: %v", err)
	}
	var buildDir string
	if c.buildDir == "" {
		// sourceDir is already an absolute directory
		buildDir = filepath.Join(sourceDir, "build-"+uuid.New())
		if err := os.Mkdir(buildDir, 0755); err != nil {
			// Unlike below, we need to fail if the "unique" directory exists.
			return c.Fail("unable to create build directory: %v", err)
		}
		// Only clean up the build directory if it was unspecified.
		defer cleanBuild(buildDir)
	} else {
		buildDir, err = filepath.Abs(c.buildDir)
		if err != nil {
			return c.Fail("unable to resolve build directory: %v", err)
		}
	}

	// Create the build directory if it doesn't already exist.
	if err := os.MkdirAll(buildDir, 0755); err != nil {
		return c.Fail("unable to create build directory: %v", err)
	}

	if err := runIn(exec.CommandContext(ctx, "cmake", "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON", sourceDir), buildDir); err != nil {
		return c.Fail("error configuring cmake: %v", err)
	}

	if err := runIn(exec.CommandContext(ctx, "cmake", "--build", "."), buildDir); err != nil {
		return c.Fail("error building repository: %v", err)
	}

	// TODO(shahms): Move compile_commands.json handling to common library.
	commands, err := readCommands(filepath.Join(buildDir, "compile_commands.json"))
	if err != nil {
		return c.Fail("unable to read compile_commands.json: %v", err)
	}
	exEnv, err := extractorEnv(buildDir)
	if err != nil {
		return c.Fail("unable to set up environment: %v", err)
	}
	sem := semaphore.NewWeighted(128) // Limit concurrency.
	extraction, ctx := errgroup.WithContext(ctx)
	for _, entry := range commands {
		entry := entry
		extraction.Go(func() error {
			if err := sem.Acquire(ctx, 1); err != nil {
				return err
			}
			defer sem.Release(1)

			cmd := exec.CommandContext(ctx, extractor, "--with_executable")
			args, ok := shell.Split(entry.Command)
			if !ok {
				return fmt.Errorf("unable to split command line")
			}
			cmd.Args = append(cmd.Args, args...)
			cmd.Dir, err = filepath.Abs(entry.Directory)
			if err != nil {
				return fmt.Errorf("unable to resolve cmake directory: %v", err)
			}
			cmd.Env = exEnv
			if _, err := cmd.Output(); err != nil {
				return fmt.Errorf("error running extractor: %v (%s)", err, err.(*exec.ExitError).Stderr)
			}
			return nil
		})
	}
	if err := extraction.Wait(); err != nil {
		return c.Fail("error extracting repository: %v", err)
	}
	return subcommands.ExitSuccess
}

// cleanBuild removes dir and all files beneath it, logging an error if it fails.
func cleanBuild(dir string) {
	if err := os.RemoveAll(dir); err != nil {
		log.Printf("unable to remove build directory: %v", err)
	}
}

// readCommands reads the JSON file at path into a slice of compileCommands.
func readCommands(path string) ([]compileCommand, error) {
	data, err := ioutil.ReadFile(path)
	if err != nil {
		return nil, err
	}
	var commands []compileCommand
	if err := json.Unmarshal(data, &commands); err != nil {
		return nil, err
	}
	return commands, nil

}

// cmakeEnvironment copies the existing environment and modifies it to be suitable for an extractor invocation.
func extractorEnv(buildDir string) ([]string, error) {
	var env []string
	for _, value := range os.Environ() {
		// Until kzip support comes along, we only support writing to a single directory so strip these options.
		if strings.HasPrefix(value, "KYTHE_INDEX_PACK=") || strings.HasPrefix(value, "KYTHE_OUTPUT_FILE=") {
			continue
		}

		// Remap KYTHE_OUTPUT_DIRECTORY to be an absolute path.
		if suffix := strings.TrimPrefix(value, "KYTHE_OUTPUT_DIRECTORY="); suffix != value {
			output, err := filepath.Abs(suffix)
			if err != nil {
				return nil, err
			}
			env = append(env, "KYTHE_OUTPUT_DIRECTORY="+output)
		} else {
			// Otherwise, preserve the environment unchanged.
			env = append(env, value)
		}

	}
	return env, nil
}

// runIn changes the cmd working directory to dir and call Run.
func runIn(cmd *exec.Cmd, dir string) error {
	cmd.Dir = dir
	return cmd.Run()
}
