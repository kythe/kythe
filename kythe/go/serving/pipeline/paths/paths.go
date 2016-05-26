/*
 * Copyright 2016 Google Inc. All rights reserved.
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

// Package paths specializes the util/reduce interfaces for Kythe *ipb.Paths.
package paths

import (
	"fmt"
	"log"
	"sort"
	"strconv"

	"kythe.io/kythe/go/services/xrefs"
	"kythe.io/kythe/go/serving/xrefs/assemble"
	"kythe.io/kythe/go/util/reduce"
	"kythe.io/kythe/go/util/schema"

	"github.com/golang/protobuf/proto"
	"golang.org/x/net/context"

	cpb "kythe.io/kythe/proto/common_proto"
	ipb "kythe.io/kythe/proto/internal_proto"
	srvpb "kythe.io/kythe/proto/serving_proto"
)

// ReducerIO is a Path-specialized interface mirroring reduce.IO.
type ReducerIO interface {
	Next() (sortKey string, p *ipb.Path, err error)
	Emit(ctx context.Context, p *ipb.Path) error
}

// Reducer implements the reduce.Reduce interface for *ipb.Paths.  Func will be
// called once per Reduce call.  If non-nil, OutputSort will be called on each
// emitted *ipb.Path and used as its sort-key in the constructed
// *ipb.SortedKeyValue.
//
// The Reducer's underlying input/output values are *ipb.SortedKeyValues with
// keys matching each Path's pivot ticket and user-defined sort keys.  Each
// Reducer is meant to be used with reduce.Sort.
//
// Paths are not written/read as given.  To limit the amount of data
// written/read, all specializations (and orignal facts) will be removed from
// each Pivot node.  A Reducer can pass-through these fields with a Path only
// populated with its Pivot node (not edges).  These pivot-only paths will be
// yielded to Func as its second parameter (and not through its ReducerIO).
// Prefix pivot-only paths will also be used to replace later Path pivots.
// Duplicate pivot-only paths will be discarded; the most specialized version
// will be chosen (or if that is ambiguous, the first).
//
// Likewise, each file target in an edge will have its specialization (and
// original facts) removed and ExpandedAnchor specializations will have their
// kind filled in based on their encompassing Path Edge kind.
type Reducer struct {
	Func       func(context.Context, *ipb.Path_Node, ReducerIO) error
	OutputSort func(*ipb.Path) string
}

// Start implements part of the reduce.Reduce interface.
func (r *Reducer) Start(_ context.Context) error { return nil }

// End implements part of the reduce.Reduce interface.
func (r *Reducer) End(_ context.Context) error { return nil }

// Reduce implements part of the reduce.Reduce interface.
func (r *Reducer) Reduce(ctx context.Context, rio reduce.IO) error {
	pr := &pathReducerIO{rio: rio, sort: r.OutputSort}
	pivot, err := pr.readPivot()
	if err != nil {
		return err
	}
	return r.Func(ctx, pivot, pr)
}

// FromSource creates a set of *ipb.Paths for each of its edges as well as a
// single *ipb.Path with only its pivot set to be the source node.
func FromSource(src *ipb.Source) []*ipb.Path {
	var paths []*ipb.Path

	n := specialize(assemble.Node(src))
	paths = append(paths, &ipb.Path{Pivot: n})
	for kind, group := range src.EdgeGroups {
		if schema.EdgeDirection(kind) != schema.Forward {
			continue
		}
		for _, tgt := range group.Edges {
			paths = append(paths, &ipb.Path{
				Pivot: &ipb.Path_Node{
					Ticket: tgt.Ticket,
				},
				Edges: []*ipb.Path_Edge{{
					Kind:    schema.MirrorEdge(kind),
					Ordinal: int32(tgt.Ordinal),
					Target:  n,
				}},
			})
		}
	}
	return paths
}

// OriginalNode returns the given node's original serving node, synthesizing one
// if necessary.
func OriginalNode(n *ipb.Path_Node) (g *srvpb.Node) {
	defer func() {
		// Ensure serving data is consistent
		sort.Sort(xrefs.ByName(g.Fact))
	}()

	if n.Original != nil {
		return n.Original
	}

	// Synthesize node without original
	log.Printf("WARNING: synthesizing node for %q", n.Ticket)
	sn := &srvpb.Node{
		Ticket: n.Ticket,
		Fact: []*cpb.Fact{
			{Name: schema.NodeKindFact, Value: []byte(n.NodeKind)},
		},
	}

	if a := n.GetRawAnchor(); a != nil {
		sn.Fact = append(sn.Fact,
			&cpb.Fact{Name: schema.AnchorStartFact, Value: []byte(strconv.FormatInt(int64(a.StartOffset), 10))},
			&cpb.Fact{Name: schema.AnchorEndFact, Value: []byte(strconv.FormatInt(int64(a.EndOffset), 10))},
		)
		if a.SnippetStart != 0 || a.SnippetEnd != 0 {
			sn.Fact = append(sn.Fact,
				&cpb.Fact{Name: schema.SnippetStartFact, Value: []byte(strconv.FormatInt(int64(a.SnippetStart), 10))},
				&cpb.Fact{Name: schema.SnippetEndFact, Value: []byte(strconv.FormatInt(int64(a.SnippetEnd), 10))},
			)
		}
	} else if a := n.GetExpandedAnchor(); a != nil {
		sn.Fact = append(sn.Fact,
			&cpb.Fact{Name: schema.AnchorStartFact, Value: []byte(strconv.FormatInt(int64(a.Span.Start.ByteOffset), 10))},
			&cpb.Fact{Name: schema.AnchorEndFact, Value: []byte(strconv.FormatInt(int64(a.Span.End.ByteOffset), 10))},
		)
		if a.SnippetSpan.Start.ByteOffset != 0 || a.SnippetSpan.End.ByteOffset != 0 {
			sn.Fact = append(sn.Fact,
				&cpb.Fact{Name: schema.SnippetStartFact, Value: []byte(strconv.FormatInt(int64(a.SnippetSpan.Start.ByteOffset), 10))},
				&cpb.Fact{Name: schema.SnippetEndFact, Value: []byte(strconv.FormatInt(int64(a.SnippetSpan.End.ByteOffset), 10))},
			)
		}
	} else if f := n.GetFile(); f != nil {
		if len(f.Text) > 0 {
			sn.Fact = append(sn.Fact, &cpb.Fact{Name: schema.TextFact, Value: f.Text})
		}
		if f.Encoding != "" {
			sn.Fact = append(sn.Fact, &cpb.Fact{Name: schema.TextEncodingFact, Value: []byte(f.Encoding)})
		}
	}

	return sn
}

// RawAnchor returns and coerces n if it is an anchor node.  nil will be returned
// otherwise.
func RawAnchor(n *ipb.Path_Node) *srvpb.RawAnchor {
	if a := n.GetRawAnchor(); a != nil {
		return a
	} else if a := n.GetExpandedAnchor(); a != nil {
		return &srvpb.RawAnchor{
			Ticket:       a.Ticket,
			StartOffset:  a.Span.Start.ByteOffset,
			EndOffset:    a.Span.End.ByteOffset,
			SnippetStart: a.SnippetSpan.Start.ByteOffset,
			SnippetEnd:   a.SnippetSpan.End.ByteOffset,
		}
	}
	return nil
}

// ToString returns a human-readable string representation of p.
func ToString(p *ipb.Path) string {
	s := "**" + p.Pivot.NodeKind + "**"
	for _, e := range p.Edges {
		if schema.EdgeDirection(e.Kind) == schema.Forward {
			s += fmt.Sprintf(" -[%s]> %s", e.Kind, e.Target.NodeKind)
		} else {
			s += fmt.Sprintf(" <[%s]- %s", schema.MirrorEdge(e.Kind), e.Target.NodeKind)
		}
	}
	return s
}

// ReverseSinglePath returns the reverse p, assuming it is a single-edge Path.
func ReverseSinglePath(p *ipb.Path) *ipb.Path {
	return &ipb.Path{
		Pivot: p.Edges[0].Target,
		Edges: []*ipb.Path_Edge{{
			Kind:    schema.MirrorEdge(p.Edges[0].Kind),
			Ordinal: p.Edges[0].Ordinal,
			Target:  p.Pivot,
		}},
	}
}

func specialize(n *srvpb.Node) *ipb.Path_Node {
	if n == nil {
		return nil
	}
	var nodeKind string
	for _, f := range n.Fact {
		if f.Name == schema.NodeKindFact {
			nodeKind = string(f.Value)
		}
	}
	switch nodeKind {
	case schema.FileKind:
		var text []byte
		var encoding string
		for _, f := range n.Fact {
			switch f.Name {
			case schema.TextFact:
				text = f.Value
			case schema.TextEncodingFact:
				encoding = string(f.Value)
			}
		}
		return &ipb.Path_Node{
			Ticket:   n.Ticket,
			NodeKind: schema.FileKind,
			Specialization: &ipb.Path_Node_File{&srvpb.File{
				Ticket:   n.Ticket,
				Text:     text,
				Encoding: encoding,
			}},
			Original: n,
		}
	case schema.AnchorKind:
		var locStart, locEnd, snippetStart, snippetEnd int
		for _, f := range n.Fact {
			switch f.Name {
			case schema.AnchorStartFact:
				locStart, _ = strconv.Atoi(string(f.Value))
			case schema.AnchorEndFact:
				locEnd, _ = strconv.Atoi(string(f.Value))
			case schema.SnippetStartFact:
				snippetStart, _ = strconv.Atoi(string(f.Value))
			case schema.SnippetEndFact:
				snippetEnd, _ = strconv.Atoi(string(f.Value))
			}
		}
		return &ipb.Path_Node{
			Ticket:   n.Ticket,
			NodeKind: schema.AnchorKind,
			Specialization: &ipb.Path_Node_RawAnchor{&srvpb.RawAnchor{
				Ticket:       n.Ticket,
				StartOffset:  int32(locStart),
				EndOffset:    int32(locEnd),
				SnippetStart: int32(snippetStart),
				SnippetEnd:   int32(snippetEnd),
			}},
			Original: n,
		}
	default:
		return &ipb.Path_Node{
			Ticket:   n.Ticket,
			NodeKind: nodeKind,
			Original: n,
		}
	}
}

type pathReducerIO struct {
	rio  reduce.IO
	sort func(*ipb.Path) string

	key   string
	pivot *ipb.Path_Node

	first     *ipb.Path
	firstSort string
}

func (pr *pathReducerIO) readPivot() (*ipb.Path_Node, error) {
	p, key, sortKey, err := pr.nextPath()
	if err != nil {
		return nil, err
	}
	pr.key = key
	if len(p.Edges) != 0 {
		log.Printf("WARNING: missing facts for node: %q", pr.key)
		pr.first, pr.firstSort = p, sortKey
		return &ipb.Path_Node{
			Ticket: pr.key,
		}, nil
	}
	pr.pivot = p.Pivot
	return pr.pivot, nil
}

// Next implements part of the ReducerIO interface.
func (pr *pathReducerIO) Next() (string, *ipb.Path, error) {
	if pr.first != nil {
		sortKey, p := pr.firstSort, pr.first
		pr.first, pr.firstSort = nil, ""
		return sortKey, p, nil
	}

	for {
		p, _, sortKey, err := pr.nextPath()
		if err != nil {
			return "", nil, err
		}
		if len(p.Edges) == 0 {
			continue
		}
		if pr.pivot != nil {
			p.Pivot = pr.pivot
		}
		return sortKey, p, nil
	}
}

func (pr *pathReducerIO) nextPath() (*ipb.Path, string, string, error) {
	i, err := pr.rio.Next()
	if err != nil {
		return nil, "", "", err
	}
	kv := i.(*ipb.SortedKeyValue)
	var p ipb.Path
	if err := proto.Unmarshal(kv.Value, &p); err != nil {
		return nil, "", "", err
	}
	return &p, kv.Key, kv.SortKey[1:], nil
}

// Emit implements part of the ReducerIO interface.
func (pr *pathReducerIO) Emit(ctx context.Context, p *ipb.Path) error {
	var sortKey string
	if pr.sort != nil {
		sortKey = pr.sort(p)
	}
	kv, err := KeyValue(sortKey, p)
	if err != nil {
		return err
	}
	return pr.rio.Emit(ctx, kv)
}

// KeyValue returns an equivalent SortedKeyValue of the given Path.  Steps will
// be taken to mimimize the data stored in p.  Each key value is meant to be
// read as a sorted ReducerIO stream.
func KeyValue(sortKey string, p *ipb.Path) (*ipb.SortedKeyValue, error) {
	if p.Pivot == nil {
		return nil, fmt.Errorf("nil Pivot in path: %v", p)
	}

	var sortKeyPrefix string

	if len(p.Edges) == 0 {
		// Sort pivot-only paths first; sort more specific kinds first
		switch {
		case p.Pivot.GetExpandedAnchor() != nil:
			sortKeyPrefix = "aa"
		case p.Pivot.GetRawAnchor() != nil:
			sortKeyPrefix = "ab"
		case p.Pivot.GetFile() != nil:
			sortKeyPrefix = "ac"
		case p.Pivot.NodeKind == "":
			sortKeyPrefix = "az"
		default:
			sortKeyPrefix = "ay"
		}
	} else {
		// Sort actual paths after pivot-only paths
		sortKeyPrefix = "b"

		// Make a copy of the Path to clean up
		p = &ipb.Path{
			Pivot: &ipb.Path_Node{
				Ticket:   p.Pivot.Ticket,
				NodeKind: p.Pivot.NodeKind,
				// do not store specialization/original facts
			},
			Edges: p.Edges,
		}

		// Compress nodes in path
		p.Edges = fixEdges(p.Edges)
	}
	rec, err := proto.Marshal(p)
	if err != nil {
		return nil, err
	}
	return &ipb.SortedKeyValue{
		Key:     p.Pivot.Ticket,
		SortKey: sortKeyPrefix + sortKey,
		Value:   rec,
	}, nil
}

func fixEdges(es []*ipb.Path_Edge) []*ipb.Path_Edge {
	res := make([]*ipb.Path_Edge, len(es))
	for i, e := range es {
		if xa := e.Target.GetExpandedAnchor(); xa != nil && xa.Kind != e.Kind {
			// As a special case, fill in an ExpandedAnchor's kind
			e.Target = &ipb.Path_Node{
				Ticket:   e.Target.Ticket,
				NodeKind: e.Target.NodeKind,
				Specialization: &ipb.Path_Node_ExpandedAnchor{&srvpb.ExpandedAnchor{
					Ticket:      xa.Ticket,
					Kind:        e.Kind,
					Parent:      xa.Parent,
					Text:        xa.Text,
					Span:        xa.Span,
					Snippet:     xa.Snippet,
					SnippetSpan: xa.SnippetSpan,
				}},
				Original: e.Target.Original,
			}
		} else if e.Target.GetFile() != nil {
			// Remove unneeded file specialization (and original) from target nodes
			e.Target = &ipb.Path_Node{
				Ticket:   e.Target.Ticket,
				NodeKind: e.Target.NodeKind,
			}
		}

		res[i] = e
	}
	return res
}
