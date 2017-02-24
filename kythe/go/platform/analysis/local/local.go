/*
 * Copyright 2015 Google Inc. All rights reserved.
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

// Package local implements CompilationAnalyzer utilities for local analyses.
package local

import (
	"context"
	"fmt"
	"io"

	"kythe.io/kythe/go/platform/analysis"
	"kythe.io/kythe/go/platform/analysis/driver"
	"kythe.io/kythe/go/platform/kindex"
)

// KIndexQueue is a driver.Queue reading each compilation from a .kindex file.
// On each call to the driver.CompilationFunc, KIndexQueue's analysis.Fetcher
// interface exposes the .kindex archive's file contents.
type KIndexQueue struct {
	analysis.Fetcher

	index int
	paths []string

	// TODO(fromberger): Support an optional revision marker.
}

// NewKIndexQueue returns a new KIndexQueue over the given paths to .kindex
// files.
func NewKIndexQueue(paths []string) *KIndexQueue { return &KIndexQueue{paths: paths} }

// Next implements the driver.Queue interface.
func (k *KIndexQueue) Next(ctx context.Context, f driver.CompilationFunc) error {
	if k.index >= len(k.paths) {
		return io.EOF
	}

	path := k.paths[k.index]
	k.index++

	cu, err := kindex.Open(ctx, path)
	if err != nil {
		return fmt.Errorf("error opening kindex file at %q: %v", path, err)
	}

	k.Fetcher = cu
	err = f(ctx, driver.Compilation{
		Unit: cu.Proto,
	})
	k.Fetcher = nil

	return err
}
