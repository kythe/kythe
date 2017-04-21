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

// Program extract_kindex implements a Bazel extra action that captures a Kythe
// compilation record for a "spawn" action.
package main

import (
	"context"
	"flag"
	"fmt"
	"io"
	"io/ioutil"
	"log"
	"os"
	"path/filepath"
	"regexp"
	"time"

	"bitbucket.org/creachadair/stringset"

	"github.com/golang/protobuf/proto"

	"kythe.io/kythe/go/extractors/bazel"
	"kythe.io/kythe/go/platform/kindex"
	"kythe.io/kythe/go/util/vnameutil"

	xapb "kythe.io/third_party/bazel/extra_actions_base_proto"
)

var (
	corpus       = flag.String("corpus", "", "Corpus label to assign (required)")
	language     = flag.String("language", "", "Language label to assign (required)")
	extraAction  = flag.String("extra_action", "", "Path of blaze.ExtraActionInfo file (required)")
	outputPath   = flag.String("output", "", "Path of output index file (required)")
	vnameRules   = flag.String("rules", "", "Path of vnames.json file (optional)")
	matchFiles   = flag.String("include", "", `RE2 matching files to include (if "", include all files)`)
	excludeFiles = flag.String("exclude", "", `RE2 matching files to exclude (if "", exclude none)`)
	sourceFiles  = flag.String("source", "", `RE2 matching source files (if "", select none)`)
	sourceArgs   = flag.String("args", "", `RE2 matching arguments to consider source files (if "", select none)`)
)

func init() {
	flag.Usage = func() {
		fmt.Fprintf(os.Stderr, `Usage: %s [options] -corpus C -language L -extra_action f.xa -output f.kindex

Read an ExtraActionInput protobuf message from the designated -extra_action
file[*] and generate a Kythe compilation record in the specified -output file.
The -corpus and -language labels are required to be non-empty.  This only works
for "spawn" actions; any other action will cause the extraction to fail.

By default, all input files listed in the XA are captured in the output.
To capture only files matching a RE2 regexp, use -include.
To explicitly exclude otherwise-matched files, use -exclude.
If both are given, -exclude applies to files selected by -include.

To designate paths as source inputs, set -source to a RE2 regexp matching them,
or set -args to a RE2 regexp matching command-line arguments that should be
considered source paths.  At least one of these must be set, and it is safe to
set both; the results will be merged.

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
	case *corpus == "":
		log.Fatal("You must provide a non-empty --corpus label")
	case *language == "":
		log.Fatal("You must provide a non-empty --language label")
	case *extraAction == "":
		log.Fatal("You must provide a non-empty --extra_action file path")
	case *outputPath == "":
		log.Fatal("You must provide a non-empty --output file path")
	case *sourceFiles == "" && *sourceArgs == "":
		log.Fatal("You must set at least one of --source and --args")
	}

	config := &bazel.Config{
		Corpus:     *corpus,
		Language:   *language,
		Rules:      mustLoadRules(*vnameRules),
		CheckInput: func(path string) (string, bool) { return path, true },
	}

	// If there is a file inclusion regexp, replace the input filter with it.
	if *matchFiles != "" {
		r := mustRegexp(*matchFiles, "inclusion")
		config.CheckInput = func(path string) (string, bool) {
			return path, r.MatchString(path)
		}
	}

	// If there is a file exclusion regexp, add it to the input filter.  We
	// know there is already a callback in place (either a match or the trivial
	// default).
	if *excludeFiles != "" {
		r := mustRegexp(*excludeFiles, "exclusion")

		base := config.CheckInput
		config.CheckInput = func(path string) (string, bool) {
			if path, ok := base(path); ok {
				return path, !r.MatchString(path)
			}
			return "", false
		}
	}

	// If there is a source matching regexp, set a filter for it.
	if *sourceFiles != "" {
		r := mustRegexp(*sourceFiles, "source file")
		config.IsSource = r.MatchString
	}

	// If we have been asked to include source files from the argument list,
	// add a fixup to do that at the end.
	if *sourceArgs != "" {
		r := mustRegexp(*sourceArgs, "source argument")
		config.Fixup = func(cu *kindex.Compilation) error {
			srcs := stringset.New(cu.Proto.SourceFile...)
			for _, arg := range cu.Proto.Argument {
				if r.MatchString(arg) {
					srcs.Add(arg)
				}
			}
			cu.Proto.SourceFile = srcs.Elements()
			return nil
		}
	}

	start := time.Now()
	cu, err := config.Extract(context.Background(), mustLoadAction(*extraAction))
	if err != nil {
		log.Fatalf("Extraction failed: %v", err)
	}
	log.Printf("Finished extracting [%v elapsed]", time.Since(start))
	mustWrite(cu, *outputPath)
}

// mustWrite writes w to path, creating the path if necessary and replacing any
// existing file at that location.
func mustWrite(w io.WriterTo, path string) {
	start := time.Now()
	f, err := os.Create(path)
	if err != nil {
		log.Fatalf("Creating output file: %v", err)
	}
	if _, err := w.WriteTo(f); err != nil {
		log.Fatalf("Writing output file: %v", err)
	} else if err := f.Close(); err != nil {
		log.Fatalf("Closing output file: %v", err)
	}
	log.Printf("Finished writing output [%v elapsed]", time.Since(start))
}

// mustLoadAction loads and parses a wire-format ExtraActionInfo message from
// the specified path, or aborts the program with an error.
func mustLoadAction(path string) *xapb.ExtraActionInfo {
	xa, err := ioutil.ReadFile(path)
	if err != nil {
		log.Fatalf("Reading extra action info: %v", err)
	}
	var info xapb.ExtraActionInfo
	if err := proto.Unmarshal(xa, &info); err != nil {
		log.Fatalf("Parsing extra action info: %v", err)
	}
	log.Printf("Read %d bytes from extra action file %q", len(xa), path)
	return &info
}

// mustLoadRules loads and parses the vname mapping rules in path.
// If path == "", this returns nil (no rules).
// In case of error the program is aborted with a log.
func mustLoadRules(path string) vnameutil.Rules {
	if path == "" {
		return nil
	}
	data, err := ioutil.ReadFile(path)
	if err != nil {
		log.Fatalf("Reading vname rules: %v", err)
	}
	rules, err := vnameutil.ParseRules(data)
	if err != nil {
		log.Fatalf("Parsing vname rules: %v", err)
	}
	return rules
}

// mustRegexp parses the given regular expression or exits the program with a
// log containing description.
func mustRegexp(expr, description string) *regexp.Regexp {
	r, err := regexp.Compile(expr)
	if err != nil {
		log.Fatalf("Invalid %s regexp: %v", description, err)
	}
	return r
}
