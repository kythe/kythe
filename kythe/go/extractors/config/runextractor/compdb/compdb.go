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

// Package compdb contains functionality necessary for extracting from a
// compile_commands.json file.
package compdb // import "kythe.io/kythe/go/extractors/config/runextractor/compdb"

import (
	"context"
	"encoding/json"
	"errors"
	"fmt"
	"io/ioutil"
	"os"
	"os/exec"
	"path/filepath"
	"strings"
	"sync"
	"sync/atomic"

	"kythe.io/kythe/go/util/log"

	"bitbucket.org/creachadair/shell"
	"golang.org/x/sync/semaphore"
)

// A compileCommand holds the decoded arguments of a LLVM compilation database
// JSON command spec.
type compileCommand struct {
	Arguments []string
	Command   string
	Directory string
}

func (cc *compileCommand) asCommand() string {
	if len(cc.Arguments) > 0 {
		return shell.Join(cc.Arguments)
	}
	return cc.Command
}

func (cc *compileCommand) asArguments() ([]string, bool) {
	if len(cc.Arguments) > 0 {
		return cc.Arguments, true
	}
	return shell.Split(cc.Command)
}

// ExtractOptions holds additional options related to compilation DB extraction.
type ExtractOptions struct {
	ExtraArguments []string // additional arguments to pass to the extractor
}

// ExtractCompilations runs the specified extractor over each compilation record
// found in the compile_commands.json file at path.
func ExtractCompilations(ctx context.Context, extractor, path string, opts *ExtractOptions) error {
	commands, err := readCommands(path)
	if err != nil {
		return err
	}
	env, err := extractorEnv()
	if err != nil {
		return err
	}

	var failCount uint64
	sem := semaphore.NewWeighted(128) // Limit concurrency.
	var wg sync.WaitGroup
	wg.Add(len(commands))
	for _, entry := range commands {
		go func(entry compileCommand) {
			defer wg.Done()
			if err := sem.Acquire(ctx, 1); err != nil {
				atomic.AddUint64(&failCount, 1)
				log.Error(err)
				return
			}
			defer sem.Release(1)

			if err := extractOne(ctx, extractor, entry, env, opts); err != nil {
				// Log error, but continue processing other compilations.
				atomic.AddUint64(&failCount, 1)
				log.Errorf("extracting compilation with command '%s': %v", entry.asCommand(), err)
			}
		}(entry)
	}
	wg.Wait()

	if failCount != 0 {
		return fmt.Errorf("Failed to extract %d compilations", failCount)
	}

	return nil
}

// extractOne invokes the extractor for the given compileCommand.
func extractOne(ctx context.Context, extractor string, cc compileCommand, env []string, opts *ExtractOptions) error {
	cmd := exec.CommandContext(ctx, extractor, "--with_executable")
	args, ok := cc.asArguments()
	if !ok {
		return fmt.Errorf("unable to split command line")
	}
	// Wire through any additional arguments from the command line.
	args = append(args, opts.extraArguments()...)
	cmd.Args = append(cmd.Args, args...)
	var err error
	cmd.Dir, err = filepath.Abs(cc.Directory)
	if err != nil {
		return fmt.Errorf("unable to resolve cmake directory: %v", err)
	}
	cmd.Env = env
	if _, err := cmd.Output(); err != nil {
		if exit, ok := err.(*exec.ExitError); ok {
			return fmt.Errorf("error running extractor: %v (%s)", exit, exit.Stderr)
		}
		return fmt.Errorf("error running extractor: %v", err)
	}
	return nil
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

// extractorEnv copies the existing environment and modifies it to be suitable for an extractor invocation.
func extractorEnv() ([]string, error) {
	var env []string
	outputFound := false
	for _, value := range os.Environ() {
		parts := strings.SplitN(value, "=", 2)
		// Until kzip support comes along, we only support writing to a single directory so strip these options.
		if parts[0] == "KYTHE_INDEX_PACK" || parts[0] == "KYTHE_OUTPUT_FILE" {
			continue
		} else if parts[0] == "KYTHE_OUTPUT_DIRECTORY" {
			// Remap KYTHE_OUTPUT_DIRECTORY to be an absolute path.
			output, err := filepath.Abs(parts[1])
			if err != nil {
				return nil, err
			}
			outputFound = true
			env = append(env, "KYTHE_OUTPUT_DIRECTORY="+output)
		} else {
			// Otherwise, preserve the environment unchanged.
			env = append(env, value)
		}

	}
	if !outputFound {
		return nil, errors.New("missing mandatory environment variable: KYTHE_OUTPUT_DIRECTORY")
	}
	return env, nil
}

// extraArguments returns a slice of additional arguments to provide to the extractor.
func (o *ExtractOptions) extraArguments() []string {
	if o != nil && len(o.ExtraArguments) > 0 {
		return o.ExtraArguments
	}
	return nil
}
