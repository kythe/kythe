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

package graphstore

import (
	"errors"
	"testing"

	"code.google.com/p/goprotobuf/proto"

	spb "kythe/proto/storage_proto"
)

func vname(signature, corpus, root, path, language string) *spb.VName {
	return &spb.VName{
		Signature: &signature,
		Corpus:    &corpus,
		Root:      &root,
		Path:      &path,
		Language:  &language,
	}
}

func TestVNameCompare(t *testing.T) {
	tests := []struct {
		car  *spb.VName
		cdr  *spb.VName
		comp Order
	}{
		{nil, nil, EQ},
		{vname("", "", "", "", ""), vname("", "", "", "", ""), EQ},
		{vname("sig", "corpus", "root", "path", "language"), vname("sig", "corpus", "root", "path", "language"), EQ},
		{vname("a", "", "", "", ""), vname("b", "", "", "", ""), LT},
		{vname("b", "", "", "", ""), vname("a", "", "", "", ""), GT},
		{vname("s", "a", "", "", ""), vname("s", "b", "", "", ""), LT},
		{vname("s", "b", "", "", ""), vname("s", "a", "", "", ""), GT},
		{vname("s", "c", "a", "", ""), vname("s", "c", "b", "", ""), LT},
		{vname("s", "c", "b", "", ""), vname("s", "c", "a", "", ""), GT},
		{vname("s", "c", "r", "a", ""), vname("s", "c", "r", "b", ""), LT},
		{vname("s", "c", "r", "b", ""), vname("s", "c", "r", "a", ""), GT},
		{vname("s", "c", "r", "p", "a"), vname("s", "c", "r", "p", "b"), LT},
		{vname("s", "c", "r", "p", "b"), vname("s", "c", "r", "p", "a"), GT},
		{vname("s", "c", "a", "p", "l"), vname("s", "c", "b", "p", "l"), LT},
		{vname("s", "c", "b", "p", "l"), vname("s", "c", "a", "p", "l"), GT},
		{vname("s", "c", "a", "b", "l"), vname("s", "c", "b", "a", "l"), LT},
		{vname("s", "c", "b", "a", "l"), vname("s", "c", "a", "b", "l"), GT},
	}

	for _, pair := range tests {
		if res := VNameCompare(pair.car, pair.cdr); res != pair.comp {
			t.Errorf("Compare({%+v}, {%+v}); Expected %d; Result %d", pair.car, pair.cdr, pair.comp, res)
		}
	}
}

func entry(source *spb.VName, edgeKind, factName string, target *spb.VName, value string) *spb.Entry {
	return &spb.Entry{
		Source:    source,
		EdgeKind:  &edgeKind,
		FactName:  &factName,
		Target:    target,
		FactValue: []byte(value),
	}
}

func TestEntryLess(t *testing.T) {
	tests := []struct {
		car  *spb.Entry
		cdr  *spb.Entry
		less bool
	}{
		{nil, nil, false},
		{entry(vname("a", "", "", "", ""), "", "/", nil, "b"),
			entry(vname("b", "", "", "", ""), "", "/", nil, "a"), true},
		{entry(vname("b", "", "", "", ""), "", "/", nil, "a"),
			entry(vname("a", "", "", "", ""), "", "/", nil, "b"), false},
		{entry(vname("eq", "", "", "", ""), "", "/", nil, ""),
			entry(vname("eq", "", "", "", ""), "", "/", nil, ""), false},
		{entry(nil, "a", "/", nil, "b"),
			entry(nil, "b", "/", nil, "a"), true},
		{entry(nil, "b", "/", nil, "a"),
			entry(nil, "a", "/", nil, "b"), false},
		{entry(nil, "eq", "/", nil, ""),
			entry(nil, "eq", "/", nil, ""), false},
		{entry(nil, "", "/a", nil, "b"),
			entry(nil, "", "/b", nil, "a"), true},
		{entry(nil, "", "/b", nil, "a"),
			entry(nil, "", "/a", nil, "b"), false},
		{entry(nil, "", "/eq", nil, ""),
			entry(nil, "", "/eq", nil, ""), false},
		{entry(nil, "", "/", vname("a", "", "", "", ""), "b"),
			entry(nil, "", "/", vname("b", "", "", "", ""), "a"), true},
		{entry(nil, "", "/", vname("b", "", "", "", ""), "a"),
			entry(nil, "", "/", vname("a", "", "", "", ""), "b"), false},
		{entry(nil, "", "/", vname("eq", "", "", "", ""), ""),
			entry(nil, "", "/", vname("eq", "", "", "", ""), ""), false},
		{entry(nil, "", "", nil, "a"),
			entry(nil, "", "", nil, "b"), false},
		{entry(nil, "", "", nil, "b"),
			entry(nil, "", "", nil, "a"), false},
		{entry(nil, "", "", nil, "eq"),
			entry(nil, "", "", nil, "eq"), false},
	}

	for _, pair := range tests {
		if res := EntryLess(pair.car, pair.cdr); res != pair.less {
			t.Errorf("Less({%+v}, {%+v});\tExpected %v;\tResult %v", pair.car, pair.cdr, pair.less, res)
		}
	}
}

// Static set of Entry protos to use for testing
var testEntries = []*spb.Entry{
	entry(nil, "", "", nil, ""),
	entry(nil, "edge", "fact", nil, "value"),
	entry(nil, "a", "", nil, ""),
	entry(nil, "b", "", nil, ""),
	entry(nil, "c", "", nil, ""),
	entry(nil, "d", "", nil, ""),
	entry(nil, "e", "", nil, ""),
	entry(nil, "f", "", nil, ""),
	entry(nil, "g", "", nil, ""),
	entry(nil, "h", "", nil, ""),
	entry(nil, "i", "", nil, ""),
}

func TestMergeTestEntries(t *testing.T) {
	tests := []struct {
		streams [][]*spb.Entry
		results []*spb.Entry
	}{
		{[][]*spb.Entry{}, []*spb.Entry{}},
		{[][]*spb.Entry{{}}, []*spb.Entry{}},
		{[][]*spb.Entry{{testEntries[0]}}, []*spb.Entry{testEntries[0]}},
		{[][]*spb.Entry{
			{testEntries[0]},
			{testEntries[1]},
			{testEntries[5]},
		}, []*spb.Entry{
			testEntries[0], testEntries[5], testEntries[1],
		}},
		{[][]*spb.Entry{
			{testEntries[0]},
			{testEntries[1]},
			{testEntries[0], testEntries[1], testEntries[1]},
		}, []*spb.Entry{
			testEntries[0], testEntries[0], testEntries[1], testEntries[1], testEntries[1],
		}},
		{[][]*spb.Entry{
			{testEntries[2], testEntries[5], testEntries[7]},
			{testEntries[3], testEntries[8]},
			{testEntries[4], testEntries[6]},
		}, []*spb.Entry{
			testEntries[2], testEntries[3], testEntries[4], testEntries[5], testEntries[6], testEntries[7], testEntries[8],
		}},
		{[][]*spb.Entry{
			{testEntries[8]},
			{testEntries[2], testEntries[3], testEntries[5]},
			{testEntries[5], testEntries[6]},
			{testEntries[7], testEntries[9], testEntries[10]},
			{testEntries[4]},
			{testEntries[6], testEntries[6], testEntries[6], testEntries[6], testEntries[6]},
		}, []*spb.Entry{
			testEntries[2], testEntries[3], testEntries[4], testEntries[5], testEntries[5],
			testEntries[6], testEntries[6], testEntries[6], testEntries[6], testEntries[6],
			testEntries[6], testEntries[7], testEntries[8], testEntries[9], testEntries[10],
		}},
	}

	for _, test := range tests {
		streams := make([]chan *spb.Entry, len(test.streams))
		merged := make(chan *spb.Entry, len(test.results))

		for idx, stream := range test.streams {
			streams[idx] = make(chan *spb.Entry)
			go func(idx int, entries []*spb.Entry) {
				defer close(streams[idx])
				for _, entry := range entries {
					streams[idx] <- entry
				}
			}(idx, stream)
		}

		mergeEntries(merged, streams)
		close(merged)
		checkEntryResults(t, "", merged, test.results)
	}
}

func checkEntryResults(t *testing.T, errorPrefix string, results <-chan *spb.Entry, expected []*spb.Entry) {
	for i := 0; i < len(expected); i++ {
		entry := <-results
		if !proto.Equal(entry, expected[i]) {
			t.Errorf("%sExpected {%+v};\tReceived {%+v}", errorPrefix, expected[i], entry)
		}
	}

	for extra := range results {
		t.Errorf("%sReceived extra Entry: {%+v}", errorPrefix, extra)
	}
}

func TestProxy(t *testing.T) {
	testError := errors.New("test")
	mocks := []*mockGraphStore{
		{Entries: []*spb.Entry{testEntries[2], testEntries[3]}},
		{Entries: []*spb.Entry{testEntries[5], testEntries[6]}, Error: testError},
		{Entries: []*spb.Entry{testEntries[2], testEntries[4]}},
	}
	allEntries := []*spb.Entry{
		testEntries[2], testEntries[2], testEntries[3],
		testEntries[4], testEntries[5], testEntries[6],
	}
	proxy := NewProxy(mocks[0], mocks[1], mocks[2])

	readReq := &spb.ReadRequest{}
	readRes := make(chan *spb.Entry)
	go func() {
		defer close(readRes)
		if err := proxy.Read(readReq, readRes); err != testError {
			t.Errorf("Incorrect Read error: %v", err)
		}
	}()
	checkEntryResults(t, "Read: ", readRes, allEntries)
	for idx, mock := range mocks {
		if mock.LastReq != readReq {
			t.Errorf("ReadRequest not set for mock %d", idx)
		}
	}

	scanReq := &spb.ScanRequest{}
	scanRes := make(chan *spb.Entry)
	go func() {
		defer close(scanRes)
		if err := proxy.Scan(scanReq, scanRes); err != testError {
			t.Errorf("Incorrect Scan error: %v", err)
		}
	}()
	checkEntryResults(t, "Scan: ", scanRes, allEntries)
	for idx, mock := range mocks {
		if mock.LastReq != scanReq {
			t.Errorf("ScanRequest not set for mock %d", idx)
		}
	}

	writeReq := &spb.WriteRequest{}
	if err := proxy.Write(writeReq); err != testError {
		t.Errorf("Incorrect Write error: %v", err)
	}
	for idx, mock := range mocks {
		if mock.LastReq != writeReq {
			t.Errorf("WriteRequest not set for mock %d", idx)
		}
	}
}

type mockGraphStore struct {
	Entries []*spb.Entry
	LastReq proto.Message
	Error   error
}

func (m *mockGraphStore) Read(req *spb.ReadRequest, stream chan<- *spb.Entry) error {
	m.LastReq = req
	for _, entry := range m.Entries {
		stream <- entry
	}
	return m.Error
}

func (m *mockGraphStore) Scan(req *spb.ScanRequest, stream chan<- *spb.Entry) error {
	m.LastReq = req
	for _, entry := range m.Entries {
		stream <- entry
	}
	return m.Error
}

func (m *mockGraphStore) Write(req *spb.WriteRequest) error {
	m.LastReq = req
	return m.Error
}

func (m *mockGraphStore) Close() error {
	return m.Error
}
