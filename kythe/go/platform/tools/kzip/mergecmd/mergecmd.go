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
	"context"
	"flag"
	"fmt"
	"io"
	"log"
	"os"
	"path/filepath"

	"kythe.io/kythe/go/platform/kzip"
	"kythe.io/kythe/go/platform/vfs"
	"kythe.io/kythe/go/util/cmdutil"

	"bitbucket.org/creachadair/stringset"

	"github.com/google/subcommands"
)

type mergeCommand struct {
	cmdutil.Info

	output   string
	append   bool
	encoding kzip.Encoding
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
	fs.BoolVar(&c.append, "append", false, "Whether to additionally merge the contents of the existing output file, if it exists")
	fs.Var(&c.encoding, "encoding", "Encoding to use on output, one of JSON, PROTO, or BOTH")
}

// Execute implements the subcommands interface and merges the provided files.
func (c *mergeCommand) Execute(ctx context.Context, fs *flag.FlagSet, _ ...interface{}) subcommands.ExitStatus {
	if c.output == "" {
		return c.Fail("required --output path missing")
	}
	opts := &kzip.WriterOptions{
		Encoding: c.encoding,
	}
	dir, file := filepath.Split(c.output)
	if dir == "" {
		dir = "."
	}
	tmpOut, err := vfs.CreateTempFile(ctx, dir, file)
	tmpName := tmpOut.Name()
	defer func() {
		if tmpOut != nil {
			tmpOut.Close()
			vfs.Remove(ctx, tmpName)
		}
	}()
	if err != nil {
		return c.Fail("Error creating temp output: %v", err)
	}
	archives := fs.Args()
	if c.append {
		orig, err := vfs.Open(ctx, c.output)
		if err == nil {
			archives = append([]string{c.output}, archives...)
			if err := orig.Close(); err != nil {
				return c.Fail("Error closing original: %v", err)
			}
		}
	}
	if err := mergeArchives(ctx, tmpOut, opts, archives); err != nil {
		return c.Fail("Error merging archives: %v", err)
	}
	if err := vfs.Rename(ctx, tmpName, c.output); err != nil {
		return c.Fail("Error renaming tmp to output: %v", err)
	}
	return subcommands.ExitSuccess
}

func mergeArchives(ctx context.Context, out io.WriteCloser, o *kzip.WriterOptions, archives []string) error {
	wr, err := kzip.NewWriteCloserWithOptions(out, o)
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
