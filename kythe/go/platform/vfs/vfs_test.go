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
	"io/ioutil"
	"os"
	"path/filepath"
	"testing"

	"kythe.io/kythe/go/test/testutil"
)

type walkResult struct {
	Path string
	Info string
}

const testDataDir = "testdata"

var files = []string{
	"a/b/*/c/empty.txt",
	"a/b/c/d/empty.txt",
}

func tempDir() string {
	root := os.Getenv("TEST_TMPDIR")
	if len(root) > 0 {
		return root
	}
	return os.TempDir()
}

func populateFiles(t *testing.T) (string, func()) {
	t.Helper()
	root := filepath.Join(tempDir(), testDataDir)
	for _, file := range files {
		path := filepath.Join(root, filepath.FromSlash(file))
		testutil.FatalOnErrT(t, "unable to create directory", os.MkdirAll(filepath.Dir(path), 0777))
		testutil.FatalOnErrT(t, "unable to write file", ioutil.WriteFile(path, nil, 0777))
	}
	return root, func() {
		testutil.FatalOnErrT(t, "failed to remove test data", os.RemoveAll(root))
	}
}

func TestGlobWalker(t *testing.T) {
	root, cleanup := populateFiles(t)
	defer cleanup()

	byglob, err := collectFiles(&globWalker{LocalFS{}}, root)
	if len(byglob) == 0 {
		t.Fatal("no files found")
	}
	testutil.FatalOnErrT(t, "unable to collect files from glob", err)
	bylocal, err := collectFiles(LocalFS{}, root)
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
