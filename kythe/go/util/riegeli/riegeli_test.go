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
	"fmt"
	"io"
	"testing"

	"kythe.io/kythe/go/util/compare"
	"kythe.io/kythe/go/util/log"

	"google.golang.org/protobuf/proto"

	rtpb "kythe.io/kythe/go/util/riegeli/riegeli_test_go_proto"
	rmpb "kythe.io/third_party/riegeli/records_metadata_go_proto"
)

func TestParseOptions(t *testing.T) {
	tests := []string{
		"",
		"default",
		"brotli",
		"brotli:5",
		"transpose",
		"uncompressed",
		"zstd",
		"zstd:5",
		"snappy",
		"brotli,transpose",
		"transpose,uncompressed",
		"brotli:5,transpose",
		"chunk_size:524288",
	}

	for _, test := range tests {
		opts, err := ParseOptions(test)
		if err != nil {
			t.Errorf("ParseOptions error: %v", err)
			continue
		}

		if found := opts.String(); found != test {
			t.Errorf("Expected: %q; found: %q", test, found)
		}
	}
}

func TestWriteEmpty(t *testing.T) {
	var buf bytes.Buffer
	if err := NewWriter(&buf, nil).Close(); err != nil {
		t.Fatal(err)
	}

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

var testedOptions = []string{
	"default",
	"uncompressed",
	"brotli",
	"zstd",
	"snappy",

	"transpose",
	"uncompressed,transpose",
	"brotli,transpose",
}

func TestReadWriteNonProto(t *testing.T) {
	for _, test := range testedOptions {
		opts, err := ParseOptions(test)
		if err != nil {
			t.Fatal(err)
		}
		t.Run(test, func(t *testing.T) {
			t.Parallel()
			testReadWriteStrings(t, opts)
		})
	}
}

func TestReadWriteProto(t *testing.T) {
	for _, test := range testedOptions {
		opts, err := ParseOptions(test)
		if err != nil {
			t.Fatal(err)
		}
		t.Run(test, func(t *testing.T) {
			t.Parallel()
			testReadWriteProtos(t, opts)
		})
	}
}

func writeStrings(t *testing.T, opts *WriterOptions, n int) *bytes.Buffer {
	var buf bytes.Buffer
	wr := NewWriter(&buf, opts)

	for i := 0; i < n; i++ {
		if err := wr.Put([]byte(fmt.Sprintf("%d", i))); err != nil {
			t.Fatalf("Error Put(%d): %v", i, err)
		}
	}
	if err := wr.Close(); err != nil {
		t.Fatalf("Close error: %v", err)
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

func writeProtos(t *testing.T, opts *WriterOptions, n int) *bytes.Buffer {
	t.Helper()
	var buf bytes.Buffer
	wr := NewWriter(&buf, opts)
	for i := 0; i < n; i++ {
		if err := wr.PutProto(numToProto(i)); err != nil {
			t.Fatalf("Error PutProto(%d): %v", i, err)
		}
	}
	if err := wr.Close(); err != nil {
		t.Fatalf("Close error: %v", err)
	}
	return &buf
}

// numToProto constructs a *rtpb.Complex using a given integer as its field
// values and as counter for repeated field sizes.
func numToProto(i int) *rtpb.Complex {
	msg := &rtpb.Complex{
		Str:  proto.String(fmt.Sprintf("s%d", i)),
		I32:  proto.Int32(int32(i)),
		I64:  proto.Int64(int64(i)),
		Bits: []byte(fmt.Sprintf("b%d", i)),
		SimpleNested: &rtpb.Simple{
			Name: proto.String(fmt.Sprintf("name%d", i)),
		},
	}
	for j := 0; j < i%8; j++ {
		msg.Rep = append(msg.Rep, fmt.Sprintf("rep%d_%d", i, j))
		msg.Group = append(msg.Group, &rtpb.Complex_Group{
			GrpStr: proto.String(fmt.Sprintf("gs%d_%d", i, j)),
		})
	}
	for j, complexNested := 0, msg; j < i%100; j++ {
		nextLevel := &rtpb.Complex{Str: proto.String(fmt.Sprintf("cn%d_%d", i, j))}
		complexNested.ComplexNested, complexNested = nextLevel, nextLevel
	}
	return msg
}

func testReadWriteProtos(t *testing.T, opts *WriterOptions) {
	const N = 1e3
	buf := writeProtos(t, opts, N)
	log.Infof("Compressed size of %q: %d bytes", t.Name(), buf.Len())
	rd := NewReader(bytes.NewReader(buf.Bytes()))
	for i := 0; i < N; i++ {
		expected := numToProto(i)
		var found rtpb.Complex
		if err := rd.NextProto(&found); err != nil {
			t.Fatalf("Read error: %v", err)
		} else if diff := compare.ProtoDiff(&found, expected); diff != "" {
			t.Errorf("Unexpected record:  (-: found; +: expected)\n%s", diff)
		}
	}
}

func TestEmptyRecord(t *testing.T) {
	var buf bytes.Buffer
	wr := NewWriter(&buf, nil)

	if err := wr.Put([]byte{}); err != nil {
		t.Fatalf("Error writing empty record: %v", err)
	} else if err := wr.Close(); err != nil {
		t.Fatalf("Close error: %v", err)
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

func TestWriterSeek(t *testing.T) {
	const N = 1e3

	buf := bytes.NewBuffer(nil)
	wr := NewWriter(buf, nil)

	positions := make([]RecordPosition, N)
	for i := 0; i < N; i++ {
		positions[i] = wr.Position()
		if err := wr.PutProto(numToProto(i)); err != nil {
			t.Fatalf("Error PutProto(%d): %v", i, err)
		}
	}
	if err := wr.Close(); err != nil {
		t.Fatalf("Error Close: %v", err)
	}

	rd := NewReadSeeker(bytes.NewReader(buf.Bytes()))
	for i, p := range positions {
		if err := rd.SeekToRecord(p); err != nil {
			t.Fatalf("Error seeking to record %d at %v: %v", i, p, err)
		}

		expected := numToProto(i)
		var found rtpb.Complex
		if err := rd.NextProto(&found); err != nil {
			t.Fatalf("Read error: %v", err)
		} else if diff := compare.ProtoDiff(&found, expected); diff != "" {
			t.Errorf("Unexpected record:  (-: found; +: expected)\n%s", diff)
		}
	}
}

func TestReaderSeekRecords(t *testing.T) {
	const N = 1e4
	buf := writeStrings(t, &WriterOptions{}, N)

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
	buf := writeStrings(t, &WriterOptions{}, N)

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
	buf := writeStrings(t, &WriterOptions{}, N).Bytes()
	rd := NewReadSeeker(bytes.NewReader(buf))

	// Ensure every byte position is seekable
	var expected int
	for i := 0; i < len(buf); i++ {
		if err := rd.Seek(int64(i)); err != nil {
			t.Fatalf("Error seeking to %d/%d for %d: %v", i, len(buf), expected, err)
		}
		pos, err := rd.Position()
		if err != nil {
			t.Fatalf("Position error: %v", err)
		}
		rec, err := rd.Next()
		if expected == N-1 {
			if err != io.EOF {
				t.Fatalf("Read past end of file at %d (%v): %v %v", i, pos, rec, err)
			}
		} else if err != nil {
			t.Fatalf("Read error at %d/%d: %v; expected: %d", i, len(buf), err, expected)
		}

		if expected != N-1 && string(rec) != fmt.Sprintf("%d", expected) {
			expected++
			if string(rec) != fmt.Sprintf("%d", expected) {
				t.Fatalf("At %d/%d found: %s; expected: %d;", i, len(buf), string(rec), expected)
			}
		}
	}

	if expected != N-1 {
		t.Fatalf("Failed to read all known records: %d != %d", expected, int(N)-1)
	}
}

func TestRecordsMetadata(t *testing.T) {
	opts := &WriterOptions{
		Transpose:   true,
		Compression: BrotliCompression(4),
	}
	expected := &rmpb.RecordsMetadata{}
	expected.RecordWriterOptions = proto.String(opts.String())

	buf := writeStrings(t, opts, 128)
	rd := NewReader(bytes.NewReader(buf.Bytes()))

	found, err := rd.RecordsMetadata()
	if err != nil {
		log.Fatal(err)
	} else if diff := compare.ProtoDiff(found, expected); diff != "" {
		t.Errorf("Unexpected RecordsMetadata:  (-: found; +: expected)\n%s", diff)
	}
}

// TODO(schroederc): test transposed chunks
// TODO(schroederc): test padding
