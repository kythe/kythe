/*
 * Copyright 2016 The Kythe Authors. All rights reserved.
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

package stream

import (
	"bytes"
	"encoding/json"
	"fmt"
	"testing"

	"kythe.io/kythe/go/platform/delimited"
	"kythe.io/kythe/go/test/testutil"
	"kythe.io/kythe/go/util/compare"

	"google.golang.org/protobuf/encoding/protojson"
	"google.golang.org/protobuf/proto"

	cpb "kythe.io/kythe/proto/common_go_proto"
	spb "kythe.io/kythe/proto/storage_go_proto"
)

func TestReader(t *testing.T) {
	r := testBuffer(testEntries)

	var i int
	if err := NewReader(r)(func(e *spb.Entry) error {
		if err := testutil.DeepEqual(testEntries[i], e); err != nil {
			t.Errorf("testEntries[%d]: %v", i, err)
		}
		i++
		return nil
	}); err != nil {
		t.Fatal(err)
	}

	if i != len(testEntries) {
		t.Fatalf("Missing %d entries", len(testEntries)-i)
	}
}

func TestJSONReader(t *testing.T) {
	r := testJSONBuffer(testEntries)

	var i int
	if err := NewJSONReader(r)(func(e *spb.Entry) error {
		if err := testutil.DeepEqual(testEntries[i], e); err != nil {
			t.Errorf("testEntries[%d]: %v", i, err)
		}
		i++
		return nil
	}); err != nil {
		t.Fatal(err)
	}

	if i != len(testEntries) {
		t.Fatalf("Missing %d entries", len(testEntries)-i)
	}
}

func TestStructuredEntry(t *testing.T) {
	ms := &cpb.MarkedSource{PreText: "hi"}
	pbms, err := proto.Marshal(ms)
	if err != nil {
		t.Fatalf("Error marshaling MarkedSource: %v", err)
	}
	entry := &spb.Entry{
		Source:    &spb.VName{Signature: "sig"},
		FactName:  "/kythe/code",
		FactValue: pbms,
	}
	entryJSON, err := json.Marshal((*StructuredEntry)(entry))
	if err != nil {
		t.Fatalf("Error marshaling entry: %v", err)
	}

	var rawOut richJSONEntry
	if err := json.Unmarshal(entryJSON, &rawOut); err != nil {
		t.Fatalf("Error unmarshaling entry (without fact_value): %v", err)
	}

	var msOut cpb.MarkedSource
	if err := protojson.Unmarshal(rawOut.FactValue, &msOut); err != nil {
		t.Fatalf("Error unmarshaling fact_value: %v", err)
	}

	if !proto.Equal(ms, &msOut) {
		t.Errorf("Failed to properly encode marked source:\n%s", rawOut.FactValue)
	}

	var entryOut StructuredEntry
	if err := json.Unmarshal(entryJSON, &entryOut); err != nil {
		t.Fatalf("Error unmarshaling StructuredEntry: %v", err)
	}

	if diff := compare.ProtoDiff(entry, (*spb.Entry)(&entryOut)); diff != "" {
		t.Errorf("Roundtrip Marshal/Unmarshal failed: \n%v\n%v\n", entry, &entryOut, diff)
	}
}

func BenchmarkReader(b *testing.B) {
	buf := testBuffer(genEntries(b.N))
	b.ResetTimer()
	if err := NewReader(buf)(func(e *spb.Entry) error {
		return nil
	}); err != nil {
		b.Fatal(err)
	}
}

func BenchmarkJSONReader(b *testing.B) {
	buf := testJSONBuffer(genEntries(b.N))
	b.ResetTimer()
	if err := NewJSONReader(buf)(func(e *spb.Entry) error {
		return nil
	}); err != nil {
		b.Fatal(err)
	}
}

func BenchmarkReaderChannel(b *testing.B) {
	buf := testBuffer(genEntries(b.N))
	b.ResetTimer()
	for range ReadEntries(buf) {
	}
}

func BenchmarkJSONReaderChannel(b *testing.B) {
	buf := testJSONBuffer(genEntries(b.N))
	b.ResetTimer()
	for range ReadJSONEntries(buf) {
	}
}

func testBuffer(entries []*spb.Entry) *bytes.Buffer {
	buf := bytes.NewBuffer(nil)
	wr := delimited.NewWriter(buf)
	for _, e := range entries {
		if err := wr.PutProto(e); err != nil {
			panic(err)
		}
	}
	return buf
}

func testJSONBuffer(entries []*spb.Entry) *bytes.Buffer {
	buf := bytes.NewBuffer(nil)
	wr := json.NewEncoder(buf)
	for _, e := range entries {
		if err := wr.Encode(e); err != nil {
			panic(err)
		}
	}
	return buf
}

var testEntries = []*spb.Entry{
	fact("node0", "kind", "test"),
	edge("node0", "edge", "node1"),
	fact("node1", "kind", "anotherTest"),
	fact("blah", "blah", "eh"),
}

func genEntries(n int) []*spb.Entry {
	entries := make([]*spb.Entry, n)
	for i := 0; i < n; i++ {
		if i%2 == 0 {
			entries[i] = fact(fmt.Sprintf("node%d", i), "kind", "test")
		} else {
			entries[i] = edge(fmt.Sprintf("node%d", i-1), "edgeKind", fmt.Sprintf("node%d", i+1))
		}
	}
	return entries
}

func fact(signature, factName, factValue string) *spb.Entry {
	return &spb.Entry{
		Source:    &spb.VName{Signature: signature},
		FactName:  factName,
		FactValue: []byte(factValue),
	}
}

func edge(signature, edgeKind, targetSig string) *spb.Entry {
	return &spb.Entry{
		Source:   &spb.VName{Signature: signature},
		EdgeKind: edgeKind,
		Target:   &spb.VName{Signature: targetSig},
		FactName: "/",
	}
}
