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

// Package xrefs defines the xrefs Service interface and some useful utility
// functions.
package xrefs

import (
	"bytes"
	"fmt"
	"log"
	"net/http"
	"regexp"
	"sort"
	"time"

	"kythe.io/kythe/go/services/web"

	"golang.org/x/net/context"

	xpb "kythe.io/kythe/proto/xref_proto"
)

// Service provides access to a Kythe graph for fast access to cross-references.
type Service interface {
	NodesEdgesService
	DecorationsService
}

// NodesEdgesService provides fast access to nodes and edges in a Kythe graph.
type NodesEdgesService interface {
	NodesService
	EdgesService
}

// NodesService provides fast access to nodes in a Kythe graph.
type NodesService interface {
	// Nodes returns a subset of the facts for each of the requested nodes.
	Nodes(context.Context, *xpb.NodesRequest) (*xpb.NodesReply, error)
}

// EdgesService provides fast access to edges in a Kythe graph.
type EdgesService interface {
	// Edges returns a subset of the outbound edges for each of a set of requested
	// nodes.
	Edges(context.Context, *xpb.EdgesRequest) (*xpb.EdgesReply, error)
}

// DecorationsService provides fast access to file decorations in a Kythe graph.
type DecorationsService interface {
	// Decorations returns an index of the nodes and edges associated with a
	// particular file node.
	Decorations(context.Context, *xpb.DecorationsRequest) (*xpb.DecorationsReply, error)
}

// NodesMap returns a map from each node ticket to a map of its facts.
func NodesMap(nodes []*xpb.NodeInfo) map[string]map[string][]byte {
	m := make(map[string]map[string][]byte, len(nodes))
	for _, n := range nodes {
		facts := make(map[string][]byte, len(n.Fact))
		for _, f := range n.Fact {
			facts[f.Name] = f.Value
		}
		m[n.Ticket] = facts
	}
	return m
}

// EdgesMap returns a map from each node ticket to a map of its outward edge kinds.
func EdgesMap(edges []*xpb.EdgeSet) map[string]map[string][]string {
	m := make(map[string]map[string][]string, len(edges))
	for _, es := range edges {
		kinds := make(map[string][]string, len(es.Group))
		for _, g := range es.Group {
			kinds[g.Kind] = g.TargetTicket
		}
		m[es.SourceTicket] = kinds
	}
	return m
}

// Normalizer fixes xref.Locations within a given source text so that each point
// has consistent byte_offset, line_number, and column_offset fields within the
// range of text's length and its line lengths.
type Normalizer struct {
	textLen   int32
	lineLen   []int32
	prefixLen []int32
}

// NewNormalizer returns a Normalizer for Locations within text.
func NewNormalizer(text []byte) *Normalizer {
	lines := bytes.Split(text, lineEnd)
	lineLen := make([]int32, len(lines))
	prefixLen := make([]int32, len(lines))
	for i := 1; i < len(lines); i++ {
		lineLen[i-1] = int32(len(lines[i-1]) + len(lineEnd))
		prefixLen[i] = prefixLen[i-1] + lineLen[i-1]
	}
	lineLen[len(lines)-1] = int32(len(lines[len(lines)-1]) + len(lineEnd))
	return &Normalizer{int32(len(text)), lineLen, prefixLen}
}

// Location returns a normalized location within the Normalizer's text.
// Normalized FILE locations have no start/end points.  Normalized SPAN
// locations have fully populated start/end points clamped in the range [0,
// len(text)).
func (n *Normalizer) Location(loc *xpb.Location) (*xpb.Location, error) {
	nl := &xpb.Location{}
	if loc == nil {
		return nl, nil
	}
	nl.Ticket = loc.Ticket
	nl.Kind = loc.Kind
	if loc.Kind == xpb.Location_FILE {
		return nl, nil
	}

	nl.Start = n.Point(loc.Start)
	nl.End = n.Point(loc.End)

	start, end := nl.Start.ByteOffset, nl.End.ByteOffset
	if start > end {
		return nil, fmt.Errorf("invalid SPAN: start (%d) is after end (%d)", start, end)
	}
	return nl, nil
}

var lineEnd = []byte("\n")

// Point returns a normalized point within the Normalizer's text.  A normalized
// point has all of its fields set consistently and clamped within the range
// [0,len(text)).
func (n *Normalizer) Point(p *xpb.Location_Point) *xpb.Location_Point {
	np := &xpb.Location_Point{}

	if p == nil {
		return np
	} else if p.ByteOffset > 0 {
		np.ByteOffset = p.ByteOffset
		if np.ByteOffset > n.textLen {
			np.ByteOffset = n.textLen
		}

		np.LineNumber = int32(sort.Search(len(n.lineLen), func(i int) bool {
			return n.prefixLen[i] > np.ByteOffset
		}))
		np.ColumnOffset = np.ByteOffset - n.prefixLen[np.LineNumber-1]
	} else if p.LineNumber > 0 {
		np.LineNumber = p.LineNumber
		np.ColumnOffset = p.ColumnOffset

		if totalLines := int32(len(n.lineLen)); p.LineNumber > totalLines {
			np.LineNumber = totalLines
		}
		if p.ColumnOffset < 0 {
			np.ColumnOffset = 0
		} else if p.ColumnOffset > 0 {
			if lineLen := n.lineLen[np.LineNumber-1]; p.ColumnOffset > lineLen {
				np.ColumnOffset = lineLen
			}
		}

		np.ByteOffset = n.prefixLen[np.LineNumber-1] + np.ColumnOffset
	}

	return np
}

// ConvertFilters converts each filter glob into an equivalent regexp.
func ConvertFilters(filters []string) []*regexp.Regexp {
	var patterns []*regexp.Regexp
	for _, filter := range filters {
		patterns = append(patterns, filterToRegexp(filter))
	}
	return patterns
}

var filterOpsRE = regexp.MustCompile("[*][*]|[*?]")

func filterToRegexp(pattern string) *regexp.Regexp {
	var re string
	for {
		loc := filterOpsRE.FindStringIndex(pattern)
		if loc == nil {
			break
		}
		re += regexp.QuoteMeta(pattern[:loc[0]])
		switch pattern[loc[0]:loc[1]] {
		case "**":
			re += ".*"
		case "*":
			re += "[^/]*"
		case "?":
			re += "[^/]"
		default:
			log.Fatal("Unknown filter operator: " + pattern[loc[0]:loc[1]])
		}
		pattern = pattern[loc[1]:]
	}
	return regexp.MustCompile(re + regexp.QuoteMeta(pattern))
}

// MatchesAny reports whether if str matches any of the patterns
func MatchesAny(str string, patterns []*regexp.Regexp) bool {
	for _, p := range patterns {
		if p.MatchString(str) {
			return true
		}
	}
	return false
}

type grpcClient struct{ xpb.XRefServiceClient }

// Nodes implements part of the Service interface.
func (w *grpcClient) Nodes(ctx context.Context, req *xpb.NodesRequest) (*xpb.NodesReply, error) {
	return w.XRefServiceClient.Nodes(ctx, req)
}

// Edges implements part of the Service interface.
func (w *grpcClient) Edges(ctx context.Context, req *xpb.EdgesRequest) (*xpb.EdgesReply, error) {
	return w.XRefServiceClient.Edges(ctx, req)
}

// Decorations implements part of the Service interface.
func (w *grpcClient) Decorations(ctx context.Context, req *xpb.DecorationsRequest) (*xpb.DecorationsReply, error) {
	return w.XRefServiceClient.Decorations(ctx, req)
}

// GRPC returns an xrefs Service backed by the given GRPC client and context.
func GRPC(c xpb.XRefServiceClient) Service { return &grpcClient{c} }

type webClient struct{ addr string }

// Nodes implements part of the Service interface.
func (w *webClient) Nodes(ctx context.Context, q *xpb.NodesRequest) (*xpb.NodesReply, error) {
	var reply xpb.NodesReply
	return &reply, web.Call(w.addr, "nodes", q, &reply)
}

// Edges implements part of the Service interface.
func (w *webClient) Edges(ctx context.Context, q *xpb.EdgesRequest) (*xpb.EdgesReply, error) {
	var reply xpb.EdgesReply
	return &reply, web.Call(w.addr, "edges", q, &reply)
}

// Decorations implements part of the Service interface.
func (w *webClient) Decorations(ctx context.Context, q *xpb.DecorationsRequest) (*xpb.DecorationsReply, error) {
	var reply xpb.DecorationsReply
	return &reply, web.Call(w.addr, "decorations", q, &reply)
}

// WebClient returns an xrefs Service based on a remote web server.
func WebClient(addr string) Service {
	return &webClient{addr}
}

// RegisterHTTPHandlers registers JSON HTTP handlers with mux using the given
// xrefs Service.  The following methods with be exposed:
//
//   GET /nodes
//     Request: JSON encoded xrefs.NodesRequest
//     Response: JSON encoded xrefs.NodesResponse
//   GET /edges
//     Request: JSON encoded xrefs.EdgesRequest
//     Response: JSON encoded xrefs.EdgesResponse
//   GET /decorations
//     Request: JSON encoded xrefs.DecorationsRequest
//     Response: JSON encoded xrefs.DecorationsResponse
//
// Note: /nodes, /edges, and /decorations will return their responses as
// serialized protobufs if the "proto" query parameter is set.
func RegisterHTTPHandlers(ctx context.Context, xs Service, mux *http.ServeMux) {
	mux.HandleFunc("/decorations", func(w http.ResponseWriter, r *http.Request) {
		start := time.Now()
		defer func() {
			log.Printf("xrefs.Decorations:\t%s", time.Since(start))
		}()
		var req xpb.DecorationsRequest
		if err := web.ReadJSONBody(r, &req); err != nil {
			http.Error(w, err.Error(), http.StatusBadRequest)
			return
		}
		reply, err := xs.Decorations(ctx, &req)
		if err != nil {
			http.Error(w, err.Error(), http.StatusInternalServerError)
			return
		}

		if err := web.WriteResponse(w, r, reply); err != nil {
			log.Println(err)
		}
	})
	mux.HandleFunc("/nodes", func(w http.ResponseWriter, r *http.Request) {
		start := time.Now()
		defer func() {
			log.Printf("xrefs.Nodes:\t%s", time.Since(start))
		}()

		var req xpb.NodesRequest
		if err := web.ReadJSONBody(r, &req); err != nil {
			http.Error(w, err.Error(), http.StatusBadRequest)
			return
		}
		reply, err := xs.Nodes(ctx, &req)
		if err != nil {
			http.Error(w, err.Error(), http.StatusInternalServerError)
			return
		}
		if err := web.WriteResponse(w, r, reply); err != nil {
			log.Println(err)
		}
	})
	mux.HandleFunc("/edges", func(w http.ResponseWriter, r *http.Request) {
		start := time.Now()
		defer func() {
			log.Printf("xrefs.Edges:\t%s", time.Since(start))
		}()

		var req xpb.EdgesRequest
		if err := web.ReadJSONBody(r, &req); err != nil {
			http.Error(w, err.Error(), http.StatusBadRequest)
			return
		}
		reply, err := xs.Edges(ctx, &req)
		if err != nil {
			http.Error(w, err.Error(), http.StatusInternalServerError)
			return
		}
		if err := web.WriteResponse(w, r, reply); err != nil {
			log.Println(err)
		}
	})
}
