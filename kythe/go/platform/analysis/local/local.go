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
package local

import (
	"bytes"
	"context"
	"errors"
	"fmt"
	"io"
	"log"
	"os"
	"os/exec"
	"path/filepath"
	"strings"

	"kythe.io/kythe/go/platform/analysis"
	"kythe.io/kythe/go/platform/analysis/driver"
	"kythe.io/kythe/go/platform/delimited"
	"kythe.io/kythe/go/platform/kindex"
	"kythe.io/kythe/go/platform/kzip"
	"kythe.io/kythe/go/platform/vfs"

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
// .kzip and .kindex files.  On each call to the driver.CompilationFunc, the
// FileQueue's analysis.Fetcher interface exposes the current file's contents.
type FileQueue struct {
	index    int                    // the next index to consume from paths
	paths    []string               // the paths of kindex files to read
	units    []*apb.CompilationUnit // units waiting to be delivered
	revision string                 // revision marker for each compilation

	fetcher analysis.Fetcher
	closer  io.Closer
}

// NewFileQueue returns a new FileQueue over the given paths to .kzip or
// .kindex files.
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
		case ".kindex":
			cu, err := kindex.Open(ctx, path)
			if err != nil {
				return fmt.Errorf("opening kindex file %q: %v", path, err)
			}
			q.fetcher = cu
			q.closer = nil // nothing to close in this case
			q.units = append(q.units, cu.Proto)
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
			log.Printf("Warning: Skipped unknown file kind: %q", path)
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

// localAnalyzer provides an implementation of the analysis.CompilationAnalyzer
// interface that works by saving the compilation unit to a temporary kzip file
// and invoking an indexer binary on it.
type localAnalyzer struct {
	cmd     []string
	fetcher analysis.Fetcher
}

// NewLocalAnalyzer creates an analyzer that will execute the given command when
// Analyze() is called. The filepath to a kzip is appended to the command, so if
// given ["my_indexer", "--index"], the final executed command will be
// ["my_indexer", "--index", "some/file.kzip"]. The indexer command should emit
// length-delimited Entry protos to stdout as described in
// //kythe/go/platform/delimited.
func NewLocalAnalyzer(cmd []string, fetcher analysis.Fetcher) (*localAnalyzer, error) {
	if len(cmd) == 0 {
		return nil, fmt.Errorf("parameter 'cmd' must not be empty")
	}
	return &localAnalyzer{cmd: cmd, fetcher: fetcher}, nil
}

// Analyze implements the analysis.CompilationAnalyzer interface.
func (a *localAnalyzer) Analyze(ctx context.Context, req *apb.AnalysisRequest, f analysis.OutputFunc) error {
	tmpkzip, err := saveSingleUnitKzip(ctx, req.Compilation, a.fetcher)
	if err != nil {
		return fmt.Errorf("creating tmp kzip: %v", err)
	}
	defer os.Remove(tmpkzip)

	// Run indexer binary
	var errBuf strings.Builder
	var outBuf bytes.Buffer
	cmd := exec.Command(a.cmd[0], append(a.cmd[1:], tmpkzip)...)
	cmd.Stdout = &outBuf
	cmd.Stderr = &errBuf
	if err = cmd.Run(); err != nil {
		return fmt.Errorf("indexing kzip %s: %v. stderr=%s", tmpkzip, err, errBuf.String())
	}

	// Deserialize output and pass it to `f`.
	rd := delimited.NewReader(&outBuf)
	for {
		n, err := rd.Next()
		if err == io.EOF {
			break
		} else if err != nil {
			return fmt.Errorf("parsing entry stream: %v", err)
		}

		if err := f(ctx, &apb.AnalysisOutput{Value: n}); err != nil {
			return fmt.Errorf("unable to write analysis output: %v", err)
		}
	}
	if err := f(ctx, &apb.AnalysisOutput{FinalResult: &apb.AnalysisResult{Status: apb.AnalysisResult_COMPLETE}}); err != nil {
		return fmt.Errorf("unable to write analysis output: %v", err)
	}

	return nil
}

// saveSingleUnitKzip creates a kzip file containing the given compilation unit
// and any files it depends on. File contents are retrieved from the given
// fetcher object.
func saveSingleUnitKzip(ctx context.Context, cu *apb.CompilationUnit, fetcher analysis.Fetcher) (path string, err error) {
	tmpfile, err := vfs.CreateTempFile(ctx, "", fmt.Sprintf("*_%s.kzip", cu.VName.Language))
	if err != nil {
		return "", err
	}

	w, err := kzip.NewWriteCloser(tmpfile)
	if err != nil {
		return "", err
	}
	defer func() {
		if closeErr := w.Close(); closeErr != nil {
			if err != nil {
				log.Printf("WARNING: unable to close kzip writer: %v", closeErr)
			} else {
				err = closeErr
			}
		}
	}()

	if _, err = w.AddUnit(cu, nil); err != nil {
		return "", err
	}

	for _, file := range cu.RequiredInput {
		b, err := fetcher.Fetch(file.Info.Path, file.Info.Digest)
		if err != nil {
			return "", fmt.Errorf("fetching file %s: %v", file.Info.Path, err)
		}
		if _, err := w.AddFile(bytes.NewReader(b)); err != nil {
			return "", fmt.Errorf("writing file to kzip %s: %v", file.Info.Path, err)
		}
	}

	return tmpfile.Name(), err
}
