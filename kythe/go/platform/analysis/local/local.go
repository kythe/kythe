/*
 * Copyright 2015 The Kythe Authors. All rights reserved.
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
package local // import "kythe.io/kythe/go/platform/analysis/local"

import (
	"context"
	"errors"
	"fmt"
	"io"
	"path/filepath"

	"kythe.io/kythe/go/platform/analysis"
	"kythe.io/kythe/go/platform/analysis/driver"
	"kythe.io/kythe/go/platform/kzip"
	"kythe.io/kythe/go/platform/vfs"
	"kythe.io/kythe/go/util/log"

	apb "kythe.io/kythe/proto/analysis_go_proto"
)

// Options control the behaviour of a FileQueue.
type Options struct {
	// The revision marker to attribute to each compilation.
	Revision string
}

func (o *Options) revision() string {
	if o == nil {
		return ""
	}
	return o.Revision
}

// A FileQueue is a driver.Queue reading each compilation from a sequence of
// .kzip files.  On each call to the driver.CompilationFunc, the
// FileQueue's analysis.Fetcher interface exposes the current file's contents.
type FileQueue struct {
	index    int                    // the next index to consume from paths
	paths    []string               // the paths of kzip files to read
	units    []*apb.CompilationUnit // units waiting to be delivered
	revision string                 // revision marker for each compilation

	fetcher analysis.Fetcher
	closer  io.Closer
}

// NewFileQueue returns a new FileQueue over the given paths to .kzip files.
func NewFileQueue(paths []string, opts *Options) *FileQueue {
	return &FileQueue{
		paths:    paths,
		revision: opts.revision(),
	}
}

// Next implements the driver.Queue interface.
func (q *FileQueue) Next(ctx context.Context, f driver.CompilationFunc) error {
	for len(q.units) == 0 {
		if q.closer != nil {
			q.closer.Close()
			q.closer = nil
		}
		if q.index >= len(q.paths) {
			return driver.ErrEndOfQueue
		}

		path := q.paths[q.index]
		q.index++
		switch filepath.Ext(path) {
		case ".kzip":
			f, err := vfs.Open(ctx, path)
			if err != nil {
				return fmt.Errorf("opening kzip file %q: %v", path, err)
			}
			rc, ok := f.(kzip.File)
			if !ok {
				f.Close()
				return fmt.Errorf("reader %T does not implement kzip.File", rc)
			}
			if err := kzip.Scan(rc, func(r *kzip.Reader, unit *kzip.Unit) error {
				q.fetcher = kzipFetcher{r}
				q.units = append(q.units, unit.Proto)
				return nil
			}); err != nil {
				f.Close()
				return fmt.Errorf("scanning kzip %q: %v", path, err)
			}
			q.closer = f

		default:
			log.Warningf("Skipped unknown file kind: %q", path)
			continue
		}
	}

	// If we get here, we have at least one more compilation in the queue.
	next := q.units[0]
	q.units = q.units[1:]
	return f(ctx, driver.Compilation{
		Unit:     next,
		Revision: q.revision,
	})
}

// Fetch implements the analysis.Fetcher interface by delegating to the
// currently-active input file. Only files in the current archive will be
// accessible for a given invocation of Fetch.
func (q *FileQueue) Fetch(path, digest string) ([]byte, error) {
	if q.fetcher == nil {
		return nil, errors.New("no data source available")
	}
	return q.fetcher.Fetch(path, digest)
}

type kzipFetcher struct{ r *kzip.Reader }

// Fetch implements the required method of analysis.Fetcher.
func (k kzipFetcher) Fetch(_, digest string) ([]byte, error) { return k.r.ReadAll(digest) }
