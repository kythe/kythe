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

// Package assemble provides functions to build the various components (nodes,
// edges, and decorations) of an xrefs serving table.
package assemble

import (
	"fmt"
	"log"
	"sort"
	"strconv"

	"kythe.io/kythe/go/services/graphstore"
	"kythe.io/kythe/go/util/kytheuri"
	"kythe.io/kythe/go/util/pager"
	"kythe.io/kythe/go/util/schema"
	"kythe.io/kythe/go/util/stringset"

	"golang.org/x/net/context"

	srvpb "kythe.io/kythe/proto/serving_proto"
	spb "kythe.io/kythe/proto/storage_proto"
)

// Source is a collection of facts and edges with a common source.
type Source struct {
	Ticket string

	Facts map[string][]byte
	Edges map[string][]string
}

// Node returns the Source as a srvpb.Node.
func (s *Source) Node() *srvpb.Node {
	facts := make([]*srvpb.Fact, 0, len(s.Facts))
	for name, value := range s.Facts {
		facts = append(facts, &srvpb.Fact{Name: name, Value: value})
	}
	sort.Sort(ByName(facts))
	return &srvpb.Node{
		Ticket: s.Ticket,
		Fact:   facts,
	}
}

// SourceFromEntries returns a new Source from the given a set of entries with
// the same source VName.
func SourceFromEntries(entries []*spb.Entry) *Source {
	if len(entries) == 0 {
		return nil
	}

	src := &Source{
		Ticket: kytheuri.ToString(entries[0].Source),
		Facts:  make(map[string][]byte),
		Edges:  make(map[string][]string),
	}

	edgeTargets := make(map[string]stringset.Set)

	for _, e := range entries {
		if graphstore.IsEdge(e) {
			tgts, ok := edgeTargets[e.EdgeKind]
			if !ok {
				tgts = stringset.New()
				edgeTargets[e.EdgeKind] = tgts
			}
			tgts.Add(kytheuri.ToString(e.Target))
		} else {
			src.Facts[e.FactName] = e.FactValue
		}
	}
	for kind, targets := range edgeTargets {
		src.Edges[kind] = targets.Slice()
		sort.Strings(src.Edges[kind])
	}

	return src
}

// FactsToMap returns a map from fact name to value.
func FactsToMap(facts []*srvpb.Fact) map[string][]byte {
	m := make(map[string][]byte, len(facts))
	for _, f := range facts {
		m[f.Name] = f.Value
	}
	return m
}

// GetFact returns the value of the first fact in facts with the given name; otherwise returns nil.
func GetFact(facts []*srvpb.Fact, name string) []byte {
	for _, f := range facts {
		if f.Name == name {
			return f.Value
		}
	}
	return nil
}

// PartialReverseEdges returns the set of partial reverse edges from the given source.  Each
// reversed Edge has its Target fully populated and its Source will have no facts.  To ensure every
// node has at least 1 Edge, the first Edge will be a self-edge without a Kind or Target.  To reduce
// the size of edge sets, each Target will have any text facts filtered (see FilterTextFacts).
func PartialReverseEdges(src *Source) []*srvpb.Edge {
	node := src.Node()

	edges := []*srvpb.Edge{{
		Source: node, // self-edge to ensure every node has at least 1 edge
	}}

	targetNode := FilterTextFacts(node)

	for kind, targets := range src.Edges {
		rev := schema.MirrorEdge(kind)
		for _, target := range targets {
			edges = append(edges, &srvpb.Edge{
				Source: &srvpb.Node{Ticket: target},
				Kind:   rev,
				Target: targetNode,
			})
		}
	}

	return edges
}

// FilterTextFacts returns a new Node without any text facts.
func FilterTextFacts(n *srvpb.Node) *srvpb.Node {
	res := &srvpb.Node{
		Ticket: n.Ticket,
		Fact:   make([]*srvpb.Fact, 0, len(n.Fact)),
	}
	for _, f := range n.Fact {
		switch f.Name {
		case schema.TextFact, schema.TextEncodingFact:
			// Skip large text facts for targets
		default:
			res.Fact = append(res.Fact, f)
		}
	}
	return res
}

// DecorationFragmentBuilder builds pieces of FileDecorations given an ordered (see AddEdge) stream
// of completed Edges.  Each fragment constructed (either by AddEdge or Flush) will be emitted using
// the Output function in the builder.  There are two types of fragments: file fragments (which have
// their SourceText, FileTicket, and Encoding set) and decoration fragments (which have only
// Decoration set).
type DecorationFragmentBuilder struct {
	Output func(ctx context.Context, file string, fragment *srvpb.FileDecorations) error

	anchor  *srvpb.Anchor
	decor   []*srvpb.FileDecorations_Decoration
	parents []string
}

// AddEdge adds the given edge to the current fragment (or emits some fragments and starts a new
// fragment with e).  AddEdge must be called in GraphStore sorted order of the Edges with the
// beginning to every set of edges with the same Source having a signaling Edge with only its Source
// set (no Kind or Target).  Otherwise, every Edge must have a completed Source, Kind, and Target.
// Flush must be called after every call to AddEdge in order to output any remaining fragments.
func (b *DecorationFragmentBuilder) AddEdge(ctx context.Context, e *srvpb.Edge) error {
	if e.Target == nil {
		// Beginning of a set of edges with a new Source
		if err := b.Flush(ctx); err != nil {
			return err
		}

		srcFacts := FactsToMap(e.Source.Fact)

		switch string(srcFacts[schema.NodeKindFact]) {
		case schema.FileKind:
			if err := b.Output(ctx, e.Source.Ticket, &srvpb.FileDecorations{
				File: &srvpb.File{
					Ticket:   e.Source.Ticket,
					Text:     srcFacts[schema.TextFact],
					Encoding: string(srcFacts[schema.TextEncodingFact]),
				},
			}); err != nil {
				return err
			}
		case schema.AnchorKind:
			anchorStart, err := strconv.Atoi(string(srcFacts[schema.AnchorStartFact]))
			if err != nil {
				log.Printf("Error parsing anchor start offset %q: %v",
					string(srcFacts[schema.AnchorStartFact]), err)
				return nil
			}
			anchorEnd, err := strconv.Atoi(string(srcFacts[schema.AnchorEndFact]))
			if err != nil {
				log.Printf("Error parsing anchor end offset %q: %v",
					string(srcFacts[schema.AnchorEndFact]), err)
				return nil
			}

			b.anchor = &srvpb.Anchor{
				Ticket:      e.Source.Ticket,
				StartOffset: int32(anchorStart),
				EndOffset:   int32(anchorEnd),
			}
		}
		return nil
	} else if b.anchor == nil {
		// We don't care about edges for non-anchors
		return nil
	}

	if e.Kind == schema.ChildOfEdge && string(GetFact(e.Target.Fact, schema.NodeKindFact)) == schema.FileKind {
		b.parents = append(b.parents, e.Target.Ticket)
	} else {
		b.decor = append(b.decor, &srvpb.FileDecorations_Decoration{
			Anchor: b.anchor,
			Kind:   e.Kind,
			Target: e.Target,
		})

		if len(b.parents) > 0 {
			fd := &srvpb.FileDecorations{Decoration: b.decor}
			for _, parent := range b.parents {
				if err := b.Output(ctx, parent, fd); err != nil {
					return err
				}
			}
			b.decor = nil
		}
	}

	return nil
}

// Flush outputs any remaining fragments that are being built.  It is safe, but usually unnecessary,
// to call Flush in between sets of Edges with the same Source.  This also means that
// DecorationFragmentBuilder can be used to construct decoration fragments in parallel by
// partitioning edges along the same boundaries.
func (b *DecorationFragmentBuilder) Flush(ctx context.Context) error {
	defer func() {
		b.anchor = nil
		b.decor = nil
		b.parents = nil
	}()

	if len(b.decor) > 0 && len(b.parents) > 0 {
		fd := &srvpb.FileDecorations{Decoration: b.decor}
		for _, parent := range b.parents {
			if err := b.Output(ctx, parent, fd); err != nil {
				return err
			}
		}
	}
	return nil
}

// ByOffset sorts file decorations by their byte offsets.
type ByOffset []*srvpb.FileDecorations_Decoration

func (s ByOffset) Len() int      { return len(s) }
func (s ByOffset) Swap(i, j int) { s[i], s[j] = s[j], s[i] }
func (s ByOffset) Less(i, j int) bool {
	if s[i].Anchor.StartOffset < s[j].Anchor.StartOffset {
		return true
	} else if s[i].Anchor.StartOffset > s[j].Anchor.StartOffset {
		return false
	} else if s[i].Anchor.EndOffset == s[j].Anchor.EndOffset {
		if s[i].Kind == s[j].Kind {
			return s[i].Target.Ticket < s[j].Target.Ticket
		}
		return s[i].Kind < s[j].Kind
	}
	return s[i].Anchor.EndOffset < s[j].Anchor.EndOffset
}

// EdgeSetBuilder constructs a set of PagedEdgeSets and EdgePages from a
// sequence of Nodes and EdgeSet_Groups.  For each set of groups with the same
// source, a call to StartEdgeSet must precede.  All EdgeSet_Groups for the same
// source are then assumed to be given sequentially to AddGroup, secondarily
// ordered by the group's edge kind.  If given in this order, Output will be
// given exactly 1 PagedEdgeSet per source with as few EdgeSet_Group per edge
// kind as to satisfy MaxEdgePageSize (MaxEdgePageSize == 0 indicates that there
// will be exactly 1 edge group per edge kind).  If not given in this order, no
// guarantees can be made.  Flush must be called after the final call to
// AddGroup.
type EdgeSetBuilder struct {
	// MaxEdgePageSize is maximum number of edges that are allowed in the
	// PagedEdgeSet and any EdgePage.  If MaxEdgePageSize <= 0, no paging is
	// attempted.
	MaxEdgePageSize int

	// Output is used to emit each PagedEdgeSet constructed.
	Output func(context.Context, *srvpb.PagedEdgeSet) error
	// OutputPage is used to emit each EdgePage constructed.
	OutputPage func(context.Context, *srvpb.EdgePage) error

	pager *pager.SetPager
}

func (b *EdgeSetBuilder) constructPager() *pager.SetPager {
	// Head:  *srvpb.Node
	// Set:   *srvpb.PagedEdgeSet
	// Group: *srvpb.EdgeSet_Group
	return &pager.SetPager{
		MaxPageSize: b.MaxEdgePageSize,

		NewSet: func(hd pager.Head) pager.Set {
			return &srvpb.PagedEdgeSet{
				EdgeSet: &srvpb.EdgeSet{Source: hd.(*srvpb.Node)},
			}
		},
		Combine: func(l, r pager.Group) pager.Group {
			lg, rg := l.(*srvpb.EdgeSet_Group), r.(*srvpb.EdgeSet_Group)
			if lg.Kind != rg.Kind {
				return nil
			}
			lg.Target = append(lg.Target, rg.Target...)
			return lg
		},
		Split: func(sz int, g pager.Group) (l, r pager.Group) {
			eg := g.(*srvpb.EdgeSet_Group)
			neg := &srvpb.EdgeSet_Group{
				Kind:   eg.Kind,
				Target: eg.Target[:sz],
			}
			eg.Target = eg.Target[sz:]
			return neg, eg
		},
		Size: func(g pager.Group) int { return len(g.(*srvpb.EdgeSet_Group).Target) },

		OutputSet: func(ctx context.Context, total int, s pager.Set, grps []pager.Group) error {
			pes := s.(*srvpb.PagedEdgeSet)

			// pes.EdgeSet.Group = []*srvpb.EdgeSet_Group(grps)
			pes.EdgeSet.Group = make([]*srvpb.EdgeSet_Group, len(grps))
			for i, g := range grps {
				pes.EdgeSet.Group[i] = g.(*srvpb.EdgeSet_Group)
			}

			sort.Sort(byEdgeKind(pes.EdgeSet.Group))
			sort.Sort(byPageKind(pes.PageIndex))
			pes.TotalEdges = int32(total)

			return b.Output(ctx, pes)
		},
		OutputPage: func(ctx context.Context, s pager.Set, g pager.Group) error {
			pes := s.(*srvpb.PagedEdgeSet)
			eviction := g.(*srvpb.EdgeSet_Group)

			src := pes.EdgeSet.Source.Ticket
			key := newPageKey(src, len(pes.PageIndex))

			// Output the EdgePage and add it to the page indices
			if err := b.OutputPage(ctx, &srvpb.EdgePage{
				PageKey:      key,
				SourceTicket: src,
				EdgesGroup:   eviction,
			}); err != nil {
				return fmt.Errorf("error emitting EdgePage: %v", err)
			}
			pes.PageIndex = append(pes.PageIndex, &srvpb.PageIndex{
				PageKey:   key,
				EdgeKind:  eviction.Kind,
				EdgeCount: int32(len(eviction.Target)),
			})
			return nil
		},
	}
}

// StartEdgeSet begins a new EdgeSet for the given source node, possibly
// emitting a PagedEdgeSet for the previous EdgeSet.  Each following call to
// AddGroup adds the group to this new EdgeSet until another call to
// StartEdgeSet is made.
func (b *EdgeSetBuilder) StartEdgeSet(ctx context.Context, src *srvpb.Node) error {
	if b.pager == nil {
		b.pager = b.constructPager()
	}
	return b.pager.StartSet(ctx, src)
}

// AddGroup adds a EdgeSet_Group to current EdgeSet being built, possibly
// emitting a new PagedEdgeSet and/or EdgePage.  StartEdgeSet must be called
// before any calls to this method.  See EdgeSetBuilder's documentation for the
// assumed order of the groups and this method's relation to StartEdgeSet.
func (b *EdgeSetBuilder) AddGroup(ctx context.Context, eg *srvpb.EdgeSet_Group) error {
	return b.pager.AddGroup(ctx, eg)
}

// Flush signals the end of the current PagedEdgeSet being built, flushing it,
// and its EdgeSet_Groups to the output function.  This should be called after
// the final call to AddGroup.  Manually calling Flush at any other time is
// unnecessary.
func (b *EdgeSetBuilder) Flush(ctx context.Context) error { return b.pager.Flush(ctx) }

func newPageKey(src string, n int) string { return fmt.Sprintf("%s.%.10d", src, n) }

var edgeOrdering = []string{
	schema.DefinesEdge,
	schema.DocumentsEdge,
	schema.RefEdge,
	schema.NamedEdge,
	schema.TypedEdge,
}

func edgeKindLess(kind1, kind2 string) bool {
	// General ordering:
	//   anchor edge kinds before non-anchor edge kinds
	//   forward edges before reverse edges
	//   edgeOrdering[i] (and variants) before edgeOrdering[i+1:]
	//   edge variants after root edge kind (ordered lexicographically)
	//   otherwise, order lexicographically

	if kind1 == kind2 {
		return false
	} else if a1, a2 := schema.IsAnchorEdge(kind1), schema.IsAnchorEdge(kind2); a1 != a2 {
		return a1
	} else if d1, d2 := schema.EdgeDirection(kind1), schema.EdgeDirection(kind2); d1 != d2 {
		return d1 == schema.Forward
	}
	kind1, kind2 = schema.Canonicalize(kind1), schema.Canonicalize(kind2)
	for _, kind := range edgeOrdering {
		if kind1 == kind {
			return true
		} else if kind2 == kind {
			return false
		} else if v1, v2 := schema.IsEdgeVariant(kind1, kind), schema.IsEdgeVariant(kind2, kind); v1 != v2 {
			return v1
		} else if v1 {
			return kind1 < kind2
		}
	}
	return kind1 < kind2
}

// byPageKind implements the sort.Interface
type byPageKind []*srvpb.PageIndex

// Implement the sort.Interface using edgeKindLess
func (s byPageKind) Len() int           { return len(s) }
func (s byPageKind) Less(i, j int) bool { return edgeKindLess(s[i].EdgeKind, s[j].EdgeKind) }
func (s byPageKind) Swap(i, j int)      { s[i], s[j] = s[j], s[i] }

// byEdgeKind implements the sort.Interface
type byEdgeKind []*srvpb.EdgeSet_Group

// Implement the sort.Interface using edgeKindLess
func (s byEdgeKind) Len() int           { return len(s) }
func (s byEdgeKind) Less(i, j int) bool { return edgeKindLess(s[i].Kind, s[j].Kind) }
func (s byEdgeKind) Swap(i, j int)      { s[i], s[j] = s[j], s[i] }

// ByName implements the sort.Interface for srvpb.Facts
type ByName []*srvpb.Fact

// Implement the sort.Interface
func (s ByName) Len() int           { return len(s) }
func (s ByName) Less(i, j int) bool { return s[i].Name < s[j].Name }
func (s ByName) Swap(i, j int)      { s[i], s[j] = s[j], s[i] }
