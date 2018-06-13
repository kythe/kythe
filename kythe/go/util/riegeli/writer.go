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

func (w *Writer) ensureFileHeader() error {
	if w.fileHeaderWritten {
		return nil
	}

	// https://github.com/google/riegeli/blob/master/doc/riegeli_records_file_format.md#file-signature
	_, err := (&chunk{Header: chunkHeader{ChunkType: fileSignatureChunkType}}).WriteTo(w.w, w.w.pos)
	// TODO(schroederc): encode RecordsMetadata chunk
	return err
}

func (w *Writer) flushRecord() error {
	if w.record == nil || w.record.numRecords == 0 {
		// Skip writing empty record chunk.
		return nil
	}

	data := w.record.encode()
	chunk := &chunk{
		Header: chunkHeader{
			ChunkType:       recordChunkType,
			DataSize:        uint64(len(data)),
			DecodedDataSize: w.record.decodedSize,
			NumRecords:      w.record.numRecords,
		},
		Data: data,
	}
	_, err := chunk.WriteTo(w.w, w.w.pos)
	w.record = newRecordChunk(w.opts)
	return err
}

// A blockWriter interleaves blockHeaders inside chunks of data.  Each
// blockHeader interrupts a single chunk, providing both its relative starting
// and ending positions.
type blockWriter struct {
	w   io.Writer
	pos int
}

// Write implements the io.Writer interface.  The given []byte must be a single
// non-empty chunk of data.  Interleaving blockHeaders will be written at every
// 64KiB boundary of the underlying io.Writer.
func (b *blockWriter) Write(chunk []byte) (n int, err error) {
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

func newRecordChunk(opts *WriterOptions) *recordChunk {
	return &recordChunk{CompressionType: opts.compressionType()}
}

// put adds a single record to the Riegeli record chunk.
func (r *recordChunk) put(rec []byte) error {
	// TODO(schroederc): compression
	size := uint64(len(rec))

	var buf [binary.MaxVarintLen64]byte
	n := binary.PutUvarint(buf[:], size)

	r.CompressedSizes = append(r.CompressedSizes, buf[:n]...)
	r.CompressedValues = append(r.CompressedValues, rec...)

	r.decodedSize += size
	r.numRecords++
	return nil
}

// encode returns the binary-encoding of the Riegeli record chunk.
func (r *recordChunk) encode() []byte {
	// TODO(schroederc): compression
	buf := bytes.NewBuffer(make([]byte, 0, 1+binary.MaxVarintLen64+len(r.CompressedSizes)+len(r.CompressedValues)))

	buf.WriteByte(byte(r.CompressionType))

	var vBuf [binary.MaxVarintLen64]byte
	n := binary.PutUvarint(vBuf[:], uint64(len(r.CompressedSizes)))
	buf.Write(vBuf[:n])

	buf.Write(r.CompressedSizes)
	buf.Write(r.CompressedValues)

	return buf.Bytes()
}

// WriteTo implements the io.WriterTo interface for chunkHeaders.
func (h *chunkHeader) WriteTo(w io.Writer) (int, error) {
	var buf [chunkHeaderSize]byte
	binary.LittleEndian.PutUint64(buf[8:16], h.DataSize)
	copy(buf[16:], h.DataHash[:])
	buf[24] = byte(h.ChunkType)
	binary.LittleEndian.PutUint64(buf[25:33], h.NumRecords) // overwrite buf[32] below
	binary.LittleEndian.PutUint64(buf[32:40], h.DecodedDataSize)
	hash := hashBytes(buf[8:])
	binary.LittleEndian.PutUint64(buf[:8], hash)
	return w.Write(buf[:])
}

// WriteTo writes the chunk to w, given its starting position within w.
func (c *chunk) WriteTo(w io.Writer, pos int) (int, error) {
	binary.LittleEndian.PutUint64(c.Header.DataHash[:], hashBytes(c.Data))
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
	return w.Write(buf.Bytes())
}
