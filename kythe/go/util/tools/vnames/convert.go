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

package main

import (
	"context"
	"flag"
	"io"
	"os"
	"strings"

	"kythe.io/kythe/go/platform/vfs"
	"kythe.io/kythe/go/util/cmdutil"

	"github.com/google/subcommands"
)

type convertRulesCmd struct {
	cmdutil.Info

	rulesPath, outputPath string
	fromFormat, toFormat  string
}

var convertRulesInfo = cmdutil.NewInfo("convert-rules", `convert VName rewrite rules (formats: {"JSON", "PROTO"})`,
	`Usage: convert-rules --from <format> --to <format>`)

func (c *convertRulesCmd) SetFlags(flag *flag.FlagSet) {
	flag.StringVar(&c.fromFormat, "from", "", "Source format of VName rewrite rules")
	flag.StringVar(&c.rulesPath, "rules", "", "Path to VName rewrite rules file (default: read from stdin)")
	flag.StringVar(&c.toFormat, "to", "", "Target format of VName rewrite rules")
	flag.StringVar(&c.outputPath, "output", "", "Output path to write converted VName rewrite rules (default: write to stdout)")
}
func (c *convertRulesCmd) Execute(ctx context.Context, flag *flag.FlagSet, args ...any) subcommands.ExitStatus {
	if c.fromFormat == "" {
		return cmdErrorf("--from <format> must be specified")
	} else if c.toFormat == "" {
		return cmdErrorf("--to <format> must be specified")
	}

	var in io.Reader
	if c.rulesPath == "" {
		in = os.Stdin
	} else {
		f, err := vfs.Open(ctx, c.rulesPath)
		if err != nil {
			return cmdErrorf("opening %q: %v", c.rulesPath, err)
		}
		defer f.Close()
		in = f
	}

	var out io.WriteCloser
	if c.outputPath == "" {
		out = os.Stdout
	} else {
		f, err := vfs.Create(ctx, c.outputPath)
		if err != nil {
			return cmdErrorf("creating %q: %v", c.outputPath, err)
		}
		out = f
	}

	rules, err := rulesFormat(strings.ToUpper(c.fromFormat)).readRules(in)
	if err != nil {
		return cmdErrorf("reading rules: %v", err)
	} else if err := rulesFormat(strings.ToUpper(c.toFormat)).writeRules(rules, out); err != nil {
		return cmdErrorf("writing rules: %v", err)
	} else if err := out.Close(); err != nil {
		return cmdErrorf("closing output: %v", err)
	}

	return subcommands.ExitSuccess
}
