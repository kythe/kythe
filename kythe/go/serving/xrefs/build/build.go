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

// Package build provides functions to build the various components (nodes,
// edges, and decorations) of an xrefs serving table.
package build

import (
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

// EdgeSetBuilder constructs a set of PagedEdgeSets from a sequence of
// EdgeSet_Groups.  All EdgeSet_Groups for the same source are assumed to be
// given sequentially to AddGroup, secondarily ordered by the group's edge kind.
// If given in this order, Output will be given exactly 1 PagedEdgeSet per
// source with exactly 1 EdgeSet_Group per edge kind.  If not given in this
// order, no guarantees can be made.  Flush must be called after the final call
// to AddGroup.
type EdgeSetBuilder struct {
	Output func(context.Context, *srvpb.PagedEdgeSet) error

	curPES *srvpb.PagedEdgeSet
	curEG  *srvpb.EdgeSet_Group
}

// AddGroup adds the given EdgeSet_Group to the builder, possibly emitting a new
// PagedEdgeSet.  See EdgeSetBuilder's documentation for the assumed order of
// the groups.
func (b *EdgeSetBuilder) AddGroup(ctx context.Context, src string, eg *srvpb.EdgeSet_Group) error {
	if b.curPES != nil && b.curPES.EdgeSet.SourceTicket != src {
		if err := b.Flush(ctx); err != nil {
			return fmt.Errorf("error flushing previous PagedEdgeSet: %v", err)
		}
	}
	if b.curPES == nil {
		b.curPES = &srvpb.PagedEdgeSet{
			EdgeSet: &srvpb.EdgeSet{
				SourceTicket: src,
			},
		}
		b.curEG = eg
		return nil
	}

	if b.curEG.Kind != eg.Kind {
		b.flushGroup()
		b.curEG = eg
	} else {
		b.curEG.TargetTicket = append(b.curEG.TargetTicket, eg.TargetTicket...)
	}

	return nil
}

func (b *EdgeSetBuilder) flushGroup() {
	b.curPES.EdgeSet.Group = append(b.curPES.EdgeSet.Group, b.curEG)
	b.curPES.TotalEdges += int32(len(b.curEG.TargetTicket))
}

// Flush signals the end of the current PagedEdgeSet being built and flushes it
// and its EdgeSet_Groups to the output function.  This should be called after
// the final call to AddGroup, but manually calling Flush at any other time is
// unnecessary.
func (b *EdgeSetBuilder) Flush(ctx context.Context) error {
	if b.curPES == nil {
		return nil
	}
	b.flushGroup()
	err := b.Output(ctx, b.curPES)
	b.curPES, b.curEG = nil, nil
	return err
}
