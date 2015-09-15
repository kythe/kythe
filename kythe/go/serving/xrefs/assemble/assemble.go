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
	"container/heap"
	"fmt"
	"log"
	"sort"
	"strconv"

	"kythe.io/kythe/go/services/graphstore"
	"kythe.io/kythe/go/services/graphstore/compare"
	"kythe.io/kythe/go/util/kytheuri"
	"kythe.io/kythe/go/util/schema"
	"kythe.io/kythe/go/util/stringset"

	"golang.org/x/net/context"

	srvpb "kythe.io/kythe/proto/serving_proto"
	spb "kythe.io/kythe/proto/storage_proto"
)

// NodeFact returns a Node_Fact from the given Entry.  If e == nil or e is not a
// node fact, nil is returned.
func NodeFact(e *spb.Entry) *srvpb.Node_Fact {
	if e == nil || graphstore.IsEdge(e) {
		return nil
	}
	return &srvpb.Node_Fact{
		Name:  e.FactName,
		Value: e.FactValue,
	}
}

// Source is a collection of facts and edges with a common source.
type Source struct {
	Ticket string

	Facts map[string][]byte
	Edges map[string][]string
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

// Sources returns a channel of Sources derived from a channel of entries in
// GraphStore order.
func Sources(entries <-chan *spb.Entry) <-chan *Source {
	ch := make(chan *Source, 1)

	go func() {
		defer close(ch)

		var es []*spb.Entry
		for entry := range entries {
			if len(es) > 0 && compare.VNames(es[0].Source, entry.Source) != compare.EQ {
				ch <- SourceFromEntries(es)
				es = nil
			}

			es = append(es, entry)
		}
		if len(es) > 0 {
			ch <- SourceFromEntries(es)
		}
	}()

	return ch
}

// DecorationFragments returns 0 or more FileDecorations fragments from the
// given Source, depending on its node kind.  If given an anchor, decoration
// fragments will be returned for each of the anchor's parents (assumed to be
// files).  If given a file, 1 decoration fragment will be returned with the
// file's source text and encoding populated.  All other nodes return 0
// decoration fragments.
func DecorationFragments(src *Source) []*srvpb.FileDecorations {
	switch string(src.Facts[schema.NodeKindFact]) {
	default:
		return nil
	case schema.FileKind:
		return []*srvpb.FileDecorations{{
			FileTicket: src.Ticket,
			SourceText: src.Facts[schema.TextFact],
			Encoding:   string(src.Facts[schema.TextEncodingFact]),
		}}
	case schema.AnchorKind:
		anchorStart, err := strconv.Atoi(string(src.Facts[schema.AnchorStartFact]))
		if err != nil {
			log.Printf("Error parsing anchor start offset %q: %v", string(src.Facts[schema.AnchorStartFact]), err)
			return nil
		}
		anchorEnd, err := strconv.Atoi(string(src.Facts[schema.AnchorEndFact]))
		if err != nil {
			log.Printf("Error parsing anchor end offset %q: %v", string(src.Facts[schema.AnchorEndFact]), err)
			return nil
		}

		anchor := &srvpb.FileDecorations_Decoration_Anchor{
			Ticket:      src.Ticket,
			StartOffset: int32(anchorStart),
			EndOffset:   int32(anchorEnd),
		}

		kinds := make([]string, len(src.Edges)-1)
		for kind := range src.Edges {
			if kind == schema.ChildOfEdge {
				continue
			}
			kinds = append(kinds, kind)
		}
		sort.Strings(kinds) // to ensure consistency

		var decor []*srvpb.FileDecorations_Decoration
		for _, kind := range kinds {
			for _, tgt := range src.Edges[kind] {
				decor = append(decor, &srvpb.FileDecorations_Decoration{
					Anchor:       anchor,
					Kind:         kind,
					TargetTicket: tgt,
				})
			}
		}
		if len(decor) == 0 {
			return nil
		}

		// Assume each anchor parent is a file
		parents := src.Edges[schema.ChildOfEdge]
		dm := make([]*srvpb.FileDecorations, len(parents))

		for i, parent := range parents {
			dm[i] = &srvpb.FileDecorations{
				FileTicket: parent,
				Decoration: decor,
			}
		}
		return dm
	}
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
			return s[i].TargetTicket < s[j].TargetTicket
		}
		return s[i].Kind < s[j].Kind
	}
	return s[i].Anchor.EndOffset < s[j].Anchor.EndOffset
}

// EdgeSetBuilder constructs a set of PagedEdgeSets and EdgePages from a
// sequence of EdgeSet_Groups.  All EdgeSet_Groups for the same source are
// assumed to be given sequentially to AddGroup, secondarily ordered by the
// group's edge kind.  If given in this order, Output will be given exactly 1
// PagedEdgeSet per source with as few EdgeSet_Group per edge kind as to satisfy
// MaxEdgePageSize (MaxEdgePageSize == 0 indicates that there will be exactly 1
// edge group per edge kind).  If not given in this order, no guarantees can be
// made.  Flush must be called after the final call to AddGroup.
type EdgeSetBuilder struct {
	// MaxEdgePageSize is maximum number of edges that are allowed in the
	// PagedEdgeSet and any EdgePage.  If MaxEdgePageSize <= 0, no paging is
	// attempted.
	MaxEdgePageSize int

	// Output is used to emit each PagedEdgeSet constructed.
	Output func(context.Context, *srvpb.PagedEdgeSet) error
	// OutputPage is used to emit each EdgePage constructed.
	OutputPage func(context.Context, *srvpb.EdgePage) error

	curPES   *srvpb.PagedEdgeSet
	curEG    *srvpb.EdgeSet_Group
	groups   byEdgeCount
	resident int
}

// AddGroup adds the given EdgeSet_Group to the builder, possibly emitting a new
// PagedEdgeSet and/or EdgePage.  See EdgeSetBuilder's documentation for the
// assumed order of the groups.
func (b *EdgeSetBuilder) AddGroup(ctx context.Context, src string, eg *srvpb.EdgeSet_Group) error {
	if b.curPES != nil && b.curPES.EdgeSet.SourceTicket != src {
		if err := b.Flush(ctx); err != nil {
			return fmt.Errorf("error flushing previous PagedEdgeSet: %v", err)
		}
	}

	// Setup b.curPES and b.curEG; ensuring both are non-nil
	if b.curPES == nil {
		b.curPES = &srvpb.PagedEdgeSet{
			EdgeSet: &srvpb.EdgeSet{
				SourceTicket: src,
			},
		}
		b.curEG = eg
	} else if b.curEG == nil {
		b.curEG = eg
	} else if b.curEG.Kind != eg.Kind {
		heap.Push(&b.groups, b.curEG)
		b.curEG = eg
	} else {
		b.curEG.TargetTicket = append(b.curEG.TargetTicket, eg.TargetTicket...)
	}
	// Update edge counters
	b.resident += len(eg.TargetTicket)
	b.curPES.TotalEdges += int32(len(eg.TargetTicket))

	// Handling creation of EdgePages, when # of resident edges passes config value
	for b.MaxEdgePageSize > 0 && b.resident > b.MaxEdgePageSize {
		var eviction *srvpb.EdgeSet_Group
		if b.curEG != nil {
			if len(b.curEG.TargetTicket) > b.MaxEdgePageSize {
				// Split the large page; evict page exactly sized b.MaxEdgePageSize
				eviction = &srvpb.EdgeSet_Group{
					Kind:         b.curEG.Kind,
					TargetTicket: b.curEG.TargetTicket[:b.MaxEdgePageSize],
				}
				b.curEG.TargetTicket = b.curEG.TargetTicket[b.MaxEdgePageSize:]
			} else if len(b.groups) == 0 || len(b.curEG.TargetTicket) > len(b.groups[0].TargetTicket) {
				// Evict b.curEG, it's larger than any other group we have
				eviction, b.curEG = b.curEG, nil
			}
		}
		if eviction == nil {
			// Evict the largest group we have
			eviction = heap.Pop(&b.groups).(*srvpb.EdgeSet_Group)
		}

		key := newPageKey(src, len(b.curPES.PageIndex))
		count := len(eviction.TargetTicket)

		// Output the EdgePage and add it to the page indices
		if err := b.OutputPage(ctx, &srvpb.EdgePage{
			PageKey:      key,
			SourceTicket: src,
			EdgesGroup:   eviction,
		}); err != nil {
			return fmt.Errorf("error emitting EdgePage: %v", err)
		}
		b.curPES.PageIndex = append(b.curPES.PageIndex, &srvpb.PageIndex{
			PageKey:   key,
			EdgeKind:  eviction.Kind,
			EdgeCount: int32(count),
		})
		b.resident -= count // update edge counter
	}

	return nil
}

// Flush signals the end of the current PagedEdgeSet being built, flushing it,
// and its EdgeSet_Groups to the output function.  This should be called after
// the final call to AddGroup.  Manually calling Flush at any other time is
// unnecessary.
func (b *EdgeSetBuilder) Flush(ctx context.Context) error {
	if b.curPES == nil {
		return nil
	}
	if b.curEG != nil {
		b.groups = append(b.groups, b.curEG)
	}
	b.curPES.EdgeSet.Group = b.groups
	sort.Sort(byEdgeKind(b.curPES.EdgeSet.Group))
	sort.Sort(byPageKind(b.curPES.PageIndex))
	err := b.Output(ctx, b.curPES)
	b.curPES, b.curEG, b.groups, b.resident = nil, nil, nil, 0
	return err
}

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

// byEdgeCount implements the heap.Interface (largest group of edges first)
type byEdgeCount []*srvpb.EdgeSet_Group

// Implement the sort.Interface
func (s byEdgeCount) Len() int           { return len(s) }
func (s byEdgeCount) Less(i, j int) bool { return len(s[i].TargetTicket) > len(s[j].TargetTicket) }
func (s byEdgeCount) Swap(i, j int)      { s[i], s[j] = s[j], s[i] }

// Implement the heap.Interface
func (s *byEdgeCount) Push(v interface{}) { *s = append(*s, v.(*srvpb.EdgeSet_Group)) }
func (s *byEdgeCount) Pop() interface{} {
	old := *s
	n := len(old) - 1
	out := old[n]
	*s = old[:n]
	return out
}
