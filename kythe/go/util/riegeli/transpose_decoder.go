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
	"encoding/hex"
	"fmt"
	"io"
	"io/ioutil"
)

// Encoding format documentation:
//   - https://github.com/google/riegeli/blob/master/doc/riegeli_records_file_format.md#transposed-chunk-with-records
//   - https://github.com/google/riegeli/blob/master/riegeli/chunk_encoding/transpose_encoder.h

// newTransposedRecordReader returns a recordReader for the given transposed
// chunk.
func newTransposedRecordReader(c *chunk) (recordReader, error) {
	r := bytes.NewReader(c.Data)

	// Format of c.Data:
	//  - Compression type byte
	//  - Header length (compressed length of Header)
	//  - Header (compressed) -- see parseTransposeStateMachine below
	//  - Record data buckets (each compressed)
	//  - State machine transitions (compressed, 1 byte per transition)

	compressionByte, err := r.ReadByte()
	if err != nil {
		return nil, fmt.Errorf("reading compression type: %v", err)
	}
	compression := compressionType(compressionByte)

	headerSize, err := binary.ReadUvarint(r)
	if err != nil {
		return nil, fmt.Errorf("reading header size: %v", err)
	}

	headerBuf := make([]byte, headerSize)
	if _, err := io.ReadFull(r, headerBuf); err != nil {
		return nil, fmt.Errorf("reading header: %v", err)
	}

	if compression != noCompression {
		headerBuf, err = decompress(bytes.NewReader(headerBuf), compression)
		if err != nil {
			return nil, fmt.Errorf("decompressing header: %v", err)
		}
	}

	machine, err := parseTransposeStateMachine(r, bytes.NewReader(headerBuf), compression)
	if err != nil {
		return nil, err
	}

	machine.numRecords = int(c.Header.NumRecords)
	if c.Header.ChunkType == fileMetadataChunkType {
		machine.numRecords = 1
	}

	records, err := machine.execute()
	if err != nil {
		return nil, fmt.Errorf("transpose state machine error: %v", err)
	}

	return &fixedRecordReader{records: records}, nil
}

// A stateMachine is the decoded structure of a Riegeli transposed chunk.  The
// state machine consists of N states, each with a tag, subtype, and pointer to
// another state (by index).  Each state may also be optionally associated with a
// data buffer.  The machine's execution starts with the `initial` state and
// moves to each state's `next` pointer after interpreting its tag/subtype.
// A state's move to the next state may be marked as `implicit`.  Whether a state's
// move is implicit or not either increments or decrements an iteration counter.
// When a state move occurs when that counter is zero, an extra transitional byte
// is read from `transitions` to indicate an offset to add the current state's
// index before continuing.
//
// Execution of a stateMachine leads to a finite stream of records read from
// data in the machine's buffers (as well as inlined data within state tags).
//
// See parseTransposeStateMachine for the decoding of these state machines.
// See (*machine).execute() for the execution implementation.
type stateMachine struct {
	initial     int
	states      []stateNode
	buffers     []byteReader
	transitions []byte
	numRecords  int

	// Sequence of varints for non-proto record sizes (only set when at least 1
	// record isn't tag-encoded.)
	nonProtoLengths byteReader
}

type stateNode struct {
	// index is the index of this state within a stateMachine's list of states.
	index int

	// A state's tag and subtype determine how to interpret the state's data and
	// buffer.  Certain tag types may inline data within its tag and subtype.
	// The tag value is an overload of the protocol buffer wire-encoding key
	// format.
	tag     uint64
	subtype tagSubtype

	// next is the index of the following state in a stateMachine's execution.  The
	// transition may be marked as implicit if the state does not require reading a
	// transitional byte from a stateMachine's `transitions`.
	next     int
	implicit bool

	// data is the state's embedded data.  The state's tag/subtype determine how
	// this is interpreted.
	data []byte

	// buffer is a pointer to a shared data buffer for this state (and possible
	// others).  The state's tag/subtype determine how this is interpreted.
	buffer byteReader
}

func parseTransposeStateMachine(src io.Reader, hdr byteReader, compressionType compressionType) (*stateMachine, error) {
	// - Header (hdr) format:
	//   - Number of separately compressed buckets that data buffers are split into [num_buckets]
	//   - Number of data buffers [num_buffers]
	//   - Array of "num_buckets" varints: sizes of buckets (compressed size)
	//   - Array of "num_buffers" varints: lengths of buffers
	//   - Number of state machine states [num_state]
	//   - States encoded in 4 blocks:
	//     - Array of "num_state" Tags/ReservedIDs
	//     - Array of "num_state" next state indices
	//     - Array of subtypes (for all tags where applicable)
	//     - Array of data buffer indices (for all tags/subtypes where applicable)
	//   - Initial state index

	machine := &stateMachine{}

	// Read the number of "buckets" of data that should be read from src.
	numBuckets, err := binary.ReadUvarint(hdr)
	if err != nil {
		return nil, fmt.Errorf("reading num_buckets: %v", err)
	}

	// Read the number of "buffers" that are encoded with the "buckets" read from src.
	numBuffers, err := binary.ReadUvarint(hdr)
	if err != nil {
		return nil, fmt.Errorf("reading num_buffers: %v", err)
	} else if numBuffers == 0 {
		return nil, fmt.Errorf("too few buffers: %d", numBuffers)
	}

	// Read and decompress each bucket of data from `src`
	buckets := make([]byteReader, numBuckets)
	for i := 0; i < int(numBuckets); i++ {
		size, err := binary.ReadUvarint(hdr)
		if err != nil {
			return nil, fmt.Errorf("reading bucket[%d] size: %v", i, err)
		}
		b := make([]byte, size)
		if _, err := io.ReadFull(src, b); err != nil {
			return nil, fmt.Errorf("reading bucket[%d]: %v", i, err)
		}
		if compressionType != noCompression {
			b, err = decompress(bytes.NewReader(b), compressionType)
			if err != nil {
				return nil, fmt.Errorf("decompressing bucket[%d]: %v", i, err)
			}
		}
		buckets[i] = bytes.NewReader(b)
	}

	// Split the buckets into the actual data buffers that will be interpreted by
	// the stateMachine during execution.
	machine.buffers = make([]byteReader, numBuffers)
	for i, bucket := 0, 0; i < int(numBuffers); i++ {
		size, err := binary.ReadUvarint(hdr)
		if err != nil {
			return nil, fmt.Errorf("reading buffer[%d] size: %v", i, err)
		}
		buf := make([]byte, size)
		var readBuffer bool
		// Read the buffer from the next available bucket.
		for ; bucket < len(buckets); bucket++ {
			if _, err := io.ReadFull(buckets[bucket], buf); err == io.EOF {
				continue
			} else if err != nil {
				return nil, fmt.Errorf("reading buffer[%d] from bucket[%d]: %v", i, bucket, err)
			}
			readBuffer = true
			break
		}
		if !readBuffer {
			return nil, fmt.Errorf("reading buffer[%d]: ran out of bucket data", i)
		}
		machine.buffers[i] = bytes.NewReader(buf)
	}

	// Ensure all data is read from the buckets.
	for bucket, rd := range buckets {
		if _, err := rd.ReadByte(); err != io.EOF {
			return nil, fmt.Errorf("trailing bucket data: bucket=%d/%d", bucket, numBuckets)
		}
	}

	// Read the number of states within the stateMachine.
	numStates, err := binary.ReadUvarint(hdr)
	if err != nil {
		return nil, fmt.Errorf("reading num_states: %v", err)
	}
	machine.states = make([]stateNode, numStates)

	// Read the tag for each state.
	tags, err := readVarintArray(hdr, int(numStates))
	if err != nil {
		return nil, fmt.Errorf("reading state tags: %v", err)
	}
	for i, tag := range tags {
		machine.states[i].tag = tag
		machine.states[i].index = i
	}

	// Read the indices for each state's "next" state node.
	indices, err := readVarintArray(hdr, int(numStates))
	if err != nil {
		return nil, fmt.Errorf("reading state indices: %v", err)
	}
	for i, next := range indices {
		if next >= numStates {
			machine.states[i].implicit = true
			machine.states[i].next = int(next - numStates)

			if machine.states[i].next >= int(numStates) {
				return nil, fmt.Errorf("invalid state transition: %d (numStates: %d)", machine.states[i].next, numStates)
			}
		} else {
			machine.states[i].next = int(next)
		}
	}

	// Count and read the required number of subtype bytes.
	var numSubtypes int
	for _, tag := range tags {
		if validProtoTag(tag) && hasSubtype(tag) {
			numSubtypes++
		}
	}
	subtypes := make([]byte, numSubtypes)
	if _, err := io.ReadFull(hdr, subtypes); err != nil {
		return nil, fmt.Errorf("reading subtypes: %v", err)
	}

	// Pre-process each state tag/subtype:
	//   - Interpret protoSubmessageType tags as delimitedEndOfSubmessageSubtypes
	//   - Optionally associate each state tag with its subtype
	//   - Optionally associate each state with a data buffer
	var subtypeIdx int
	var hasNonProto bool
	for state := 0; state < int(numStates); state++ {
		tag := tags[state]
		switch tagID(tag) {
		case noOpTag:
			// nothing
		case nonProtoTag:
			bufferIdx, err := binary.ReadUvarint(hdr)
			if err != nil {
				return nil, fmt.Errorf("reading state[%d].buffer_index: %v", state, err)
			}
			machine.states[state].buffer = machine.buffers[bufferIdx]
			hasNonProto = true
		case startOfMessageTag:
		case startOfSubmessageTag:
		default:
			subtype := trivialSubtype

			// End of submessage is encoded as protoSubmessageType.
			if protoWireType(tag&7) == protoSubmessageType {
				tag -= protoSubmessageType - protoBytesType
				subtype = delimitedEndOfSubmessageSubtype
			}

			if hasSubtype(tag) {
				subtype = tagSubtype(subtypes[subtypeIdx])
				subtypeIdx++
			}

			if hasDataBuffer(tag, subtype) {
				bufferIdx, err := binary.ReadUvarint(hdr)
				if err != nil {
					return nil, fmt.Errorf("reading state[%d].buffer_index: %v", state, err)
				}
				machine.states[state].buffer = machine.buffers[bufferIdx]
			}
			machine.states[state].subtype = subtype

			var buf [binary.MaxVarintLen64]byte
			n := binary.PutUvarint(buf[:], tag)
			tagData := buf[:n]
			if protoWireType(tag&7) == protoVarintType && subtype >= varintInline0Subtype {
				tagData = append(tagData, byte(subtype-varintInline0Subtype))
			}
			machine.states[state].data = tagData
		}
	}
	if subtypeIdx != len(subtypes) {
		return nil, fmt.Errorf("not all subtypes used: %d/%d", subtypeIdx, len(subtypes))
	}

	if hasNonProto {
		machine.nonProtoLengths = machine.buffers[len(machine.buffers)-1]
	}

	// Read the state index for the initial stateMachine state.
	initState, err := binary.ReadUvarint(hdr)
	if err != nil {
		return nil, fmt.Errorf("reading initial_state: %v", err)
	}
	machine.initial = int(initState)

	// Ensure the full header has been read.
	leftover, err := ioutil.ReadAll(hdr)
	if len(leftover) != 0 || err != nil {
		return nil, fmt.Errorf("leftover header bytes (err: %v): %s", err, hex.EncodeToString(leftover))
	}

	// Read/decompress the transition bytes from the tail of `src`.
	if compressionType == noCompression {
		machine.transitions, err = ioutil.ReadAll(src)
	} else {
		machine.transitions, err = decompress(&trivialByteReader{src}, compressionType)
	}
	if err != nil {
		return nil, fmt.Errorf("reading transitions: %v", err)
	}

	return machine, nil
}

// execute will execute a stateMachine and return the sequence of records it
// interpreted as a result.
func (m *stateMachine) execute() ([][]byte, error) {
	var (
		// currentState is the current state to be interpreted
		currentState = m.states[m.initial]

		// numIters is the number of iterations before reading next transition byte
		numIters int

		// submessageStack is a stack of end positions for currently open submessages
		submessageStack []int
		// submessageStackData is the associated data for each submessage start
		submessageStackData [][]byte

		writer = &backwardWriter{} // currently open record being written

		records   = make([][]byte, m.numRecords) // all finished output records
		recordIdx = m.numRecords - 1
	)

	if currentState.implicit {
		numIters++
	}

	// TODO(schroederc): harden against adversarial state machines

	// Repeatedly interpret the currentState's tag and transition to the next
	// state until we've read all of m.transitions.
	for {
		switch tagID(currentState.tag) {
		case noOpTag:
			// do nothing
		case nonProtoTag:
			size, err := binary.ReadUvarint(m.nonProtoLengths)
			if err != nil {
				return nil, fmt.Errorf("reading non-proto length: %v", err)
			}
			rec := make([]byte, size)
			if _, err := io.ReadFull(currentState.buffer, rec); err != nil {
				return nil, fmt.Errorf("reading non-proto: %v", err)
			}
			records[recordIdx] = rec
			recordIdx--
		case startOfMessageTag:
			// We've finished a full record.  Add it to the output records and reset
			// the writer for the next record.
			if len(submessageStack) != 0 {
				return nil, fmt.Errorf("submessageStack still open: %v", submessageStack)
			}
			rec := make([]byte, writer.Len())
			io.ReadFull(writer, rec)
			records[recordIdx] = rec
			recordIdx--
			writer.Reset()
		case startOfSubmessageTag:
			// We've finished a submessage.  Pop the submessageStack and write both
			// the submesage's size and its associated tag data.
			if len(submessageStack) == 0 {
				return nil, fmt.Errorf("submessageStack underflow")
			}
			size := writer.Len() - submessageStack[len(submessageStack)-1]
			writer.PushUvarint(uint64(size))
			writer.Push(submessageStackData[len(submessageStackData)-1])
			submessageStack = submessageStack[:len(submessageStack)-1]
			submessageStackData = submessageStackData[:len(submessageStackData)-1]
		default:
			// The meat of the stateMachine.  Interpret the state based on its
			// protocol buffer wire type and append the current record writer.
			switch protoWireType(currentState.tag & 7) {
			case protoVarintType:
				subtype := currentState.subtype
				if subtype >= varintInline0Subtype {
					// Inlined varints are fully encoded as the state's data.
					writer.Push(currentState.data)
				} else {
					// Large varints have their size encoded in their subtype and its data
					// in the state's buffer.
					bufferSize := int(subtype-varint1Subtype) + 1
					buf := make([]byte, bufferSize)
					if _, err := io.ReadFull(currentState.buffer, buf); err != nil {
						return nil, fmt.Errorf("reading varint buffer: %v", err)
					}
					for i := 0; i < len(buf)-1; i++ {
						buf[i] |= 0x80
					}
					writer.Push(buf)
					writer.Push(currentState.data)
				}
			case protoFixed32Type:
				// Read a int32 from the state's buffer and append the state's data.
				num := make([]byte, 4)
				if _, err := io.ReadFull(currentState.buffer, num); err != nil {
					return nil, fmt.Errorf("reading buffer: %v", err)
				}
				writer.Push(num)
				writer.Push(currentState.data)
			case protoFixed64Type:
				// Read a int64 from the state's buffer and append the state's data.
				num := make([]byte, 8)
				if _, err := io.ReadFull(currentState.buffer, num); err != nil {
					return nil, fmt.Errorf("reading buffer: %v", err)
				}
				writer.Push(num)
				writer.Push(currentState.data)
			case protoBytesType, protoSubmessageType:
				switch currentState.subtype {
				case delimitedStringSubtype:
					// Read a varint-prefixed string from the state's buffer and append
					// the state's data.
					size, err := binary.ReadUvarint(currentState.buffer)
					if err != nil {
						return nil, fmt.Errorf("reading delimited string size: %v", err)
					}
					strData := make([]byte, size)
					if _, err := io.ReadFull(currentState.buffer, strData); err != nil {
						return nil, fmt.Errorf("reading delimited string data: %v", err)
					}

					writer.Push(strData)
					writer.PushUvarint(size)
					writer.Push(currentState.data)
				case delimitedEndOfSubmessageSubtype:
					// We're now at the end of a submessage.  We need to keep track of the
					// submessage's size as we write it (in reverse) so add the current
					// writer's size to the submessageStack.
					submessageStack = append(submessageStack, writer.Len())
					submessageStackData = append(submessageStackData, currentState.data)
				default:
					return nil, fmt.Errorf("unknown protoBytesType: %s", []byte{byte(currentState.subtype)})
				}
			case protoStartGroupType, protoEndGroupType:
				writer.Push(currentState.data)
			default:
				return nil, fmt.Errorf("unknown proto type: 0x%x", currentState.subtype)
			}
		}

		// Transition to next state
		currentState = m.states[currentState.next]

		if numIters == 0 {
			// Read a byte transition to move by an additional offset
			if len(m.transitions) == 0 {
				// Successful end of currentState machine
				break
			}
			trans := m.transitions[0]
			m.transitions = m.transitions[1:]
			offset := int(trans >> 2)
			currentState = m.states[currentState.index+offset]
			numIters = int(trans & 3)
			if currentState.implicit {
				numIters++
			}
		} else if !currentState.implicit {
			numIters--
		}
	}

	if writer.Len() != 0 {
		return nil, fmt.Errorf("unexpected leftover record bytes: %d", writer.Len())
	}

	// Ensure we read all data from the buffers.
	for i, b := range m.buffers {
		if leftover, err := ioutil.ReadAll(b); len(leftover) != 0 || err != nil {
			return nil, fmt.Errorf("buffer[%d] leftover (err: %v): %s", i, err, hex.EncodeToString(leftover))
		}
	}

	return records, nil
}

func readVarintArray(r io.ByteReader, size int) ([]uint64, error) {
	ns := make([]uint64, size)
	for i := 0; i < int(size); i++ {
		n, err := binary.ReadUvarint(r)
		if err != nil {
			return nil, fmt.Errorf("reading varint[%d]: %v", i, err)
		}
		ns[i] = n
	}
	return ns, nil
}

type trivialByteReader struct{ io.Reader }

// ReadByte implements the io.ByteReader interface.
func (t *trivialByteReader) ReadByte() (byte, error) {
	var buf [1]byte
	_, err := t.Read(buf[:])
	return buf[0], err
}
