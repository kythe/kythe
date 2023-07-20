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

// Package filtercmd provides the kzip command for filtering archives.
package filtercmd // import "kythe.io/kythe/go/platform/tools/kzip/filtercmd"

import (
	"context"
	"flag"
	"fmt"
	"io"
	"path/filepath"

	"kythe.io/kythe/go/platform/kzip"
	"kythe.io/kythe/go/platform/tools/kzip/flags"
	"kythe.io/kythe/go/platform/vfs"
	"kythe.io/kythe/go/util/cmdutil"
	"kythe.io/kythe/go/util/flagutil"
	"kythe.io/kythe/go/util/log"

	"bitbucket.org/creachadair/stringset"
	"github.com/google/subcommands"
)

type filterCommand struct {
	cmdutil.Info

	input     string
	output    string
	encoding  flags.EncodingFlag
	languages flagutil.StringSet
}

// New creates a new subcommand for merging kzip files.
func New() subcommands.Command {
	return &filterCommand{
		Info:     cmdutil.NewInfo("filter", "filter units from kzip file", "--input path --output path unit-hash*"),
		encoding: flags.EncodingFlag{Encoding: kzip.EncodingJSON},
	}
}

// SetFlags implements the subcommands interface and provides command-specific flags
// for merging kzip files.
func (c *filterCommand) SetFlags(fs *flag.FlagSet) {
	fs.StringVar(&c.output, "output", "", "Path to output kzip file")
	fs.StringVar(&c.input, "input", "", "Path to input kzip file")
	fs.Var(&c.encoding, "encoding", "Encoding to use on output, one of JSON, PROTO, or ALL")
	fs.Var(&c.languages, "languages", "When specified retains only compilation units for given language.")
}

// Execute implements the subcommands interface and filters the input file.
func (c *filterCommand) Execute(ctx context.Context, fs *flag.FlagSet, _ ...any) subcommands.ExitStatus {
	if c.output == "" {
		return c.Fail("Required --output path missing")
	}
	if c.input == "" {
		return c.Fail("Required --input path missing")
	}
	opt := kzip.WithEncoding(c.encoding.Encoding)
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
	units := stringset.New(fs.Args()...)
	langs := stringset.Set(c.languages)
	var filter func(*kzip.Unit) bool
	if !units.Empty() && !langs.Empty() {
		return c.Fail("Error filtering can be done only by either digests or languages but not both simultaneously.")
	} else if !units.Empty() {
		filter = func(u *kzip.Unit) bool {
			return units.Contains(u.Digest)
		}
	} else if !langs.Empty() {
		filter = func(u *kzip.Unit) bool {
			return langs.Contains(u.Proto.VName.Language)
		}
	}
	if err := filterArchive(ctx, tmpOut, c.input, filter, opt); err != nil {
		return c.Fail("Error filtering archives: %v", err)
	}
	if err := vfs.Rename(ctx, tmpName, c.output); err != nil {
		return c.Fail("Error renaming tmp to output: %v", err)
	}
	return subcommands.ExitSuccess
}

func filterArchive(ctx context.Context, out io.WriteCloser, input string, filter func(*kzip.Unit) bool, opts ...kzip.WriterOption) error {
	filesAdded := stringset.New()

	f, err := vfs.Open(ctx, input)
	if err != nil {
		return fmt.Errorf("error opening archive: %v", err)
	}
	defer f.Close()

	stat, err := vfs.Stat(ctx, input)
	if err != nil {
		return err
	}
	size := stat.Size()
	if size == 0 {
		log.Infof("Skipping empty .kzip: %s", input)
		return nil
	}

	rd, err := kzip.NewReader(f, size)
	if err != nil {
		return fmt.Errorf("error creating reader: %v", err)
	}

	wr, err := kzip.NewWriteCloser(out, opts...)
	if err != nil {
		return fmt.Errorf("error creating writer: %v", err)
	}

	// scan the input, and for matching units, copy into output
	err = rd.Scan(func(u *kzip.Unit) error {
		if !filter(u) {
			// non-matching unit, do not copy
			return nil
		}
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
		_, err := wr.AddUnit(u.Proto, u.Index)
		return err
	})
	if err == nil {
		return wr.Close()
	}
	wr.Close()
	return err
}
