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
	"fmt"
	"os"

	"kythe.io/kythe/go/platform/kzip"
	"kythe.io/kythe/go/util/cmdutil"

	"bitbucket.org/creachadair/stringset"
	"github.com/google/subcommands"
)

type infoCommand struct {
	cmdutil.Info

	input string
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
}

// Execute implements the subcommands interface and gathers info from the requested file.
func (c *infoCommand) Execute(ctx context.Context, fs *flag.FlagSet, _ ...interface{}) subcommands.ExitStatus {
	if c.input == "" {
		return c.Fail("required --input path missing")
	}
	f, err := os.Open(c.input)
	if err != nil {
		return c.Fail("error opening archive: %v", err)
	}
	defer f.Close()

	// Get file and unit counts broken down by corpus, language.
	fileBreakdown := make(map[string]map[string]int)
	unitBreakdown := make(map[string]map[string]int)
	var totalFiles, totalUnits int
	corpora := stringset.New()
	err = kzip.Scan(f, func(rd *kzip.Reader, u *kzip.Unit) error {
		totalUnits++
		if _, ok := unitBreakdown[u.Proto.GetVName().GetCorpus()]; !ok {
			unitBreakdown[u.Proto.GetVName().GetCorpus()] = make(map[string]int)
		}
		unitBreakdown[u.Proto.GetVName().GetCorpus()][u.Proto.GetVName().GetLanguage()]++
		corpora.Add(u.Proto.GetVName().GetCorpus())
		for _, ri := range u.Proto.RequiredInput {
			totalFiles++
			if _, ok := fileBreakdown[ri.GetVName().GetCorpus()]; !ok {
				fileBreakdown[ri.GetVName().GetCorpus()] = make(map[string]int)
			}
			corpora.Add(ri.GetVName().GetCorpus())
			fileBreakdown[ri.GetVName().GetCorpus()][ri.GetVName().GetLanguage()]++
		}
		return nil
	})
	if err != nil {
		return c.Fail("error while scanning: %v", err)
	}

	// Write output as json.
	out := make(map[string]interface{})
	out["unit_counts"] = unitBreakdown
	out["file_counts"] = fileBreakdown
	out["total_files"] = totalFiles
	out["total_units"] = totalUnits
	data, err := json.MarshalIndent(&out, "", "  ")
	if err != nil {
		return c.Fail("error marshaling json output: %v", err)
	}
	fmt.Print(string(data))

	return subcommands.ExitSuccess
}
