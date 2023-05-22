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

// Package compdbcmd extracts from a compile_commands.json file.
package compdbcmd // import "kythe.io/kythe/go/extractors/config/runextractor/compdbcmd"

import (
	"context"
	"flag"
	"fmt"
	"os"
	"path/filepath"

	"kythe.io/kythe/go/extractors/config/runextractor/compdb"
	"kythe.io/kythe/go/util/cmdutil"

	"github.com/google/subcommands"
)

type compdbCommand struct {
	cmdutil.Info

	extractor string
	path      string
}

// New creates a new subcommand for running compdb extraction.
func New() subcommands.Command {
	return &compdbCommand{
		Info: cmdutil.NewInfo("compdb", "extract a repo from compile_commands.json",
			`runextractor compdb [OPTIONS] -- [extractor_args...]

Any flags specified in [extractor_args...] will be passed verbatim to the chosen extractor binary.
`),
	}
}

// SetFlags implements the subcommands interface and provides command-specific
// flags for compdb extraction.
func (c *compdbCommand) SetFlags(fs *flag.FlagSet) {
	fs.StringVar(&c.extractor, "extractor", "", "A required path to the extractor binary to use.")
	fs.StringVar(&c.path, "path", "./compile_commands.json", "Path to JSON compilations database.")
}

func (c *compdbCommand) checkFlags() error {
	for _, key := range []string{"KYTHE_CORPUS", "KYTHE_ROOT_DIRECTORY", "KYTHE_OUTPUT_DIRECTORY"} {
		if os.Getenv(key) == "" {
			return fmt.Errorf("required %s not set", key)
		}
	}
	if c.extractor == "" {
		return fmt.Errorf("required -extractor not set")
	}
	return nil
}

// Execute implements the subcommands interface and runs compdb extraction.
func (c *compdbCommand) Execute(ctx context.Context, fs *flag.FlagSet, args ...any) subcommands.ExitStatus {
	if err := c.checkFlags(); err != nil {
		return c.Fail("Incorrect flags: %v", err)
	}
	// Since we have to change our working directory, resolve all of our paths early.
	extractor, err := filepath.Abs(c.extractor)
	if err != nil {
		return c.Fail("Unable to resolve path to extractor: %v", err)
	}
	if err := compdb.ExtractCompilations(ctx, extractor, c.path, &compdb.ExtractOptions{ExtraArguments: fs.Args()}); err != nil {
		return c.Fail("Error extracting repository: %v", err)
	}
	return subcommands.ExitSuccess
}
