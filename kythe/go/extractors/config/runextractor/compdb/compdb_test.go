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

package compdb

import (
	"context"
	"os"
	"path/filepath"
	"reflect"
	"testing"

	"kythe.io/kythe/go/platform/kzip"
)

const (
	testPath      = "kythe/go/extractors/config/runextractor/compdb"
	extractorPath = "kythe/cxx/extractor/cxx_extractor"
	workspace     = "io_kythe"
)

// setupEnvironment establishes the necessary environment variables and
// current working directory for running tests.
// Returns expected working directory.
func setupEnvironment(t *testing.T) string {
	t.Helper()
	// TODO(shahms): ExtractCompilations should take an output path.
	if _, ok := os.LookupEnv("TEST_TMPDIR"); !ok {
		t.Skip("Skipping test due to incompatible environment (missing TEST_TMPDIR)")
	}
	output := t.TempDir()
	if err := os.Setenv("KYTHE_OUTPUT_DIRECTORY", output); err != nil {
		t.Fatalf("Error setting KYTHE_OUTPUT_DIRECTORY: %v", err)
	}
	root, err := os.Getwd()
	if err != nil {
		t.Fatalf("Unable to get working directory: %v", err)
	}
	return root
}

func testExtractCompilationsEndToEndWithDatabase(t *testing.T, compdbPath string) {
	root := setupEnvironment(t)
	defer os.Chdir(root)

	extractor, err := filepath.Abs(extractorPath)
	if err != nil {
		t.Fatalf("Unable to get absolute path to extractor: %v", err)
	}
	// Paths in compilation_database.json are relative to the testdata directory, so change there.
	if err := os.Chdir(filepath.Join(root, testPath, "testdata")); err != nil {
		t.Fatalf("Unable to change working directory: %v", err)
	}
	if err := ExtractCompilations(context.Background(), extractor, compdbPath, nil); err != nil {
		t.Fatalf("Error running ExtractCompilations: %v", err)
	}
	err = filepath.Walk(os.Getenv("KYTHE_OUTPUT_DIRECTORY"), func(path string, info os.FileInfo, err error) error {
		if err != nil {
			return err
		} else if info.IsDir() {
			return nil
		} else if filepath.Ext(path) != ".kzip" {
			t.Logf("Ignoring non-kzip file: %v", path)
			return nil
		}
		reader, err := os.Open(path)
		if err != nil {
			return err
		}
		defer reader.Close()
		err = kzip.Scan(reader, func(r *kzip.Reader, unit *kzip.Unit) error {
			if !reflect.DeepEqual(unit.Proto.SourceFile, []string{"test_file.cc"}) {
				t.Fatalf("Invalid source_file: %v", unit.Proto.SourceFile)
			}
			return nil
		})
		if err != nil {
			return err
		}
		return nil
	})
	if err != nil {
		t.Fatalf("Error processing extracted output: %v", err)
	}

}

func TestExtractCompilationsEndToEnd(t *testing.T) {
	t.Run("command", func(t *testing.T) {
		testExtractCompilationsEndToEndWithDatabase(t, "compilation_database.json")
	})
	t.Run("arguments", func(t *testing.T) {
		testExtractCompilationsEndToEndWithDatabase(t, "compilation_database_arguments.json")
	})
}
