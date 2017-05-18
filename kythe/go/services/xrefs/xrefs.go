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
	"context"
	"errors"
	"flag"
	"fmt"
	"log"
	"math"
	"net/http"
	"regexp"
	"sort"
	"strconv"
	"strings"
	"time"

	"kythe.io/kythe/go/services/web"
	"kythe.io/kythe/go/util/kytheuri"
	"kythe.io/kythe/go/util/schema/edges"
	"kythe.io/kythe/go/util/schema/facts"
	"kythe.io/kythe/go/util/schema/nodes"

	"bitbucket.org/creachadair/stringset"
	"github.com/golang/protobuf/proto"
	"github.com/sergi/go-diff/diffmatchpatch"

	cpb "kythe.io/kythe/proto/common_proto"
	gpb "kythe.io/kythe/proto/graph_proto"
	xpb "kythe.io/kythe/proto/xref_proto"
)

// TODO(schroederc): remove all Slow* functions and their corresponding flags
var (
	disableAllSlowPaths = flag.Bool("disable_slow_paths", false, "Disable all slow path serving code")

	disableSlowOverrides     = flag.Bool("disable_slow_overrides", false, "Disable SlowOverrides serving code")
	disableSlowDefinitions   = flag.Bool("disable_slow_definitions", false, "Disable SlowDefinitions serving code")
	disableSlowDecls         = flag.Bool("disable_slow_declarations", false, "Disable SlowDeclarationsForCrossReferences serving code")
	disableSlowCallers       = flag.Bool("disable_slow_callers", false, "Disable SlowCallersForCrossReferences serving code")
	disableSlowDocumentation = flag.Bool("disable_slow_documentation", false, "Disable SlowDocumentation serving code")
	disableSlowSignatures    = flag.Bool("disable_slow_signatures", false, "Disable SlowSignature serving code")
)

// Service defines the interface for file based cross-references.  Informally,
// the cross-references of an entity comprise the definitions of that entity,
// together with all the places where those definitions are referenced through
// constructs such as type declarations, variable references, function calls,
// and so on.
type Service interface {
	GraphService

	// TODO(fromberger): Separate direct graph access from xrefs.

	// Decorations returns an index of the nodes associated with a specified file.
	Decorations(context.Context, *xpb.DecorationsRequest) (*xpb.DecorationsReply, error)

	// CrossReferences returns the global cross-references for the given nodes.
	CrossReferences(context.Context, *xpb.CrossReferencesRequest) (*xpb.CrossReferencesReply, error)

	// Documentation takes a set of tickets and returns documentation for them.
	Documentation(context.Context, *xpb.DocumentationRequest) (*xpb.DocumentationReply, error)
}

// GraphService exposes direct access to nodes and edges in a Kythe graph.
type GraphService interface {
	Nodes(context.Context, *gpb.NodesRequest) (*gpb.NodesReply, error)
	Edges(context.Context, *gpb.EdgesRequest) (*gpb.EdgesReply, error)
}

// ErrDecorationsNotFound is returned by an implementation of the Decorations
// method when decorations for the given file cannot be found.
var ErrDecorationsNotFound = errors.New("file decorations not found")

// FixTickets converts the specified tickets, which are expected to be Kythe
// URIs, into canonical form. It is an error if len(tickets) == 0.
func FixTickets(tickets []string) ([]string, error) {
	if len(tickets) == 0 {
		return nil, errors.New("no tickets specified")
	}

	canonical := make([]string, len(tickets))
	for i, ticket := range tickets {
		fixed, err := kytheuri.Fix(ticket)
		if err != nil {
			return nil, fmt.Errorf("invalid ticket %q: %v", ticket, err)
		}
		canonical[i] = fixed
	}
	return canonical, nil
}

// InSpanBounds reports whether [start,end) is bounded by the specified
// [startBoundary,endBoundary) span.
func InSpanBounds(kind xpb.DecorationsRequest_SpanKind, start, end, startBoundary, endBoundary int32) bool {
	switch kind {
	case xpb.DecorationsRequest_WITHIN_SPAN:
		return start >= startBoundary && end <= endBoundary
	case xpb.DecorationsRequest_AROUND_SPAN:
		return start <= startBoundary && end >= endBoundary
	default:
		log.Printf("WARNING: unknown DecorationsRequest_SpanKind: %v", kind)
	}
	return false
}

// IsDefKind reports whether the given edgeKind matches the requested
// definition kind.
func IsDefKind(requestedKind xpb.CrossReferencesRequest_DefinitionKind, edgeKind string, incomplete bool) bool {
	// TODO(schroederc): handle full vs. binding CompletesEdge
	edgeKind = edges.Canonical(edgeKind)
	if IsDeclKind(xpb.CrossReferencesRequest_ALL_DECLARATIONS, edgeKind, incomplete) {
		return false
	}
	switch requestedKind {
	case xpb.CrossReferencesRequest_NO_DEFINITIONS:
		return false
	case xpb.CrossReferencesRequest_FULL_DEFINITIONS:
		return edgeKind == edges.Defines || edges.IsVariant(edgeKind, edges.Completes)
	case xpb.CrossReferencesRequest_BINDING_DEFINITIONS:
		return edgeKind == edges.DefinesBinding || edges.IsVariant(edgeKind, edges.Completes)
	case xpb.CrossReferencesRequest_ALL_DEFINITIONS:
		return edges.IsVariant(edgeKind, edges.Defines) || edges.IsVariant(edgeKind, edges.Completes)
	default:
		log.Printf("ERROR: unhandled CrossReferencesRequest_DefinitionKind: %v", requestedKind)
		return false
	}
}

// IsDeclKind reports whether the given edgeKind matches the requested
// declaration kind
func IsDeclKind(requestedKind xpb.CrossReferencesRequest_DeclarationKind, edgeKind string, incomplete bool) bool {
	edgeKind = edges.Canonical(edgeKind)
	switch requestedKind {
	case xpb.CrossReferencesRequest_NO_DECLARATIONS:
		return false
	case xpb.CrossReferencesRequest_ALL_DECLARATIONS:
		return (incomplete && edges.IsVariant(edgeKind, edges.Defines)) || edgeKind == internalDeclarationKind
	default:
		log.Printf("ERROR: unhandled CrossReferenceRequest_DeclarationKind: %v", requestedKind)
		return false
	}
}

// IsRefKind determines whether the given edgeKind matches the requested
// reference kind.
func IsRefKind(requestedKind xpb.CrossReferencesRequest_ReferenceKind, edgeKind string) bool {
	edgeKind = edges.Canonical(edgeKind)
	switch requestedKind {
	case xpb.CrossReferencesRequest_NO_REFERENCES:
		return false
	case xpb.CrossReferencesRequest_CALL_REFERENCES:
		return edgeKind == edges.RefCall
	case xpb.CrossReferencesRequest_NON_CALL_REFERENCES:
		return edgeKind != edges.RefCall && edges.IsVariant(edgeKind, edges.Ref)
	case xpb.CrossReferencesRequest_ALL_REFERENCES:
		return edges.IsVariant(edgeKind, edges.Ref)
	default:
		log.Printf("ERROR: unhandled CrossReferencesRequest_ReferenceKind: %v", requestedKind)
		return false
	}
}

// Internal-only edge kinds for cross-references
const (
	internalKindPrefix         = "#internal/"
	internalCallerKindDirect   = internalKindPrefix + "ref/call/direct"
	internalCallerKindOverride = internalKindPrefix + "ref/call/override"
	internalDeclarationKind    = internalKindPrefix + "ref/declare"
)

// IsInternalKind determines whether the given edge kind is an internal variant.
func IsInternalKind(kind string) bool {
	return strings.HasPrefix(kind, internalKindPrefix)
}

// IsRelatedNodeKind determines whether the give edge kind is a non-anchor,
// related node kind.
func IsRelatedNodeKind(kind string) bool {
	return !IsInternalKind(kind) && !edges.IsAnchorEdge(kind)
}

// IsCallerKind determines whether the given edgeKind matches the requested
// caller kind.
func IsCallerKind(requestedKind xpb.CrossReferencesRequest_CallerKind, edgeKind string) bool {
	edgeKind = edges.Canonical(edgeKind)
	switch requestedKind {
	case xpb.CrossReferencesRequest_NO_CALLERS:
		return false
	case xpb.CrossReferencesRequest_DIRECT_CALLERS:
		return edgeKind == internalCallerKindDirect
	case xpb.CrossReferencesRequest_OVERRIDE_CALLERS:
		return edgeKind == internalCallerKindDirect || edgeKind == internalCallerKindOverride
	default:
		log.Printf("ERROR: unhandled CrossReferencesRequest_CallerKind: %v", requestedKind)
		return false
	}
}

// AllEdges returns all edges for a particular EdgesRequest.  This means that
// the returned reply will not have a next page token.  WARNING: the paging API
// exists for a reason; using this can lead to very large memory consumption
// depending on the request.
func AllEdges(ctx context.Context, es GraphService, req *gpb.EdgesRequest) (*gpb.EdgesReply, error) {
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

// SlowOverrides retrieves the list of Overrides for every input ticket. The map that it
// returns will only contain entries for input tickets that have at least one Override.
func SlowOverrides(ctx context.Context, xs Service, tickets []string) (map[string][]*xpb.DecorationsReply_Override, error) {
	if *disableSlowOverrides || *disableAllSlowPaths {
		log.Println("SlowOverrides disabled")
		return nil, nil
	}
	start := time.Now()
	log.Println("WARNING: performing slow-lookup of overrides")
	overrides := make(map[string][]*xpb.DecorationsReply_Override)
	edgeKinds := []string{
		edges.Overrides,
		edges.Extends,
		edges.ExtendsPrivate,
		edges.ExtendsPrivateVirtual,
		edges.ExtendsProtected,
		edges.ExtendsProtectedVirtual,
		edges.ExtendsPublic,
		edges.ExtendsPublicVirtual,
		edges.ExtendsVirtual,
	}
	err := forAllEdges(ctx, xs, stringset.New(tickets...), edgeKinds, func(source, target, _, edgeKind string) error {
		sig, err := SlowSignature(ctx, xs, target)
		if err != nil {
			log.Printf("SlowOverrides: error getting signature for %s: %v", target, err)
			sig = &cpb.MarkedSource{}
		}
		okind := xpb.DecorationsReply_Override_EXTENDS
		if edgeKind == edges.Overrides {
			okind = xpb.DecorationsReply_Override_OVERRIDES
		}
		overrides[source] = append(overrides[source], &xpb.DecorationsReply_Override{
			Target:       target,
			MarkedSource: sig,
			Kind:         okind,
		})
		return nil
	})
	// It's not necessarily the case that we should find *any* overrides, so counting them is not useful here.
	log.Printf("SlowOverrides: finished in %s", time.Since(start))
	if err != nil {
		return nil, err
	}
	return overrides, nil
}

// SlowDefinitions attempts to return a definition location for every node ticket given.  A
// definition will be returned only if it is unambiguous, but the definition may be indirect
// (through an intermediary node).
func SlowDefinitions(ctx context.Context, xs Service, tickets []string) (map[string]*xpb.Anchor, error) {
	if *disableSlowDefinitions || *disableAllSlowPaths {
		log.Println("SlowDefinitions disabled")
		return nil, nil
	}
	start := time.Now()
	log.Println("WARNING: performing slow-lookup of definitions")
	defs := make(map[string]*xpb.Anchor)

	nodeTargets := make(map[string]string, len(tickets))
	for _, ticket := range tickets {
		nodeTargets[ticket] = ticket
	}

	const maxJumps = 2
	for i := 0; i < maxJumps && len(nodeTargets) > 0; i++ {
		tickets := make([]string, 0, len(nodeTargets))
		for ticket := range nodeTargets {
			tickets = append(tickets, ticket)
		}

		xReply, err := xs.CrossReferences(ctx, &xpb.CrossReferencesRequest{
			Ticket:         tickets,
			DefinitionKind: xpb.CrossReferencesRequest_BINDING_DEFINITIONS,

			// Get node kinds of related nodes for indirect definitions
			Filter: []string{facts.NodeKind},
		})
		if err != nil {
			return nil, fmt.Errorf("error retrieving definition locations: %v", err)
		}

		nextJump := make(map[string]string)

		// Give client a definition location for each reference that has only 1
		// definition location.
		//
		// If a node does not have a single definition, but does have a relevant
		// relation to another node, try to find a single definition for the
		// related node instead.
		for ticket, cr := range xReply.CrossReferences {
			if len(cr.Definition) == 1 {
				loc := cr.Definition[0]
				// TODO(schroederc): handle differing kinds; completes vs. binding
				loc.Anchor.Kind = ""
				defs[nodeTargets[ticket]] = loc.Anchor
			}
		}

		nodeTargets = nextJump
	}

	var found int
	for _, ticket := range tickets {
		if _, ok := defs[ticket]; ok {
			found++
		}
	}
	log.Printf("SlowDefinitions: found %d/%d (%.1f%%) requested definitions in %s", found, len(tickets), float64(100*found)/float64(len(tickets)), time.Since(start))

	return defs, nil
}

// SlowDeclarationsForCrossReferences finds the tickets of every declaration completed by the same definitions.
func SlowDeclarationsForCrossReferences(ctx context.Context, xs Service, ticket string) ([]string, error) {
	if *disableSlowDecls || *disableAllSlowPaths {
		log.Println("SlowDeclarationsForCrossReferences disabled")
		return nil, nil
	}
	start := time.Now()
	defer func() { log.Printf("SlowDeclarationsForCrossReferences: %s", time.Since(start)) }()
	log.Println("WARNING: performing slow-lookup of declarations")
	// Find the set of definitions covered by the ticket (which may be either a declaration or a definition).
	reply, err := xs.CrossReferences(ctx, &xpb.CrossReferencesRequest{
		Ticket:         []string{ticket},
		DefinitionKind: xpb.CrossReferencesRequest_ALL_DEFINITIONS,
	})
	if err != nil {
		return nil, err
	}

	defanchors := stringset.New()
	for _, crs := range reply.CrossReferences {
		for _, ra := range crs.Definition {
			defanchors.Add(ra.Anchor.Ticket)
		}
	}

	// Follow all of the completion edges from the definition anchors to their declarations.
	decls := stringset.New()
	if err := forAllEdges(ctx, xs, defanchors, []string{edges.Completes, edges.CompletesUniquely},
		func(_, target, _, _ string) error {
			decls.Add(target)
			return nil
		}); err != nil {
		return nil, err
	}

	return decls.Elements(), nil
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

// PatchSpan returns the given Span's byte offsets mapped from the Patcher's
// oldText to its newText using Patcher.Patch.
func (p *Patcher) PatchSpan(span *cpb.Span) (newStart, newEnd int32, exists bool) {
	return p.Patch(span.GetStart().GetByteOffset(), span.GetEnd().GetByteOffset())
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

	if loc.Span == nil {
		return nil, errors.New("invalid SPAN: missing span")
	} else if loc.Span.Start == nil {
		return nil, errors.New("invalid SPAN: missing span start point")
	} else if loc.Span.End == nil {
		return nil, errors.New("invalid SPAN: missing span end point")
	}

	nl.Span = n.Span(loc.Span)

	start, end := nl.Span.Start.ByteOffset, nl.Span.End.ByteOffset
	if start > end {
		return nil, fmt.Errorf("invalid SPAN: start (%d) is after end (%d)", start, end)
	}
	return nl, nil
}

// Span returns a Span with its start and end normalized.
func (n *Normalizer) Span(s *cpb.Span) *cpb.Span {
	if s == nil {
		return nil
	}
	return &cpb.Span{
		Start: n.Point(s.Start),
		End:   n.Point(s.End),
	}
}

// SpanOffsets returns a Span based on normalized start and end byte offsets.
func (n *Normalizer) SpanOffsets(start, end int32) *cpb.Span {
	return &cpb.Span{
		Start: n.ByteOffset(start),
		End:   n.ByteOffset(end),
	}
}

var lineEnd = []byte("\n")

// Point returns a normalized point within the Normalizer's text.  A normalized
// point has all of its fields set consistently and clamped within the range
// [0,len(text)).
func (n *Normalizer) Point(p *cpb.Point) *cpb.Point {
	if p == nil {
		return nil
	}

	if p.ByteOffset > 0 {
		return n.ByteOffset(p.ByteOffset)
	} else if p.LineNumber > 0 {
		np := &cpb.Point{
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

	return &cpb.Point{LineNumber: 1}
}

// ByteOffset returns a normalized point based on the given offset within the
// Normalizer's text.  A normalized point has all of its fields set consistently
// and clamped within the range [0,len(text)).
func (n *Normalizer) ByteOffset(offset int32) *cpb.Point {
	np := &cpb.Point{ByteOffset: offset}
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
		re := filterToRegexp(filter)
		if re == matchesAll {
			return []*regexp.Regexp{re}
		}
		patterns = append(patterns, re)
	}
	return patterns
}

var (
	filterOpsRE = regexp.MustCompile("[*][*]|[*?]")
	matchesAll  = regexp.MustCompile(".*")
)

func filterToRegexp(pattern string) *regexp.Regexp {
	if pattern == "**" {
		return matchesAll
	}
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
		if p == matchesAll || p.MatchString(str) {
			return true
		}
	}
	return false
}

func forAllEdges(ctx context.Context, service Service, source stringset.Set, edge []string, f func(source, target, targetKind, edgeKind string) error) error {
	if source.Empty() {
		return nil
	}
	req := &gpb.EdgesRequest{
		Ticket: source.Elements(),
		Kind:   edge,
		Filter: []string{facts.NodeKind},
	}
	edges, err := AllEdges(ctx, service, req)
	if err != nil {
		return err
	}
	for source, es := range edges.EdgeSets {
		for edgeKind, group := range es.Groups {
			for _, edge := range group.Edge {
				info, foundInfo := edges.Nodes[edge.TargetTicket]
				kind := ""
				if foundInfo {
					kind = string(info.Facts[facts.NodeKind])
				}
				err = f(source, edge.TargetTicket, kind, edgeKind)
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

// Expand frontier until it includes all nodes connected by defines/binding, completes,
// completes/uniquely, and (optionally) overrides edges. Return the resulting set of
// tickets with anchors removed.
func expandDefRelatedNodeSet(ctx context.Context, service Service, frontier stringset.Set, includeOverrides bool) (stringset.Set, error) {
	// From the schema, we know the source of a defines* or completes* edge should be an anchor. Because of this,
	// we don't have to partition our set/queries on node kind.
	edgeKinds := []string{
		edges.DefinesBinding,
		edges.Mirror(edges.DefinesBinding),
		edges.Completes,
		edges.Mirror(edges.Completes),
		edges.CompletesUniquely,
		edges.Mirror(edges.CompletesUniquely),
	}
	if includeOverrides {
		edgeKinds = append(edgeKinds, edges.Overrides, edges.Mirror(edges.Overrides))
	}
	// We keep a worklist of anchors and semantic nodes that are reachable from undirected edges
	// marked [overrides,] defines/binding, completes, or completes/uniquely. This overestimates
	// possible calls because it ignores link structure.
	// TODO(zarko): We need to mark nodes we reached through overrides.
	// All nodes that we've visited.
	var retired, anchors stringset.Set
	iterations := 0
	for len(retired) < maxCallersNodeSetSize && len(frontier) != 0 && iterations < maxCallersExpansions {
		iterations++
		// Nodes that we haven't visited yet but will the next time around the loop.
		next := stringset.New()
		err := forAllEdges(ctx, service, frontier, edgeKinds, func(_, target, kind, _ string) error {
			if kind == nodes.Anchor {
				anchors.Add(target)
			}
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
	for ticket := range anchors {
		retired.Discard(ticket)
	}
	return retired, nil
}

// CallersRequest is the data needed for SlowCallersForCrossReferences.
type CallersRequest struct {
	Ticket string

	IncludeOverrides   bool
	GenerateSignatures bool

	PageSize  int32
	PageToken string
}

// CallersReply is a paged response from SlowCallersForCrossReferences.
type CallersReply struct {
	Callers []*xpb.CrossReferencesReply_RelatedAnchor

	EstimatedTotal int64
	NextPageToken  string
}

// SlowCallersForCrossReferences is an implementation of callgraph support meant
// for intermediate-term use by CrossReferences.
func SlowCallersForCrossReferences(ctx context.Context, service Service, req *CallersRequest) (*CallersReply, error) {
	if *disableSlowCallers || *disableAllSlowPaths {
		log.Println("SlowCallersForCrossReferences disabled")
		return &CallersReply{}, nil
	}
	start := time.Now()
	defer func() { log.Printf("SlowCallersForCrossReferences: %s", time.Since(start)) }()
	log.Println("WARNING: performing slow-lookup of callers")
	ticket, err := kytheuri.Fix(req.Ticket)
	if err != nil {
		return nil, err
	}
	callees, err := expandDefRelatedNodeSet(ctx, service, stringset.New(ticket), req.IncludeOverrides)
	if err != nil {
		return nil, err
	} else if callees.Empty() {
		return &CallersReply{}, nil
	}
	// This will not recursively call SlowCallersForCrossReferences as we're requesting NO_CALLERS above.
	xrefs, err := service.CrossReferences(ctx, &xpb.CrossReferencesRequest{
		Ticket:         callees.Elements(),
		DefinitionKind: xpb.CrossReferencesRequest_NO_DEFINITIONS,
		ReferenceKind:  xpb.CrossReferencesRequest_CALL_REFERENCES,
		CallerKind:     xpb.CrossReferencesRequest_NO_CALLERS,
		AnchorText:     true,
		PageSize:       req.PageSize,
		PageToken:      req.PageToken,
	})
	if err != nil {
		return nil, fmt.Errorf("can't get CrossReferences for callees set: %v", err)
	}
	reply := &CallersReply{
		// For the lack of anything better, we page the callers by call-sites.
		NextPageToken: xrefs.NextPageToken,
		// Overestimate the total number of callers by using the number of call-sites.
		EstimatedTotal: xrefs.Total.References,
	}
	// Now we've got a bunch of call site anchors. We need to figure out which functions they belong to.
	// anchors is the set of all callsite anchor tickets.
	var anchors stringset.Set
	// callsite anchor ticket to anchor details
	expandedAnchors := make(map[string]*xpb.Anchor)
	// semantic node ticket to owned callsite anchor tickets
	parentToAnchor := make(map[string][]string)
	for _, refs := range xrefs.CrossReferences {
		for _, ref := range refs.Reference {
			if ref.Anchor.Kind == edges.RefCall {
				anchors.Add(ref.Anchor.Ticket)
				expandedAnchors[ref.Anchor.Ticket] = ref.Anchor
			}
		}
	}
	var parentTickets []string
	if err := forAllEdges(ctx, service, anchors, []string{edges.ChildOf}, func(source, target, kind, _ string) error {
		anchor, ok := expandedAnchors[source]
		if !ok {
			log.Printf("Warning: missing expanded anchor for %v", source)
			return nil
		}
		// anchor.Parent is the syntactic parent of the anchor; we want the semantic parent.
		if target != anchor.Parent && kind != nodes.Anchor {
			if _, exists := parentToAnchor[target]; !exists {
				parentTickets = append(parentTickets, target)
			}
			parentToAnchor[target] = append(parentToAnchor[target], source)
		}
		return nil
	}); err != nil {
		return nil, err
	} else if len(parentTickets) == 0 {
		return reply, nil
	}
	// Get definition-site anchors. This won't recurse through this function because we're requesting NO_CALLERS.
	defs, err := service.CrossReferences(ctx, &xpb.CrossReferencesRequest{
		Ticket:         parentTickets,
		DefinitionKind: xpb.CrossReferencesRequest_ALL_DEFINITIONS,
		AnchorText:     true,
		PageSize:       math.MaxInt32, // attempt to get ALL definitions (warn on failure below)
	})
	if err != nil {
		return nil, fmt.Errorf("can't get CrossReferences for xref references set: %v", err)
	}
	if defs.NextPageToken != "" {
		log.Printf("Warning: paged CrossReferences reply looking for ALL_DEFINITIONS")
	}
	for ticket, refs := range defs.CrossReferences {
		for _, def := range refs.Definition {
			// This chooses the last Anchor arbitrarily.
			expandedAnchors[ticket] = def.Anchor
		}
	}
	var relatedAnchors []*xpb.CrossReferencesReply_RelatedAnchor
	for caller, calls := range parentToAnchor {
		callerAnchor, found := expandedAnchors[caller]
		if !found {
			log.Printf("Warning: missing expanded anchor for caller %v", ticket)
			continue
		}
		var signature *cpb.MarkedSource
		if req.GenerateSignatures {
			signature, err = SlowSignature(ctx, service, caller)
			if err != nil {
				return nil, fmt.Errorf("error looking up signature for caller ticket %q: %v", caller, err)
			}
		}
		var sites []*xpb.Anchor
		for _, ticket := range calls {
			related, found := expandedAnchors[ticket]
			if found {
				sites = append(sites, related)
			} else {
				log.Printf("Warning: missing expanded anchor for callsite %v", ticket)
			}
		}
		// See below regarding output stability.
		sort.Sort(byAnchor(sites))
		relatedAnchors = append(relatedAnchors, &xpb.CrossReferencesReply_RelatedAnchor{
			Anchor:       callerAnchor,
			MarkedSource: signature,
			Site:         sites,
			Ticket:       caller,
		})
	}
	// For pagination to work correctly, we must return RelatedAnchors in the same
	// order for the same request. Iterations over the parentToAnchor map are
	// not guaranteed to follow the same order, so relatedAnchors is arbitrarily
	// permuted. Undo this here.
	sort.Sort(byRelatedAnchor(relatedAnchors))
	reply.Callers = relatedAnchors
	return reply, nil
}

// byAnchor implements sort.Interface for []*xpb.Anchor based on the Ticket field.
type byAnchor []*xpb.Anchor

func (a byAnchor) Len() int           { return len(a) }
func (a byAnchor) Swap(i, j int)      { a[i], a[j] = a[j], a[i] }
func (a byAnchor) Less(i, j int) bool { return a[i].Ticket < a[j].Ticket }

// byRelatedAnchor implements sort.Interface for []*xpb.CrossReferencesReply_RelatedAnchor
// based on the Ticket field.
type byRelatedAnchor []*xpb.CrossReferencesReply_RelatedAnchor

func (a byRelatedAnchor) Len() int           { return len(a) }
func (a byRelatedAnchor) Swap(i, j int)      { a[i], a[j] = a[j], a[i] }
func (a byRelatedAnchor) Less(i, j int) bool { return a[i].Ticket < a[j].Ticket }

const (
	// The maximum number of times to recur in signature generation.
	maxFormatExpansions = 10
)

// extractParams extracts target tickets of param edges into a string slice ordered by ordinal.
func extractParams(edges []*gpb.EdgeSet_Group_Edge) []string {
	arr := make([]string, len(edges))
	for _, edge := range edges {
		ordinal := int(edge.Ordinal)
		if ordinal >= len(arr) || ordinal < 0 || arr[ordinal] != "" {
			for edgeix, edge := range edges {
				arr[edgeix] = edge.TargetTicket
			}
			return arr
		}
		arr[ordinal] = edge.TargetTicket
	}
	return arr
}

// getLanguage returns the language field of a Kythe ticket.
func getLanguage(ticket string) string {
	uri, err := kytheuri.Parse(ticket)
	if err != nil {
		return ""
	}
	return uri.Language
}

// slowLookupMeta retrieves the meta node for some node kind in some language.
func slowLookupMeta(ctx context.Context, service Service, language string, kind string) (*cpb.MarkedSource, error) {
	uri := kytheuri.URI{Language: language, Signature: kind + "#meta"}
	req := &gpb.NodesRequest{
		Ticket: []string{uri.String()},
		Filter: []string{facts.Code},
	}
	nodes, err := service.Nodes(ctx, req)
	if err != nil {
		return nil, fmt.Errorf("during slowLookupMeta: %v", err)
	}
	for _, node := range nodes.Nodes {
		for _, value := range node.Facts {
			rsig := &cpb.MarkedSource{}
			err = proto.Unmarshal(value, rsig)
			return rsig, err
		}
	}
	return nil, nil
}

// findParam finds the ticket for param number num in edges. It returns the empty string if a match wasn't found.
func findParam(reply *gpb.EdgesReply, num int) string {
	for _, set := range reply.EdgeSets {
		for kind, group := range set.Groups {
			if kind == edges.Param {
				for _, edge := range group.Edge {
					if int(edge.Ordinal) == num {
						return edge.TargetTicket
					}
				}
			}
		}
	}
	return ""
}

func getFactValue(edges *gpb.EdgesReply, ticket, fact string) []byte {
	if n := edges.Nodes[ticket]; n != nil {
		return n.Facts[fact]
	}
	return nil
}

// SignatureDetails contains information from the graph used by slowSignatureLevel to build signatures from MarkedSource.
type SignatureDetails struct {
	// Maps from tickets to node kinds.
	kinds      map[string]string
	signatures map[string]*cpb.MarkedSource
	// The tickets of this node's parameters (in parameter order).
	params []string
	// The first default param, if one is set.
	defaultParams map[string]int
}

// Returns the first default param for ticket or a negative value if there is none.
func (d *SignatureDetails) defaultParam(ticket string) int {
	dp, ok := d.defaultParams[ticket]
	if ok {
		return dp
	}
	return -1
}

// findSignatureDetails fills a SignatureDetails from the graph.
func findSignatureDetails(ctx context.Context, service Service, ticket string) (*SignatureDetails, error) {
	req := &gpb.EdgesRequest{
		Ticket:   []string{ticket},
		Kind:     []string{edges.Param},
		PageSize: math.MaxInt32,
		Filter:   []string{facts.NodeKind, facts.Code, facts.ParamDefault},
	}
	allEdges, err := AllEdges(ctx, service, req)
	if err != nil {
		return nil, fmt.Errorf("during AllEdges in findSignatureDetails: %v", err)
	}
	details := SignatureDetails{
		kinds:         make(map[string]string),
		signatures:    make(map[string]*cpb.MarkedSource),
		defaultParams: make(map[string]int),
	}
	for nodeTicket, node := range allEdges.Nodes {
		details.kinds[nodeTicket] = string(node.Facts[facts.NodeKind])
		if d := string(node.Facts[facts.ParamDefault]); d != "" {
			details.defaultParams[nodeTicket], err = strconv.Atoi(d)
			if err != nil {
				return nil, fmt.Errorf("invalid default parameter %q: %v", d, err)
			}
		}
		if sersig := node.Facts[facts.Code]; len(sersig) != 0 {
			sig := &cpb.MarkedSource{}
			if proto.Unmarshal(sersig, sig) == nil {
				details.signatures[nodeTicket] = sig
			}
		}
	}
	for _, set := range allEdges.EdgeSets {
		if group := set.Groups[edges.Param]; group != nil {
			details.params = extractParams(group.Edge)
		}
	}
	return &details, nil
}

// slowSignatureState holds the state necessary to traverse MarkedSource to produce signatures.
type slowSignatureState struct {
	ctx     context.Context
	service Service
	// The node whose MarkedSource we're traversing.
	ticket string
	// ticket's kind.
	kind string
	// The MarkedSource being traversed.
	marked *cpb.MarkedSource
	// ticket's first default parameter, or negative if there is no such parameter.
	defaultParam int
	// Details about ticket and its params.
	details *SignatureDetails
	// The query depth.
	depth int
}

// slowSignatureTraverseLevel traverses MarkedSource data for one particular ticket. If it needs to
// issue graph queries for other tickets, it will defer to slowSignatureLevel (at the next depth).
func slowSignatureTraverseLevel(s slowSignatureState) (*cpb.MarkedSource, error) {
	// Copy the template MarkedSource to the output MarkedSource. We may elaborate the template depending on its kind.
	// Data that are irrelevant for static MarkedSource nodes (like LookupIndex) are dropped.
	outsig := &cpb.MarkedSource{
		Kind:                 s.marked.Kind,
		PreText:              s.marked.PreText,
		PostChildText:        s.marked.PostChildText,
		PostText:             s.marked.PostText,
		DefaultChildrenCount: s.marked.DefaultChildrenCount,
		AddFinalListToken:    s.marked.AddFinalListToken,
		Link:                 s.marked.Link,
	}
	switch s.marked.Kind {
	case cpb.MarkedSource_PARAMETER_LOOKUP_BY_PARAM, cpb.MarkedSource_PARAMETER_LOOKUP_BY_PARAM_WITH_DEFAULTS:
		outsig.Kind = cpb.MarkedSource_PARAMETER
		if int(s.marked.LookupIndex) >= len(s.details.params) {
			log.Printf("Lookup index out of bounds for ticket %v", s.ticket)
			return outsig, nil
		}
		for _, param := range s.details.params[int(s.marked.LookupIndex):] {
			outc, err := slowSignatureLevel(slowSignatureState{
				ctx:          s.ctx,
				service:      s.service,
				ticket:       param,
				kind:         s.details.kinds[param],
				marked:       s.details.signatures[param],
				defaultParam: s.details.defaultParam(param),
				depth:        s.depth + 1,
			})
			if err != nil || outc == nil {
				log.Printf("Couldn't render node %v's param (ticket %v): %v", s.ticket, param, err)
			} else {
				outsig.Child = append(outsig.Child, outc)
			}
		}
		if s.marked.Kind == cpb.MarkedSource_PARAMETER_LOOKUP_BY_PARAM_WITH_DEFAULTS && s.defaultParam >= 0 {
			count := len(s.details.params) - s.defaultParam
			if int(s.marked.LookupIndex) > s.defaultParam {
				count -= int(s.marked.LookupIndex) - s.defaultParam
			}
			if count < 0 {
				log.Printf("Default parameter count of %v underflows parameter list", s.ticket)
			} else {
				outsig.DefaultChildrenCount = uint32(count)
			}
		}
	case cpb.MarkedSource_LOOKUP_BY_PARAM:
		if int(s.marked.LookupIndex) >= len(s.details.params) {
			outsig.Kind = cpb.MarkedSource_BOX
			log.Printf("Couldn't look up param %v of node with ticket %v: param out of range", s.marked.LookupIndex, s.ticket)
		} else {
			param := s.details.params[s.marked.LookupIndex]
			maybesig, err := slowSignatureLevel(slowSignatureState{
				ctx:          s.ctx,
				service:      s.service,
				ticket:       param,
				kind:         s.details.kinds[param],
				marked:       s.details.signatures[param],
				defaultParam: s.details.defaultParam(param),
				depth:        s.depth + 1,
			})
			if err != nil || maybesig == nil {
				log.Printf("Couldn't render param %v of node %v with ticket %v: %v", s.marked.LookupIndex, s.ticket, param, err)
			} else {
				outsig = maybesig
			}
		}
	}
	// Expand children in the context of the current node.
	for _, c := range s.marked.Child {
		outc, err := slowSignatureTraverseLevel(slowSignatureState{
			ctx:          s.ctx,
			service:      s.service,
			ticket:       s.ticket,
			kind:         s.kind,
			marked:       c,
			defaultParam: s.defaultParam,
			depth:        s.depth,
			details:      s.details,
		})
		if err != nil || outc == nil {
			log.Printf("Signature on node %v had a bad child", s.ticket)
		} else {
			outsig.Child = append(outsig.Child, outc)
		}
	}
	return outsig, nil
}

// slowSignatureLevel issues graph queries to traverse s.ticket's MarkedSource.
// In particular, it replaces the details field of s with an appropriate details struct
// for s.ticket, then calls recursively to slowSignatureTraverseLevel.
func slowSignatureLevel(s slowSignatureState) (*cpb.MarkedSource, error) {
	if s.depth > maxFormatExpansions {
		return &cpb.MarkedSource{Kind: cpb.MarkedSource_BOX, PreText: "..."}, nil
	}
	if s.kind == "" {
		log.Printf("Node %v missing kind", s.ticket)
	}
	details, err := findSignatureDetails(s.ctx, s.service, s.ticket)
	if err != nil {
		return nil, fmt.Errorf("during findSignatureDetails in slowSignatureLevel: %v", err)
	}
	// tapp nodes allow their 0th param to provide a format.
	if s.marked == nil && s.kind == nodes.TApp && len(details.params) > 0 && details.signatures[details.params[0]] != nil {
		s.marked = details.signatures[details.params[0]]
	}
	// Try looking for a meta node as a last resort.
	if s.marked == nil {
		s.marked, err = slowLookupMeta(s.ctx, s.service, getLanguage(s.ticket), s.kind)
		if err != nil {
			return nil, fmt.Errorf("during slowLookupMeta in slowSignatureLevel: %v", err)
		}
		if s.marked == nil {
			log.Printf("Could not deduce signature for node %v", s.ticket)
			return nil, nil
		}
	}
	return slowSignatureTraverseLevel(slowSignatureState{
		ctx:          s.ctx,
		service:      s.service,
		ticket:       s.ticket,
		kind:         s.kind,
		marked:       s.marked,
		defaultParam: s.defaultParam,
		depth:        s.depth,
		details:      details,
	})
}

// SlowSignature generates an cpb.MarkedSource given a ticket.
func SlowSignature(ctx context.Context, service Service, ticket string) (*cpb.MarkedSource, error) {
	if *disableSlowSignatures || *disableAllSlowPaths {
		log.Println("SlowSignature disabled")
		return nil, nil
	}
	start := time.Now()
	defer func() { log.Printf("SlowSignature: %s", time.Since(start)) }()
	log.Println("WARNING: performing slow-lookup of signature")
	req := &gpb.NodesRequest{
		Ticket: []string{ticket},
		Filter: []string{facts.NodeKind, facts.Code, facts.ParamDefault},
	}
	nodes, err := service.Nodes(ctx, req)
	if err != nil {
		return nil, fmt.Errorf("during Nodes in SlowSignature: %v", err)
	}
	if len(nodes.Nodes) == 0 {
		return nil, fmt.Errorf("could not find node %v", ticket)
	}

	var kind string
	var sig *cpb.MarkedSource
	defaultParam := -1
	for _, node := range nodes.Nodes {
		kind = string(node.Facts[facts.NodeKind])
		if node.Facts[facts.ParamDefault] != nil {
			defaultParam, err = strconv.Atoi(string(node.Facts[facts.ParamDefault]))
			if err != nil {
				log.Printf("Node %v has an invalid param/default fact", ticket)
			}
		}
		if node.Facts[facts.Code] != nil {
			sig = &cpb.MarkedSource{}
			err = proto.Unmarshal(node.Facts[facts.Code], sig)
			if err != nil {
				return nil, fmt.Errorf("could not unmarshal signature: %v", err)
			}
		}
	}
	result, err := slowSignatureLevel(slowSignatureState{
		ctx:          ctx,
		service:      service,
		ticket:       ticket,
		kind:         kind,
		marked:       sig,
		defaultParam: defaultParam,
		depth:        0,
	})
	return result, err
}

// Data from a doc node that documents some other ticket.
type associatedDocNode struct {
	rawText string
	link    []*cpb.Link
	// Tickets this associatedDocNode documents.
	documented []string
}

// A document being compiled for Documentation.
type preDocument struct {
	// The Document being compiled.
	document *xpb.DocumentationReply_Document
	// Maps from tickets to doc nodes.
	docNode map[string]*associatedDocNode
}

// Context for executing Documentation calls.
type documentDetails struct {
	// Maps doc tickets to associatedDocNodes.
	docTicketToAssocNode map[string]*associatedDocNode
	// Maps tickets to Documents being prepared. More than one ticket may map to the same preDocument
	// (e.g., if they are in the same equivalence class defined by expandDefRelatedNodeSet.
	ticketToPreDocument map[string]*preDocument
	// Maps tickets to MarkedSource.
	ticketToMarkedSource map[string]*cpb.MarkedSource
	// Tickets of known doc nodes.
	docs stringset.Set
	// Maps tickets to definition anchors when they're known.
	ticketToDefinition map[string]*xpb.Anchor
}

// getDocRelatedNodes fills details with information about the associated doc nodes of allTickets.
func getDocRelatedNodes(ctx context.Context, service Service, details documentDetails, allTickets stringset.Set) error {
	// We can't ask for text facts here (since they get filtered out).
	dreq := &gpb.EdgesRequest{
		Ticket:   allTickets.Elements(),
		Kind:     []string{edges.Mirror(edges.Documents)},
		PageSize: math.MaxInt32,
		Filter:   []string{facts.NodeKind},
	}
	dedges, err := AllEdges(ctx, service, dreq)
	if err != nil {
		return fmt.Errorf("couldn't AllEdges: %v", err)
	}
	for nodeTicket, node := range dedges.Nodes {
		kind := string(node.Facts[facts.NodeKind])
		if kind == nodes.Doc {
			details.docs.Add(nodeTicket)
		}
	}
	for sourceTicket, set := range dedges.EdgeSets {
		if group := set.Groups[edges.Mirror(edges.Documents)]; group != nil {
			for _, edge := range group.Edge {
				if preDoc := details.ticketToPreDocument[sourceTicket]; preDoc != nil {
					assocNode := details.docTicketToAssocNode[edge.TargetTicket]
					if assocNode == nil {
						assocNode = &associatedDocNode{}
						details.docTicketToAssocNode[edge.TargetTicket] = assocNode
					}
					assocNode.documented = append(assocNode.documented, sourceTicket)
					preDoc.docNode[edge.TargetTicket] = assocNode
				}
			}
		}
	}
	return nil
}

// getDocText gets text for doc nodes we've found.
func getDocText(ctx context.Context, service Service, details documentDetails) error {
	if details.docs.Empty() {
		return nil
	}
	nreq := &gpb.NodesRequest{
		Ticket: details.docs.Elements(),
		Filter: []string{facts.Text},
	}
	nodes, err := service.Nodes(ctx, nreq)
	if err != nil {
		return fmt.Errorf("error in Nodes during getDocText: %v", err)
	}
	for nodeTicket, node := range nodes.Nodes {
		if text, ok := node.Facts[facts.Text]; ok {
			if assocNode := details.docTicketToAssocNode[nodeTicket]; assocNode != nil {
				assocNode.rawText = string(text)
			}
		}
	}
	return nil
}

// getDocLinks gets links for doc nodes we've found.
func getDocLinks(ctx context.Context, service Service, details documentDetails) error {
	if details.docs.Empty() {
		return nil
	}
	preq := &gpb.EdgesRequest{
		Ticket:   details.docs.Elements(),
		Kind:     []string{edges.Param},
		PageSize: math.MaxInt32,
	}
	pedges, err := AllEdges(ctx, service, preq)
	if err != nil {
		return fmt.Errorf("error in Edges during getDocLinks: %v", err)
	}
	for sourceTicket, set := range pedges.EdgeSets {
		if assocDoc := details.docTicketToAssocNode[sourceTicket]; assocDoc != nil {
			for _, group := range set.Groups {
				params := extractParams(group.Edge)
				assocDoc.link = make([]*cpb.Link, len(params))
				for i, param := range extractParams(group.Edge) {
					assocDoc.link[i] = &cpb.Link{Definition: []string{param}}
				}
			}
		}
	}
	return nil
}

// compilePreDocument finishes and returns the Document attached to ticket's preDocument.
func compilePreDocument(ctx context.Context, service Service, details documentDetails, ticket string, preDocument *preDocument) (*xpb.DocumentationReply_Document, error) {
	document := preDocument.document
	document.MarkedSource = details.ticketToMarkedSource[ticket]
	if document.MarkedSource == nil {
		log.Printf("WARNING: No signature stored for %v", ticket)
		sig, err := SlowSignature(ctx, service, ticket)
		if err != nil {
			return nil, fmt.Errorf("can't get SlowSignature for %v: %v", ticket, err)
		}
		document.MarkedSource = sig
	}
	text := &xpb.Printable{}
	document.Text = text
	for _, assocDoc := range preDocument.docNode {
		// Assume the longest document is the best one to show.
		// TODO(zarko): Amend the API to pass down all documents and allow the
		// client to make this decision (informed by provenance).
		if len(assocDoc.rawText) > len(text.RawText) {
			text.RawText = assocDoc.rawText
			text.Link = assocDoc.link
		}
	}
	return document, nil
}

func linkTickets(p *xpb.Printable, s stringset.Set) {
	if p == nil {
		return
	}
	for _, l := range p.Link {
		for _, d := range l.Definition {
			s.Add(d)
		}
	}
}

// signatureLinkTickets inserts into s all of the Definition tickets for all
// links in sg and its children.
func signatureLinkTickets(sg *cpb.MarkedSource, s stringset.Set) {
	if sg == nil {
		return
	}
	for _, c := range sg.Child {
		signatureLinkTickets(c, s)
	}
	for _, l := range sg.Link {
		for _, d := range l.Definition {
			s.Add(d)
		}
	}
}

// mergeDocumentationReply adds from's document to into.Document[0].Children and merges its Nodes and DefinitionLocations.
func mergeDocumentationReply(from, into *xpb.DocumentationReply) {
	into.Document[0].Children = append(into.Document[0].Children, from.Document...)
	for k, v := range from.Nodes {
		if into.Nodes == nil {
			into.Nodes = make(map[string]*cpb.NodeInfo)
		}
		into.Nodes[k] = v
	}
	for k, v := range from.DefinitionLocations {
		if into.DefinitionLocations == nil {
			into.DefinitionLocations = make(map[string]*xpb.Anchor)
		}
		into.DefinitionLocations[k] = v
	}
}

// slowMultilevelDocumentation is an implementation of the Documentation API (with IncludeChildren set) based on other APIs.
func slowMultilevelDocumentation(ctx context.Context, service Service, req *xpb.DocumentationRequest) (*xpb.DocumentationReply, error) {
	start := time.Now()
	defer func() { log.Printf("slowMultilevelDocumentation: %s", time.Since(start)) }()
	log.Println("WARNING: performing slow multilevel lookup of documentation")
	if len(req.Ticket) != 1 {
		return nil, fmt.Errorf("only expected one top-level ticket in slowMultilevelDocumentation")
	}
	tickets, err := FixTickets(req.Ticket)
	if err != nil {
		return nil, err
	}
	var subTickets stringset.Set
	if err := forAllEdges(ctx, service, stringset.New(tickets...), []string{edges.Mirror(edges.ChildOf)},
		func(_, target, targetKind, _ string) error {
			if targetKind != nodes.Anchor {
				subTickets.Add(target)
			}
			return nil
		}); err != nil {
		return nil, fmt.Errorf("error getting childof edges in slowMultilevelDocumentation: %v", err)
	}
	if len(subTickets) == 1 {
		// The source ticket might have been an abs.
		var subSubTickets stringset.Set
		childTicket := subTickets.Elements()[0]
		isAbsChild := false
		if err := forAllEdges(ctx, service, stringset.New(childTicket), []string{edges.Mirror(edges.ChildOf), edges.ChildOf},
			func(_, target, targetKind, edgeKind string) error {
				if edgeKind == edges.ChildOf {
					if targetKind == nodes.Abs {
						isAbsChild = true
					}
				} else if targetKind != nodes.Anchor {
					subSubTickets.Add(target)
				}
				return nil
			}); err != nil {
			return nil, fmt.Errorf("error getting childof edges in slowMultilevelDocumentation: %v", err)
		}
		if isAbsChild {
			// Include the abs child's children along with the abs's children.
			subTickets.Update(subSubTickets)
			subTickets.Discard(childTicket)
		}
	}
	retDoc, err := SlowDocumentation(ctx, service, &xpb.DocumentationRequest{
		Ticket: []string{tickets[0]},
		Filter: req.Filter,
	})
	if err != nil {
		return nil, fmt.Errorf("error getting root document in slowMultilevelDocumentation: %v", err)
	}
	if len(retDoc.Document) != 1 {
		return nil, fmt.Errorf("unexpected number (%v) of root documents in slowMultilevelDocumentation", len(retDoc.Document))
	}
	for subTicket := range subTickets {
		doc, err := SlowDocumentation(ctx, service, &xpb.DocumentationRequest{
			Ticket: []string{subTicket},
			Filter: req.Filter,
		})
		if err != nil {
			return nil, fmt.Errorf("error getting subdocument in slowMultilevelDocumentation: %v", err)
		}
		mergeDocumentationReply(doc, retDoc)
	}
	return retDoc, nil
}

const (
	// The maximum number of times SlowDocumentation will flip CrossReferences pages.
	maxDocumentationXrefPages = 10
)

// SlowDocumentation is an implementation of the Documentation API built from other APIs.
func SlowDocumentation(ctx context.Context, service Service, req *xpb.DocumentationRequest) (*xpb.DocumentationReply, error) {
	if *disableSlowDocumentation || *disableAllSlowPaths {
		log.Println("SlowDocumentation disabled")
		return &xpb.DocumentationReply{}, nil
	}
	if req.IncludeChildren {
		return slowMultilevelDocumentation(ctx, service, req)
	}
	start := time.Now()
	defer func() { log.Printf("SlowDocumentation: %s", time.Since(start)) }()
	log.Println("WARNING: performing slow-lookup of documentation")
	tickets, err := FixTickets(req.Ticket)
	if err != nil {
		return nil, err
	}
	details := documentDetails{
		ticketToPreDocument:  make(map[string]*preDocument),
		docTicketToAssocNode: make(map[string]*associatedDocNode),
		docs:                 make(stringset.Set),
		ticketToMarkedSource: make(map[string]*cpb.MarkedSource),
		ticketToDefinition:   make(map[string]*xpb.Anchor),
	}
	// We assume that expandDefRelatedNodeSet will return disjoint sets (and thus we can treat them as equivalence classes
	// with the original request's ticket as the characteristic element).
	var allTickets stringset.Set
	var ambiguousDefinitionTickets stringset.Set
	var pageToken string
	pagesFlipped := 0
	// Try to get signatures from CrossReferences, which is potentially faster.
	for ; pagesFlipped < maxDocumentationXrefPages; pagesFlipped++ {
		xReply, err := service.CrossReferences(ctx, &xpb.CrossReferencesRequest{
			Ticket:                 tickets,
			PageToken:              pageToken,
			DefinitionKind:         xpb.CrossReferencesRequest_BINDING_DEFINITIONS,
			ExperimentalSignatures: true,
		})
		if err != nil {
			return nil, err
		}
		for ticket, xrefSet := range xReply.CrossReferences {
			details.ticketToMarkedSource[ticket] = xrefSet.MarkedSource
			if !ambiguousDefinitionTickets.Contains(ticket) {
				for _, def := range xrefSet.Definition {
					if def.Anchor.Kind == edges.DefinesBinding {
						if details.ticketToDefinition[ticket] != nil {
							delete(details.ticketToDefinition, ticket)
							ambiguousDefinitionTickets.Add(ticket)
						} else {
							details.ticketToDefinition[ticket] = def.Anchor
						}
					}
				}
			}
		}
		pageToken = xReply.NextPageToken
		if pageToken == "" {
			break
		}
	}
	if pagesFlipped == maxDocumentationXrefPages {
		log.Println("WARNING: Exceeded maxDocumentationXrefPages")
	}
	for _, ticket := range tickets {
		// TODO(zarko): Include outbound override edges.
		// TODO(zarko): Optimize this if possible from merge_with field in xref data
		ticketSet, err := expandDefRelatedNodeSet(ctx, service, stringset.New(ticket) /*includeOverrides=*/, false)
		if err != nil {
			return nil, fmt.Errorf("couldn't expandDefRelatedNodeSet in Documentation: %v", err)
		}
		document := &preDocument{document: &xpb.DocumentationReply_Document{Ticket: ticket}, docNode: make(map[string]*associatedDocNode)}
		for ticket := range ticketSet {
			allTickets.Add(ticket)
			details.ticketToPreDocument[ticket] = document
		}
	}
	if err = getDocRelatedNodes(ctx, service, details, allTickets); err != nil {
		return nil, fmt.Errorf("couldn't getDocRelatedNodes: %v", err)
	}
	if err = getDocText(ctx, service, details); err != nil {
		return nil, fmt.Errorf("couldn't getDocText: %v", err)
	}
	if err = getDocLinks(ctx, service, details); err != nil {
		return nil, fmt.Errorf("couldn't getDocLinks: %v", err)
	}
	// Finish up and attach the Documents we've built to the reply.
	reply := &xpb.DocumentationReply{}
	var definitionSet stringset.Set
	for _, ticket := range tickets {
		preDocument, found := details.ticketToPreDocument[ticket]
		if !found {
			continue
		}
		document, err := compilePreDocument(ctx, service, details, ticket, preDocument)
		if err != nil {
			return nil, fmt.Errorf("during compilePreDocument for %v: %v", ticket, err)
		}
		definitionSet.Add(document.Ticket)
		linkTickets(document.Text, definitionSet)
		signatureLinkTickets(document.MarkedSource, definitionSet)
		reply.Document = append(reply.Document, document)
	}
	var nodes map[string]*cpb.NodeInfo
	if len(definitionSet) != 0 {
		nodesReply, err := service.Nodes(ctx, &gpb.NodesRequest{
			Filter: req.Filter,
			Ticket: definitionSet.Elements(),
		})
		if err != nil {
			return nil, fmt.Errorf("during Nodes: %v", err)
		}
		nodes = nodesReply.Nodes
	}
	for ticket := range details.ticketToDefinition {
		definitionSet.Discard(ticket)
	}
	definitionSet.Remove(ambiguousDefinitionTickets)
	defs, err := SlowDefinitions(ctx, service, definitionSet.Elements())
	if err != nil {
		return nil, fmt.Errorf("during SlowDefinitions for %v: %v", definitionSet, err)
	}
	if defs == nil && len(details.ticketToDefinition) != 0 {
		defs = make(map[string]*xpb.Anchor)
	}
	for ticket, def := range details.ticketToDefinition {
		defs[ticket] = def
	}
	if len(defs) != 0 {
		reply.DefinitionLocations = make(map[string]*xpb.Anchor, len(defs))
		for _, def := range defs {
			reply.DefinitionLocations[def.Ticket] = def
		}
	}
	if len(nodes) != 0 {
		reply.Nodes = make(map[string]*cpb.NodeInfo, len(nodes))
		for node, info := range nodes {
			if def, ok := defs[node]; ok {
				info.Definition = def.Ticket
			}
			reply.Nodes[node] = info
		}
	}
	return reply, nil
}

type grpcClient struct {
	xpb.XRefServiceClient
	gpb.GraphServiceClient
}

// Nodes implements part of the Service interface.
func (w *grpcClient) Nodes(ctx context.Context, req *gpb.NodesRequest) (*gpb.NodesReply, error) {
	return w.GraphServiceClient.Nodes(ctx, req)
}

// Edges implements part of the Service interface.
func (w *grpcClient) Edges(ctx context.Context, req *gpb.EdgesRequest) (*gpb.EdgesReply, error) {
	return w.GraphServiceClient.Edges(ctx, req)
}

// Decorations implements part of the Service interface.
func (w *grpcClient) Decorations(ctx context.Context, req *xpb.DecorationsRequest) (*xpb.DecorationsReply, error) {
	return w.XRefServiceClient.Decorations(ctx, req)
}

// CrossReferences implements part of the Service interface.
func (w *grpcClient) CrossReferences(ctx context.Context, req *xpb.CrossReferencesRequest) (*xpb.CrossReferencesReply, error) {
	return w.XRefServiceClient.CrossReferences(ctx, req)
}

// Documentation implements part of the Service interface.
func (w *grpcClient) Documentation(ctx context.Context, req *xpb.DocumentationRequest) (*xpb.DocumentationReply, error) {
	return w.XRefServiceClient.Documentation(ctx, req)
}

// GRPC returns an xrefs Service backed by the given GRPC client and context.
func GRPC(xref xpb.XRefServiceClient, graph gpb.GraphServiceClient) Service {
	return &grpcClient{XRefServiceClient: xref, GraphServiceClient: graph}
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

// Documentation implements part of the Service interface.
func (w *webClient) Documentation(ctx context.Context, q *xpb.DocumentationRequest) (*xpb.DocumentationReply, error) {
	var reply xpb.DocumentationReply
	return &reply, web.Call(w.addr, "documentation", q, &reply)
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
//   GET /documentation
//     Request: JSON encoded xrefs.DocumentationRequest
//     Response: JSON encoded xrefs.DocumentationReply
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
	mux.HandleFunc("/documentation", func(w http.ResponseWriter, r *http.Request) {
		start := time.Now()
		defer func() {
			log.Printf("xrefs.Documentation:\t%s", time.Since(start))
		}()
		var req xpb.DocumentationRequest
		if err := web.ReadJSONBody(r, &req); err != nil {
			http.Error(w, err.Error(), http.StatusBadRequest)
			return
		}
		reply, err := xs.Documentation(ctx, &req)
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

		var req gpb.NodesRequest
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

		var req gpb.EdgesRequest
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

// ByName orders a slice of facts by their fact names.
type ByName []*cpb.Fact

func (s ByName) Len() int           { return len(s) }
func (s ByName) Less(i, j int) bool { return s[i].Name < s[j].Name }
func (s ByName) Swap(i, j int)      { s[i], s[j] = s[j], s[i] }

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
