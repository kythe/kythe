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

package languageserver

import (
	"context"
	"encoding/json"
	"os"

	"kythe.io/kythe/go/util/log"

	"github.com/sourcegraph/go-langserver/pkg/lsp"
	"github.com/sourcegraph/jsonrpc2"
)

// ServerHandler produces a JSONRPC 2.0 handler from a Server
func ServerHandler(ls *Server) jsonrpc2.Handler {
	shutdownIssued := false
	return jsonrpc2.HandlerWithError(
		func(c context.Context, conn *jsonrpc2.Conn, req *jsonrpc2.Request) (any, error) {
			var (
				ret any
				err error
			)

			switch req.Method {
			case "initialize":
				var p lsp.InitializeParams
				if err := json.Unmarshal(*req.Params, &p); err != nil {
					return nil, err
				}
				ret, err = ls.Initialize(p)
			case "textDocument/didOpen":
				var p lsp.DidOpenTextDocumentParams
				if err := json.Unmarshal(*req.Params, &p); err != nil {
					return nil, err
				}
				err = ls.TextDocumentDidOpen(p)
			case "textDocument/didChange":
				var p lsp.DidChangeTextDocumentParams
				if err := json.Unmarshal(*req.Params, &p); err != nil {
					return nil, err
				}
				err = ls.TextDocumentDidChange(p)
			case "textDocument/references":
				var p lsp.ReferenceParams
				if err := json.Unmarshal(*req.Params, &p); err != nil {
					return nil, err
				}
				ret, err = ls.TextDocumentReferences(p)
			case "textDocument/definition":
				var p lsp.TextDocumentPositionParams
				if err := json.Unmarshal(*req.Params, &p); err != nil {
					return nil, err
				}
				ret, err = ls.TextDocumentDefinition(p)
			case "textDocument/didClose":
				var p lsp.DidCloseTextDocumentParams
				if err := json.Unmarshal(*req.Params, &p); err != nil {
					return nil, err
				}
				err = ls.TextDocumentDidClose(p)
			case "textDocument/hover":
				var p lsp.TextDocumentPositionParams
				if err := json.Unmarshal(*req.Params, &p); err != nil {
					return nil, err
				}
				ret, err = ls.TextDocumentHover(p)
			case "shutdown":
				log.Info("shutdown command received...")
				shutdownIssued = true
			case "exit":
				log.Info("exiting...")
				if shutdownIssued {
					os.Exit(0)
				} else {
					os.Exit(1)
				}
			}
			if err != nil {
				log.Info(err)
			}

			// Because we're already logging errors, returning them to the client
			// will only create noise. Some methods have formal error responses that
			// we will eventually return here
			return ret, nil
		})
}
