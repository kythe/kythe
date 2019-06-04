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
	"io/ioutil"

	"github.com/golang/protobuf/proto"

	rmpb "kythe.io/third_party/riegeli/records_metadata_go_proto"
)

type reader struct {
	r *chunkReader

	metadata *rmpb.RecordsMetadata

	recordReader recordReader
	chunkSize    int64
}

// RecordsMetadata implements part of the Reader interface.
func (r *reader) RecordsMetadata() (*rmpb.RecordsMetadata, error) {
	if r.metadata == nil {
		if err := r.ensureRecordReader(); err != nil && err != io.EOF {
			return nil, err
		} else if r.metadata == nil {
			r.metadata = new(rmpb.RecordsMetadata)
		}
	}
	return r.metadata, nil
}

// Next implements part of the Reader interface.
func (r *reader) Next() ([]byte, error) { return r.nextRecord() }

// NextProto implements part of the Reader interface.
func (r *reader) NextProto(msg proto.Message) error {
	rec, err := r.Next()
	if err != nil {
		return err
	}
	return proto.Unmarshal(rec, msg)
}

// Position implements part of the Reader interface.
func (r *reader) Position() (RecordPosition, error) {
	// Verify file before returning a position.
	if _, err := r.RecordsMetadata(); err != nil {
		return RecordPosition{}, fmt.Errorf("error verifying file: %v", err)
	}

	return RecordPosition{
		ChunkBegin:  r.r.Position(),
		RecordIndex: int64(r.recordReader.Index()),
	}, nil
}

// SeekToRecord implements part of the ReadSeeker interface.
func (r *reader) SeekToRecord(pos RecordPosition) error {
	// Verify file before seeking.
	if _, err := r.RecordsMetadata(); err != nil {
		return fmt.Errorf("error verifying file: %v", err)
	}

	if r.r.Position() != pos.ChunkBegin {
		// We're seeking outside of the current chunk.
		if err := r.r.Seek(pos.ChunkBegin); err != nil {
			return err
		}
		r.recordReader = nil
		if err := r.ensureRecordReader(); err != nil {
			return err
		}
	}

	r.recordReader.Seek(int(pos.RecordIndex))
	return nil
}

// Seek implements part of the ReadSeeker interface.
func (r *reader) Seek(pos int64) error {
	// Verify file before seeking.
	if _, err := r.RecordsMetadata(); err != nil {
		return fmt.Errorf("error verifying file: %v", err)
	}

	if pos < r.r.Position() || pos >= r.r.Position()+r.chunkSize {
		// We're seeking outside of the current chunk.
		if err := r.r.SeekToChunkContaining(pos); err != nil && err != io.EOF {
			return fmt.Errorf("failed to seek to enclosing chunk: %v", err)
		}
		r.recordReader = nil
		if err := r.ensureRecordReader(); err == io.EOF {
			// Seeking to the end of the file is allowed.
			return nil
		} else if err != nil {
			return err
		}
	}
	recordIndex := int(pos - r.r.Position())
	r.recordReader.Seek(recordIndex)
	return nil
}

func (r *reader) nextRecord() ([]byte, error) {
	if err := r.ensureRecordReader(); err != nil {
		return nil, err
	}
	return r.recordReader.Next()
}

func (r *reader) ensureRecordReader() error {
	if r.recordReader != nil && r.recordReader.Len() == 0 {
		r.recordReader = nil
	}

	for r.recordReader == nil {
		c, chunkSize, err := r.r.Next()
		if err != nil {
			return err
		} else if c.Header.NumRecords == 0 && c.Header.ChunkType != fileSignatureChunkType && c.Header.ChunkType != fileMetadataChunkType {
			// ignore chunks with no records; even for unknown chunk types
			continue
		}
		r.chunkSize = chunkSize

		switch c.Header.ChunkType {
		case fileSignatureChunkType:
			// TODO(schroederc): verify once at beginning of reader
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
			if err != nil {
				return fmt.Errorf("reading RecordsMetadata: %v", err)
			}
			r.metadata = new(rmpb.RecordsMetadata)
			if err := proto.Unmarshal(rec, r.metadata); err != nil {
				return fmt.Errorf("bad RecordsMetadata: %v", err)
			}
		case transposedChunkType:
			r.recordReader, err = newTransposedRecordReader(c)
			if err != nil {
				return fmt.Errorf("bad transpose chunk: %v", err)
			} else if uint64(r.recordReader.Len()) != c.Header.NumRecords {
				return fmt.Errorf("mismatching number of transposed records: found: %d; expected: %d", r.recordReader.Len(), c.Header.NumRecords)
			}
		case recordChunkType:
			r.recordReader, err = newRecordChunkReader(c)
			if err != nil {
				return fmt.Errorf("bad record chunk: %v", err)
			} else if uint64(r.recordReader.Len()) != c.Header.NumRecords {
				return fmt.Errorf("mismatching number of records: found: %d; expected: %d", r.recordReader.Len(), c.Header.NumRecords)
			}
		default:
			return fmt.Errorf("unsupported read of chunk_type: '%s'", []byte{byte(c.Header.ChunkType)})
		}
	}
	return nil
}

func verifySignature(c *chunk) error {
	if c.Header != fileSignatureChunk.Header {
		return fmt.Errorf("invalid file signature: %+v", c)
	} else if len(c.Data) != 0 {
		return fmt.Errorf("extraneous data with file signature: %q", c.Data)
	}
	return nil
}

// A recordReader reads a finite stream of records.
type recordReader interface {
	// Next reads and returns the next record.  io.EOF is returned if no further
	// records exist.
	Next() ([]byte, error)

	// Len returns the number of records left to read.
	Len() int

	// Index returns the index of the record that will be returned by Next.
	// Index() >= 0 && Index() <= Len ())
	Index() int

	// Seek positions the reader before the record at the given 0-based index.
	Seek(index int)
}

type fixedRecordReader struct {
	records [][]byte
	index   int
}

// Len implements part of the recordReader interface.
func (r *fixedRecordReader) Len() int { return len(r.records) - r.index }

// Next implements part of the recordReader interface.
func (r *fixedRecordReader) Next() ([]byte, error) {
	if r.index >= len(r.records) {
		return nil, io.EOF
	}

	rec := r.records[r.index]
	r.index++
	return rec, nil
}

// Index implements part of the recordReader interface.
func (r *fixedRecordReader) Index() int { return r.index }

// Seek implements part of the recordReader interface.
func (r *fixedRecordReader) Seek(index int) {
	if index < 0 {
		index = 0
	} else if index >= len(r.records) {
		index = len(r.records)
	}
	r.index = index
}

func newRecordChunkReader(c *chunk) (recordReader, error) {
	rec, err := decodeRecordChunk(c)
	if err != nil {
		return nil, fmt.Errorf("decoding record chunk: %v", err)
	}

	sizesBuf := rec.CompressedSizes
	valsBuf := rec.CompressedValues

	// Decode sizes/values, if necessary
	if rec.CompressionType != noCompression {
		sizesDec, err := newDecompressor(bytes.NewReader(rec.CompressedSizes), rec.CompressionType)
		if err != nil {
			return nil, err
		}
		sizesBuf, err = ioutil.ReadAll(sizesDec)
		if err != nil {
			return nil, fmt.Errorf("error decompressing record sizes: %v", err)
		} else if err := sizesDec.Close(); err != nil {
			return nil, fmt.Errorf("error closing record sizes decompressor: %v", err)
		}

		valsDec, err := newDecompressor(bytes.NewReader(rec.CompressedValues), rec.CompressionType)
		if err != nil {
			return nil, err
		}
		valsBuf = make([]byte, c.Header.DecodedDataSize)
		if _, err := io.ReadFull(valsDec, valsBuf); err != nil {
			return nil, fmt.Errorf("error decompressing record values: %v", err)
		} else if b, err := valsDec.ReadByte(); c.Header.DecodedDataSize != 0 && err == nil {
			return nil, fmt.Errorf("read past end of expected record values buffer: %v %v", b, err)
		} else if err := valsDec.Close(); err != nil {
			return nil, fmt.Errorf("error closing record values decompressor: %v", err)
		}
	} else if uint64(len(valsBuf)) != c.Header.DecodedDataSize {
		return nil, fmt.Errorf("bad uncompressed DecodedDataSize: %d vs %d", len(valsBuf), c.Header.DecodedDataSize)
	}

	sizes := bytes.NewReader(sizesBuf)
	records := make([][]byte, 0, c.Header.NumRecords)
	for i := 0; i < int(c.Header.NumRecords); i++ {
		size, err := binary.ReadUvarint(sizes)
		if err != nil {
			return nil, fmt.Errorf("error reading record size: %v", err)
		} else if size > uint64(len(valsBuf)) {
			return nil, fmt.Errorf("not enough data for record of size %d; found %d bytes", size, len(valsBuf))
		}
		val := valsBuf[:size]
		records = append(records, val)
		valsBuf = valsBuf[size:]
	}

	if b, err := sizes.ReadByte(); err != io.EOF {
		return nil, fmt.Errorf("trailing record size data: 0x%x (err: %v)", b, err)
	} else if len(valsBuf) != 0 {
		return nil, fmt.Errorf("trailing record value data: %v", valsBuf)
	}

	return &fixedRecordReader{records: records}, nil
}

type blockReader struct {
	r   io.ReadSeeker
	buf *bytes.Reader

	header   *blockHeader
	position int64
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
	n, err := b.buf.Read(bs)
	b.position += int64(n)
	return n, err
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
	} else if hdr, err := decodeBlockHeader(bytes.NewReader(block[:blockHeaderSize])); err != nil {
		// TODO(schroederc): recover to next block on failure here
		return nil, fmt.Errorf("decoding block header: %v", err)
	} else {
		b.header = hdr
		b.position += blockHeaderSize
		return block[blockHeaderSize:n], nil
	}
}

// Position returns the current position within the underlying ReadSeeker.
func (b *blockReader) Position() int64 {
	if b.position%blockSize == blockHeaderSize {
		return b.position - blockHeaderSize
	}
	return b.position
}

// Seek seeks to the the given position within the underlying ReadSeeker.
func (b *blockReader) Seek(pos int64) error {
	blockStart := (pos / blockSize) * blockSize
	if pos == blockStart {
		pos = blockStart + blockHeaderSize
	} else if pos-blockStart < blockHeaderSize {
		return fmt.Errorf("attempting to seek into block header: %d", pos)
	}
	if err := b.readBlock(blockStart); err != nil {
		return err
	} else if _, err := b.buf.Seek(pos-(blockStart+blockHeaderSize), io.SeekStart); err != nil {
		return err
	}
	b.position = pos
	return nil
}

func (b *blockReader) readBlock(blockStart int64) error {
	if b.buf != nil && b.position >= blockStart && b.position < blockStart+blockSize {
		return nil
	}
	_, err := b.r.Seek(blockStart, io.SeekStart)
	if err != nil {
		return fmt.Errorf("failed to seek to beginning of block: %v", err)
	}
	b.position = blockStart
	block, err := b.Next()
	if err != nil {
		return err
	}
	b.buf = bytes.NewReader(block)
	return nil
}

// SeekToNextChunkInBlock seeks to the first chunk starting from the block
// starting at the given offset.
func (b *blockReader) SeekToNextChunkInBlock(blockStart int64) error {
	var offset int64
	for {
		if err := b.readBlock(blockStart); err != nil {
			return err
		}
		if b.header.PreviousChunk == 0 {
			// Block starts with a chunk
			offset = 0
		} else {
			// Block interrupts a chunk
			offset = int64(b.header.NextChunk) - blockHeaderSize
		}

		if offset < usableBlockSize {
			break
		}

		blockStart += blockSize
	}
	if _, err := b.buf.Seek(offset, io.SeekStart); err != nil {
		return err
	}
	b.position = blockStart + blockHeaderSize + offset
	return nil
}

// A chunkReader reads a sequential stream of chunks.
type chunkReader struct {
	r *blockReader

	position int64
}

// Next reads the next full chunk.
func (c *chunkReader) Next() (*chunk, int64, error) {
	c.position = c.r.Position()
	h, err := decodeChunkHeader(c.r)
	if err == io.EOF {
		return nil, 0, io.EOF
	} else if err != nil {
		return nil, 0, fmt.Errorf("reading chunk header: %v", err)
	}
	data := make([]byte, h.DataSize)
	if h.DataSize > 0 {
		if _, err := io.ReadFull(c.r, data); err != nil {
			return nil, 0, fmt.Errorf("reading chunk data: %v", err)
		}
	}
	if hash, expected := hashBytes(data), binary.LittleEndian.Uint64(h.DataHash[:]); hash != expected {
		err = fmt.Errorf("chunk hash mismatch: 0x%x vs 0x%x", hash, expected)
	}
	chunkSize := chunkHeaderSize + int64(len(data))
	if padding := paddingSize(int(c.position), h); padding > 0 {
		if _, err = io.CopyN(ioutil.Discard, c.r, int64(padding)); err != nil {
			err = fmt.Errorf("failed to discard padding: %v", err)
		}
		chunkSize += int64(padding)
	}
	blockHeaders := blockHeaderSize * int64(interveningBlockHeaders(int(c.position), int(chunkSize)))
	chunkSize += blockHeaders
	return &chunk{Header: *h, Data: data}, chunkSize, err
}

// Seek seeks to the chunk at the given position.
func (c *chunkReader) Seek(pos int64) error { return c.r.Seek(pos) }

// Seek seeks to the chunk that contains the given position.
func (c *chunkReader) SeekToChunkContaining(pos int64) error {
	blockStart := (pos / blockSize) * blockSize
	if err := c.r.SeekToNextChunkInBlock(blockStart); err != nil {
		return err
	}

	if pos < c.r.Position() {
		// Chunk starts in previous block
		blockStart -= blockSize
		if err := c.r.SeekToNextChunkInBlock(blockStart); err != nil {
			return err
		}
	}

	// Seek through chunks in block.
	for {
		c.position = c.r.Position()
		h, err := decodeChunkHeader(c.r)
		if err == io.EOF {
			return io.EOF
		} else if err != nil {
			return fmt.Errorf("reading chunk header at %d: %v", c.position, err)
		}

		chunkSize := chunkHeaderSize + int64(h.DataSize) + int64(paddingSize(int(c.position), h))
		blockHeaders := blockHeaderSize * int64(interveningBlockHeaders(int(c.position), int(chunkSize)))
		chunkSize += blockHeaders

		nextChunk := c.position + chunkSize
		if pos < nextChunk {
			// We're at the chunk containing the desired position.
			break
		} else if err := c.r.Seek(nextChunk); err != nil {
			return fmt.Errorf("error seeking to next chunk: %v", err)
		}
	}
	return c.r.Seek(c.position)
}

// Position returns the position of the current chunk.
func (c *chunkReader) Position() int64 { return c.position }

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
