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
	"path/filepath"

	"kythe.io/kythe/go/platform/analysis"
	"kythe.io/kythe/go/platform/analysis/driver"
	"kythe.io/kythe/go/platform/kindex"
)

// An Option controls the behaviour of a KIndexQueue.
type Option func(*KIndexQueue)

// KIndexQueue is a driver.Queue reading each compilation from a .kindex file.
// On each call to the driver.CompilationFunc, KIndexQueue's analysis.Fetcher
// interface exposes the .kindex archive's file contents.
type KIndexQueue struct {
	analysis.Fetcher

	index    int      // the next index to consume from paths
	paths    []string // the paths of kindex files to read
	revision string   // the revision marker to attribute to each compilation
}

// Revision returns an option that sets the revision marker on compilations
// delivered by a KIndexQueue.
func Revision(rev string) Option {
	return func(k *KIndexQueue) { k.revision = rev }
}

// NewKIndexQueue returns a new KIndexQueue over the given paths to .kindex
// files.
func NewKIndexQueue(paths []string, opts ...Option) *KIndexQueue {
	q := &KIndexQueue{paths: paths}
	for _, opt := range opts {
		opt(q)
	}
	return q
}

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
		Unit:       cu.Proto,
		Revision:   k.revision,
		UnitDigest: filepath.Base(path),
	})
	k.Fetcher = nil

	return err
}
