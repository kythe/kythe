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
	"kythe.io/kythe/go/util/markedsource"

	cpb "kythe.io/kythe/proto/common_proto"
	xpb "kythe.io/kythe/proto/xref_proto"

	"github.com/sourcegraph/go-langserver/pkg/lsp"
)

// Server provides a Language Server for interacting with data in a Kythe index
type Server struct {
	workspaces   []Workspace
	docs         map[LocalFile]*document
	XRefs        xrefs.Service
	newWorkspace func(lsp.DocumentURI) (Workspace, error)
}

// NewServer constructs a Server object
func NewServer(x xrefs.Service, newWorkspace func(lsp.DocumentURI) (Workspace, error)) Server {
	return Server{
		docs:         make(map[LocalFile]*document),
		workspaces:   nil,
		XRefs:        x,
		newWorkspace: newWorkspace,
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
			HoverProvider:      true,
			DefinitionProvider: true,
		},
	}, nil
}

// TextDocumentDidOpen allows the client to inform the Server that a file has
// been opened. The Kythe Language Server uses this time to fetch file
// decorations.
func (ls *Server) TextDocumentDidOpen(params lsp.DidOpenTextDocumentParams) error {
	log.Printf("Opened file: %q", params.TextDocument.URI)

	local, err := ls.localFromURI(params.TextDocument.URI)
	if err != nil {
		return fmt.Errorf("failed creating local path from URI %q:\n%v", params.TextDocument.URI, err)

	}

	log.Printf("Found %q in workspace %q", local.RelativePath, local.Workspace.Root())

	ticket, err := local.KytheURI()
	if err != nil {
		return err
	}

	dec, err := ls.XRefs.Decorations(context.TODO(), &xpb.DecorationsRequest{
		Location: &xpb.Location{
			Ticket: ticket.String(),
		},
		References:        true,
		TargetDefinitions: true,
		SourceText:        true,
	})

	if err != nil {
		return fmt.Errorf("failed to find xrefs for %q:\n%v", local, err)
	}

	log.Printf("Server returned %d refs in file %q", len(dec.Reference), local)

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
			def:      r.TargetDefinition,
			oldRange: *rng,
		})
	}

	defLocs := ls.defLocations(local.Workspace, dec.DefinitionLocations)
	log.Printf("Found %d defs in file %q", len(defLocs), ticket.String())
	log.Printf("Found %d refs in file %q", len(refs), ticket.String())
	ls.docs[local] = newDocument(refs, string(dec.SourceText), params.TextDocument.Text, defLocs)
	log.Printf("Currently opened: %d files", len(ls.docs))

	return nil
}

// TextDocumentDidChange is called when the client edits a file. The Kythe
// Language Server simply stores the new content and marks the file as dirty
func (ls *Server) TextDocumentDidChange(params lsp.DidChangeTextDocumentParams) error {
	local, err := ls.localFromURI(params.TextDocument.URI)
	if err != nil {
		return err
	}

	if doc, ok := ls.docs[local]; ok {
		// Because the sync Kind is full text, each change notification
		// contains exactly 1 change with the full text of the document
		doc.updateSource(params.ContentChanges[0].Text)
	} else {
		return fmt.Errorf("change notification received for file that was never opened %q", local)
	}

	return nil
}

// TextDocumentDidClose removes all cached information about the open document. Because
// all information extracted from documents are stored internally to the document object,
// this removal shouldn't leak memory
func (ls *Server) TextDocumentDidClose(params lsp.DidCloseTextDocumentParams) error {
	log.Printf("Document close notification received: %v", params)
	local, err := ls.localFromURI(params.TextDocument.URI)
	if err != nil {
		return err
	}

	delete(ls.docs, local)
	return nil
}

// TextDocumentReferences uses a position in code to produce a list of
// locations throughout the project that reference the same semantic node. This
// can trigger a diff if the source file is dirty.
//
// NOTE: As per the lsp spec, references must return an error or a valid array.
// Therefore, if no error is returned, a non-nil location slice must be returned
func (ls *Server) TextDocumentReferences(params lsp.ReferenceParams) ([]lsp.Location, error) {
	log.Printf("Searching for references at %v", params.TextDocumentPositionParams)

	local, err := ls.localFromURI(params.TextDocument.URI)
	if err != nil {
		return nil, err
	}

	// If we don't have decorations we can't find references
	doc, exists := ls.docs[local]
	if !exists {
		log.Printf("References requested from unknown file %q", local)
		return []lsp.Location{}, nil
	}

	ref := doc.xrefs(params.Position)
	if ref == nil {
		return []lsp.Location{}, nil
	}

	xrefs, err := ls.XRefs.CrossReferences(context.TODO(), &xpb.CrossReferencesRequest{
		Ticket:          []string{ref.ticket},
		DeclarationKind: xpb.CrossReferencesRequest_ALL_DECLARATIONS,
		DefinitionKind:  xpb.CrossReferencesRequest_BINDING_DEFINITIONS,
		ReferenceKind:   xpb.CrossReferencesRequest_NON_CALL_REFERENCES,
		PageSize:        50, // TODO(djrenren): make this configurable
	})
	if err != nil {
		return nil, fmt.Errorf("failed to find xrefs for ticket %q:\n%v", ref.ticket, err)
	}

	refs := xrefs.CrossReferences[ref.ticket]
	if refs == nil {
		log.Printf("XRef service provided no xrefs for ticket %q", ref.ticket)
		return []lsp.Location{}, nil
	}

	return ls.refLocs(local.Workspace, refs), nil
}

// TextDocumentDefinition uses a position in code to produce a list of
// locations throughout the project that define the semantic node at the original position.
// This can trigger a diff if the source file is dirty
//
// NOTE: As per the lsp spec, definition must return an error or a non-null result.
// Therefore, if no error is returned, a non-nil location slice must be returned
func (ls *Server) TextDocumentDefinition(params lsp.TextDocumentPositionParams) ([]lsp.Location, error) {
	log.Printf("Searching for definition at %v", params)
	local, err := ls.localFromURI(params.TextDocument.URI)
	if err != nil {
		return []lsp.Location{}, err
	}

	// If we don't have decorations we can't find definitions
	doc, exists := ls.docs[local]
	if !exists {
		log.Printf("References requested from unknown file %q", local)
		return []lsp.Location{}, nil
	}

	// If there's no ref at the location we don't have definitions
	ref := doc.xrefs(params.Position)
	if ref == nil {
		log.Printf("No ref found at %v", params.Position)
		return []lsp.Location{}, nil
	}

	// If the ref's target definition is in the document's definition locations
	// we can return the loc without a service request
	if l, ok := doc.defLocs[ref.def]; ok {
		log.Printf("Found target definition for %q locally: %q at %v", ref.ticket, ref.def, *l)
		defLocal, err := local.Workspace.LocalFromURI(l.URI)

		if err != nil {
			return []lsp.Location{}, nil
		}

		// If we have the doc containing the reference, map it to its new location
		if defDoc, ok := ls.docs[defLocal]; ok {
			newRange := defDoc.rangeInNewSource(l.Range)
			loc := *l
			if newRange != nil {
				loc.Range = *newRange
				return []lsp.Location{loc}, nil
			}

			// There definition range doesn't exist anymore
			return []lsp.Location{}, nil
		}

		log.Printf("Unable to map definition to local location")
		// We don't how to map it so we just return the location from Kythe
		return []lsp.Location{*l}, nil
	}

	xrefs, err := ls.XRefs.CrossReferences(context.TODO(), &xpb.CrossReferencesRequest{
		Ticket:          []string{ref.ticket},
		DeclarationKind: xpb.CrossReferencesRequest_ALL_DECLARATIONS,
		DefinitionKind:  xpb.CrossReferencesRequest_BINDING_DEFINITIONS,
	})

	if err != nil {
		return nil, fmt.Errorf("failed to find xrefs for ticket %q: %v", ref.ticket, err)
	}

	refs := xrefs.CrossReferences[ref.ticket]
	if refs == nil {
		log.Printf("XRef service provided no xrefs for ticket %q", ref.ticket)
		return []lsp.Location{}, nil
	}

	return ls.refLocs(local.Workspace, refs), nil
}

// TextDocumentHover produces a documentation string for the entity referenced at a given location
func (ls *Server) TextDocumentHover(params lsp.TextDocumentPositionParams) (lsp.Hover, error) {
	local, err := ls.localFromURI(params.TextDocument.URI)
	if err != nil {
		return lsp.Hover{}, err
	}

	// If we don't have decorations we can't find documentation
	doc, exists := ls.docs[local]
	if !exists {
		log.Printf("References requested from unknown file %q", local)
		return lsp.Hover{}, nil
	}

	// If there's no ref at the location we don't have documentation
	ref := doc.xrefs(params.Position)
	if ref == nil {
		log.Printf("No ref found at %v", params.Position)
		return lsp.Hover{}, nil
	}

	docReply, err := ls.XRefs.Documentation(context.TODO(), &xpb.DocumentationRequest{
		Ticket: []string{ref.ticket},
	})
	if err != nil {
		log.Printf("Error fetching documentation for %q: %v", ref.ticket, err)
		return lsp.Hover{}, nil
	}

	if len(docReply.Document) < 1 || docReply.Document[0].MarkedSource == nil {
		log.Printf("No Documentation found for %q", ref.ticket)
		return lsp.Hover{}, nil
	}

	kuri, err := kytheuri.Parse(docReply.Document[0].Ticket)
	if err != nil {
		log.Printf("Invalid ticket returned from documentation request: %v", err)
		return lsp.Hover{}, nil
	}

	sig := markedsource.Render(docReply.Document[0].MarkedSource)

	return lsp.Hover{
		Contents: []lsp.MarkedString{{
			Language: kuri.Language,
			Value:    sig,
		}},
		Range: ref.newRange,
	}, nil
}

func (ls *Server) localFromURI(u lsp.DocumentURI) (LocalFile, error) {
	for _, w := range ls.workspaces {
		local, err := w.LocalFromURI(u)
		if err == nil {
			return local, nil
		}
	}

	w, err := ls.newWorkspace(u)
	if err != nil {
		return LocalFile{}, err
	}

	ls.workspaces = append(ls.workspaces, w)
	return w.LocalFromURI(u)
}

func (ls *Server) anchorToLoc(w Workspace, a *xpb.Anchor) *lsp.Location {
	if a == nil || a.Span == nil {
		return nil
	}

	r := spanToRange(a.Span)
	if r == nil {
		return nil
	}

	ticket, err := kytheuri.Parse(a.Parent)
	if err != nil {
		return nil
	}

	local, err := w.LocalFromKytheURI(*ticket)
	if err != nil {
		return nil
	}

	return &lsp.Location{
		URI:   lsp.DocumentURI(fmt.Sprintf("file://%s", local)),
		Range: *r,
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

func (ls *Server) defLocations(w Workspace, t map[string]*xpb.Anchor) map[string]*lsp.Location {
	m := make(map[string]*lsp.Location)
	for k, v := range t {
		l := ls.anchorToLoc(w, v)
		if l != nil {
			m[k] = l
		}
	}
	return m
}

// refLocs takes a cross reference set and produces a slice of locations. This slice
// is guaranteed to be non-null even if empty
func (ls *Server) refLocs(w Workspace, r *xpb.CrossReferencesReply_CrossReferenceSet) []lsp.Location {
	locs := []lsp.Location{}
	for _, a := range r.Reference {
		l := ls.anchorToLoc(w, a.Anchor)
		if l != nil {
			locs = append(locs, *l)
		}
	}

	for _, a := range r.Definition {
		l := ls.anchorToLoc(w, a.Anchor)
		if l != nil {
			locs = append(locs, *l)
		}
	}

	for _, a := range r.Declaration {
		l := ls.anchorToLoc(w, a.Anchor)
		if l != nil {
			locs = append(locs, *l)
		}
	}

	return locs
}
