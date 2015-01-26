/*
 * Copyright 2014 Google Inc. All rights reserved.
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

// Package tools implements common utility functions for xrefs binary tools.
package tools

import (
	"fmt"
	"log"
	"strconv"
	"time"

	"kythe/go/serving/xrefs"
	"kythe/go/storage"
	"kythe/go/storage/filetree"
	"kythe/go/util/kytheuri"
	"kythe/go/util/schema"
	"kythe/go/util/stringset"

	spb "kythe/proto/storage_proto"
	xpb "kythe/proto/xref_proto"

	"code.google.com/p/goprotobuf/proto"
)

// AddReverseEdges scans gs for all forward edges, adding a reverse for each
// back into the GraphStore.
func AddReverseEdges(gs storage.GraphStore) error {
	log.Println("Adding reverse edges")
	var (
		totalEntries int
		addedEdges   int
	)
	startTime := time.Now()
	err := storage.EachScanEntry(gs, nil, func(entry *spb.Entry) error {
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

// CreateFileTree times the population of a filetree.Map with a given
// GraphStore.
func CreateFileTree(gs storage.GraphStore) (filetree.FileTree, error) {
	log.Println("Creating GraphStore file tree")
	startTime := time.Now()
	defer func() {
		log.Printf("Tree populated in %v", time.Since(startTime))
	}()
	t := filetree.NewMap()
	return t, t.Populate(gs)
}

var (
	anchorFilters  = []string{schema.NodeKindFact, "/kythe/loc/*"}
	revDefinesEdge = schema.MirrorEdge(schema.DefinesEdge)
	revNamedEdge   = schema.MirrorEdge(schema.NamedEdge)
)

// AnchorLocation is an unwrapped anchor node with its parent file's VName.
type AnchorLocation struct {
	Anchor string     `json:"anchor"`
	File   *spb.VName `json:"file"`
	Start  int        `json:"start"`
	End    int        `json:"end"`
}

// XRefs returns a set of related AnchorLocations for the given ticket in a map
// keyed by edge kind. If indirectNames is set, the resulting set of anchors
// will include those for nodes that can be reached through related name nodes.
// TODO(schroederc): clean up and decide if this goes into the xrefs.Service api
func XRefs(xs xrefs.Service, ticket string, indirectNames bool) (map[string][]*AnchorLocation, int, error) {
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
			ticket := file.Slice()[0]
			vname, err := kytheuri.ToVName(ticket)
			if err != nil {
				return nil, 0, fmt.Errorf("failed to parse file VName %q: %v", ticket, err)
			}
			loc.File = vname
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
func mergeIndirectMaps(xs xrefs.Service, anchorEdges map[string]map[string][]string, anchorNodes map[string]*xpb.NodeInfo, ticket string, indirectEdgeKind string) error {
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
