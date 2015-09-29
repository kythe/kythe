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
	"errors"
	"fmt"
	"log"
	"math"
	"net/http"
	"regexp"
	"sort"
	"strconv"
	"time"

	"kythe.io/kythe/go/services/web"
	"kythe.io/kythe/go/util/schema"
	"kythe.io/kythe/go/util/stringset"

	"github.com/sergi/go-diff/diffmatchpatch"
	"golang.org/x/net/context"

	xpb "kythe.io/kythe/proto/xref_proto"
)

// Service provides access to a Kythe graph for fast access to cross-references.
type Service interface {
	NodesEdgesService
	DecorationsService
	CrossReferencesService
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

// CrossReferencesService provides fast access to cross-references in a Kythe graph.
type CrossReferencesService interface {
	// CrossReferences returns the global references of the given nodes.
	CrossReferences(context.Context, *xpb.CrossReferencesRequest) (*xpb.CrossReferencesReply, error)
}

const defaultXRefPageSize = 1024

// CrossReferences returns the cross-references for the given tickets using the
// given NodesEdgesService.
func CrossReferences(ctx context.Context, xs NodesEdgesService, req *xpb.CrossReferencesRequest) (*xpb.CrossReferencesReply, error) {
	log.Println("WARNING: using experimental CrossReferences API")
	if len(req.Ticket) == 0 {
		return nil, errors.New("no cross-references requested")
	}

	requestedPageSize := int(req.PageSize)
	if requestedPageSize == 0 {
		requestedPageSize = defaultXRefPageSize
	}

	eReply, err := xs.Edges(ctx, &xpb.EdgesRequest{
		Ticket:    req.Ticket,
		PageSize:  int32(requestedPageSize),
		PageToken: req.PageToken,
	})
	if err != nil {
		return nil, fmt.Errorf("error getting edges for cross-references: %v", err)
	}

	reply := &xpb.CrossReferencesReply{
		CrossReferences: make(map[string]*xpb.CrossReferencesReply_CrossReferenceSet),

		NextPageToken: eReply.NextPageToken,
	}
	var allRelatedNodes stringset.Set
	if len(req.Filter) > 0 {
		reply.Nodes = make(map[string]*xpb.NodeInfo)
		allRelatedNodes = stringset.New()
	}

	// Cache location Normalizers for parent files across all anchors
	normalizers := make(map[string]*Normalizer)

	var totalXRefs int
	for {
		for _, es := range eReply.EdgeSet {
			xr, ok := reply.CrossReferences[es.SourceTicket]
			if !ok {
				xr = &xpb.CrossReferencesReply_CrossReferenceSet{Ticket: es.SourceTicket}
			}

			var count int
			for _, g := range es.Group {
				switch {
				case isDefKind(req.DefinitionKind, g.Kind):
					anchors, err := completeAnchors(ctx, xs, req.AnchorText, normalizers, g.Kind, g.TargetTicket)
					if err != nil {
						return nil, fmt.Errorf("error resolving definition anchors: %v", err)
					}
					count += len(anchors)
					xr.Definition = append(xr.Definition, anchors...)
				case isRefKind(req.ReferenceKind, g.Kind):
					anchors, err := completeAnchors(ctx, xs, req.AnchorText, normalizers, g.Kind, g.TargetTicket)
					if err != nil {
						return nil, fmt.Errorf("error resolving reference anchors: %v", err)
					}
					count += len(anchors)
					xr.Reference = append(xr.Reference, anchors...)
				case isDocKind(req.DocumentationKind, g.Kind):
					anchors, err := completeAnchors(ctx, xs, req.AnchorText, normalizers, g.Kind, g.TargetTicket)
					if err != nil {
						return nil, fmt.Errorf("error resolving documentation anchors: %v", err)
					}
					count += len(anchors)
					xr.Documentation = append(xr.Documentation, anchors...)
				case allRelatedNodes != nil && !schema.IsAnchorEdge(g.Kind):
					count += len(g.TargetTicket)
					for _, target := range g.TargetTicket {
						xr.RelatedNode = append(xr.RelatedNode, &xpb.CrossReferencesReply_RelatedNode{
							Ticket:       target,
							RelationKind: g.Kind,
						})
					}
					allRelatedNodes.Add(g.TargetTicket...)
				}
			}

			if count > 0 {
				reply.CrossReferences[xr.Ticket] = xr
				totalXRefs += count
			}
		}

		if reply.NextPageToken == "" || totalXRefs > 0 {
			break
		}

		// We need to return at least 1 xref, if there are any
		log.Println("Extra CrossReferences Edges call: ", reply.NextPageToken)
		eReply, err = xs.Edges(ctx, &xpb.EdgesRequest{
			Ticket:    req.Ticket,
			PageSize:  int32(requestedPageSize),
			PageToken: reply.NextPageToken,
		})
		if err != nil {
			return nil, fmt.Errorf("error getting edges for cross-references: %v", err)
		}
		reply.NextPageToken = eReply.NextPageToken
	}

	if len(allRelatedNodes) > 0 {
		nReply, err := xs.Nodes(ctx, &xpb.NodesRequest{
			Ticket: allRelatedNodes.Slice(),
			Filter: req.Filter,
		})
		if err != nil {
			return nil, fmt.Errorf("error retrieving related nodes: %v", err)
		}
		for _, n := range nReply.Node {
			reply.Nodes[n.Ticket] = n
		}
	}

	return reply, nil
}

var (
	revDefinesEdge        = schema.MirrorEdge(schema.DefinesEdge)
	revDefinesBindingEdge = schema.MirrorEdge(schema.DefinesBindingEdge)

	revRefEdge = schema.MirrorEdge(schema.RefEdge)

	revDocumentsEdge = schema.MirrorEdge(schema.DocumentsEdge)
)

func isDefKind(requestedKind xpb.CrossReferencesRequest_DefinitionKind, edgeKind string) bool {
	switch requestedKind {
	case xpb.CrossReferencesRequest_NO_DEFINITIONS:
		return false
	case xpb.CrossReferencesRequest_FULL_DEFINITIONS:
		return edgeKind == revDefinesEdge
	case xpb.CrossReferencesRequest_BINDING_DEFINITIONS:
		return edgeKind == revDefinesBindingEdge
	case xpb.CrossReferencesRequest_ALL_DEFINITIONS:
		return schema.IsEdgeVariant(edgeKind, revDefinesEdge)
	default:
		panic("unhandled CrossReferencesRequest_DefinitionKind")
	}
}

func isRefKind(requestedKind xpb.CrossReferencesRequest_ReferenceKind, edgeKind string) bool {
	switch requestedKind {
	case xpb.CrossReferencesRequest_NO_REFERENCES:
		return false
	case xpb.CrossReferencesRequest_ALL_REFERENCES:
		return schema.IsEdgeVariant(edgeKind, revRefEdge)
	default:
		panic("unhandled CrossReferencesRequest_ReferenceKind")
	}
}

func isDocKind(requestedKind xpb.CrossReferencesRequest_DocumentationKind, edgeKind string) bool {
	switch requestedKind {
	case xpb.CrossReferencesRequest_NO_DOCUMENTATION:
		return false
	case xpb.CrossReferencesRequest_ALL_DOCUMENTATION:
		return schema.IsEdgeVariant(edgeKind, revDocumentsEdge)
	default:
		panic("unhandled CrossDocumentationRequest_DocumentationKind")
	}
}

func getSpan(facts map[string][]byte, startFact, endFact string) (startOffset, endOffset int, err error) {
	start := string(facts[startFact])
	end := string(facts[endFact])
	if start == "" || end == "" {
		return 0, 0, fmt.Errorf("missing location facts; found: %s=%q and %s=%q",
			startFact, start, endFact, end)
	}
	so, err := strconv.Atoi(start)
	if err != nil {
		return 0, 0, fmt.Errorf("error parsing %s value %q: %v", startFact, start, err)
	}
	eo, err := strconv.Atoi(end)
	if err != nil {
		return 0, 0, fmt.Errorf("error parsing %s value %q: %v", endFact, end, err)
	}
	if so > eo {
		return 0, 0, fmt.Errorf("invalid %s/%s span: %d-%d", startFact, endFact, so, eo)
	}

	return so, eo, nil
}

func normalizeSpan(norm *Normalizer, startOffset, endOffset int32) (start, end *xpb.Location_Point, err error) {
	start = norm.ByteOffset(startOffset)
	end = norm.ByteOffset(endOffset)

	if start.ByteOffset != startOffset {
		err = fmt.Errorf("inconsistent start location; expected: %d; found; %d",
			startOffset, start.ByteOffset)
	} else if end.ByteOffset != endOffset {
		err = fmt.Errorf("inconsistent end location; expected: %d; found; %d",
			endOffset, end.ByteOffset)
	}
	return
}

func completeAnchors(ctx context.Context, xs NodesEdgesService, retrieveText bool, normalizers map[string]*Normalizer, edgeKind string, anchors []string) ([]*xpb.Anchor, error) {
	edgeKind = schema.Canonicalize(edgeKind)

	// AllEdges is relatively safe because each anchor will have very few parents (almost always 1)
	reply, err := AllEdges(ctx, xs, &xpb.EdgesRequest{
		Ticket: anchors,
		Kind:   []string{schema.ChildOfEdge},
		Filter: []string{
			schema.NodeKindFact,
			schema.TextFact,
			schema.AnchorLocFilter,
			schema.SnippetLocFilter,
		},
	})
	if err != nil {
		return nil, err
	}
	nodes := NodesMap(reply.Node)

	var result []*xpb.Anchor
	for _, es := range reply.EdgeSet {
		ticket := es.SourceTicket

		if nodeKind := string(nodes[ticket][schema.NodeKindFact]); nodeKind != schema.AnchorKind {
			log.Printf("Found non-anchor target to %q edge: %q (kind: %q)", edgeKind, ticket, nodeKind)
			continue
		}

		// Parse anchor location start/end facts
		so, eo, err := getSpan(nodes[ticket], schema.AnchorStartFact, schema.AnchorEndFact)
		if err != nil {
			log.Printf("Invalid anchor span for %q: %v", ticket, err)
			continue
		}

		// For each file parent to the anchor, add an Anchor to the result.
		for _, g := range es.Group {
			if g.Kind != schema.ChildOfEdge {
				continue
			}

			for _, parent := range g.TargetTicket {
				if parentKind := string(nodes[parent][schema.NodeKindFact]); parentKind != schema.FileKind {
					log.Printf("Found non-file parent to anchor: %q (kind: %q)", parent, parentKind)
					continue
				}

				a := &xpb.Anchor{
					Ticket: ticket,
					Kind:   edgeKind,
					Parent: parent,
				}

				text := nodes[a.Parent][schema.TextFact]
				norm, ok := normalizers[a.Parent]
				if !ok {
					norm = NewNormalizer(text)
					normalizers[a.Parent] = norm
				}

				a.Start, a.End, err = normalizeSpan(norm, int32(so), int32(eo))
				if err != nil {
					log.Printf("Invalid anchor span %q in file %q: %v", ticket, parent, err)
					continue
				}

				if retrieveText && a.Start.ByteOffset < a.End.ByteOffset {
					// TODO(schroederc): handle non-UTF8 encodings
					a.Text = string(text[a.Start.ByteOffset:a.End.ByteOffset])
				}

				if snippetStart, snippetEnd, err := getSpan(nodes[ticket], schema.SnippetStartFact, schema.SnippetEndFact); err == nil {
					startPoint, endPoint, err := normalizeSpan(norm, int32(snippetStart), int32(snippetEnd))
					if err != nil {
						log.Printf("Invalid snippet span %q in file %q: %v", ticket, parent, err)
					} else {
						a.Snippet = string(text[startPoint.ByteOffset:endPoint.ByteOffset])
					}
				}

				// fallback to a line-based snippet if the indexer did not provide its own snippet offsets
				if a.Snippet == "" {
					nextLine := norm.Point(&xpb.Location_Point{LineNumber: a.Start.LineNumber + 1})
					a.Snippet = string(text[a.Start.ByteOffset-a.Start.ColumnOffset : nextLine.ByteOffset-1])
				}

				result = append(result, a)
			}

			break // we've handled the only /kythe/edge/childof group
		}
	}

	return result, nil
}

// AllEdges returns all edges for a particular EdgesRequest.  This means that
// the returned reply will not have a next page token.  WARNING: the paging API
// exists for a reason; using this can lead to very large memory consumption
// depending on the request.
func AllEdges(ctx context.Context, es EdgesService, req *xpb.EdgesRequest) (*xpb.EdgesReply, error) {
	req.PageSize = math.MaxInt32
	reply, err := es.Edges(ctx, req)
	if err != nil || reply.NextPageToken == "" {
		return reply, err
	}

	nodes, edges := NodesMap(reply.Node), EdgesMap(reply.EdgeSet)

	for reply.NextPageToken != "" && err == nil {
		req.PageToken = reply.NextPageToken
		reply, err = es.Edges(ctx, req)
		if err != nil {
			return nil, err
		}
		nodesMapInto(reply.Node, nodes)
		edgesMapInto(reply.EdgeSet, edges)
	}

	reply = &xpb.EdgesReply{
		Node:    make([]*xpb.NodeInfo, 0, len(nodes)),
		EdgeSet: make([]*xpb.EdgeSet, 0, len(edges)),
	}

	for ticket, facts := range nodes {
		info := &xpb.NodeInfo{
			Ticket: ticket,
			Fact:   make([]*xpb.Fact, 0, len(facts)),
		}
		for name, val := range facts {
			info.Fact = append(info.Fact, &xpb.Fact{
				Name:  name,
				Value: val,
			})
		}
		reply.Node = append(reply.Node, info)
	}

	for source, kinds := range edges {
		set := &xpb.EdgeSet{
			SourceTicket: source,
			Group:        make([]*xpb.EdgeSet_Group, 0, len(kinds)),
		}
		for kind, targets := range kinds {
			set.Group = append(set.Group, &xpb.EdgeSet_Group{
				Kind:         kind,
				TargetTicket: targets,
			})
		}
		reply.EdgeSet = append(reply.EdgeSet, set)
	}

	return reply, err
}

// NodesMap returns a map from each node ticket to a map of its facts.
func NodesMap(nodes []*xpb.NodeInfo) map[string]map[string][]byte {
	m := make(map[string]map[string][]byte, len(nodes))
	nodesMapInto(nodes, m)
	return m
}

func nodesMapInto(nodes []*xpb.NodeInfo, m map[string]map[string][]byte) {
	for _, n := range nodes {
		facts, ok := m[n.Ticket]
		if !ok {
			facts = make(map[string][]byte, len(n.Fact))
			m[n.Ticket] = facts
		}
		for _, f := range n.Fact {
			facts[f.Name] = f.Value
		}
	}
}

// EdgesMap returns a map from each node ticket to a map of its outward edge kinds.
func EdgesMap(edges []*xpb.EdgeSet) map[string]map[string][]string {
	m := make(map[string]map[string][]string, len(edges))
	edgesMapInto(edges, m)
	return m
}

func edgesMapInto(edges []*xpb.EdgeSet, m map[string]map[string][]string) {
	for _, es := range edges {
		kinds, ok := m[es.SourceTicket]
		if !ok {
			kinds = make(map[string][]string, len(es.Group))
			m[es.SourceTicket] = kinds
		}
		for _, g := range es.Group {
			kinds[g.Kind] = append(kinds[g.Kind], g.TargetTicket...)
		}
	}
}

// Patcher uses a computed diff between two texts to map spans from the original
// text to the new text.
type Patcher struct {
	dmp  *diffmatchpatch.DiffMatchPatch
	diff []diffmatchpatch.Diff
}

// NewPatcher returns a Patcher based on the diff between oldText and newText.
func NewPatcher(oldText, newText []byte) *Patcher {
	dmp := diffmatchpatch.New()
	return &Patcher{dmp, dmp.DiffCleanupEfficiency(dmp.DiffMain(string(oldText), string(newText), true))}
}

// Patch returns the resulting span of mapping the given span from the Patcher's
// constructed oldText to its newText.  If the span no longer exists in newText
// or is invalid, the returned bool will be false.  As a convenience, if p==nil,
// the original span will be returned.
func (p *Patcher) Patch(spanStart, spanEnd int32) (newStart, newEnd int32, exists bool) {
	if spanStart > spanEnd {
		return 0, 0, false
	} else if p == nil {
		return spanStart, spanEnd, true
	}

	var old, new int32
	for _, d := range p.diff {
		l := int32(len(d.Text))
		if old > spanStart {
			return 0, 0, false
		}
		switch d.Type {
		case diffmatchpatch.DiffEqual:
			if old <= spanStart && spanEnd <= old+l {
				newStart = new + (spanStart - old)
				newEnd = new + (spanEnd - old)
				exists = true
				return
			}
			old += l
			new += l
		case diffmatchpatch.DiffDelete:
			old += l
		case diffmatchpatch.DiffInsert:
			new += l
		}
	}

	return 0, 0, false
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
	if p == nil {
		return nil
	} else if p.ByteOffset > 0 {
		return n.ByteOffset(p.ByteOffset)
	} else if p.LineNumber > 0 {
		np := &xpb.Location_Point{
			LineNumber:   p.LineNumber,
			ColumnOffset: p.ColumnOffset,
		}

		if totalLines := int32(len(n.lineLen)); p.LineNumber > totalLines {
			np.LineNumber = totalLines
			np.ColumnOffset = n.lineLen[np.LineNumber-1] - 1
		}
		if np.ColumnOffset < 0 {
			np.ColumnOffset = 0
		} else if np.ColumnOffset > 0 {
			if lineLen := n.lineLen[np.LineNumber-1] - 1; p.ColumnOffset > lineLen {
				np.ColumnOffset = lineLen
			}
		}

		np.ByteOffset = n.prefixLen[np.LineNumber-1] + np.ColumnOffset

		return np
	} else {
		return &xpb.Location_Point{LineNumber: 1}
	}
}

// ByteOffset returns a normalized point based on the given offset within the
// Normalizer's text.  A normalized point has all of its fields set consistently
// and clamped within the range [0,len(text)).
func (n *Normalizer) ByteOffset(offset int32) *xpb.Location_Point {
	np := &xpb.Location_Point{ByteOffset: offset}
	if np.ByteOffset > n.textLen {
		np.ByteOffset = n.textLen
	}

	np.LineNumber = int32(sort.Search(len(n.lineLen), func(i int) bool {
		return n.prefixLen[i] > np.ByteOffset
	}))
	np.ColumnOffset = np.ByteOffset - n.prefixLen[np.LineNumber-1]

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

// CrossReferences implements part of the Service interface.
func (w *grpcClient) CrossReferences(ctx context.Context, req *xpb.CrossReferencesRequest) (*xpb.CrossReferencesReply, error) {
	return w.XRefServiceClient.CrossReferences(ctx, req)
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

// CrossReferences implements part of the Service interface.
func (w *webClient) CrossReferences(ctx context.Context, q *xpb.CrossReferencesRequest) (*xpb.CrossReferencesReply, error) {
	var reply xpb.CrossReferencesReply
	return &reply, web.Call(w.addr, "xrefs", q, &reply)
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
//   GET /xrefs
//     Request: JSON encoded xrefs.CrossReferencesRequest
//     Response: JSON encoded xrefs.CrossReferencesResponse
//
// Note: /nodes, /edges, /decorations, and /xrefs will return their responses as
// serialized protobufs if the "proto" query parameter is set.
func RegisterHTTPHandlers(ctx context.Context, xs Service, mux *http.ServeMux) {
	mux.HandleFunc("/xrefs", func(w http.ResponseWriter, r *http.Request) {
		start := time.Now()
		defer func() {
			log.Printf("xrefs.CrossReferences:\t%s", time.Since(start))
		}()
		var req xpb.CrossReferencesRequest
		if err := web.ReadJSONBody(r, &req); err != nil {
			http.Error(w, err.Error(), http.StatusBadRequest)
			return
		}
		reply, err := CrossReferences(ctx, xs, &req)
		if err != nil {
			http.Error(w, err.Error(), http.StatusInternalServerError)
			return
		}

		if err := web.WriteResponse(w, r, reply); err != nil {
			log.Println(err)
		}
	})
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
