/*
 * Copyright 2017 The Kythe Authors. All rights reserved.
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
package cli // import "kythe.io/kythe/go/services/cli"

import (
	"context"
	"encoding/json"
	"flag"
	"fmt"
	"os"
	"strings"

	"kythe.io/kythe/go/services/filetree"
	"kythe.io/kythe/go/services/graph"
	"kythe.io/kythe/go/services/web"
	"kythe.io/kythe/go/services/xrefs"
	"kythe.io/kythe/go/serving/identifiers"
	"kythe.io/kythe/go/util/log"

	"github.com/google/subcommands"
	"google.golang.org/protobuf/proto"
)

// DisplayJSON is true if the user wants all service responses to be displayed
// as JSON (using the PrintJSON and PrintJSONMessage functions).
var DisplayJSON bool

var (
	logRequests = flag.Bool("log_requests", false, "Log all requests to stderr as JSON")
	out         = os.Stdout
)

var jsonMarshaler = web.JSONMarshaler

func init() {
	jsonMarshaler.Options.Indent = "  "
	flag.BoolVar(&DisplayJSON, "json", DisplayJSON, "Display results as JSON")
}

// API contains access points the CLI's backend services.
type API struct {
	XRefService       xrefs.Service
	GraphService      graph.Service
	FileTreeService   filetree.Service
	IdentifierService identifiers.Service
}

// Execute registers all Kythe CLI commands to subcommands.DefaultCommander and
// executes it with the given API.
func Execute(ctx context.Context, api API) subcommands.ExitStatus {
	subcommands.ImportantFlag("json")
	subcommands.ImportantFlag("log_requests")
	subcommands.Register(subcommands.HelpCommand(), "usage")
	subcommands.Register(subcommands.FlagsCommand(), "usage")
	subcommands.Register(subcommands.CommandsCommand(), "usage")

	RegisterCommand(&nodesCommand{}, "graph")
	RegisterCommand(&edgesCommand{}, "graph")

	RegisterCommand(&identCommand{}, "")
	RegisterCommand(&lsCommand{}, "")

	RegisterCommand(&decorCommand{}, "xrefs")
	RegisterCommand(&diagnosticsCommand{}, "xrefs")
	RegisterCommand(&docsCommand{}, "xrefs")
	RegisterCommand(&sourceCommand{}, "xrefs")
	RegisterCommand(&xrefsCommand{}, "xrefs")

	return subcommands.Execute(ctx, api)
}

// RegisterCommand adds a KytheCommand to the list of subcommands for the
// specified group.
func RegisterCommand(c KytheCommand, group string) {
	cmd := &commandWrapper{c}
	subcommands.Register(cmd, group)
	for _, a := range c.Aliases() {
		subcommands.Alias(a, cmd)
	}
}

// TODO(schroederc): more documentation per command
// TODO(schroederc): split commands into separate packages

type commandWrapper struct{ KytheCommand }

func (w *commandWrapper) Execute(ctx context.Context, f *flag.FlagSet, args ...any) subcommands.ExitStatus {
	if len(args) != 1 {
		return subcommands.ExitUsageError
	}
	api, ok := args[0].(API)
	if !ok {
		return subcommands.ExitUsageError
	}
	if err := w.Run(ctx, f, api); err != nil {
		log.Errorf("%v", err)
		return subcommands.ExitFailure
	}
	return subcommands.ExitSuccess
}

// A KytheCommand is a type-safe version of the subcommands.Command interface.
type KytheCommand interface {
	Name() string
	Aliases() []string
	Synopsis() string
	Usage() string
	SetFlags(*flag.FlagSet)
	Run(context.Context, *flag.FlagSet, API) error
}

type baseKytheCommand struct{}

func (kc *baseKytheCommand) Aliases() []string      { return nil }
func (kc *baseKytheCommand) Usage() string          { return "" }
func (kc *baseKytheCommand) SetFlags(*flag.FlagSet) {}

// LogRequest should be passed all proto request messages for logging.
func LogRequest(req proto.Message) {
	if *logRequests {
		str, err := jsonMarshaler.MarshalToString(req)
		if err != nil {
			log.Fatalf("Failed to encode request for logging %v: %v", req, err)
		}
		log.Infof("%s: %s", baseTypeName(req), string(str))
	}
}

// PrintJSONMessage prints the given proto message to the console.  This should
// be called whenever the DisplayJSON flag is true.
func PrintJSONMessage(resp proto.Message) error { return jsonMarshaler.Marshal(out, resp) }

// PrintJSON prints the given value to the console.  This should be called
// whenever the DisplayJSON flag is true.  PrintJSONMessage should be preferred
// when possible.
func PrintJSON(val any) error { return json.NewEncoder(out).Encode(val) }

func baseTypeName(x any) string {
	ss := strings.SplitN(fmt.Sprintf("%T", x), ".", 2)
	if len(ss) == 2 {
		return ss[1]
	}
	return ss[0]
}
