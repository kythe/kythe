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

// Binary vnames is a utility for handling VNames.
//
// Examples:
//   # Convert VName rewrite rules from json to proto.
//   vnames convert-rules --from json --to proto < rules.json > rules.pb.bin
//
//   # Apply VName rewrite rules to stdin.
//   print -l path1 path2 path3 | vnames apply-rules --rules rules.json
package main

import (
	"context"
	"flag"
	"os"

	"github.com/google/subcommands"
)

func main() {
	flag.Parse()

	subcommands.Register(&applyRulesCmd{Info: applyRulesInfo}, "rules")
	subcommands.Register(&convertRulesCmd{Info: convertRulesInfo}, "rules")

	subcommands.Register(subcommands.FlagsCommand(), "info")
	subcommands.Register(subcommands.HelpCommand(), "info")

	ctx := context.Background()
	os.Exit(int(subcommands.Execute(ctx)))
}
