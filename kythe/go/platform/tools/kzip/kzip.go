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

// Binary kzip provides tools to work with kzip archives.
//
// Examples:
//   # Merge 5 kzip archives into a single file.
//   kzip merge --output output.kzip in{0,1,2,3,4}.kzip
package main

import (
	"context"
	"flag"
	"os"

	"kythe.io/kythe/go/platform/tools/kzip/createcmd"
	"kythe.io/kythe/go/platform/tools/kzip/filtercmd"
	"kythe.io/kythe/go/platform/tools/kzip/infocmd"
	"kythe.io/kythe/go/platform/tools/kzip/mergecmd"
	"kythe.io/kythe/go/platform/tools/kzip/viewcmd"

	"github.com/google/subcommands"
)

func init() {
	subcommands.Register(createcmd.New(), "")
	subcommands.Register(filtercmd.New(), "")
	subcommands.Register(infocmd.New(), "")
	subcommands.Register(mergecmd.New(), "")
	subcommands.Register(viewcmd.New(), "")
}

func main() {
	flag.Parse()
	ctx := context.Background()

	os.Exit(int(subcommands.Execute(ctx)))
}
