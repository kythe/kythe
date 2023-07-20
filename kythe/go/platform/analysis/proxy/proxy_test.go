/*
 * Copyright 2017 The Kythe Authors. All rights reserved.
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
	"encoding/json"
	"errors"
	"io"
	"testing"

	"kythe.io/kythe/go/test/testutil"
	"kythe.io/kythe/go/util/compare"
	"kythe.io/kythe/go/util/log"
	"kythe.io/kythe/go/util/schema/facts"

	"google.golang.org/protobuf/encoding/protojson"
	"google.golang.org/protobuf/proto"

	apb "kythe.io/kythe/proto/analysis_go_proto"
	cpb "kythe.io/kythe/proto/common_go_proto"
	spb "kythe.io/kythe/proto/storage_go_proto"
)

// A testreq is the equivalent of a request, pre-encoding.
type testreq struct {
	Type string `json:"req,omitempty"`
	Args any    `json:"args,omitempty"`
}

// An indexer simulates an indexer process. It sends each of the requests in
// its queue in turn, and records the responses.
type indexer struct {
	in  *json.Decoder
	out *json.Encoder

	reqs []testreq
	rsps []string // JSON format
	errc chan error
}

func (ix *indexer) run() {
	defer close(ix.errc)
	for _, req := range ix.reqs {
		if err := ix.out.Encode(req); err != nil {
			ix.errc <- err
			return
		}

		var rsp response
		if err := ix.in.Decode(&rsp); err != nil {
			ix.errc <- err
			return
		}

		jrsp := mustMarshal(&rsp)
		ix.rsps = append(ix.rsps, jrsp)
	}
}

func newIndexer(in io.Reader, out io.Writer, reqs ...testreq) *indexer {
	return &indexer{
		in:   json.NewDecoder(in),
		out:  json.NewEncoder(out),
		reqs: reqs,
		errc: make(chan error, 1),
	}
}

func (ix *indexer) wait() error { return <-ix.errc }

type handler struct {
	analysis func() (*apb.AnalysisRequest, error)
	output   func(...*spb.Entry) error
	done     func(error)
	file     func(path, digest string) ([]byte, error)
}

// Analysis implements part of the Handler interface.  If no function is set,
// testReq is returned without error.
func (h handler) Analysis() (*apb.AnalysisRequest, error) {
	if f := h.analysis; f != nil {
		return f()
	}
	return testReq, nil
}

// Output implements part of the Handler interface. If no function is set, the
// output is discarded without error.
func (h handler) Output(entries ...*spb.Entry) error {
	if f := h.output; f != nil {
		return f(entries...)
	}
	return nil
}

// Done implements part of the Handler interface. If no function is set, this
// is silently a no-op.
func (h handler) Done(err error) {
	if f := h.done; f != nil {
		f(err)
	}
}

// File implements part of the Handler interface. If no function is set, an
// error with string "notfound" is returned.
func (h handler) File(path, digest string) ([]byte, error) {
	if f := h.file; f != nil {
		return f(path, digest)
	}
	return nil, errors.New("notfound")
}

// runProxy runs an indexer with the given requests and a proxy delegating to
// h.  The responses collected by the indexer are returned.
func runProxy(h Handler, reqs ...testreq) ([]string, error) {
	pin, pout := io.Pipe() // proxy to indexer
	xin, xout := io.Pipe() // indexer to proxy
	ix := newIndexer(xin, pout, reqs...)
	go func() {
		defer pout.Close() // signal EOF to the driver
		ix.run()
	}()
	err := New(pin, xout).Run(h)
	if err != nil {
		xout.Close()
	}
	if xerr := ix.wait(); err == nil {
		err = xerr
	}
	return ix.rsps, err
}

// Dummy values for testing.
var (
	testReq = &apb.AnalysisRequest{
		Compilation: &apb.CompilationUnit{
			VName: &spb.VName{Signature: "test"},
		},
		Revision:        "1",
		FileDataService: "Q",
	}

	testEntries = []*spb.Entry{
		{Source: &spb.VName{Signature: "A"}, EdgeKind: "loves", Target: &spb.VName{Signature: "B"}},
		{Source: &spb.VName{Signature: "C"}, FactName: "versus", FactValue: []byte("D")},
	}
)

const analysisReply = `{"rsp":"ok","args":{"fds":"Q","rev":"1","unit":{"v_name":{"signature":"test"}}}}`
const analysisWireReply = `{"rsp":"ok","args":{"fds":"Q","rev":"1","unit":"CgYKBHRlc3Q="}}`

func mustMarshal(v any) string {
	bits, err := json.Marshal(v)
	if err != nil {
		log.Fatalf("Error marshaling JSON: %v", err)
	}
	return string(bits)
}

func encodeEntries(es []*spb.Entry) json.RawMessage {
	var messages []json.RawMessage
	for _, e := range es {
		rec, err := protojson.MarshalOptions{UseProtoNames: true}.Marshal(e)
		if err != nil {
			panic(err)
		}
		messages = append(messages, rec)
	}

	msg, err := json.Marshal(messages)
	if err != nil {
		panic(err)
	}
	return msg
}

func encodeWireEntries(es []*spb.Entry) json.RawMessage {
	var messages [][]byte
	for _, e := range es {
		rec, err := proto.MarshalOptions{}.Marshal(e)
		if err != nil {
			panic(err)
		}
		messages = append(messages, rec)
	}

	msg, err := json.Marshal(messages)
	if err != nil {
		panic(err)
	}
	return msg
}

func TestNOOP(t *testing.T) {
	// Verify that startup and shutdown are clean.
	if rsps, err := runProxy(handler{}); err != nil {
		t.Errorf("Proxy failed on empty input: %v", err)
	} else if len(rsps) != 0 {
		t.Errorf("Empty input returned responses: %+v", rsps)
	}
}

func TestErrors(t *testing.T) {
	tests := []struct {
		desc string
		h    Handler
		reqs []testreq
		want []string
	}{{
		desc: "Error getting analysis",
		h:    handler{analysis: func() (*apb.AnalysisRequest, error) { return nil, errors.New("bad") }},
		reqs: []testreq{{Type: "analysis"}},
		want: []string{`{"rsp":"error","args":"bad"}`},
	}, {
		desc: "Error sending outputs",
		h:    handler{output: func(...*spb.Entry) error { return errors.New("bad") }},
		reqs: []testreq{{Type: "analysis"}, {Type: "output", Args: encodeEntries(testEntries)}},
		want: []string{
			analysisReply,
			`{"rsp":"error","args":"bad"}`,
		},
	}, {
		desc: "Requests sent out of order provoke an error",
		h:    handler{},
		reqs: []testreq{
			{Type: "done", Args: status{OK: true}},
			{Type: "output", Args: encodeEntries(testEntries)},
		},
		want: []string{
			`{"rsp":"error","args":"no analysis is in progress"}`,
			`{"rsp":"error","args":"no analysis is in progress"}`,
		},
	}, {
		desc: "File not found",
		h:    handler{},
		reqs: []testreq{{Type: "file", Args: file{Path: "foo", Digest: "bar"}}},
		want: []string{`{"rsp":"error","args":"notfound"}`},
	}, {
		desc: "An error in output terminates the analysis",
		h: handler{
			output: func(es ...*spb.Entry) error {
				if es[0].EdgeKind == "fail" {
					return errors.New("bad")
				}
				return nil
			},
		},
		reqs: []testreq{
			{Type: "analysis"}, // succeeds
			{Type: "output", Args: encodeEntries([]*spb.Entry{{EdgeKind: "ok"}})},   // succeeds
			{Type: "output", Args: encodeEntries([]*spb.Entry{{EdgeKind: "fail"}})}, // fails
			{Type: "output", Args: encodeEntries([]*spb.Entry{{EdgeKind: "wah"}})},  // fails
			{Type: "done", Args: status{OK: false, Message: "cat abuse"}},           // fails
			{Type: "analysis"}, // succeeds
		},
		want: []string{
			analysisReply,
			`{"rsp":"ok"}`,
			`{"rsp":"error","args":"bad"}`,
			`{"rsp":"error","args":"no analysis is in progress"}`,
			`{"rsp":"error","args":"no analysis is in progress"}`,
			analysisReply,
		},
	}}

	for _, test := range tests {
		t.Log("Testing:", test.desc)
		t.Logf(" - requests: %#q", test.reqs)
		rsps, err := runProxy(test.h, test.reqs...)
		if err != nil {
			t.Errorf("Unexpected error from proxy: %v", err)
		}
		t.Logf(" - responses: %+v", rsps)
		if diff := compare.ProtoDiff(rsps, test.want); diff != "" {
			t.Errorf("Incorrect responses; wanted %+v: %s", test.want, diff)
		}
	}
}

func TestAnalysisWorks(t *testing.T) {
	// Enact a standard analyzer transaction, and verify that the responses work.
	var doneCalled bool
	var gotEntries []*spb.Entry

	rsps, err := runProxy(handler{
		done: func(err error) {
			if err != nil {
				t.Errorf("Done was called with an error: %v", err)
			}
			doneCalled = true
		},
		output: func(es ...*spb.Entry) error {
			gotEntries = append(gotEntries, es...)
			return nil
		},
		file: func(path, digest string) ([]byte, error) {
			if path == "exists" {
				return []byte("data"), nil
			}
			return nil, errors.New("notfound")
		},
	},
		testreq{Type: "analysis"},
		testreq{Type: "file", Args: file{Path: "exists"}},
		testreq{Type: "output", Args: encodeEntries(testEntries)},
		testreq{Type: "file", Args: file{Path: "does not exist"}},
		testreq{Type: "done"},
	)
	if err != nil {
		t.Errorf("Unexpected error from proxy: %v", err)
	}

	// Verify that the proxy followed the protocol reasonably.
	if !doneCalled {
		t.Error("The handler's Done method was never called")
	}
	if diff := compare.ProtoDiff(gotEntries, testEntries); diff != "" {
		t.Errorf("Incorrect entries:\n got: %+v\nwant: %+v: %s", gotEntries, testEntries, diff)
	}

	// Verify that we got the expected replies back from the proxy.
	want := []string{
		analysisReply, // from Analyze
		`{"rsp":"ok","args":{"content":"ZGF0YQ==","path":"exists"}}`, // from File (1)
		`{"rsp":"ok"}`,                      // from Output
		`{"rsp":"error","args":"notfound"}`, // from File (2)
		`{"rsp":"ok"}`,                      // from Done
	}
	if diff := compare.ProtoDiff(rsps, want); diff != "" {
		t.Errorf("Wrong incorrect responses:\n got: %+v\nwant: %+v: %s", rsps, want, diff)
	}
}

func TestCodeJSON(t *testing.T) {
	ms := &cpb.MarkedSource{}
	json, err := protojson.Marshal(ms)
	testutil.Fatalf(t, "protojson.Marshal: %v", err)
	rec, err := proto.Marshal(ms)
	testutil.Fatalf(t, "proto.Marshal: %v", err)

	testEntries := []*spb.Entry{
		&spb.Entry{
			Source:    &spb.VName{Signature: "sig"},
			FactName:  codeJSONFact,
			FactValue: json,
		},
	}
	expectedEntries := []*spb.Entry{
		&spb.Entry{
			Source:    &spb.VName{Signature: "sig"},
			FactName:  facts.Code,
			FactValue: rec,
		},
	}

	var gotEntries []*spb.Entry

	rsps, err := runProxy(handler{
		done: func(err error) {
			if err != nil {
				t.Errorf("Done was called with an error: %v", err)
			}
		},
		output: func(es ...*spb.Entry) error {
			gotEntries = append(gotEntries, es...)
			return nil
		},
		file: func(path, digest string) ([]byte, error) {
			if path == "exists" {
				return []byte("data"), nil
			}
			return nil, errors.New("notfound")
		},
	},
		testreq{Type: "analysis"},
		testreq{Type: "file", Args: file{Path: "exists"}},
		testreq{Type: "output", Args: encodeEntries(testEntries)},
		testreq{Type: "file", Args: file{Path: "does not exist"}},
		testreq{Type: "done"},
	)
	if err != nil {
		t.Errorf("Unexpected error from proxy: %v", err)
	}

	if diff := compare.ProtoDiff(gotEntries, expectedEntries); diff != "" {
		t.Errorf("Incorrect entries:\n got: %+v\nwant: %+v: %s", gotEntries, testEntries, diff)
	}

	// Verify that we got the expected replies back from the proxy.
	want := []string{
		analysisReply, // from Analyze
		`{"rsp":"ok","args":{"content":"ZGF0YQ==","path":"exists"}}`, // from File (1)
		`{"rsp":"ok"}`,                      // from Output
		`{"rsp":"error","args":"notfound"}`, // from File (2)
		`{"rsp":"ok"}`,                      // from Done
	}
	if diff := compare.ProtoDiff(rsps, want); diff != "" {
		t.Errorf("Wrong incorrect responses:\n got: %+v\nwant: %+v: %s", rsps, want, diff)
	}
}

func TestWireAnalysisWorks(t *testing.T) {
	// Enact a standard analyzer transaction, and verify that the responses work.
	var doneCalled bool
	var gotEntries []*spb.Entry

	rsps, err := runProxy(handler{
		done: func(err error) {
			if err != nil {
				t.Errorf("Done was called with an error: %v", err)
			}
			doneCalled = true
		},
		output: func(es ...*spb.Entry) error {
			gotEntries = append(gotEntries, es...)
			return nil
		},
		file: func(path, digest string) ([]byte, error) {
			if path == "exists" {
				return []byte("data"), nil
			}
			return nil, errors.New("notfound")
		},
	},
		testreq{Type: "analysis_wire"},
		testreq{Type: "file", Args: file{Path: "exists"}},
		testreq{Type: "output_wire", Args: encodeWireEntries(testEntries)},
		testreq{Type: "file", Args: file{Path: "does not exist"}},
		testreq{Type: "done"},
	)
	if err != nil {
		t.Errorf("Unexpected error from proxy: %v", err)
	}

	// Verify that the proxy followed the protocol reasonably.
	if !doneCalled {
		t.Error("The handler's Done method was never called")
	}
	if diff := compare.ProtoDiff(gotEntries, testEntries); diff != "" {
		t.Errorf("Incorrect entries:\n got: %+v\nwant: %+v: %s", gotEntries, testEntries, diff)
	}

	// Verify that we got the expected replies back from the proxy.
	want := []string{
		analysisWireReply, // from Analyze Wire
		`{"rsp":"ok","args":{"content":"ZGF0YQ==","path":"exists"}}`, // from File (1)
		`{"rsp":"ok"}`,                      // from Output Wire
		`{"rsp":"error","args":"notfound"}`, // from File (2)
		`{"rsp":"ok"}`,                      // from Done
	}
	if diff := compare.ProtoDiff(rsps, want); diff != "" {
		t.Errorf("Wrong incorrect responses:\n got: %+v\nwant: %+v: %s", rsps, want, diff)
	}
}
