/*
 * Copyright 2017 Google Inc. All rights reserved.
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

// Package cli exposes a CLI interface to the Kythe services.
package cli

import (
	"context"
	"flag"
	"fmt"
	"log"
	"os"
	"strings"

	"kythe.io/kythe/go/services/filetree"
	"kythe.io/kythe/go/services/web"
	"kythe.io/kythe/go/services/xrefs"

	"github.com/golang/protobuf/proto"
	"github.com/google/subcommands"
)

var (
	logRequests = flag.Bool("log_requests", false, "Log all requests to stderr as JSON")
	displayJSON = flag.Bool("json", false, "Display results as JSON")
	out         = os.Stdout
)

var jsonMarshaler = web.JSONMarshaler

func init() { jsonMarshaler.Indent = "  " }

// API contains access points the CLI's backend services.
type API struct {
	XRefService     xrefs.Service
	FileTreeService filetree.Service
}

// Execute registers all Kythe CLI commands to subcommands.DefaultCommander and
// executes it with the given API.
func Execute(ctx context.Context, api API) subcommands.ExitStatus {
	subcommands.ImportantFlag("json")
	subcommands.ImportantFlag("log_requests")
	subcommands.Register(subcommands.HelpCommand(), "usage")
	subcommands.Register(subcommands.FlagsCommand(), "usage")
	subcommands.Register(subcommands.CommandsCommand(), "usage")
	registerAllCommands(subcommands.DefaultCommander)
	return subcommands.Execute(ctx, api)
}

// registerAllCommands registers all Kythe subcommands with the given Commander.
func registerAllCommands(cdr *subcommands.Commander) {
	cdr.Register(&commandWrapper{&nodesCommand{}}, "graph")
	cdr.Register(&commandWrapper{&edgesCommand{}}, "graph")

	cdr.Register(&commandWrapper{&lsCommand{}}, "")

	cdr.Register(&commandWrapper{&decorCommand{}}, "xrefs")
	cdr.Register(&commandWrapper{&diagnosticsCommand{}}, "xrefs")
	cdr.Register(&commandWrapper{&docsCommand{}}, "xrefs")
	cdr.Register(&commandWrapper{&sourceCommand{}}, "xrefs")
	cdr.Register(&commandWrapper{&xrefsCommand{}}, "xrefs")
}

// TODO(schroederc): subcommand aliases
// TODO(schroederc): more documentation per command
// TODO(schroederc): split commands into separate packages

type commandWrapper struct{ kytheCommand }

func (w *commandWrapper) Execute(ctx context.Context, f *flag.FlagSet, args ...interface{}) subcommands.ExitStatus {
	if len(args) != 1 {
		return subcommands.ExitUsageError
	}
	api, ok := args[0].(API)
	if !ok {
		return subcommands.ExitUsageError
	}
	if err := w.Run(ctx, f, api); err != nil {
		log.Printf("ERROR: %v", err)
		return subcommands.ExitFailure
	}
	return subcommands.ExitSuccess
}

// A kytheCommand is a type-safe version of the subcommands.Command interface.
type kytheCommand interface {
	Name() string
	Synopsis() string
	Usage() string
	SetFlags(*flag.FlagSet)
	Run(context.Context, *flag.FlagSet, API) error
}

func logRequest(req proto.Message) {
	if *logRequests {
		str, err := jsonMarshaler.MarshalToString(req)
		if err != nil {
			log.Fatalf("Failed to encode request for logging %v: %v", req, err)
		}
		log.Printf("%s: %s", baseTypeName(req), string(str))
	}
}

func baseTypeName(x interface{}) string {
	ss := strings.SplitN(fmt.Sprintf("%T", x), ".", 2)
	if len(ss) == 2 {
		return ss[1]
	}
	return ss[0]
}
