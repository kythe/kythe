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

// Program extract_proto_kzip implements a Bazel extra action that captures a
// Kythe compilation record for a protobuf "spawn" action.
package main

import (
	"context"
	"flag"
	"fmt"
	"io/ioutil"
	"os"
	"path/filepath"
	"strings"
	"time"

	"kythe.io/kythe/go/extractors/bazel"
	"kythe.io/kythe/go/util/log"
	"kythe.io/kythe/go/util/vnameutil"

	"bitbucket.org/creachadair/stringset"

	apb "kythe.io/kythe/proto/analysis_go_proto"
)

var (
	corpus      = flag.String("corpus", "", "Corpus label to assign")
	language    = flag.String("language", "", "Language label to assign (required)")
	extraAction = flag.String("extra_action", "", "Path of bazel.ExtraActionInfo file (required)")
	outputPath  = flag.String("output", "", "Path of output index file (required)")
	vnameRules  = flag.String("rules", "", "Path of vnames.json file (optional)")
	verboseLog  = flag.Bool("v", false, "Enable verbose (per-file) logging")
)

func init() {
	flag.Usage = func() {
		fmt.Fprintf(os.Stderr, `Usage: %s [options] -corpus C -language L -extra_action f.xa -output f.kzip

Read an ExtraActionInput protobuf message from the designated -extra_action
file[*] and generate a Kythe compilation record in the specified -output file.
The -corpus and -language labels are required to be non-empty.  This only works
for protobuf "spawn" actions; any other action will cause extraction to fail.

[*] https://bazel.build/versions/master/docs/be/extra-actions.html

Options:
`, filepath.Base(os.Args[0]))
		flag.PrintDefaults()
	}
}

func main() {
	flag.Parse()

	// Verify that required flags are set.
	switch {
	case *language == "":
		log.Fatal("You must provide a non-empty --language label")
	case *extraAction == "":
		log.Fatal("You must provide a non-empty --extra_action file path")
	case *outputPath == "":
		log.Fatal("You must provide a non-empty --output file path")
	}

	// Ensure the extra action can be loaded.
	info, err := bazel.LoadAction(*extraAction)
	if err != nil {
		log.Fatalf("Loading extra action: %v", err)
	}
	pkg := bazel.PackageName(info.GetOwner())
	log.Infof("Extra action for target %q (package %q)", info.GetOwner(), pkg)

	rules, err := vnameutil.LoadRules(*vnameRules)
	if err != nil {
		log.Fatalf("Loading rules: %v", err)
	}
	config := &bazel.Config{
		Corpus:   *corpus,
		Language: *language,
		Rules:    rules,
		Verbose:  *verboseLog,
		CheckInput: func(path string) (string, bool) {
			switch filepath.Ext(path) {
			case ".proto", ".protodevel":
				return path, true
			default:
				return path, false
			}
		},
	}

	// Add a fixup to extract the source files belonging to this compilation
	// from the command-line arguments. The arguments contain mappings of the
	// form:
	//
	//     -I<import>=<filepath>
	//
	// where <import> denotes an import path as written in the .proto source
	// and <filepath> denotes the actual file path to be imported to satisfy
	// it. We need to take these mappings into account when identifying the
	// source files that belong to this compilation
	config.FixUnit = func(cu *apb.CompilationUnit) error {
		var inputs stringset.Set
		for _, ri := range cu.RequiredInput {
			inputs.Add(ri.Info.GetPath())
		}

		// Pass 1: Gather the import mappings.
		importToPath := make(map[string]string)
		for _, arg := range cu.Argument {
			if trim := strings.TrimPrefix(arg, "-I"); trim != arg {
				parts := strings.SplitN(trim, "=", 2)
				if len(parts) == 2 {
					importToPath[parts[0]] = parts[1]
				}
			}
		}

		// Pass 2: Gather the input files.
		srcs := stringset.New(cu.SourceFile...)

		// The argument list may include a response file; expand it if so.
		args, err := expandParams(cu.Argument)
		if err != nil {
			return err
		}

		for _, arg := range args {
			if strings.HasPrefix(arg, "-") {
				continue // don't capture flags
			} else if ext := filepath.Ext(arg); ext != ".proto" && ext != ".protodevel" {
				continue // skip non-proto files
			} else if path, ok := importToPath[arg]; ok && inputs.Contains(path) {
				srcs.Add(path) // map imports to paths
			} else if inputs.Contains(arg) {
				srcs.Add(arg)
			}
		}
		cu.Argument = args
		cu.SourceFile = srcs.Elements()
		return nil
	}

	start := time.Now()
	ai, err := bazel.SpawnAction(info)
	if err != nil {
		log.Fatalf("Invalid extra action: %v", err)
	}

	ctx := context.Background()
	if err := config.ExtractToKzip(ctx, ai, *outputPath); err != nil {
		log.Fatalf("Extraction failed: %v", err)
	}
	log.Infof("Finished extracting [%v elapsed]", time.Since(start))
}

// expandParams expands any parameter files occurring in args into their
// contents, and returns the modified argument list. Parameter files are
// distinguished by having a "@" prefix.
func expandParams(args []string) ([]string, error) {
	var expanded []string
	for _, arg := range args {
		trim := strings.TrimPrefix(arg, "@")
		if trim == arg {
			expanded = append(expanded, arg) // not a response file
			continue
		}
		data, err := ioutil.ReadFile(trim)
		if err != nil {
			return nil, fmt.Errorf("reading response file: %v", err)
		}
		lines := strings.TrimSpace(string(data))
		expanded = append(expanded, strings.Split(lines, "\n")...)
	}
	return expanded, nil
}
