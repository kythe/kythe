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
	"fmt"

	"github.com/minio/highwayhash"
)

func hashBytes(b []byte) uint64 {
	h, _ := highwayhash.New64(hashKey[:])
	h.Write(b)
	return h.Sum64()
}

// https://github.com/google/riegeli/blob/master/doc/riegeli_records_file_format.md#conventions
// Binary-encoding of ('Riegeli/', 'records\n', 'Riegeli/', 'records\n')
var hashKey = []byte{
	0x52, 0x69, 0x65, 0x67, 0x65, 0x6c, 0x69, 0x2f,
	0x72, 0x65, 0x63, 0x6f, 0x72, 0x64, 0x73, 0x0a,
	0x52, 0x69, 0x65, 0x67, 0x65, 0x6c, 0x69, 0x2f,
	0x72, 0x65, 0x63, 0x6f, 0x72, 0x64, 0x73, 0x0a,
}

// A blockHeader is located every 64KiB in a Riegeli file.
// https://github.com/google/riegeli/blob/master/doc/riegeli_records_file_format.md#block-header
type blockHeader struct { // 24 bytes
	// HeaderHash    [8]byte
	PreviousChunk uint64 // 8 bytes
	NextChunk     uint64 // 8 bytes
}

// A chunk is the unit of dat within a Riegeli block.
// https://github.com/google/riegeli/blob/master/doc/riegeli_records_file_format.md#chunk
type chunk struct { // 40 bytes + len(Data) + padding
	Header chunkHeader
	Data   []byte
}
type chunkHeader struct { // 40 bytes
	// HeaderHash      [8]byte
	DataSize        uint64 // 8 bytes
	DataHash        [8]byte
	ChunkType       chunkType
	NumRecords      uint64 // 7 bytes
	DecodedDataSize uint64 // 8 bytes
}

type chunkType byte

const (
	fileSignatureChunkType chunkType = 0x73
	fileMetadataChunkType  chunkType = 0x6d
	paddingChunkType       chunkType = 0x70
	recordChunkType        chunkType = 0x72
	transposedChunkType    chunkType = 0x74
)

// compressionType is the compression format for a chunk
// https://github.com/google/riegeli/blob/master/doc/riegeli_records_file_format.md#chunk-data
type compressionType byte

const (
	noCompression     compressionType = 0
	brotliCompression compressionType = 0x62
	zstdCompression   compressionType = 0x7a
	snappyCompression compressionType = 0x73
)

// A recordChunk is the standard chunk type for user records in a Riegeli file.
// https://github.com/google/riegeli/blob/master/doc/riegeli_records_file_format.md#simple-chunk-with-records
type recordChunk struct { // 1 + len(varint(CompressedSizesSize)) + len(CompressedSizes) + len(CompressedValues)
	CompressionType compressionType
	// CompressedSizesSize uint64 == len(CompressedSizes)
	CompressedSizes  []byte // len([]varint64) == NumRecords
	CompressedValues []byte
}

// https://github.com/google/riegeli/blob/master/doc/riegeli_records_file_format.md#implementation-notes
const (
	blockSize       = 1 << 16
	blockHeaderSize = 24
	usableBlockSize = blockSize - blockHeaderSize
	chunkHeaderSize = 40
)

func interveningBlockHeaders(pos, size int) int {
	if pos%blockSize == blockHeaderSize {
		panic(fmt.Errorf("invalid chunk boundary: %d", pos))
	}
	return (size + (pos+usableBlockSize-1)%blockSize) / usableBlockSize
}

func paddingSize(pos int, h *chunkHeader) int {
	size := chunkHeaderSize + int(h.DataSize)
	if int(h.NumRecords) <= size {
		return 0
	}
	return int(h.NumRecords) - size
}
