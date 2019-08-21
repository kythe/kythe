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

// Package infocmd provides the kzip command for obtaining info about a kzip archive.
package infocmd // import "kythe.io/kythe/go/platform/tools/kzip/infocmd"

import (
	"context"
	"flag"
	"os"
	"strings"

	"kythe.io/kythe/go/platform/kzip/info"
	"kythe.io/kythe/go/platform/vfs"
	"kythe.io/kythe/go/util/cmdutil"

	"github.com/golang/protobuf/jsonpb"
	"github.com/golang/protobuf/proto"
	"github.com/google/subcommands"
)

type infoCommand struct {
	cmdutil.Info

	input       string
	writeFormat string
}

// New creates a new subcommand for obtaining info on a kzip file.
func New() subcommands.Command {
	return &infoCommand{
		Info: cmdutil.NewInfo("info", "info on single kzip archive", "--input path"),
	}
}

// SetFlags implements the subcommands interface and provides command-specific flags
// for the info command.
func (c *infoCommand) SetFlags(fs *flag.FlagSet) {
	fs.StringVar(&c.input, "input", "", "Path for input kzip file (required)")
	fs.StringVar(&c.writeFormat, "write_format", "json", "Output format, can be 'json' or 'proto'")
}

// Execute implements the subcommands interface and gathers info from the requested file.
func (c *infoCommand) Execute(ctx context.Context, fs *flag.FlagSet, _ ...interface{}) subcommands.ExitStatus {
	if c.input == "" {
		return c.Fail("required --input path missing")
	}
	f, err := vfs.Open(ctx, c.input)
	if err != nil {
		return c.Fail("opening archive: %v", err)
	}
	defer f.Close()

	c.writeFormat = strings.ToLower(c.writeFormat)
	if c.writeFormat != "json" && c.writeFormat != "proto" {
		return c.Fail("Invalid --write_format. Can be 'json' or 'proto'.")
	}

	kzipInfo, err := info.KzipInfo(f)
	if err != nil {
		return c.Fail("scanning kzip: %v", err)
	}
	switch c.writeFormat {
	case "json":
		m := jsonpb.Marshaler{OrigName: true}
		if err := m.Marshal(os.Stdout, kzipInfo); err != nil {
			return c.Fail("marshaling json: %v", err)
		}
	case "proto":
		proto.MarshalText(os.Stdout, kzipInfo)
	}

	return subcommands.ExitSuccess
}
