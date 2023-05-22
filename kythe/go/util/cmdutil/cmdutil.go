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

// Package cmdutil exports shared logic for implementing command-line
// subcommands using the github.com/google/subcommands package.
package cmdutil // import "kythe.io/kythe/go/util/cmdutil"

import (
	"context"
	"flag"
	"fmt"
	"log"
	"strings"

	"github.com/google/subcommands"
)

// Info implements the methods of the subcommands.Command interface that handle
// the command name and documentation. It also provides a noop default SetFlags
// method and a default Execute method that prints its usage and exits.
type Info struct {
	name     string
	synopsis string
	usage    string
}

// NewInfo constructs an Info that reports the specified arguments for command
// name, brief synopsis, and usage.
func NewInfo(name, synopsis, usage string) Info {
	if !strings.HasSuffix(usage, "\n") {
		usage += "\n"
	}
	return Info{name: name, synopsis: synopsis, usage: usage}
}

// Name implements part of subcommands.Command.
func (i Info) Name() string { return i.name }

// Synopsis implements part of subcommands.Command.
func (i Info) Synopsis() string { return i.synopsis }

// Usage implements part of subcommands.Command.
func (i Info) Usage() string { return i.usage + "\nOptions:\n" }

// SetFlags implements part of subcommands.Command.
func (i Info) SetFlags(*flag.FlagSet) {}

// Execute implements part of subcommands.Command.
// It prints the usage string to stdout and returns success.
func (i Info) Execute(context.Context, *flag.FlagSet, ...any) subcommands.ExitStatus {
	fmt.Print(i.usage) // the undecorated usage string
	return subcommands.ExitSuccess
}

// Fail logs an error message and returns subcommands.ExitFailure.
func (i Info) Fail(msg string, args ...any) subcommands.ExitStatus {
	log.Output(1, fmt.Sprintf(msg, args...))
	return subcommands.ExitFailure
}
