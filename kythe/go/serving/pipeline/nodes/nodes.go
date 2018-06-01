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

	"kythe.io/kythe/go/services/graphstore/compare"
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
				i++
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
	c := compare.Ints(int(a.GetKytheName()), int(b.GetKytheName()))
	if c != compare.EQ {
		return c
	}
	return compare.Strings(a.GetGenericName(), b.GetGenericName())
}

func compareEdges(a, b *ppb.Edge) compare.Order {
	if c := compare.Ints(int(a.GetKytheKind()), int(b.GetKytheKind())); c != compare.EQ {
		return c
	} else if c := compare.Strings(a.GetGenericKind(), b.GetGenericKind()); c != compare.EQ {
		return c
	} else if c := compare.Ints(int(a.Ordinal), int(b.Ordinal)); c != compare.EQ {
		return c
	}
	return compare.VNames(a.Target, b.Target)
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
