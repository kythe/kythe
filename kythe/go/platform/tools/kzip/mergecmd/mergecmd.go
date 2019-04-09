/*
 * Copyright 2019 The Kythe Authors. All rights reserved.
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

// Package mergecmd provides the kzip command for merging archives.
package mergecmd

import (
	"bufio"
	"context"
	"flag"
	"fmt"
	"log"
	"os"

	"kythe.io/kythe/go/platform/kzip"
	"kythe.io/kythe/go/platform/vfs"
	"kythe.io/kythe/go/util/cmdutil"

	"bitbucket.org/creachadair/stringset"

	"github.com/google/subcommands"
)

type mergeCommand struct {
	cmdutil.Info

	output     string
	inputsFile string
}

// New creates a new subcommand for merging kzip files.
func New() subcommands.Command {
	return &mergeCommand{
		Info: cmdutil.NewInfo("merge", "merge kzip files", "--output path kzip-file*"),
	}
}

// SetFlags implements the subcommands interface and provides command-specific flags
// for merging kzip files.
func (c *mergeCommand) SetFlags(fs *flag.FlagSet) {
	fs.StringVar(&c.output, "output", "", "Path to output kzip file")
	fs.StringVar(&c.inputsFile, "inputs_list_file", "", "A file from which to read input kzip file names")
}

// Execute implements the subcommands interface and merges the provided files.
func (c *mergeCommand) Execute(ctx context.Context, fs *flag.FlagSet, _ ...interface{}) subcommands.ExitStatus {
	if c.output == "" {
		return c.Fail("required --output path missing")
	}
	archives := fs.Args()
	if c.inputsFile != "" {
		if len(archives) != 0 {
			return c.Fail("--inputs_list_file cannot be combined with command line inputs")
		}
		scanner := bufio.NewScanner(os.Stdin)
		for scanner.Scan() {
			archives = append(archives, scanner.Text())
		}
	}
	if err := mergeArchives(ctx, c.output, archives); err != nil {
		return c.Fail("Error merging archives: ", err)
	}
	return subcommands.ExitSuccess
}

func mergeArchives(ctx context.Context, output string, archives []string) error {
	out, err := vfs.Create(ctx, output)
	if err != nil {
		return fmt.Errorf("error creating output: %v", err)
	}
	wr, err := kzip.NewWriteCloser(out)
	if err != nil {
		out.Close()
		return fmt.Errorf("error creating writer: %v", err)
	}

	filesAdded := stringset.New()
	for _, path := range archives {
		if err := mergeInto(wr, path, filesAdded); err != nil {
			wr.Close()
			return err
		}
	}

	if err := wr.Close(); err != nil {
		return fmt.Errorf("error closing writer: %v", err)
	}
	return nil
}

func mergeInto(wr *kzip.Writer, path string, filesAdded stringset.Set) error {
	f, err := os.Open(path)
	if err != nil {
		return fmt.Errorf("error opening archive: %v", err)
	}
	defer f.Close()

	stat, err := f.Stat()
	if err != nil {
		return err
	}
	size := stat.Size()
	if size == 0 {
		log.Printf("Skipping empty .kzip: %s", path)
		return nil
	}

	rd, err := kzip.NewReader(f, size)
	if err != nil {
		return fmt.Errorf("error creating reader: %v", err)
	}

	return rd.Scan(func(u *kzip.Unit) error {
		for _, ri := range u.Proto.RequiredInput {
			if filesAdded.Add(ri.Info.Digest) {
				r, err := rd.Open(ri.Info.Digest)
				if err != nil {
					return fmt.Errorf("error opening file: %v", err)
				}
				if _, err := wr.AddFile(r); err != nil {
					r.Close()
					return fmt.Errorf("error adding file: %v", err)
				} else if err := r.Close(); err != nil {
					return fmt.Errorf("error closing file: %v", err)
				}
			}
		}
		// TODO(schroederc): duplicate compilations with different revisions
		_, err = wr.AddUnit(u.Proto, u.Index)
		return err
	})
}
