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

// Package proxy implements a wrapper that proxies an analysis request to a
// Kythe indexer that does not speak protocol buffers or RPCs.
//
// Communication with the underlying indexer is via JSON over pipes, with
// messages from the proxy to the indexer on one side and replies on the other.
// Each JSON message is sent as a single complete line terminated by a newline
// (LF) character.
//
// Communication between proxy and indexer is in lock-step; there is only one
// transaction active at a time in either direction and the indexer initiates
// all requests.
//
// The protocol between indexer (X) and proxy (P) is:
//
//	X → P: {"req":"analysis"}<LF>
//	P → X: {"rsp":"ok","args":{"unit":<unit>,"rev":<revision>,"fds":<addr>}}<LF>
//	       {"rsp":"error","args":<error>}<LF>
//
//	X → P: {"req":"analysis_wire"}<LF>
//	P → X: {"rsp":"ok","args":{"unit":<unit_wire>,"rev":<revision>,"fds":<addr>}}<LF>
//	       {"rsp":"error","args":<error>}<LF>
//
//	X → P: {"req":"output","args":[<entry>...]}<LF>
//	P → X: {"rsp":"ok"}<LF>
//	       {"rsp":"error","args":<error>}<LF>
//
//	X → P: {"req":"output_wire","args":[<entry_wire>...]}<LF>
//	P → X: {"rsp":"ok"}<LF>
//	       {"rsp":"error","args":<error>}<LF>
//
//	X → P: {"req":"done","args":{"ok":true,"msg":<error>}}<LF>
//	P → X: {"rsp":"ok"}<LF>
//	       {"rsp":"error","args":<error>}<LF>
//
//	X → P: {"req":"file","args":{"path":<path>,"digest":<digest>}}<LF>
//	P → X: {"rsp":"ok","args":{"path":<path>,"digest":<digest>,"content":<bytes>}}<LF>
//	       {"rsp":"error","args":<error>}<LF>
//
// Where:
//
//	<addr>       -- service address
//	<bytes>      -- BASE-64 encoded bytes (string)
//	<digest>     -- file content digest (string)
//	<entry>      -- JSON encoded kythe.proto.Entry message
//	<entry_wire> -- BASE-64 encoded kythe.proto.Entry message in wire format (string)
//	<error>      -- error diagnostic (string)
//	<path>       -- file path (string)
//	<revision>   -- revision marker (string)
//	<unit>       -- JSON encoded kythe.proto.CompilationUnit message
//	<unit_wire>  -- BASE-64 encoded kythe.proto.CompilationUnit message in wire format (string)
//	<LF>         -- line feed character (decimal code 10)
//
// When rsp="error" in a reply, the args are an error string. The ordinary flow
// for the indexer is:
//
//	{"req":"analysis"}   -- to start a new analysis task
//	{"req":"output",...} -- to send outputs for the task
//	{"req":"file",...}   -- to fetch required input files.
//	    ... (as many times as needed) ...
//	{"req":"done",...}   -- to mark the analysis as complete
//	                        and to report success/failure
//
// The proxy supports an extra /kythe/code/json fact.  Its value will be
// interpreted as a JSON-encoded kythe.proto.common.MarkedSource message and
// will be rewritted to the equivalent wire-encoded /kythe/code fact.
//
// In case of an indexing error, the indexer is free to terminate the analysis
// early and report {"req":"done","args":{"ok":false}} to the driver.
//
// In case of an error writing output data, the driver will report an error in
// response to the "output" call. Once this happens, the analysis is abandoned
// as if the indexer had called "done". Subsequent calls to "output" or "done"
// will behave as if no analysis is in progress. The indexer is free at that
// point to start a new analysis.
package proxy // import "kythe.io/kythe/go/platform/analysis/proxy"

import (
	"encoding/json"
	"errors"
	"fmt"
	"io"

	"kythe.io/kythe/go/util/log"
	"kythe.io/kythe/go/util/schema/facts"

	"google.golang.org/protobuf/encoding/protojson"
	"google.golang.org/protobuf/proto"

	apb "kythe.io/kythe/proto/analysis_go_proto"
	cpb "kythe.io/kythe/proto/common_go_proto"
	spb "kythe.io/kythe/proto/storage_go_proto"
)

// New returns a proxy that reads requests from in and writes responses to out.
// The caller must invoke Run to start processing requests.
func New(in io.Reader, out io.Writer) *Proxy {
	return &Proxy{
		in:  json.NewDecoder(in),
		out: json.NewEncoder(out),
	}
}

// A request represents a request from the indexer to the proxy.
type request struct {
	Type string          `json:"req"` // the type of the request
	Args json.RawMessage `json:"args,omitempty"`
}

// A response represents a response from the proxy to the indexer.
type response struct {
	Status string `json:"rsp"` // status message
	Args   any    `json:"args,omitempty"`
}

// A unit represents a compilation unit (in JSON form) to be analyzed.
type unit struct {
	Unit            json.RawMessage `json:"unit"`
	Revision        string          `json:"rev,omitempty"`
	FileDataService string          `json:"fds,omitempty"`
}

// A unit represents a compilation unit (in base64-encoded wire format) to be analyzed.
type unitWire struct {
	Unit            []byte `json:"unit"`
	Revision        string `json:"rev,omitempty"`
	FileDataService string `json:"fds,omitempty"`
}

// A file represents a file request or content reply.
type file struct {
	Path    string `json:"path,omitempty"`
	Digest  string `json:"digest,omitempty"`
	Content []byte `json:"content,omitempty"`
}

// A status represents a status message from a "done" request.
type status struct {
	OK      bool   `json:"ok,omitempty"`
	Message string `json:"msg,omitempty"`
}

// A Proxy processes requests from an indexer subprocess on the other end of a
// pipe by dispatching them to a callback handler.  The caller must invoke Run
// to start processing requests.
type Proxy struct {
	in  *json.Decoder // input from the indexer
	out *json.Encoder // output to the indexer
	err error
}

// Run blocks serving requests from the indexer until communication with the
// indexer reports an error. The resulting error is returned unless it is
// io.EOF, in which case Run returns nil.
func (p *Proxy) Run(h Handler) error {
	var hasReq bool // true when a request is being processed
	for {
		var req request
		if err := p.in.Decode(&req); err == io.EOF {
			return nil
		} else if err != nil {
			return err
		}

		switch req.Type {
		case "analysis":
			// Prerequisite: There is no analysis request already in progress.
			if hasReq {
				p.reply("error", "an analysis is already in progress")
			} else if req, err := h.Analysis(); err != nil {
				p.reply("error", err.Error())
			} else {
				hasReq = true
				u, err := protojson.MarshalOptions{UseProtoNames: true}.Marshal(req.Compilation)
				if err != nil {
					return fmt.Errorf("error marshalling compilation unit as JSON: %w", err)
				}
				p.reply("ok", &unit{
					Unit:            json.RawMessage(u),
					Revision:        req.Revision,
					FileDataService: req.FileDataService,
				})
			}

		case "analysis_wire":
			// Prerequisite: There is no analysis request already in progress.
			if hasReq {
				p.reply("error", "an analysis is already in progress")
			} else if req, err := h.Analysis(); err != nil {
				p.reply("error", err.Error())
			} else {
				hasReq = true
				u, err := proto.MarshalOptions{}.Marshal(req.Compilation)
				if err != nil {
					return fmt.Errorf("error marshalling compilation unit as wire format: %w", err)
				}
				p.reply("ok", &unitWire{
					Unit:            u,
					Revision:        req.Revision,
					FileDataService: req.FileDataService,
				})
			}

		case "output":
			// Prerequisite: There is an analysis request in progress.
			if !hasReq {
				p.reply("error", "no analysis is in progress")
				break
			}
			entries, err := decodeEntries(req.Args)
			if err != nil {
				p.reply("error", "invalid entries: "+err.Error())
			} else if err := h.Output(entries...); err != nil {
				p.reply("error", err.Error())
			} else {
				p.reply("ok", nil)
				break
			}

			// Analysis is now abandoned; report the error back to the
			// handler and give up.
			h.Done(err)
			hasReq = false

		case "output_wire":
			// Prerequisite: There is an analysis request in progress.
			if !hasReq {
				p.reply("error", "no analysis is in progress")
				break
			}
			entries, err := decodeWireEntries(req.Args)
			if err != nil {
				p.reply("error", "invalid entries: "+err.Error())
			} else if err := h.Output(entries...); err != nil {
				p.reply("error", err.Error())
			} else {
				p.reply("ok", nil)
				break
			}

			// Analysis is now abandoned; report the error back to the
			// handler and give up.
			h.Done(err)
			hasReq = false

		case "done":
			// Prerequisite: There is an analysis request in progress.
			if !hasReq {
				p.reply("error", "no analysis is in progress")
				break
			}
			hasReq = false
			if req.Args == nil {
				h.Done(nil)
			} else {
				var stat status
				if err := json.Unmarshal(req.Args, &stat); err != nil {
					h.Done(err)
					p.reply("error", "invalid status: "+err.Error())
					break
				} else if !stat.OK {
					h.Done(errors.New("analysis failed: " + stat.Message))
				} else {
					h.Done(nil)
				}
			}
			p.reply("ok", nil)

		case "file":
			// A file request can be issued at any time, though in practice it
			// may fail when no request is present since different requests may
			// come from different file sources.
			var info file
			if err := json.Unmarshal(req.Args, &info); err != nil {
				p.reply("error", "invalid file request: "+err.Error())
				break
			} else if info.Path == "" && info.Digest == "" {
				p.reply("error", "empty file request")
				break
			}
			bits, err := h.File(info.Path, info.Digest)
			if err != nil {
				p.reply("error", err.Error())
			} else {
				p.reply("ok", &file{
					Path:    info.Path,
					Digest:  info.Digest,
					Content: bits,
				})
			}

		default:
			p.reply("error", "invalid request: "+req.Type)
		}

		if p.err != nil {
			return p.err // deferred from above
		}
	}
}

const codeJSONFact = facts.Code + "/json"

func rewriteEntry(e *spb.Entry) (*spb.Entry, error) {
	if e.GetFactName() == codeJSONFact {
		ms := new(cpb.MarkedSource)
		if err := protojson.Unmarshal(e.GetFactValue(), ms); err != nil {
			return nil, err
		}
		rec, err := proto.Marshal(ms)
		if err != nil {
			return nil, err
		}
		return &spb.Entry{
			Source:    e.Source,
			FactName:  facts.Code,
			FactValue: rec,
		}, nil
	}
	return e, nil
}

func decodeEntries(jsonArray json.RawMessage) ([]*spb.Entry, error) {
	var messages []json.RawMessage
	if err := json.Unmarshal(jsonArray, &messages); err != nil {
		return nil, err
	}
	entries := make([]*spb.Entry, len(messages))
	for i, msg := range messages {
		e := new(spb.Entry)
		if err := protojson.Unmarshal(msg, e); err != nil {
			return nil, err
		}
		e, err := rewriteEntry(e)
		if err != nil {
			log.Errorf("could not rewrite entry: %v", err)
			continue
		}
		entries[i] = e
	}
	return entries, nil
}

func decodeWireEntries(jsonArray json.RawMessage) ([]*spb.Entry, error) {
	var messages [][]byte
	if err := json.Unmarshal(jsonArray, &messages); err != nil {
		return nil, err
	}
	entries := make([]*spb.Entry, len(messages))
	for i, msg := range messages {
		e := new(spb.Entry)
		if err := (proto.UnmarshalOptions{}.Unmarshal(msg, e)); err != nil {
			return nil, err
		}
		e, err := rewriteEntry(e)
		if err != nil {
			log.Errorf("could not rewrite entry: %v", err)
			continue
		}
		entries[i] = e
	}
	return entries, nil
}

func (p *Proxy) reply(status string, args any) {
	p.err = p.out.Encode(&response{
		Status: status,
		Args:   args,
	})
}

// A Handler provides callbacks to handle requests issued by the indexer.
type Handler interface {
	// Analysis fetches a new analysis request. It is an error if there is an
	// unfinished request already in flight.
	Analysis() (*apb.AnalysisRequest, error)

	// Output delivers output from an ongoing analysis.
	Output(...*spb.Entry) error

	// Done reports an ongoing analysis as complete, with the error indicating
	// success or failure.
	Done(error)

	// File requests the contents of a file given a path and/or digest, at
	// least one of which must be nonempty.
	File(path, digest string) ([]byte, error)
}
