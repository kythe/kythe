/*
 * Copyright 2015 The Kythe Authors. All rights reserved.
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
//
//	# Show complete command listing
//	kythe
//
//	# List all corpus root uris
//	kythe --api /path/to/table ls --uris
//
//	# List root directory contents for corpus named 'somecorpus'
//	kythe --api /path/to/table ls kythe://somecorpus
//
//	# List Kythe's kythe/cxx/common directory (as URIs)
//	kythe --api /path/to/table ls --uris kythe://kythe?path=kythe/cxx/common
//
//	# Display all file anchor decorations for kythe/cxx/common/CommandLineUtils.cc
//	kythe --api /path/to/table decor kythe://kythe?lang=c%2B%2B?path=kythe/cxx/common/CommandLineUtils.cc
//
//	# Show all outward edges for a particular node
//	kythe --api /path/to/table edges kythe:?lang=java#java.util.List
//
//	# Show reverse /kythe/edge/defines edges for a node
//	kythe --api /path/to/table edges --kinds '%/kythe/edge/defines' kythe://kythe?lang=java?path=kythe/java/com/google/devtools/kythe/analyzers/base/EntrySet.java#1887f665ee4c77287d1022c151000a489e17147215309818cf4150c601442cc5
//
//	# Show all facts (except /kythe/text) for a node
//	kythe --api /path/to/table node kythe:?lang=c%2B%2B#StripPrefix%3Acommon%3Akythe%23n%23D%40kythe%2Fcxx%2Fcommon%2FCommandLineUtils.cc%3A167%3A1
package main

import (
	"context"
	"flag"
	"os"

	"kythe.io/kythe/go/services/cli"
	"kythe.io/kythe/go/serving/api"
)

func main() {
	apiFlag := api.Flag("api", api.CommonDefault, api.CommonFlagUsage)
	flag.Parse()

	ctx := context.Background()
	status := cli.Execute(ctx, cli.API{
		XRefService:       *apiFlag,
		GraphService:      *apiFlag,
		FileTreeService:   *apiFlag,
		IdentifierService: *apiFlag,
	})
	(*apiFlag).Close(ctx)
	os.Exit(int(status))
}
