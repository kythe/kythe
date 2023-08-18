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

// Package xrefs provides test utilities for the xrefs.Service.
//
// This package includes the Atomizer which allows for XRefService proto
// response messages to be shattered in GraphStore entries.  By indexing,
// post-processing, and serving source code with verifier
// (http://www.kythe.io/docs/kythe-verifier.html) goals, using the Atomizer to
// shatter the served DecorationsReply and CrossReferencesReply messages, and
// feeding the resulting entries into the verifier, one can write integration
// tests between the Kythe indexers and Kythe server.
package xrefs // import "kythe.io/kythe/go/test/services/xrefs"

import (
	"context"
	"fmt"
	"path/filepath"
	"strings"

	"kythe.io/kythe/go/util/kytheuri"
	"kythe.io/kythe/go/util/log"
	"kythe.io/kythe/go/util/markedsource"
	"kythe.io/kythe/go/util/schema/edges"
	"kythe.io/kythe/go/util/schema/facts"
	"kythe.io/kythe/go/util/schema/nodes"

	"google.golang.org/protobuf/proto"

	cpb "kythe.io/kythe/proto/common_go_proto"
	spb "kythe.io/kythe/proto/storage_go_proto"
	xpb "kythe.io/kythe/proto/xref_go_proto"
)

// An Atomizer shatters XRefService proto replies into equivalent GraphStore
// entries.  The resulting composition of the emitted entries reflects a Kythe
// graph following the Kythe schema in most respects.  In particular, the
// emitted graph approximates an input Kythe graph that was used to derive the
// XRefService replies (through post-processing and serving).
type Atomizer func(context.Context, *spb.Entry) error

type atomizerPanic struct{ error }

// catchErrors recovers from a possible panic.  If the panic was caused by a
// atomizerPanic, the underlying error will replace the ret error.
// Otherwise, the panic error will directly replace the ret error.  If
// recovering from a panic and *ret != nil, the original ret error will be
// logged as a warning.
func (a Atomizer) catchErrors(ret *error) {
	if err := recover(); err != nil {
		if pe, ok := err.(atomizerPanic); ok {
			if *ret != nil {
				log.Warningf("%v", *ret)
			}
			*ret = pe.error
		} else {
			panic(err)
		}
	}
}

func (a Atomizer) emit(ctx context.Context, entry *spb.Entry) {
	if err := a(ctx, entry); err != nil {
		// Instead of checking for errors throughout the code, panic with a special
		// error that will be caught in the top-level methods by
		// Atomizer.catchErrors.
		panic(atomizerPanic{err})
	}
}

// Decorations emits a file node for the reply's Location, emits an anchor node
// for each reference along with an edge to its target, and emits all node
// facts directly.
func (a Atomizer) Decorations(ctx context.Context, decor *xpb.DecorationsReply) (ret error) {
	defer a.catchErrors(&ret)
	file := a.parseTicket(decor.Location.Ticket)
	a.emitFact(ctx, file, facts.NodeKind, []byte(nodes.File))
	a.emitFact(ctx, file, facts.Text, decor.SourceText)
	a.emitFact(ctx, file, facts.TextEncoding, []byte(decor.Encoding))
	for _, ref := range decor.Reference {
		anchor := a.anchorNode(ctx, decor.Location.Ticket, ref.Span, nil)
		a.emitEdge(ctx, anchor, ref.Kind, a.parseTicket(ref.TargetTicket))
	}
	for ticket, node := range decor.Nodes {
		for name, val := range node.Facts {
			a.emitFact(ctx, a.parseTicket(ticket), name, val)
		}
	}
	return nil
}

// CrossReferences emits anchor nodes for all RelatedAnchors along with edges to
// their targets, emits edges for each RelatedNode, and emits all node facts
// directly.
// TODO(schroederc): handle Callers
func (a Atomizer) CrossReferences(ctx context.Context, xrefs *xpb.CrossReferencesReply) (ret error) {
	defer a.catchErrors(&ret)
	for _, xrs := range xrefs.CrossReferences {
		src := a.parseTicket(xrs.Ticket)
		if xrs.MarkedSource != nil {
			rec, err := proto.Marshal(xrs.MarkedSource)
			if err != nil {
				return err
			}
			a.emitFact(ctx, src, facts.Code, rec)

			rendered := markedsource.Render(xrs.MarkedSource)
			a.emitFact(ctx, src, facts.Code+"/rendered", []byte(rendered))
			ident := markedsource.RenderSimpleIdentifier(xrs.MarkedSource, markedsource.PlaintextContent, nil)
			a.emitFact(ctx, src, facts.Code+"/rendered/identifier", []byte(ident))
			params := markedsource.RenderSimpleParams(xrs.MarkedSource, markedsource.PlaintextContent, nil)
			if len(params) > 0 {
				a.emitFact(ctx, src, facts.Code+"/rendered/params", []byte(strings.Join(params, ",")))
			}
		}
		a.emitAnchors(ctx, src, xrs.Definition)
		a.emitAnchors(ctx, src, xrs.Declaration)
		a.emitAnchors(ctx, src, xrs.Reference)
		a.emitAnchors(ctx, src, xrs.Caller)
		for _, rn := range xrs.RelatedNode {
			if rn.RelationKind == edges.Param || rn.Ordinal != 0 {
				a.emitOrdinal(ctx, src, rn.RelationKind, a.parseTicket(rn.Ticket), rn.Ordinal)
			} else {
				a.emitEdge(ctx, src, rn.RelationKind, a.parseTicket(rn.Ticket))
			}
		}
	}
	for ticket, node := range xrefs.Nodes {
		for name, val := range node.Facts {
			a.emitFact(ctx, a.parseTicket(ticket), name, val)
		}
	}
	return nil
}

func (a Atomizer) parseTicket(ticket string) *spb.VName {
	uri, err := kytheuri.Parse(ticket)
	if err != nil {
		panic(atomizerPanic{err})
	}
	return uri.VName()
}

// anchorURI synthesizes an anchor URI from a span and its parent file.
func (a Atomizer) anchorURI(fileTicket string, span *cpb.Span) *kytheuri.URI {
	uri, err := kytheuri.Parse(fileTicket)
	if err != nil {
		panic(atomizerPanic{err})
	}
	uri.Signature = fmt.Sprintf("a[%d,%d)", span.GetStart().GetByteOffset(), span.GetEnd().GetByteOffset())
	// The language doesn't have to exactly match the schema; just use the file's
	// extension as an approximation.
	uri.Language = strings.TrimPrefix(filepath.Ext(uri.Path), ".")
	return uri
}

func (a Atomizer) anchorNode(ctx context.Context, parentFile string, span *cpb.Span, snippet *cpb.Span) *spb.VName {
	anchor := a.anchorURI(parentFile, span).VName()
	a.emitFact(ctx, anchor, facts.NodeKind, []byte(nodes.Anchor))
	a.emitFact(ctx, anchor, facts.AnchorStart, []byte(fmt.Sprintf("%d", span.Start.ByteOffset)))
	a.emitFact(ctx, anchor, facts.AnchorEnd, []byte(fmt.Sprintf("%d", span.End.ByteOffset)))
	if snippet != nil {
		a.emitFact(ctx, anchor, facts.SnippetStart, []byte(fmt.Sprintf("%d", snippet.Start.ByteOffset)))
		a.emitFact(ctx, anchor, facts.SnippetEnd, []byte(fmt.Sprintf("%d", snippet.End.ByteOffset)))
	}
	return anchor
}

func (a Atomizer) emitFact(ctx context.Context, src *spb.VName, fact string, value []byte) {
	a.emit(ctx, &spb.Entry{
		Source:    src,
		FactName:  fact,
		FactValue: value,
	})
}

func (a Atomizer) emitOrdinal(ctx context.Context, src *spb.VName, kind string, tgt *spb.VName, ordinal int32) {
	a.emitEdge(ctx, src, fmt.Sprintf("%s.%d", kind, ordinal), tgt)
}

func (a Atomizer) emitEdge(ctx context.Context, src *spb.VName, kind string, tgt *spb.VName) {
	a.emit(ctx, &spb.Entry{
		Source:   src,
		FactName: "/",
		EdgeKind: kind,
		Target:   tgt,
	})
}

func (a Atomizer) emitAnchors(ctx context.Context, n *spb.VName, ras []*xpb.CrossReferencesReply_RelatedAnchor) {
	for _, ra := range ras {
		anchor := a.anchorNode(ctx, ra.Anchor.Parent, ra.Anchor.Span, ra.Anchor.SnippetSpan)
		a.emitEdge(ctx, anchor, ra.Anchor.Kind, n)
	}
}
