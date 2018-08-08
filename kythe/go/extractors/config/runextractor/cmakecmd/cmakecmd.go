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

// Package cmakecmd extracts a cmake repo.
// TODO(#2861): Actually implement this
package cmakecmd

import (
	"context"

	"kythe.io/kythe/go/util/cmdutil"

	"github.com/google/subcommands"
)

type cmakeCommand struct {
	cmdutil.Info

	buildFile       string
	javacWrapper    string
	pomPreProcessor string
}

// New creates a new subcommand for running cmake extraction.
func New() subcommands.Command {
	return &cmakeCommand{
		Info: cmdutil.NewInfo("cmake", "extract a repo built with cmake",
			`docstring TBD`),
	}
}

// SetFlags implements the subcommands interface and provides command-specific
// flags for cmake extraction.
func (c *cmakeCommand) SetFlags(fs *flag.FlagSet) {
	// fs.StringVar(&c.someFlag, "some_flag", "default-value", "Flag Description")
}

// Execute implements the subcommands interface and runs cmake extraction.
func (c *cmakeCommand) Execute(ctx context.Context, fs *flag.FlagSet, args ...interface{}) subcommands.ExitStatus {
	// if err := exec.Command("run", "some", "thing") {
	//   c.Fail("some error %v", err)
	return subcommands.ExitSuccess
}
