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
	"flag"
	"fmt"
	"log"
	"os"
	"path/filepath"
	"sort"

	"kythe.io/kythe/go/services/filetree"
	"kythe.io/kythe/go/services/xrefs"
	"kythe.io/kythe/go/util/build"
)

// TODO(schroederc): refactor into reusable Tool struct instead of using globals
//                   absolutely everywhere...

// Run executes a Kythe CLI command using the given services.  As a
// precondition, flag.Parse() must be called.  Run must only be called once.
func Run(xService xrefs.Service, ftService filetree.Service) {
	if len(flag.Args()) == 0 {
		flag.Usage()
		os.Exit(0)
	}
	xs = xService
	ft = ftService
	if err := getCommand(flag.Arg(0)).run(); err != nil {
		log.Fatal("ERROR: ", err)
	}
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
				getCommand(flag.Arg(0)).Usage()
			}
			return nil
		})
	flag.Usage = globalUsage
}

func getCommand(name string) command {
	c, ok := cmds[name]
	if !ok {
		synonymn, found := cmdSynonymns[name]
		if found {
			c, ok = cmds[synonymn]
		}
	}
	if !ok {
		fmt.Fprintf(os.Stderr, "ERROR: unknown command %q\n", name)
		globalUsage()
		os.Exit(1)
	}
	return c
}
