/*
 * Copyright 2018 Google Inc. All rights reserved.
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

// Package riegeli implements a Reader and Writer for the Riegeli records
// format.
//
// C++ implementation: https://github.com/google/riegeli
// Format spec: https://github.com/google/riegeli/blob/master/doc/riegeli_records_file_format.md
package riegeli

import (
	"bytes"
	"io"

	"github.com/golang/protobuf/proto"
)

// Defaults for the WriterOptions.
const (
	DefaultChunkSize   = 1 << 20
	DefaultCompression = NoCompression // TODO(schroederc): default to brotli/zstd
)

// CompressionType is the type of compression used for encoding Riegeli chunks.
type CompressionType compressionType

const (
	// NoCompression indicates that no compression will be used to encode chunks.
	NoCompression compressionType = noCompression

	// TODO(schroederc): add brotli compression
	// TODO(schroederc): add zstd compression
)

// WriterOptions customizes the behavior of a Riegeli Writer.
type WriterOptions struct {
	// Desired uncompressed size of a chunk which groups records.
	ChunkSize uint64

	// Compression is the type of compression used for encoding chunks.
	Compression *CompressionType
}

// TODO(schroederc): encode/decode options as a string for RecordsMetadata

func (o *WriterOptions) compressionType() compressionType {
	if o == nil || o.Compression == nil {
		return compressionType(DefaultCompression)
	}
	return compressionType(*o.Compression)
}

func (o *WriterOptions) chunkSize() uint64 {
	if o == nil || o.ChunkSize == 0 {
		return DefaultChunkSize
	}
	return o.ChunkSize
}

// NewWriter returns a Riegeli Writer for a new Riegeli file to be written to w.
func NewWriter(w io.Writer, opts *WriterOptions) *Writer { return NewWriterAt(w, 0, opts) }

// NewWriterAt returns a Riegeli Writer at the given byte offset within w.
func NewWriterAt(w io.Writer, pos int, opts *WriterOptions) *Writer {
	return &Writer{opts: opts, w: &blockWriter{w: w, pos: pos}}
}

// Writer is a Riegeli records file writer.
//
// TODO(schroederc): add support for writing RecordsMetadata
// TODO(schroederc): add support for tranposed records
type Writer struct {
	opts   *WriterOptions
	w      *blockWriter
	record *recordChunk

	fileHeaderWritten bool
}

// Put writes/buffers the given []byte as a Riegili record.
func (w *Writer) Put(rec []byte) error {
	if err := w.ensureFileHeader(); err != nil {
		return err
	} else if w.record == nil {
		w.record = newRecordChunk(w.opts)
	}

	if err := w.record.put(rec); err != nil {
		return err
	} else if w.record.decodedSize > w.opts.chunkSize() {
		return w.Flush()
	}
	return nil
}

// PutProto writes/buffers the given proto.Message as a Riegili record.
func (w *Writer) PutProto(msg proto.Message) error {
	rec, err := proto.Marshal(msg)
	if err != nil {
		return err
	}
	return w.Put(rec)
}

// Flush writes any buffered records to the underlying io.Writer.
func (w *Writer) Flush() error {
	if err := w.ensureFileHeader(); err != nil {
		return err
	}
	return w.flushRecord()
}

// TODO(schroederc): add concatenation function

// NewReader returns a Riegeli Reader for r.
func NewReader(r io.Reader) *Reader { return &Reader{r: &chunkReader{&blockReader{r: r}}} }

// Reader is a sequential Riegeli records file reader.
//
// TODO(schroederc): add support for seeking
// TODO(schroederc): add support for reading RecordsMetadata
// TODO(schroederc): add support for tranposed records
type Reader struct {
	r *chunkReader

	numRecords  uint64
	sizesReader *bytes.Reader
	valsReader  *bytes.Reader
}

// Next reads and returns the next Riegeli record from the underlying io.Reader.
func (r *Reader) Next() ([]byte, error) { return r.nextRecord() }

// NextProto reads, unmarshals, and returns the next proto.Message from the
// underlying io.Reader.
func (r *Reader) NextProto(msg proto.Message) error {
	rec, err := r.Next()
	if err != nil {
		return err
	}
	return proto.Unmarshal(rec, msg)
}
