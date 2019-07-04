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
//   X → P: {"req":"analysis"}<LF>
//   P → X: {"rsp":"ok","args":{"unit":<unit>,"rev":<revision>,"fds":<addr>}}<LF>
//          {"rsp":"error","args":<error>}<LF>
//
//   X → P: {"req":"output","args":[<entry>...]}<LF>
//   P → X: {"rsp":"ok"}<LF>
//          {"rsp":"error","args":<error>}<LF>
//
//   X → P: {"req":"done","args":{"ok":true,"msg":<error>}}<LF>
//   P → X: {"rsp":"ok"}<LF>
//          {"rsp":"error","args":<error>}<LF>
//
//   X → P: {"req":"file","args":{"path":<path>,"digest":<digest>}}<LF>
//   P → X: {"rsp":"ok","args":{"path":<path>,"digest":<digest>,"content":<bytes>}}<LF>
//          {"rsp":"error","args":<error>}<LF>
//
// Where:
//
//    <addr>     -- service address
//    <bytes>    -- BASE-64 encoded bytes (string)
//    <digest>   -- file content digest (string)
//    <entry>    -- JSON encoded kythe.proto.Entry message
//    <error>    -- error diagnostic (string)
//    <path>     -- file path (string)
//    <revision> -- revision marker (string)
//    <unit>     -- JSON encoded kythe.proto.CompilationUnit message
//    <LF>       -- line feed character (decimal code 10)
//
// When rsp="error" in a reply, the args are an error string. The ordinary flow
// for the indexer is:
//
//     {"req":"analysis"}   -- to start a new analysis task
//     {"req":"output",...} -- to send outputs for the task
//     {"req":"file",...}   -- to fetch required input files.
//         ... (as many times as needed) ...
//     {"req":"done",...}   -- to mark the analysis as complete
//                             and to report success/failure
//
// In case of an indexing error, the indexer is free to terminate the analysis
// early and report {"req":"done","args":{"ok":false}} to the driver.
//
// In case of an error writing output data, the driver will report an error in
// response to the "output" call. Once this happens, the analysis is abandoned
// as if the indexer had called "done". Subsequent calls to "output" or "done"
// will behave as if no analysis is in progress. The indexer is free at that
// point to start a new analysis.
//
package proxy // import "kythe.io/kythe/go/platform/analysis/proxy"

import (
	"encoding/json"
	"errors"
	"io"

	apb "kythe.io/kythe/proto/analysis_go_proto"
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
	Status string      `json:"rsp"` // status message
	Args   interface{} `json:"args,omitempty"`
}

// A unit represents a compilation unit to be analyzed.
type unit struct {
	Unit            *apb.CompilationUnit `json:"unit"`
	Revision        string               `json:"rev,omitempty"`
	FileDataService string               `json:"fds,omitempty"`
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
				p.reply("ok", &unit{
					Unit:            req.Compilation,
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
			var entries []*spb.Entry
			err := json.Unmarshal(req.Args, &entries)
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

func (p *Proxy) reply(status string, args interface{}) {
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
