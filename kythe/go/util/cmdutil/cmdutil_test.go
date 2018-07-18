/*
 * Copyright 2018 The Kythe Authors. All rights reserved.
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

package cmdutil_test

import (
	"context"
	"flag"
	"fmt"

	"github.com/google/subcommands"
	"kythe.io/kythe/go/util/cmdutil"
)

func ExampleNewInfo() {
	cmd := struct {
		cmdutil.Info
	}{
		Info: cmdutil.NewInfo("example", "Demonstrate how to set up a subcommand",
			`Show the user how to use the cmdutil.NewInfo function.`),
	}

	// Set up a flag set for demo purposes; most tools will use the default
	// command line flags from the flag package.
	fs := flag.NewFlagSet("test", flag.ExitOnError)
	fs.Parse([]string{"example", "foo"})

	// Register a command with the dispatcher.
	cmdr := subcommands.NewCommander(fs, "cmdutil_test")
	cmdr.Register(cmd, "examples")

	// Execute...
	fmt.Println(cmdr.Execute(context.Background(), fs))
	// Output:
	// Show the user how to use the cmdutil.NewInfo function.
	// 0
}
