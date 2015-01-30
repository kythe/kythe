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
	"fmt"
	"log"
	"net/http"
	"regexp"
	"strconv"
	"time"

	"kythe/go/services/web"
	"kythe/go/util/schema"
	"kythe/go/util/stringset"

	xpb "kythe/proto/xref_proto"
)

// Service provides access to a Kythe graph for fast access to cross-references.
type Service interface {
	NodesService
	EdgesService
	DecorationsService
}

// NodesService provides fast access to nodes in a Kythe graph.
type NodesService interface {
	// Nodes returns a subset of the facts for each of the requested nodes.
	Nodes(*xpb.NodesRequest) (*xpb.NodesReply, error)
}

// EdgesService provides fast access to edges in a Kythe graph.
type EdgesService interface {
	// Edges returns a subset of the outbound edges for each of a set of requested
	// nodes.
	Edges(*xpb.EdgesRequest) (*xpb.EdgesReply, error)
}

// DecorationsService provides fast access to file decorations in a Kythe graph.
type DecorationsService interface {
	// Decorations returns an index of the nodes and edges associated with a
	// particular file node.
	Decorations(*xpb.DecorationsRequest) (*xpb.DecorationsReply, error)
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
//   GET /xrefs?ticket=<ticket>
//     Response: JSON map from edgeKind to a set of anchor/file locations that
//               attach to the given node.
//
// Note: /nodes, /edges, and /decorations will return their responses as
// serialized protobufs if the "proto" query parameter is set.
func RegisterHTTPHandlers(xs Service, mux *http.ServeMux) {
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
		reply, err := xs.Decorations(&req)
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
		reply, err := xs.Nodes(&req)
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
		reply, err := xs.Edges(&req)
		if err != nil {
			http.Error(w, err.Error(), http.StatusInternalServerError)
			return
		}
		if err := web.WriteResponse(w, r, reply); err != nil {
			log.Println(err)
		}
	})
	mux.HandleFunc("/xrefs", func(w http.ResponseWriter, r *http.Request) {
		startTime := time.Now()
		ticket := web.Arg(r, "ticket")
		if ticket == "" {
			http.Error(w, "Bad Request: missing target parameter", http.StatusBadRequest)
			return
		}

		refs, total, err := XRefs(xs, ticket, true)
		if err != nil {
			http.Error(w, err.Error(), http.StatusInternalServerError)
			return
		} else if len(refs) == 0 {
			http.Error(w, fmt.Sprintf("No references found for ticket %q", ticket), http.StatusNotFound)
			return
		}

		log.Printf("xrefs.XRefs:\t%s\t%d", time.Since(startTime), total)
		if err := web.WriteJSONResponse(w, r, refs); err != nil {
			log.Println(err)
		}
	})
}

var (
	anchorFilters  = []string{schema.NodeKindFact, schema.AnchorLocFilter}
	revDefinesEdge = schema.MirrorEdge(schema.DefinesEdge)
	revNamedEdge   = schema.MirrorEdge(schema.NamedEdge)
)

// AnchorLocation is an unwrapped anchor node with its parent file's ticket.
type AnchorLocation struct {
	Anchor string `json:"anchor"`
	File   string `json:"file"`
	Start  int    `json:"start"`
	End    int    `json:"end"`
}

// XRefs returns a set of related AnchorLocations for the given ticket in a map
// keyed by edge kind. If indirectNames is set, the resulting set of anchors
// will include those for nodes that can be reached through related name nodes.
// TODO(schroederc): clean up and decide if this goes into the Service api
func XRefs(xs Service, ticket string, indirectNames bool) (map[string][]*AnchorLocation, int, error) {
	// Graph path:
	//  node[ticket]
	//    ( --%edge-> || --edge-> relatedNode --%defines-> )
	//    []anchor --rev(childof)-> file

	// node[ticket] --*-> []anchor
	anchorEdges, anchorNodes, err := edgesMaps(xs.Edges(&xpb.EdgesRequest{
		Ticket: []string{ticket},
		Filter: anchorFilters,
	}))
	if err != nil {
		return nil, 0, fmt.Errorf("bad anchor edges request: %v", err)
	}

	if indirectNames {
		// Assuming node[ticket] is a name node, add the set of related nodes/edges
		// for the nodes that declare node[ticket] their name.
		if err := mergeIndirectMaps(xs, anchorEdges, anchorNodes, ticket, revNamedEdge); err != nil {
			return nil, 0, nil
		}
		// Add the set of related nodes/edges for the name nodes of node[ticket].
		if err := mergeIndirectMaps(xs, anchorEdges, anchorNodes, ticket, schema.NamedEdge); err != nil {
			return nil, 0, nil
		}
	}

	// Preliminary response map w/o File tickets populated
	anchorLocs := make(map[string][]*AnchorLocation)

	anchorTargetSet := stringset.New()
	relatedNodeSet := stringset.New()
	relatedNodeEdgeKinds := make(map[string][]string) // ticket -> []edgeKind
	for kind, targets := range anchorEdges[ticket] {
		if ck := schema.Canonicalize(kind); ck != schema.NamedEdge && ck != schema.ChildOfEdge {
			for _, target := range targets {
				if schema.EdgeDirection(kind) == schema.Reverse {
					loc := nodeAnchorLocation(anchorNodes[target])
					if loc != nil {
						// --%revEdge-> anchor
						anchorTargetSet.Add(target)
						anchorLocs[kind] = append(anchorLocs[kind], loc)
						continue
					}
				}
				// --edge-> relatedNode
				relatedNodeSet.Add(target)
				relatedNodeEdgeKinds[target] = append(relatedNodeEdgeKinds[target], kind)
			}
		}
	}

	if len(relatedNodeSet) > 0 {
		// relatedNode --%defines-> anchor
		relatedAnchorEdges, relatedAnchorNodes, err := edgesMaps(xs.Edges(&xpb.EdgesRequest{
			Ticket: relatedNodeSet.Slice(),
			Kind:   []string{revDefinesEdge},
			Filter: anchorFilters,
		}))
		if err != nil {
			return nil, 0, fmt.Errorf("bad inter anchor edges request: %v", err)
		}

		for interNode, edgeKinds := range relatedNodeEdgeKinds {
			for _, target := range relatedAnchorEdges[interNode][revDefinesEdge] {
				node := relatedAnchorNodes[target]
				if nodeKind(node) == schema.AnchorKind {
					loc := nodeAnchorLocation(node)
					if loc == nil {
						continue
					}
					anchorTargetSet.Add(target)
					for _, kind := range edgeKinds {
						anchorLocs[kind] = append(anchorLocs[kind], loc)
					}
				}
			}
		}
	}

	// []anchor -> file
	fileEdges, fileNodes, err := edgesMaps(xs.Edges(&xpb.EdgesRequest{
		Ticket: anchorTargetSet.Slice(),
		Kind:   []string{schema.ChildOfEdge},
		Filter: []string{schema.NodeKindFact},
	}))
	if err != nil {
		return nil, 0, fmt.Errorf("bad files edges request: %v", err)
	}

	// Response map to send as JSON (filtered from anchorLocs for only anchors w/ known files)
	refs := make(map[string][]*AnchorLocation)

	// Find files for each of anchorLocs and filter those without known files
	var totalRefs int
	for kind, locs := range anchorLocs {
		var fileLocs []*AnchorLocation
		for _, loc := range locs {
			file := stringset.New()
			for _, targets := range fileEdges[loc.Anchor] {
				for _, target := range targets {
					if nodeKind(fileNodes[target]) == schema.FileKind {
						file.Add(target)
					}
				}
			}
			if len(file) != 1 {
				log.Printf("XRefs: not one file found for anchor %q: %v", loc.Anchor, file.Slice())
				continue
			}
			loc.File = file.Slice()[0]
			fileLocs = append(fileLocs, loc)
		}
		if len(fileLocs) != 0 {
			totalRefs += len(fileLocs)
			refs[kind] = fileLocs
		}
	}

	return refs, totalRefs, nil
}

// mergeIndirectMaps will find the related nodes of a ticket through the
// indirectEdgeKind using xs, adding each node's anchors to the given nodes and
// edges maps.
func mergeIndirectMaps(xs Service, anchorEdges map[string]map[string][]string, anchorNodes map[string]*xpb.NodeInfo, ticket string, indirectEdgeKind string) error {
	var nodes []string
	for _, target := range anchorEdges[ticket][indirectEdgeKind] {
		nodes = append(nodes, target)
	}

	moreAnchorEdges, moreAnchorNodes, err := edgesMaps(xs.Edges(&xpb.EdgesRequest{
		Ticket: nodes,
		Filter: anchorFilters,
	}))
	if err != nil {
		return fmt.Errorf("bad defer anchor edges request: %v", err)
	}

	for ticket, info := range moreAnchorNodes {
		anchorNodes[ticket] = info
	}
	for _, moreEdges := range moreAnchorEdges {
		for edgeKind, targets := range moreEdges {
			anchorEdges[ticket][edgeKind] = append(anchorEdges[ticket][edgeKind], targets...)
		}
	}

	return nil
}

// edgesMaps post-processes an EdgesReply into a ticket->edgeKind->[]targets map
// and a nodes map.
func edgesMaps(r *xpb.EdgesReply, err error) (map[string]map[string][]string, map[string]*xpb.NodeInfo, error) {
	if err != nil {
		return nil, nil, err
	}

	edges := make(map[string]map[string][]string)
	for _, s := range r.EdgeSet {
		g := make(map[string][]string)
		for _, group := range s.Group {
			g[group.GetKind()] = group.TargetTicket
		}
		edges[s.GetSourceTicket()] = g
	}
	nodes := make(map[string]*xpb.NodeInfo)
	for _, n := range r.Node {
		nodes[n.GetTicket()] = n
	}
	return edges, nodes, nil
}

// nodeKind returns the schema.NodeKindFact value of the given node, or if not
// found, ""
func nodeKind(n *xpb.NodeInfo) string {
	for _, f := range n.GetFact() {
		if f.GetName() == schema.NodeKindFact {
			return string(f.Value)
		}
	}
	return ""
}

// nodeAnchorLocation returns an equivalent AnchorLocation for the given node.
// Returns nil if the given node isn't a valid anchor
func nodeAnchorLocation(anchor *xpb.NodeInfo) *AnchorLocation {
	if nodeKind(anchor) != schema.AnchorKind {
		return nil
	}
	var start, end int
	for _, f := range anchor.Fact {
		var err error
		switch f.GetName() {
		case schema.AnchorStartFact:
			start, err = strconv.Atoi(string(f.Value))
		case schema.AnchorEndFact:
			end, err = strconv.Atoi(string(f.Value))
		}
		if err != nil {
			log.Printf("Failed to parse %q: %v", string(f.Value), err)
		}
	}
	return &AnchorLocation{
		Anchor: anchor.GetTicket(),
		Start:  start,
		End:    end,
	}
}
