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

// Package riegeli implements a Reader and Writer for the Riegeli records
// format.
//
// C++ implementation: https://github.com/google/riegeli
// Format spec: https://github.com/google/riegeli/blob/master/doc/riegeli_records_file_format.md
package riegeli // import "kythe.io/kythe/go/util/riegeli"

import (
	"errors"
	"fmt"
	"io"
	"sort"
	"strconv"
	"strings"

	"google.golang.org/protobuf/proto"

	rmpb "kythe.io/third_party/riegeli/records_metadata_go_proto"
)

// Defaults for the WriterOptions.
const (
	DefaultChunkSize uint64 = 1 << 20

	DefaultBrotliLevel = 9
	DefaultZSTDLevel   = 9
)

// DefaultCompression is the default Compression for the WriterOptions.
var DefaultCompression = BrotliCompression(DefaultBrotliLevel)

// CompressionType is the type of compression used for encoding Riegeli chunks.
type CompressionType interface {
	fmt.Stringer
	isCompressionType()
}

type compressionLevel struct {
	compressionType
	level int
}

// String encodes the compressionLevel as a textual WriterOption.
func (c *compressionLevel) String() string {
	switch c.compressionType {
	case noCompression:
		return uncompressedOption
	case brotliCompression:
		if c.level != DefaultBrotliLevel {
			return fmt.Sprintf("%s:%d", brotliOption, c.level)
		}
		return brotliOption
	case zstdCompression:
		if c.level != DefaultZSTDLevel {
			return fmt.Sprintf("%s:%d", zstdOption, c.level)
		}
		return zstdOption
	case snappyCompression:
		return snappyOption
	default:
		panic(fmt.Errorf("unsupported compression_type: '%s'", []byte{byte(c.compressionType)}))
	}
}

func (*compressionLevel) isCompressionType() {}

var (
	// NoCompression indicates that no compression will be used to encode chunks.
	NoCompression CompressionType = &compressionLevel{noCompression, 0}

	// SnappyCompression indicates to use Snappy compression.
	SnappyCompression CompressionType = &compressionLevel{snappyCompression, 0}
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

// ZSTDCompression returns a CompressionType for zstd compression with the given
// compression level.  If level < 0 || level > 22 (outside of the levels
// specified by the zstdlib spec), then the DefaultZSTDLevel will be used.
func ZSTDCompression(level int) CompressionType {
	if level < 0 || level > 22 {
		level = DefaultZSTDLevel
	}
	return &compressionLevel{zstdCompression, level}
}

// WriterOptions customizes the behavior of a Riegeli Writer.
type WriterOptions struct {
	// Desired uncompressed size of a chunk which groups records.
	ChunkSize uint64

	// Compression is the type of compression used for encoding chunks.
	Compression CompressionType

	// Transpose determines whether Protocol Buffer messages have their component
	// key-value entries encoded in separate buffers for better compression.
	Transpose bool
}

// Textual WriterOptions format:
// https://github.com/google/riegeli/blob/master/doc/record_writer_options.md
const (
	brotliOption       = "brotli"
	chunkSizeOption    = "chunk_size"
	defaultOptions     = "default"
	transposeOption    = "transpose"
	uncompressedOption = "uncompressed"
	zstdOption         = "zstd"
	snappyOption       = "snappy"
)

// ParseOptions decodes a WriterOptions from text:
//
//   options ::= option? ("," option?)*
//   option ::=
//     "default" |
//     "transpose" (":" ("true" | "false"))? |
//     "uncompressed" |
//     "brotli" (":" brotli_level)? |
//     "zstd" (":" zstd_level)? |
//     "chunk_size" ":" chunk_size
//   brotli_level ::= integer 0..11 (default 9)
//   zstd_level ::= integer 0..22 (default 9)
//   chunk_size ::= positive integer
func ParseOptions(s string) (*WriterOptions, error) {
	if s == "" {
		return nil, nil
	}
	opts := &WriterOptions{}
	for _, opt := range strings.Split(s, ",") {
		kv := strings.SplitN(opt, ":", 2)
		switch kv[0] {
		case defaultOptions: // ignore
		case snappyOption:
			opts.Compression = SnappyCompression
		case brotliOption:
			level := DefaultBrotliLevel
			if len(kv) != 1 {
				var err error
				level, err = strconv.Atoi(kv[1])
				if err != nil {
					return nil, fmt.Errorf("malformed option: %q: %v", opt, err)
				}
			}
			opts.Compression = BrotliCompression(level)
		case zstdOption:
			level := DefaultZSTDLevel
			if len(kv) != 1 {
				var err error
				level, err = strconv.Atoi(kv[1])
				if err != nil {
					return nil, fmt.Errorf("malformed option: %q: %v", opt, err)
				}
			}
			opts.Compression = ZSTDCompression(level)
		case transposeOption:
			switch {
			case len(kv) == 1 || kv[1] == "true":
				opts.Transpose = true
			case kv[1] == "false":
				opts.Transpose = false
			default:
				return nil, fmt.Errorf("malformed option: %q", opt)
			}
		case chunkSizeOption:
			chunkSize := DefaultChunkSize
			if len(kv) != 1 {
				var err error
				chunkSize, err = strconv.ParseUint(kv[1], 10, 0)
				if err != nil {
					return nil, fmt.Errorf("malformed option: %q: %v", opt, err)
				}
			}
			opts.ChunkSize = chunkSize
		case uncompressedOption:
			if len(kv) != 1 {
				return nil, fmt.Errorf("malformed option: %q", opt)
			}
			opts.Compression = NoCompression
		default:
			return nil, fmt.Errorf("unknown option: %q", opt)
		}
	}
	return opts, nil
}

// String encodes the WriterOptions as text.
func (o *WriterOptions) String() string {
	if o == nil {
		return ""
	}
	var options []string
	if o.ChunkSize > 0 {
		options = append(options, fmt.Sprintf("%s:%d", chunkSizeOption, o.ChunkSize))
	}
	if o.Compression != nil {
		options = append(options, o.Compression.String())
	}
	if o.Transpose {
		options = append(options, transposeOption)
	}
	if len(options) == 0 {
		return defaultOptions
	}
	sort.Strings(options)
	return strings.Join(options, ",")
}

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

func (o *WriterOptions) transpose() bool {
	if o == nil {
		return false
	}
	return o.Transpose
}

// NewWriter returns a Riegeli Writer for a new Riegeli file to be written to w.
func NewWriter(w io.Writer, opts *WriterOptions) *Writer { return NewWriterAt(w, 0, opts) }

// NewWriterAt returns a Riegeli Writer at the given byte offset within w.
func NewWriterAt(w io.Writer, pos int, opts *WriterOptions) *Writer {
	return &Writer{
		opts: opts,
		w:    &blockWriter{w: w, pos: pos},

		fileHeaderWritten: pos != 0,
	}
}

// Writer is a Riegeli records file writer.
type Writer struct {
	opts *WriterOptions
	w    *blockWriter

	recordWriter *talliedRecordWriter

	fileHeaderWritten bool
}

// Put writes/buffers the given []byte as a Riegili record.
func (w *Writer) Put(rec []byte) error {
	err := w.ensureFileHeader()
	if err != nil {
		return err
	}

	if w.recordWriter == nil {
		if err := w.setupRecordWriter(); err != nil {
			return err
		}
	}

	if err := w.recordWriter.Put(rec); err != nil {
		return err
	} else if w.recordWriter.decodedSize >= w.opts.chunkSize() {
		return w.Flush()
	}
	return nil
}

// PutProto writes/buffers the given proto.Message as a Riegili record.
func (w *Writer) PutProto(msg proto.Message) error {
	err := w.ensureFileHeader()
	if err != nil {
		return err
	}

	if w.recordWriter == nil {
		if err := w.setupRecordWriter(); err != nil {
			return err
		}
	}

	if _, err := w.recordWriter.PutProto(msg); err != nil {
		return err
	} else if w.recordWriter.decodedSize >= w.opts.chunkSize() {
		return w.Flush()
	}
	return nil
}

// Flush writes any buffered records to the underlying io.Writer.
func (w *Writer) Flush() error {
	if err := w.ensureFileHeader(); err != nil {
		return err
	}
	return w.flushRecord()
}

// Close releases all resources associated with Writer.  Any buffered records
// will be flushed before releasing any resources.
func (w *Writer) Close() error {
	if err := w.Flush(); err != nil {
		return fmt.Errorf("error flushing writer: %v", err)
	} else if w.recordWriter != nil {
		// Ensure the recordWriter is closed even if it is empty.
		return w.recordWriter.Close()
	}
	return nil
}

// Position returns the current position of the Writer.
func (w *Writer) Position() RecordPosition {
	if !w.fileHeaderWritten {
		return RecordPosition{ChunkBegin: int64(w.w.pos) + blockHeaderSize}
	}
	return RecordPosition{
		ChunkBegin:  int64(w.w.pos),
		RecordIndex: int64(w.recordWriter.numRecords),
	}
}

// TODO(schroederc): add concatenation function

// A RecordPosition is a pointer to the starting offset of a record within a
// Riegeli file.
type RecordPosition struct {
	// ChunkBegin is the starting offset of a chunk within a Riegeli file.
	ChunkBegin int64

	// RecordIndex is the index of a record within the chunk starting at
	// ChunkBegin.
	RecordIndex int64
}

// index returns an integer index corresponding to the given RecordPosition.
func (r RecordPosition) index() int64 { return r.ChunkBegin + r.RecordIndex }

// Reader is a sequential Riegeli records file reader.
type Reader interface {
	// RecordsMetadata returns the optional metadata from the underlying Riegeli
	// file.  If not found, an empty RecordsMetadata is returned and err == nil.
	RecordsMetadata() (*rmpb.RecordsMetadata, error)

	// Next reads and returns the next Riegeli record from the underlying io.Reader.
	Next() ([]byte, error)

	// NextProto reads, unmarshals, and returns the next proto.Message from the
	// underlying io.Reader.
	NextProto(msg proto.Message) error

	// Position returns the current position of the Reader.
	Position() (RecordPosition, error)
}

// ReadSeeker is a Riegeli records file reader able to seek to arbitrary positions.
type ReadSeeker interface {
	Reader

	// Seek interprets pos as an offset to a record within the Riegeli file.  pos
	// must be between 0 and the file's size.  If pos is between records, Seek will
	// position the reader to the next record in the file.
	Seek(pos int64) error

	// SeekToRecord seeks to the given RecordPosition.
	SeekToRecord(pos RecordPosition) error
}

type errSeeker struct{ io.Reader }

// Seek implements the io.Seeker interface.
func (errSeeker) Seek(offset int64, whence int) (int64, error) {
	return 0, errors.New("Seek should not be called on a Reader")
}

// NewReader returns a Riegeli Reader for r.
func NewReader(r io.Reader) Reader { return NewReadSeeker(&errSeeker{r}) }

// NewReadSeeker returns a Riegeli ReadSeeker for r.
func NewReadSeeker(r io.ReadSeeker) ReadSeeker {
	return &reader{r: &chunkReader{r: &blockReader{r: r}}}
}
