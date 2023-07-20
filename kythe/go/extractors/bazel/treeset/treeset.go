/*
 * Copyright 2021 The Kythe Authors. All rights reserved.
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

// Package treeset provides functions for extracting targets that use bazel treesets.
package treeset

import (
	"context"
	"os"
	"path/filepath"

	"kythe.io/kythe/go/platform/vfs"
	"kythe.io/kythe/go/util/log"

	"bitbucket.org/creachadair/stringset"
)

// ListSources returns the source files underneath path. If path is a file, it returns a set with
// a single element. If path is a directory, it returns a set containing all the files
// (recursively) under path.
func ListSources(ctx context.Context, arg string) (stringset.Set, error) {
	fi, err := vfs.Stat(ctx, arg)
	if err != nil {
		return nil, err
	}
	if fi.IsDir() {
		s := stringset.New()
		if err := filepath.Walk(arg, func(path string, info os.FileInfo, err error) error {
			if err != nil {
				return err
			}
			if !info.IsDir() {
				s.Add(path)
			}
			return nil
		}); err != nil {
			return nil, err
		}
		return s, nil
	}
	return stringset.New(arg), nil
}

// FindMissingTreeInputs returns the required files that are not explicitly listed in bazel's
// inputs because they were tree artifacts.
//
// Some bazel rules use tree artifacts for the inputs to the compiler. These are directories that
// Bazel expands to files when the action is run. Consequently, the list of inputs that Bazel has
// only contains the tree artifact directory. This function reports the files that are (1) required,
// (2) not included in bazel's inputs, and (3) have their parent directory included in Bazel's
// inputs.
func FindMissingTreeInputs(inputs []string, requiredFiles stringset.Set) []string {
	missingInputs := stringset.New()
	inputsSet := stringset.New(inputs...)
	for file := range requiredFiles {
		if inputsSet.Contains(file) {
			continue
		}
		dir := filepath.Dir(file)
		for dir != "" {
			if inputsSet.Contains(dir) {
				missingInputs.Add(file)
				break
			}
			dir = filepath.Dir(dir)
		}
		if dir == "" {
			log.Warningf("couldn't find an input for %s\n", file)
		}

	}
	return missingInputs.Elements()
}

// ExpandDirectories returns the list of files contained in the provided paths.
//
// Paths can be individual files or directories. If it's a directory - all files
// within that directory added to the result.
func ExpandDirectories(ctx context.Context, paths []string) []string {
	var nps []string
	for _, root := range paths {
		files, err := ListSources(ctx, root)
		if err != nil {
			log.Warningf("couldn't list files for %s: %s\n", root, err)
			nps = append(nps, root)
		} else {
			nps = append(nps, files.Elements()...)
		}
	}

	return nps
}
