/*
 * Copyright 2015 Google Inc. All rights reserved.
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

package proxy

import (
	"errors"
	"fmt"
	"io"
	"testing"

	"kythe.io/kythe/go/services/graphstore"

	"github.com/golang/protobuf/proto"
	"golang.org/x/net/context"

	spb "kythe.io/kythe/proto/storage_proto"
)

var ctx = context.Background()

// Static set of Entry protos to use for testing
var testEntries = []entry{
	{},
	{K: "edge", F: "fact", V: "value"},
	{K: "a"}, {K: "b"}, {K: "c"}, {K: "d"}, {K: "e"},
	{K: "f"}, {K: "g"}, {K: "h"}, {K: "i"},
}

func te(i int) *spb.Entry { return testEntries[i].proto() }

func tes(is ...int) (es []*spb.Entry) {
	for _, i := range is {
		es = append(es, te(i))
	}
	return es
}

func TestMergeOrder(t *testing.T) {
	tests := []struct {
		streams [][]*spb.Entry // Each stream must be in order
		want    []*spb.Entry
	}{
		{nil, nil},

		{[][]*spb.Entry{}, []*spb.Entry{}},

		{[][]*spb.Entry{{}}, []*spb.Entry{}},

		{[][]*spb.Entry{{te(0)}}, tes(0)},

		{[][]*spb.Entry{tes(0), tes(1), tes(5)}, tes(0, 5, 1)},

		{[][]*spb.Entry{tes(0), tes(1), tes(0, 1, 1)}, tes(0, 1)},

		{[][]*spb.Entry{tes(2, 5, 7), tes(3, 8), tes(4, 6)}, tes(2, 3, 4, 5, 6, 7, 8)},

		{[][]*spb.Entry{tes(8), tes(2, 3, 5), tes(5, 6), tes(7, 9, 10), tes(4), tes(6, 6, 6, 6, 6)},
			tes(2, 3, 4, 5, 6, 7, 8, 9, 10)},
	}

	for i, test := range tests {
		t.Logf("Begin test %d", i)
		var ss []graphstore.Service
		for _, stream := range test.streams {
			ss = append(ss, &mockGraphStore{Entries: stream})
		}
		result := make(chan *spb.Entry)
		done := checkResults(t, fmt.Sprintf("Merge test %d", i), result, test.want)

		if err := New(ss...).Read(ctx, new(spb.ReadRequest), func(e *spb.Entry) error {
			result <- e
			return nil
		}); err != nil {
			t.Errorf("Test %d Read failed: %v", i, err)
		}
		close(result)
		<-done
	}
}

func TestProxy(t *testing.T) {
	testError := errors.New("test")
	mocks := []*mockGraphStore{
		{Entries: []*spb.Entry{te(2), te(3)}},
		{Entries: []*spb.Entry{te(5), te(6)}, Error: testError},
		{Entries: []*spb.Entry{te(2), te(4)}},
	}
	allEntries := []*spb.Entry{
		te(2), te(3), te(4),
		te(5), te(6),
	}
	proxy := New(mocks[0], mocks[1], mocks[2])

	readReq := new(spb.ReadRequest)
	readRes := make(chan *spb.Entry)
	readDone := checkResults(t, "Read", readRes, allEntries)

	if err := proxy.Read(ctx, readReq, func(e *spb.Entry) error {
		readRes <- e
		return nil
	}); err != testError {
		t.Errorf("Incorrect Read error: %v", err)
	}
	close(readRes)
	<-readDone

	for idx, mock := range mocks {
		if mock.LastReq != readReq {
			t.Errorf("Read request was not sent to service %d", idx)
		}
	}

	scanReq := new(spb.ScanRequest)
	scanRes := make(chan *spb.Entry)
	scanDone := checkResults(t, "Scan", scanRes, allEntries)

	if err := proxy.Scan(ctx, scanReq, func(e *spb.Entry) error {
		scanRes <- e
		return nil
	}); err != testError {
		t.Errorf("Incorrect Scan error: %v", err)
	}
	close(scanRes)
	<-scanDone

	for idx, mock := range mocks {
		if mock.LastReq != scanReq {
			t.Errorf("Scan request was not sent to for service %d", idx)
		}
	}

	writeReq := new(spb.WriteRequest)
	if err := proxy.Write(ctx, writeReq); err != testError {
		t.Errorf("Incorrect Write error: %v", err)
	}
	for idx, mock := range mocks {
		if mock.LastReq != writeReq {
			t.Errorf("Write request was not sent to service %d", idx)
		}
	}
}

// Verify that a proxy store behaves sensibly if an operation fails.
func TestCancellation(t *testing.T) {
	bomb := entry{K: "bomb", F: "die", V: "horrible catastrophe"}
	stores := []graphstore.Service{
		&mockGraphStore{Entries: tes(8, 3, 2, 4, 5, 9, 2, 0)},
		&mockGraphStore{Entries: []*spb.Entry{bomb.proto()}},
		&mockGraphStore{Entries: tes(1, 1, 1, 0, 1, 6, 5)},
		&mockGraphStore{Entries: tes(3, 10, 7)},
	}
	p := New(stores...)

	// Check that a callback returning a non-EOF error propagates an error to
	// the caller.
	var numEntries int
	if err := p.Scan(ctx, new(spb.ScanRequest), func(e *spb.Entry) error {
		if e.FactName == bomb.F {
			return errors.New(bomb.V)
		}
		numEntries++
		return nil
	}); err != nil {
		t.Logf("Got expected error: %v", err)
	} else {
		t.Error("Unexpected success! Got nil, wanted an error")
	}
	if numEntries != 3 {
		t.Errorf("Wrong number of entries scanned: got %d, want 3", numEntries)
	}

	// Check that a callback returning io.EOF ends without error.
	numEntries = 0
	if err := p.Read(ctx, new(spb.ReadRequest), func(e *spb.Entry) error {
		if e.FactName == bomb.F {
			return io.EOF
		}
		numEntries++
		return nil
	}); err != nil {
		t.Errorf("Read: unexpected error: %v", err)
	}
	if numEntries != 3 {
		t.Errorf("Wrong number of entries read: got %d, want 3", numEntries)
	}
}

type vname struct {
	S, C, R, P, L string
}

func (v vname) proto() *spb.VName {
	return &spb.VName{
		Signature: v.S,
		Corpus:    v.C,
		Path:      v.P,
		Root:      v.R,
		Language:  v.L,
	}
}

type entry struct {
	S, T    vname
	K, F, V string
}

func (e entry) proto() *spb.Entry {
	return &spb.Entry{
		Source:    e.S.proto(),
		EdgeKind:  e.K,
		FactName:  e.F,
		Target:    e.T.proto(),
		FactValue: []byte(e.V),
	}
}

type mockGraphStore struct {
	Entries []*spb.Entry
	LastReq proto.Message
	Error   error
}

func (m *mockGraphStore) Read(ctx context.Context, req *spb.ReadRequest, f graphstore.EntryFunc) error {
	m.LastReq = req
	for _, entry := range m.Entries {
		if err := f(entry); err == io.EOF {
			return nil
		} else if err != nil {
			return err
		}
	}
	return m.Error
}

func (m *mockGraphStore) Scan(ctx context.Context, req *spb.ScanRequest, f graphstore.EntryFunc) error {
	m.LastReq = req
	for _, entry := range m.Entries {
		if err := f(entry); err == io.EOF {
			return nil
		} else if err != nil {
			return err
		}
	}
	return m.Error
}

func (m *mockGraphStore) Write(ctx context.Context, req *spb.WriteRequest) error {
	m.LastReq = req
	return m.Error
}

func (m *mockGraphStore) Close(ctx context.Context) error { return m.Error }

// checkResults starts a goroutine that consumes entries from results and
// compares them to corresponding members of want.  If the corresponding values
// are unequal or if there are more or fewer results than wanted, errors are
// logged to t prefixed with the given tag.
//
// The returned channel is closed when all results have been checked.
func checkResults(t *testing.T, tag string, results <-chan *spb.Entry, want []*spb.Entry) chan struct{} {
	done := make(chan struct{})
	go func() {
		defer close(done)
		i := 0
		for entry := range results {
			if i < len(want) {
				if !proto.Equal(entry, want[i]) {
					t.Errorf("%s result %d: got {%+v}, want {%+v}", tag, i, entry, want[i])
				}
			} else {
				t.Errorf("%s extra result %d: {%+v}", tag, i, entry)
			}
			i++
		}
		for i < len(want) {
			t.Errorf("%s missing result %d: {%+v}", tag, i, want[i])
			i++
		}
	}()
	return done
}
