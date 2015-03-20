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

package sql

import (
	"errors"
	"fmt"

	"kythe.io/kythe/go/services/xrefs"
	"kythe.io/kythe/go/util/kytheuri"
	"kythe.io/kythe/go/util/schema"
	"kythe.io/kythe/go/util/stringset"

	xpb "kythe.io/kythe/proto/xref_proto"
)

// Nodes implements part of the xrefs.Service interface.
func (db *DB) Nodes(req *xpb.NodesRequest) (*xpb.NodesReply, error) {
	reply := &xpb.NodesReply{}
	patterns := xrefs.ConvertFilters(req.Filter)
	for _, t := range req.Ticket {
		uri, err := kytheuri.Parse(t)
		if err != nil {
			return nil, fmt.Errorf("invalid ticket %q: %v", t, err)
		}

		rows, err := db.nodeFactsStmt.Query(uri.Signature, uri.Corpus, uri.Root, uri.Language, uri.Path)
		if err != nil {
			return nil, fmt.Errorf("sql select error for node %q: %v", t, err)
		}

		var facts []*xpb.Fact
		for rows.Next() {
			var fact xpb.Fact
			err = rows.Scan(&fact.Name, &fact.Value)
			if err != nil {
				rows.Close()
				return nil, fmt.Errorf("sql scan error for node %q: %v", t, err)
			}
			if len(patterns) == 0 || xrefs.MatchesAny(fact.Name, patterns) {
				facts = append(facts, &fact)
			}
		}
		if err := rows.Close(); err != nil {
			return nil, fmt.Errorf("error closing rows for node %q: %v", t, err)
		}

		if len(facts) != 0 {
			reply.Node = append(reply.Node, &xpb.NodeInfo{
				Ticket: t,
				Fact:   facts,
			})
		}
	}
	return reply, nil
}

// Edges implements part of the xrefs.Service interface.
func (db *DB) Edges(req *xpb.EdgesRequest) (*xpb.EdgesReply, error) {
	if req.PageSize != 0 || req.PageToken != "" {
		return nil, errors.New("edge pages unimplemented for SQL DB")
	}
	reply := &xpb.EdgesReply{}

	allowedKinds := stringset.New(req.Kind...)

	nodeTickets := stringset.New()
	for _, t := range req.Ticket {
		uri, err := kytheuri.Parse(t)
		if err != nil {
			return nil, err
		}

		edges := make(map[string]stringset.Set)
		var (
			target kytheuri.URI
			kind   string
		)

		rows, err := db.edgesStmt.Query(uri.Signature, uri.Corpus, uri.Root, uri.Path, uri.Language)
		if err != nil {
			rows.Close()
			return nil, fmt.Errorf("forward edges query error: %v", err)
		}
		for rows.Next() {
			if err := rows.Scan(&target.Signature, &target.Corpus, &target.Root, &target.Path, &target.Language, &kind); err != nil {
				rows.Close()
				return nil, fmt.Errorf("forward edges scan error: %v", err)
			} else if len(allowedKinds) != 0 && !allowedKinds.Contains(kind) {
				continue
			}

			targets, ok := edges[kind]
			if !ok {
				targets = stringset.New()
				edges[kind] = targets
			}

			ticket := target.String()
			targets.Add(ticket)
			nodeTickets.Add(ticket)
		}
		if err := rows.Close(); err != nil {
			return nil, err
		}

		rows, err = db.revEdgesStmt.Query(uri.Signature, uri.Corpus, uri.Root, uri.Path, uri.Language)
		if err != nil {
			rows.Close()
			return nil, fmt.Errorf("reverse edges query error: %v", err)
		}
		for rows.Next() {
			if err := rows.Scan(&target.Signature, &target.Corpus, &target.Root, &target.Path, &target.Language, &kind); err != nil {
				rows.Close()
				return nil, fmt.Errorf("reverse edges scan error: %v", err)
			}

			kind = schema.MirrorEdge(kind)
			if len(allowedKinds) != 0 && !allowedKinds.Contains(kind) {
				continue
			}

			targets, ok := edges[kind]
			if !ok {
				targets = stringset.New()
				edges[kind] = targets
			}

			ticket := target.String()
			targets.Add(ticket)
			nodeTickets.Add(ticket)
		}
		if err := rows.Close(); err != nil {
			return nil, err
		}

		var g []*xpb.EdgeSet_Group
		for kind, targets := range edges {
			g = append(g, &xpb.EdgeSet_Group{
				Kind:         kind,
				TargetTicket: targets.Slice(),
			})
		}

		if len(g) != 0 {
			reply.EdgeSet = append(reply.EdgeSet, &xpb.EdgeSet{
				SourceTicket: t,
				Group:        g,
			})
		}
	}

	nodesReply, err := db.Nodes(&xpb.NodesRequest{
		Ticket: nodeTickets.Slice(),
		Filter: req.Filter,
	})
	if err != nil {
		return nil, fmt.Errorf("nodes request error: %v", err)
	}
	reply.Node = nodesReply.Node
	return reply, nil
}

var revChildOfEdge = schema.MirrorEdge(schema.ChildOfEdge)

// Decorations implements part of the xrefs.Service interface.
func (db *DB) Decorations(req *xpb.DecorationsRequest) (*xpb.DecorationsReply, error) {
	if req.GetLocation() == nil {
		return nil, errors.New("missing location")
	} else if req.Location.Kind != xpb.Location_FILE {
		return nil, fmt.Errorf("%s location kind unimplemented", req.Location.Kind)
	} else if len(req.DirtyBuffer) != 0 {
		return nil, errors.New("dirty buffers unimplemented")
	}

	fileTicket := req.Location.Ticket

	edgesReply, err := db.Edges(&xpb.EdgesRequest{
		Ticket: []string{fileTicket},
		Kind:   []string{revChildOfEdge},
		Filter: []string{schema.NodeKindFact, schema.TextFact, schema.TextEncodingFact},
	})
	if err != nil {
		return nil, err
	}

	reply := &xpb.DecorationsReply{
		Location: &xpb.Location{
			Ticket: fileTicket,
		},
	}

	nodes := xrefs.NodesMap(edgesReply.Node)

	if req.SourceText {
		if nodes[fileTicket] == nil {
			nodesReply, err := db.Nodes(&xpb.NodesRequest{Ticket: []string{fileTicket}})
			if err != nil {
				return nil, err
			}
			nodes = xrefs.NodesMap(nodesReply.Node)
		}
		reply.SourceText = nodes[fileTicket][schema.TextFact]
		reply.Encoding = string(nodes[fileTicket][schema.TextEncodingFact])
	}

	nodeTickets := stringset.New()

	if req.References {
		// Traverse the following chain of edges:
		//   file --%/kythe/edge/childof-> []anchor --forwardEdgeKind-> []target
		edges := xrefs.EdgesMap(edgesReply.EdgeSet)

		for _, anchor := range edges[fileTicket][revChildOfEdge] {
			if string(nodes[anchor][schema.NodeKindFact]) != schema.AnchorKind {
				continue
			}

			uri, err := kytheuri.Parse(anchor)
			if err != nil {
				return nil, fmt.Errorf("invalid anchor ticket found %q: %v", anchor, err)
			}
			rows, err := db.anchorEdgesStmt.Query(schema.ChildOfEdge, uri.Signature, uri.Corpus, uri.Root, uri.Path, uri.Language)
			if err != nil {
				return nil, fmt.Errorf("anchor %q edge query error: %v", anchor, err)
			}

			for rows.Next() {
				var target kytheuri.URI
				var kind string
				if err := rows.Scan(&target.Signature, &target.Corpus, &target.Root, &target.Path, &target.Language, &kind); err != nil {
					return nil, fmt.Errorf("anchor %q edge scan error: %v", anchor, err)
				}
				ticket := target.String()
				reply.Reference = append(reply.Reference, &xpb.DecorationsReply_Reference{
					SourceTicket: anchor,
					Kind:         kind,
					TargetTicket: ticket,
				})
				nodeTickets.Add(anchor, ticket)
			}
		}
	}

	nodesReply, err := db.Nodes(&xpb.NodesRequest{Ticket: nodeTickets.Slice()})
	if err != nil {
		return nil, err
	}
	reply.Node = nodesReply.Node

	return reply, nil
}
