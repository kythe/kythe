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

// Package xrefs provides a high-performance serving table implementation of the
// xrefs.Service.
//
// Table format:
//   edgeSets:<ticket>      -> srvpb.PagedEdgeSet
//   edgePages:<page_key>   -> srvpb.EdgePage
//   decor:<ticket>         -> srvpb.FileDecorations
//   xrefs:<ticket>         -> srvpb.PagedCrossReferences
//   xrefPages:<page_key>   -> srvpb.PagedCrossReferences_Page
package xrefs

import (
	"encoding/base64"
	"errors"
	"fmt"
	"log"
	"regexp"
	"strings"

	"kythe.io/kythe/go/services/xrefs"
	"kythe.io/kythe/go/storage/table"
	"kythe.io/kythe/go/util/kytheuri"
	"kythe.io/kythe/go/util/schema"
	"kythe.io/kythe/go/util/stringset"

	cpb "kythe.io/kythe/proto/common_proto"
	ipb "kythe.io/kythe/proto/internal_proto"
	srvpb "kythe.io/kythe/proto/serving_proto"
	xpb "kythe.io/kythe/proto/xref_proto"

	"github.com/golang/protobuf/proto"
	"golang.org/x/net/context"
)

type edgeSetResult struct {
	PagedEdgeSet *srvpb.PagedEdgeSet

	Err error
}

type staticLookupTables interface {
	pagedEdgeSets(ctx context.Context, tickets []string) (<-chan edgeSetResult, error)
	edgePage(ctx context.Context, key string) (*srvpb.EdgePage, error)
	fileDecorations(ctx context.Context, ticket string) (*srvpb.FileDecorations, error)
	crossReferences(ctx context.Context, ticket string) (*srvpb.PagedCrossReferences, error)
	crossReferencesPage(ctx context.Context, key string) (*srvpb.PagedCrossReferences_Page, error)
}

// SplitTable implements the xrefs Service interface using separate static
// lookup tables for each API component.
type SplitTable struct {
	// Edges is a table of srvpb.PagedEdgeSets keyed by their source tickets.
	Edges table.ProtoBatch

	// EdgePages is a table of srvpb.EdgePages keyed by their page keys.
	EdgePages table.Proto

	// Decorations is a table of srvpb.FileDecorations keyed by their source
	// location tickets.
	Decorations table.Proto

	// CrossReferences is a table of srvpb.PagedCrossReferences keyed by their
	// source node tickets.
	CrossReferences table.Proto

	// CrossReferencePages is a table of srvpb.PagedCrossReferences_Pages keyed by
	// their page keys.
	CrossReferencePages table.Proto
}

func lookupPagedEdgeSets(ctx context.Context, tbl table.ProtoBatch, keys [][]byte) (<-chan edgeSetResult, error) {
	rs, err := tbl.LookupBatch(ctx, keys, (*srvpb.PagedEdgeSet)(nil))
	if err != nil {
		return nil, err
	}
	ch := make(chan edgeSetResult)
	go func() {
		defer close(ch)
		for r := range rs {
			if r.Err == table.ErrNoSuchKey {
				log.Printf("Could not locate edges with key %q", r.Key)
				ch <- edgeSetResult{Err: r.Err}
				continue
			} else if r.Err != nil {
				ticket := strings.TrimPrefix(string(r.Key), edgeSetsTablePrefix)
				ch <- edgeSetResult{
					Err: fmt.Errorf("edges lookup error (ticket %q): %v", ticket, r.Err),
				}
				continue
			}

			ch <- edgeSetResult{PagedEdgeSet: r.Value.(*srvpb.PagedEdgeSet)}
		}
	}()
	return ch, nil
}

func toKeys(ss []string) [][]byte {
	keys := make([][]byte, len(ss), len(ss))
	for i, s := range ss {
		keys[i] = []byte(s)
	}
	return keys
}

func (s *SplitTable) pagedEdgeSets(ctx context.Context, tickets []string) (<-chan edgeSetResult, error) {
	return lookupPagedEdgeSets(ctx, s.Edges, toKeys(tickets))
}
func (s *SplitTable) edgePage(ctx context.Context, key string) (*srvpb.EdgePage, error) {
	var ep srvpb.EdgePage
	return &ep, s.EdgePages.Lookup(ctx, []byte(key), &ep)
}
func (s *SplitTable) fileDecorations(ctx context.Context, ticket string) (*srvpb.FileDecorations, error) {
	var fd srvpb.FileDecorations
	return &fd, s.Decorations.Lookup(ctx, []byte(ticket), &fd)
}
func (s *SplitTable) crossReferences(ctx context.Context, ticket string) (*srvpb.PagedCrossReferences, error) {
	var cr srvpb.PagedCrossReferences
	return &cr, s.CrossReferences.Lookup(ctx, []byte(ticket), &cr)
}
func (s *SplitTable) crossReferencesPage(ctx context.Context, key string) (*srvpb.PagedCrossReferences_Page, error) {
	var p srvpb.PagedCrossReferences_Page
	return &p, s.CrossReferencePages.Lookup(ctx, []byte(key), &p)
}

// Key prefixes for the combinedTable implementation.
const (
	crossRefTablePrefix     = "xrefs:"
	crossRefPageTablePrefix = "xrefPages:"
	decorTablePrefix        = "decor:"
	edgeSetsTablePrefix     = "edgeSets:"
	edgePagesTablePrefix    = "edgePages:"
)

type combinedTable struct{ table.ProtoBatch }

func (c *combinedTable) pagedEdgeSets(ctx context.Context, tickets []string) (<-chan edgeSetResult, error) {
	keys := make([][]byte, len(tickets), len(tickets))
	for i, ticket := range tickets {
		keys[i] = EdgeSetKey(ticket)
	}
	return lookupPagedEdgeSets(ctx, c, keys)
}
func (c *combinedTable) edgePage(ctx context.Context, key string) (*srvpb.EdgePage, error) {
	var ep srvpb.EdgePage
	return &ep, c.Lookup(ctx, EdgePageKey(key), &ep)
}
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

// NewSplitTable returns an xrefs.Service based on the given serving tables for
// each API component.
func NewSplitTable(c *SplitTable) xrefs.Service { return &tableImpl{c} }

// NewCombinedTable returns an xrefs.Service for the given combined xrefs
// serving table.  The table's keys are expected to be constructed using only
// the EdgeSetKey, EdgePageKey, and DecorationsKey functions.
func NewCombinedTable(t table.ProtoBatch) xrefs.Service { return &tableImpl{&combinedTable{t}} }

// EdgeSetKey returns the edgeset CombinedTable key for the given source ticket.
func EdgeSetKey(ticket string) []byte {
	return []byte(edgeSetsTablePrefix + ticket)
}

// EdgePageKey returns the edgepage CombinedTable key for the given key.
func EdgePageKey(key string) []byte {
	return []byte(edgePagesTablePrefix + key)
}

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

// tableImpl implements the xrefs Service interface using static lookup tables.
type tableImpl struct{ staticLookupTables }

// Nodes implements part of the xrefs Service interface.
func (t *tableImpl) Nodes(ctx context.Context, req *xpb.NodesRequest) (*xpb.NodesReply, error) {
	tickets, err := xrefs.FixTickets(req.Ticket)
	if err != nil {
		return nil, err
	}

	rs, err := t.pagedEdgeSets(ctx, tickets)
	if err != nil {
		return nil, err
	}
	defer func() {
		// drain channel in case of errors
		for _ = range rs {
		}
	}()

	reply := &xpb.NodesReply{Nodes: make(map[string]*xpb.NodeInfo, len(req.Ticket))}
	patterns := xrefs.ConvertFilters(req.Filter)

	for r := range rs {
		if r.Err == table.ErrNoSuchKey {
			continue
		} else if r.Err != nil {
			return nil, r.Err
		}
		node := r.PagedEdgeSet.Source
		ni := &xpb.NodeInfo{Facts: make(map[string][]byte, len(node.Fact))}
		for _, f := range node.Fact {
			if len(patterns) == 0 || xrefs.MatchesAny(f.Name, patterns) {
				ni.Facts[f.Name] = f.Value
			}
		}
		if len(ni.Facts) > 0 {
			reply.Nodes[node.Ticket] = ni
		}
	}
	return reply, nil
}

const (
	defaultPageSize = 2048
	maxPageSize     = 10000
)

// Edges implements part of the xrefs Service interface.
func (t *tableImpl) Edges(ctx context.Context, req *xpb.EdgesRequest) (*xpb.EdgesReply, error) {
	tickets, err := xrefs.FixTickets(req.Ticket)
	if err != nil {
		return nil, err
	}

	allowedKinds := stringset.New(req.Kind...)
	return t.edges(ctx, edgesRequest{
		Tickets: tickets,
		Filters: req.Filter,
		Kinds: func(kind string) bool {
			return len(allowedKinds) == 0 || allowedKinds.Contains(kind)
		},

		PageSize:  int(req.PageSize),
		PageToken: req.PageToken,
	})
}

type edgesRequest struct {
	Tickets []string
	Filters []string
	Kinds   func(string) bool

	TotalOnly bool
	PageSize  int
	PageToken string
}

func (t *tableImpl) edges(ctx context.Context, req edgesRequest) (*xpb.EdgesReply, error) {
	stats := filterStats{
		max: int(req.PageSize),
	}
	if req.TotalOnly {
		stats.max = 0
	} else if stats.max < 0 {
		return nil, fmt.Errorf("invalid page_size: %d", req.PageSize)
	} else if stats.max == 0 {
		stats.max = defaultPageSize
	} else if stats.max > maxPageSize {
		stats.max = maxPageSize
	}

	if req.PageToken != "" {
		rec, err := base64.StdEncoding.DecodeString(req.PageToken)
		if err != nil {
			return nil, fmt.Errorf("invalid page_token: %q", req.PageToken)
		}
		var t ipb.PageToken
		if err := proto.Unmarshal(rec, &t); err != nil || t.Index < 0 {
			return nil, fmt.Errorf("invalid page_token: %q", req.PageToken)
		}
		stats.skip = int(t.Index)
	}
	pageToken := stats.skip

	nodeTickets := stringset.New()

	rs, err := t.pagedEdgeSets(ctx, req.Tickets)
	if err != nil {
		return nil, err
	}
	defer func() {
		// drain channel in case of errors or early return
		for _ = range rs {
		}
	}()

	patterns := xrefs.ConvertFilters(req.Filters)

	reply := &xpb.EdgesReply{
		EdgeSets: make(map[string]*xpb.EdgeSet),
		Nodes:    make(map[string]*xpb.NodeInfo),

		TotalEdgesByKind: make(map[string]int64),
	}
	for r := range rs {
		if r.Err == table.ErrNoSuchKey {
			continue
		} else if r.Err != nil {
			return nil, r.Err
		}
		pes := r.PagedEdgeSet
		countEdgeKinds(pes, req.Kinds, reply.TotalEdgesByKind)

		// Don't scan the EdgeSet_Groups if we're already at the specified page_size.
		if stats.total == stats.max {
			continue
		}

		groups := make(map[string]*xpb.EdgeSet_Group)
		for _, grp := range pes.Group {
			if req.Kinds == nil || req.Kinds(grp.Kind) {
				ng, ns := stats.filter(grp)
				if ng != nil {
					for _, n := range ns {
						if len(patterns) > 0 && !nodeTickets.Contains(n.Ticket) {
							nodeTickets.Add(n.Ticket)
							reply.Nodes[n.Ticket] = nodeToInfo(patterns, n)
						}
					}
					groups[grp.Kind] = ng
					if stats.total == stats.max {
						break
					}
				}
			}
		}

		// TODO(schroederc): ensure that pes.EdgeSet.Groups and pes.PageIndexes of
		// the same kind are grouped together in the EdgesReply

		if stats.total != stats.max {
			for _, idx := range pes.PageIndex {
				if req.Kinds == nil || req.Kinds(idx.EdgeKind) {
					if stats.skipPage(idx) {
						log.Printf("Skipping EdgePage: %s", idx.PageKey)
						continue
					}

					log.Printf("Retrieving EdgePage: %s", idx.PageKey)
					ep, err := t.edgePage(ctx, idx.PageKey)
					if err == table.ErrNoSuchKey {
						return nil, fmt.Errorf("internal error: missing edge page: %q", idx.PageKey)
					} else if err != nil {
						return nil, fmt.Errorf("edge page lookup error (page key: %q): %v", idx.PageKey, err)
					}

					ng, ns := stats.filter(ep.EdgesGroup)
					if ng != nil {
						for _, n := range ns {
							if len(patterns) > 0 && !nodeTickets.Contains(n.Ticket) {
								nodeTickets.Add(n.Ticket)
								reply.Nodes[n.Ticket] = nodeToInfo(patterns, n)
							}
						}
						groups[ep.EdgesGroup.Kind] = ng
						if stats.total == stats.max {
							break
						}
					}
				}
			}
		}

		if len(groups) > 0 {
			reply.EdgeSets[pes.Source.Ticket] = &xpb.EdgeSet{Groups: groups}

			if len(patterns) > 0 && !nodeTickets.Contains(pes.Source.Ticket) {
				nodeTickets.Add(pes.Source.Ticket)
				reply.Nodes[pes.Source.Ticket] = nodeToInfo(patterns, pes.Source)
			}
		}
	}
	totalEdgesPossible := int(sumEdgeKinds(reply.TotalEdgesByKind))
	if stats.total > stats.max {
		log.Panicf("totalEdges greater than maxEdges: %d > %d", stats.total, stats.max)
	} else if pageToken+stats.total > totalEdgesPossible && pageToken <= totalEdgesPossible {
		log.Panicf("pageToken+totalEdges greater than totalEdgesPossible: %d+%d > %d", pageToken, stats.total, totalEdgesPossible)
	}

	if pageToken+stats.total != totalEdgesPossible && stats.total != 0 {
		rec, err := proto.Marshal(&ipb.PageToken{Index: int32(pageToken + stats.total)})
		if err != nil {
			return nil, fmt.Errorf("internal error: error marshalling page token: %v", err)
		}
		reply.NextPageToken = base64.StdEncoding.EncodeToString(rec)
	}

	return reply, nil
}

func countEdgeKinds(pes *srvpb.PagedEdgeSet, kindFilter func(string) bool, totals map[string]int64) {
	for _, grp := range pes.Group {
		if kindFilter == nil || kindFilter(grp.Kind) {
			totals[grp.Kind] += int64(len(grp.Edge))
		}
	}
	for _, page := range pes.PageIndex {
		if kindFilter == nil || kindFilter(page.EdgeKind) {
			totals[page.EdgeKind] += int64(page.EdgeCount)
		}
	}
}

func sumEdgeKinds(totals map[string]int64) int64 {
	var sum int64
	for _, cnt := range totals {
		sum += cnt
	}
	return sum
}

type filterStats struct {
	skip, total, max int
}

func (s *filterStats) skipPage(idx *srvpb.PageIndex) bool {
	if int(idx.EdgeCount) <= s.skip {
		s.skip -= int(idx.EdgeCount)
		return true
	}
	return false
}

func (s *filterStats) filter(g *srvpb.EdgeGroup) (*xpb.EdgeSet_Group, []*srvpb.Node) {
	edges := g.Edge
	if len(edges) <= s.skip {
		s.skip -= len(edges)
		return nil, nil
	} else if s.skip > 0 {
		edges = edges[s.skip:]
		s.skip = 0
	}

	if len(edges) > s.max-s.total {
		edges = edges[:(s.max - s.total)]
	}

	s.total += len(edges)

	targets := make([]*srvpb.Node, len(edges))
	for i, e := range edges {
		targets[i] = e.Target
	}

	return &xpb.EdgeSet_Group{
		Edge: e2e(edges),
	}, targets
}

func e2e(es []*srvpb.EdgeGroup_Edge) []*xpb.EdgeSet_Group_Edge {
	edges := make([]*xpb.EdgeSet_Group_Edge, len(es))
	for i, e := range es {
		edges[i] = &xpb.EdgeSet_Group_Edge{
			TargetTicket: e.Target.Ticket,
			Ordinal:      e.Ordinal,
		}
	}
	return edges
}

func nodeToInfo(patterns []*regexp.Regexp, n *srvpb.Node) *xpb.NodeInfo {
	ni := &xpb.NodeInfo{Facts: make(map[string][]byte, len(n.Fact))}
	for _, f := range n.Fact {
		if xrefs.MatchesAny(f.Name, patterns) {
			ni.Facts[f.Name] = f.Value
		}
	}
	return ni
}

// Decorations implements part of the xrefs Service interface.
func (t *tableImpl) Decorations(ctx context.Context, req *xpb.DecorationsRequest) (*xpb.DecorationsReply, error) {
	if req.GetLocation() == nil || req.GetLocation().Ticket == "" {
		return nil, errors.New("missing location")
	}

	ticket, err := kytheuri.Fix(req.GetLocation().Ticket)
	if err != nil {
		return nil, fmt.Errorf("invalid ticket %q: %v", req.GetLocation().Ticket, err)
	}

	decor, err := t.fileDecorations(ctx, ticket)
	if err == table.ErrNoSuchKey {
		return nil, xrefs.ErrDecorationsNotFound
	} else if err != nil {
		return nil, fmt.Errorf("lookup error for file decorations %q: %v", ticket, err)
	}

	text := decor.File.Text
	if len(req.DirtyBuffer) > 0 {
		text = req.DirtyBuffer
	}
	norm := xrefs.NewNormalizer(text)

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
			reply.SourceText = text[loc.Start.ByteOffset:loc.End.ByteOffset]
		}
	}

	if req.References {
		patterns := xrefs.ConvertFilters(req.Filter)

		var patcher *xrefs.Patcher
		if len(req.DirtyBuffer) > 0 {
			patcher = xrefs.NewPatcher(decor.File.Text, req.DirtyBuffer)
		}

		// The span with which to constrain the set of returned anchor references.
		var startBoundary, endBoundary int32
		spanKind := req.SpanKind
		if loc.Kind == xpb.Location_FILE {
			startBoundary = 0
			endBoundary = int32(len(text))
			spanKind = xpb.DecorationsRequest_WITHIN_SPAN
		} else {
			startBoundary = loc.Start.ByteOffset
			endBoundary = loc.End.ByteOffset
		}

		reply.Reference = make([]*xpb.DecorationsReply_Reference, 0, len(decor.Decoration))
		reply.Nodes = make(map[string]*xpb.NodeInfo)

		seenTarget := stringset.New()

		// Reference.TargetTicket -> NodeInfo (superset of reply.Nodes)
		var nodes map[string]*xpb.NodeInfo
		if len(patterns) > 0 {
			nodes = make(map[string]*xpb.NodeInfo)
			for _, n := range decor.Target {
				nodes[n.Ticket] = nodeToInfo(patterns, n)
			}
		}

		// Reference.TargetTicket -> []Reference set
		var refs map[string][]*xpb.DecorationsReply_Reference
		// ExpandedAnchor.Ticket -> ExpandedAnchor
		var defs map[string]*srvpb.ExpandedAnchor
		if req.TargetDefinitions {
			refs = make(map[string][]*xpb.DecorationsReply_Reference)
			reply.DefinitionLocations = make(map[string]*xpb.Anchor)

			defs = make(map[string]*srvpb.ExpandedAnchor)
			for _, def := range decor.TargetDefinitions {
				defs[def.Ticket] = def
			}
		}

		for _, d := range decor.Decoration {
			start, end, exists := patcher.Patch(d.Anchor.StartOffset, d.Anchor.EndOffset)
			// Filter non-existent anchor.  Anchors can no longer exist if we were
			// given a dirty buffer and the anchor was inside a changed region.
			if exists {
				if xrefs.InSpanBounds(spanKind, start, end, startBoundary, endBoundary) {
					d.Anchor.StartOffset = start
					d.Anchor.EndOffset = end

					r := decorationToReference(norm, d)
					if req.TargetDefinitions {
						if def, ok := defs[d.TargetDefinition]; ok {
							reply.DefinitionLocations[d.TargetDefinition] = a2a(def, false).Anchor
						} else {
							refs[r.TargetTicket] = append(refs[r.TargetTicket], r)
						}
					} else {
						r.TargetDefinition = ""
					}

					reply.Reference = append(reply.Reference, r)

					if !seenTarget.Contains(r.TargetTicket) && nodes != nil {
						reply.Nodes[r.TargetTicket] = nodes[r.TargetTicket]
						seenTarget.Add(r.TargetTicket)
					}
				}
			}
		}

		// Only compute target definitions if the serving data doesn't contain any
		// TODO(schroederc): remove this once serving data is always populated
		if req.TargetDefinitions && len(defs) == 0 {
			targetTickets := make([]string, 0, len(refs))
			for ticket := range refs {
				targetTickets = append(targetTickets, ticket)
			}

			defs, err := xrefs.SlowDefinitions(t, ctx, targetTickets)
			if err != nil {
				return nil, fmt.Errorf("error retrieving target definitions: %v", err)
			}

			reply.DefinitionLocations = make(map[string]*xpb.Anchor, len(defs))
			for tgt, def := range defs {
				for _, ref := range refs[tgt] {
					if def.Ticket != ref.SourceTicket {
						ref.TargetDefinition = def.Ticket
						if _, ok := reply.DefinitionLocations[def.Ticket]; !ok {
							reply.DefinitionLocations[def.Ticket] = def
						}
					}
				}
			}
		}
	}

	return reply, nil
}

type span struct{ start, end int32 }

func decorationToReference(norm *xrefs.Normalizer, d *srvpb.FileDecorations_Decoration) *xpb.DecorationsReply_Reference {
	return &xpb.DecorationsReply_Reference{
		SourceTicket:     d.Anchor.Ticket,
		TargetTicket:     d.Target,
		Kind:             d.Kind,
		AnchorStart:      norm.ByteOffset(d.Anchor.StartOffset),
		AnchorEnd:        norm.ByteOffset(d.Anchor.EndOffset),
		TargetDefinition: d.TargetDefinition,
	}
}

// CrossReferences implements part of the xrefs.Service interface.
func (t *tableImpl) CrossReferences(ctx context.Context, req *xpb.CrossReferencesRequest) (*xpb.CrossReferencesReply, error) {
	tickets, err := xrefs.FixTickets(req.Ticket)
	if err != nil {
		return nil, err
	}

	stats := refStats{
		max: int(req.PageSize),
	}
	if stats.max < 0 {
		return nil, fmt.Errorf("invalid page_size: %d", req.PageSize)
	} else if stats.max == 0 {
		stats.max = defaultPageSize
	} else if stats.max > maxPageSize {
		stats.max = maxPageSize
	}

	var edgesPageToken string
	if req.PageToken != "" {
		rec, err := base64.StdEncoding.DecodeString(req.PageToken)
		if err != nil {
			return nil, fmt.Errorf("invalid page_token: %q", req.PageToken)
		}
		var t ipb.PageToken
		if err := proto.Unmarshal(rec, &t); err != nil || t.Index < 0 {
			return nil, fmt.Errorf("invalid page_token: %q", req.PageToken)
		}
		stats.skip = int(t.Index)
		edgesPageToken = t.SecondaryToken
	}
	pageToken := stats.skip

	reply := &xpb.CrossReferencesReply{
		CrossReferences: make(map[string]*xpb.CrossReferencesReply_CrossReferenceSet, len(req.Ticket)),
		Nodes:           make(map[string]*xpb.NodeInfo, len(req.Ticket)),

		Total: &xpb.CrossReferencesReply_Total{},
	}
	var nextToken *ipb.PageToken

	wantMoreCrossRefs := edgesPageToken == "" &&
		(req.DefinitionKind != xpb.CrossReferencesRequest_NO_DEFINITIONS ||
			req.DeclarationKind != xpb.CrossReferencesRequest_NO_DECLARATIONS ||
			req.ReferenceKind != xpb.CrossReferencesRequest_NO_REFERENCES ||
			req.DocumentationKind != xpb.CrossReferencesRequest_NO_DOCUMENTATION ||
			req.CallerKind != xpb.CrossReferencesRequest_NO_CALLERS)

	for _, ticket := range tickets {
		// TODO(schroederc): retrieve PagedCrossReferences in parallel
		cr, err := t.crossReferences(ctx, ticket)
		if err == table.ErrNoSuchKey {
			log.Println("Missing CrossReferences:", ticket)
			continue
		} else if err != nil {
			return nil, fmt.Errorf("error looking up cross-references for ticket %q: %v", ticket, err)
		}

		crs := &xpb.CrossReferencesReply_CrossReferenceSet{
			Ticket: ticket,
		}

		if req.ExperimentalSignatures {
			crs.DisplayName, err = xrefs.SlowSignature(ctx, t, ticket)
			if err != nil {
				log.Printf("WARNING: error looking up signature for ticket %q: %v", ticket, err)
			}
		}

		for _, grp := range cr.Group {
			switch {
			case xrefs.IsDefKind(req.DefinitionKind, grp.Kind, cr.Incomplete):
				reply.Total.Definitions += int64(len(grp.Anchor))
				if wantMoreCrossRefs {
					stats.addAnchors(&crs.Definition, grp.Anchor, req.AnchorText)
				}
			case xrefs.IsDeclKind(req.DeclarationKind, grp.Kind, cr.Incomplete):
				reply.Total.Declarations += int64(len(grp.Anchor))
				if wantMoreCrossRefs {
					stats.addAnchors(&crs.Declaration, grp.Anchor, req.AnchorText)
				}
			case xrefs.IsDocKind(req.DocumentationKind, grp.Kind):
				reply.Total.Documentation += int64(len(grp.Anchor))
				if wantMoreCrossRefs {
					stats.addAnchors(&crs.Documentation, grp.Anchor, req.AnchorText)
				}
			case xrefs.IsRefKind(req.ReferenceKind, grp.Kind):
				reply.Total.References += int64(len(grp.Anchor))
				if wantMoreCrossRefs {
					stats.addAnchors(&crs.Reference, grp.Anchor, req.AnchorText)
				}
			}
		}

		if wantMoreCrossRefs && req.CallerKind != xpb.CrossReferencesRequest_NO_CALLERS {
			anchors, err := xrefs.SlowCallersForCrossReferences(ctx, t, req.CallerKind == xpb.CrossReferencesRequest_OVERRIDE_CALLERS, req.ExperimentalSignatures, ticket)
			if err != nil {
				return nil, fmt.Errorf("error in SlowCallersForCrossReferences: %v", err)
			}
			reply.Total.Callers += int64(len(anchors))
			stats.addRelatedAnchors(&crs.Caller, anchors, req.AnchorText)
		}

		for _, idx := range cr.PageIndex {
			switch {
			case xrefs.IsDefKind(req.DefinitionKind, idx.Kind, cr.Incomplete):
				reply.Total.Definitions += int64(idx.Count)
				if wantMoreCrossRefs && !stats.skipPage(idx) {
					p, err := t.crossReferencesPage(ctx, idx.PageKey)
					if err != nil {
						return nil, fmt.Errorf("internal error: error retrieving cross-references page: %v", idx.PageKey)
					}
					stats.addAnchors(&crs.Definition, p.Group.Anchor, req.AnchorText)
				}
			case xrefs.IsDeclKind(req.DeclarationKind, idx.Kind, cr.Incomplete):
				reply.Total.Declarations += int64(idx.Count)
				if wantMoreCrossRefs && !stats.skipPage(idx) {
					p, err := t.crossReferencesPage(ctx, idx.PageKey)
					if err != nil {
						return nil, fmt.Errorf("internal error: error retrieving cross-references page: %v", idx.PageKey)
					}
					stats.addAnchors(&crs.Declaration, p.Group.Anchor, req.AnchorText)
				}
			case xrefs.IsDocKind(req.DocumentationKind, idx.Kind):
				reply.Total.Documentation += int64(idx.Count)
				if wantMoreCrossRefs && !stats.skipPage(idx) {
					p, err := t.crossReferencesPage(ctx, idx.PageKey)
					if err != nil {
						return nil, fmt.Errorf("internal error: error retrieving cross-references page: %v", idx.PageKey)
					}
					stats.addAnchors(&crs.Documentation, p.Group.Anchor, req.AnchorText)
				}
			case xrefs.IsRefKind(req.ReferenceKind, idx.Kind):
				reply.Total.References += int64(idx.Count)
				if wantMoreCrossRefs && !stats.skipPage(idx) {
					p, err := t.crossReferencesPage(ctx, idx.PageKey)
					if err != nil {
						return nil, fmt.Errorf("internal error: error retrieving cross-references page: %v", idx.PageKey)
					}
					stats.addAnchors(&crs.Reference, p.Group.Anchor, req.AnchorText)
				}
			}
		}

		if len(crs.Declaration) > 0 || len(crs.Definition) > 0 || len(crs.Reference) > 0 || len(crs.Documentation) > 0 || len(crs.Caller) > 0 {
			reply.CrossReferences[crs.Ticket] = crs
		}
	}

	if pageToken+stats.total != sumTotalCrossRefs(reply.Total) && stats.total != 0 {
		nextToken = &ipb.PageToken{Index: int32(pageToken + stats.total)}
	}

	if len(req.Filter) > 0 {
		er, err := t.edges(ctx, edgesRequest{
			Tickets:   tickets,
			Filters:   req.Filter,
			Kinds:     func(kind string) bool { return !schema.IsAnchorEdge(kind) },
			PageToken: edgesPageToken,
			TotalOnly: (stats.max <= stats.total),
			PageSize:  stats.max - stats.total,
		})
		if err != nil {
			return nil, fmt.Errorf("error getting related nodes: %v", err)
		}
		reply.Total.RelatedNodesByRelation = er.TotalEdgesByKind
		for ticket, es := range er.EdgeSets {
			nodes := stringset.New()
			crs, ok := reply.CrossReferences[ticket]
			if !ok {
				crs = &xpb.CrossReferencesReply_CrossReferenceSet{
					Ticket: ticket,
				}
			}
			for kind, g := range es.Groups {
				for _, edge := range g.Edge {
					nodes.Add(edge.TargetTicket)
					crs.RelatedNode = append(crs.RelatedNode, &xpb.CrossReferencesReply_RelatedNode{
						RelationKind: kind,
						Ticket:       edge.TargetTicket,
						Ordinal:      edge.Ordinal,
					})
				}
			}
			if len(nodes) > 0 {
				for ticket, n := range er.Nodes {
					if nodes.Contains(ticket) {
						reply.Nodes[ticket] = n
					}
				}
			}

			if !ok && len(crs.RelatedNode) > 0 {
				reply.CrossReferences[ticket] = crs
			}
		}

		if er.NextPageToken != "" {
			nextToken = &ipb.PageToken{SecondaryToken: er.NextPageToken}
		}
	}

	if nextToken != nil {
		rec, err := proto.Marshal(nextToken)
		if err != nil {
			return nil, fmt.Errorf("internal error: error marshalling page token: %v", err)
		}
		reply.NextPageToken = base64.StdEncoding.EncodeToString(rec)
	}

	if req.NodeDefinitions {
		nodeTickets := make([]string, 0, len(reply.Nodes))
		for ticket := range reply.Nodes {
			nodeTickets = append(nodeTickets, ticket)
		}

		// TODO(schroederc): cache this in the serving data
		defs, err := xrefs.SlowDefinitions(t, ctx, nodeTickets)
		if err != nil {
			return nil, fmt.Errorf("error retrieving node definitions: %v", err)
		}

		reply.DefinitionLocations = make(map[string]*xpb.Anchor, len(defs))
		for ticket, def := range defs {
			node, ok := reply.Nodes[ticket]
			if !ok {
				panic(fmt.Sprintf("extra definition returned for unknown node %q: %v", ticket, def))
			}
			node.Definition = def.Ticket
			if _, ok := reply.DefinitionLocations[def.Ticket]; !ok {
				reply.DefinitionLocations[def.Ticket] = def
			}
		}
	}

	return reply, nil
}

func sumTotalCrossRefs(ts *xpb.CrossReferencesReply_Total) int {
	var relatedNodes int
	for _, cnt := range ts.RelatedNodesByRelation {
		relatedNodes += int(cnt)
	}
	return int(ts.Definitions) + int(ts.Declarations) + int(ts.References) + int(ts.Documentation) + relatedNodes
}

type refStats struct {
	skip, total, max int
}

func (s *refStats) skipPage(idx *srvpb.PagedCrossReferences_PageIndex) bool {
	if s.skip > int(idx.Count) {
		s.skip -= int(idx.Count)
		return true
	}
	return false
}

func (s *refStats) addAnchors(to *[]*xpb.CrossReferencesReply_RelatedAnchor, as []*srvpb.ExpandedAnchor, anchorText bool) bool {
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
		*to = append(*to, a2a(a, anchorText))
	}
	return s.total == s.max
}

func (s *refStats) addRelatedAnchors(to *[]*xpb.CrossReferencesReply_RelatedAnchor, as []*xpb.CrossReferencesReply_RelatedAnchor, anchorText bool) bool {
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
		if !anchorText {
			a.Anchor.Text = ""
		}
		*to = append(*to, a)
	}
	return s.total == s.max
}

func a2a(a *srvpb.ExpandedAnchor, anchorText bool) *xpb.CrossReferencesReply_RelatedAnchor {
	var text string
	if anchorText {
		text = a.Text
	}
	return &xpb.CrossReferencesReply_RelatedAnchor{Anchor: &xpb.Anchor{
		Ticket:       a.Ticket,
		Kind:         schema.Canonicalize(a.Kind),
		Parent:       a.Parent,
		Text:         text,
		Start:        p2p(a.Span.Start),
		End:          p2p(a.Span.End),
		Snippet:      a.Snippet,
		SnippetStart: p2p(a.SnippetSpan.Start),
		SnippetEnd:   p2p(a.SnippetSpan.End),
	}}
}

func p2p(p *cpb.Point) *xpb.Location_Point {
	return &xpb.Location_Point{
		ByteOffset:   p.ByteOffset,
		LineNumber:   p.LineNumber,
		ColumnOffset: p.ColumnOffset,
	}
}

// Callers implements part of the xrefs Service interface.
func (t *tableImpl) Callers(ctx context.Context, req *xpb.CallersRequest) (*xpb.CallersReply, error) {
	return xrefs.SlowCallers(ctx, t, req)
}

// Callers implements part of the xrefs Service interface.
func (t *tableImpl) Documentation(ctx context.Context, req *xpb.DocumentationRequest) (*xpb.DocumentationReply, error) {
	return xrefs.SlowDocumentation(ctx, t, req)
}
