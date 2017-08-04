/*
 * Copyright 2017 Google Inc. All rights reserved.
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

// Package languageserver provides an implementation of the Language Server
// Protocol v3.0 (https://github.com/Microsoft/language-server-protocol)
// This server implements the following capabilities:
// 		textDocumentSync (full)
//		referenceProvider
package languageserver

import (
	"context"
	"fmt"
	"log"

	"kythe.io/kythe/go/services/xrefs"
	"kythe.io/kythe/go/util/kytheuri"

	cpb "kythe.io/kythe/proto/common_proto"
	xpb "kythe.io/kythe/proto/xref_proto"

	"github.com/sourcegraph/go-langserver/pkg/lsp"
)

// Server provides a Language Server for interacting with data in a Kythe index
type Server struct {
	docs  map[string]*document
	paths pathConfig
	XRefs xrefs.Service
}

// NewServer constructs a Server object
func NewServer(x xrefs.Service) Server {
	var p pathConfig
	return Server{
		docs:  make(map[string]*document),
		paths: p,
		XRefs: x,
	}
}

// Initialize is invoked before any other methods, and allows the Server to
// receive configuration info (such as the project root) and announce its capabilities.
func (ls *Server) Initialize(params lsp.InitializeParams) (*lsp.InitializeResult, error) {
	log.Println("Server Initializing...")

	fullSync := lsp.TDSKFull
	return &lsp.InitializeResult{
		Capabilities: lsp.ServerCapabilities{
			TextDocumentSync: lsp.TextDocumentSyncOptionsOrKind{
				Kind:    &fullSync,
				Options: nil,
			},
			ReferencesProvider: true,
		},
	}, nil
}

// TextDocumentDidOpen allows the client to inform the Server that a file has
// been opened. The Kythe Language Server uses this time to fetch file
// decorations.
func (ls *Server) TextDocumentDidOpen(params lsp.DidOpenTextDocumentParams) error {
	local, err := ls.paths.localFromURI(params.TextDocument.URI)
	if err != nil {
		return fmt.Errorf("failed creating local path from URI (%s):\n%v", params.TextDocument.URI, err)

	}

	ticket, err := ls.paths.kytheURIFromLocal(local)
	if err != nil {
		return err
	}

	dec, err := ls.XRefs.Decorations(context.TODO(), &xpb.DecorationsRequest{
		Location: &xpb.Location{
			Ticket: ticket.String(),
		},
		References: true,
		// TODO(djrenren): Use this for definition lookups without a service request
		TargetDefinitions: false,
		SourceText:        true,
	})

	if err != nil {
		return fmt.Errorf("failed to find xrefs for %s:\n%v", local, err)
	}

	var refs []*RefResolution
	for _, r := range dec.Reference {
		if r.Span == nil {
			continue
		}

		rng := spanToRange(r.Span)
		if rng == nil {
			continue
		}

		refs = append(refs, &RefResolution{
			ticket:   r.TargetTicket,
			oldRange: *rng,
		})
	}

	log.Printf("Found %d refs in file '%s'", len(refs), local)
	ls.docs[local] = newDocument(refs, string(dec.SourceText), params.TextDocument.Text)
	return nil
}

// TextDocumentDidChange is called when the client edits a file. The Kythe
// Language Server simply stores the new content and marks the file as dirty
func (ls *Server) TextDocumentDidChange(params lsp.DidChangeTextDocumentParams) error {
	local, err := ls.paths.localFromURI(params.TextDocument.URI)
	if err != nil {
		return err
	}

	if doc, ok := ls.docs[local]; ok {
		// Because the sync Kind is full text, each change notification
		// contains exactly 1 change with the full text of the document
		doc.updateSource(params.ContentChanges[0].Text)
	} else {
		return fmt.Errorf("change notification received for file that was never opened")
	}

	return nil
}

// TextDocumentReferences uses a position in code to produce a list of
// locations throughout the project that reference the same semantic node. This
// can trigger a diff if the source file is dirty
func (ls *Server) TextDocumentReferences(params lsp.ReferenceParams) ([]lsp.Location, error) {
	local, err := ls.paths.localFromURI(params.TextDocument.URI)
	if err != nil {
		return nil, err
	}

	// If we don't have decorations we can't find references
	doc, exists := ls.docs[local]
	if !exists {
		log.Printf("References requested from unknown file '%s'", local)
		return []lsp.Location{}, nil
	}

	ticket, err := doc.xrefs(params.Position)
	if err != nil {
		return []lsp.Location{}, nil
	}

	xrefs, err := ls.XRefs.CrossReferences(context.TODO(), &xpb.CrossReferencesRequest{
		Ticket:          []string{*ticket},
		DeclarationKind: xpb.CrossReferencesRequest_ALL_DECLARATIONS,
		DefinitionKind:  xpb.CrossReferencesRequest_BINDING_DEFINITIONS,
		ReferenceKind:   xpb.CrossReferencesRequest_NON_CALL_REFERENCES,
		PageSize:        50, // TODO(djrenren): make this configurable
	})
	if err != nil {
		return nil, fmt.Errorf("failed to find xrefs for ticket '%s':\n%v", *ticket, err)

	}

	refs := xrefs.CrossReferences[*ticket]
	if refs == nil {
		log.Printf("XRef service provided no xrefs for ticket '%s'", *ticket)
		return []lsp.Location{}, nil
	}

	var locs []lsp.Location
	for _, a := range refs.Reference {
		l := ls.anchorToLoc(*a)
		if l != nil {
			locs = append(locs, *l)
		}
	}

	return locs, nil
}

func (ls *Server) anchorToLoc(a xpb.CrossReferencesReply_RelatedAnchor) *lsp.Location {
	if a.Anchor == nil || a.Anchor.Span == nil {
		return nil
	}

	r := spanToRange(a.Anchor.Span)
	if r == nil {
		return nil
	}

	ticket, err := kytheuri.Parse(a.Anchor.Parent)
	if err != nil {
		return nil
	}

	local, err := ls.paths.localFromKytheURI(*ticket)
	if err != nil {
		return nil
	}

	return &lsp.Location{
		URI:   lsp.DocumentURI(fmt.Sprintf("file://%s", local)),
		Range: lsp.Range(*r),
	}
}

func spanToRange(s *cpb.Span) *lsp.Range {
	if s == nil || s.Start == nil || s.End == nil {
		return nil
	}

	// LineNumber is 1 indexed, so 0 indicates unknown which
	// means it can't be used for lookups so we discard
	if s.Start.LineNumber == 0 || s.End.LineNumber == 0 {
		return nil
	}

	return &lsp.Range{
		Start: lsp.Position{
			Line:      int(s.Start.LineNumber - 1),
			Character: int(s.Start.ColumnOffset),
		},
		End: lsp.Position{
			Line:      int(s.End.LineNumber - 1),
			Character: int(s.End.ColumnOffset),
		},
	}
}
