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
	"log"
	"time"

	"kythe/go/services/graphstore"
	"kythe/go/services/xrefs"
	"kythe/go/util/kytheuri"
	"kythe/go/util/schema"
	"kythe/go/util/stringset"

	spb "kythe/proto/storage_proto"
	xpb "kythe/proto/xref_proto"

	"code.google.com/p/goprotobuf/proto"
)

// AddReverseEdges scans gs for all forward edges, adding a reverse for each
// back into the GraphStore.  This is necessary for a GraphStoreService to work
// properly.
func AddReverseEdges(gs graphstore.Service) error {
	log.Println("Adding reverse edges")
	var (
		totalEntries int
		addedEdges   int
	)
	startTime := time.Now()
	err := graphstore.EachScanEntry(gs, nil, func(entry *spb.Entry) error {
		kind := entry.GetEdgeKind()
		if kind != "" && schema.EdgeDirection(kind) == schema.Forward {
			if err := gs.Write(&spb.WriteRequest{
				Source: entry.Target,
				Update: []*spb.WriteRequest_Update{{
					Target:    entry.Source,
					EdgeKind:  proto.String(schema.MirrorEdge(kind)),
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

// A GraphStoreService partially implements the xrefs.Service interface directly
// using a GraphStore Service with stored reverse edges.  This is a
// low-performance, simple alternative to creating the serving Table
// representation.
// TODO(schroederc): parallelize GraphStore calls
type GraphStoreService struct {
	gs graphstore.Service
}

// NewGraphStoreService returns a new GraphStoreService given an
// existing GraphStore.
func NewGraphStoreService(gs graphstore.Service) *GraphStoreService {
	return &GraphStoreService{gs}
}

// Nodes implements part of the Service interface.
func (g *GraphStoreService) Nodes(req *xpb.NodesRequest) (*xpb.NodesReply, error) {
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
		info := &xpb.NodeInfo{Ticket: &req.Ticket[i]}
		if err := graphstore.EachReadEntry(g.gs, &spb.ReadRequest{Source: vname}, func(entry *spb.Entry) error {
			if len(patterns) == 0 || xrefs.MatchesAny(entry.GetFactName(), patterns) {
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
func (g *GraphStoreService) Edges(req *xpb.EdgesRequest) (*xpb.EdgesReply, error) {
	if req.GetPageToken() != "" {
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

		if err := graphstore.EachReadEntry(g.gs, &spb.ReadRequest{
			Source:   vname,
			EdgeKind: proto.String("*"),
		}, func(entry *spb.Entry) error {
			edgeKind := entry.GetEdgeKind()
			if edgeKind == "" {
				// node fact
				if len(patterns) > 0 && xrefs.MatchesAny(entry.GetFactName(), patterns) {
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
				g := &xpb.EdgeSet_Group{Kind: proto.String(edgeKind)}
				for target := range targets {
					g.TargetTicket = append(g.TargetTicket, target)
					targetSet.Add(target)
				}
				groups = append(groups, g)
			}
			reply.EdgeSet = append(reply.EdgeSet, &xpb.EdgeSet{
				SourceTicket: proto.String(ticket),
				Group:        groups,
			})

			// In addition, only add a NodeInfo if the filters have resulting facts.
			if len(filteredFacts) > 0 {
				reply.Node = append(reply.Node, &xpb.NodeInfo{
					Ticket: proto.String(ticket),
					Fact:   filteredFacts,
				})
			}
		}
	}

	// Ensure reply.Node is a unique set by removing already requested nodes from targetSet
	for _, n := range reply.Node {
		targetSet.Remove(n.GetTicket())
	}

	// Batch request all leftover target nodes
	nodesReply, err := g.Nodes(&xpb.NodesRequest{Ticket: targetSet.Slice(), Filter: req.Filter})
	if err != nil {
		return nil, fmt.Errorf("failure getting target nodes: %v", err)
	}
	reply.Node = append(reply.Node, nodesReply.Node...)

	return reply, nil
}

// Decorations implements part of the Service interface.
func (g *GraphStoreService) Decorations(req *xpb.DecorationsRequest) (*xpb.DecorationsReply, error) {
	if len(req.DirtyBuffer) > 0 {
		return nil, errors.New("UNIMPLEMENTED: dirty buffers")
	} else if req.Location.GetKind() != xpb.Location_FILE {
		return nil, errors.New("UNIMPLEMENTED: non-FILE locations")
	}

	fileVName, err := kytheuri.ToVName(req.Location.GetTicket())
	if err != nil {
		return nil, fmt.Errorf("invalid file ticket %q: %v", req.Location.GetTicket(), err)
	}

	reply := &xpb.DecorationsReply{Location: req.Location}

	// Handle DecorationsRequest.SourceText switch
	if req.GetSourceText() {
		text, encoding, err := getSourceText(g.gs, fileVName)
		if err != nil {
			return nil, fmt.Errorf("failed to retrieve file text: %v", err)
		}
		reply.SourceText = text
		reply.Encoding = &encoding
	}

	// Handle DecorationsRequest.References switch
	if req.GetReferences() {
		// Traverse the following chain of edges:
		//   file --%/kythe/edge/childof-> []anchor --forwardEdgeKind-> []target
		//
		// Add []anchor and []target nodes to reply.Node
		// Add all {anchor, forwardEdgeKind, target} tuples to reply.Reference

		children, err := getEdges(g.gs, fileVName, func(e *spb.Entry) bool {
			return e.GetEdgeKind() == revChildOfEdgeKind
		})
		if err != nil {
			return nil, fmt.Errorf("failed to retrieve file children: %v", err)
		}

		targetSet := stringset.New()
		for _, edge := range children {
			anchor := edge.Target
			anchorNodeReply, err := g.Nodes(&xpb.NodesRequest{Ticket: []string{kytheuri.ToString(anchor)}})
			if err != nil {
				return nil, fmt.Errorf("failure getting reference source node: %v", err)
			} else if len(anchorNodeReply.Node) != 1 {
				return nil, fmt.Errorf("found %d nodes for {%+v}", len(anchorNodeReply.Node), anchor)
			} else if infoNodeKind(anchorNodeReply.Node[0]) != schema.AnchorKind {
				// Skip child if it isn't an anchor node
				continue
			}
			reply.Node = append(reply.Node, anchorNodeReply.Node[0])

			targets, err := getEdges(g.gs, anchor, func(e *spb.Entry) bool {
				return schema.EdgeDirection(e.GetEdgeKind()) == schema.Forward && e.GetEdgeKind() != schema.ChildOfEdge
			})
			if err != nil {
				return nil, fmt.Errorf("failed to retrieve targets of anchor %v: %v", anchor, err)
			}
			if len(targets) == 0 {
				log.Printf("Anchor missing forward edges: {%+v}", anchor)
			}
			for _, edge := range targets {
				targetTicket := kytheuri.ToString(edge.Target)
				targetSet.Add(targetTicket)
				reply.Reference = append(reply.Reference, &xpb.DecorationsReply_Reference{
					SourceTicket: proto.String(kytheuri.ToString(anchor)),
					Kind:         proto.String(edge.Kind),
					TargetTicket: proto.String(targetTicket),
				})
			}
		}

		// Batch request all Reference target nodes
		nodesReply, err := g.Nodes(&xpb.NodesRequest{Ticket: targetSet.Slice()})
		if err != nil {
			return nil, fmt.Errorf("failure getting reference target nodes: %v", err)
		}
		reply.Node = append(reply.Node, nodesReply.Node...)
	}

	return reply, nil
}

var revChildOfEdgeKind = schema.MirrorEdge(schema.ChildOfEdge)

func infoNodeKind(info *xpb.NodeInfo) string {
	for _, fact := range info.Fact {
		if fact.GetName() == schema.NodeKindFact {
			return string(fact.Value)
		}
	}
	return ""
}

func getSourceText(gs graphstore.Service, fileVName *spb.VName) (text []byte, encoding string, err error) {
	if err := graphstore.EachReadEntry(gs, &spb.ReadRequest{
		Source: fileVName,
	}, func(entry *spb.Entry) error {
		switch entry.GetFactName() {
		case schema.FileTextFact:
			text = entry.GetFactValue()
		case schema.FileEncodingFact:
			encoding = string(entry.GetFactValue())
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
func getEdges(gs graphstore.Service, node *spb.VName, pred func(*spb.Entry) bool) ([]*edgeTarget, error) {
	var targets []*edgeTarget

	if err := graphstore.EachReadEntry(gs, &spb.ReadRequest{
		Source:   node,
		EdgeKind: proto.String("*"),
	}, func(entry *spb.Entry) error {
		if entry.GetEdgeKind() != "" && pred(entry) {
			targets = append(targets, &edgeTarget{entry.GetEdgeKind(), entry.Target})
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
