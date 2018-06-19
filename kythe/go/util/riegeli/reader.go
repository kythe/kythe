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
	"fmt"
	"io"
)

func (r *Reader) nextRecord() ([]byte, error) {
	if err := r.readNextRecordChunk(); err != nil {
		return nil, err
	}

	size, err := binary.ReadUvarint(r.sizesReader)
	if err != nil {
		return nil, fmt.Errorf("error reading record size: %v", err)
	}
	val := make([]byte, size)
	if _, err := io.ReadFull(r.valsReader, val); err != nil {
		return nil, fmt.Errorf("error reading record value: %v", err)
	}
	r.numRecords--
	return val, nil
}

func (r *Reader) readNextRecordChunk() error {
	for r.valsReader == nil || r.numRecords == 0 {
		c, err := r.r.Next()
		if err != nil {
			return err
		} else if c.Header.ChunkType != recordChunkType {
			// TODO(schroederc): support non-record chunk types (especially RecordsMetadata)
			continue
		}

		rec, err := decodeRecordChunk(c)
		if err != nil {
			return fmt.Errorf("decoding record chunk: %v", err)
		} else if rec.CompressionType != noCompression {
			// TODO(schroederc): compression
			return fmt.Errorf("unsupported compression type: %s", []byte{byte(rec.CompressionType)})
		} else if uint64(len(rec.CompressedValues)) != c.Header.DecodedDataSize {
			return fmt.Errorf("bad DecodedDataSize: %d vs %d", len(rec.CompressedValues), c.Header.DecodedDataSize)
		}
		r.numRecords = c.Header.NumRecords
		r.sizesReader = bytes.NewReader(rec.CompressedSizes)
		r.valsReader = bytes.NewReader(rec.CompressedValues)
	}
	return nil
}

type blockReader struct {
	r   io.Reader
	buf *bytes.Reader
}

// Read implements the io.Reader interface by skipping over the interleaven
// block headers and sequentially reading chunk data.
func (b *blockReader) Read(bs []byte) (int, error) {
	if b.buf == nil || b.buf.Len() == 0 {
		block, err := b.Next()
		if err != nil {
			return 0, err
		}
		b.buf = bytes.NewReader(block)
	}
	return b.buf.Read(bs)
}

// Next reads the next full block of data.
func (b *blockReader) Next() ([]byte, error) {
	var block [blockSize]byte
	if n, err := io.ReadFull(b.r, block[:]); err == io.EOF {
		return nil, io.EOF
	} else if err != nil && err != io.ErrUnexpectedEOF {
		return nil, err
	} else {
		return block[blockHeaderSize:n], nil
	}
}

// A chunkReader reads a sequential stream of chunks.
type chunkReader struct{ r io.Reader }

// Next reads the next full chunk.
func (c *chunkReader) Next() (*chunk, error) {
	h, err := decodeChunkHeader(c.r)
	if err != nil {
		return nil, err
	}
	// log.Printf("Read chunkHeader: %#v", h)
	data := make([]byte, h.DataSize)
	if _, err := io.ReadFull(c.r, data); err != nil {
		return nil, err
	} else if hash, expected := hashBytes(data), binary.LittleEndian.Uint64(h.DataHash[:]); hash != expected {
		err = fmt.Errorf("chunk hash mismatch: 0x%x vs 0x%x", hash, expected)
	}
	// TODO(schroederc): read padding
	return &chunk{Header: *h, Data: data}, err
}

func decodeRecordChunk(c *chunk) (*recordChunk, error) {
	r := bytes.NewReader(c.Data)
	ct, err := r.ReadByte()
	if err != nil {
		return nil, fmt.Errorf("error reading compression type: %v", err)
	}
	css, err := binary.ReadUvarint(r)
	if err != nil {
		return nil, fmt.Errorf("error reading CompressedSizesSize: %v", err)
	}
	cs := make([]byte, css)
	if _, err := io.ReadFull(r, cs); err != nil {
		return nil, fmt.Errorf("error reading CompressedSizes: %v", err)
	}
	vals := make([]byte, r.Len())
	if _, err := io.ReadFull(r, vals); err != nil {
		return nil, fmt.Errorf("error reading CompressedValues: %v", err)
	}
	rec := &recordChunk{
		CompressionType:  compressionType(ct),
		CompressedSizes:  cs,
		CompressedValues: vals,

		numRecords: c.Header.NumRecords,
	}
	return rec, nil
}

func decodeBlockHeader(r io.Reader) (*blockHeader, error) {
	var buf [blockHeaderSize]byte
	if _, err := io.ReadFull(r, buf[:]); err != nil {
		return nil, err
	}
	headerHash := hashBytes(buf[8:])
	h := &blockHeader{
		PreviousChunk: binary.LittleEndian.Uint64(buf[8:16]),
		NextChunk:     binary.LittleEndian.Uint64(buf[16:24]),
	}
	if expected := binary.LittleEndian.Uint64(buf[:8]); expected != headerHash {
		return h, fmt.Errorf("bad blockHeader hash: found %d; expected %d", headerHash, expected)
	}
	return h, nil
}

func decodeChunkHeader(r io.Reader) (*chunkHeader, error) {
	var buf [chunkHeaderSize]byte
	if _, err := io.ReadFull(r, buf[:]); err != nil {
		return nil, err
	}
	headerHash := hashBytes(buf[8:])
	h := &chunkHeader{
		DataSize:        binary.LittleEndian.Uint64(buf[8:16]),
		ChunkType:       chunkType(buf[24]),
		NumRecords:      binary.LittleEndian.Uint64(append(buf[25:32:32], 0x00)),
		DecodedDataSize: binary.LittleEndian.Uint64(buf[32:40]),
	}
	copy(h.DataHash[:], buf[16:24])
	if expected := binary.LittleEndian.Uint64(buf[:8]); expected != headerHash {
		return h, fmt.Errorf("bad chunkHeader hash: found 0x%x; expected %x", headerHash, expected)
	}
	return h, nil
}
