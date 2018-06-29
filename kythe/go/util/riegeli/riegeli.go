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
	"io"

	"github.com/golang/protobuf/proto"

	rmpb "kythe.io/third_party/riegeli/records_metadata_go_proto"
)

// Defaults for the WriterOptions.
const (
	DefaultChunkSize = 1 << 20

	DefaultBrotliLevel = 9
)

// DefaultCompression is the default Compression for the WriterOptions.
var DefaultCompression = BrotliCompression(DefaultBrotliLevel)

// CompressionType is the type of compression used for encoding Riegeli chunks.
type CompressionType interface{ isCompressionType() }

type compressionLevel struct {
	compressionType
	level int
}

func (*compressionLevel) isCompressionType() {}

var (
	// NoCompression indicates that no compression will be used to encode chunks.
	NoCompression CompressionType = &compressionLevel{noCompression, 0}
	// TODO(schroederc): add zstd compression
)

// BrotliCompression returns a CompressionType for Brotli compression with the
// given quality level.  If level < 0 || level > 11, then the DefaultBrotliLevel
// will be used.
func BrotliCompression(level int) CompressionType {
	if level < 0 || level > 11 {
		level = DefaultBrotliLevel
	}
	return &compressionLevel{brotliCompression, level}
}

// WriterOptions customizes the behavior of a Riegeli Writer.
type WriterOptions struct {
	// Desired uncompressed size of a chunk which groups records.
	ChunkSize uint64

	// Compression is the type of compression used for encoding chunks.
	Compression CompressionType
}

// TODO(schroederc): encode/decode options as a string for RecordsMetadata

func (o *WriterOptions) compressionType() compressionType {
	c := DefaultCompression
	if o != nil && o.Compression != nil {
		c = o.Compression
	}
	return c.(*compressionLevel).compressionType
}

func (o *WriterOptions) compressionLevel() int {
	c := DefaultCompression
	if o != nil && o.Compression != nil {
		c = o.Compression
	}
	return c.(*compressionLevel).level
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
	opts *WriterOptions
	w    *blockWriter

	recordWriter *recordChunkWriter

	fileHeaderWritten bool
}

// Put writes/buffers the given []byte as a Riegili record.
func (w *Writer) Put(rec []byte) error {
	if err := w.ensureFileHeader(); err != nil {
		return err
	} else if w.recordWriter == nil {
		w.recordWriter = newRecordChunkWriter(w.opts)
	}

	if err := w.recordWriter.put(rec); err != nil {
		return err
	} else if w.recordWriter.decodedSize >= w.opts.chunkSize() {
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
type Reader struct {
	r *chunkReader

	metadata *rmpb.RecordsMetadata

	recordReader recordReader
}

// RecordsMetadata returns the optional metadata from the underlying Riegeli
// file.  If not found, nil is returned for both the metadata and the error.
func (r *Reader) RecordsMetadata() (*rmpb.RecordsMetadata, error) {
	if err := r.ensureRecordReader(); err != nil && err != io.EOF {
		return nil, err
	}
	return r.metadata, nil
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
