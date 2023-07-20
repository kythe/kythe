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
	"fmt"
	"os"
	"runtime"
	"strings"

	"kythe.io/kythe/go/platform/kzip"
	"kythe.io/kythe/go/platform/kzip/info"
	"kythe.io/kythe/go/platform/vfs"
	"kythe.io/kythe/go/util/cmdutil"

	"github.com/google/subcommands"
	"google.golang.org/protobuf/encoding/protojson"
	"google.golang.org/protobuf/encoding/prototext"
)

type infoCommand struct {
	cmdutil.Info

	input           string
	writeFormat     string
	readConcurrency int
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
	fs.IntVar(&c.readConcurrency, "read_concurrency", runtime.NumCPU(), "Max concurrency of reading compilation units from the kzip. Defaults to the number of cpu cores.")
}

// Execute implements the subcommands interface and gathers info from the requested file.
func (c *infoCommand) Execute(ctx context.Context, fs *flag.FlagSet, _ ...any) subcommands.ExitStatus {
	if c.input == "" {
		return c.Fail("Required --input path missing")
	}
	f, err := vfs.Open(ctx, c.input)
	if err != nil {
		return c.Fail("Opening archive: %v", err)
	}
	defer f.Close()

	c.writeFormat = strings.ToLower(c.writeFormat)
	if c.writeFormat != "json" && c.writeFormat != "proto" {
		return c.Fail("Invalid --write_format. Can be 'json' or 'proto'.")
	}

	s, err := vfs.Stat(ctx, c.input)
	if err != nil {
		return c.Fail("Couldn't stat kzip file: %v", err)
	}
	kzipInfo, err := info.KzipInfo(f, s.Size(), kzip.ReadConcurrency(c.readConcurrency))
	if err != nil {
		return c.Fail("Scanning kzip: %v", err)
	}
	var rec []byte
	switch c.writeFormat {
	case "json":
		m := protojson.MarshalOptions{UseProtoNames: true}
		rec, err = m.Marshal(kzipInfo)
		if err != nil {
			return c.Fail("Marshaling json: %v", err)
		}
	case "proto":
		rec, err = prototext.Marshal(kzipInfo)
		if err != nil {
			return c.Fail("Marshaling text: %v", err)
		}
	}
	// Add a new line to the output after we write the kzip info.
	defer func() { fmt.Println() }()
	if _, err := os.Stdout.Write(rec); err != nil {
		return c.Fail("Writing unit: %v", err)
	}

	return subcommands.ExitSuccess
}
