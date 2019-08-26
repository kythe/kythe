/*
 * Copyright 2015 The Kythe Authors. All rights reserved.
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

// Package xrefs provides a high-performance table-based implementation of the
// xrefs.Service.
//
// Table format:
//   decor:<ticket>         -> srvpb.FileDecorations
//   docs:<ticket>          -> srvpb.Document
//   xrefs:<ticket>         -> srvpb.PagedCrossReferences
//   xrefPages:<page_key>   -> srvpb.PagedCrossReferences_Page
package xrefs // import "kythe.io/kythe/go/serving/xrefs"

import (
	"context"
	"encoding/base64"
	"flag"
	"fmt"
	"log"
	"regexp"
	"strings"

	"kythe.io/kythe/go/services/xrefs"
	"kythe.io/kythe/go/storage/table"
	"kythe.io/kythe/go/util/flagutil"
	"kythe.io/kythe/go/util/kytheuri"
	"kythe.io/kythe/go/util/schema/edges"
	"kythe.io/kythe/go/util/schema/facts"
	"kythe.io/kythe/go/util/schema/tickets"
	"kythe.io/kythe/go/util/span"

	"bitbucket.org/creachadair/stringset"
	"github.com/golang/protobuf/proto"
	"github.com/golang/snappy"
	"golang.org/x/net/trace"
	"google.golang.org/grpc/codes"
	"google.golang.org/grpc/status"

	cpb "kythe.io/kythe/proto/common_go_proto"
	ipb "kythe.io/kythe/proto/internal_go_proto"
	srvpb "kythe.io/kythe/proto/serving_go_proto"
	xpb "kythe.io/kythe/proto/xref_go_proto"
)

var (
	mergeCrossReferences = flag.Bool("merge_cross_references", true, "Whether to merge nodes when responding to a CrossReferencesRequest")

	experimentalCrossReferenceIndirectionKinds flagutil.StringMultimap
)

func init() {
	flag.Var(&experimentalCrossReferenceIndirectionKinds, "experimental_cross_reference_indirection_kinds",
		`Comma-separated set of key-value pairs (node_kind=edge_kind) to indirect through in CrossReferences.  For example, "talias=/kythe/edge/aliases" indicates that the targets of a 'talias' node's '/kythe/edge/aliases' related nodes will have their cross-references merged into the root 'talias' node's.  A "*=edge_kind" entry indicates to indirect through the specified edge kind for any node kind.`)
}

type staticLookupTables interface {
	fileDecorations(ctx context.Context, ticket string) (*srvpb.FileDecorations, error)
	crossReferences(ctx context.Context, ticket string) (*srvpb.PagedCrossReferences, error)
	crossReferencesPage(ctx context.Context, key string) (*srvpb.PagedCrossReferences_Page, error)
	documentation(ctx context.Context, ticket string) (*srvpb.Document, error)
}

// SplitTable implements the xrefs Service interface using separate static
// lookup tables for each API component.
type SplitTable struct {
	// Decorations is a table of srvpb.FileDecorations keyed by their source
	// location tickets.
	Decorations table.Proto

	// CrossReferences is a table of srvpb.PagedCrossReferences keyed by their
	// source node tickets.
	CrossReferences table.Proto

	// CrossReferencePages is a table of srvpb.PagedCrossReferences_Pages keyed by
	// their page keys.
	CrossReferencePages table.Proto

	// Documentation is a table of srvpb.Documents keyed by their node ticket.
	Documentation table.Proto
}

func (s *SplitTable) fileDecorations(ctx context.Context, ticket string) (*srvpb.FileDecorations, error) {
	tracePrintf(ctx, "Reading FileDecorations: %s", ticket)
	var fd srvpb.FileDecorations
	return &fd, s.Decorations.Lookup(ctx, []byte(ticket), &fd)
}
func (s *SplitTable) crossReferences(ctx context.Context, ticket string) (*srvpb.PagedCrossReferences, error) {
	tracePrintf(ctx, "Reading PagedCrossReferences: %s", ticket)
	var cr srvpb.PagedCrossReferences
	return &cr, s.CrossReferences.Lookup(ctx, []byte(ticket), &cr)
}
func (s *SplitTable) crossReferencesPage(ctx context.Context, key string) (*srvpb.PagedCrossReferences_Page, error) {
	tracePrintf(ctx, "Reading PagedCrossReferences.Page: %s", key)
	var p srvpb.PagedCrossReferences_Page
	return &p, s.CrossReferencePages.Lookup(ctx, []byte(key), &p)
}
func (s *SplitTable) documentation(ctx context.Context, ticket string) (*srvpb.Document, error) {
	tracePrintf(ctx, "Reading Document: %s", ticket)
	var d srvpb.Document
	return &d, s.Documentation.Lookup(ctx, []byte(ticket), &d)
}

// Key prefixes for the combinedTable implementation.
const (
	crossRefTablePrefix      = "xrefs:"
	crossRefPageTablePrefix  = "xrefPages:"
	decorTablePrefix         = "decor:"
	documentationTablePrefix = "docs:"
)

type combinedTable struct{ table.Proto }

func (c *combinedTable) fileDecorations(ctx context.Context, ticket string) (*srvpb.FileDecorations, error) {
	var fd srvpb.FileDecorations
	return &fd, c.Lookup(ctx, DecorationsKey(ticket), &fd)
}
func (c *combinedTable) crossReferences(ctx context.Context, ticket string) (*srvpb.PagedCrossReferences, error) {
	var cr srvpb.PagedCrossReferences
	return &cr, c.Lookup(ctx, CrossReferencesKey(ticket), &cr)
}
func (c *combinedTable) crossReferencesPage(ctx context.Context, key string) (*srvpb.PagedCrossReferences_Page, error) {
	var p srvpb.PagedCrossReferences_Page
	return &p, c.Lookup(ctx, CrossReferencesPageKey(key), &p)
}
func (c *combinedTable) documentation(ctx context.Context, ticket string) (*srvpb.Document, error) {
	var d srvpb.Document
	return &d, c.Lookup(ctx, DocumentationKey(ticket), &d)
}

// NewSplitTable returns a table based on the given serving tables for each API
// component.
func NewSplitTable(c *SplitTable) *Table { return &Table{c} }

// NewCombinedTable returns a table for the given combined xrefs lookup table.
// The table's keys are expected to be constructed using only the *Key functions.
func NewCombinedTable(t table.Proto) *Table { return &Table{&combinedTable{t}} }

// DecorationsKey returns the decorations CombinedTable key for the given source
// location ticket.
func DecorationsKey(ticket string) []byte {
	return []byte(decorTablePrefix + ticket)
}

// CrossReferencesKey returns the cross-references CombinedTable key for the
// given node ticket.
func CrossReferencesKey(ticket string) []byte {
	return []byte(crossRefTablePrefix + ticket)
}

// CrossReferencesPageKey returns the cross-references page CombinedTable key
// for the given key.
func CrossReferencesPageKey(key string) []byte {
	return []byte(crossRefPageTablePrefix + key)
}

// DocumentationKey returns the documentation CombinedTable key for the given
// ticket.
func DocumentationKey(ticket string) []byte {
	return []byte(documentationTablePrefix + ticket)
}

// Table implements the xrefs Service interface using static lookup tables.
type Table struct{ staticLookupTables }

const (
	defaultPageSize = 2048
	maxPageSize     = 10000
)

func nodeToInfo(patterns []*regexp.Regexp, n *srvpb.Node) *cpb.NodeInfo {
	ni := &cpb.NodeInfo{Facts: make(map[string][]byte, len(n.Fact))}
	for _, f := range n.Fact {
		if xrefs.MatchesAny(f.Name, patterns) {
			ni.Facts[f.Name] = f.Value
		}
	}
	if len(ni.Facts) == 0 {
		return nil
	}
	return ni
}

// Decorations implements part of the xrefs Service interface.
func (t *Table) Decorations(ctx context.Context, req *xpb.DecorationsRequest) (*xpb.DecorationsReply, error) {
	if req.GetLocation() == nil || req.GetLocation().Ticket == "" {
		return nil, status.Error(codes.InvalidArgument, "missing location")
	}

	ticket, err := kytheuri.Fix(req.GetLocation().Ticket)
	if err != nil {
		return nil, status.Errorf(codes.InvalidArgument, "invalid ticket %q: %v", req.GetLocation().Ticket, err)
	}

	decor, err := t.fileDecorations(ctx, ticket)
	if err == table.ErrNoSuchKey {
		return nil, xrefs.ErrDecorationsNotFound
	} else if err != nil {
		return nil, canonicalError(err, "file decorations", ticket)
	}

	if decor.File == nil {
		if len(decor.Diagnostic) == 0 {
			log.Printf("Error: FileDecorations.file is missing without related diagnostics: %q", req.Location.Ticket)
			return nil, xrefs.ErrDecorationsNotFound
		}

		// FileDecorations may be saved without a File if the file does not exist in
		// the index but related diagnostics do exist.  If diagnostics were
		// requested, we may return them successfully, but otherwise, an error
		// indicating a missing file is returned.
		if req.Diagnostics {
			return &xpb.DecorationsReply{
				Location:   req.Location,
				Diagnostic: decor.Diagnostic,
			}, nil
		}
		return nil, xrefs.ErrDecorationsNotFound
	}

	text := decor.File.Text
	if len(req.DirtyBuffer) > 0 {
		text = req.DirtyBuffer
	}
	norm := span.NewNormalizer(text)

	loc, err := norm.Location(req.GetLocation())
	if err != nil {
		return nil, err
	}

	reply := &xpb.DecorationsReply{Location: loc}

	if req.SourceText {
		reply.Encoding = decor.File.Encoding
		if loc.Kind == xpb.Location_FILE {
			reply.SourceText = text
		} else {
			reply.SourceText = text[loc.Span.Start.ByteOffset:loc.Span.End.ByteOffset]
		}
	}

	var patcher *span.Patcher
	if len(req.DirtyBuffer) > 0 {
		patcher = span.NewPatcher(decor.File.Text, req.DirtyBuffer)
	}

	// The span with which to constrain the set of returned anchor references.
	var startBoundary, endBoundary int32
	spanKind := req.SpanKind
	if loc.Kind == xpb.Location_FILE {
		startBoundary = 0
		endBoundary = int32(len(text))
		spanKind = xpb.DecorationsRequest_WITHIN_SPAN
	} else {
		startBoundary = loc.Span.Start.ByteOffset
		endBoundary = loc.Span.End.ByteOffset
	}

	if req.References {
		patterns := xrefs.ConvertFilters(req.Filter)
		buildConfigs := stringset.New(req.BuildConfig...)

		reply.Reference = make([]*xpb.DecorationsReply_Reference, 0, len(decor.Decoration))
		reply.Nodes = make(map[string]*cpb.NodeInfo, len(decor.Target))

		// Reference.TargetTicket -> NodeInfo (superset of reply.Nodes)
		nodes := make(map[string]*cpb.NodeInfo, len(decor.Target))
		if len(patterns) > 0 {
			for _, n := range decor.Target {
				if info := nodeToInfo(patterns, n); info != nil {
					nodes[n.Ticket] = info
				}
			}
		}
		tracePrintf(ctx, "Potential target nodes: %d", len(nodes))

		// All known definition locations (Anchor.Ticket -> Anchor)
		defs := make(map[string]*xpb.Anchor, len(decor.TargetDefinitions))
		for _, def := range decor.TargetDefinitions {
			defs[def.Ticket] = a2a(def, false).Anchor
		}
		if req.TargetDefinitions {
			reply.DefinitionLocations = make(map[string]*xpb.Anchor, len(decor.TargetDefinitions))
		}
		tracePrintf(ctx, "Potential target defs: %d", len(defs))

		bindings := stringset.New()

		for _, d := range decor.Decoration {
			// Filter decorations by requested build configs.
			if len(buildConfigs) != 0 && !buildConfigs.Contains(d.Anchor.BuildConfiguration) {
				continue
			}

			start, end, exists := patcher.Patch(d.Anchor.StartOffset, d.Anchor.EndOffset)
			// Filter non-existent anchor.  Anchors can no longer exist if we were
			// given a dirty buffer and the anchor was inside a changed region.
			if !exists || !span.InBounds(spanKind, start, end, startBoundary, endBoundary) {
				continue
			}

			d.Anchor.StartOffset = start
			d.Anchor.EndOffset = end

			r := decorationToReference(norm, d)
			if req.TargetDefinitions {
				if def, ok := defs[d.TargetDefinition]; ok {
					reply.DefinitionLocations[d.TargetDefinition] = def
				}
			} else {
				r.TargetDefinition = ""
			}

			if !req.SemanticScopes {
				r.SemanticScope = ""
			}

			if req.ExtendsOverrides && (r.Kind == edges.Defines || r.Kind == edges.DefinesBinding) {
				bindings.Add(r.TargetTicket)
			}

			reply.Reference = append(reply.Reference, r)

			if n := nodes[r.TargetTicket]; n != nil {
				reply.Nodes[r.TargetTicket] = n
			}
		}
		tracePrintf(ctx, "References: %d", len(reply.Reference))

		if len(decor.TargetOverride) > 0 {
			// Read overrides from serving data
			reply.ExtendsOverrides = make(map[string]*xpb.DecorationsReply_Overrides, len(bindings))

			for _, o := range decor.TargetOverride {
				if bindings.Contains(o.Overriding) {
					def := defs[o.OverriddenDefinition]
					if def != nil && len(buildConfigs) != 0 && !buildConfigs.Contains(def.BuildConfig) {
						// Skip override with undesirable build configuration.
						continue
					}

					os, ok := reply.ExtendsOverrides[o.Overriding]
					if !ok {
						os = &xpb.DecorationsReply_Overrides{}
						reply.ExtendsOverrides[o.Overriding] = os
					}

					ov := &xpb.DecorationsReply_Override{
						Target:       o.Overridden,
						Kind:         xpb.DecorationsReply_Override_Kind(o.Kind),
						MarkedSource: o.MarkedSource,
					}
					os.Override = append(os.Override, ov)

					if n := nodes[o.Overridden]; n != nil {
						reply.Nodes[o.Overridden] = n
					}
					if req.TargetDefinitions && def != nil {
						ov.TargetDefinition = o.OverriddenDefinition
						reply.DefinitionLocations[o.OverriddenDefinition] = def
					}
				}
			}
			tracePrintf(ctx, "ExtendsOverrides: %d", len(reply.ExtendsOverrides))
		}
		tracePrintf(ctx, "DefinitionLocations: %d", len(reply.DefinitionLocations))
	}

	if req.Diagnostics {
		for _, diag := range decor.Diagnostic {
			if diag.Span == nil {
				reply.Diagnostic = append(reply.Diagnostic, diag)
			} else {
				start, end, exists := patcher.PatchSpan(diag.Span)
				// Filter non-existent (or out-of-bounds) diagnostic.  Diagnostics can
				// no longer exist if we were given a dirty buffer and the diagnostic
				// was inside a changed region.
				if !exists || !span.InBounds(spanKind, start, end, startBoundary, endBoundary) {
					continue
				}

				diag.Span = norm.SpanOffsets(start, end)
				reply.Diagnostic = append(reply.Diagnostic, diag)
			}
		}
		tracePrintf(ctx, "Diagnostics: %d", len(reply.Diagnostic))
	}

	if req.Snippets == xpb.SnippetsKind_NONE {
		for _, anchor := range reply.DefinitionLocations {
			clearSnippet(anchor)
		}
	}

	return reply, nil
}

func decorationToReference(norm *span.Normalizer, d *srvpb.FileDecorations_Decoration) *xpb.DecorationsReply_Reference {
	span := norm.SpanOffsets(d.Anchor.StartOffset, d.Anchor.EndOffset)
	return &xpb.DecorationsReply_Reference{
		TargetTicket:     d.Target,
		Kind:             d.Kind,
		Span:             span,
		TargetDefinition: d.TargetDefinition,
		BuildConfig:      d.Anchor.BuildConfiguration,
		SemanticScope:    d.SemanticScope,
	}
}

// CrossReferences implements part of the xrefs.Service interface.
func (t *Table) CrossReferences(ctx context.Context, req *xpb.CrossReferencesRequest) (*xpb.CrossReferencesReply, error) {
	tickets, err := xrefs.FixTickets(req.Ticket)
	if err != nil {
		return nil, err
	}

	stats := refStats{
		max: int(req.PageSize),
	}
	if stats.max < 0 {
		return nil, status.Errorf(codes.InvalidArgument, "invalid page_size: %d", req.PageSize)
	} else if stats.max == 0 {
		stats.max = defaultPageSize
	} else if stats.max > maxPageSize {
		stats.max = maxPageSize
	}

	var pageToken ipb.PageToken
	if req.PageToken != "" {
		rec, err := base64.StdEncoding.DecodeString(req.PageToken)
		if err != nil {
			return nil, status.Errorf(codes.InvalidArgument, "invalid page_token: %q", req.PageToken)
		}
		rec, err = snappy.Decode(nil, rec)
		if err != nil {
			return nil, status.Errorf(codes.InvalidArgument, "invalid page_token: %q", req.PageToken)
		}
		if err := proto.Unmarshal(rec, &pageToken); err != nil {
			return nil, status.Errorf(codes.InvalidArgument, "invalid page_token: %q", req.PageToken)
		}
		for _, index := range pageToken.Indices {
			if index < 0 {
				return nil, status.Errorf(codes.InvalidArgument, "invalid page_token: %q", req.PageToken)
			}
		}
	}
	initialSkip := int(pageToken.Indices["skip"])
	stats.skip = initialSkip

	reply := &xpb.CrossReferencesReply{
		CrossReferences: make(map[string]*xpb.CrossReferencesReply_CrossReferenceSet, len(req.Ticket)),
		Nodes:           make(map[string]*cpb.NodeInfo, len(req.Ticket)),

		Total: &xpb.CrossReferencesReply_Total{},
	}
	if len(req.Filter) > 0 {
		reply.Total.RelatedNodesByRelation = make(map[string]int64)
	}
	if req.NodeDefinitions {
		reply.DefinitionLocations = make(map[string]*xpb.Anchor)
	}

	buildConfigs := stringset.New(req.BuildConfig...)
	patterns := xrefs.ConvertFilters(req.Filter)

	nextPageToken := &ipb.PageToken{
		SubTokens: make(map[string]string),
		Indices:   make(map[string]int32),
	}

	mergeInto := make(map[string]string)
	for _, ticket := range tickets {
		mergeInto[ticket] = ticket
	}

	relatedKinds := stringset.New(req.RelatedNodeKind...)

	wantMoreCrossRefs := (req.DefinitionKind != xpb.CrossReferencesRequest_NO_DEFINITIONS ||
		req.DeclarationKind != xpb.CrossReferencesRequest_NO_DECLARATIONS ||
		req.ReferenceKind != xpb.CrossReferencesRequest_NO_REFERENCES ||
		req.CallerKind != xpb.CrossReferencesRequest_NO_CALLERS ||
		len(req.Filter) > 0)

	var foundCrossRefs bool
	for i := 0; i < len(tickets); i++ {
		// TODO(schroederc): change default behavior to APPROXIMATE rather than PRECISE totals
		if req.TotalsQuality == xpb.CrossReferencesRequest_APPROXIMATE_TOTALS && stats.done() {
			log.Printf("WARNING: stopping CrossReferences index reads after %d/%d tickets", i, len(tickets))
			break
		}

		ticket := tickets[i]
		cr, err := t.crossReferences(ctx, ticket)
		if err == table.ErrNoSuchKey {
			continue
		} else if err != nil {
			return nil, canonicalError(err, "cross-references", ticket)
		}
		foundCrossRefs = true

		// If this node is to be merged into another, we will use that node's ticket
		// for all further book-keeping purposes.
		ticket = mergeInto[ticket]

		// We may have partially completed the xrefs set due merge nodes.
		crs := reply.CrossReferences[ticket]
		if crs == nil {
			crs = &xpb.CrossReferencesReply_CrossReferenceSet{
				Ticket: ticket,
			}

			// If visiting a non-merge node and facts are requested, add them to the result.
			if ticket == cr.SourceTicket && len(patterns) > 0 && cr.SourceNode != nil {
				if _, ok := reply.Nodes[ticket]; !ok {
					if info := nodeToInfo(patterns, cr.SourceNode); info != nil {
						reply.Nodes[ticket] = info
					}
				}
			}
		}
		if crs.MarkedSource == nil {
			crs.MarkedSource = cr.MarkedSource
		}

		if *mergeCrossReferences {
			// Add any additional merge nodes to the set of table lookups
			for _, mergeNode := range cr.MergeWith {
				tickets = addMergeNode(mergeInto, tickets, ticket, mergeNode)
			}
		}

		// Read the set of indirection edge kinds for the given node kind.
		nodeKind := nodeKind(cr.SourceNode)
		indirections := experimentalCrossReferenceIndirectionKinds[nodeKind].
			Union(experimentalCrossReferenceIndirectionKinds["*"])

		for _, grp := range cr.Group {
			// Filter anchor groups based on requested build configs
			if len(buildConfigs) != 0 && !buildConfigs.Contains(grp.BuildConfig) && !xrefs.IsRelatedNodeKind(relatedKinds, grp.Kind) {
				continue
			}

			switch {
			case xrefs.IsDefKind(req.DefinitionKind, grp.Kind, cr.Incomplete):
				reply.Total.Definitions += int64(len(grp.Anchor))
				if wantMoreCrossRefs {
					stats.addAnchors(&crs.Definition, grp, req.AnchorText)
				}
			case xrefs.IsDeclKind(req.DeclarationKind, grp.Kind, cr.Incomplete):
				reply.Total.Declarations += int64(len(grp.Anchor))
				if wantMoreCrossRefs {
					stats.addAnchors(&crs.Declaration, grp, req.AnchorText)
				}
			case xrefs.IsRefKind(req.ReferenceKind, grp.Kind):
				reply.Total.References += int64(len(grp.Anchor))
				if wantMoreCrossRefs {
					stats.addAnchors(&crs.Reference, grp, req.AnchorText)
				}
			case len(grp.RelatedNode) > 0:
				// If requested, add related nodes to merge node set.
				if indirections.Contains(grp.Kind) {
					for _, rn := range grp.RelatedNode {
						tickets = addMergeNode(mergeInto, tickets, ticket, rn.Node.GetTicket())
					}
				}

				if len(req.Filter) > 0 && xrefs.IsRelatedNodeKind(relatedKinds, grp.Kind) {
					reply.Total.RelatedNodesByRelation[grp.Kind] += int64(len(grp.RelatedNode))
					if wantMoreCrossRefs {
						stats.addRelatedNodes(reply, crs, grp, patterns)
					}
				}
			case xrefs.IsCallerKind(req.CallerKind, grp.Kind):
				reply.Total.Callers += int64(len(grp.Caller))
				if wantMoreCrossRefs {
					stats.addCallers(crs, grp)
				}
			}
		}

		for _, idx := range cr.PageIndex {
			// Filter anchor pages based on requested build configs
			if len(buildConfigs) != 0 && !buildConfigs.Contains(idx.BuildConfig) && !xrefs.IsRelatedNodeKind(relatedKinds, idx.Kind) {
				continue
			}

			switch {
			case xrefs.IsDefKind(req.DefinitionKind, idx.Kind, cr.Incomplete):
				reply.Total.Definitions += int64(idx.Count)
				if wantMoreCrossRefs && !stats.skipPage(idx) {
					p, err := t.crossReferencesPage(ctx, idx.PageKey)
					if err != nil {
						return nil, fmt.Errorf("internal error: error retrieving cross-references page: %v", idx.PageKey)
					}
					stats.addAnchors(&crs.Definition, p.Group, req.AnchorText)
				}
			case xrefs.IsDeclKind(req.DeclarationKind, idx.Kind, cr.Incomplete):
				reply.Total.Declarations += int64(idx.Count)
				if wantMoreCrossRefs && !stats.skipPage(idx) {
					p, err := t.crossReferencesPage(ctx, idx.PageKey)
					if err != nil {
						return nil, fmt.Errorf("internal error: error retrieving cross-references page: %v", idx.PageKey)
					}
					stats.addAnchors(&crs.Declaration, p.Group, req.AnchorText)
				}
			case xrefs.IsRefKind(req.ReferenceKind, idx.Kind):
				reply.Total.References += int64(idx.Count)
				if wantMoreCrossRefs && !stats.skipPage(idx) {
					p, err := t.crossReferencesPage(ctx, idx.PageKey)
					if err != nil {
						return nil, fmt.Errorf("internal error: error retrieving cross-references page: %v", idx.PageKey)
					}
					stats.addAnchors(&crs.Reference, p.Group, req.AnchorText)
				}
			case xrefs.IsRelatedNodeKind(nil, idx.Kind):
				var p *srvpb.PagedCrossReferences_Page

				// If requested, add related nodes to merge node set.
				if indirections.Contains(idx.Kind) {
					p, err = t.crossReferencesPage(ctx, idx.PageKey)
					if err != nil {
						return nil, fmt.Errorf("internal error: error retrieving cross-references page: %v", idx.PageKey)
					}

					for _, rn := range p.Group.RelatedNode {
						tickets = addMergeNode(mergeInto, tickets, ticket, rn.Node.GetTicket())
					}
				}

				if len(req.Filter) > 0 && xrefs.IsRelatedNodeKind(relatedKinds, idx.Kind) {
					reply.Total.RelatedNodesByRelation[idx.Kind] += int64(idx.Count)
					if wantMoreCrossRefs && !stats.skipPage(idx) {
						if p == nil {
							p, err = t.crossReferencesPage(ctx, idx.PageKey)
							if err != nil {
								return nil, fmt.Errorf("internal error: error retrieving cross-references page: %v", idx.PageKey)
							}
						}
						stats.addRelatedNodes(reply, crs, p.Group, patterns)
					}
				}
			case xrefs.IsCallerKind(req.CallerKind, idx.Kind):
				reply.Total.Callers += int64(idx.Count)
				if wantMoreCrossRefs && !stats.skipPage(idx) {
					p, err := t.crossReferencesPage(ctx, idx.PageKey)
					if err != nil {
						return nil, fmt.Errorf("internal error: error retrieving cross-references page: %v", idx.PageKey)
					}
					stats.addCallers(crs, p.Group)
				}
			}
		}

		if len(crs.Declaration) > 0 || len(crs.Definition) > 0 || len(crs.Reference) > 0 || len(crs.Caller) > 0 || len(crs.RelatedNode) > 0 {
			reply.CrossReferences[crs.Ticket] = crs
			tracePrintf(ctx, "CrossReferenceSet: %s", crs.Ticket)
		}
	}
	if !foundCrossRefs {
		// Short-circuit return; skip any slow requests.
		return &xpb.CrossReferencesReply{}, nil
	}

	if initialSkip+stats.total != sumTotalCrossRefs(reply.Total) && stats.total != 0 {
		nextPageToken.Indices["skip"] = int32(initialSkip + stats.total)
	}

	if _, skip := nextPageToken.Indices["skip"]; skip {
		rec, err := proto.Marshal(nextPageToken)
		if err != nil {
			return nil, fmt.Errorf("internal error: error marshalling page token: %v", err)
		}
		reply.NextPageToken = base64.StdEncoding.EncodeToString(snappy.Encode(nil, rec))
	}

	if req.Snippets == xpb.SnippetsKind_NONE {
		for _, crs := range reply.CrossReferences {
			for _, def := range crs.Definition {
				clearRelatedSnippets(def)
			}
			for _, dec := range crs.Declaration {
				clearRelatedSnippets(dec)
			}
			for _, ref := range crs.Reference {
				clearRelatedSnippets(ref)
			}
			for _, ca := range crs.Caller {
				clearRelatedSnippets(ca)
			}
		}
		for _, def := range reply.DefinitionLocations {
			clearSnippet(def)
		}
	}

	return reply, nil
}

func addMergeNode(mergeMap map[string]string, allTickets []string, rootNode, mergeNode string) []string {
	if prevMerge, ok := mergeMap[mergeNode]; ok {
		if prevMerge != rootNode {
			log.Printf("WARNING: node %q already previously merged with %q", mergeNode, prevMerge)
		}
		return allTickets
	}
	allTickets = append(allTickets, mergeNode)
	mergeMap[mergeNode] = rootNode
	return allTickets
}

func nodeKind(n *srvpb.Node) string {
	for _, f := range n.Fact {
		if f.Name == facts.NodeKind {
			return string(f.Value)
		}
	}
	return ""
}

func sumTotalCrossRefs(ts *xpb.CrossReferencesReply_Total) int {
	var relatedNodes int
	for _, cnt := range ts.RelatedNodesByRelation {
		relatedNodes += int(cnt)
	}
	return int(ts.Callers) + int(ts.Definitions) + int(ts.Declarations) + int(ts.References) + int(ts.Documentation) + relatedNodes
}

type refStats struct {
	// number of refs:
	//   to skip (returned on previous pages)
	//   max to return (the page size)
	//   total (count of refs so far read for current page)
	skip, total, max int
}

func (s *refStats) done() bool { return s.total == s.max }

func (s *refStats) skipPage(idx *srvpb.PagedCrossReferences_PageIndex) bool {
	if s.skip > int(idx.Count) {
		s.skip -= int(idx.Count)
		return true
	}
	return s.total >= s.max
}

func (s *refStats) addCallers(crs *xpb.CrossReferencesReply_CrossReferenceSet, grp *srvpb.PagedCrossReferences_Group) bool {
	cs := grp.Caller

	if s.done() {
		// We've already hit our cap; return true that we're done.
		return true
	} else if s.skip > len(cs) {
		// We can skip this entire group.
		s.skip -= len(cs)
		return false
	} else if s.skip > 0 {
		// Skip part of the group, and put the rest in the reply.
		cs = cs[s.skip:]
		s.skip = 0
	}

	if s.total+len(cs) > s.max {
		cs = cs[:(s.max - s.total)]
	}
	s.total += len(cs)
	for _, c := range cs {
		ra := &xpb.CrossReferencesReply_RelatedAnchor{
			Anchor: a2a(c.Caller, false).Anchor,
			Ticket: c.SemanticCaller,
			Site:   make([]*xpb.Anchor, 0, len(c.Callsite)),
		}
		ra.MarkedSource = c.MarkedSource
		for _, site := range c.Callsite {
			ra.Site = append(ra.Site, a2a(site, false).Anchor)
		}
		crs.Caller = append(crs.Caller, ra)
	}
	return s.done() // return whether we've hit our cap
}

func (s *refStats) addRelatedNodes(reply *xpb.CrossReferencesReply, crs *xpb.CrossReferencesReply_CrossReferenceSet, grp *srvpb.PagedCrossReferences_Group, patterns []*regexp.Regexp) bool {
	ns := grp.RelatedNode
	nodes := reply.Nodes
	defs := reply.DefinitionLocations

	if s.total == s.max {
		// We've already hit our cap; return true that we're done.
		return true
	} else if s.skip > len(ns) {
		// We can skip this entire group.
		s.skip -= len(ns)
		return false
	} else if s.skip > 0 {
		// Skip part of the group, and put the rest in the reply.
		ns = ns[s.skip:]
		s.skip = 0
	}

	if s.total+len(ns) > s.max {
		ns = ns[:(s.max - s.total)]
	}
	s.total += len(ns)
	for _, rn := range ns {
		if _, ok := nodes[rn.Node.Ticket]; !ok {
			if info := nodeToInfo(patterns, rn.Node); info != nil {
				nodes[rn.Node.Ticket] = info
				if defs != nil && rn.Node.DefinitionLocation != nil {
					nodes[rn.Node.Ticket].Definition = rn.Node.DefinitionLocation.Ticket
					defs[rn.Node.DefinitionLocation.Ticket] = a2a(rn.Node.DefinitionLocation, false).Anchor
				}
			}
		}
		crs.RelatedNode = append(crs.RelatedNode, &xpb.CrossReferencesReply_RelatedNode{
			RelationKind: grp.Kind,
			Ticket:       rn.Node.Ticket,
			Ordinal:      rn.Ordinal,
		})
	}
	return s.total == s.max // return whether we've hit our cap
}

func (s *refStats) addAnchors(to *[]*xpb.CrossReferencesReply_RelatedAnchor, grp *srvpb.PagedCrossReferences_Group, anchorText bool) bool {
	kind := edges.Canonical(grp.Kind)
	as := grp.Anchor

	if s.total == s.max {
		return true
	} else if s.skip > len(as) {
		s.skip -= len(as)
		return false
	} else if s.skip > 0 {
		as = as[s.skip:]
		s.skip = 0
	}

	if s.total+len(as) > s.max {
		as = as[:(s.max - s.total)]
	}
	s.total += len(as)
	for _, a := range as {
		ra := a2a(a, anchorText)
		ra.Anchor.Kind = kind
		*to = append(*to, ra)
	}
	return s.total == s.max
}

func a2a(a *srvpb.ExpandedAnchor, anchorText bool) *xpb.CrossReferencesReply_RelatedAnchor {
	var text string
	if anchorText {
		text = a.Text
	}
	parent, err := tickets.AnchorFile(a.Ticket)
	if err != nil {
		log.Printf("Error parsing anchor ticket: %v", err)
	}
	return &xpb.CrossReferencesReply_RelatedAnchor{Anchor: &xpb.Anchor{
		Ticket:      a.Ticket,
		Kind:        edges.Canonical(a.Kind),
		Parent:      parent,
		Text:        text,
		Span:        a.Span,
		Snippet:     a.Snippet,
		SnippetSpan: a.SnippetSpan,
		BuildConfig: a.BuildConfiguration,
	}}
}

func d2d(d *srvpb.Document, patterns []*regexp.Regexp, nodes map[string]*cpb.NodeInfo, defs map[string]*xpb.Anchor) *xpb.DocumentationReply_Document {
	for _, node := range d.Node {
		if _, ok := nodes[node.Ticket]; ok {
			continue
		}

		if n := nodeToInfo(patterns, node); n != nil {
			nodes[node.Ticket] = n
			if def := node.DefinitionLocation; def != nil {
				n.Definition = def.Ticket
				if _, ok := defs[def.Ticket]; !ok {
					defs[def.Ticket] = a2a(def, false).Anchor
				}
			}
		}
	}

	return &xpb.DocumentationReply_Document{
		Ticket: d.Ticket,
		Text: &xpb.Printable{
			RawText: d.RawText,
			Link:    d.Link,
		},
		MarkedSource: d.MarkedSource,
	}
}

func (t *Table) lookupDocument(ctx context.Context, ticket string) (*srvpb.Document, error) {
	d, err := t.documentation(ctx, ticket)
	if err != nil {
		return nil, err
	}
	tracePrintf(ctx, "Document: %s", ticket)

	// If DocumentedBy is provided, replace document with another lookup.
	if d.DocumentedBy != "" {
		doc, err := t.documentation(ctx, d.DocumentedBy)
		if err != nil {
			log.Printf("Error looking up subsuming documentation for {%+v}: %v", d, err)
			return nil, err
		}

		// Ensure the subsuming documentation has the correct ticket and node.
		doc.Ticket = ticket
		for _, n := range d.Node {
			if n.Ticket == ticket {
				doc.Node = append(doc.Node, n)
				break
			}
		}

		tracePrintf(ctx, "DocumentedBy: %s", d.DocumentedBy)
		d = doc
	}
	return d, nil
}

// Documentation implements part of the xrefs Service interface.
func (t *Table) Documentation(ctx context.Context, req *xpb.DocumentationRequest) (*xpb.DocumentationReply, error) {
	tickets, err := xrefs.FixTickets(req.Ticket)
	if err != nil {
		return nil, err
	}

	reply := &xpb.DocumentationReply{
		Nodes:               make(map[string]*cpb.NodeInfo, len(tickets)),
		DefinitionLocations: make(map[string]*xpb.Anchor, len(tickets)),
	}
	patterns := xrefs.ConvertFilters(req.Filter)
	if len(patterns) == 0 {
		// Match all facts if given no filters
		patterns = xrefs.ConvertFilters([]string{"**"})
	}

	for _, ticket := range tickets {
		d, err := t.lookupDocument(ctx, ticket)
		if err == table.ErrNoSuchKey {
			continue
		} else if err != nil {
			return nil, canonicalError(err, "documentation", ticket)
		}

		doc := d2d(d, patterns, reply.Nodes, reply.DefinitionLocations)
		if req.IncludeChildren {
			for _, child := range d.ChildTicket {
				// TODO(schroederc): store children with root of documentation tree
				cd, err := t.lookupDocument(ctx, child)
				if err == table.ErrNoSuchKey {
					continue
				} else if err != nil {
					return nil, canonicalError(err, "documentation child", ticket)
				}

				doc.Children = append(doc.Children, d2d(cd, patterns, reply.Nodes, reply.DefinitionLocations))
			}
			tracePrintf(ctx, "Children: %d", len(d.ChildTicket))
		}

		reply.Document = append(reply.Document, doc)
	}
	tracePrintf(ctx, "Documents: %d (nodes: %d) (defs: %d", len(reply.Document), len(reply.Nodes), len(reply.DefinitionLocations))

	return reply, nil
}

func clearRelatedSnippets(ra *xpb.CrossReferencesReply_RelatedAnchor) {
	clearSnippet(ra.Anchor)
	for _, site := range ra.Site {
		clearSnippet(site)
	}
}

func clearSnippet(anchor *xpb.Anchor) {
	anchor.Snippet = ""
	anchor.SnippetSpan = nil
}

func tracePrintf(ctx context.Context, msg string, args ...interface{}) {
	if t, ok := trace.FromContext(ctx); ok {
		t.LazyPrintf(msg, args...)
	}
}

// Wrap known error types with corresponding rpc status errors.
// If no sensible parsing can be found, just return fmt.Errorf with some hints
// about the calling code and ticket.
// Follows util::error::Code from
// http://google.github.io/google-api-cpp-client/latest/doxygen/namespacegoogleapis_1_1util_1_1error.html
func canonicalError(err error, caller string, ticket string) error {
	switch code := status.Code(err); code {
	case codes.Canceled:
		return xrefs.ErrCanceled
	case codes.DeadlineExceeded:
		return xrefs.ErrDeadlineExceeded
	default:
		st := err.Error()
		if strings.Contains(st, "RPC::CANCELLED") || strings.Contains(st, "context canceled") {
			return xrefs.ErrCanceled
		}
		if strings.Contains(st, "RPC::DEADLINE_EXCEEDED") || strings.Contains(st, "context deadline exceeded") {
			return xrefs.ErrDeadlineExceeded
		}
		return status.Error(code, st)
	}
}
