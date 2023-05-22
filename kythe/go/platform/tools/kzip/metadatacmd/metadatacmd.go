/*
 * Copyright 2020 The Kythe Authors. All rights reserved.
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

// Package metadatacmd provides the kzip command for creating metadata kzips.
package metadatacmd // import "kythe.io/kythe/go/platform/tools/kzip/metadatacmd"

import (
	"context"
	"flag"
	"time"

	"kythe.io/kythe/go/platform/kzip"
	"kythe.io/kythe/go/platform/kzip/buildmetadata"
	"kythe.io/kythe/go/platform/tools/kzip/flags"
	"kythe.io/kythe/go/platform/vfs"
	"kythe.io/kythe/go/util/cmdutil"
	"kythe.io/kythe/go/util/log"

	"github.com/google/subcommands"
)

const (
	// GitDateFormat is the default date format used by `git log`.
	GitDateFormat = "Mon Jan 2 15:04:05 2006 -0700"
)

type metadataCommand struct {
	cmdutil.Info

	output          string
	corpus          string
	encoding        flags.EncodingFlag
	commitTimestamp string
	timestampFormat string
}

// New creates a new subcommand for creating metadata kzip files.
func New() subcommands.Command {
	return &metadataCommand{
		Info:     cmdutil.NewInfo("create_metadata", "create a metadata kzip file, which records extra repository information", "--output path --commit_timestamp *"),
		encoding: flags.EncodingFlag{Encoding: kzip.EncodingJSON},
	}
}

// SetFlags implements the subcommands interface and provides command-specific flags
// for merging kzip files.
func (c *metadataCommand) SetFlags(fs *flag.FlagSet) {
	fs.StringVar(&c.output, "output", "", "Path to output kzip file")
	fs.StringVar(&c.corpus, "corpus", "", "Corpus to set in the unit's VName")
	fs.StringVar(&c.commitTimestamp, "commit_timestamp", "", "Timestamp when this version of the repository was checked in")
	fs.StringVar(&c.timestampFormat, "timestamp_format", GitDateFormat, "Format of timestamp passed to --commit_timestamp. Defaults to git's default date format: 'Mon Jan 2 15:04:05 2006 -0700'")
	fs.Var(&c.encoding, "encoding", "Encoding to use on output, one of JSON, PROTO, or ALL")
}

// Execute implements the subcommands interface and creates a metadata kzip file.
func (c *metadataCommand) Execute(ctx context.Context, fs *flag.FlagSet, _ ...any) (exitcode subcommands.ExitStatus) {
	if c.output == "" {
		return c.Fail("Required --output path missing")
	}
	if c.commitTimestamp == "" {
		return c.Fail("Required --commit_timestamp missing")
	}
	if c.corpus == "" {
		log.Warningf("No --corpus provided")
	}

	t, err := time.Parse(c.timestampFormat, c.commitTimestamp)
	if err != nil {
		return c.Fail("Unable to parse timestamp: '%s'", c.commitTimestamp)
	}
	unit, err := buildmetadata.CreateMetadataUnit(c.corpus, t)
	if err != nil {
		return c.Fail("Error creating compilation unit: %v", err)
	}

	f, err := vfs.Create(ctx, c.output)
	if err != nil {
		return c.Fail("Creating output file: %v", err)
	}
	opt := kzip.WithEncoding(c.encoding.Encoding)
	w, err := kzip.NewWriteCloser(f, opt)
	if err != nil {
		f.Close()
		return c.Fail("Failed to close writer: %v", err)
	}
	if _, err := w.AddUnit(unit, nil); err != nil {
		w.Close()
		return c.Fail("Failed to add unit: %v", err)
	}
	if err := w.Close(); err != nil {
		return c.Fail("Failed to close kzip: %v", err)
	}

	return subcommands.ExitSuccess
}
