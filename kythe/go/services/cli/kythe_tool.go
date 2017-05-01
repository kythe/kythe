/*
 * Copyright 2015 Google Inc. All rights reserved.
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
	"errors"
	"flag"
	"fmt"
	"os"
	"path/filepath"
	"sort"

	"kythe.io/kythe/go/services/filetree"
	"kythe.io/kythe/go/services/xrefs"
	"kythe.io/kythe/go/util/build"
)

// API contains access points the CLI's backend services.
type API struct {
	XRefService     xrefs.Service
	FileTreeService filetree.Service
}

// Command represents a single CLI command that can be executed against an API.
type Command interface {
	// Run executes a command against a given API.
	Run(context.Context, API) error
}

// ParseCommand returns the Command specified by the given command-line
// arguments.  The first argument must be the name of the Command.
//
// Example arguments: []string{"magic_command", "--flag1", "blah"}
func ParseCommand(args []string) (Command, error) {
	if len(args) == 0 {
		return nil, errors.New("no subcommand specified")
	}
	name, args := args[0], args[1:]
	c, err := getCommand(name)
	if err != nil {
		return nil, err
	} else if err := c.parseArguments(args); err != nil {
		return nil, err
	}
	return c, nil
}

var shortHelp bool

func globalUsage() {
	fmt.Fprintf(os.Stderr, `Usage: %s <global-flags> <command> <flags>

%s

Examples:
  %[1]s ls --uris kythe://kythe?path=kythe/cxx/common
  %[1]s node kythe:?lang=java#java.util.List
`, filepath.Base(os.Args[0]), build.VersionLine())
	if !shortHelp {
		fmt.Fprintln(os.Stderr, "\nGlobal Flags:")
		flag.PrintDefaults()
	}
	fmt.Fprintln(os.Stderr, "\nCommands:")
	var cmdNames []string
	for name := range cmds {
		cmdNames = append(cmdNames, name)
	}
	sort.Strings(cmdNames)
	for _, name := range cmdNames {
		cmds[name].Usage()
		fmt.Fprintln(os.Stderr)
	}
}

var cmds = map[string]command{
	"edges":  cmdEdges,
	"ls":     cmdLS,
	"node":   cmdNode,
	"decor":  cmdDecor,
	"source": cmdSource,
	"xrefs":  cmdXRefs,
	"docs":   cmdDocs,
}

var cmdSynonymns = map[string]string{
	"edge":             "edges",
	"nodes":            "node",
	"decorations":      "decor",
	"refs":             "decor", // for backwards-compatibility
	"cross-references": "xrefs",
}

func init() {
	cmds["help"] = newCommand("help", "[command]",
		"Print help information for the given command",
		func(flag *flag.FlagSet) {
			flag.BoolVar(&shortHelp, "short", false, "Display only command descriptions")
		}, func(flag *flag.FlagSet) error {
			if len(flag.Args()) == 0 {
				globalUsage()
			} else {
				c, err := getCommand(flag.Arg(0))
				if err != nil {
					return err
				}
				c.Usage()
			}
			return nil
		})
	flag.Usage = globalUsage
}

func getCommand(name string) (*command, error) {
	c, ok := cmds[name]
	if !ok {
		synonymn, found := cmdSynonymns[name]
		if found {
			c, ok = cmds[synonymn]
		}
	}
	if !ok {
		return nil, fmt.Errorf("unknown command %q", name)
	}
	return &c, nil
}
