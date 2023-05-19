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

// Package languageserver provides an implementation of the Language Server
// Protocol v3.0 (https://github.com/Microsoft/language-server-protocol)
// This server implements the following capabilities:
//
//	textDocumentSync (full)
//	referenceProvider
package languageserver // import "kythe.io/kythe/go/languageserver"

import (
	"bytes"
	"context"
	"fmt"
	"strings"

	"kythe.io/kythe/go/services/xrefs"
	"kythe.io/kythe/go/util/kytheuri"
	"kythe.io/kythe/go/util/log"
	"kythe.io/kythe/go/util/markedsource"
	cpb "kythe.io/kythe/proto/common_go_proto"
	xpb "kythe.io/kythe/proto/xref_go_proto"

	"github.com/sourcegraph/go-langserver/pkg/lsp"
)

// Server provides a Language Server for interacting with data in a Kythe index
type Server struct {
	workspaces []Workspace
	docs       map[LocalFile]*document
	XRefs      xrefs.Service
	opts       *Options
}

// Options control optional behaviours of the language server implementation.
type Options struct {
	// The number of cross-references the server will request by default.
	// If ≤ 0, a reasonable default will be chosen.
	PageSize int

	// If set, this function will be called to produce a workspace for the
	// given LSP document. If unset, uses NewSettingsWorkspaceFromURI.
	NewWorkspace func(lsp.DocumentURI) (Workspace, error)
}

func (o *Options) pageSize() int {
	if o == nil || o.PageSize <= 0 {
		return 500
	}
	return o.PageSize
}

func (o *Options) newWorkspace(u lsp.DocumentURI) (Workspace, error) {
	if o == nil || o.NewWorkspace == nil {
		return NewSettingsWorkspaceFromURI(u)
	}
	return o.NewWorkspace(u)
}

// NewServer constructs a server that delegates cross-reference requests to the
// specified xrefs implementation. If opts == nil, sensible defaults are used.
func NewServer(xrefs xrefs.Service, opts *Options) Server {
	return Server{
		docs:       make(map[LocalFile]*document),
		workspaces: nil,
		XRefs:      xrefs,
		opts:       opts,
	}
}

// Initialize is invoked before any other methods, and allows the Server to
// receive configuration info (such as the project root) and announce its capabilities.
func (ls *Server) Initialize(params lsp.InitializeParams) (*lsp.InitializeResult, error) {
	log.Info("Server Initializing...")

	fullSync := lsp.TDSKFull
	return &lsp.InitializeResult{
		Capabilities: lsp.ServerCapabilities{
			TextDocumentSync: &lsp.TextDocumentSyncOptionsOrKind{
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
	log.Infof("Opened file: %q", params.TextDocument.URI)

	local, err := ls.localFromURI(params.TextDocument.URI)
	if err != nil {
		return fmt.Errorf("failed creating local path from URI %q:\n%v", params.TextDocument.URI, err)

	}

	log.Infof("Found %q in workspace %q", local.RelativePath, local.Workspace.Root())

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

	log.Infof("Server returned %d refs in file %q", len(dec.Reference), local)

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
	log.Infof("Found %d defs in file %q", len(defLocs), ticket.String())
	log.Infof("Found %d refs in file %q", len(refs), ticket.String())
	ls.docs[local] = newDocument(refs, string(dec.SourceText), params.TextDocument.Text, defLocs)
	log.Infof("Currently opened: %d files", len(ls.docs))

	return nil
}

// TextDocumentDidChange is called when the client edits a file. The Kythe
// Language Server simply stores the new content and marks the file as dirty
func (ls *Server) TextDocumentDidChange(params lsp.DidChangeTextDocumentParams) error {
	local, err := ls.localFromURI(params.TextDocument.URI)
	if err != nil {
		return err
	} else if len(params.ContentChanges) == 0 {
		// This should not be possible, since we request full sync, but some
		// plugins appear not to respect this in every case.  Treat this
		// similarly to a file that was never opened.
		return fmt.Errorf("unexpectedly empty change update on %q", params.TextDocument.URI)
	}

	if doc, ok := ls.docs[local]; ok {
		// Because the sync Kind is full text, each change notification
		// contains exactly 1 change with the full text of the document
		doc.updateSource(params.ContentChanges[0].Text)
		return nil
	}

	// If we get here, we were passed a change for a file that was never
	// opened.  That might be true, or it might be that the server restarted
	// and lost its context. Give the caller the benefit of the doubt and
	// attempt to open the file.
	log.Infof("No matching open document found for %q; attempting to open instead", params.TextDocument.URI)
	if err := ls.TextDocumentDidOpen(lsp.DidOpenTextDocumentParams{
		TextDocument: lsp.TextDocumentItem{URI: params.TextDocument.URI},
	}); err != nil {
		return err
	}

	// Assuming that worked, process the change.
	if doc, ok := ls.docs[local]; ok {
		doc.updateSource(params.ContentChanges[0].Text)
		return nil
	}
	return fmt.Errorf("change notification received for file that was never opened %q", local)
}

// TextDocumentDidClose removes all cached information about the open document. Because
// all information extracted from documents are stored internally to the document object,
// this removal shouldn't leak memory
func (ls *Server) TextDocumentDidClose(params lsp.DidCloseTextDocumentParams) error {
	log.Infof("Document close notification received: %v", params)
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
	log.Infof("Searching for references at %v", params.TextDocumentPositionParams)

	local, err := ls.localFromURI(params.TextDocument.URI)
	if err != nil {
		return nil, err
	}

	// If we don't have decorations we can't find references
	doc, exists := ls.docs[local]
	if !exists {
		log.Warningf("References requested from unknown file %q", local)
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
		PageSize:        int32(ls.opts.pageSize()),
	})
	if err != nil {
		return nil, fmt.Errorf("failed to find xrefs for ticket %q:\n%v", ref.ticket, err)
	}

	refs := xrefs.CrossReferences[ref.ticket]
	if refs == nil {
		log.Warningf("XRef service provided no xrefs for ticket %q", ref.ticket)
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
	log.Infof("Searching for definition at %v", params)
	local, err := ls.localFromURI(params.TextDocument.URI)
	if err != nil {
		return []lsp.Location{}, err
	}

	// If we don't have decorations we can't find definitions
	doc, exists := ls.docs[local]
	if !exists {
		log.Infof("References requested from unknown file %q", local)
		return []lsp.Location{}, nil
	}

	// If there's no ref at the location we don't have definitions
	ref := doc.xrefs(params.Position)
	if ref == nil {
		log.Warningf("No ref found at %v", params.Position)
		return []lsp.Location{}, nil
	}

	// If the ref's target definition is in the document's definition locations
	// we can return the loc without a service request
	if l, ok := doc.defLocs[ref.def]; ok {
		log.Infof("Found target definition for %q locally: %q at %v", ref.ticket, ref.def, *l)
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

			// Their definition range doesn't exist anymore, or is empty.  If
			// it's empty fall through and return the original location Kythe
			// reported.
			if l.Range.Start != l.Range.End {
				return []lsp.Location{}, nil
			}
		}
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
		log.Warningf("XRef service provided no xrefs for ticket %q", ref.ticket)
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
		log.Warningf("References requested from unknown file %q", local)
		return lsp.Hover{}, nil
	}

	// If there's no ref at the location we don't have documentation
	ref := doc.xrefs(params.Position)
	if ref == nil {
		log.Warningf("No ref found at %v", params.Position)
		return lsp.Hover{}, nil
	}

	// The first time we hover over a reference, generate hover documentation for it.
	if ref.markup == "" {
		docReply, err := ls.XRefs.Documentation(context.TODO(), &xpb.DocumentationRequest{
			Ticket: []string{ref.ticket},
		})
		if err != nil {
			log.Errorf("Error fetching documentation for %q: %v", ref.ticket, err)
			return lsp.Hover{}, nil
		}

		if len(docReply.Document) < 1 || docReply.Document[0].MarkedSource == nil {
			log.Warningf("No Documentation found for %q", ref.ticket)
			return lsp.Hover{}, nil
		}

		kuri, err := kytheuri.Parse(docReply.Document[0].Ticket)
		if err != nil {
			log.Errorf("Invalid ticket returned from documentation request: %v", err)
			return lsp.Hover{}, nil
		}
		ref.markup = markedsource.Render(docReply.Document[0].MarkedSource)
		ref.comment = stripComment(docReply.Document[0].GetText().GetRawText())
		ref.lang = kuri.Language
	}

	contents := []lsp.MarkedString{{
		Language: ref.lang,
		Value:    ref.markup,
	}}
	if ref.comment != "" {
		contents = append(contents, lsp.MarkedString{
			Language: ref.lang,
			Value:    ref.comment,
		})
	}
	return lsp.Hover{
		Contents: contents,
		Range:    ref.newRange,
	}, nil
}

func (ls *Server) localFromURI(u lsp.DocumentURI) (LocalFile, error) {
	for _, w := range ls.workspaces {
		local, err := w.LocalFromURI(u)
		if err == nil {
			return local, nil
		}
	}

	w, err := ls.opts.newWorkspace(u)
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
		URI:   local.URI(),
		Range: *r,
	}
}

func spanToRange(s *cpb.Span) *lsp.Range {
	if s == nil || s.Start == nil {
		return nil
	} else if s.End == nil {
		s.End = s.Start
	}

	// LineNumber is 1 indexed, so 0 indicates it's unknown, which in turn
	// means it can't be used for lookups so we discard these.
	if s.Start.LineNumber == 0 || s.End.LineNumber == 0 {
		return nil
	}

	return &lsp.Range{
		Start: lsp.Position{
			// N.B. LSP line numbers are 0-based, Kythe is 1-based.
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
			locs = append(locs, ls.locationInNewSource(*l))
		}
	}

	for _, a := range r.Definition {
		l := ls.anchorToLoc(w, a.Anchor)
		if l != nil {
			locs = append(locs, ls.locationInNewSource(*l))
		}
	}

	for _, a := range r.Declaration {
		l := ls.anchorToLoc(w, a.Anchor)
		if l != nil {
			locs = append(locs, ls.locationInNewSource(*l))
		}
	}

	return locs
}

// locationInNewSource maps a target loc to a new location. For files that are
// already open and indexed this will reflect any local patching; otherwise the
// input location is returned intact.
func (ls *Server) locationInNewSource(loc lsp.Location) lsp.Location {
	local, err := ls.localFromURI(loc.URI)
	if err != nil {
		return loc
	}

	// If we already have references for this location, update the location
	// through any patches we've discovered.
	if locDoc, ok := ls.docs[local]; ok {
		if newRange := locDoc.rangeInNewSource(loc.Range); newRange != nil {
			loc.Range = *newRange
		}
	}

	return loc
}

// stripComment removes linkage markup from documentation comments.  Kythe
// indexers may insert bracketed spans into comments to carry links to other
// places in the graph. This format is not generally supported, and we have no
// way in the protocol to convey the link targets, so remove the markers if
// they have been populated. This is a no-op if text contains no link markers.
func stripComment(text string) string {
	var buf bytes.Buffer
	for text != "" {
		i := strings.IndexAny(text, "\\[]")
		if i < 0 {
			buf.WriteString(text)
			break
		}
		buf.WriteString(text[:i])
		if text[i] == '\\' && i+1 < len(text) {
			buf.WriteByte(text[i+1])
			i++
		}
		text = text[i+1:]
	}
	return buf.String()
}
