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
	"io/ioutil"

	"github.com/golang/protobuf/proto"

	rmpb "kythe.io/third_party/riegeli/records_metadata_go_proto"
)

func (r *Reader) nextRecord() ([]byte, error) {
	if err := r.ensureRecordReader(); err != nil {
		return nil, err
	}
	return r.recordReader.Next()
}

func (r *Reader) ensureRecordReader() error {
	if r.recordReader != nil && r.recordReader.Len() == 0 {
		if err := r.recordReader.Close(); err != nil {
			return fmt.Errorf("error closing record reader: %v", err)
		}
		r.recordReader = nil
	}

	for r.recordReader == nil {
		c, err := r.r.Next()
		if err != nil {
			return err
		} else if c.Header.NumRecords == 0 && c.Header.ChunkType != fileMetadataChunkType {
			// ignore chunks with no records; even for unknown chunk types
			continue
		}

		switch c.Header.ChunkType {
		case fileSignatureChunkType:
			if err := verifySignature(c); err != nil {
				return err
			}
		case fileMetadataChunkType:
			rd, err := newTransposedRecordReader(c)
			if err != nil {
				return fmt.Errorf("bad transpose chunk: %v", err)
			} else if rd.Len() != 1 {
				return fmt.Errorf("didn't find single RecordsMetadata record: found %d", rd.Len())
			}
			rec, err := rd.Next()
			cErr := rd.Close()
			if err != nil {
				return fmt.Errorf("reading RecordsMetadata: %v", err)
			} else if cErr != nil {
				return fmt.Errorf("closing RecordsMetadata reader: %v", err)
			}
			r.metadata = new(rmpb.RecordsMetadata)
			if err := proto.Unmarshal(rec, r.metadata); err != nil {
				return fmt.Errorf("bad RecordsMetadata: %v", err)
			}
		case transposedChunkType:
			r.recordReader, err = newTransposedRecordReader(c)
			if err != nil {
				return fmt.Errorf("bad transpose chunk: %v", err)
			} else if r.recordReader.Len() != c.Header.NumRecords {
				return fmt.Errorf("mismatching number of transposed records: found: %d; expected: %d", r.recordReader.Len(), c.Header.NumRecords)
			}
		case recordChunkType:
			r.recordReader, err = newRecordChunkReader(c)
			if err != nil {
				return fmt.Errorf("bad record chunk: %v", err)
			} else if r.recordReader.Len() != c.Header.NumRecords {
				return fmt.Errorf("mismatching number of records: found: %d; expected: %d", r.recordReader.Len(), c.Header.NumRecords)
			}
		default:
			return fmt.Errorf("unsupported read of chunk_type: '%s'", []byte{byte(c.Header.ChunkType)})
		}
	}
	return nil
}

func verifySignature(c *chunk) error {
	if c.Header != fileSignatureChunkHeader {
		return fmt.Errorf("invalid file signature header: %+v", c.Header)
	} else if len(c.Data) != 0 {
		return fmt.Errorf("extraneous data with file signature: %q", c.Data)
	}
	return nil
}

// A recordReader reads a finite stream of records.
type recordReader interface {
	io.Closer

	// Next reads and returns the next record.  io.EOF is returned if no further
	// records exist.
	Next() ([]byte, error)

	// Len returns the number of records left to read.
	Len() uint64
}

type recordChunkReader struct {
	numRecords uint64

	sizesReader, valsReader decompressor
}

// Len implements part of the recordReader interface.
func (r *recordChunkReader) Len() uint64 { return r.numRecords }

// Next implements part of the recordReader interface.
func (r *recordChunkReader) Next() ([]byte, error) {
	if r.numRecords == 0 {
		return nil, io.EOF
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

// Close implements the io.Closer interface.
func (r *recordChunkReader) Close() error {
	sErr, vErr := r.sizesReader.Close(), r.valsReader.Close()
	if sErr != nil {
		return sErr
	}
	return vErr
}

func newRecordChunkReader(c *chunk) (recordReader, error) {
	rec, err := decodeRecordChunk(c)
	if err != nil {
		return nil, fmt.Errorf("decoding record chunk: %v", err)
	}

	r := &recordChunkReader{numRecords: c.Header.NumRecords}

	sizes := bytes.NewReader(rec.CompressedSizes)
	vals := bytes.NewReader(rec.CompressedValues)

	r.sizesReader, err = newDecompressor(sizes, rec.CompressionType)
	if err != nil {
		return nil, err
	}
	r.valsReader, err = newDecompressor(vals, rec.CompressionType)
	if err != nil {
		return nil, err
	}

	if rec.CompressionType == noCompression && uint64(len(rec.CompressedValues)) != c.Header.DecodedDataSize {
		return nil, fmt.Errorf("bad uncompressed DecodedDataSize: %d vs %d", len(rec.CompressedValues), c.Header.DecodedDataSize)
	}

	return r, nil
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
		return nil, fmt.Errorf("reading block: %v", err)
	} else if n < blockHeaderSize {
		return nil, fmt.Errorf("short read for block header: %d", n)
	} else if _, err := decodeBlockHeader(bytes.NewReader(block[:blockHeaderSize])); err != nil {
		// Throw away the block header; we're only validating it exists.
		// TODO(schroederc): recover to next block on failure here
		return nil, fmt.Errorf("decoding block header: %v", err)
	} else {
		return block[blockHeaderSize:n], nil
	}
}

// A chunkReader reads a sequential stream of chunks.
type chunkReader struct{ r *blockReader }

// Next reads the next full chunk.
func (c *chunkReader) Next() (*chunk, error) {
	var blockPos int
	if c.r.buf != nil {
		blockPos = int(c.r.buf.Size() - int64(c.r.buf.Len()))
	}
	h, err := decodeChunkHeader(c.r)
	if err == io.EOF {
		return nil, io.EOF
	} else if err != nil {
		return nil, fmt.Errorf("reading chunk header: %v", err)
	}
	data := make([]byte, h.DataSize)
	if h.DataSize > 0 {
		if _, err := io.ReadFull(c.r, data); err != nil {
			return nil, fmt.Errorf("reading chunk data: %v", err)
		}
	}
	if hash, expected := hashBytes(data), binary.LittleEndian.Uint64(h.DataHash[:]); hash != expected {
		err = fmt.Errorf("chunk hash mismatch: 0x%x vs 0x%x", hash, expected)
	}
	if padding := paddingSize(blockPos, h); padding > 0 {
		if _, err = io.CopyN(ioutil.Discard, c.r, int64(padding)); err != nil {
			err = fmt.Errorf("failed to discard padding: %v", err)
		}
	}
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
	i, _ := r.Seek(0, io.SeekCurrent)
	if leftover := len(c.Data[i:]); leftover < int(css) {
		return nil, fmt.Errorf("not enough data for CompressedSizes: %d < %d", leftover, css)
	}
	valsStart := i + int64(css)
	rec := &recordChunk{
		CompressionType:  compressionType(ct),
		CompressedSizes:  c.Data[i:valsStart],
		CompressedValues: c.Data[valsStart:],
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

// A byteReadCloser trivially implements io.ByteReader for a io.ReadCloser.
type byteReadCloser struct{ io.ReadCloser }

// ReadByte implements the io.ByteReader interface.
func (b byteReadCloser) ReadByte() (byte, error) {
	var buf [1]byte
	_, err := b.Read(buf[:])
	return buf[0], err
}
