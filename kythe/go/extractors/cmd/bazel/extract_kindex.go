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
	"log"
	"os"
	"path/filepath"
	"regexp"
	"time"

	"bitbucket.org/creachadair/stringset"

	"kythe.io/kythe/go/extractors/bazel"
	"kythe.io/kythe/go/platform/kindex"
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
	scopedSource = flag.Bool("scoped", false, "Only match source paths within the target package")
	verboseLog   = flag.Bool("v", false, "Enable verbose (per-file) logging")
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

If -scoped is true, a file selected by matching the -source regexp will only be
considered as a source input if it also contains the Bazel package name as a
substring. For example, when building //foo/bar/baz:quux with -source '\.go$'
and -scoped set, a source file like "workdir/blah/foo/bar/baz/my/file.go" will
be a source input, but "workdir/blah/zip/zip/my/file.go" will not.

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

	// Ensure the extra action can be loaded.
	info, err := bazel.LoadAction(*extraAction)
	if err != nil {
		log.Fatalf("Loading extra action: %v", err)
	}
	pkg := bazel.PackageName(info.GetOwner())
	log.Printf("Extra action for target %q (package %q)", info.GetOwner(), pkg)

	rules, err := bazel.LoadRules(*vnameRules)
	if err != nil {
		log.Fatalf("Loading rules: %v", err)
	}
	config := &bazel.Config{
		Corpus:     *corpus,
		Language:   *language,
		Rules:      rules,
		Verbose:    *verboseLog,
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
		if *scopedSource {
			config.IsSource = func(path string) bool {
				return r.MatchString(path) && bazel.PathInPackage(path, pkg)
			}
		} else {
			config.IsSource = r.MatchString
		}
	}

	// If we have been asked to include source files from the argument list,
	// add a fixup to do that at the end.
	if *sourceArgs != "" {
		r := mustRegexp(*sourceArgs, "source argument")
		config.Fixup = func(cu *kindex.Compilation) error {
			var inputs stringset.Set
			for _, ri := range cu.Proto.RequiredInput {
				inputs.Add(ri.Info.GetPath())
			}

			srcs := stringset.New(cu.Proto.SourceFile...)
			for _, arg := range cu.Proto.Argument {
				if r.MatchString(arg) && inputs.Contains(arg) {
					srcs.Add(arg)
				}
			}
			cu.Proto.SourceFile = srcs.Elements()
			return nil
		}
	}

	start := time.Now()
	cu, err := config.Extract(context.Background(), info)
	if err != nil {
		log.Fatalf("Extraction failed: %v", err)
	}
	log.Printf("Finished extracting [%v elapsed]", time.Since(start))
	if err := bazel.Write(cu, *outputPath); err != nil {
		log.Fatalf("Writing index: %v", err)
	}
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
