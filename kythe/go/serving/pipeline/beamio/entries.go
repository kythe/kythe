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

package beamio

import (
	"context"
	"fmt"
	"io"
	"strings"

	"kythe.io/kythe/go/storage/stream"
	"kythe.io/kythe/go/util/riegeli"

	"github.com/apache/beam/sdks/go/pkg/beam"
	"github.com/apache/beam/sdks/go/pkg/beam/io/filesystem"

	spb "kythe.io/kythe/proto/storage_go_proto"
)

func init() {
	beam.RegisterFunction(readFile)
}

// ReadEntries reads a set of *spb.Entry messages into a PCollection from the
// given file, or files stored in a directory.  The file can be part of any
// filesystem registered with the beam/io/filesystem package and can either be a
// delimited protobuf stream or a Riegeli file.
func ReadEntries(ctx context.Context, s beam.Scope, fileOrDir string) (beam.PCollection, error) {
	if strings.HasSuffix(fileOrDir, "/") {
		var errv beam.PCollection
		fs, err := filesystem.New(ctx, fileOrDir)
		if err != nil {
			return errv, err
		}
		defer fs.Close()
		files, err := fs.List(ctx, fileOrDir+"*")
		if err != nil {
			return errv, err
		}
		if len(files) == 0 {
			return errv, fmt.Errorf("no entries found in %s - maybe mistyped path?", fileOrDir)
		}
		return beam.ParDo(s, readFile, beam.CreateList(s, files)), nil
	}
	return beam.ParDo(s, readFile, beam.Create(s, fileOrDir)), nil
}

func readFile(ctx context.Context, file string, emit func(*spb.Entry)) error {
	if isRiegeli(ctx, file) {
		return readRiegeli(ctx, file, emit)
	}
	return readStream(ctx, file, emit)
}

func isRiegeli(ctx context.Context, file string) bool {
	fs, err := filesystem.New(ctx, file)
	if err != nil {
		return false
	}
	defer fs.Close()
	f, err := fs.OpenRead(ctx, file)
	if err != nil {
		return false
	}
	defer f.Close()
	rd := riegeli.NewReader(f)
	if _, err := rd.RecordsMetadata(); err != nil {
		return false
	}
	return true
}

func readStream(ctx context.Context, filename string, emit func(*spb.Entry)) error {
	fs, err := filesystem.New(ctx, filename)
	if err != nil {
		return err
	}
	f, err := fs.OpenRead(ctx, filename)
	if err != nil {
		return err
	}
	defer f.Close()
	for e := range stream.ReadEntries(f) {
		emit(e)
	}
	return fs.Close()
}

func readRiegeli(ctx context.Context, filename string, emit func(*spb.Entry)) error {
	fs, err := filesystem.New(ctx, filename)
	if err != nil {
		return err
	}
	defer fs.Close()
	f, err := fs.OpenRead(ctx, filename)
	if err != nil {
		return err
	}
	defer f.Close()

	rd := riegeli.NewReader(f)
	for {
		var e spb.Entry
		if err := rd.NextProto(&e); err == io.EOF {
			return nil
		} else if err != nil {
			return err
		}
		emit(&e)
	}
}
