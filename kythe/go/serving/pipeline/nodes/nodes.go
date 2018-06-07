/*
 * Copyright 2018 Google Inc. All rights reserved.
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

// Package nodes provides Beam transformations over *ppb.Nodes.
package nodes

import (
	"bytes"
	"context"
	"fmt"
	"reflect"
	"sort"

	"kythe.io/kythe/go/util/compare"
	"kythe.io/kythe/go/util/schema"
	"kythe.io/kythe/go/util/schema/edges"
	"kythe.io/kythe/go/util/schema/facts"

	"github.com/apache/beam/sdks/go/pkg/beam"

	ppb "kythe.io/kythe/proto/pipeline_go_proto"
	scpb "kythe.io/kythe/proto/schema_go_proto"
	spb "kythe.io/kythe/proto/storage_go_proto"
)

func init() {
	beam.RegisterFunction(embedSourceKey)
	beam.RegisterFunction(entryToNode)

	beam.RegisterType(reflect.TypeOf((*Filter)(nil)).Elem())
	beam.RegisterType(reflect.TypeOf((*combineNodes)(nil)).Elem())
}

// FromEntries transforms a PCollection of *ppb.Entry protos into *ppb.Nodes.
func FromEntries(s beam.Scope, entries beam.PCollection) beam.PCollection {
	return beam.ParDo(s, embedSourceKey,
		beam.CombinePerKey(s, &combineNodes{},
			beam.ParDo(s, entryToNode, entries)))
}

func entryToNode(e *spb.Entry, emit func(*spb.VName, *ppb.Node)) error {
	if e.Source == nil {
		return fmt.Errorf("invalid Entry: source is missing: %+v", e)
	}

	n := &ppb.Node{}
	if e.EdgeKind == "" {
		if e.FactName == "" || e.Target != nil {
			return fmt.Errorf("invalid fact Entry: {%v}", e)
		}

		switch e.FactName {
		case facts.NodeKind:
			kind := string(e.FactValue)
			if k := schema.NodeKind(kind); k != scpb.NodeKind_UNKNOWN_NODE_KIND {
				n.Kind = &ppb.Node_KytheKind{k}
			} else {
				n.Kind = &ppb.Node_GenericKind{kind}
			}
		case facts.Subkind:
			subkind := string(e.FactValue)
			if k := schema.Subkind(subkind); k != scpb.Subkind_UNKNOWN_SUBKIND {
				n.Subkind = &ppb.Node_KytheSubkind{k}
			} else {
				n.Subkind = &ppb.Node_GenericSubkind{subkind}
			}
		default:
			n.Fact = append(n.Fact, entryToFact(e))
		}
	} else {
		if (e.FactName != "/" && e.FactName != "") || len(e.FactValue) != 0 || e.Target == nil {
			return fmt.Errorf("invalid edge Entry: {%v}", e)
		}

		n.Edge = append(n.Edge, entryToEdge(e))
	}

	emit(e.Source, n)
	return nil
}

func entryToEdge(e *spb.Entry) *ppb.Edge {
	kind, ord, _ := edges.ParseOrdinal(e.EdgeKind)
	g := &ppb.Edge{Target: e.Target, Ordinal: int32(ord)}
	edgeKind := schema.EdgeKind(kind)
	if edgeKind == scpb.EdgeKind_UNKNOWN_EDGE_KIND {
		g.Kind = &ppb.Edge_GenericKind{kind}
	} else {
		g.Kind = &ppb.Edge_KytheKind{edgeKind}
	}
	return g
}

func entryToFact(e *spb.Entry) *ppb.Fact {
	f := &ppb.Fact{Value: e.FactValue}
	name := schema.FactName(e.FactName)
	if name == scpb.FactName_UNKNOWN_FACT_NAME {
		f.Name = &ppb.Fact_GenericName{e.FactName}
	} else {
		f.Name = &ppb.Fact_KytheName{name}
	}
	return f
}

var conflictingFactsCounter = beam.NewCounter("kythe.nodes", "conflicting-facts")

// combineNodes is a Beam combiner for *ppb.Nodes.  All facts and edges are
// merged into a single *ppb.Node.  If a fact has multiple values, an arbitrary
// value is chosen (this includes special-case facts like node kinds).
// Duplicate edges are removed.
type combineNodes struct{}

func (combineNodes) CreateAccumulator() *ppb.Node { return &ppb.Node{} }

func (c *combineNodes) MergeAccumulators(ctx context.Context, accum, n *ppb.Node) *ppb.Node {
	if n.Kind != nil {
		if accum.Kind != nil &&
			(accum.GetKytheKind() != n.GetKytheKind() || accum.GetGenericKind() != n.GetGenericKind()) {
			conflictingFactsCounter.Inc(ctx, 1)
		}
		accum.Kind = n.Kind
	}
	if n.Subkind != nil {
		if accum.Subkind != nil &&
			(accum.GetKytheSubkind() != n.GetKytheSubkind() || accum.GetGenericSubkind() != n.GetGenericSubkind()) {
			conflictingFactsCounter.Inc(ctx, 1)
		}
		accum.Subkind = n.Subkind
	}
	accum.Fact = append(accum.Fact, n.Fact...)
	accum.Edge = append(accum.Edge, n.Edge...)
	return accum
}

func (c *combineNodes) AddInput(ctx context.Context, accum, n *ppb.Node) *ppb.Node {
	return c.MergeAccumulators(ctx, accum, n)
}

func (c *combineNodes) ExtractOutput(ctx context.Context, n *ppb.Node) *ppb.Node {
	// TODO(schroederc): deduplicate earlier during combine
	if len(n.Fact) > 1 {
		sort.Slice(n.Fact, func(a, b int) bool { return compareFacts(n.Fact[a], n.Fact[b]) == compare.LT })
		j := 1
		for i := 1; i < len(n.Fact); i++ {
			if compareFacts(n.Fact[j-1], n.Fact[i]) != compare.EQ {
				n.Fact[j] = n.Fact[i]
				j++
			} else if !bytes.Equal(n.Fact[j-1].Value, n.Fact[i].Value) {
				conflictingFactsCounter.Inc(ctx, 1)
			}
		}
		n.Fact = n.Fact[:j]
	}
	if len(n.Edge) > 1 {
		sort.Slice(n.Edge, func(a, b int) bool { return compareEdges(n.Edge[a], n.Edge[b]) == compare.LT })
		j := 1
		for i := 1; i < len(n.Edge); i++ {
			if compareEdges(n.Edge[j-1], n.Edge[i]) != compare.EQ {
				n.Edge[j] = n.Edge[i]
				j++
				i++
			}
		}
		n.Edge = n.Edge[:j]
	}
	return n
}

func compareFacts(a, b *ppb.Fact) compare.Order {
	return compare.Ints(int(a.GetKytheName()), int(b.GetKytheName())).
		AndThen(a.GetGenericName(), b.GetGenericName())
}

func compareEdges(a, b *ppb.Edge) compare.Order {
	return compare.Ints(int(a.GetKytheKind()), int(b.GetKytheKind())).
		AndThen(a.GetGenericKind(), b.GetGenericKind()).
		AndThen(int(a.Ordinal), int(b.Ordinal)).
		AndThen(a.Target, b.Target,
			compare.With(func(a, b interface{}) compare.Order {
				return compare.VNames(a.(*spb.VName), b.(*spb.VName))
			}))
}

func embedSourceKey(src *spb.VName, n *ppb.Node) *ppb.Node {
	return &ppb.Node{
		Source:  src,
		Kind:    n.Kind,
		Subkind: n.Subkind,
		Fact:    n.Fact,
		Edge:    n.Edge,
	}
}

// Filter is a beam DoFn that emits *ppb.Nodes matching a set of kinds/subkinds.
// Optionally, each processed node's facts/edges will also be filtered to the
// desired set.
//
// The semantics of the Filter are such that a "zero"-value Filter will pass all
// Nodes through unaltered.  Each part of the filter only applies if set to a
// non-nil value and all parts are applied independently.
//
// Examples:
//
//   Emit only "record" nodes with the "class" subkind with all their facts/edges:
//     &Filter {
//       FilterByKind:    []string{"record"},
//       FilterBySubkind: []string{"class"},
//     }
//
//   Emit only "anchor" nodes (any subkind) with all their facts/edges:
//     &Filter {FilterByKind: []string{"anchor"}}
//
//   Emit only "anchor" nodes with only the loc/{start,end} facts and no edges:
//     &Filter {
//       FilterByKind: []string{"anchor"},
//       IncludeFacts: []string{"/kythe/loc/start", "/kythe/loc/end"},
//       IncludeEdges: []string{},
//     }
//
//   Emit only "anchor" nodes with their "childof" edges (but all their facts):
//     &Filter {
//       FilterByKind: []string{"anchor"},
//       IncludeEdges: []string{"/kythe/edge/childof"},
//     }
//
//   Emit all nodes without any of their edges (but all their facts):
//     &Filter {IncludeEdges: []string{}}
type Filter struct {
	// FilterByKind, if non-nil, configures the filter to only pass through nodes
	// that match one of the given kinds.
	FilterByKind []string
	// FilterBySubkind, if non-nil, configures the filter to only pass through
	// nodes that match one of the given subkinds.
	FilterBySubkind []string

	// IncludeFacts, if non-nil, configures the filter to remove all facts not
	// explicitly contained with the slice.
	IncludeFacts []string
	// IncludeEdges, if non-nil, configures the filter to remove all edges with a
	// kind not explicitly contained with the slice.
	IncludeEdges []string
}

// ProcessElement emits the given Node if it matches the given Filter.
func (f *Filter) ProcessElement(n *ppb.Node, emit func(*ppb.Node)) error {
	if f.FilterByKind != nil && !contains(Kind(n), f.FilterByKind) {
		return nil
	} else if f.FilterBySubkind != nil && !contains(Subkind(n), f.FilterBySubkind) {
		return nil
	}

	// Shortcut case for when no fact/edge filters are given.
	if f.IncludeFacts == nil && f.IncludeEdges == nil {
		emit(n)
		return nil
	}

	facts := n.Fact
	if f.IncludeFacts != nil {
		if len(f.IncludeFacts) == 0 {
			facts = nil
		} else {
			facts = make([]*ppb.Fact, 0, len(n.Fact))
			for _, fact := range n.Fact {
				if contains(FactName(fact), f.IncludeFacts) {
					facts = append(facts, fact)
				}
			}
		}
	}

	edges := n.Edge
	if f.IncludeEdges != nil {
		if len(f.IncludeEdges) == 0 {
			edges = nil
		} else {
			edges = make([]*ppb.Edge, 0, len(n.Edge))
			for _, edge := range n.Edge {
				if contains(edgeKind(edge), f.IncludeEdges) {
					edges = append(edges, edge)
				}
			}
		}
	}

	emit(&ppb.Node{
		Source:  n.Source,
		Kind:    n.Kind,
		Subkind: n.Subkind,
		Fact:    facts,
		Edge:    edges,
	})
	return nil
}

// Kind returns the string representation of the node's kind.
func Kind(n *ppb.Node) string {
	if k := n.GetGenericKind(); k != "" {
		return k
	}
	return schema.NodeKindString(n.GetKytheKind())
}

// Subkind returns the string representation of the node's subkind.
func Subkind(n *ppb.Node) string {
	if k := n.GetGenericSubkind(); k != "" {
		return k
	}
	return schema.SubkindString(n.GetKytheSubkind())
}

// FactName returns the string representation of the fact's name.
func FactName(f *ppb.Fact) string {
	if k := f.GetGenericName(); k != "" {
		return k
	}
	return schema.FactNameString(f.GetKytheName())
}

func edgeKind(e *ppb.Edge) string {
	if k := e.GetGenericKind(); k != "" {
		return k
	}
	return schema.EdgeKindString(e.GetKytheKind())
}

func contains(s string, lst []string) bool {
	for _, ss := range lst {
		if s == ss {
			return true
		}
	}
	return false
}
