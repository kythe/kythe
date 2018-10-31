/*
 * Copyright 2018 The Kythe Authors. All rights reserved.
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

package graph

import (
	"context"
	"testing"

	"kythe.io/kythe/go/services/graph"
	"kythe.io/kythe/go/serving/graph/columnar"
	"kythe.io/kythe/go/storage/inmemory"
	"kythe.io/kythe/go/storage/keyvalue"
	"kythe.io/kythe/go/util/compare"
	"kythe.io/kythe/go/util/kytheuri"
	"kythe.io/kythe/go/util/schema/edges"
	"kythe.io/kythe/go/util/schema/facts"
	kinds "kythe.io/kythe/go/util/schema/nodes"

	cpb "kythe.io/kythe/proto/common_go_proto"
	gpb "kythe.io/kythe/proto/graph_go_proto"
	gspb "kythe.io/kythe/proto/graph_serving_go_proto"
	scpb "kythe.io/kythe/proto/schema_go_proto"
	spb "kythe.io/kythe/proto/storage_go_proto"
)

func mustWriteEdges(t *testing.T, w keyvalue.Writer, e *gspb.Edges) {
	kv, err := columnar.EncodeEdgesEntry(columnar.EdgesKeyPrefix, e)
	if err != nil {
		t.Fatal(err)
	}
	mustWrite(t, w, kv.Key, kv.Value)
}

func mustWrite(t *testing.T, w keyvalue.Writer, key, val []byte) {
	if err := w.Write(key, val); err != nil {
		t.Fatal(err)
	}
}

func TestServingEdges(t *testing.T) {
	ctx := context.Background()
	db := inmemory.NewKeyValueDB()
	w, err := db.Writer(ctx)
	if err != nil {
		t.Fatal(err)
	}

	// Mark table as columnar
	mustWrite(t, w, []byte(ColumnarTableKeyMarker), []byte{})

	src := &spb.VName{Corpus: "corpus", Signature: "sig"}
	edgeEntries := []*gspb.Edges{{
		Source: src,
		Entry: &gspb.Edges_Index_{&gspb.Edges_Index{
			Node: &scpb.Node{
				Kind: &scpb.Node_KytheKind{scpb.NodeKind_RECORD},
				Fact: []*scpb.Fact{{
					Name:  &scpb.Fact_KytheName{scpb.FactName_TEXT},
					Value: []byte("value"),
				}},
			},
		}},
	}, {
		Source: src,
		Entry: &gspb.Edges_Edge_{&gspb.Edges_Edge{
			Kind:   &gspb.Edges_Edge_KytheKind{scpb.EdgeKind_PARAM},
			Target: &spb.VName{Signature: "param0"},
		}},
	}, {
		Source: src,
		Entry: &gspb.Edges_Edge_{&gspb.Edges_Edge{
			Kind:    &gspb.Edges_Edge_KytheKind{scpb.EdgeKind_PARAM},
			Ordinal: 1,
			Target:  &spb.VName{Signature: "param1"},
		}},
	}, {
		Source: src,
		Entry: &gspb.Edges_Edge_{&gspb.Edges_Edge{
			Kind:    &gspb.Edges_Edge_KytheKind{scpb.EdgeKind_CHILD_OF},
			Reverse: true,
			Target:  &spb.VName{Signature: "child1"},
		}},
	}, {
		Source: src,
		Entry: &gspb.Edges_Edge_{&gspb.Edges_Edge{
			Kind:    &gspb.Edges_Edge_KytheKind{scpb.EdgeKind_CHILD_OF},
			Reverse: true,
			Target:  &spb.VName{Signature: "child2"},
		}},
	}, {
		Source: src,
		Entry: &gspb.Edges_Target_{&gspb.Edges_Target{
			Node: &scpb.Node{
				Source: &spb.VName{Signature: "child1"},
				Kind:   &scpb.Node_KytheKind{scpb.NodeKind_FUNCTION},
			},
		}},
	}, {
		Source: src,
		Entry: &gspb.Edges_Target_{&gspb.Edges_Target{
			Node: &scpb.Node{
				Source:  &spb.VName{Signature: "child2"},
				Kind:    &scpb.Node_KytheKind{scpb.NodeKind_RECORD},
				Subkind: &scpb.Node_KytheSubkind{scpb.Subkind_CLASS},
			},
		}},
	}, {
		Source: src,
		Entry: &gspb.Edges_Target_{&gspb.Edges_Target{
			Node: &scpb.Node{
				Source:  &spb.VName{Signature: "param0"},
				Kind:    &scpb.Node_KytheKind{scpb.NodeKind_VARIABLE},
				Subkind: &scpb.Node_KytheSubkind{scpb.Subkind_LOCAL_PARAMETER},
			},
		}},
	}}
	for _, fd := range edgeEntries {
		mustWriteEdges(t, w, fd)
	}

	if err := w.Close(); err != nil {
		t.Fatal(err)
	}

	gs := NewService(ctx, db)
	srcTicket := kytheuri.ToString(src)

	t.Run("source_node", makeEdgesTestCase(ctx, gs, &gpb.EdgesRequest{
		Ticket: []string{srcTicket},
		Kind:   []string{"non_existent_kind"},
		Filter: []string{"**"},
	}, &gpb.EdgesReply{
		Nodes: map[string]*cpb.NodeInfo{
			srcTicket: &cpb.NodeInfo{
				Facts: map[string][]byte{
					facts.NodeKind: []byte(kinds.Record),
					facts.Text:     []byte("value"),
				},
			},
		},
	}))

	t.Run("filter_node_kind", makeEdgesTestCase(ctx, gs, &gpb.EdgesRequest{
		Ticket: []string{srcTicket},
		Kind:   []string{"non_existent_kind"},
		Filter: []string{"/kythe/node/kind"},
	}, &gpb.EdgesReply{
		Nodes: map[string]*cpb.NodeInfo{
			srcTicket: {
				Facts: map[string][]byte{
					facts.NodeKind: []byte(kinds.Record),
				},
			},
		},
	}))

	t.Run("ordinals", makeEdgesTestCase(ctx, gs, &gpb.EdgesRequest{
		Ticket: []string{srcTicket},
		Kind:   []string{edges.Param},
	}, &gpb.EdgesReply{
		EdgeSets: map[string]*gpb.EdgeSet{
			srcTicket: {
				Groups: map[string]*gpb.EdgeSet_Group{
					edges.Param: {
						Edge: []*gpb.EdgeSet_Group_Edge{{
							Ordinal:      0,
							TargetTicket: "kythe:#param0",
						}, {
							Ordinal:      1,
							TargetTicket: "kythe:#param1",
						}},
					},
				},
			},
		},
	}))

	t.Run("reverse", makeEdgesTestCase(ctx, gs, &gpb.EdgesRequest{
		Ticket: []string{srcTicket},
		Kind:   []string{"%" + edges.ChildOf},
	}, &gpb.EdgesReply{
		EdgeSets: map[string]*gpb.EdgeSet{
			srcTicket: {
				Groups: map[string]*gpb.EdgeSet_Group{
					"%" + edges.ChildOf: {
						Edge: []*gpb.EdgeSet_Group_Edge{{
							TargetTicket: "kythe:#child1",
						}, {
							TargetTicket: "kythe:#child2",
						}},
					},
				},
			},
		},
	}))

	t.Run("filtered_targets", makeEdgesTestCase(ctx, gs, &gpb.EdgesRequest{
		Ticket: []string{srcTicket},
		Kind:   []string{"%" + edges.ChildOf},
		Filter: []string{"**"},
	}, &gpb.EdgesReply{
		EdgeSets: map[string]*gpb.EdgeSet{
			srcTicket: {
				Groups: map[string]*gpb.EdgeSet_Group{
					"%" + edges.ChildOf: {
						Edge: []*gpb.EdgeSet_Group_Edge{{
							TargetTicket: "kythe:#child1",
						}, {
							TargetTicket: "kythe:#child2",
						}},
					},
				},
			},
		},
		Nodes: map[string]*cpb.NodeInfo{
			srcTicket: &cpb.NodeInfo{
				Facts: map[string][]byte{
					facts.NodeKind: []byte(kinds.Record),
					facts.Text:     []byte("value"),
				},
			},
			"kythe:#child1": &cpb.NodeInfo{
				Facts: map[string][]byte{
					facts.NodeKind: []byte(kinds.Function),
				},
			},
			"kythe:#child2": &cpb.NodeInfo{
				Facts: map[string][]byte{
					facts.NodeKind: []byte(kinds.Record),
					facts.Subkind:  []byte(kinds.Class),
				},
			},
		},
	}))

	t.Run("all", makeEdgesTestCase(ctx, gs, &gpb.EdgesRequest{
		Ticket: []string{srcTicket},
		Filter: []string{facts.NodeKind},
	}, &gpb.EdgesReply{
		EdgeSets: map[string]*gpb.EdgeSet{
			srcTicket: {
				Groups: map[string]*gpb.EdgeSet_Group{
					edges.Param: {
						Edge: []*gpb.EdgeSet_Group_Edge{{
							Ordinal:      0,
							TargetTicket: "kythe:#param0",
						}, {
							Ordinal:      1,
							TargetTicket: "kythe:#param1",
						}},
					},
					"%" + edges.ChildOf: {
						Edge: []*gpb.EdgeSet_Group_Edge{{
							TargetTicket: "kythe:#child1",
						}, {
							TargetTicket: "kythe:#child2",
						}},
					},
				},
			},
		},
		Nodes: map[string]*cpb.NodeInfo{
			srcTicket:       &cpb.NodeInfo{Facts: map[string][]byte{facts.NodeKind: []byte(kinds.Record)}},
			"kythe:#child1": &cpb.NodeInfo{Facts: map[string][]byte{facts.NodeKind: []byte(kinds.Function)}},
			"kythe:#child2": &cpb.NodeInfo{Facts: map[string][]byte{facts.NodeKind: []byte(kinds.Record)}},
			"kythe:#param0": &cpb.NodeInfo{Facts: map[string][]byte{facts.NodeKind: []byte(kinds.Variable)}},
		},
	}))
}

func TestServingNodes(t *testing.T) {
	ctx := context.Background()
	db := inmemory.NewKeyValueDB()
	w, err := db.Writer(ctx)
	if err != nil {
		t.Fatal(err)
	}

	// Mark table as columnar
	mustWrite(t, w, []byte(ColumnarTableKeyMarker), []byte{})

	edgeEntries := []*gspb.Edges{{
		Source: &spb.VName{Signature: "node1"},
		Entry: &gspb.Edges_Index_{&gspb.Edges_Index{
			Node: &scpb.Node{
				Kind: &scpb.Node_KytheKind{scpb.NodeKind_RECORD},
				Fact: []*scpb.Fact{{
					Name:  &scpb.Fact_KytheName{scpb.FactName_TEXT},
					Value: []byte("value"),
				}},
			},
		}},
	}, {
		Source: &spb.VName{Signature: "node2"},
		Entry: &gspb.Edges_Index_{&gspb.Edges_Index{
			Node: &scpb.Node{
				Subkind: &scpb.Node_KytheSubkind{scpb.Subkind_CLASS},
				Fact: []*scpb.Fact{{
					Name:  &scpb.Fact_KytheName{scpb.FactName_TEXT},
					Value: []byte("text"),
				}},
			},
		}},
	}, {
		Source: &spb.VName{Signature: "node3"},
		Entry: &gspb.Edges_Index_{&gspb.Edges_Index{
			Node: &scpb.Node{
				Kind:    &scpb.Node_KytheKind{scpb.NodeKind_VARIABLE},
				Subkind: &scpb.Node_KytheSubkind{scpb.Subkind_LOCAL_PARAMETER},
				Fact: []*scpb.Fact{{
					Name:  &scpb.Fact_KytheName{scpb.FactName_TEXT},
					Value: []byte("text3"),
				}},
			},
		}},
	}}
	for _, fd := range edgeEntries {
		mustWriteEdges(t, w, fd)
	}

	if err := w.Close(); err != nil {
		t.Fatal(err)
	}

	gs := NewService(ctx, db)

	t.Run("nodes", makeNodesTestCase(ctx, gs, &gpb.NodesRequest{
		Ticket: []string{"kythe:#node1", "kythe:#node2", "kythe:#bad", "kythe:#node3"},
	}, &gpb.NodesReply{
		Nodes: map[string]*cpb.NodeInfo{
			"kythe:#node1": &cpb.NodeInfo{
				Facts: map[string][]byte{
					facts.NodeKind: []byte(kinds.Record),
					facts.Text:     []byte("value"),
				},
			},
			"kythe:#node2": &cpb.NodeInfo{
				Facts: map[string][]byte{
					facts.Subkind: []byte(kinds.Class),
					facts.Text:    []byte("text"),
				},
			},
			"kythe:#node3": &cpb.NodeInfo{
				Facts: map[string][]byte{
					facts.NodeKind: []byte(kinds.Variable),
					facts.Subkind:  []byte(kinds.LocalParameter),
					facts.Text:     []byte("text3"),
				},
			},
		},
	}))

	t.Run("single_node", makeNodesTestCase(ctx, gs, &gpb.NodesRequest{
		Ticket: []string{"kythe:#node1"},
	}, &gpb.NodesReply{
		Nodes: map[string]*cpb.NodeInfo{
			"kythe:#node1": &cpb.NodeInfo{
				Facts: map[string][]byte{
					facts.NodeKind: []byte(kinds.Record),
					facts.Text:     []byte("value"),
				},
			},
		},
	}))

	t.Run("filter", makeNodesTestCase(ctx, gs, &gpb.NodesRequest{
		Ticket: []string{"kythe:#node1", "kythe:#node2", "kythe:#bad", "kythe:#node3"},
		Filter: []string{facts.NodeKind},
	}, &gpb.NodesReply{
		Nodes: map[string]*cpb.NodeInfo{
			"kythe:#node1": &cpb.NodeInfo{
				Facts: map[string][]byte{
					facts.NodeKind: []byte(kinds.Record),
				},
			},
			"kythe:#node3": &cpb.NodeInfo{
				Facts: map[string][]byte{
					facts.NodeKind: []byte(kinds.Variable),
				},
			},
		},
	}))
}

func makeEdgesTestCase(ctx context.Context, gs graph.Service, req *gpb.EdgesRequest, expected *gpb.EdgesReply) func(*testing.T) {
	return func(t *testing.T) {
		reply, err := gs.Edges(ctx, req)
		if err != nil {
			t.Fatalf("Edges error: %v", err)
		}
		if diff := compare.ProtoDiff(expected, reply); diff != "" {
			t.Fatalf("EdgesReply differences: (- expected; + found)\n%s", diff)
		}
	}
}

func makeNodesTestCase(ctx context.Context, gs graph.Service, req *gpb.NodesRequest, expected *gpb.NodesReply) func(*testing.T) {
	return func(t *testing.T) {
		reply, err := gs.Nodes(ctx, req)
		if err != nil {
			t.Fatalf("Nodes error: %v", err)
		}
		if diff := compare.ProtoDiff(expected, reply); diff != "" {
			t.Fatalf("NodesReply differences: (- expected; + found)\n%s", diff)
		}
	}
}
