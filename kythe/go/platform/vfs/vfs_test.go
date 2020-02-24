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

package vfs

import (
	"context"
	"os"
	"testing"

	"kythe.io/kythe/go/test/testutil"
)

const testDataDir = "testdata"

type walkResult struct {
	Path string
	Info string
}

func TestGlobWalker(t *testing.T) {
	byglob, err := collectFiles(&globWalker{LocalFS{}}, testutil.TestFilePath(t, testDataDir))
	testutil.FatalOnErrT(t, "unable to collect files from glob", err)
	bylocal, err := collectFiles(LocalFS{}, testutil.TestFilePath(t, testDataDir))
	testutil.FatalOnErrT(t, "unable to collect local files", err)
	testutil.FatalOnErrT(t, "local and glob walks differ", testutil.DeepEqual(bylocal, byglob))
}

func collectFiles(walk Walker, root string) ([]walkResult, error) {
	var results []walkResult
	return results, walk.Walk(context.Background(), root, func(path string, info os.FileInfo, err error) error {
		if err != nil {
			return err
		}
		results = append(results, walkResult{path, info.Name()})
		return nil
	})
}
