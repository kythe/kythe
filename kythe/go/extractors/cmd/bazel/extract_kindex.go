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
	"time"

	"kythe.io/kythe/go/extractors/bazel"
)

var (
	outputPath = flag.String("output", "", "Path of output index file (required)")

	settings bazel.Settings
)

func init() {
	settings.SetFlags(nil, "")

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
	if *outputPath == "" {
		log.Fatal("You must provide a non-empty --output file path")
	}

	config, info, err := bazel.NewFromSettings(settings)
	if err != nil {
		log.Fatalf("Invalid config settings: %v", err)
	}

	start := time.Now()
	ai, err := bazel.SpawnAction(info)
	if err != nil {
		log.Fatalf("Invalid extra action: %v", err)
	}
	cu, err := config.Extract(context.Background(), ai)
	if err != nil {
		log.Fatalf("Extraction failed: %v", err)
	}
	log.Printf("Finished extracting [%v elapsed]", time.Since(start))
	if err := bazel.Write(cu, *outputPath); err != nil {
		log.Fatalf("Writing index: %v", err)
	}
}
