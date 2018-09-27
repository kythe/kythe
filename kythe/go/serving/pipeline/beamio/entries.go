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

	"kythe.io/kythe/go/storage/stream"

	"github.com/apache/beam/sdks/go/pkg/beam"
	"github.com/apache/beam/sdks/go/pkg/beam/io/filesystem"

	spb "kythe.io/kythe/proto/storage_go_proto"
)

func init() {
	beam.RegisterFunction(readStream)
}

// ReadEntries reads a delimited stream of *spb.Entry messages into a
// PCollection from the given file.  The file can be part of any filesystem
// registered with the beam/io/filesystem package.
func ReadEntries(s beam.Scope, file string) beam.PCollection {
	return beam.ParDo(s, readStream, beam.Create(s, file))
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
