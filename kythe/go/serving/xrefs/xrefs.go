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
//   nodes:<ticket>         -> srvpb.Node
//   edgeSets:<ticket>      -> srvpb.PagedEdgeSet
//   edgePages:<page_token> -> srvpb.EdgePage
//   decor:<ticket>         -> srvpb.FileDecorations
package xrefs

import (
	"encoding/base64"
	"errors"
	"fmt"
	"log"
	"strconv"

	"kythe.io/kythe/go/services/xrefs"
	"kythe.io/kythe/go/storage/table"
	"kythe.io/kythe/go/util/kytheuri"
	"kythe.io/kythe/go/util/schema"
	"kythe.io/kythe/go/util/stringset"

	srvpb "kythe.io/kythe/proto/serving_proto"
	xpb "kythe.io/kythe/proto/xref_proto"

	"github.com/golang/protobuf/proto"
	"golang.org/x/net/context"
)

type staticLookupTables interface {
	node(ctx context.Context, ticket string) (*srvpb.Node, error)
	pagedEdgeSet(ctx context.Context, ticket string) (*srvpb.PagedEdgeSet, error)
	edgePage(ctx context.Context, key string) (*srvpb.EdgePage, error)
	fileDecorations(ctx context.Context, ticket string) (*srvpb.FileDecorations, error)
}

// SplitTable implements the xrefs Service interface using separate static
// lookup tables for each API component.
type SplitTable struct {
	// Nodes is a table of srvpb.Nodes keyed by their tickets.
	Nodes table.Proto

	// Edges is a table of srvpb.PagedEdgeSets keyed by their source tickets.
	Edges table.Proto

	// EdgePages is a table of srvpb.EdgePages keyed by their page keys.
	EdgePages table.Proto

	// Decorations is a table of srvpb.FileDecorations keyed by their source
	// location tickets.
	Decorations table.Proto
}

func (s *SplitTable) node(ctx context.Context, ticket string) (*srvpb.Node, error) {
	var n srvpb.Node
	err := s.Nodes.Lookup(ctx, []byte(ticket), &n)
	return &n, err
}
func (s *SplitTable) pagedEdgeSet(ctx context.Context, ticket string) (*srvpb.PagedEdgeSet, error) {
	var pes srvpb.PagedEdgeSet
	err := s.Edges.Lookup(ctx, []byte(ticket), &pes)
	return &pes, err
}

func (s *SplitTable) edgePage(ctx context.Context, key string) (*srvpb.EdgePage, error) {
	var ep srvpb.EdgePage
	err := s.EdgePages.Lookup(ctx, []byte(key), &ep)
	return &ep, err
}
func (s *SplitTable) fileDecorations(ctx context.Context, ticket string) (*srvpb.FileDecorations, error) {
	var fd srvpb.FileDecorations
	err := s.Decorations.Lookup(ctx, []byte(ticket), &fd)
	return &fd, err
}

// Key prefixed for the combinedTable implementation.
const (
	nodesTablePrefix     = "nodes:"
	decorTablePrefix     = "decor:"
	edgeSetsTablePrefix  = "edgeSets:"
	edgePagesTablePrefix = "edgePages:"
)

type combinedTable struct{ table.Proto }

func (c *combinedTable) node(ctx context.Context, ticket string) (*srvpb.Node, error) {
	var n srvpb.Node
	err := c.Lookup(ctx, NodeKey(ticket), &n)
	return &n, err
}
func (c *combinedTable) pagedEdgeSet(ctx context.Context, ticket string) (*srvpb.PagedEdgeSet, error) {
	var pes srvpb.PagedEdgeSet
	err := c.Lookup(ctx, EdgeSetKey(ticket), &pes)
	return &pes, err
}
func (c *combinedTable) edgePage(ctx context.Context, key string) (*srvpb.EdgePage, error) {
	var ep srvpb.EdgePage
	err := c.Lookup(ctx, EdgePageKey(key), &ep)
	return &ep, err
}
func (c *combinedTable) fileDecorations(ctx context.Context, ticket string) (*srvpb.FileDecorations, error) {
	var fd srvpb.FileDecorations
	err := c.Lookup(ctx, DecorationsKey(ticket), &fd)
	return &fd, err
}

// NewSplitTable returns an xrefs.Service based on the given serving tables for
// each API component.
func NewSplitTable(c *SplitTable) xrefs.Service { return &tableImpl{c} }

// NewCombinedTable returns an xrefs.Service for the given combined xrefs
// serving table.  The table's keys are expected to be constructed using only
// the NodeKey, EdgeSetKey, EdgePageKey, and DecorationsKey functions.
func NewCombinedTable(t table.Proto) xrefs.Service { return &tableImpl{&combinedTable{t}} }

// NodeKey returns the nodes CombinedTable key for the given ticket.
func NodeKey(ticket string) []byte {
	return []byte(nodesTablePrefix + ticket)
}

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

// tableImpl implements the xrefs Service interface using static lookup tables.
// TODO(schroederc): parallelize multiple lookup requests
type tableImpl struct{ staticLookupTables }

// Nodes implements part of the xrefs Service interface.
func (t *tableImpl) Nodes(ctx context.Context, req *xpb.NodesRequest) (*xpb.NodesReply, error) {
	reply := &xpb.NodesReply{}
	patterns := xrefs.ConvertFilters(req.Filter)
	for _, rawTicket := range req.Ticket {
		ticket, err := kytheuri.Fix(rawTicket)
		if err != nil {
			return nil, fmt.Errorf("invalid ticket %q: %v", rawTicket, err)
		}

		n, err := t.node(ctx, ticket)
		if err == table.ErrNoSuchKey {
			continue
		} else if err != nil {
			return nil, fmt.Errorf("lookup error for node %q: %v", ticket, err)
		}
		ni := &xpb.NodeInfo{Ticket: n.Ticket}
		for _, fact := range n.Fact {
			if len(patterns) == 0 || xrefs.MatchesAny(fact.Name, patterns) {
				ni.Fact = append(ni.Fact, &xpb.Fact{Name: fact.Name, Value: fact.Value})
			}
		}
		if len(ni.Fact) > 0 {
			reply.Node = append(reply.Node, ni)
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
	if len(req.Ticket) == 0 {
		return nil, errors.New("no tickets specified")
	}
	stats := filterStats{
		max: int(req.PageSize),
	}
	if stats.max < 0 {
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
		var t srvpb.PageToken
		if err := proto.Unmarshal(rec, &t); err != nil || t.Index < 0 {
			return nil, fmt.Errorf("invalid page_token: %q", req.PageToken)
		}
		stats.skip = int(t.Index)
	}
	pageToken := stats.skip

	var totalEdgesPossible int

	allowedKinds := stringset.New(req.Kind...)
	nodeTickets := stringset.New()

	reply := &xpb.EdgesReply{}
	for _, rawTicket := range req.Ticket {
		ticket, err := kytheuri.Fix(rawTicket)
		if err != nil {
			return nil, fmt.Errorf("invalid ticket %q: %v", rawTicket, err)
		}

		pes, err := t.pagedEdgeSet(ctx, ticket)
		if err == table.ErrNoSuchKey {
			continue
		} else if err != nil {
			return nil, fmt.Errorf("lookup error for node edges %q: %v", ticket, err)
		}
		totalEdgesPossible += int(pes.TotalEdges)

		var groups []*xpb.EdgeSet_Group
		for _, grp := range pes.EdgeSet.Group {
			if len(allowedKinds) == 0 || allowedKinds.Contains(grp.Kind) {
				ng := stats.filter(grp)
				if ng != nil {
					nodeTickets.Add(ng.TargetTicket...)
					groups = append(groups, ng)
					if stats.total == stats.max {
						break
					}
				}
			}
		}

		for _, idx := range pes.PageIndex {
			if len(allowedKinds) == 0 || allowedKinds.Contains(idx.EdgeKind) {
				ep, err := t.edgePage(ctx, idx.PageKey)
				if err == table.ErrNoSuchKey {
					return nil, fmt.Errorf("missing edge page: %q", idx.PageKey)
				} else if err != nil {
					return nil, fmt.Errorf("lookup error for node edges %q: %v", ticket, err)
				}

				ng := stats.filter(ep.EdgesGroup)
				if ng != nil {
					nodeTickets.Add(ng.TargetTicket...)
					groups = append(groups, ng)
					if stats.total == stats.max {
						break
					}
				}
			}
		}

		if len(groups) > 0 {
			nodeTickets.Add(pes.EdgeSet.SourceTicket)
			reply.EdgeSet = append(reply.EdgeSet, &xpb.EdgeSet{
				SourceTicket: pes.EdgeSet.SourceTicket,
				Group:        groups,
			})
		}
	}
	if stats.total > stats.max {
		log.Panicf("totalEdges greater than maxEdges: %d > %d", stats.total, stats.max)
	} else if pageToken+stats.total > totalEdgesPossible && pageToken <= totalEdgesPossible {
		log.Panicf("pageToken+totalEdges greater than totalEdgesPossible: %d+%d > %d", pageToken, stats.total, totalEdgesPossible)
	}

	if len(req.Filter) > 0 {
		nReply, err := t.Nodes(ctx, &xpb.NodesRequest{
			Ticket: nodeTickets.Slice(),
			Filter: req.Filter,
		})
		if err != nil {
			return nil, fmt.Errorf("error getting nodes: %v", err)
		}
		reply.Node = nReply.Node
	}

	if pageToken+stats.total != totalEdgesPossible && stats.total != 0 {
		// TODO: take into account an empty last page (due to kind filters)
		rec, err := proto.Marshal(&srvpb.PageToken{Index: int32(pageToken + stats.total)})
		if err != nil {
			return nil, fmt.Errorf("error marshalling page token: %v", err)
		}
		reply.NextPageToken = base64.StdEncoding.EncodeToString(rec)
	}
	return reply, nil
}

type filterStats struct {
	skip, total, max int
}

func (s *filterStats) filter(g *srvpb.EdgeSet_Group) *xpb.EdgeSet_Group {
	targets := g.TargetTicket
	if len(g.TargetTicket) <= s.skip {
		s.skip -= len(g.TargetTicket)
		return nil
	} else if s.skip > 0 {
		targets = targets[s.skip:]
		s.skip = 0
	}

	if len(targets) > s.max-s.total {
		targets = targets[:(s.max - s.total)]
	}

	s.total += len(targets)
	return &xpb.EdgeSet_Group{
		Kind:         g.Kind,
		TargetTicket: targets,
	}
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
		return nil, fmt.Errorf("decorations not found for file %q", ticket)
	} else if err != nil {
		return nil, fmt.Errorf("lookup error for file decorations %q: %v", ticket, err)
	}

	text := decor.SourceText
	if len(req.DirtyBuffer) > 0 {
		text = req.DirtyBuffer
	}
	loc, err := xrefs.NewNormalizer(text).Location(req.GetLocation())
	if err != nil {
		return nil, err
	}

	reply := &xpb.DecorationsReply{Location: loc}

	if req.SourceText {
		reply.Encoding = decor.Encoding
		if loc.Kind == xpb.Location_FILE {
			reply.SourceText = text
		} else {
			reply.SourceText = text[loc.Start.ByteOffset:loc.End.ByteOffset]
		}
	}

	if req.References {
		// Set of node tickets for which to retrieve facts.  These are the nodes
		// used in the returned references (both anchor sources and node targets).
		nodeTickets := stringset.New()

		var patcher *xrefs.Patcher
		var offsetMapping map[string]span // Map from anchor ticket to patched span
		if len(req.DirtyBuffer) > 0 {
			patcher = xrefs.NewPatcher(decor.SourceText, req.DirtyBuffer)
			offsetMapping = make(map[string]span)
		}

		// The span with which to constrain the set of returned anchor references.
		var startBoundary, endBoundary int32
		if loc.Kind == xpb.Location_FILE {
			startBoundary = 0
			endBoundary = int32(len(text))
		} else {
			startBoundary = loc.Start.ByteOffset
			endBoundary = loc.End.ByteOffset
		}

		reply.Reference = make([]*xpb.DecorationsReply_Reference, 0, len(decor.Decoration))
		for _, d := range decor.Decoration {
			start, end, exists := patcher.Patch(d.Anchor.StartOffset, d.Anchor.EndOffset)
			// Filter non-existent anchor.  Anchors can no longer exist if we were
			// given a dirty buffer and the anchor was inside a changed region.
			if exists {
				if start >= startBoundary && end <= endBoundary {
					if offsetMapping != nil {
						// Save the patched span to update the corresponding facts of the
						// anchor node in reply.Node.
						offsetMapping[d.Anchor.Ticket] = span{start, end}
					}
					reply.Reference = append(reply.Reference, decorationToReference(d))
					nodeTickets.Add(d.Anchor.Ticket)
					nodeTickets.Add(d.TargetTicket)
				}
			}
		}

		// Retrieve facts for all nodes referenced in the file decorations.
		nodesReply, err := t.Nodes(ctx, &xpb.NodesRequest{Ticket: nodeTickets.Slice()})
		if err != nil {
			return nil, fmt.Errorf("error getting nodes: %v", err)
		}
		reply.Node = nodesReply.Node

		// Patch anchor node facts in reply to match dirty buffer
		if len(offsetMapping) > 0 {
			for _, n := range reply.Node {
				if span, ok := offsetMapping[n.Ticket]; ok {
					for _, f := range n.Fact {
						switch f.Name {
						case schema.AnchorStartFact:
							f.Value = []byte(strconv.Itoa(int(span.start)))
						case schema.AnchorEndFact:
							f.Value = []byte(strconv.Itoa(int(span.end)))
						}
					}
				}
			}
		}
	}

	return reply, nil
}

type span struct{ start, end int32 }

func decorationToReference(d *srvpb.FileDecorations_Decoration) *xpb.DecorationsReply_Reference {
	return &xpb.DecorationsReply_Reference{
		SourceTicket: d.Anchor.Ticket,
		TargetTicket: d.TargetTicket,
		Kind:         d.Kind,
	}
}
