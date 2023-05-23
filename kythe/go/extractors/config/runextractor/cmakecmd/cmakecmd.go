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
package cmakecmd // import "kythe.io/kythe/go/extractors/config/runextractor/cmakecmd"

import (
	"context"
	"flag"
	"fmt"
	"io/ioutil"
	"os"
	"os/exec"
	"path/filepath"

	"kythe.io/kythe/go/extractors/config/runextractor/compdb"
	"kythe.io/kythe/go/util/cmdutil"
	"kythe.io/kythe/go/util/flagutil"
	"kythe.io/kythe/go/util/log"

	"github.com/google/subcommands"
)

type cmakeCommand struct {
	cmdutil.Info

	extractor      string
	buildDir       string
	sourceDir      string
	extraCmakeArgs flagutil.StringList
}

// New creates a new subcommand for running cmake extraction.
func New() subcommands.Command {
	return &cmakeCommand{
		Info: cmdutil.NewInfo("cmake", "extract a repo build with CMake", `documight`),
	}
}

// SetFlags implements the subcommands interface and provides command-specific
// flags for cmake extraction.
func (c *cmakeCommand) SetFlags(fs *flag.FlagSet) {
	fs.StringVar(&c.extractor, "extractor", "", "A required path to the extractor binary to use.")
	fs.StringVar(&c.sourceDir, "sourcedir", ".", "A required path to the repository root. Defaults to the current directory.")
	fs.StringVar(&c.buildDir, "builddir", "", "An optional path to the directory in which to build. If empty, defaults to a unique subdirectory of sourcedir.")
	fs.Var(&c.extraCmakeArgs, "extra_cmake_args", "A comma-separated list of extra arguments to pass to CMake.")
}

func (c *cmakeCommand) checkFlags() error {
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

// Execute implements the subcommands interface and runs cmake extraction.
func (c *cmakeCommand) Execute(ctx context.Context, fs *flag.FlagSet, args ...any) subcommands.ExitStatus {
	if err := c.checkFlags(); err != nil {
		return c.Fail("Incorrect flags: %v", err)
	}
	// Since we have to change our working directory, resolve all of our paths early.
	extractor, err := filepath.Abs(c.extractor)
	if err != nil {
		return c.Fail("Unable to resolve path to extractor: %v", err)
	}
	sourceDir, err := filepath.Abs(c.sourceDir)
	if err != nil {
		return c.Fail("Unable to resolve source directory: %v", err)
	}
	var buildDir string
	if c.buildDir == "" {
		// sourceDir is already an absolute directory
		if buildDir, err = ioutil.TempDir(sourceDir, "build-"); err != nil {
			// Unlike below, we need to fail if the "unique" directory exists.
			return c.Fail("Unable to create build directory: %v", err)
		}
		// Only clean up the build directory if it was unspecified.
		defer cleanBuild(buildDir)
	} else {
		buildDir, err = filepath.Abs(c.buildDir)
		if err != nil {
			return c.Fail("Unable to resolve build directory: %v", err)
		}
	}

	// Create the build directory if it doesn't already exist.
	if err := os.MkdirAll(buildDir, 0755); err != nil {
		return c.Fail("Unable to create build directory: %v", err)
	}

	// The Kythe extractor is clang based, so tell cmake to use clang so the right
	// commands are generated.
	cmakeArgs := []string{"-DCMAKE_CXX_COMPILER=clang++", "-DCMAKE_C_COMPILER=clang", "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON", sourceDir}
	cmakeArgs = append(cmakeArgs, c.extraCmakeArgs...)
	if err := runIn(exec.CommandContext(ctx, "cmake", cmakeArgs...), buildDir); err != nil {
		return c.Fail("Error configuring cmake: %v", err)
	}

	if err := runIn(exec.CommandContext(ctx, "cmake", "--build", "."), buildDir); err != nil {
		return c.Fail("Error building repository: %v", err)
	}

	if err := compdb.ExtractCompilations(ctx, extractor, filepath.Join(buildDir, "compile_commands.json"), nil); err != nil {
		return c.Fail("Error extracting repository: %v", err)
	}
	return subcommands.ExitSuccess
}

// cleanBuild removes dir and all files beneath it, logging an error if it fails.
func cleanBuild(dir string) {
	if err := os.RemoveAll(dir); err != nil {
		log.Errorf("unable to remove build directory: %v", err)
	}
}

// runIn changes the cmd working directory to dir and calls Run. The cmd's
// stderr is forwarded to stderr.
func runIn(cmd *exec.Cmd, dir string) error {
	cmd.Dir = dir
	cmd.Stderr = os.Stderr
	log.Infof("Running: %q in %q", cmd, cmd.Dir)
	return cmd.Run()
}
