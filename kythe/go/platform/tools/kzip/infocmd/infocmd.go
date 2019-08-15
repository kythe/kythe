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
	"encoding/json"
	"flag"
	"os"
	"strings"

	"kythe.io/kythe/go/platform/kzip"
	"kythe.io/kythe/go/platform/vfs"
	"kythe.io/kythe/go/util/cmdutil"
	apb "kythe.io/kythe/proto/analysis_go_proto"

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
		return c.Fail("error opening archive: %v", err)
	}
	defer f.Close()

	c.writeFormat = strings.ToLower(c.writeFormat)
	if c.writeFormat != "json" && c.writeFormat != "proto" {
		return c.Fail("Invalid --write_format. Can be 'json' or 'proto'.")
	}

	// Get file and unit counts broken down by corpus, language.
	kzipInfo := &apb.KzipInfo{Corpora: make(map[string]*apb.KzipInfo_CorpusInfo)}
	err = kzip.Scan(f, func(rd *kzip.Reader, u *kzip.Unit) error {
		kzipInfo.TotalUnits++
		if kzipInfo.Corpora[u.Proto.GetVName().GetCorpus()] == nil {
			kzipInfo.Corpora[u.Proto.GetVName().GetCorpus()] = &apb.KzipInfo_CorpusInfo{
				Files: make(map[string]int32),
				Units: make(map[string]int32)}
		}
		kzipInfo.Corpora[u.Proto.GetVName().GetCorpus()].Units[u.Proto.GetVName().GetLanguage()]++

		for _, ri := range u.Proto.RequiredInput {
			kzipInfo.TotalFiles++
			if kzipInfo.Corpora[ri.GetVName().GetCorpus()] == nil {
				kzipInfo.Corpora[ri.GetVName().GetCorpus()] = &apb.KzipInfo_CorpusInfo{
					Files: make(map[string]int32),
					Units: make(map[string]int32)}
			}
			kzipInfo.Corpora[ri.GetVName().GetCorpus()].Files[ri.GetVName().GetLanguage()]++
		}
		return nil
	})
	if err != nil {
		return c.Fail("error while scanning: %v", err)
	}

	switch c.writeFormat {
	case "json":
		enc := json.NewEncoder(os.Stdout)
		enc.Encode(kzipInfo)
	case "proto":
		proto.MarshalText(os.Stdout, kzipInfo)
	}

	return subcommands.ExitSuccess
}
