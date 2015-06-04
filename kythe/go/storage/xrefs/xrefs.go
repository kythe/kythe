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

// Package xrefs contains a simple implementation of the xrefs.Service interface
// backed by a graphstore.Service.
package xrefs

import (
	"errors"
	"fmt"
	"io"
	"log"
	"sort"
	"strconv"
	"time"

	"kythe.io/kythe/go/services/graphstore"
	"kythe.io/kythe/go/services/xrefs"
	"kythe.io/kythe/go/util/kytheuri"
	"kythe.io/kythe/go/util/schema"
	"kythe.io/kythe/go/util/stringset"

	"golang.org/x/net/context"

	spb "kythe.io/kythe/proto/storage_proto"
	xpb "kythe.io/kythe/proto/xref_proto"
)

// EnsureReverseEdges checks if gs contains reverse edges.  If it doesn't, it
// will scan gs for all forward edges, adding a reverse for each back into the
// GraphStore.  This is necessary for a GraphStoreService to work properly.
func EnsureReverseEdges(ctx context.Context, gs graphstore.Service) error {
	var edge *spb.Entry
	if err := gs.Scan(ctx, &spb.ScanRequest{}, func(e *spb.Entry) error {
		if e.EdgeKind != "" {
			edge = e
			return io.EOF
		}
		return nil
	}); err != nil {
		return err
	}

	if edge == nil {
		log.Println("No edges found in GraphStore")
		return nil
	} else if schema.EdgeDirection(edge.EdgeKind) == schema.Reverse {
		return nil
	}

	var foundReverse bool
	if err := gs.Read(ctx, &spb.ReadRequest{
		Source:   edge.Target,
		EdgeKind: schema.MirrorEdge(edge.EdgeKind),
	}, func(entry *spb.Entry) error {
		foundReverse = true
		return nil
	}); err != nil {
		return fmt.Errorf("error checking for reverse edge: %v", err)
	}
	if foundReverse {
		return nil
	}
	return addReverseEdges(ctx, gs)
}

func addReverseEdges(ctx context.Context, gs graphstore.Service) error {
	log.Println("Adding reverse edges")
	var (
		totalEntries int
		addedEdges   int
	)
	startTime := time.Now()
	err := gs.Scan(ctx, new(spb.ScanRequest), func(entry *spb.Entry) error {
		kind := entry.EdgeKind
		if kind != "" && schema.EdgeDirection(kind) == schema.Forward {
			if err := gs.Write(ctx, &spb.WriteRequest{
				Source: entry.Target,
				Update: []*spb.WriteRequest_Update{{
					Target:    entry.Source,
					EdgeKind:  schema.MirrorEdge(kind),
					FactName:  entry.FactName,
					FactValue: entry.FactValue,
				}},
			}); err != nil {
				return fmt.Errorf("Failed to write reverse edge: %v", err)
			}
			addedEdges++
		}
		totalEntries++
		return nil
	})
	log.Printf("Wrote %d reverse edges to GraphStore (%d total entries): %v", addedEdges, totalEntries, time.Since(startTime))
	return err
}

// A GraphStoreService partially implements the xrefs.Service interface
// directly using a graphstore.Service with stored reverse edges.  This is a
// low-performance, simple alternative to creating the serving Table
// representation.
// TODO(schroederc): parallelize GraphStore calls
type GraphStoreService struct {
	gs graphstore.Service
}

// NewGraphStoreService returns a new GraphStoreService given an
// existing graphstore.Service.
func NewGraphStoreService(gs graphstore.Service) *GraphStoreService {
	return &GraphStoreService{gs}
}

// Nodes implements part of the Service interface.
func (g *GraphStoreService) Nodes(ctx context.Context, req *xpb.NodesRequest) (*xpb.NodesReply, error) {
	patterns := xrefs.ConvertFilters(req.Filter)

	var names []*spb.VName
	for _, ticket := range req.Ticket {
		name, err := kytheuri.ToVName(ticket)
		if err != nil {
			return nil, err
		}
		names = append(names, name)
	}
	var nodes []*xpb.NodeInfo
	for i, vname := range names {
		info := &xpb.NodeInfo{Ticket: req.Ticket[i]}
		if err := g.gs.Read(ctx, &spb.ReadRequest{Source: vname}, func(entry *spb.Entry) error {
			if len(patterns) == 0 || xrefs.MatchesAny(entry.FactName, patterns) {
				info.Fact = append(info.Fact, entryToFact(entry))
			}
			return nil
		}); err != nil {
			return nil, err
		}
		nodes = append(nodes, info)
	}
	return &xpb.NodesReply{Node: nodes}, nil
}

// Edges implements part of the Service interface.
func (g *GraphStoreService) Edges(ctx context.Context, req *xpb.EdgesRequest) (*xpb.EdgesReply, error) {
	if len(req.Ticket) == 0 {
		return nil, errors.New("no tickets specified")
	} else if req.PageToken != "" {
		return nil, errors.New("UNIMPLEMENTED: page_token")
	}

	patterns := xrefs.ConvertFilters(req.Filter)
	allowedKinds := stringset.New(req.Kind...)
	targetSet := stringset.New()
	reply := new(xpb.EdgesReply)

	for _, ticket := range req.Ticket {
		vname, err := kytheuri.ToVName(ticket)
		if err != nil {
			return nil, fmt.Errorf("invalid ticket %q: %v", ticket, err)
		}

		var (
			// EdgeKind -> StringSet<TargetTicket>
			filteredEdges = make(map[string]stringset.Set)
			filteredFacts []*xpb.Fact
		)

		if err := g.gs.Read(ctx, &spb.ReadRequest{
			Source:   vname,
			EdgeKind: "*",
		}, func(entry *spb.Entry) error {
			edgeKind := entry.EdgeKind
			if edgeKind == "" {
				// node fact
				if len(patterns) > 0 && xrefs.MatchesAny(entry.FactName, patterns) {
					filteredFacts = append(filteredFacts, entryToFact(entry))
				}
			} else {
				// edge
				if len(req.Kind) == 0 || allowedKinds.Contains(edgeKind) {
					targets := filteredEdges[edgeKind]
					if targets == nil {
						targets = stringset.New()
						filteredEdges[edgeKind] = targets
					}
					targets.Add(kytheuri.ToString(entry.Target))
				}
			}
			return nil
		}); err != nil {
			return nil, fmt.Errorf("failed to retrieve entries for ticket %q", ticket)
		}

		// Only add a EdgeSet if there are targets for the requested edge kinds.
		if len(filteredEdges) > 0 {
			var groups []*xpb.EdgeSet_Group
			for edgeKind, targets := range filteredEdges {
				g := &xpb.EdgeSet_Group{Kind: edgeKind}
				for target := range targets {
					g.TargetTicket = append(g.TargetTicket, target)
					targetSet.Add(target)
				}
				groups = append(groups, g)
			}
			reply.EdgeSet = append(reply.EdgeSet, &xpb.EdgeSet{
				SourceTicket: ticket,
				Group:        groups,
			})

			// In addition, only add a NodeInfo if the filters have resulting facts.
			if len(filteredFacts) > 0 {
				reply.Node = append(reply.Node, &xpb.NodeInfo{
					Ticket: ticket,
					Fact:   filteredFacts,
				})
			}
		}
	}

	// Only request Nodes when there are fact filters given.
	if len(req.Filter) > 0 {
		// Ensure reply.Node is a unique set by removing already requested nodes from targetSet
		for _, n := range reply.Node {
			targetSet.Remove(n.Ticket)
		}

		// Batch request all leftover target nodes
		nodesReply, err := g.Nodes(ctx, &xpb.NodesRequest{Ticket: targetSet.Slice(), Filter: req.Filter})
		if err != nil {
			return nil, fmt.Errorf("failure getting target nodes: %v", err)
		}
		reply.Node = append(reply.Node, nodesReply.Node...)
	}

	return reply, nil
}

// Decorations implements part of the Service interface.
func (g *GraphStoreService) Decorations(ctx context.Context, req *xpb.DecorationsRequest) (*xpb.DecorationsReply, error) {
	if len(req.DirtyBuffer) > 0 {
		return nil, errors.New("UNIMPLEMENTED: dirty buffers")
	} else if req.GetLocation() == nil {
		// TODO(schroederc): allow empty location when given dirty buffer
		return nil, errors.New("missing location")
	}

	fileVName, err := kytheuri.ToVName(req.Location.Ticket)
	if err != nil {
		return nil, fmt.Errorf("invalid file ticket %q: %v", req.Location.Ticket, err)
	}

	text, encoding, err := getSourceText(ctx, g.gs, fileVName)
	if err != nil {
		return nil, fmt.Errorf("failed to retrieve file text: %v", err)
	}

	loc, err := xrefs.NewNormalizer(text).Location(req.GetLocation())
	if err != nil {
		return nil, err
	}

	reply := &xpb.DecorationsReply{Location: loc}

	// Handle DecorationsRequest.SourceText switch
	if req.SourceText {
		if loc.Kind == xpb.Location_FILE {
			reply.SourceText = text
		} else {
			reply.SourceText = text[loc.Start.ByteOffset:loc.End.ByteOffset]
		}
		reply.Encoding = encoding
	}

	// Handle DecorationsRequest.References switch
	if req.References {
		// Traverse the following chain of edges:
		//   file --%/kythe/edge/childof-> []anchor --forwardEdgeKind-> []target
		//
		// Add []anchor and []target nodes to reply.Node
		// Add all {anchor, forwardEdgeKind, target} tuples to reply.Reference

		children, err := getEdges(ctx, g.gs, fileVName, func(e *spb.Entry) bool {
			return e.EdgeKind == revChildOfEdgeKind
		})
		if err != nil {
			return nil, fmt.Errorf("failed to retrieve file children: %v", err)
		}

		targetSet := stringset.New()
		for _, edge := range children {
			anchor := edge.Target
			ticket := kytheuri.ToString(anchor)
			anchorNodeReply, err := g.Nodes(ctx, &xpb.NodesRequest{Ticket: []string{ticket}})
			if err != nil {
				return nil, fmt.Errorf("failure getting reference source node: %v", err)
			} else if len(anchorNodeReply.Node) != 1 {
				return nil, fmt.Errorf("found %d nodes for {%+v}", len(anchorNodeReply.Node), anchor)
			}

			node, ok := xrefs.NodesMap(anchorNodeReply.Node)[ticket]
			if !ok {
				return nil, fmt.Errorf("failed to find info for node %q", ticket)
			} else if string(node[schema.NodeKindFact]) != schema.AnchorKind {
				// Skip child if it isn't an anchor node
				continue
			} else if loc.Kind == xpb.Location_SPAN {
				// Check if anchor fits within request source text window

				anchorStart, err := strconv.Atoi(string(node[schema.AnchorStartFact]))
				if err != nil {
					log.Printf("Invalid anchor start offset %q for node %q: %v", node[schema.AnchorStartFact], ticket, err)
					continue
				} else if int32(anchorStart) < loc.Start.ByteOffset {
					continue
				}
				anchorEnd, err := strconv.Atoi(string(node[schema.AnchorEndFact]))
				if err != nil {
					log.Printf("Invalid anchor end offset %q for node %q: %v", node[schema.AnchorEndFact], ticket, err)
					continue
				} else if anchorStart > anchorEnd {
					log.Printf("Invalid anchor offset span %d:%d", anchorStart, anchorEnd)
					continue
				} else if int32(anchorEnd) > loc.End.ByteOffset {
					continue
				}
			}

			targets, err := getEdges(ctx, g.gs, anchor, func(e *spb.Entry) bool {
				return schema.EdgeDirection(e.EdgeKind) == schema.Forward && e.EdgeKind != schema.ChildOfEdge
			})
			if err != nil {
				return nil, fmt.Errorf("failed to retrieve targets of anchor %v: %v", anchor, err)
			}
			if len(targets) == 0 {
				log.Printf("Anchor missing forward edges: {%+v}", anchor)
				continue
			}

			reply.Node = append(reply.Node, anchorNodeReply.Node[0])
			for _, edge := range targets {
				targetTicket := kytheuri.ToString(edge.Target)
				targetSet.Add(targetTicket)
				reply.Reference = append(reply.Reference, &xpb.DecorationsReply_Reference{
					SourceTicket: ticket,
					Kind:         edge.Kind,
					TargetTicket: targetTicket,
				})
			}
		}
		sortReferences(reply)

		// Ensure returned nodes are not duplicated.
		for _, n := range reply.Node {
			targetSet.Remove(n.Ticket)
		}
		// Batch request all Reference target nodes
		nodesReply, err := g.Nodes(ctx, &xpb.NodesRequest{Ticket: targetSet.Slice()})
		if err != nil {
			return nil, fmt.Errorf("failure getting reference target nodes: %v", err)
		}
		reply.Node = append(reply.Node, nodesReply.Node...)
	}

	return reply, nil
}

func sortReferences(d *xpb.DecorationsReply) {
	refs := d.Reference
	spans := make([]struct{ start, end int }, len(refs))
	nodes := xrefs.NodesMap(d.Node)
	for i, r := range refs {
		if n, ok := nodes[r.SourceTicket]; ok {
			// Ignore errors; 0 works fine for sorting purposes
			start, _ := strconv.Atoi(string(n[schema.AnchorStartFact]))
			end, _ := strconv.Atoi(string(n[schema.AnchorEndFact]))
			spans[i] = struct{ start, end int }{start, end}
		}
	}
	sort.Sort(bySpan{refs, spans})
}

type bySpan struct {
	refs  []*xpb.DecorationsReply_Reference
	spans []struct{ start, end int }
}

func (s bySpan) Len() int { return len(s.refs) }
func (s bySpan) Swap(i, j int) {
	s.refs[i], s.refs[j] = s.refs[j], s.refs[i]
	s.spans[i], s.spans[j] = s.spans[j], s.spans[i]
}
func (s bySpan) Less(i, j int) bool {
	if s.spans[i].start < s.spans[j].start {
		return true
	} else if s.spans[i].start > s.spans[j].start {
		return false
	} else if s.spans[i].end < s.spans[j].end {
		return true
	}
	return false
}

var revChildOfEdgeKind = schema.MirrorEdge(schema.ChildOfEdge)

func getSourceText(ctx context.Context, gs graphstore.Service, fileVName *spb.VName) (text []byte, encoding string, err error) {
	if err := gs.Read(ctx, &spb.ReadRequest{Source: fileVName}, func(entry *spb.Entry) error {
		switch entry.FactName {
		case schema.TextFact:
			text = entry.FactValue
		case schema.TextEncodingFact:
			encoding = string(entry.FactValue)
		default:
			// skip other file facts
		}
		return nil
	}); err != nil {
		return nil, "", fmt.Errorf("read error: %v", err)
	}
	if text == nil {
		err = fmt.Errorf("file not found: %+v", fileVName)
	}
	return
}

type edgeTarget struct {
	Kind   string
	Target *spb.VName
}

// getEdges returns edgeTargets with the given node as their source.  Only edge
// entries that return true when applied to pred are returned.
func getEdges(ctx context.Context, gs graphstore.Service, node *spb.VName, pred func(*spb.Entry) bool) ([]*edgeTarget, error) {
	var targets []*edgeTarget

	if err := gs.Read(ctx, &spb.ReadRequest{
		Source:   node,
		EdgeKind: "*",
	}, func(entry *spb.Entry) error {
		if entry.EdgeKind != "" && pred(entry) {
			targets = append(targets, &edgeTarget{entry.EdgeKind, entry.Target})
		}
		return nil
	}); err != nil {
		return nil, fmt.Errorf("read error: %v", err)
	}
	return targets, nil
}

func entryToFact(entry *spb.Entry) *xpb.Fact {
	return &xpb.Fact{
		Name:  entry.FactName,
		Value: entry.FactValue,
	}
}
