/*
 * Copyright 2020 The Kythe Authors. All rights reserved.
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

// Program copy_kzip implements a Bazel extra action that directly copies a Kythe
// compilation record produced by a "spawn" action.
package main

import (
	"context"
	"flag"
	"io"
	"time"

	"kythe.io/kythe/go/extractors/bazel"
	"kythe.io/kythe/go/platform/vfs"
	"kythe.io/kythe/go/util/log"
)

var (
	outputPath  = flag.String("output", "", "Path of output index file (required)")
	extraAction = flag.String("extra_action", "", "Path of the SpawnAction file (required)")
)

func main() {
	flag.Parse()

	// Verify that required flags are set.
	if *outputPath == "" {
		log.Fatal("You must provide a non-empty --output file path")
	}
	if *extraAction == "" {
		log.Fatal("You must provide a non-empty --extra_action file path")
	}

	info, err := bazel.LoadAction(*extraAction)
	if err != nil {
		log.Fatalf("Unable to load extra action: %v", err)
	}
	start := time.Now()
	ai, err := bazel.SpawnAction(info)
	if err != nil {
		log.Fatalf("Unable to read SpawnInfo: %v", err)
	}
	if len(ai.Outputs) != 1 {
		log.Fatalf("Can only copy a single output kzip, not %d", len(ai.Outputs))
	}

	if err := copyFile(ai.Outputs[0], *outputPath); err != nil {
		log.Fatalf("Unable to copy input to output: %v", err)
	}

	log.Infof("Finished extracting [%v elapsed]", time.Since(start))
}

func copyFile(input, output string) error {
	ctx := context.Background()
	f, err := vfs.Open(ctx, input)
	if err != nil {
		return err
	}
	defer f.Close()

	o, err := vfs.Create(ctx, output)
	if err != nil {
		return err
	}

	if _, err := io.Copy(o, f); err != nil {
		o.Close()
		return err
	}

	return o.Close()
}
