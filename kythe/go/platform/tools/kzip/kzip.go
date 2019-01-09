/*
 * Copyright 2019 The Kythe Authors. All rights reserved.
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

// Binary kzip provides tools to work with kzip archives.
//
// Examples:
//   # Merge 5 kzip archives into a single file.
//   kzip merge --output output.kzip in{0,1,2,3,4}.kzip
package main

import (
	"context"
	"flag"
	"fmt"
	"log"
	"os"

	"kythe.io/kythe/go/platform/kzip"
	"kythe.io/kythe/go/platform/vfs"
	"kythe.io/kythe/go/util/flagutil"

	"bitbucket.org/creachadair/stringset"
)

func init() {
	flag.Usage = flagutil.SimpleUsage("Utility to modify/create kzip archives",
		"merge --output path kzip-file*")
	// TODO(schroederc): other subcommands
}

var output = flag.String("output", "", "Path to output kzip file")

func main() {
	flag.Parse()

	if flag.Arg(0) != "merge" {
		log.Printf("ERROR: unknown subcommand: %q", flag.Arg(0))
		flag.Usage()
		os.Exit(2)
	}

	if err := flag.CommandLine.Parse(flag.Args()[1:]); err != nil {
		log.Fatal(err)
	}
	if *output == "" {
		log.Println("ERROR: required --output path missing")
		flag.Usage()
		os.Exit(2)
	}

	if err := mergeArchives(context.Background(), *output, flag.Args()); err != nil {
		log.Fatalf("Error merging archives: %v", err)
	}
}

func mergeArchives(ctx context.Context, output string, archives []string) error {
	out, err := vfs.Create(ctx, output)
	if err != nil {
		return fmt.Errorf("error creating output: %v", err)
	}
	wr, err := kzip.NewWriteCloser(out)
	if err != nil {
		out.Close()
		return fmt.Errorf("error creating writer: %v", err)
	}

	filesAdded := stringset.New()
	for _, path := range archives {
		if err := mergeInto(wr, path, filesAdded); err != nil {
			wr.Close()
			return err
		}
	}

	if err := wr.Close(); err != nil {
		return fmt.Errorf("error closing writer: %v", err)
	}
	return nil
}

func mergeInto(wr *kzip.Writer, path string, filesAdded stringset.Set) error {
	f, err := os.Open(path)
	if err != nil {
		return fmt.Errorf("error opening archive: %v", err)
	}
	defer f.Close()

	stat, err := f.Stat()
	if err != nil {
		return err
	}
	size := stat.Size()
	if size == 0 {
		log.Printf("Skipping empty .kzip: %s", path)
		return nil
	}

	rd, err := kzip.NewReader(f, size)
	if err != nil {
		return fmt.Errorf("error creating reader: %v", err)
	}

	return rd.Scan(func(u *kzip.Unit) error {
		for _, ri := range u.Proto.RequiredInput {
			if filesAdded.Add(ri.Info.Digest) {
				r, err := rd.Open(ri.Info.Digest)
				if err != nil {
					return err
				}
				defer r.Close()
				if _, err := wr.AddFile(r); err != nil {
					return err
				}
				return nil
			}
		}
		// TODO(schroederc): duplicate compilations with different revisions
		_, err = wr.AddUnit(u.Proto, u.Index)
		return err
	})
}
