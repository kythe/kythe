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

// Package extutil implements shared code for extracting and writing output
// from Bazel actions, either to .kindex or a .kzip files. This is a temporary
// measure to support migrating to .kzip output.
package extutil // import "kythe.io/kythe/go/extractors/bazel/extutil"

import (
	"context"
	"fmt"
	"path/filepath"

	"kythe.io/kythe/go/extractors/bazel"
)

// ExtractAndWrite extracts a spawn action through c and writes the results to
// the specified output file. The output format is based on the file extension:
//
//   .kindex   -- writes a kindex file
//   .kzip     -- writes a kzip file
//   otherwise -- reports an error
//
func ExtractAndWrite(ctx context.Context, c *bazel.Config, ai *bazel.ActionInfo, outputPath string) error {
	switch ext := filepath.Ext(outputPath); ext {
	case ".kindex":
		cu, err := c.Extract(ctx, ai)
		if err != nil {
			return fmt.Errorf("extracting: %v", err)
		}
		if err := bazel.Write(cu, outputPath); err != nil {
			return fmt.Errorf("writing kindex: %v", err)
		}

	case ".kzip":
		w, err := bazel.NewKZIP(outputPath)
		if err != nil {
			return fmt.Errorf("creating kzip writer: %v", err)
		}
		if _, err := c.ExtractToFile(ctx, ai, w); err != nil {
			return fmt.Errorf("extracting: %v", err)
		}
		if err := w.Close(); err != nil {
			return fmt.Errorf("closing output: %v", err)
		}

	default:
		return fmt.Errorf("unknown output extension %q", ext)
	}
	return nil
}
