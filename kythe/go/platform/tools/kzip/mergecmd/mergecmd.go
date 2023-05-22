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
package mergecmd // import "kythe.io/kythe/go/platform/tools/kzip/mergecmd"

import (
	"context"
	"flag"
	"fmt"
	"io"
	"os"
	"path/filepath"
	"strings"

	"kythe.io/kythe/go/platform/kzip"
	"kythe.io/kythe/go/platform/tools/kzip/flags"
	"kythe.io/kythe/go/platform/vfs"
	"kythe.io/kythe/go/util/cmdutil"
	"kythe.io/kythe/go/util/log"

	"bitbucket.org/creachadair/stringset"
	"github.com/google/subcommands"
)

type mergeCommand struct {
	cmdutil.Info

	output             string
	append             bool
	encoding           flags.EncodingFlag
	recursive          bool
	ignoreDuplicateCUs bool
	rules              vnameRules

	unitsBeforeFiles bool
}

// New creates a new subcommand for merging kzip files.
func New() subcommands.Command {
	return &mergeCommand{
		Info:     cmdutil.NewInfo("merge", "merge kzip files", "--output path kzip-file*"),
		encoding: flags.EncodingFlag{Encoding: kzip.DefaultEncoding()},
	}
}

// SetFlags implements the subcommands interface and provides command-specific flags
// for merging kzip files.
func (c *mergeCommand) SetFlags(fs *flag.FlagSet) {
	fs.StringVar(&c.output, "output", "", "Path to output kzip file")
	fs.BoolVar(&c.append, "append", false, "Whether to additionally merge the contents of the existing output file, if it exists")
	fs.Var(&c.encoding, "encoding", "Encoding to use on output, one of JSON, PROTO, or ALL")
	fs.BoolVar(&c.recursive, "recursive", false, "Recurisvely merge .kzip files from directories")
	fs.Var(&c.rules, "rules", "VName rules to apply while merging (optional)")
	fs.BoolVar(&c.ignoreDuplicateCUs, "ignore_duplicate_cus", false, "Do not fail if we try to add the same CU twice")
	fs.BoolVar(&c.unitsBeforeFiles, "experimental_write_units_first", false, "When writing the kzip file, puts CU entries before files")
}

// Execute implements the subcommands interface and merges the provided files.
func (c *mergeCommand) Execute(ctx context.Context, fs *flag.FlagSet, _ ...any) subcommands.ExitStatus {
	if c.output == "" {
		return c.Fail("Required --output path missing")
	}
	opt := kzip.WithEncoding(c.encoding.Encoding)
	dir, file := filepath.Split(c.output)
	if dir == "" {
		dir = "."
	}
	tmpOut, err := vfs.CreateTempFile(ctx, dir, file)
	if err != nil {
		return c.Fail("Error creating temp output: %v", err)
	}
	tmpName := tmpOut.Name()
	defer func() {
		if tmpOut != nil {
			tmpOut.Close()
			vfs.Remove(ctx, tmpName)
		}
	}()
	archives := fs.Args()
	if c.recursive {
		archives, err = recurseDirectories(ctx, archives)
		if err != nil {
			return c.Fail("Error reading archives: %s", err)
		}
	}
	if c.append {
		orig, err := vfs.Open(ctx, c.output)
		if err == nil {
			archives = append([]string{c.output}, archives...)
			if err := orig.Close(); err != nil {
				return c.Fail("Error closing original: %v", err)
			}
		}
	}
	if err := c.mergeArchives(ctx, tmpOut, archives, opt); err != nil {
		return c.Fail("Error merging archives: %v", err)
	}
	if err := vfs.Rename(ctx, tmpName, c.output); err != nil {
		return c.Fail("Error renaming tmp to output: %v", err)
	}
	return subcommands.ExitSuccess
}

func (c *mergeCommand) mergeArchives(ctx context.Context, out io.WriteCloser, archives []string, opts ...kzip.WriterOption) error {
	wr, err := kzip.NewWriteCloser(out, opts...)
	if err != nil {
		out.Close()
		return fmt.Errorf("error creating writer: %v", err)
	}

	filesAdded := stringset.New()
	for _, path := range archives {
		if err := c.mergeInto(ctx, wr, path, filesAdded); err != nil {
			wr.Close()
			return err
		}
	}

	if err := wr.Close(); err != nil {
		return fmt.Errorf("error closing writer: %v", err)
	}
	return nil
}

func (c *mergeCommand) mergeInto(ctx context.Context, wr *kzip.Writer, path string, filesAdded stringset.Set) error {
	f, err := vfs.Open(ctx, path)
	if err != nil {
		return fmt.Errorf("error opening archive: %v", err)
	}
	defer f.Close()

	stat, err := vfs.Stat(ctx, path)
	if err != nil {
		return err
	}
	size := stat.Size()
	if size == 0 {
		log.Infof("Skipping empty .kzip: %s", path)
		return nil
	}

	rd, err := kzip.NewReader(f, size)
	if err != nil {
		return fmt.Errorf("error creating reader: %v", err)
	}

	if c.unitsBeforeFiles {
		var requiredDigests []string
		if err := c.mergeUnitsInto(ctx, wr, rd, func(digest string) error {
			requiredDigests = append(requiredDigests, digest)
			return nil
		}); err != nil {
			return err
		}
		for _, digest := range requiredDigests {
			if err := copyFileInto(wr, rd, digest, filesAdded); err != nil {
				return err
			}
		}
		return nil
	}
	return c.mergeUnitsInto(ctx, wr, rd, func(digest string) error {
		return copyFileInto(wr, rd, digest, filesAdded)
	})
}

func (c *mergeCommand) mergeUnitsInto(ctx context.Context, wr *kzip.Writer, rd *kzip.Reader, f func(digest string) error) error {
	return rd.Scan(func(u *kzip.Unit) error {
		for _, ri := range u.Proto.RequiredInput {
			if err := f(ri.Info.Digest); err != nil {
				return err
			}
			if vname, match := c.rules.Apply(ri.Info.Path); match {
				ri.VName = vname
			}
		}
		// TODO(schroederc): duplicate compilations with different revisions
		_, err := wr.AddUnit(u.Proto, u.Index)
		if c.ignoreDuplicateCUs && err == kzip.ErrUnitExists {
			log.Infof("Found duplicate CU: %v", u.Proto.GetDetails())
			return nil
		}
		return err
	})
}

func copyFileInto(wr *kzip.Writer, rd *kzip.Reader, digest string, filesAdded stringset.Set) error {
	if filesAdded.Add(digest) {
		r, err := rd.Open(digest)
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
	return nil
}

func recurseDirectories(ctx context.Context, archives []string) ([]string, error) {
	var files []string
	for _, path := range archives {
		err := vfs.Walk(ctx, path, func(file string, info os.FileInfo, err error) error {
			if err != nil || info.IsDir() {
				return err
			}

			// Include the file if it was directly specified or ends in .kzip.
			if file == path || strings.HasSuffix(file, ".kzip") {
				files = append(files, file)
			}

			return err
		})
		if err != nil {
			return files, err
		}
	}
	return files, nil

}
