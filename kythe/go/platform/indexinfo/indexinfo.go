/*
 * Copyright 2014 Google Inc. All rights reserved.
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

// Package indexinfo implements an interface to compilation index files which
// are standalone CompilationUnits with all of their required inputs.
//
// Example:
//   idx, err := indexfile.Open("path/to/unit.kindex")
//   if err != nil {
//     log.Fatal(err)
//   }
//
//   // Compilation also satisfies io.WriterTo so it can be written back out.
//   var buf bytes.Buffer
//   idx.WriteTo(&buf)
//
// On disk, an index file is a GZip compressed stream of varint-prefixed data
// blocks containing wire-format protocol messages.  The first message is a
// CompilationUnit, the remaining messages are FileData messages, one for each
// of the required inputs for the CompilationUnit.
//
// These proto messages are defined in //kythe/proto:analysis_proto
package indexinfo

import (
	"compress/gzip"
	"errors"
	"fmt"
	"io"
	"os"
	"sync"

	"kythe/go/platform/analysis"
	"kythe/go/platform/delimited"

	apb "kythe/proto/analysis_proto"

	"code.google.com/p/goprotobuf/proto"
)

// Standard file extension for Kythe indexinfo files.
const FileExt = ".kindex"

// ErrCorruptIndex is returned by New when the index file is corrupt.
var ErrCorruptIndex = errors.New("corrupt index file")

// IndexInfo is a CompilationUnit with the contents for all of its required inputs.
type IndexInfo struct {
	Compilation *apb.CompilationUnit

	Files []*apb.FileData
}

// Open creates an IndexInfo associated with the given path, by opening the
// file at that path and reading its contents into memory.
func Open(path string) (*IndexInfo, error) {
	f, err := os.Open(path)
	if err != nil {
		return nil, err
	}
	defer f.Close()
	return New(f)
}

// New creates an IndexInfo associated with the given io.Reader, which is
// expected to be positioned at the beginning of an index file or a data source
// of equivalent format.
func New(r io.Reader) (*IndexInfo, error) {
	var rd *delimited.Reader
	if gz, err := gzip.NewReader(r); err != nil {
		return nil, err
	} else {
		rd = delimited.NewReader(gz)
	}

	// The first block is the CompilationUnit message.
	cu := new(apb.CompilationUnit)
	if rec, err := rd.Next(); err != nil {
		return nil, err
	} else if err := proto.Unmarshal(rec, cu); err != nil {
		return nil, err
	}

	// All the subsequent blocks are FileData messages.
	var files []*apb.FileData
	for {
		rec, err := rd.Next()
		if err == io.EOF {
			break
		} else if err != nil {
			return nil, err
		}

		fd := new(apb.FileData)
		if err := proto.Unmarshal(rec, fd); err != nil {
			return nil, err
		}
		files = append(files, fd)
	}

	return &IndexInfo{
		Compilation: cu,
		Files:       files,
	}, nil
}

// WriteTo implements the io.WriterTo interface, writing the contents of the
// IndexInfo in index file format.  Returns the total number of bytes written
// after GZip compression was applied.
func (idx *IndexInfo) WriteTo(w io.Writer) (int64, error) {
	gz, err := gzip.NewWriterLevel(w, gzip.BestCompression)
	if err != nil {
		return 0, err
	}
	defer gz.Close()
	w = delimited.NewWriter(gz)

	cu, err := proto.Marshal(idx.Compilation)
	if err != nil {
		return 0, fmt.Errorf("error marshalling CompilationUnit: %v", err)
	}

	var written int64
	n, err := w.Write(cu)
	written += int64(n)
	if err != nil {
		return written, fmt.Errorf("error writing CompilationUnit: %v", err)
	}

	for _, file := range idx.Files {
		fd, err := proto.Marshal(file)
		if err != nil {
			return written, fmt.Errorf("error marshalling %#v: %v", file, err)
		}

		m, err := w.Write(fd)
		written += int64(m)
		if err != nil {
			return written, fmt.Errorf("error writing FileData: %v", err)
		}
	}

	return written, nil
}

// FromCompilation creates an index info from an arbitrary
// CompilationUnit and an analysis.Fetcher.  This requires fetching the input
// compilation's required files, so it may produce an error; in that case a
// partial compilation is also returned.
func FromCompilation(c *apb.CompilationUnit, f analysis.Fetcher) (*IndexInfo, error) {
	index := &IndexInfo{Compilation: c}

	var wg sync.WaitGroup
	type result struct {
		input *apb.CompilationUnit_FileInput
		data  []byte
		error
	}

	results := make(chan result)
	wg.Add(len(index.Compilation.RequiredInput))
	for _, input := range index.Compilation.RequiredInput {
		go func(input *apb.CompilationUnit_FileInput) {
			data, err := f.Fetch(input.GetInfo().GetPath(), input.GetInfo().GetDigest())
			if err != nil {
				results <- result{error: err}
			}
			results <- result{input: input, data: data}
			wg.Done()
		}(input)
	}
	go func() {
		wg.Wait()
		close(results)
	}()

	var lastError error
	for r := range results {
		if r.error != nil {
			lastError = r.error
		} else {
			index.Files = append(index.Files, &apb.FileData{
				Content: r.data,
				Info:    &apb.FileInfo{Path: r.input.GetInfo().Path, Digest: r.input.GetInfo().Digest},
			})
		}
	}

	return index, lastError
}
