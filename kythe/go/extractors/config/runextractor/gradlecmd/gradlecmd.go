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

// Package gradlecmd extracts a gradle repo.
package gradlecmd

import (
	"context"
	"flag"
	"fmt"
	"os/exec"

	"kythe.io/kythe/go/extractors/config/runextractor/backup"
	"kythe.io/kythe/go/util/cmdutil"

	"github.com/google/subcommands"
)

type gradleCommand struct {
	cmdutil.Info

	buildFile    string
	javacWrapper string
}

func New() subcommands.Command {
	return &gradleCommand{
		Info: cmdutil.NewInfo("gradle", "extract a repo built with gradle",
			`docstring TBD`),
	}
}

func (g *gradleCommand) SetFlags(fs *flag.FlagSet) {
	fs.StringVar(&g.javacWrapper, "javac_wrapper", "", "A required executable that wraps javac for Kythe extraction.")
	fs.StringVar(&g.buildFile, "build_file", "gradle.build", "The config file for a gradle repo, defaults to 'gradle.build'")
}

func (g gradleCommand) verifyFlags() error {
	if g.buildFile == "" {
		return fmt.Errorf("gradle build file (e.g. 'gradle.build') not set")
	}
	if g.javacWrapper == "" {
		return fmt.Errorf("required -javac_wrapper not set")
	}
	return nil
}

func (g *gradleCommand) Execute(ctx context.Context, fs *flag.FlagSet, args ...interface{}) subcommands.ExitStatus {
	if err := g.verifyFlags(); err != nil {
		return g.Fail("incorrect flags: %v", err)
	}
	tf, err := backup.New(g.buildFile)
	if err != nil {
		return g.Fail("error backing up %s: %v", g.buildFile, err)
	}
	defer tf.Release()
	if err := PreProcessGradleBuild(g.buildFile); err != nil {
		return g.Fail("error modifying %s: %v", g.buildFile, err)
	}
	if err := exec.Command("gradle", "build").Run(); err != nil {
		return g.Fail("error executing gradle build: %v", err)
	}
	if err := tf.Restore(); err != nil {
		return g.Fail("error restoring %s from %s: %v", g.buildFile, tf, err)
	}
	return subcommands.ExitSuccess
}
