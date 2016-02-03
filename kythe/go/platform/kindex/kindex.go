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

// Package kindex implements an interface to compilation index files which are
// standalone CompilationUnits with all of their required inputs.
//
// Example: Reading an index file.
//   c, err := kindex.Open("path/to/unit.kindex")
//   if err != nil {
//     log.Fatal(err)
//   }
//
// Example: Writing an index file.
//   var buf bytes.Buffer
//   c.WriteTo(&buf)
//
// On disk, a kindex file is a GZip compressed stream of varint-prefixed data
// blocks containing wire-format protobuf messages.  The first message is a
// CompilationUnit, the remaining messages are FileData messages, one for each
// of the required inputs for the CompilationUnit.
//
// These proto messages are defined in //kythe/proto:analysis_proto
package kindex

import (
	"bytes"
	"compress/gzip"
	"crypto/sha256"
	"encoding/hex"
	"fmt"
	"io"
	"os"
	"sync"

	"kythe.io/kythe/go/platform/analysis"
	"kythe.io/kythe/go/platform/delimited"
	"kythe.io/kythe/go/platform/vfs"

	apb "kythe.io/kythe/proto/analysis_proto"

	"github.com/golang/protobuf/proto"
	"golang.org/x/net/context"
)

// Extension is the standard file extension for Kythe compilation index files.
const Extension = ".kindex"

// Compilation is a CompilationUnit with the contents for all of its required inputs.
type Compilation struct {
	Proto *apb.CompilationUnit `json:"compilation"`
	Files []*apb.FileData      `json:"files"`
}

// Open opens a kindex file at the given path (using vfs.Open) and reads
// its contents into memory.
func Open(ctx context.Context, path string) (*Compilation, error) {
	f, err := vfs.Open(ctx, path)
	if err != nil {
		return nil, err
	}
	defer f.Close()
	return New(f)
}

// New reads a kindex file from r, which is expected to be positioned at the
// beginning of an index file or a data source of equivalent format.
func New(r io.Reader) (*Compilation, error) {
	gz, err := gzip.NewReader(r)
	if err != nil {
		return nil, err
	}
	rd := delimited.NewReader(gz)

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

	return &Compilation{
		Proto: cu,
		Files: files,
	}, nil
}

// Fetch implements the analysis.Fetcher interface for files attached to c.
// If digest == "", files are matched by path only.
func (c *Compilation) Fetch(path, digest string) ([]byte, error) {
	for _, f := range c.Files {
		info := f.GetInfo()
		fp := info.Path
		fd := info.Digest
		if path == fp && (digest == "" || digest == fd) {
			return f.Content, nil
		}
		if digest != "" && digest == fd {
			return f.Content, nil
		}
	}
	return nil, os.ErrNotExist
}

// WriteTo implements the io.WriterTo interface, writing the contents of the
// Compilation in index file format.  Returns the total number of bytes written
// after GZip compression was applied.
func (c *Compilation) WriteTo(w io.Writer) (int64, error) {
	gz, err := gzip.NewWriterLevel(w, gzip.BestCompression)
	if err != nil {
		return 0, err
	}
	w = delimited.NewWriter(gz)

	buf := proto.NewBuffer(nil)
	if err := buf.Marshal(c.Proto); err != nil {
		gz.Close()
		return 0, fmt.Errorf("marshalling compilation: %v", err)
	}

	var total int64

	nw, err := w.Write(buf.Bytes())
	total += int64(nw)
	if err != nil {
		gz.Close()
		return total, fmt.Errorf("writing compilation: %v", err)
	}

	for _, file := range c.Files {
		buf.Reset()
		if err := buf.Marshal(file); err != nil {
			gz.Close()
			return total, fmt.Errorf("marshaling file data: %v", err)
		}
		nw, err := w.Write(buf.Bytes())
		total += int64(nw)
		if err != nil {
			gz.Close()
			return total, fmt.Errorf("writing file data: %v", err)
		}
	}
	if err := gz.Close(); err != nil {
		return total, err
	}
	return total, nil
}

// FromUnit creates a compilation index by fetching all the required inputs of
// unit from f.
func FromUnit(unit *apb.CompilationUnit, f analysis.Fetcher) (*Compilation, error) {
	errc := make(chan error)
	inputs := make([]*apb.FileData, len(unit.RequiredInput))
	var wg sync.WaitGroup
	for i := range unit.RequiredInput {
		wg.Add(1)
		go func(i int) {
			defer wg.Done()

			in := unit.RequiredInput[i].GetInfo()
			data, err := f.Fetch(in.Path, in.Digest)
			if err != nil {
				errc <- err
				return
			}
			inputs[i] = &apb.FileData{
				Content: data,
				Info: &apb.FileInfo{
					Path:   in.Path,
					Digest: in.Digest,
				},
			}
		}(i)
	}
	go func() { wg.Wait(); close(errc) }()
	var err error
	for e := range errc {
		if err == nil {
			err = e
		}
	}
	if err != nil {
		return nil, err
	}
	return &Compilation{
		Proto: unit,
		Files: inputs,
	}, nil
}

// FileData creates a file data protobuf message by fully reading the contents
// of r, having the designated path.
func FileData(path string, r io.Reader) (*apb.FileData, error) {
	var buf bytes.Buffer
	hash := sha256.New()

	w := io.MultiWriter(&buf, hash)
	if _, err := io.Copy(w, r); err != nil {
		return nil, err
	}
	digest := hex.EncodeToString(hash.Sum(nil))
	return &apb.FileData{
		Content: buf.Bytes(),
		Info: &apb.FileInfo{
			Path:   path,
			Digest: digest,
		},
	}, nil
}
