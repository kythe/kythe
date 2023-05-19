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

// Package graph defines the graph Service interface and some useful utility
// functions.
package graph // import "kythe.io/kythe/go/services/graph"

import (
	"context"
	"fmt"
	"math"
	"net/http"
	"sort"
	"time"

	"kythe.io/kythe/go/services/web"
	"kythe.io/kythe/go/util/log"

	cpb "kythe.io/kythe/proto/common_go_proto"
	gpb "kythe.io/kythe/proto/graph_go_proto"
)

// Service exposes direct access to nodes and edges in a Kythe graph.
type Service interface {
	Nodes(context.Context, *gpb.NodesRequest) (*gpb.NodesReply, error)
	Edges(context.Context, *gpb.EdgesRequest) (*gpb.EdgesReply, error)
}

// AllEdges returns all edges for a particular EdgesRequest.  This means that
// the returned reply will not have a next page token.  WARNING: the paging API
// exists for a reason; using this can lead to very large memory consumption
// depending on the request.
func AllEdges(ctx context.Context, es Service, req *gpb.EdgesRequest) (*gpb.EdgesReply, error) {
	req.PageSize = math.MaxInt32
	reply, err := es.Edges(ctx, req)
	if err != nil || reply.NextPageToken == "" {
		return reply, err
	}

	nodes, edges := NodesMap(reply.Nodes), EdgesMap(reply.EdgeSets)

	for reply.NextPageToken != "" && err == nil {
		req.PageToken = reply.NextPageToken
		reply, err = es.Edges(ctx, req)
		if err != nil {
			return nil, err
		}
		nodesMapInto(reply.Nodes, nodes)
		edgesMapInto(reply.EdgeSets, edges)
	}

	reply = &gpb.EdgesReply{
		Nodes:    make(map[string]*cpb.NodeInfo, len(nodes)),
		EdgeSets: make(map[string]*gpb.EdgeSet, len(edges)),
	}

	for ticket, facts := range nodes {
		info := &cpb.NodeInfo{
			Facts: make(map[string][]byte, len(facts)),
		}
		for name, val := range facts {
			info.Facts[name] = val
		}
		reply.Nodes[ticket] = info
	}

	for source, groups := range edges {
		set := &gpb.EdgeSet{
			Groups: make(map[string]*gpb.EdgeSet_Group, len(groups)),
		}
		for kind, targets := range groups {
			edges := make([]*gpb.EdgeSet_Group_Edge, 0, len(targets))
			for target, ordinals := range targets {
				for ordinal := range ordinals {
					edges = append(edges, &gpb.EdgeSet_Group_Edge{
						TargetTicket: target,
						Ordinal:      ordinal,
					})
				}
			}
			sort.Sort(ByOrdinal(edges))
			set.Groups[kind] = &gpb.EdgeSet_Group{
				Edge: edges,
			}
		}
		reply.EdgeSets[source] = set
	}

	return reply, err
}

// BoundedRequests guards against requests for more tickets than allowed per
// the MaxTickets configuration.
type BoundedRequests struct {
	MaxTickets int
	Service
}

// Nodes implements part of the Service interface.
func (b BoundedRequests) Nodes(ctx context.Context, req *gpb.NodesRequest) (*gpb.NodesReply, error) {
	if len(req.Ticket) > b.MaxTickets {
		return nil, fmt.Errorf("too many tickets requested: %d (max %d)", len(req.Ticket), b.MaxTickets)
	}
	return b.Service.Nodes(ctx, req)
}

// Edges implements part of the Service interface.
func (b BoundedRequests) Edges(ctx context.Context, req *gpb.EdgesRequest) (*gpb.EdgesReply, error) {
	if len(req.Ticket) > b.MaxTickets {
		return nil, fmt.Errorf("too many tickets requested: %d (max %d)", len(req.Ticket), b.MaxTickets)
	}
	return b.Service.Edges(ctx, req)
}

type webClient struct{ addr string }

// Nodes implements part of the Service interface.
func (w *webClient) Nodes(ctx context.Context, q *gpb.NodesRequest) (*gpb.NodesReply, error) {
	var reply gpb.NodesReply
	return &reply, web.Call(w.addr, "nodes", q, &reply)
}

// Edges implements part of the Service interface.
func (w *webClient) Edges(ctx context.Context, q *gpb.EdgesRequest) (*gpb.EdgesReply, error) {
	var reply gpb.EdgesReply
	return &reply, web.Call(w.addr, "edges", q, &reply)
}

// WebClient returns a graph Service based on a remote web server.
func WebClient(addr string) Service {
	return &webClient{addr}
}

// RegisterHTTPHandlers registers JSON HTTP handlers with mux using the given
// graph Service.  The following methods with be exposed:
//
//	GET /nodes
//	  Request: JSON encoded graph.NodesRequest
//	  Response: JSON encoded graph.NodesReply
//	GET /edges
//	  Request: JSON encoded graph.EdgesRequest
//	  Response: JSON encoded graph.EdgesReply
//
// Note: /nodes, and /edges will return their responses as serialized protobufs
// if the "proto" query parameter is set.
func RegisterHTTPHandlers(ctx context.Context, gs Service, mux *http.ServeMux) {
	mux.HandleFunc("/nodes", func(w http.ResponseWriter, r *http.Request) {
		start := time.Now()
		defer func() {
			log.Infof("graph.Nodes:\t%s", time.Since(start))
		}()

		var req gpb.NodesRequest
		if err := web.ReadJSONBody(r, &req); err != nil {
			http.Error(w, err.Error(), http.StatusBadRequest)
			return
		}
		reply, err := gs.Nodes(ctx, &req)
		if err != nil {
			http.Error(w, err.Error(), http.StatusInternalServerError)
			return
		}
		if err := web.WriteResponse(w, r, reply); err != nil {
			log.Info(err)
		}
	})
	mux.HandleFunc("/edges", func(w http.ResponseWriter, r *http.Request) {
		start := time.Now()
		defer func() {
			log.Infof("graph.Edges:\t%s", time.Since(start))
		}()

		var req gpb.EdgesRequest
		if err := web.ReadJSONBody(r, &req); err != nil {
			http.Error(w, err.Error(), http.StatusBadRequest)
			return
		}
		reply, err := gs.Edges(ctx, &req)
		if err != nil {
			http.Error(w, err.Error(), http.StatusInternalServerError)
			return
		}
		if err := web.WriteResponse(w, r, reply); err != nil {
			log.Info(err)
		}
	})
}

// NodesMap returns a map from each node ticket to a map of its facts.
func NodesMap(nodes map[string]*cpb.NodeInfo) map[string]map[string][]byte {
	m := make(map[string]map[string][]byte, len(nodes))
	nodesMapInto(nodes, m)
	return m
}

func nodesMapInto(nodes map[string]*cpb.NodeInfo, m map[string]map[string][]byte) {
	for ticket, n := range nodes {
		facts, ok := m[ticket]
		if !ok {
			facts = make(map[string][]byte, len(n.Facts))
			m[ticket] = facts
		}
		for name, value := range n.Facts {
			facts[name] = value
		}
	}
}

// EdgesMap returns a map from each node ticket to a map of its outward edge kinds.
func EdgesMap(edges map[string]*gpb.EdgeSet) map[string]map[string]map[string]map[int32]struct{} {
	m := make(map[string]map[string]map[string]map[int32]struct{}, len(edges))
	edgesMapInto(edges, m)
	return m
}

func edgesMapInto(edges map[string]*gpb.EdgeSet, m map[string]map[string]map[string]map[int32]struct{}) {
	for source, es := range edges {
		kinds, ok := m[source]
		if !ok {
			kinds = make(map[string]map[string]map[int32]struct{}, len(es.Groups))
			m[source] = kinds
		}
		for kind, g := range es.Groups {
			for _, e := range g.Edge {
				targets, ok := kinds[kind]
				if !ok {
					targets = make(map[string]map[int32]struct{})
					kinds[kind] = targets
				}
				ordinals, ok := targets[e.TargetTicket]
				if !ok {
					ordinals = make(map[int32]struct{})
					targets[e.TargetTicket] = ordinals
				}
				ordinals[e.Ordinal] = struct{}{}
			}
		}
	}
}

// ByOrdinal orders the edges in an edge group by their ordinals, with ties
// broken by ticket.
type ByOrdinal []*gpb.EdgeSet_Group_Edge

func (s ByOrdinal) Len() int      { return len(s) }
func (s ByOrdinal) Swap(i, j int) { s[i], s[j] = s[j], s[i] }
func (s ByOrdinal) Less(i, j int) bool {
	if s[i].Ordinal == s[j].Ordinal {
		return s[i].TargetTicket < s[j].TargetTicket
	}
	return s[i].Ordinal < s[j].Ordinal
}
