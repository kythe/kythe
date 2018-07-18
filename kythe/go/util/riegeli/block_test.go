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
	"encoding/hex"
	"io"
	"io/ioutil"
	"testing"
)

func TestBlockWriter_fullBlock(t *testing.T) {
	var buf bytes.Buffer
	w := &blockWriter{w: &buf}

	// Write a full block
	chunk := bytes.Repeat([]byte{0}, usableBlockSize)
	n, err := w.WriteChunk(chunk)
	if err != nil {
		t.Fatal(err)
	} else if expected := blockSize; n != expected {
		t.Fatalf("Unexpected write size: found: %d; expected: %d", n, expected)
	} else if buf.Len() != expected {
		t.Fatalf("Unexpected output size: found: %d; expected: %d", buf.Len(), expected)
	}

	r := bytes.NewReader(buf.Bytes())
	h, err := decodeBlockHeader(r)
	if err != nil {
		t.Fatalf("Error decoding block header: %v", err)
	} else if expected := 0; h.PreviousChunk != uint64(expected) {
		t.Fatalf("Unexpected PreviousChunk offset: found: %d; expected: %d", h.PreviousChunk, expected)
	} else if expected := blockSize; h.NextChunk != uint64(expected) {
		t.Fatalf("Unexpected NextChunk offset: found: %d; expected: %d", h.NextChunk, expected)
	}

	rest, err := ioutil.ReadAll(r)
	if err != nil {
		t.Fatalf("Error reading chunk: %v", err)
	} else if !bytes.Equal(rest, chunk) {
		t.Fatalf("Unexpected chunk bytes: found: %d; expected: %d", rest, chunk)
	}
}

func TestBlockWriter_crossBlock(t *testing.T) {
	var buf bytes.Buffer
	w := &blockWriter{w: &buf}

	// Write almost a full block
	chunk := bytes.Repeat([]byte{0}, usableBlockSize-10)
	n, err := w.WriteChunk(chunk)
	if err != nil {
		t.Fatal(err)
	} else if expected := blockHeaderSize + len(chunk); n != expected {
		t.Fatalf("Unexpected write size: found: %d; expected: %d", n, expected)
	} else if buf.Len() != expected {
		t.Fatalf("Unexpected output size: found: %d; expected: %d", buf.Len(), expected)
	}

	// Write another chunk that crosses a block header boundary
	n, err = w.WriteChunk(chunk)
	if err != nil {
		t.Fatal(err)
	} else if expected := blockHeaderSize + len(chunk); n != expected {
		t.Fatalf("Unexpected write size: found: %d; expected: %d", n, expected)
	} else if buf.Len() != expected*2 {
		t.Fatalf("Unexpected output size: found: %d; expected: %d", buf.Len(), expected*2)
	}

	r := bytes.NewReader(buf.Bytes())
	// Decode first block header
	h, err := decodeBlockHeader(r)
	if err != nil {
		t.Fatalf("Error decoding block header: %v", err)
	} else if expected := 0; h.PreviousChunk != uint64(expected) {
		t.Fatalf("Unexpected PreviousChunk offset: found: %d; expected: %d", h.PreviousChunk, expected)
	} else if expected := blockHeaderSize + len(chunk); h.NextChunk != uint64(expected) {
		t.Fatalf("Unexpected NextChunk offset: found: %d; expected: %d", h.NextChunk, expected)
	}

	// Read first chunk
	chunkRead := make([]byte, len(chunk))
	if _, err := io.ReadFull(r, chunkRead); err != nil {
		t.Fatalf("Error reading chunk: %v", err)
	} else if !bytes.Equal(chunkRead, chunk) {
		t.Fatalf("Unexpected chunk bytes: found: %d; expected: %d", chunkRead, chunk)
	}

	// Read part of second chunk
	if _, err := io.ReadFull(r, chunkRead[:10]); err != nil {
		t.Fatalf("Error reading chunk: %v", err)
	}

	// Decode second block header
	h, err = decodeBlockHeader(r)
	if err != nil {
		t.Fatalf("Error decoding block header: %v", err)
	} else if expected := 10; h.PreviousChunk != uint64(expected) {
		t.Fatalf("Unexpected PreviousChunk offset: found: %d; expected: %d", h.PreviousChunk, expected)
	} else if expected := blockHeaderSize + len(chunk) - 10; h.NextChunk != uint64(expected) {
		t.Fatalf("Unexpected NextChunk offset: found: %d; expected: %d", h.NextChunk, expected)
	}

	// Read rest of second chunk
	if _, err := io.ReadFull(r, chunkRead[10:]); err != nil {
		t.Fatalf("Error reading chunk: %v", err)
	} else if !bytes.Equal(chunkRead, chunk) {
		t.Fatalf("Unexpected chunk bytes: found: %d; expected: %d", chunkRead, chunk)
	}
}

func TestBlockWriter_singleChunk(t *testing.T) {
	var buf bytes.Buffer
	w := &blockWriter{w: &buf}

	// Write a small chunk
	chunk := bytes.Repeat([]byte{0}, 1024)
	n, err := w.WriteChunk(chunk)
	if err != nil {
		t.Fatal(err)
	} else if expected := blockHeaderSize + len(chunk); n != expected {
		t.Fatalf("Unexpected write size: found: %d; expected: %d", n, expected)
	} else if buf.Len() != expected {
		t.Fatalf("Unexpected output size: found: %d; expected: %d", buf.Len(), expected)
	}

	r := bytes.NewReader(buf.Bytes())
	h, err := decodeBlockHeader(r)
	if err != nil {
		t.Fatalf("Error decoding block header: %v", err)
	} else if expected := 0; h.PreviousChunk != uint64(expected) {
		t.Fatalf("Unexpected PreviousChunk offset: found: %d; expected: %d", h.PreviousChunk, expected)
	} else if expected := blockHeaderSize + len(chunk); h.NextChunk != uint64(expected) {
		t.Fatalf("Unexpected NextChunk offset: found: %d; expected: %d", h.NextChunk, expected)
	}

	rest, err := ioutil.ReadAll(r)
	if err != nil {
		t.Fatalf("Error reading chunk: %v", err)
	} else if !bytes.Equal(rest, chunk) {
		t.Fatalf("Unexpected chunk bytes: found: %d; expected: %d", rest, chunk)
	}
}

func TestBlockWriter_multipleChunks(t *testing.T) {
	var buf bytes.Buffer
	w := &blockWriter{w: &buf}

	// Write multiple chunks
	chunk := bytes.Repeat([]byte{0}, 1024)
	numChunks := usableBlockSize / len(chunk)
	n, err := w.WriteChunk(chunk)
	if err != nil {
		t.Fatal(err)
	} else if expected := blockHeaderSize + len(chunk); n != expected {
		t.Fatalf("Unexpected write size: found: %d; expected: %d", n, expected)
	} else if buf.Len() != expected {
		t.Fatalf("Unexpected output size: found: %d; expected: %d", buf.Len(), expected)
	}
	for i := 1; i < numChunks; i++ {
		n, err := w.WriteChunk(chunk)
		if err != nil {
			t.Fatal(err)
		} else if expected := len(chunk); n != expected { // doesn't include blockHeaderSize overhead
			t.Fatalf("Unexpected write size: found: %d; expected: %d", n, expected)
		}
	}

	r := bytes.NewReader(buf.Bytes())
	h, err := decodeBlockHeader(r)
	if err != nil {
		t.Fatalf("Error decoding block header: %v", err)
	} else if expected := 0; h.PreviousChunk != uint64(expected) {
		t.Fatalf("Unexpected PreviousChunk offset: found: %d; expected: %d", h.PreviousChunk, expected)
	} else if expected := blockHeaderSize + len(chunk); h.NextChunk != uint64(expected) {
		t.Fatalf("Unexpected NextChunk offset: found: %d; expected: %d", h.NextChunk, expected)
	}
}

func TestBlockReader_sequential(t *testing.T) {
	var buf bytes.Buffer
	w := &blockWriter{w: &buf}

	// Write multiple chunks across multiple blocks
	const numChunks = 255
	const chunkSize = usableBlockSize / 10
	for i := 0; i < numChunks; i++ {
		chunk := bytes.Repeat([]byte{byte(i)}, chunkSize)
		if _, err := w.WriteChunk(chunk); err != nil {
			t.Fatal(err)
		}
	}

	r := &blockReader{r: bytes.NewReader(buf.Bytes())}
	for i := 0; i < numChunks; i++ {
		expected := bytes.Repeat([]byte{byte(i)}, chunkSize)
		found := make([]byte, chunkSize)
		if _, err := io.ReadFull(r, found); err != nil {
			t.Fatal(err)
		} else if !bytes.Equal(expected, found) {
			t.Fatalf("Unexpected chunk bytes: found: %d; expected: %d", found, expected)
		}
	}

	found := make([]byte, chunkSize)
	if n, err := r.Read(found); err != io.EOF {
		t.Fatalf("Unexpected read of %d bytes past end (err=%v): %s", n, err, hex.EncodeToString(found))
	}
}
