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
	"encoding/binary"
	"io"

	"google.golang.org/protobuf/encoding/protowire"
)

// A tagID is an overload of the protocol buffer wire key encodings.
type tagID uint32

const (
	noOpTag tagID = iota
	nonProtoTag
	startOfSubmessageTag
	startOfMessageTag
	rootTag // never encoded
)

// protoWireTypes are the standard protocol buffer wire key types with the
// addition of the Riegeli protoSubmessageType.
type protoWireType int

const (
	protoVarintType     = protoWireType(protowire.VarintType)
	protoFixed64Type    = protoWireType(protowire.Fixed64Type)
	protoBytesType      = protoWireType(protowire.BytesType)
	protoStartGroupType = protoWireType(protowire.StartGroupType)
	protoEndGroupType   = protoWireType(protowire.EndGroupType)
	protoFixed32Type    = protoWireType(protowire.Fixed32Type)
	protoSubmessageType = protoWireType(6)
)

// A tagSubtype differentiates varint/delimited tag types.
type tagSubtype int

const (
	trivialSubtype tagSubtype = 0

	// protowire.VarintType subtypes
	varint1Subtype         = 0
	varintMaxSubtype       = varint1Subtype + binary.MaxVarintLen64 - 1
	varintInline0Subtype   = varintMaxSubtype + 1
	varintInlineMaxSubtype = varintInline0Subtype + 0x7f

	// protowire.BytesType subtypes
	delimitedStringSubtype            = 0
	delimitedStartOfSubmessageSubtype = 1
	delimitedEndOfSubmessageSubtype   = 2
)

// hasDataBuffer returns whether a tag/subtype pairing has an associated data
// buffer.
func hasDataBuffer(tag uint64, subtype tagSubtype) bool {
	switch protoWireType(tag & 7) {
	case protoVarintType:
		return subtype < varintInline0Subtype
	case protoFixed32Type, protoFixed64Type:
		return true
	case protoBytesType:
		return subtype == delimitedStringSubtype
	default:
		return false
	}
}

// validProtoTag returns whether a tag is an actual protocol buffer tag (rather
// than a Riegeli overload).
func validProtoTag(tag uint64) bool {
	switch protoWireType(tag & 7) {
	case protoVarintType, protoFixed32Type, protoFixed64Type, protoBytesType, protoStartGroupType, protoEndGroupType:
		return tag >= 8
	default:
		return false
	}
}

// hasSubtype returns whether a tag has an associated subtype.
func hasSubtype(tag uint64) bool {
	return protoWireType(tag&7) == protoVarintType
}

// A backwardWriter buffers each []byte pushed into it and allows them to be
// read in reverse order.
//
// Note: this is necessary for transposed Riegeli records because each of theirs
// tags are encoded in reverse order
type backwardWriter struct {
	pieces [][]byte
	size   int
}

// Reset resets the writer to an empty state.
func (w *backwardWriter) Reset() { *w = backwardWriter{} }

// Push adds the given []byte to the front of the data to be read.
func (w *backwardWriter) Push(b []byte) {
	if len(b) != 0 {
		w.pieces = append(w.pieces, b)
		w.size += len(b)
	}
}

// PushUvarint encodes and pushes a uvarint to the front of the data to be read.
func (w *backwardWriter) PushUvarint(n uint64) {
	var buf [binary.MaxVarintLen64]byte
	size := binary.PutUvarint(buf[:], n)
	w.Push(buf[:size])
}

// Len returns the total size of data left to be read.
func (w *backwardWriter) Len() int { return w.size }

// Read implements the io.Reader interface.  It reads the []byte buffers in
// reverse order of how they were pushed.
func (w *backwardWriter) Read(b []byte) (n int, err error) {
	if len(w.pieces) == 0 {
		return 0, io.EOF
	}
	for p := len(w.pieces) - 1; p >= 0; p-- {
		left := len(b) - n
		piece := w.pieces[p]
		if left >= len(piece) {
			copy(b[n:], piece)
			n += len(piece)
			w.size -= len(piece)
			w.pieces = w.pieces[:p]
			continue
		}

		copy(b[n:], piece[:left])
		n += left
		w.size -= left
		w.pieces[p] = piece[left:]
		break
	}
	return
}
