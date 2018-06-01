/*
 * Copyright 2014 Google Inc. All rights reserved.
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

package delimited

import (
	"bytes"
	"errors"
	"io"
	"reflect"
	"strings"
	"testing"
)

const testData = "\x00\x01A\x02BC\x03DEF"

func TestGoodReader(t *testing.T) {
	r := strings.NewReader(testData)
	rd := NewReader(r)

	for _, want := range []string{"", "A", "BC", "DEF"} {
		got, err := rd.Next()
		if err != nil {
			t.Errorf("Unexpected read error: %v", err)
		} else if s := string(got); s != want {
			t.Errorf("Next record: got %q, want %q", s, want)
		}
	}

	// The stream should have been fully consumed.
	if got, err := rd.Next(); err != io.EOF {
		t.Errorf("Next record: got %q [%v], want EOF", string(got), err)
	}
}

func TestCorruptReader(t *testing.T) {
	const corrupt = "\x05ABCD" // n = 5, only 4 bytes of data

	r := strings.NewReader(corrupt)
	rd := NewReader(r)

	got, err := rd.Next()
	if err != io.ErrUnexpectedEOF {
		t.Fatalf("Next record: got %q [%v], want %v", string(got), err, io.ErrUnexpectedEOF)
	}
	t.Logf("Next record gave expected error: %v", err)
}

func TestGoodWriter(t *testing.T) {
	var w bytes.Buffer
	wr := NewWriter(&w)

	for _, record := range []string{"", "A", "BC", "DEF"} {
		if err := wr.Put([]byte(record)); err != nil {
			t.Errorf("Put %q: unexpected error: %v", record, err)
		}
	}
	if got := w.String(); got != testData {
		t.Errorf("Writer result: got %q, want %q", got, testData)
	}
}

type errWriter struct {
	nc  int
	err error
}

func (w *errWriter) Write(data []byte) (int, error) {
	if w.err != nil && w.nc == 0 {
		return 0, w.err
	}
	w.nc--
	return len(data), nil
}

func TestCorruptWriter(t *testing.T) {
	bad := errors.New("FAIL")
	w := &errWriter{nc: 1, err: bad}
	wr := NewWriter(w)

	err := wr.Put([]byte("whatever"))
	if err == nil {
		t.Fatalf("Put: got error nil, want error %v", bad)
	}
	t.Logf("Put record gave expected error: %v", err)
}

func TestRoundTrip(t *testing.T) {
	const input = "Some of what a fool thinks often remains."

	// Write all the words in the input as records to a delimited writer.
	words := strings.Fields(input)
	var buf bytes.Buffer

	wr := NewWriter(&buf)
	for _, word := range words {
		if err := wr.Put([]byte(word)); err != nil {
			t.Errorf("Put %q: unexpected error: %v", word, err)
		}
	}
	t.Logf("After writing, buf=%q len=%d", buf.Bytes(), buf.Len())

	// Read all the records back from the buffer with a delimited reader.
	var got []string
	rd := NewReader(&buf)
	for {
		rec, err := rd.Next()
		if err != nil {
			if err != io.EOF {
				t.Errorf("Next: unexpected error: %v", err)
			}
			break
		}
		got = append(got, string(rec))
	}

	// Verify that we got the original words back.
	if !reflect.DeepEqual(got, words) {
		t.Errorf("Round trip of %q: got %+q, want %+q", input, got, words)
	}
}
