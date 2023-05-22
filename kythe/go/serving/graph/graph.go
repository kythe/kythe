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

// Package graph provides a high-performance table-based implementation of the
// graph.Service.
//
// Table format:
//
//	edgeSets:<ticket>      -> srvpb.PagedEdgeSet
//	edgePages:<page_key>   -> srvpb.EdgePage
package graph // import "kythe.io/kythe/go/serving/graph"

import (
	"context"
	"encoding/base64"
	"fmt"
	"regexp"
	"strings"

	"kythe.io/kythe/go/services/xrefs"
	"kythe.io/kythe/go/storage/table"
	"kythe.io/kythe/go/util/log"

	"bitbucket.org/creachadair/stringset"
	"golang.org/x/net/trace"
	"google.golang.org/protobuf/proto"

	cpb "kythe.io/kythe/proto/common_go_proto"
	gpb "kythe.io/kythe/proto/graph_go_proto"
	ipb "kythe.io/kythe/proto/internal_go_proto"
	srvpb "kythe.io/kythe/proto/serving_go_proto"
)

func tracePrintf(ctx context.Context, msg string, args ...any) {
	if t, ok := trace.FromContext(ctx); ok {
		t.LazyPrintf(msg, args...)
	}
}

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

// Key prefixes for the combinedTable implementation.
const (
	edgeSetsTablePrefix  = "edgeSets:"
	edgePagesTablePrefix = "edgePages:"
)

type edgeSetResult struct {
	PagedEdgeSet *srvpb.PagedEdgeSet

	Err error
}

type staticLookupTables interface {
	pagedEdgeSets(ctx context.Context, tickets []string) (<-chan edgeSetResult, error)
	edgePage(ctx context.Context, key string) (*srvpb.EdgePage, error)
}

// SplitTable implements the graph Service interface using separate static
// lookup tables for each API component.
type SplitTable struct {
	// Edges is a table of srvpb.PagedEdgeSets keyed by their source tickets.
	Edges table.Proto

	// EdgePages is a table of srvpb.EdgePages keyed by their page keys.
	EdgePages table.Proto
}

func lookupPagedEdgeSets(ctx context.Context, tbl table.Proto, keys [][]byte) (<-chan edgeSetResult, error) {
	ch := make(chan edgeSetResult)
	go func() {
		defer close(ch)
		for _, key := range keys {
			var pes srvpb.PagedEdgeSet
			if err := tbl.Lookup(ctx, key, &pes); err == table.ErrNoSuchKey {
				log.Warningf("Could not locate edges with key %q", key)
				ch <- edgeSetResult{Err: err}
				continue
			} else if err != nil {
				ticket := strings.TrimPrefix(string(key), edgeSetsTablePrefix)
				ch <- edgeSetResult{
					Err: fmt.Errorf("edges lookup error (ticket %q): %v", ticket, err),
				}
				continue
			}

			ch <- edgeSetResult{PagedEdgeSet: &pes}
		}
	}()
	return ch, nil
}

func toKeys(ss []string) [][]byte {
	keys := make([][]byte, len(ss))
	for i, s := range ss {
		keys[i] = []byte(s)
	}
	return keys
}

const (
	defaultPageSize = 2048
	maxPageSize     = 10000
)

func (s *SplitTable) pagedEdgeSets(ctx context.Context, tickets []string) (<-chan edgeSetResult, error) {
	tracePrintf(ctx, "Reading PagedEdgeSets: %s", tickets)
	return lookupPagedEdgeSets(ctx, s.Edges, toKeys(tickets))
}
func (s *SplitTable) edgePage(ctx context.Context, key string) (*srvpb.EdgePage, error) {
	tracePrintf(ctx, "Reading EdgePage: %s", key)
	var ep srvpb.EdgePage
	return &ep, s.EdgePages.Lookup(ctx, []byte(key), &ep)
}

// Table implements the GraphService interface using static lookup tables.
type Table struct{ staticLookupTables }

// Nodes implements part of the graph Service interface.
func (t *Table) Nodes(ctx context.Context, req *gpb.NodesRequest) (*gpb.NodesReply, error) {
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
		for range rs {
		}
	}()

	reply := &gpb.NodesReply{Nodes: make(map[string]*cpb.NodeInfo, len(req.Ticket))}
	patterns := xrefs.ConvertFilters(req.Filter)

	for r := range rs {
		if r.Err == table.ErrNoSuchKey {
			continue
		} else if r.Err != nil {
			return nil, r.Err
		}
		node := r.PagedEdgeSet.Source
		ni := &cpb.NodeInfo{Facts: make(map[string][]byte, len(node.Fact))}
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

// Edges implements part of the graph Service interface.
func (t *Table) Edges(ctx context.Context, req *gpb.EdgesRequest) (*gpb.EdgesReply, error) {
	tickets, err := xrefs.FixTickets(req.Ticket)
	if err != nil {
		return nil, err
	}

	allowedKinds := stringset.New(req.Kind...)
	return t.edges(ctx, edgesRequest{
		Tickets: tickets,
		Filters: req.Filter,
		Kinds: func(kind string) bool {
			return allowedKinds.Empty() || allowedKinds.Contains(kind)
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

func (t *Table) edges(ctx context.Context, req edgesRequest) (*gpb.EdgesReply, error) {
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

	var nodeTickets stringset.Set

	rs, err := t.pagedEdgeSets(ctx, req.Tickets)
	if err != nil {
		return nil, err
	}
	defer func() {
		// drain channel in case of errors or early return
		for range rs {
		}
	}()

	patterns := xrefs.ConvertFilters(req.Filters)

	reply := &gpb.EdgesReply{
		EdgeSets: make(map[string]*gpb.EdgeSet),
		Nodes:    make(map[string]*cpb.NodeInfo),

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

		groups := make(map[string]*gpb.EdgeSet_Group)
		for _, grp := range pes.Group {
			if req.Kinds == nil || req.Kinds(grp.Kind) {
				ng, ns := stats.filter(grp)
				if ng != nil {
					for _, n := range ns {
						if len(patterns) > 0 && !nodeTickets.Contains(n.Ticket) {
							nodeTickets.Add(n.Ticket)
							if info := nodeToInfo(patterns, n); info != nil {
								reply.Nodes[n.Ticket] = info
							}
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
						log.Warningf("Skipping EdgePage: %s", idx.PageKey)
						continue
					}

					log.Infof("Retrieving EdgePage: %s", idx.PageKey)
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
								if info := nodeToInfo(patterns, n); info != nil {
									reply.Nodes[n.Ticket] = info
								}
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
			reply.EdgeSets[pes.Source.Ticket] = &gpb.EdgeSet{Groups: groups}

			if len(patterns) > 0 && !nodeTickets.Contains(pes.Source.Ticket) {
				nodeTickets.Add(pes.Source.Ticket)
				if info := nodeToInfo(patterns, pes.Source); info != nil {
					reply.Nodes[pes.Source.Ticket] = info
				}
			}
		}
	}
	totalEdgesPossible := int(sumEdgeKinds(reply.TotalEdgesByKind))
	if stats.total > stats.max {
		panic(fmt.Sprintf("totalEdges greater than maxEdges: %d > %d", stats.total, stats.max))
	} else if pageToken+stats.total > totalEdgesPossible && pageToken <= totalEdgesPossible {
		panic(fmt.Sprintf("pageToken+totalEdges greater than totalEdgesPossible: %d+%d > %d", pageToken, stats.total, totalEdgesPossible))
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
	return s.total >= s.max
}

func (s *filterStats) filter(g *srvpb.EdgeGroup) (*gpb.EdgeSet_Group, []*srvpb.Node) {
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

	return &gpb.EdgeSet_Group{
		Edge: e2e(edges),
	}, targets
}

func e2e(es []*srvpb.EdgeGroup_Edge) []*gpb.EdgeSet_Group_Edge {
	edges := make([]*gpb.EdgeSet_Group_Edge, len(es))
	for i, e := range es {
		edges[i] = &gpb.EdgeSet_Group_Edge{
			TargetTicket: e.Target.Ticket,
			Ordinal:      e.Ordinal,
		}
	}
	return edges
}

// NewSplitTable returns a table based on the given serving tables for each API
// component.
func NewSplitTable(c *SplitTable) *Table { return &Table{c} }

// NewCombinedTable returns a table for the given combined graph lookup table.
// The table's keys are expected to be constructed using only the EdgeSetKey,
// EdgePageKey, and DecorationsKey functions.
func NewCombinedTable(t table.Proto) *Table { return &Table{&combinedTable{t}} }

// EdgeSetKey returns the edgeset CombinedTable key for the given source ticket.
func EdgeSetKey(ticket string) []byte {
	return []byte(edgeSetsTablePrefix + ticket)
}

// EdgePageKey returns the edgepage CombinedTable key for the given key.
func EdgePageKey(key string) []byte {
	return []byte(edgePagesTablePrefix + key)
}

type combinedTable struct{ table.Proto }

func (c *combinedTable) pagedEdgeSets(ctx context.Context, tickets []string) (<-chan edgeSetResult, error) {
	keys := make([][]byte, len(tickets))
	for i, ticket := range tickets {
		keys[i] = EdgeSetKey(ticket)
	}
	return lookupPagedEdgeSets(ctx, c, keys)
}
func (c *combinedTable) edgePage(ctx context.Context, key string) (*srvpb.EdgePage, error) {
	var ep srvpb.EdgePage
	return &ep, c.Lookup(ctx, EdgePageKey(key), &ep)
}
