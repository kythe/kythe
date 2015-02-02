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
	"bytes"
	"errors"
	"fmt"
	"log"
	"strconv"

	"kythe/go/services/xrefs"
	"kythe/go/storage/table"
	"kythe/go/util/stringset"

	srvpb "kythe/proto/serving_proto"
	xpb "kythe/proto/xref_proto"

	"code.google.com/p/goprotobuf/proto"
)

const (
	nodesTablePrefix     = "nodes:"
	decorTablePrefix     = "decor:"
	edgeSetsTablePrefix  = "edgeSets:"
	edgePagesTablePrefix = "edgePages:"
)

// Table implements the xrefs Service interface using a static lookup table.
// TODO(schroederc): parallelize multiple Table.DB lookup requests
type Table struct{ table.Proto }

// Nodes implements part of the xrefs Service interface.
func (t *Table) Nodes(req *xpb.NodesRequest) (*xpb.NodesReply, error) {
	reply := &xpb.NodesReply{}
	patterns := xrefs.ConvertFilters(req.Filter)
	for _, ticket := range req.Ticket {
		n, err := t.rawNode(ticket)
		if err == table.ErrNoSuchKey {
			continue
		} else if err != nil {
			return nil, fmt.Errorf("lookup error for node %q: %v", ticket, err)
		}
		ni := &xpb.NodeInfo{Ticket: n.Ticket}
		for _, fact := range n.Fact {
			if len(patterns) == 0 || xrefs.MatchesAny(fact.GetName(), patterns) {
				ni.Fact = append(ni.Fact, &xpb.Fact{Name: fact.Name, Value: fact.Value})
			}
		}
		if len(ni.Fact) > 0 {
			reply.Node = append(reply.Node, ni)
		}
	}
	return reply, nil
}

func (t *Table) rawNode(ticket string) (*srvpb.Node, error) {
	var n srvpb.Node
	err := t.Lookup(NodeKey(ticket), &n)
	return &n, err
}

// NodeKey returns the nodes lookup table key for the given ticket.
func NodeKey(ticket string) []byte {
	return []byte(nodesTablePrefix + ticket)
}

const (
	defaultPageSize = 2048
	maxPageSize     = 10000
)

// Edges implements part of the xrefs Service interface.
func (t *Table) Edges(req *xpb.EdgesRequest) (*xpb.EdgesReply, error) {
	stats := filterStats{
		max: int(req.GetPageSize()),
	}
	if stats.max < 0 {
		return nil, fmt.Errorf("invalid page_size: %d", req.GetPageSize())
	} else if stats.max == 0 {
		stats.max = defaultPageSize
	} else if stats.max > maxPageSize {
		stats.max = maxPageSize
	}

	if req.GetPageToken() != "" {
		n, err := strconv.Atoi(req.GetPageToken())
		if err != nil || n < 0 {
			return nil, fmt.Errorf("invalid page_token: %q", req.GetPageToken())
		}
		stats.skip = n
	}
	pageToken := stats.skip

	var totalEdgesPossible int

	allowedKinds := stringset.New(req.Kind...)
	nodeTickets := stringset.New()

	reply := &xpb.EdgesReply{}
	for _, ticket := range req.Ticket {
		var pes srvpb.PagedEdgeSet
		if err := t.Lookup(EdgeSetKey(ticket), &pes); err == table.ErrNoSuchKey {
			continue
		} else if err != nil {
			return nil, fmt.Errorf("lookup error for node edges %q: %v", ticket, err)
		}
		totalEdgesPossible += int(pes.GetTotalEdges())

		var groups []*xpb.EdgeSet_Group
		for _, grp := range pes.EdgeSet.Group {
			if len(allowedKinds) == 0 || allowedKinds.Contains(grp.GetKind()) {
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
			if len(allowedKinds) == 0 || allowedKinds.Contains(idx.GetEdgeKind()) {
				var ep srvpb.EdgePage
				if err := t.Lookup([]byte(edgePagesTablePrefix+idx.GetPageKey()), &ep); err == table.ErrNoSuchKey {
					return nil, fmt.Errorf("missing edge page: %q", idx.GetPageKey())
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
			nodeTickets.Add(pes.EdgeSet.GetSourceTicket())
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
		nReply, err := t.Nodes(&xpb.NodesRequest{
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
		reply.NextPageToken = proto.String(strconv.Itoa(pageToken + stats.total))
	}
	return reply, nil
}

// EdgeSetKey returns the edgeset lookup table key for the given ticket.
func EdgeSetKey(ticket string) []byte {
	return []byte(edgeSetsTablePrefix + ticket)
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
func (t *Table) Decorations(req *xpb.DecorationsRequest) (*xpb.DecorationsReply, error) {
	if len(req.DirtyBuffer) > 0 {
		log.Println("TODO: implement DecorationsRequest.DirtyBuffer")
		return nil, errors.New("dirty buffers unimplemented")
	}

	var decor srvpb.FileDecorations
	ticket := req.GetLocation().GetTicket()
	if err := t.Lookup(DecorationsKey(ticket), &decor); err == table.ErrNoSuchKey {
		return nil, fmt.Errorf("decorations not found for file %q", ticket)
	} else if err != nil {
		return nil, fmt.Errorf("lookup error for file decorations %q: %v", ticket, err)
	}

	reply := &xpb.DecorationsReply{}
	windowStart := req.GetLocation().GetStart().GetByteOffset()
	windowEnd := req.GetLocation().GetEnd().GetByteOffset()
	if windowStart > windowEnd {
		return nil, fmt.Errorf("invalid SPAN: start (%d) is after end (%d)", windowStart, windowEnd)
	} else if windowEnd >= int32(len(decor.SourceText)) {
		return nil, fmt.Errorf("invalid SPAN: end (%d) is past size of source text (%d)", windowEnd, len(decor.SourceText))
	} else if windowStart < 0 || windowEnd < 0 {
		return nil, fmt.Errorf("invalid SPAN: negative offset {%+v}", req.GetLocation())
	}

	if req.GetSourceText() {
		reply.Encoding = decor.Encoding
		if req.GetLocation().GetKind() == xpb.Location_FILE {
			reply.SourceText = decor.SourceText
		} else {
			reply.SourceText = decor.SourceText[windowStart:windowEnd]
		}
	}

	if req.GetReferences() {
		nodeTickets := stringset.New()
		if req.Location.GetKind() == xpb.Location_FILE {
			reply.Reference = make([]*xpb.DecorationsReply_Reference, len(decor.Decoration))
			for i, d := range decor.Decoration {
				reply.Reference[i] = decorationToReference(d)
				nodeTickets.Add(d.Anchor.GetTicket())
				nodeTickets.Add(d.GetTargetTicket())
			}
		} else {
			for _, d := range decor.Decoration {
				// TODO(schroederc): handle invalid Anchor spans (e.g. [100 -1])
				if d.Anchor.GetStartOffset() >= windowStart && d.Anchor.GetEndOffset() < windowEnd {
					reply.Reference = append(reply.Reference, decorationToReference(d))
					nodeTickets.Add(d.Anchor.GetTicket())
					nodeTickets.Add(d.GetTargetTicket())
				}
			}
		}

		nodesReply, err := t.Nodes(&xpb.NodesRequest{Ticket: nodeTickets.Slice()})
		if err != nil {
			return nil, fmt.Errorf("error getting nodes: %v", err)
		}
		reply.Node = nodesReply.Node
	}

	reply.Location = &xpb.Location{
		Ticket: req.Location.Ticket,
		Kind:   req.Location.Kind,
	}
	if req.Location.GetKind() == xpb.Location_SPAN {
		reply.Location.Start = normalizePoint(decor.SourceText, req.Location.Start)
		reply.Location.End = normalizePoint(decor.SourceText, req.Location.End)
	}

	return reply, nil
}

// DecorationsKey returns the decorations lookup table key for the given ticket.
func DecorationsKey(ticket string) []byte {
	return []byte(decorTablePrefix + ticket)
}

func decorationToReference(d *srvpb.FileDecorations_Decoration) *xpb.DecorationsReply_Reference {
	return &xpb.DecorationsReply_Reference{
		SourceTicket: d.Anchor.Ticket,
		TargetTicket: d.TargetTicket,
		Kind:         d.Kind,
	}
}

var lineEnd = []byte("\n")

func normalizePoint(text []byte, p *xpb.Location_Point) *xpb.Location_Point {
	if p == nil {
		return nil
	}
	// TODO line (+column)? -> byte_offset (+column)? conversion
	offset := p.GetByteOffset()
	textBefore := text[:offset]
	np := &xpb.Location_Point{
		ByteOffset: p.ByteOffset,
		LineNumber: proto.Int(bytes.Count(textBefore, lineEnd) + 1),
	}
	lineStart := int32(bytes.LastIndex(textBefore, lineEnd))
	if lineStart != -1 {
		np.ColumnOffset = proto.Int32(offset - lineStart - 1)
	} else {
		np.ColumnOffset = proto.Int32(offset)
	}
	return np
}
