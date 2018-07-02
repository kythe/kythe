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

package riegeli

import (
	"bytes"
	"encoding/binary"
	"errors"
	"fmt"
	"io"
)

// https://github.com/google/riegeli/blob/master/doc/riegeli_records_file_format.md#file-signature
var fileSignatureChunk = &chunk{Header: chunkHeader{ChunkType: fileSignatureChunkType}}

func init() {
	binary.LittleEndian.PutUint64(fileSignatureChunk.Header.DataHash[:], hashBytes(fileSignatureChunk.Data))
}

func (w *Writer) ensureFileHeader() error {
	if w.fileHeaderWritten {
		return nil
	}

	_, err := fileSignatureChunk.WriteTo(w.w, w.w.pos)
	// TODO(schroederc): encode RecordsMetadata chunk
	w.fileHeaderWritten = true
	return err
}

func (w *Writer) flushRecord() error {
	if w.recordWriter == nil || w.recordWriter.numRecords == 0 {
		// Skip writing empty record chunk.
		return nil
	}

	data, err := w.recordWriter.encode()
	if err != nil {
		return fmt.Errorf("encoding record chunk: %v", err)
	}
	chunk := &chunk{
		Header: chunkHeader{
			ChunkType:       recordChunkType,
			DataSize:        uint64(len(data)),
			DecodedDataSize: w.recordWriter.decodedSize,
			NumRecords:      w.recordWriter.numRecords,
		},
		Data: data,
	}
	if _, err := chunk.WriteTo(w.w, w.w.pos); err != nil {
		return err
	}
	w.recordWriter, err = newRecordChunkWriter(w.opts)
	return err
}

// A blockWriter interleaves blockHeaders inside chunks of data.  Each
// blockHeader interrupts a single chunk, providing both its relative starting
// and ending positions.
type blockWriter struct {
	w   io.Writer
	pos int
}

// WriteChunk writes a single chunk with interleaving blockHeaders written at
// every 64KiB boundary of the underlying io.Writer.
func (b *blockWriter) WriteChunk(chunk []byte) (n int, err error) {
	if len(chunk) == 0 {
		return 0, errors.New("zero-sized chunk")
	}

	nextBlock := ((b.pos + blockSize - 1) / blockSize) * blockSize
	remainingBlockSize := nextBlock - b.pos

	// The chunk fits entirely within the current block; write it and return.
	if remainingBlockSize >= len(chunk) {
		n, err = b.w.Write(chunk)
		b.pos += n
		return
	}

	blockHeaders := interveningBlockHeaders(b.pos, len(chunk))
	chunkStart, chunkEnd := b.pos, b.pos+len(chunk)+blockHeaders*blockHeaderSize

	// Fill up the current block with as much of the chunk as possible.
	if remainingBlockSize > 0 {
		n, err = b.w.Write(chunk[:remainingBlockSize])
		b.pos += n
		if err != nil {
			return
		}
	}

	// For each remaining slice of the chunk of size usableBlockSize, write a
	// blockHeader and that slice of data.
	for i, blocksLeft := remainingBlockSize, blockHeaders; blocksLeft > 0; i, blocksLeft = i+usableBlockSize, blocksLeft-1 {
		// write blockHeader
		blockStart := b.pos
		n, err := (&blockHeader{
			PreviousChunk: uint64(blockStart - chunkStart),
			NextChunk:     uint64(chunkEnd - blockStart),
		}).WriteTo(b.w)
		b.pos += n
		if err != nil {
			return b.pos - chunkStart, err
		}

		l := len(chunk) - i
		if l > usableBlockSize {
			// write the maximum size of the chunk possible
			l = usableBlockSize
		}

		n, err = b.w.Write(chunk[i : i+l])
		b.pos += n
		if err != nil {
			return b.pos - chunkStart, err
		}
	}

	return b.pos - chunkStart, nil
}

// WriteTo implements the io.WriterTo interface for blockHeaders.
func (b *blockHeader) WriteTo(w io.Writer) (int, error) {
	var buf [blockHeaderSize]byte
	binary.LittleEndian.PutUint64(buf[8:], b.PreviousChunk)
	binary.LittleEndian.PutUint64(buf[16:], b.NextChunk)
	binary.LittleEndian.PutUint64(buf[:], hashBytes(buf[8:]))
	return w.Write(buf[:])
}

type recordChunkWriter struct {
	compressionType         compressionType
	numRecords, decodedSize uint64

	sizesCompressor, valsCompressor compressor
}

func (r *recordChunkWriter) put(rec []byte) error {
	size := uint64(len(rec))

	var buf [binary.MaxVarintLen64]byte
	n := binary.PutUvarint(buf[:], size)

	if _, err := r.sizesCompressor.Write(buf[:n]); err != nil {
		return fmt.Errorf("compressing record size: %v", err)
	} else if _, err := r.valsCompressor.Write(rec); err != nil {
		return fmt.Errorf("compressing record: %v", err)
	}

	r.decodedSize += size
	r.numRecords++
	return nil
}

// Close implements the io.Closer interface.
func (r *recordChunkWriter) Close() error {
	if err := r.sizesCompressor.Close(); err != nil {
		return fmt.Errorf("closing record size compressor: %v", err)
	} else if err := r.valsCompressor.Close(); err != nil {
		return fmt.Errorf("closing record value compressor: %v", err)
	}
	return nil
}

// encode returns the binary-encoding of the Riegeli record chunk.
func (r *recordChunkWriter) encode() ([]byte, error) {
	if err := r.Close(); err != nil {
		return nil, err
	}

	sizesSizePrefix := make([]byte, binary.MaxVarintLen64)
	n := binary.PutUvarint(sizesSizePrefix[:], uint64(r.sizesCompressor.Len()))
	sizesSizePrefix = sizesSizePrefix[:n]

	// TODO(schroederc): reuse buffers
	buf := bytes.NewBuffer(make([]byte, 0, 1+len(sizesSizePrefix)+r.sizesCompressor.Len()+r.valsCompressor.Len()))

	buf.WriteByte(byte(r.compressionType))
	buf.Write(sizesSizePrefix)
	r.sizesCompressor.WriteTo(buf)
	r.valsCompressor.WriteTo(buf)

	return buf.Bytes(), nil
}

func newRecordChunkWriter(opts *WriterOptions) (*recordChunkWriter, error) {
	vals, err := newCompressor(opts)
	if err != nil {
		return nil, err
	}
	sizes, err := newCompressor(opts)
	if err != nil {
		return nil, err
	}
	return &recordChunkWriter{
		compressionType: opts.compressionType(),
		valsCompressor:  vals,
		sizesCompressor: sizes,
	}, nil
}

// WriteTo implements the io.WriterTo interface for chunkHeaders.
func (h *chunkHeader) WriteTo(w io.Writer) (int, error) {
	// header_hash       (8 bytes) — hash of the rest of the header
	// data_size         (8 bytes) — size of data
	// data_hash         (8 bytes) — hash of data
	// chunk_type        (1 byte)  — determines how to interpret data
	// num_records       (7 bytes) — number of records after decoding
	// decoded_data_size (8 bytes) — sum of record sizes after decoding
	var buf [chunkHeaderSize]byte
	binary.LittleEndian.PutUint64(buf[8:16], h.DataSize)
	copy(buf[16:], h.DataHash[:])
	buf[24] = byte(h.ChunkType)
	// NumRecords is only 7 bytes, but the binary package requires an 8 byte
	// buffer.  Pass the 8 bytes and overwrite the last byte when encoding
	// decoded_data_size.
	binary.LittleEndian.PutUint64(buf[25:33], h.NumRecords) // overwrite buf[32] below
	binary.LittleEndian.PutUint64(buf[32:40], h.DecodedDataSize)
	hash := hashBytes(buf[8:])
	binary.LittleEndian.PutUint64(buf[:8], hash)
	return w.Write(buf[:])
}

// WriteTo writes the chunk to w, given its starting position within w.
func (c *chunk) WriteTo(w *blockWriter, pos int) (int, error) {
	binary.LittleEndian.PutUint64(c.Header.DataHash[:], hashBytes(c.Data))
	// TODO(schroederc): reuse buffers
	var buf bytes.Buffer
	if _, err := c.Header.WriteTo(&buf); err != nil {
		return 0, err
	}
	(&buf).Write(c.Data)
	padding := paddingSize(pos, &c.Header)
	for i := 0; i < padding; i++ {
		(&buf).WriteByte(0)
	}
	if buf.Len() != chunkHeaderSize+len(c.Data)+padding {
		return 0, fmt.Errorf("bad chunk size: %v", buf.Len())
	}
	return w.WriteChunk(buf.Bytes())
}
