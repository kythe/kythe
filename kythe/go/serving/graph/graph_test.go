/*
 * Copyright 2015 The Kythe Authors. All rights reserved.
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

	"kythe.io/kythe/go/storage/table"
	"kythe.io/kythe/go/test/testutil"
	"kythe.io/kythe/go/util/kytheuri"

	"bitbucket.org/creachadair/stringset"
	"golang.org/x/text/encoding"
	"golang.org/x/text/encoding/unicode"
	"golang.org/x/text/transform"
	"google.golang.org/protobuf/proto"

	cpb "kythe.io/kythe/proto/common_go_proto"
	gpb "kythe.io/kythe/proto/graph_go_proto"
	srvpb "kythe.io/kythe/proto/serving_go_proto"
)

var (
	ctx = context.Background()

	nodes = []*srvpb.Node{
		{
			Ticket: "kythe://someCorpus?lang=otpl#signature",
			Fact:   makeFactList("/kythe/node/kind", "testNode"),
		}, {
			Ticket: "kythe://someCorpus#aTicketSig",
			Fact:   makeFactList("/kythe/node/kind", "testNode"),
		}, {
			Ticket: "kythe://someCorpus?lang=otpl#something",
			Fact: makeFactList(
				"/kythe/node/kind", "name",
				"/some/other/fact", "value",
			),
		}, {
			Ticket: "kythe://someCorpus?lang=otpl#sig2",
			Fact:   makeFactList("/kythe/node/kind", "name"),
		}, {
			Ticket: "kythe://someCorpus?lang=otpl#sig3",
			Fact:   makeFactList("/kythe/node/kind", "name"),
		}, {
			Ticket: "kythe://someCorpus?lang=otpl#sig4",
			Fact:   makeFactList("/kythe/node/kind", "name"),
		}, {
			Ticket: "kythe://someCorpus?lang=otpl?path=/some/valid/path#a83md71",
			Fact: makeFactList(
				"/kythe/node/kind", "file",
				"/kythe/text", "; some file content here\nfinal line\n",
				"/kythe/text/encoding", "utf-8",
			),
		}, {
			Ticket: "kythe://c?lang=otpl?path=/a/path#6-9",
			Fact: makeFactList(
				"/kythe/node/kind", "anchor",
				"/kythe/loc/start", "6",
				"/kythe/loc/end", "9",
			),
		}, {
			Ticket: "kythe://c?lang=otpl?path=/a/path#27-33",
			Fact: makeFactList(
				"/kythe/node/kind", "anchor",
				"/kythe/loc/start", "27",
				"/kythe/loc/end", "33",
			),
		}, {
			Ticket: "kythe://c?lang=otpl?path=/a/path#map",
			Fact:   makeFactList("/kythe/node/kind", "function"),
		}, {
			Ticket: "kythe://core?lang=otpl#empty?",
			Fact:   makeFactList("/kythe/node/kind", "function"),
		}, {
			Ticket: "kythe://c?lang=otpl?path=/a/path#51-55",
			Fact: makeFactList(
				"/kythe/node/kind", "anchor",
				"/kythe/loc/start", "51",
				"/kythe/loc/end", "55",
			),
		}, {
			Ticket: "kythe://core?lang=otpl#cons",
			Fact: makeFactList(
				"/kythe/node/kind", "function",
				// Canary to ensure we don't patch anchor facts in non-anchor nodes
				"/kythe/loc/start", "51",
			),
		}, {
			Ticket: "kythe://c?path=/a/path",
			Fact: makeFactList(
				"/kythe/node/kind", "file",
				"/kythe/text/encoding", "utf-8",
				"/kythe/text", "some random text\nhere and  \n  there\nsome random text\nhere and  \n  there\n",
			),
		}, {
			Ticket: "kythe:?path=some/utf16/file",
			Fact: []*cpb.Fact{{
				Name:  "/kythe/text/encoding",
				Value: []byte("utf-16le"),
			}, {
				Name:  "/kythe/node/kind",
				Value: []byte("file"),
			}, {
				Name:  "/kythe/text",
				Value: encodeText(utf16LE, "これはいくつかのテキストです\n"),
			}},
		}, {
			Ticket: "kythe:?path=some/utf16/file#0-4",
			Fact: makeFactList(
				"/kythe/node/kind", "anchor",
				"/kythe/loc/start", "0",
				"/kythe/loc/end", "4",
			),
		},
	}

	tbl = &testTable{
		Nodes: nodes,
		EdgeSets: []*srvpb.PagedEdgeSet{
			{
				TotalEdges: 3,

				Source: getNode("kythe://someCorpus?lang=otpl#something"),
				Group: []*srvpb.EdgeGroup{
					{
						Kind: "someEdgeKind",
						Edge: getEdgeTargets(
							"kythe://someCorpus#aTicketSig",
						),
					},
					{
						Kind: "anotherEdge",
						Edge: getEdgeTargets(
							"kythe://someCorpus#aTicketSig",
							"kythe://someCorpus?lang=otpl#signature",
						),
					},
				},
			}, {
				TotalEdges: 6,

				Source: getNode("kythe://someCorpus?lang=otpl#signature"),
				Group: []*srvpb.EdgeGroup{{
					Kind: "%/kythe/edge/ref",
					Edge: getEdgeTargets(
						"kythe://c?lang=otpl?path=/a/path#51-55",
						"kythe:?path=some/utf16/file#0-4",
					),
				}, {
					Kind: "%/kythe/edge/defines/binding",
					Edge: getEdgeTargets("kythe://c?lang=otpl?path=/a/path#27-33"),
				}},

				PageIndex: []*srvpb.PageIndex{{
					PageKey:   "firstPage",
					EdgeKind:  "someEdgeKind",
					EdgeCount: 2,
				}, {
					PageKey:   "secondPage",
					EdgeKind:  "anotherEdge",
					EdgeCount: 1,
				}},
			}, {
				TotalEdges: 2,

				Source: getNode("kythe:?path=some/utf16/file#0-4"),
				Group: []*srvpb.EdgeGroup{{
					Kind: "/kythe/edge/ref",
					Edge: getEdgeTargets("kythe://someCorpus?lang=otpl#signature"),
				}},
			}, {
				TotalEdges: 1,

				Source: getNode("kythe:?path=some/utf16/file"),
			}, {
				TotalEdges: 2,

				Source: getNode("kythe://c?lang=otpl?path=/a/path#51-55"),
				Group: []*srvpb.EdgeGroup{{
					Kind: "/kythe/edge/ref",
					Edge: getEdgeTargets("kythe://someCorpus?lang=otpl#signature"),
				}},
			}, {
				TotalEdges: 2,

				Source: getNode("kythe://c?lang=otpl?path=/a/path#27-33"),
				Group: []*srvpb.EdgeGroup{{
					Kind: "/kythe/edge/defines/binding",
					Edge: getEdgeTargets("kythe://someCorpus?lang=otpl#signature"),
				}},
			},
		},
		EdgePages: []*srvpb.EdgePage{
			{
				PageKey:      "orphanedEdgePage",
				SourceTicket: "kythe://someCorpus?lang=otpl#something",
			}, {
				PageKey:      "firstPage",
				SourceTicket: "kythe://someCorpus?lang=otpl#signature",
				EdgesGroup: &srvpb.EdgeGroup{
					Kind: "someEdgeKind",
					Edge: getEdgeTargets(
						"kythe://someCorpus?lang=otpl#sig3",
						"kythe://someCorpus?lang=otpl#sig4",
					),
				},
			}, {
				PageKey:      "secondPage",
				SourceTicket: "kythe://someCorpus?lang=otpl#signature",
				EdgesGroup: &srvpb.EdgeGroup{
					Kind: "anotherEdge",
					Edge: getEdgeTargets(
						"kythe://someCorpus?lang=otpl#sig2",
					),
				},
			},
		},
	}
)

func getEdgeTargets(tickets ...string) []*srvpb.EdgeGroup_Edge {
	es := make([]*srvpb.EdgeGroup_Edge, len(tickets))
	for i, t := range tickets {
		es[i] = &srvpb.EdgeGroup_Edge{
			Target:  getNode(t),
			Ordinal: int32(i),
		}
	}
	return es
}

func getNode(t string) *srvpb.Node {
	for _, n := range nodes {
		if n.Ticket == t {
			return n
		}
	}
	return &srvpb.Node{Ticket: t}
}

var utf16LE = unicode.UTF16(unicode.LittleEndian, unicode.IgnoreBOM)

func encodeText(e encoding.Encoding, text string) []byte {
	res, _, err := transform.Bytes(e.NewEncoder(), []byte(text))
	if err != nil {
		panic(err)
	}
	return res
}

func TestNodes(t *testing.T) {
	st := tbl.Construct(t)

	for _, node := range tbl.Nodes {
		reply, err := st.Nodes(ctx, &gpb.NodesRequest{
			Ticket: []string{node.Ticket},
		})
		testutil.Fatalf(t, "NodesRequest error: %v", err)

		if len(reply.Nodes) != 1 {
			t.Fatalf("Expected 1 node for %q; found %d: {%v}", node.Ticket, len(reply.Nodes), reply)
		} else if err := testutil.DeepEqual(nodeInfo(node), reply.Nodes[node.Ticket]); err != nil {
			t.Fatal(err)
		}
	}

	var tickets []string
	expected := make(map[string]*cpb.NodeInfo)
	for _, n := range tbl.Nodes {
		tickets = append(tickets, n.Ticket)
		expected[n.Ticket] = nodeInfo(n)
	}
	reply, err := st.Nodes(ctx, &gpb.NodesRequest{Ticket: tickets})
	testutil.Fatalf(t, "NodesRequest error: %v", err)

	if err := testutil.DeepEqual(expected, reply.Nodes); err != nil {
		t.Fatal(err)
	}
}

func TestNodesMissing(t *testing.T) {
	st := tbl.Construct(t)
	reply, err := st.Nodes(ctx, &gpb.NodesRequest{
		Ticket: []string{"kythe:#someMissingTicket"},
	})
	testutil.Fatalf(t, "NodesRequest error: %v", err)

	if len(reply.Nodes) > 0 {
		t.Fatalf("Received unexpected reply for missing node: {%v}", reply)
	}
}

func TestEdgesSinglePage(t *testing.T) {
	tests := []struct {
		Ticket string
		Kinds  []string

		EdgeSet *srvpb.PagedEdgeSet
	}{{
		Ticket:  tbl.EdgeSets[0].Source.Ticket,
		EdgeSet: tbl.EdgeSets[0],
	}, {
		Ticket:  tbl.EdgeSets[0].Source.Ticket,
		Kinds:   []string{"someEdgeKind", "anotherEdge"},
		EdgeSet: tbl.EdgeSets[0],
	}}

	st := tbl.Construct(t)

	for _, test := range tests {
		reply, err := st.Edges(ctx, &gpb.EdgesRequest{
			Ticket: []string{test.Ticket},
			Kind:   test.Kinds,
		})
		testutil.Fatalf(t, "EdgesRequest error: %v", err)

		if len(reply.Nodes) > 0 {
			t.Errorf("Received unexpected nodes in EdgesReply: {%v}", reply)
		}
		if reply.NextPageToken != "" {
			t.Errorf("Received unexpected next_page_token in EdgesReply: {%v}", reply)
		}
		if len(reply.EdgeSets) != 1 {
			t.Errorf("Expected 1 EdgeSet in EdgesReply; found %d: {%v}", len(reply.EdgeSets), reply)
		}

		expected := edgeSet(test.Kinds, test.EdgeSet, nil)
		if err := testutil.DeepEqual(expected, reply.EdgeSets[test.Ticket]); err != nil {
			t.Error(err)
		}
		if err := testutil.DeepEqual(map[string]int64{
			"someEdgeKind": 1,
			"anotherEdge":  2,
		}, reply.TotalEdgesByKind); err != nil {
			t.Error(err)
		}
	}
}

func TestEdgesLastPage(t *testing.T) {
	tests := [][]string{
		nil, // all kinds
		{"%/kythe/edge/ref"},
		{"%/kythe/edge/defines/binding"},
	}

	ticket := tbl.EdgeSets[1].Source.Ticket
	es := tbl.EdgeSets[1]
	pages := []*srvpb.EdgePage{tbl.EdgePages[1], tbl.EdgePages[2]}

	st := tbl.Construct(t)

	for _, kinds := range tests {
		reply, err := st.Edges(ctx, &gpb.EdgesRequest{
			Ticket: []string{ticket},
			Kind:   kinds,
		})
		testutil.Fatalf(t, "EdgesRequest error: %v", err)

		if len(reply.Nodes) > 0 {
			t.Errorf("Received unexpected nodes in EdgesReply: {%v}", reply)
		}
		if reply.NextPageToken != "" {
			t.Errorf("Received unexpected next_page_token in EdgesReply: {%v}", reply)
		}
		if len(reply.EdgeSets) != 1 {
			t.Fatalf("Expected 1 EdgeSet in EdgesReply; found %d: {%v}", len(reply.EdgeSets), reply)
		}

		expected := edgeSet(kinds, es, pages)
		if err := testutil.DeepEqual(expected, reply.EdgeSets[ticket]); err != nil {
			t.Error(err)
		}
	}
}

func TestEdgesMultiPage(t *testing.T) {
	tests := []struct {
		Ticket string
		Kinds  []string

		EdgeSet *srvpb.PagedEdgeSet
		Pages   []*srvpb.EdgePage
	}{{
		Ticket:  tbl.EdgeSets[1].Source.Ticket,
		EdgeSet: tbl.EdgeSets[1],
		Pages:   []*srvpb.EdgePage{tbl.EdgePages[1], tbl.EdgePages[2]},
	}}

	st := tbl.Construct(t)

	for _, test := range tests {
		reply, err := st.Edges(ctx, &gpb.EdgesRequest{
			Ticket: []string{test.Ticket},
			Kind:   test.Kinds,
		})
		testutil.Fatalf(t, "EdgesRequest error: %v", err)

		if len(reply.Nodes) > 0 {
			t.Errorf("Received unexpected nodes in EdgesReply: {%v}", reply)
		}
		if reply.NextPageToken != "" {
			t.Errorf("Received unexpected next_page_token in EdgesReply: {%v}", reply)
		}
		if len(reply.EdgeSets) != 1 {
			t.Errorf("Expected 1 EdgeSet in EdgesReply; found %d: {%v}", len(reply.EdgeSets), reply)
		}

		expected := edgeSet(test.Kinds, test.EdgeSet, test.Pages)
		if err := testutil.DeepEqual(expected, reply.EdgeSets[test.Ticket]); err != nil {
			t.Error(err)
		}
		if err := testutil.DeepEqual(map[string]int64{
			"%/kythe/edge/defines/binding": 1,
			"%/kythe/edge/ref":             2,
			"someEdgeKind":                 2,
			"anotherEdge":                  1,
		}, reply.TotalEdgesByKind); err != nil {
			t.Error(err)
		}
	}
}

func TestEdgesMissing(t *testing.T) {
	st := tbl.Construct(t)
	reply, err := st.Edges(ctx, &gpb.EdgesRequest{
		Ticket: []string{"kythe:#someMissingTicket"},
	})
	testutil.Fatalf(t, "EdgesRequest error: %v", err)

	if len(reply.EdgeSets) > 0 || len(reply.Nodes) > 0 || reply.NextPageToken != "" {
		t.Fatalf("Received unexpected reply for missing edges: {%v}", reply)
	}
}

func nodeInfo(n *srvpb.Node) *cpb.NodeInfo {
	ni := &cpb.NodeInfo{Facts: make(map[string][]byte, len(n.Fact))}
	for _, f := range n.Fact {
		ni.Facts[f.Name] = f.Value
	}
	return ni
}

func makeFactList(keyVals ...string) []*cpb.Fact {
	if len(keyVals)%2 != 0 {
		panic("makeFactList: odd number of key values")
	}
	facts := make([]*cpb.Fact, 0, len(keyVals)/2)
	for i := 0; i < len(keyVals); i += 2 {
		facts = append(facts, &cpb.Fact{
			Name:  keyVals[i],
			Value: []byte(keyVals[i+1]),
		})
	}
	return facts
}

func edgeSet(kinds []string, pes *srvpb.PagedEdgeSet, pages []*srvpb.EdgePage) *gpb.EdgeSet {
	set := stringset.New(kinds...)

	es := &gpb.EdgeSet{Groups: make(map[string]*gpb.EdgeSet_Group, len(pes.Group))}
	for _, g := range pes.Group {
		if set.Contains(g.Kind) || len(set) == 0 {
			es.Groups[g.Kind] = &gpb.EdgeSet_Group{
				Edge: e2e(g.Edge),
			}
		}
	}
	for _, ep := range pages {
		g := ep.EdgesGroup
		if set.Contains(g.Kind) || len(set) == 0 {
			es.Groups[g.Kind] = &gpb.EdgeSet_Group{
				Edge: e2e(g.Edge),
			}
		}
	}
	return es
}

type testTable struct {
	Nodes     []*srvpb.Node
	EdgePages []*srvpb.EdgePage
	EdgeSets  []*srvpb.PagedEdgeSet
}

func (tbl *testTable) Construct(t *testing.T) *Table {
	p := make(testProtoTable)
	var tickets stringset.Set
	for _, n := range tbl.Nodes {
		tickets.Add(n.Ticket)
	}
	for _, es := range tbl.EdgeSets {
		tickets.Discard(es.Source.Ticket)
		testutil.Fatalf(t, "Error writing edge set: %v", p.Put(ctx, EdgeSetKey(mustFix(t, es.Source.Ticket)), es))
	}
	// Fill in EdgeSets for zero-degree nodes
	for ticket := range tickets {
		es := &srvpb.PagedEdgeSet{
			Source: getNode(ticket),
		}
		testutil.Fatalf(t, "Error writing edge set: %v", p.Put(ctx, EdgeSetKey(mustFix(t, es.Source.Ticket)), es))
	}
	for _, ep := range tbl.EdgePages {
		testutil.Fatalf(t, "Error writing edge page: %v", p.Put(ctx, EdgePageKey(ep.PageKey), ep))
	}
	return NewCombinedTable(p)
}

func mustFix(t *testing.T, ticket string) string {
	ft, err := kytheuri.Fix(ticket)
	if err != nil {
		t.Fatalf("Error fixing ticket %q: %v", ticket, err)
	}
	return ft
}

type testProtoTable map[string]proto.Message

func (t testProtoTable) Put(_ context.Context, key []byte, val proto.Message) error {
	t[string(key)] = val
	return nil
}

func (t testProtoTable) Lookup(_ context.Context, key []byte, msg proto.Message) error {
	m, ok := t[string(key)]
	if !ok {
		return table.ErrNoSuchKey
	}
	proto.Merge(msg, m)
	return nil
}

func (t testProtoTable) LookupValues(_ context.Context, key []byte, m proto.Message, f func(proto.Message) error) error {
	val, ok := t[string(key)]
	if !ok {
		return nil
	}
	msg := m.ProtoReflect().New().Interface()
	proto.Merge(msg, val)
	if err := f(msg); err != nil && err != table.ErrStopLookup {
		return err
	}
	return nil
}

func (t testProtoTable) Buffered() table.BufferedProto { panic("UNIMPLEMENTED") }

func (t testProtoTable) Close(_ context.Context) error { return nil }
