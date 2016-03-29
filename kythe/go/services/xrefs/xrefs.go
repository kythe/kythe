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
	"time"

	"kythe.io/kythe/go/services/web"
	"kythe.io/kythe/go/util/kytheuri"
	"kythe.io/kythe/go/util/schema"
	"kythe.io/kythe/go/util/stringset"

	"github.com/golang/protobuf/proto"
	"github.com/sergi/go-diff/diffmatchpatch"
	"golang.org/x/net/context"

	cpb "kythe.io/kythe/proto/common_proto"
	xpb "kythe.io/kythe/proto/xref_proto"
)

// Service provides access to a Kythe graph for fast access to cross-references.
type Service interface {
	NodesEdgesService
	DecorationsService
	CrossReferencesService
	CallersService
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

// CallersService provides fast access to the callgraph implied by a Kythe graph.
type CallersService interface {
	// Callers takes a set of tickets for semantic objects and returns the set
	// of places where those objects were called.
	Callers(context.Context, *xpb.CallersRequest) (*xpb.CallersReply, error)
}

var (
	// ErrDecorationsNotFound is returned from Decorations when decorations for
	// the given file cannot be found.
	ErrDecorationsNotFound = errors.New("file decorations not found")
)

// FixTickets returns an equivalent slice of normalized tickets.
func FixTickets(rawTickets []string) ([]string, error) {
	if len(rawTickets) == 0 {
		return nil, errors.New("no tickets specified")
	}

	var tickets []string
	for _, rawTicket := range rawTickets {
		ticket, err := kytheuri.Fix(rawTicket)
		if err != nil {
			return nil, fmt.Errorf("invalid ticket %q: %v", rawTicket, err)
		}
		tickets = append(tickets, ticket)
	}
	return tickets, nil
}

// IsDefKind determines whether the given edgeKind matches the requested
// definition kind.
func IsDefKind(requestedKind xpb.CrossReferencesRequest_DefinitionKind, edgeKind string) bool {
	// TODO(schroederc): add separate declarations group for `/kythe/complete != "definition"` nodes
	//                   use /kythe/edge/completes anchors as sole definition(s)
	edgeKind = schema.Canonicalize(edgeKind)
	switch requestedKind {
	case xpb.CrossReferencesRequest_NO_DEFINITIONS:
		return false
	case xpb.CrossReferencesRequest_FULL_DEFINITIONS:
		return edgeKind == schema.DefinesEdge || schema.IsEdgeVariant(edgeKind, schema.CompletesEdge)
	case xpb.CrossReferencesRequest_BINDING_DEFINITIONS:
		return edgeKind == schema.DefinesBindingEdge || schema.IsEdgeVariant(edgeKind, schema.CompletesEdge)
	case xpb.CrossReferencesRequest_ALL_DEFINITIONS:
		return schema.IsEdgeVariant(edgeKind, schema.DefinesEdge) || schema.IsEdgeVariant(edgeKind, schema.CompletesEdge)
	default:
		panic("unhandled CrossReferencesRequest_DefinitionKind")
	}
}

// IsRefKind determines whether the given edgeKind matches the requested
// reference kind.
func IsRefKind(requestedKind xpb.CrossReferencesRequest_ReferenceKind, edgeKind string) bool {
	edgeKind = schema.Canonicalize(edgeKind)
	switch requestedKind {
	case xpb.CrossReferencesRequest_NO_REFERENCES:
		return false
	case xpb.CrossReferencesRequest_ALL_REFERENCES:
		return schema.IsEdgeVariant(edgeKind, schema.RefEdge)
	default:
		panic("unhandled CrossReferencesRequest_ReferenceKind")
	}
}

// IsDocKind determines whether the given edgeKind matches the requested
// documentation kind.
func IsDocKind(requestedKind xpb.CrossReferencesRequest_DocumentationKind, edgeKind string) bool {
	edgeKind = schema.Canonicalize(edgeKind)
	switch requestedKind {
	case xpb.CrossReferencesRequest_NO_DOCUMENTATION:
		return false
	case xpb.CrossReferencesRequest_ALL_DOCUMENTATION:
		return schema.IsEdgeVariant(edgeKind, schema.DocumentsEdge)
	default:
		panic("unhandled CrossDocumentationRequest_DocumentationKind")
	}
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
			Fact:   make([]*cpb.Fact, 0, len(facts)),
		}
		for name, val := range facts {
			info.Fact = append(info.Fact, &cpb.Fact{
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
				Kind: kind,
				Edge: TicketsToEdges(targets),
			})
		}
		reply.EdgeSet = append(reply.EdgeSet, set)
	}

	return reply, err
}

// TicketsToEdges returns an equivalent set of *xpb.EdgeSet_Group_Edges.
func TicketsToEdges(tickets []string) (edges []*xpb.EdgeSet_Group_Edge) {
	for _, ticket := range tickets {
		edges = append(edges, &xpb.EdgeSet_Group_Edge{TargetTicket: ticket})
	}
	return
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
			for _, e := range g.Edge {
				kinds[g.Kind] = append(kinds[g.Kind], e.TargetTicket)
			}
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
	}

	if p.ByteOffset > 0 {
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
	}

	return &xpb.Location_Point{LineNumber: 1}
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

func forAllEdges(ctx context.Context, service Service, source stringset.Set, edge []string, f func(source string, target string) error) error {
	req := &xpb.EdgesRequest{
		Ticket: source.Slice(),
		Kind:   edge,
	}
	edges, err := AllEdges(ctx, service, req)
	if err != nil {
		return err
	}
	for _, es := range edges.EdgeSet {
		for _, group := range es.Group {
			for _, edge := range group.Edge {
				err = f(es.SourceTicket, edge.TargetTicket)
				if err != nil {
					return err
				}
			}
		}
	}
	return nil
}

const (
	// The maximum number of times Callers can expand its frontier.
	maxCallersExpansions = 10
	// The maximum size of the node set Callers will consider. This includes ref/call anchors and functions.
	maxCallersNodeSetSize = 1024
)

// findCallableDetail returns a map from non-callable semantic object tickets to CallableDetail,
// where the CallableDetail does not have the SemanticObject or SemanticObjectCallable fields set.
func findCallableDetail(ctx context.Context, service Service, tickets stringset.Set) (map[string]*xpb.CallersReply_CallableDetail, error) {
	ticketToDetail := make(map[string]*xpb.CallersReply_CallableDetail)
	xreq := &xpb.CrossReferencesRequest{
		Ticket:            tickets.Slice(),
		DefinitionKind:    xpb.CrossReferencesRequest_BINDING_DEFINITIONS,
		ReferenceKind:     xpb.CrossReferencesRequest_NO_REFERENCES,
		DocumentationKind: xpb.CrossReferencesRequest_NO_DOCUMENTATION,
		AnchorText:        true,
		PageSize:          math.MaxInt32,
	}
	xrefs, err := service.CrossReferences(ctx, xreq)
	if err != nil {
		log.Printf("Can't get CrossReferences for Callers set: %v", err)
		return nil, err
	}
	if xrefs.NextPageToken != "" {
		return nil, errors.New("UNIMPLEMENTED: Paged CrossReferences reply")
	}
	for _, xrefSet := range xrefs.CrossReferences {
		detail := &xpb.CallersReply_CallableDetail{}
		// Just pick the first definition.
		if len(xrefSet.Definition) >= 1 {
			detail.Definition = xrefSet.Definition[0]
			// ... and assume its text is the identifier of the function.
			detail.Identifier = detail.Definition.Text
			// ... and is also a fully-qualified name for it.
			detail.DisplayName = detail.Identifier
		}
		// TODO(zarko): Fill in parameters (and proper DisplayName/Identifier values.)
		ticketToDetail[xrefSet.Ticket] = detail
	}
	return ticketToDetail, nil
}

// SlowCallers is an implementation of the Callers API built from other APIs.
func SlowCallers(ctx context.Context, service Service, req *xpb.CallersRequest) (*xpb.CallersReply, error) {
	tickets, err := FixTickets(req.SemanticObject)
	// TODO(zarko): Map callable tickets back to function tickets; support CallToOverride.
	if err != nil {
		return nil, err
	}
	var edges []string
	// From the schema, we know the source of a defines* or completes* edge should be an anchor. Because of this,
	// we don't have to partition our set/queries on node kind.
	if req.IncludeOverrides {
		edges = []string{
			schema.OverridesEdge,
			schema.MirrorEdge(schema.OverridesEdge),
			schema.DefinesBindingEdge,
			schema.MirrorEdge(schema.DefinesBindingEdge),
			schema.CompletesEdge,
			schema.MirrorEdge(schema.CompletesEdge),
			schema.CompletesUniquelyEdge,
			schema.MirrorEdge(schema.CompletesUniquelyEdge),
		}
	} else {
		edges = []string{
			schema.DefinesBindingEdge,
			schema.MirrorEdge(schema.DefinesBindingEdge),
			schema.CompletesEdge,
			schema.MirrorEdge(schema.CompletesEdge),
			schema.CompletesUniquelyEdge,
			schema.MirrorEdge(schema.CompletesUniquelyEdge),
		}
	}
	// We keep a worklist of anchors and semantic nodes that are reachable from undirected edges
	// marked [overrides,] defines/binding, completes, or completes/uniquely. This overestimates
	// possible calls because it ignores link structure. This set *excludes* callables.
	// TODO(zarko): We need to mark nodes we reached through overrides.
	// All nodes that we've visited.
	retired := stringset.New()
	// Nodes that we haven't visited yet, but will this time around the loop.
	frontier := stringset.New(tickets...)
	iterations := 0
	for len(retired) < maxCallersNodeSetSize && len(frontier) != 0 && iterations < maxCallersExpansions {
		iterations++
		// Nodes that we haven't visited yet but will the next time around the loop.
		next := stringset.New()
		err = forAllEdges(ctx, service, frontier, edges, func(_, target string) error {
			if !retired.Contains(target) && !frontier.Contains(target) {
				next.Add(target)
			}
			return nil
		})
		for old := range frontier {
			retired.Add(old)
		}
		frontier = next
		if err != nil {
			return nil, err
		}
	}
	if len(retired) > maxCallersNodeSetSize {
		log.Printf("Callers iteration truncated (set too big)")
	}
	if iterations >= maxCallersExpansions {
		log.Printf("Callers iteration truncated (too many expansions)")
	}
	log.Printf("Finding callables for %d node(s) after %d iteration(s)", len(retired), iterations)
	// Now frontier is empty. The schema dictates that only semantic nodes in retired will have callableas edges.
	// callableToNode maps callables to nodes they're %callableas.
	callableToNode := make(map[string][]string)
	err = forAllEdges(ctx, service, retired, []string{schema.CallableAsEdge}, func(source string, target string) error {
		callableToNode[target] = append(callableToNode[target], source)
		return nil
	})
	if err != nil {
		return nil, err
	}
	// We can use CrossReferences to get detailed information about anchors that ref/call our callables. This unfortunately excludes blame information.
	xrefTickets := []string{}
	for callable := range callableToNode {
		xrefTickets = append(xrefTickets, callable)
	}
	xreq := &xpb.CrossReferencesRequest{
		Ticket:            xrefTickets,
		DefinitionKind:    xpb.CrossReferencesRequest_NO_DEFINITIONS,
		ReferenceKind:     xpb.CrossReferencesRequest_ALL_REFERENCES,
		DocumentationKind: xpb.CrossReferencesRequest_NO_DOCUMENTATION,
		AnchorText:        true,
		PageSize:          math.MaxInt32,
	}
	xrefs, err := service.CrossReferences(ctx, xreq)
	if err != nil {
		log.Printf("Can't get CrossReferences for Callers set: %v", err)
		return nil, err
	}
	if xrefs.NextPageToken != "" {
		return nil, errors.New("UNIMPLEMENTED: Paged CrossReferences reply")
	}
	// Now we've got a bunch of call site anchors. We need to figure out which functions they belong to.
	// anchors is the set of all callsite anchor tickets.
	anchors := stringset.New()
	// expandedAnchors maps callsite anchor tickets to anchor details.
	expandedAnchors := make(map[string]*xpb.Anchor)
	// parentToAnchor maps semantic node tickets to the callsite anchor tickets they own.
	parentToAnchor := make(map[string][]string)
	for _, refSet := range xrefs.CrossReferences {
		for _, ref := range refSet.Reference {
			if ref.Kind == schema.RefCallEdge {
				anchors.Add(ref.Ticket)
				expandedAnchors[ref.Ticket] = ref
			}
		}
	}
	err = forAllEdges(ctx, service, anchors, []string{schema.ChildOfEdge}, func(source string, target string) error {
		anchor, ok := expandedAnchors[source]
		if !ok {
			return errors.New("missing expanded anchor")
		}
		// anchor.Parent is the syntactic parent of the anchor; we want the semantic parent.
		if target != anchor.Parent {
			parentToAnchor[target] = append(parentToAnchor[target], source)
		}
		return nil
	})
	if err != nil {
		return nil, err
	}
	// Now begin filling out the response.
	reply := &xpb.CallersReply{}
	// We'll fetch CallableDetail only once for each interesting ticket, then add it to the response later.
	detailToFetch := stringset.New()
	for calleeCallable, calleeNodes := range callableToNode {
		for _, calleeNode := range calleeNodes {
			rCalleeNode := &xpb.CallersReply_CallableDetail{
				SemanticObject:         calleeNode,
				SemanticObjectCallable: calleeCallable,
			}
			reply.Callee = append(reply.Callee, rCalleeNode)
			detailToFetch.Add(calleeNode)
		}
	}
	for caller, calls := range parentToAnchor {
		rCaller := &xpb.CallersReply_Caller{
			Detail: &xpb.CallersReply_CallableDetail{
				SemanticObject: caller,
			},
		}
		detailToFetch.Add(caller)
		for _, callsite := range calls {
			rCallsite := &xpb.CallersReply_Caller_CallSite{}
			rCallsite.Anchor = expandedAnchors[callsite]
			// TODO(zarko): Set rCallsite.CallToOverride.
			rCaller.CallSite = append(rCaller.CallSite, rCallsite)
		}
		reply.Caller = append(reply.Caller, rCaller)
	}
	// Gather CallableDetail for each of our semantic nodes.
	details, err := findCallableDetail(ctx, service, detailToFetch)
	if err != nil {
		return nil, err
	}
	// ... and merge it back into the response.
	for _, detail := range reply.Callee {
		msg, found := details[detail.SemanticObject]
		if found {
			proto.Merge(detail, msg)
		}
	}
	for _, caller := range reply.Caller {
		msg, found := details[caller.Detail.SemanticObject]
		if found {
			proto.Merge(caller.Detail, msg)
		}
	}
	return reply, nil
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

// Callers implements part of the Service interface.
func (w *grpcClient) Callers(ctx context.Context, req *xpb.CallersRequest) (*xpb.CallersReply, error) {
	return w.XRefServiceClient.Callers(ctx, req)
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

// Callers implements part of the Service interface.
func (w *webClient) Callers(ctx context.Context, q *xpb.CallersRequest) (*xpb.CallersReply, error) {
	var reply xpb.CallersReply
	return &reply, web.Call(w.addr, "callers", q, &reply)
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
//     Response: JSON encoded xrefs.NodesReply
//   GET /edges
//     Request: JSON encoded xrefs.EdgesRequest
//     Response: JSON encoded xrefs.EdgesReply
//   GET /decorations
//     Request: JSON encoded xrefs.DecorationsRequest
//     Response: JSON encoded xrefs.DecorationsReply
//   GET /xrefs
//     Request: JSON encoded xrefs.CrossReferencesRequest
//     Response: JSON encoded xrefs.CrossReferencesReply
//   GET /callers
//     Request: JSON encoded xrefs.CallersRequest
//     Response: JSON encoded xrefs.CallersReply
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
		reply, err := xs.CrossReferences(ctx, &req)
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
	mux.HandleFunc("/callers", func(w http.ResponseWriter, r *http.Request) {
		start := time.Now()
		defer func() {
			log.Printf("xrefs.Callers:\t%s", time.Since(start))
		}()
		var req xpb.CallersRequest
		if err := web.ReadJSONBody(r, &req); err != nil {
			http.Error(w, err.Error(), http.StatusBadRequest)
			return
		}
		reply, err := xs.Callers(ctx, &req)
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

// ByName implements the sort.Interface for cpb.Facts
type ByName []*cpb.Fact

// Implement the sort.Interface
func (s ByName) Len() int           { return len(s) }
func (s ByName) Less(i, j int) bool { return s[i].Name < s[j].Name }
func (s ByName) Swap(i, j int)      { s[i], s[j] = s[j], s[i] }
