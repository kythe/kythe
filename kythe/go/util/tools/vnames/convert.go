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
	"os"
	"strings"

	"github.com/google/subcommands"
	"kythe.io/kythe/go/util/cmdutil"
)

type convertRulesCmd struct {
	cmdutil.Info

	fromFormat, toFormat string
}

var convertRulesInfo = cmdutil.NewInfo("convert-rules", `convert VName rewrite rules (formats: {"JSON", "PROTO"})`,
	`Usage: convert-rules --from <format> --to <format>`)

func (c *convertRulesCmd) SetFlags(flag *flag.FlagSet) {
	flag.StringVar(&c.fromFormat, "from", "", "Source format of VName rewrite rules")
	flag.StringVar(&c.toFormat, "to", "", "Target format of VName rewrite rules")
}
func (c *convertRulesCmd) Execute(ctx context.Context, flag *flag.FlagSet, args ...interface{}) subcommands.ExitStatus {
	if c.fromFormat == "" {
		return cmdErrorf("--from <format> must be specified")
	} else if c.toFormat == "" {
		return cmdErrorf("--to <format> must be specified")
	}

	rules, err := rulesFormat(strings.ToUpper(c.fromFormat)).readRules(os.Stdin)
	if err != nil {
		return cmdErrorf("reading rules: %v", err)
	} else if err := rulesFormat(strings.ToUpper(c.toFormat)).writeRules(rules, os.Stdout); err != nil {
		return cmdErrorf("writing rules: %v", err)
	}
	return subcommands.ExitSuccess
}
