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

// Binary kythe exposes a CLI interface to the xrefs and filetree
// services backed by a combined serving table.
//
// Examples:
//   # Show complete command listing
//   kythe
//
//   # List all corpus root uris
//   kythe --api /path/to/table ls --uris
//
//   # List root directory contents for corpus named 'somecorpus'
//   kythe --api /path/to/table ls kythe://somecorpus
//
//   # List Kythe's kythe/cxx/common directory (as URIs)
//   kythe --api /path/to/table ls --uris kythe://kythe?path=kythe/cxx/common
//
//   # Display all file anchors references for kythe/cxx/common/CommandLineUtils.cc
//   kythe --api /path/to/table refs kythe://kythe?lang=c%2B%2B?path=kythe/cxx/common/CommandLineUtils.cc
//
//   # Show all outward edges for a particular node
//   kythe --api /path/to/table edges kythe:?lang=java#java.util.List
//
//   # Show reverse /kythe/edge/defines edges for a node
//   kythe --api /path/to/table edges --kinds '%/kythe/edge/defines' kythe://kythe?lang=java?path=kythe/java/com/google/devtools/kythe/analyzers/base/EntrySet.java#1887f665ee4c77287d1022c151000a489e17147215309818cf4150c601442cc5
//
//   # Show all facts (except /kythe/text) for a node
//   kythe --api /path/to/table node kythe:?lang=c%2B%2B#StripPrefix%3Acommon%3Akythe%23n%23D%40kythe%2Fcxx%2Fcommon%2FCommandLineUtils.cc%3A167%3A1
//
//   # Search for all Java class nodes with the given VName path
//   kythe --api /path/to/table search --lang java --path kythe/java/com/google/devtools/kythe/analyzers/base/EntrySet.java /kythe/node/kind record /kythe/subkind class
package main

import (
	"flag"
	"fmt"
	"log"
	"os"
	"path/filepath"
	"sort"
	"strings"

	"kythe.io/kythe/go/services/filetree"
	"kythe.io/kythe/go/services/search"
	"kythe.io/kythe/go/services/xrefs"
	ftsrv "kythe.io/kythe/go/serving/filetree"
	srchsrv "kythe.io/kythe/go/serving/search"
	xsrv "kythe.io/kythe/go/serving/xrefs"
	"kythe.io/kythe/go/storage/leveldb"
	"kythe.io/kythe/go/storage/table"

	"google.golang.org/grpc"

	ftpb "kythe.io/kythe/proto/filetree_proto"
	spb "kythe.io/kythe/proto/storage_proto"
	xpb "kythe.io/kythe/proto/xref_proto"
)

var (
	apiSpec = flag.String("api", "https://xrefs-dot-kythe-repo.appspot.com", "Backing API specification (e.g. JSON HTTP server: https://xrefs-dot-kythe-repo.appspot.com or GRPC server: localhost:1003 or local serving table path: /var/kythe_serving)")

	shortHelp bool
)

func globalUsage() {
	fmt.Fprintf(os.Stderr, `Usage: %s <global-flags> <command> <flags>

Examples:
  %[1]s ls --uris kythe://kythe?path=kythe/cxx/common
  %[1]s search --path kythe/cxx/common/CommandLineUtils.h /kythe/node/kind file
  %[1]s node kythe:?lang=java#java.util.List
`, filepath.Base(os.Args[0]))
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
	"refs":   cmdRefs,
	"source": cmdSource,
	"search": cmdSearch,
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

func main() {
	flag.Parse()
	if len(flag.Args()) == 0 {
		flag.Usage()
		os.Exit(0)
	} else if *apiSpec == "" {
		log.Fatal("--api specification is required")
	}

	if strings.HasPrefix(*apiSpec, "http://") || strings.HasPrefix(*apiSpec, "https://") {
		xs = xrefs.WebClient(*apiSpec)
		ft = filetree.WebClient(*apiSpec)
		idx = search.WebClient(*apiSpec)
	} else if _, err := os.Stat(*apiSpec); err == nil {
		db, err := leveldb.Open(*apiSpec, nil)
		if err != nil {
			log.Fatalf("Error opening local DB at %q: %v", *apiSpec, err)
		}
		defer db.Close()

		tbl := &table.KVProto{db}
		xs = &xsrv.Table{tbl}
		ft = &ftsrv.Table{tbl}
		idx = &srchsrv.Table{&table.KVInverted{db}}
	} else {
		conn, err := grpc.Dial(*apiSpec)
		if err != nil {
			log.Fatalf("Error connecting to remote API %q: %v", *apiSpec, err)
		}
		defer conn.Close()
		xs = xrefs.GRPC(xpb.NewXRefServiceClient(conn))
		ft = filetree.GRPC(ftpb.NewFileTreeServiceClient(conn))
		idx = search.GRPC(spb.NewSearchServiceClient(conn))
	}

	if err := getCommand(flag.Arg(0)).run(); err != nil {
		log.Fatal("ERROR: ", err)
	}
}

func getCommand(name string) command {
	c, ok := cmds[name]
	if !ok {
		fmt.Fprintf(os.Stderr, "ERROR: unknown command %q\n", name)
		globalUsage()
		os.Exit(1)
	}
	return c
}
