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

package riegeli

import (
	"bytes"
	"encoding/binary"
	"fmt"
	"io"

	"github.com/DataDog/zstd"
	"github.com/google/brotli/go/cbrotli"
)

// A decompressor decodes a compressed Riegeli block.
type decompressor interface {
	byteReader
	io.Closer
}

func newDecompressor(r byteReader, c compressionType) (decompressor, error) {
	if c == noCompression {
		return &nopDecompressorClose{r}, nil
	}

	if _, err := binary.ReadUvarint(r); err != nil {
		return nil, fmt.Errorf("bad varint prefix for compressed block: %v", err)
	}
	switch c {
	case brotliCompression:
		return &byteReadCloser{cbrotli.NewReader(r)}, nil
	case zstdCompression:
		return &byteReadCloser{zstd.NewReader(r)}, nil
	default:
		return nil, fmt.Errorf("unsupported compression_type: '%s'", []byte{byte(c)})
	}
}

// A byteReadCloser trivially implements io.ByteReader for a io.ReadCloser.
type byteReadCloser struct{ io.ReadCloser }

// ReadByte implements the io.ByteReader interface.
func (b byteReadCloser) ReadByte() (byte, error) {
	var buf [1]byte
	_, err := io.ReadFull(b.ReadCloser, buf[:])
	return buf[0], err
}

// A compressor builds a Riegeli compressed block.
type compressor interface {
	writerTo
	io.Closer
}

func newCompressor(opts *WriterOptions) (compressor, error) {
	buf := bytes.NewBuffer(nil)
	switch opts.compressionType() {
	case noCompression:
		return &nopCompressorClose{buf}, nil
	case brotliCompression:
		brotliOpts := cbrotli.WriterOptions{Quality: opts.compressionLevel()}
		w := cbrotli.NewWriter(buf, brotliOpts)
		return &sizePrefixedWriterTo{buf: buf, WriteCloser: w}, nil
	case zstdCompression:
		lvl := opts.compressionLevel()
		return &sizePrefixedWriterTo{buf: buf, WriteCloser: &batchCompressor{
			Compress: func(src []byte) ([]byte, error) { return zstd.CompressLevel(nil, src, lvl) },
			Buffer:   buf,
		}}, nil
	default:
		return nil, fmt.Errorf("unsupported compression_type: '%s'", []byte{byte(opts.compressionType())})
	}
}

// A batchCompressor buffers all bytes written to it.  On Close, the Buffer is compressed.
type batchCompressor struct {
	Compress func([]byte) ([]byte, error)
	Buffer   *bytes.Buffer
}

// Write implements part of the io.WriteCloser interface.
func (b *batchCompressor) Write(buf []byte) (int, error) { return b.Buffer.Write(buf) }

// Close implements part of the io.WriteCloser interface.
func (b *batchCompressor) Close() error {
	compressed, err := b.Compress(b.Buffer.Bytes())
	if err != nil {
		return err
	}
	*b.Buffer = *bytes.NewBuffer(compressed)
	return nil
}

type sizePrefixedWriterTo struct {
	buf *bytes.Buffer
	io.WriteCloser
	prefix []byte
}

// Close implements part of the compressor interface.
func (w *sizePrefixedWriterTo) Close() error {
	if err := w.WriteCloser.Close(); err != nil {
		return err
	}

	w.prefix = make([]byte, binary.MaxVarintLen64)
	n := int64(binary.PutUvarint(w.prefix[:], uint64(w.buf.Len())))
	w.prefix = w.prefix[:n]

	return nil
}

// WriteTo implements part of the compressor interface.
func (w *sizePrefixedWriterTo) WriteTo(out io.Writer) (int64, error) {
	if n, err := out.Write(w.prefix); err != nil {
		return int64(n), err
	}
	n, err := w.buf.WriteTo(out)
	n += int64(len(w.prefix))
	return n, err
}

// Len implements part of the compressor interface.
func (w *sizePrefixedWriterTo) Len() int { return len(w.prefix) + w.buf.Len() }

type byteReader interface {
	io.Reader
	io.ByteReader
}

type writerTo interface {
	io.WriterTo
	io.Writer

	// Len returns the total data that will be written by WriteTo.
	Len() int
}

// A nopDecompressorClose trivially implements io.Closer for a byteReader.
type nopDecompressorClose struct{ byteReader }

// Close implements the io.Closer interface.
func (nopDecompressorClose) Close() error { return nil }

// A nopCompressorClose trivially implements io.Closer for a writerTo.
type nopCompressorClose struct{ writerTo }

// Close implements the io.Closer interface.
func (nopCompressorClose) Close() error { return nil }
