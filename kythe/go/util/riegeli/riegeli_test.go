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
	"encoding/hex"
	"fmt"
	"io"
	"testing"
)

func TestWriteEmpty(t *testing.T) {
	var buf bytes.Buffer
	NewWriter(&buf, nil).Flush()

	// The standard Riegeli file header
	expected := []byte{
		0x83, 0xaf, 0x70, 0xd1, 0x0d, 0x88, 0x4a, 0x3f,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x91, 0xba, 0xc2, 0x3c, 0x92, 0x87, 0xe1, 0xa9,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0xe1, 0x9f, 0x13, 0xc0, 0xe9, 0xb1, 0xc3, 0x72,
		0x73, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	}
	if found := buf.Bytes(); !bytes.Equal(found, expected) {
		t.Errorf("Found: %s; expected: %s", hex.EncodeToString(found), hex.EncodeToString(expected))
	}
}

func TestReadWriteStringsDefaults(t *testing.T) { testReadWriteStrings(t, nil) }

func TestReadWriteStringsUncompressed(t *testing.T) {
	testReadWriteStrings(t, &WriterOptions{Compression: NoCompression})
}

func TestReadWriteStringsBrotli(t *testing.T) {
	testReadWriteStrings(t, &WriterOptions{Compression: BrotliCompression(-1)})
}

func TestReadWriteStringsZSTD(t *testing.T) {
	testReadWriteStrings(t, &WriterOptions{Compression: ZSTDCompression(-1)})
}

func writeStrings(t *testing.T, opts *WriterOptions, n int) *bytes.Buffer {
	var buf bytes.Buffer
	wr := NewWriter(&buf, opts)

	for i := 0; i < n; i++ {
		if err := wr.Put([]byte(fmt.Sprintf("%d", i))); err != nil {
			t.Fatalf("Error Put(%d): %v", i, err)
		}
	}
	if err := wr.Flush(); err != nil {
		t.Fatalf("Flush error: %v", err)
	}
	return &buf
}

func testReadWriteStrings(t *testing.T, opts *WriterOptions) {
	const N = 1e5
	buf := writeStrings(t, opts, N)
	rd := NewReader(bytes.NewReader(buf.Bytes()))
	for i := 0; i < N; i++ {
		rec, err := rd.Next()
		if err != nil {
			t.Fatalf("Read error: %v", err)
		} else if string(rec) != fmt.Sprintf("%d", i) {
			t.Errorf("Found: %s; expected: %d;", hex.EncodeToString(rec), i)
		}
	}
	rec, err := rd.Next()
	if err != io.EOF {
		t.Errorf("Unexpected final Read: %q %v", hex.EncodeToString(rec), err)
	}
}

func TestEmptyRecord(t *testing.T) {
	var buf bytes.Buffer
	wr := NewWriter(&buf, nil)

	if err := wr.Put([]byte{}); err != nil {
		t.Fatalf("Error writing empty record: %v", err)
	} else if err := wr.Flush(); err != nil {
		t.Fatalf("Flush error: %v", err)
	}

	rd := NewReader(bytes.NewReader(buf.Bytes()))
	if rec, err := rd.Next(); err != nil {
		t.Fatalf("Error reading empty record: %v", err)
	} else if len(rec) != 0 {
		t.Fatalf("Found non-empty record: %v", rec)
	}

	if rec, err := rd.Next(); err != io.EOF {
		t.Fatalf("Unexpected Next record/error: %v %v", rec, err)
	}
}

func TestReaderSeekRecords(t *testing.T) {
	const N = 1e4
	buf := writeStrings(t, nil, N)

	rd := NewReadSeeker(bytes.NewReader(buf.Bytes()))
	lastIndex := int64(-1)
	var positions []RecordPosition
	for i := 0; i < N; i++ {
		pos, err := rd.Position()
		if err != nil {
			t.Fatalf("Error getting position: %v", err)
		} else if _, err := rd.Next(); err != nil {
			t.Fatalf("Error reading sequentially: %v", err)
		}
		positions = append(positions, pos)
		idx := pos.index()
		if lastIndex >= idx {
			t.Errorf("Position not monotonically increasing: %d >= %d", lastIndex, idx)
		}
		lastIndex = idx
	}
	if rec, err := rd.Next(); err != io.EOF {
		t.Fatalf("Unexpected Next record/error: %v %v", rec, err)
	}

	// Read all records by seeking to each position in reverse order
	for i := int(N - 1); i >= 0; i-- {
		p := positions[i]
		if err := rd.SeekToRecord(p); err != nil {
			t.Fatalf("Error seeking to record %d at %v: %v", i, p, err)
		}
		rec, err := rd.Next()
		if err != nil {
			t.Fatalf("Read error at %v: %v", p, err)
		} else if string(rec) != fmt.Sprintf("%d", i) {
			t.Errorf("At %v found: %s; expected: %d;", p, hex.EncodeToString(rec), i)
		}
	}
}

func TestReaderSeekKnownPositions(t *testing.T) {
	const N = 1e4
	buf := writeStrings(t, nil, N)

	rd := NewReadSeeker(bytes.NewReader(buf.Bytes()))
	lastIndex := int64(-1)
	var positions []RecordPosition
	for i := 0; i < N; i++ {
		pos, err := rd.Position()
		if err != nil {
			t.Fatalf("Error getting position: %v", err)
		} else if _, err := rd.Next(); err != nil {
			t.Fatalf("Error reading sequentially: %v", err)
		}
		positions = append(positions, pos)
		idx := pos.index()
		if lastIndex >= idx {
			t.Errorf("Position not monotonically increasing: %d >= %d", lastIndex, idx)
		}
		lastIndex = idx
	}
	if rec, err := rd.Next(); err != io.EOF {
		t.Fatalf("Unexpected Next record/error: %v %v", rec, err)
	}

	// Read all records by seeking to each position in reverse order
	for i := int(N - 1); i >= 0; i-- {
		p := positions[i]
		if err := rd.Seek(p.index()); err != nil {
			t.Fatalf("Error seeking to record %d at %v (%d): %v", i, p, p.index(), err)
		}
		rec, err := rd.Next()
		if err != nil {
			t.Fatalf("Read error at %v: %v", p, err)
		} else if string(rec) != fmt.Sprintf("%d", i) {
			t.Errorf("At %v found: %s; expected: %d;", p, hex.EncodeToString(rec), i)
		}
	}
}

func TestReaderSeekAllPositions(t *testing.T) {
	const N = 1e4
	buf := writeStrings(t, nil, N)

	rd := NewReadSeeker(bytes.NewReader(buf.Bytes()))

	// Ensure every byte position is seekable
	var expected int
	for i := 0; i < buf.Len(); i++ {
		if err := rd.Seek(int64(i)); err != nil {
			t.Fatalf("Error seeking to %d/%d: %v", i, buf.Len(), err)
		}
		rec, err := rd.Next()
		if expected == N-1 {
			if err != io.EOF {
				t.Fatalf("Read past end of file at %d: %v %v", i, rec, err)
			}
		} else if err != nil {
			t.Fatalf("Read error at %d/%d: %v; expected: %d", i, buf.Len(), err, expected)
		}

		if expected != N-1 && string(rec) != fmt.Sprintf("%d", expected) {
			expected++
			if string(rec) != fmt.Sprintf("%d", expected) {
				t.Fatalf("At %d/%d found: %s; expected: %d;", i, buf.Len(), hex.EncodeToString(rec), expected)
			}
		}
	}

	if expected != N-1 {
		t.Fatalf("Failed to read all known records: %d != %d", expected, int(N)-1)
	}
}

// TODO(schroederc): test transposed chunks
// TODO(schroederc): test RecordsMetadata
// TODO(schroederc): test padding
