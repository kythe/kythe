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

	"google.golang.org/protobuf/proto"
)

func newTransposeChunkWriter(opts *WriterOptions) (recordWriter, error) {
	return &transposedChunkWriter{
		opts: opts,

		nextNodeID:    rootTag + 1,
		data:          make([][]*buffer, totalBufferTypes),
		nodes:         make(map[nodeID]*messageNode),
		encodedTagPos: make(map[encodedTag]int),
	}, nil
}

type bufferType int

// Categories for buffers
const (
	varintBuffer bufferType = iota
	fixed32Buffer
	fixed64Buffer
	delimitedBuffer
	nonProtoBuffer

	// totalBufferTypes is the total number of buffer types
	totalBufferTypes
)

type nodeID struct {
	parentID tagID
	fieldNum uint32
}

type encodedTag struct {
	tagID   tagID
	tag     uint64
	subtype tagSubtype
}

type messageNode struct {
	id     tagID
	writer *backwardWriter
}

type buffer struct {
	id     nodeID
	writer *backwardWriter
}

type transposedChunkWriter struct {
	opts *WriterOptions

	nextNodeID tagID
	nodes      map[nodeID]*messageNode

	encodedTagPos map[encodedTag]int
	encodedTags   []int

	// bufferType -> []*buffer
	data [][]*buffer

	nonProtoLengths backwardWriter
}

// Close implements part of the recordWriter interface.
func (t *transposedChunkWriter) Close() error { return nil }

// Put implements part of the recordWriter interface.
func (t *transposedChunkWriter) Put(rec []byte) error {
	if isProtoMessage(rec) {
		return t.putProto(rec)
	}
	return t.putNonProto(rec)
}

// PutProto implements part of the recordWriter interface.
func (t *transposedChunkWriter) PutProto(msg proto.Message) (int, error) {
	rec, err := proto.Marshal(msg)
	if err != nil {
		return 0, err
	}
	return len(rec), t.putProto(rec)
}

func (t *transposedChunkWriter) addEncodedTag(e encodedTag) int {
	if pos, ok := t.encodedTagPos[e]; ok {
		return pos
	}
	pos := len(t.encodedTagPos)
	t.encodedTagPos[e] = pos
	return pos
}

func (t *transposedChunkWriter) getNode(nodeID nodeID) *messageNode {
	node, ok := t.nodes[nodeID]
	if !ok {
		newID := t.nextNodeID
		t.nextNodeID++
		node = &messageNode{id: newID}
		t.nodes[nodeID] = node
	}
	return node
}

func (t *transposedChunkWriter) getBuffer(nodeID nodeID, typ bufferType) *backwardWriter {
	node := t.getNode(nodeID)
	if node.writer == nil {
		buf := &buffer{id: nodeID, writer: &backwardWriter{}}
		t.data[typ] = append(t.data[typ], buf)
		node.writer = buf.writer
	}
	return node.writer
}

func (t *transposedChunkWriter) putNonProto(rec []byte) error {
	t.encodedTags = append(t.encodedTags, t.addEncodedTag(encodedTag{tagID: nonProtoTag}))
	t.getBuffer(nodeID{nonProtoTag, 0}, nonProtoBuffer).Push(rec)
	t.nonProtoLengths.PushUvarint(uint64(len(rec)))
	return nil
}

func (t *transposedChunkWriter) putProto(rec []byte) error {
	t.encodedTags = append(t.encodedTags, t.addEncodedTag(encodedTag{tagID: startOfMessageTag}))
	return t.addMessage(rec, rootTag, 0)
}

// maxRecursionDepth is the cutoff point of decoding delimited string fields as
// submessages.
const maxRecursionDepth = 100

func (t *transposedChunkWriter) addMessage(rec []byte, parentID tagID, depth int) error {
	rd := bytes.NewReader(rec)
	for rd.Len() > 0 {
		tag, err := binary.ReadUvarint(rd)
		if err != nil {
			return err
		}

		field := uint32(tag >> 3)
		switch protoWireType(tag & 7) {
		case protoVarintType:
			size := rd.Len()
			n, err := binary.ReadUvarint(rd)
			if err != nil {
				return fmt.Errorf("reading varint for 0x%x: %v", tag, err)
			}
			size = size - rd.Len()
			// TODO(schroederc): inline small varints
			t.encodedTags = append(t.encodedTags, t.addEncodedTag(encodedTag{
				tagID:   parentID,
				tag:     tag,
				subtype: varint1Subtype + tagSubtype(size-1),
			}))
			// TODO(schroederc): don't decode/encode varint
			buf := make([]byte, binary.MaxVarintLen64)
			buf = buf[:binary.PutUvarint(buf[:], n)]
			t.getBuffer(nodeID{parentID, field}, varintBuffer).Push(buf)
		case protoFixed32Type:
			var buf [4]byte
			if _, err := io.ReadFull(rd, buf[:]); err != nil { // TODO remove all ReadFull with slices
				return fmt.Errorf("reading fixed32 for 0x%x: %v", tag, err)
			}
			t.encodedTags = append(t.encodedTags, t.addEncodedTag(encodedTag{
				tagID: parentID,
				tag:   tag,
			}))
			t.getBuffer(nodeID{parentID, field}, fixed32Buffer).Push(buf[:])
		case protoFixed64Type:
			var buf [8]byte
			if _, err := io.ReadFull(rd, buf[:]); err != nil {
				return fmt.Errorf("reading fixed64 for 0x%x: %v", tag, err)
			}
			t.encodedTags = append(t.encodedTags, t.addEncodedTag(encodedTag{
				tagID: parentID,
				tag:   tag,
			}))
			t.getBuffer(nodeID{parentID, field}, fixed64Buffer).Push(buf[:])
		case protoBytesType:
			pos, _ := rd.Seek(0, io.SeekCurrent)
			size, err := binary.ReadUvarint(rd)
			if err != nil {
				return fmt.Errorf("reading delimited size for 0x%x: %v", tag, err)
			}
			recPos, _ := rd.Seek(0, io.SeekCurrent)
			sizePrefix := recPos - pos
			if len(rec)-int(recPos) < int(size) {
				return fmt.Errorf("reading delimited record for 0x%x (size %d): unexpected EOF", tag, size)
			}
			buf := rec[pos : pos+int64(size)+sizePrefix]
			rd.Seek(int64(size), io.SeekCurrent)
			if isProtoMessage(buf[sizePrefix:]) && depth < maxRecursionDepth {
				t.encodedTags = append(t.encodedTags, t.addEncodedTag(encodedTag{
					tagID:   parentID,
					tag:     tag,
					subtype: delimitedStartOfSubmessageSubtype,
				}))
				node := t.getNode(nodeID{parentID, field})
				if err := t.addMessage(buf[sizePrefix:], node.id, depth+1); err != nil {
					return fmt.Errorf("encoding submessage: %v", err)
				}
				t.encodedTags = append(t.encodedTags, t.addEncodedTag(encodedTag{
					tagID:   parentID,
					tag:     tag,
					subtype: delimitedEndOfSubmessageSubtype,
				}))
				continue
			}
			t.encodedTags = append(t.encodedTags, t.addEncodedTag(encodedTag{
				tagID:   parentID,
				tag:     tag,
				subtype: delimitedStringSubtype,
			}))
			t.getBuffer(nodeID{parentID, field}, delimitedBuffer).Push(buf)
		case protoStartGroupType, protoEndGroupType:
			t.encodedTags = append(t.encodedTags, t.addEncodedTag(encodedTag{
				tagID: parentID,
				tag:   tag,
			}))
		default:
			return fmt.Errorf("unknown proto wire tag: 0x%x", tag)
		}
	}
	return nil
}

func isProtoMessage(rec []byte) bool {
	if len(rec) == 0 {
		return false
	}
	rd := bytes.NewReader(rec)
	var groupStack []uint64
	for rd.Len() > 0 {
		tag, err := binary.ReadUvarint(rd)
		if err != nil {
			return false
		}

		field := tag >> 3
		if field == 0 {
			return false
		}

		switch protoWireType(tag & 7) {
		case protoVarintType:
			if _, err := binary.ReadUvarint(rd); err != nil {
				return false
			}
		case protoFixed32Type:
			if rd.Len() < 4 {
				return false
			}
			rd.Seek(4, io.SeekCurrent)
		case protoFixed64Type:
			if rd.Len() < 8 {
				return false
			}
			rd.Seek(8, io.SeekCurrent)
		case protoBytesType:
			size, err := binary.ReadUvarint(rd)
			if err != nil {
				return false
			} else if rd.Len() < int(size) {
				return false
			}
			rd.Seek(int64(size), io.SeekCurrent)
		case protoStartGroupType:
			groupStack = append(groupStack, field)
		case protoEndGroupType:
			if len(groupStack) == 0 || groupStack[len(groupStack)-1] != field {
				return false
			}
			groupStack = groupStack[:len(groupStack)-1]
		default:
			return false
		}
	}
	return len(groupStack) == 0
}

const maxTransitions = 63

// Encode implements part of the recordWriter interface.
func (t *transposedChunkWriter) Encode() ([]byte, error) {
	if err := t.Close(); err != nil {
		return nil, fmt.Errorf("closing transposedChunkWriter: %v", err)
	}

	// TODO(schroederc): split buffers into multiple buckets
	data, err := newCompressor(t.opts)
	if err != nil {
		return nil, err
	}
	bufferIndices := make(map[nodeID]int)
	var bufferIdx int
	var bufferSizes []int
	for _, bufs := range t.data {
		for _, buf := range bufs {
			bufferIdx++
			bufferIndices[buf.id] = bufferIdx
			bufferSizes = append(bufferSizes, buf.writer.Len())
			if _, err := io.Copy(data, buf.writer); err != nil {
				return nil, fmt.Errorf("compressing buffer: %v", err)
			}
		}
	}
	if t.nonProtoLengths.Len() > 0 {
		bufferSizes = append(bufferSizes, t.nonProtoLengths.Len())
		if _, err := io.Copy(data, &t.nonProtoLengths); err != nil {
			return nil, fmt.Errorf("compressing non_proto_length: %v", err)
		}
	}
	if err := data.Close(); err != nil {
		return nil, err
	}

	states, ts, init, err := t.buildStateMachine(bufferIndices)
	if err != nil {
		return nil, err
	}

	buf := bytes.NewBuffer(nil)
	buf.WriteByte(byte(t.opts.compressionType()))

	// Encode header
	hdr, err := newCompressor(t.opts)
	if err != nil {
		return nil, err
	} else if err := t.encodeHeader(hdr, states, init, data.Len(), bufferSizes); err != nil {
		return nil, err
	} else if err := hdr.Close(); err != nil {
		return nil, err
	} else if _, err := writeUvarint(buf, uint64(hdr.Len())); err != nil {
		return nil, fmt.Errorf("writing header_length: %v", err)
	} else if _, err := hdr.WriteTo(buf); err != nil {
		return nil, err
	}

	// Encode data bucket
	if _, err := data.WriteTo(buf); err != nil {
		return nil, err
	}

	// Encode transitions
	transitions, err := newCompressor(t.opts)
	if err != nil {
		return nil, err
	} else if _, err := transitions.Write(ts); err != nil {
		return nil, err
	} else if err := transitions.Close(); err != nil {
		return nil, err
	} else if _, err := transitions.WriteTo(buf); err != nil {
		return nil, err
	}

	return buf.Bytes(), nil
}

type stateInfo struct {
	tag  uint64
	next int

	subtype   tagSubtype
	bufferIdx int
}

func (t *transposedChunkWriter) buildStateMachine(bufferIndices map[nodeID]int) (states []stateInfo, transitions []byte, initialState int, err error) {
	if len(t.encodedTags) == 0 {
		states = []stateInfo{{}} // no-op state machine
		return
	}

	possibleNexts := make(map[int]map[int]struct{})
	possiblePrevs := make(map[int]map[int]struct{})
	for _, pos := range t.encodedTags {
		possiblePrevs[pos] = make(map[int]struct{})
		possibleNexts[pos] = make(map[int]struct{})
	}
	for i, pos := range t.encodedTags {
		if i < len(t.encodedTags)-1 {
			prev := t.encodedTags[i+1]
			possiblePrevs[pos][prev] = struct{}{}
			possibleNexts[prev][pos] = struct{}{}
		}
	}
	possibleNexts[t.encodedTags[0]][len(t.encodedTagPos)] = struct{}{} // halt

	states = make([]stateInfo, len(t.encodedTagPos)+len(t.encodedTagPos)/maxTransitions+1)
	for et, pos := range t.encodedTagPos {
		// Determine the tag's encoding and associated buffer.
		tag := et.tag
		nodeID := nodeID{et.tagID, uint32(et.tag >> 3)}
		buffer := bufferIndices[nodeID]
		if et.tagID == nonProtoTag || et.tagID == startOfMessageTag {
			tag = uint64(et.tagID)
		} else if protoWireType(et.tag&7) == protoBytesType {
			if et.subtype == delimitedEndOfSubmessageSubtype {
				tag = ((tag >> 3) << 3) | uint64(protoSubmessageType)
			} else if et.subtype == delimitedStartOfSubmessageSubtype {
				tag = uint64(startOfSubmessageTag)
			}
		}

		// Clear any unused buffer index.
		if et.tagID != nonProtoTag && !(validProtoTag(tag) && hasDataBuffer(tag, et.subtype)) {
			// don't read a buffer (buffer is set because field could be encoded as a
			// different type elsewhere)
			buffer = 0
		}

		// Determine the smallest id the set of subsequent nodes.  This will be used
		// as the state's static `next` pointer.  Transition byte offsets will be
		// used to jump to other nodes.
		var minNext int
		for next := range possibleNexts[pos] {
			minNext = next
			break
		}
		for next := range possibleNexts[pos] {
			if minNext > next {
				minNext = next
			}
		}

		states[posToIndex(pos)] = stateInfo{
			tag:       tag,
			subtype:   et.subtype,
			next:      posToIndex(minNext),
			bufferIdx: buffer,
		}
	}

	// TODO(schroederc): better optimize the state machine
	// Cheat the state machine building to make the initial implementation
	// simpler.  Instead of splitting nodes to handle out-degrees greater than the
	// maximum number of transitions, just add a no-op state periodically that can
	// be used to trampoline to any later node.
	for i := 0; i < len(states); i += maxTransitions + 1 {
		states[i] = stateInfo{next: i + 1} // no-op jump state
	}

	// Mark implicit states
	for pos, nexts := range possibleNexts {
		if len(nexts) <= 1 {
			idx := posToIndex(pos)
			if states[idx].next < len(states) {
				states[idx].next += len(states) // mark implicit
			}
		}
	}

	// Encode each transition in the state machine.
	for i := len(t.encodedTags) - 1; i > 0; i-- {
		pos := t.encodedTags[i]
		if len(possibleNexts[pos]) > 1 {
			// Next state is ambiguous.  Determine the next state's offset to the
			// current index.  If it is greater than a single transition byte can
			// handle, trampoline through the no-op states created above.
			idx := posToIndex(pos)
			base := states[idx].next
			offset := posToIndex(t.encodedTags[i-1]) - base
			if offset > maxTransitions {
				jump := (base/(maxTransitions+1))*(maxTransitions+1) + maxTransitions + 1
				transitions = append(transitions, byte(jump-base)<<2)
				offset -= (jump - base)
				for offset > maxTransitions {
					jump += maxTransitions + 1
					transitions = append(transitions, byte(maxTransitions)<<2)
					offset -= (maxTransitions + 1)
				}
				offset--
			}

			transitions = append(transitions, byte(offset)<<2)
		}
	}

	initialState = posToIndex(t.encodedTags[len(t.encodedTags)-1])
	return
}

func posToIndex(pos int) int { return pos + 1 + pos/maxTransitions }

func (t *transposedChunkWriter) encodeHeader(c compressor, states []stateInfo, initialState int, bucketSize int, bufferSizes []int) error {
	// TODO(schroederc): split buffers into multiple buckets
	const numBuckets = 1
	if _, err := writeUvarint(c, numBuckets); err != nil {
		return fmt.Errorf("writing num_buckets: %v", err)
	}

	if _, err := writeUvarint(c, uint64(len(bufferSizes))); err != nil {
		return fmt.Errorf("writing num_buffers: %v", err)
	}

	if _, err := writeUvarint(c, uint64(bucketSize)); err != nil {
		return fmt.Errorf("writing bucket_size: %v", err)
	}
	for i, size := range bufferSizes {
		if _, err := writeUvarint(c, uint64(size)); err != nil {
			return fmt.Errorf("writing buffer_size[%d]: %v", i, err)
		}
	}

	// Encode states
	if _, err := writeUvarint(c, uint64(len(states))); err != nil {
		return fmt.Errorf("writing num_states: %v", err)
	}
	// States are split into 4 parallel buffers.
	var tags, indices, subtypes, buffers bytes.Buffer
	var numWithBuffers int
	for _, s := range states {
		writeUvarint(&tags, s.tag)
		writeUvarint(&indices, uint64(s.next))
		if validProtoTag(s.tag) && hasSubtype(s.tag) {
			subtypes.WriteByte(byte(s.subtype))
		}
		if s.bufferIdx > 0 {
			writeUvarint(&buffers, uint64(s.bufferIdx-1))
			numWithBuffers++
		}
	}
	if _, err := tags.WriteTo(c); err != nil {
		return err
	} else if _, err := indices.WriteTo(c); err != nil {
		return err
	} else if _, err := subtypes.WriteTo(c); err != nil {
		return err
	} else if _, err := buffers.WriteTo(c); err != nil {
		return err
	}

	if _, err := writeUvarint(c, uint64(initialState)); err != nil {
		return fmt.Errorf("writing initial_state: %v", err)
	}

	return nil
}

func writeUvarint(w io.Writer, n uint64) (int, error) {
	var buf [binary.MaxVarintLen64]byte
	return w.Write(buf[:binary.PutUvarint(buf[:], n)])
}
