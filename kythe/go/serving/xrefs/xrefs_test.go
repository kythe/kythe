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

package xrefs

import (
	"bytes"
	"sort"
	"testing"

	"kythe.io/kythe/go/services/xrefs"
	"kythe.io/kythe/go/storage/table"
	"kythe.io/kythe/go/test/testutil"
	"kythe.io/kythe/go/util/kytheuri"
	"kythe.io/kythe/go/util/stringset"

	"github.com/golang/protobuf/proto"
	"golang.org/x/net/context"
	"golang.org/x/text/encoding"
	"golang.org/x/text/encoding/unicode"
	"golang.org/x/text/transform"

	cpb "kythe.io/kythe/proto/common_proto"
	srvpb "kythe.io/kythe/proto/serving_proto"
	xpb "kythe.io/kythe/proto/xref_proto"
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
			Ticket: "kythe://someCorpus?path=some/path#aFileNode",
			Fact: makeFactList(
				"/kythe/node/kind", "file",
				"/kythe/text/encoding", "utf-8",
				"/kythe/text", "some random text\nhere and  \n  there\nsome random text\nhere and  \n  there\n",
			),
		}, {
			Ticket: "kythe://someCorpus?path=some/utf16/file#utf16FTW",
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
				}, {
					Kind: "/kythe/edge/childof",
					Edge: getEdgeTargets("kythe://someCorpus?path=some/utf16/file#utf16FTW"),
				}},
			}, {
				TotalEdges: 1,

				Source: getNode("kythe://someCorpus?path=some/utf16/file#utf16FTW"),
				Group: []*srvpb.EdgeGroup{{
					Kind: "%/kythe/edge/childof",
					Edge: getEdgeTargets("kythe:?path=some/utf16/file#0-4"),
				}},
			}, {
				TotalEdges: 2,

				Source: getNode("kythe://c?lang=otpl?path=/a/path#51-55"),
				Group: []*srvpb.EdgeGroup{{
					Kind: "/kythe/edge/ref",
					Edge: getEdgeTargets("kythe://someCorpus?lang=otpl#signature"),
				}, {
					Kind: "/kythe/edge/childof",
					Edge: getEdgeTargets("kythe://someCorpus?path=some/path#aFileNode"),
				}},
			}, {
				TotalEdges: 2,

				Source: getNode("kythe://c?lang=otpl?path=/a/path#27-33"),
				Group: []*srvpb.EdgeGroup{{
					Kind: "/kythe/edge/defines/binding",
					Edge: getEdgeTargets("kythe://someCorpus?lang=otpl#signature"),
				}, {
					Kind: "/kythe/edge/childof",
					Edge: getEdgeTargets("kythe://someCorpus?path=some/path#aFileNode"),
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
		Decorations: []*srvpb.FileDecorations{
			{
				File: &srvpb.File{
					Ticket:   "kythe://someCorpus?lang=otpl?path=/some/valid/path#a83md71",
					Text:     []byte("; some file content here\nfinal line\n"),
					Encoding: "utf-8",
				},
			},
			{
				File: &srvpb.File{
					Ticket: "kythe://someCorpus?lang=otpl?path=/a/path#b7te37tn4",
					Text: []byte(`(defn map [f coll]
  (if (empty? coll)
    []
    (cons (f (first coll)) (map f (rest coll)))))
`),
					Encoding: "utf-8",
				},
				Decoration: []*srvpb.FileDecorations_Decoration{
					{
						Anchor: &srvpb.RawAnchor{
							Ticket:      "kythe://c?lang=otpl?path=/a/path#6-9",
							StartOffset: 6,
							EndOffset:   9,
						},
						Kind:   "/kythe/defines/binding",
						Target: "kythe://c?lang=otpl?path=/a/path#map",
					},
					{
						Anchor: &srvpb.RawAnchor{
							Ticket:      "kythe://c?lang=otpl?path=/a/path#27-33",
							StartOffset: 27,
							EndOffset:   33,
						},
						Kind:   "/kythe/refs",
						Target: "kythe://core?lang=otpl#empty?",
					},
					{
						Anchor: &srvpb.RawAnchor{
							Ticket:      "kythe://c?lang=otpl?path=/a/path#51-55",
							StartOffset: 51,
							EndOffset:   55,
						},
						Kind:   "/kythe/refs",
						Target: "kythe://core?lang=otpl#cons",
					},
				},
				Target: getNodes("kythe://c?lang=otpl?path=/a/path#map", "kythe://core?lang=otpl#empty?", "kythe://core?lang=otpl#cons"),
			},
		},

		RefSets: []*srvpb.PagedCrossReferences{{
			SourceTicket: "kythe://someCorpus?lang=otpl#signature",

			Group: []*srvpb.PagedCrossReferences_Group{{
				Kind: "%/kythe/edge/defines/binding",
				Anchor: []*srvpb.ExpandedAnchor{{
					Ticket: "kythe://c?lang=otpl?path=/a/path#27-33",
					Kind:   "/kythe/edge/defines/binding",
					Parent: "kythe://someCorpus?path=some/path#aFileNode",

					Span: &cpb.Span{
						Start: &cpb.Point{
							ByteOffset:   27,
							LineNumber:   2,
							ColumnOffset: 10,
						},
						End: &cpb.Point{
							ByteOffset:   33,
							LineNumber:   3,
							ColumnOffset: 5,
						},
					},

					SnippetSpan: &cpb.Span{
						Start: &cpb.Point{
							ByteOffset: 17,
							LineNumber: 2,
						},
						End: &cpb.Point{
							ByteOffset:   27,
							LineNumber:   2,
							ColumnOffset: 10,
						},
					},
					Snippet: "here and  ",
				}},
			}},

			PageIndex: []*srvpb.PagedCrossReferences_PageIndex{{
				PageKey: "aBcDeFg",
				Kind:    "%/kythe/edge/ref",
				Count:   2,
			}},
		}},
		RefPages: []*srvpb.PagedCrossReferences_Page{{
			PageKey: "aBcDeFg",
			Group: &srvpb.PagedCrossReferences_Group{
				Kind: "%/kythe/edge/ref",
				Anchor: []*srvpb.ExpandedAnchor{{
					Ticket: "kythe:?path=some/utf16/file#0-4",
					Kind:   "/kythe/edge/ref",
					Parent: "kythe://someCorpus?path=some/utf16/file#utf16FTW",

					Span: &cpb.Span{
						Start: &cpb.Point{LineNumber: 1},
						End:   &cpb.Point{ByteOffset: 4, LineNumber: 1, ColumnOffset: 4},
					},

					SnippetSpan: &cpb.Span{
						Start: &cpb.Point{
							LineNumber: 1,
						},
						End: &cpb.Point{
							ByteOffset:   28,
							LineNumber:   1,
							ColumnOffset: 28,
						},
					},
					Snippet: "これはいくつかのテキストです",
				}, {
					Ticket: "kythe://c?lang=otpl?path=/a/path#51-55",
					Kind:   "/kythe/edge/ref",
					Parent: "kythe://someCorpus?path=some/path#aFileNode",

					Span: &cpb.Span{
						Start: &cpb.Point{
							ByteOffset:   51,
							LineNumber:   4,
							ColumnOffset: 15,
						},
						End: &cpb.Point{
							ByteOffset:   55,
							LineNumber:   5,
							ColumnOffset: 2,
						},
					},

					SnippetSpan: &cpb.Span{
						Start: &cpb.Point{
							ByteOffset: 36,
							LineNumber: 4,
						},
						End: &cpb.Point{
							ByteOffset:   52,
							LineNumber:   4,
							ColumnOffset: 16,
						},
					},
					Snippet: "some random text",
				}},
			},
		}},
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

func getNodes(ts ...string) []*srvpb.Node {
	var res []*srvpb.Node
	for _, t := range ts {
		res = append(res, getNode(t))
	}
	return res
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
		reply, err := st.Nodes(ctx, &xpb.NodesRequest{
			Ticket: []string{node.Ticket},
		})
		testutil.FatalOnErrT(t, "NodesRequest error: %v", err)

		if len(reply.Nodes) != 1 {
			t.Fatalf("Expected 1 node for %q; found %d: {%v}", node.Ticket, len(reply.Nodes), reply)
		} else if err := testutil.DeepEqual(nodeInfo(node), reply.Nodes[node.Ticket]); err != nil {
			t.Fatal(err)
		}
	}

	var tickets []string
	expected := make(map[string]*xpb.NodeInfo)
	for _, n := range tbl.Nodes {
		tickets = append(tickets, n.Ticket)
		expected[n.Ticket] = nodeInfo(n)
	}
	reply, err := st.Nodes(ctx, &xpb.NodesRequest{Ticket: tickets})
	testutil.FatalOnErrT(t, "NodesRequest error: %v", err)

	if err := testutil.DeepEqual(expected, reply.Nodes); err != nil {
		t.Fatal(err)
	}
}

func TestNodesMissing(t *testing.T) {
	st := tbl.Construct(t)
	reply, err := st.Nodes(ctx, &xpb.NodesRequest{
		Ticket: []string{"kythe:#someMissingTicket"},
	})
	testutil.FatalOnErrT(t, "NodesRequest error: %v", err)

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
		reply, err := st.Edges(ctx, &xpb.EdgesRequest{
			Ticket: []string{test.Ticket},
			Kind:   test.Kinds,
		})
		testutil.FatalOnErrT(t, "EdgesRequest error: %v", err)

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
		reply, err := st.Edges(ctx, &xpb.EdgesRequest{
			Ticket: []string{ticket},
			Kind:   kinds,
		})
		testutil.FatalOnErrT(t, "EdgesRequest error: %v", err)

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
		reply, err := st.Edges(ctx, &xpb.EdgesRequest{
			Ticket: []string{test.Ticket},
			Kind:   test.Kinds,
		})
		testutil.FatalOnErrT(t, "EdgesRequest error: %v", err)

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
	reply, err := st.Edges(ctx, &xpb.EdgesRequest{
		Ticket: []string{"kythe:#someMissingTicket"},
	})
	testutil.FatalOnErrT(t, "EdgesRequest error: %v", err)

	if len(reply.EdgeSets) > 0 || len(reply.Nodes) > 0 || reply.NextPageToken != "" {
		t.Fatalf("Received unexpected reply for missing edges: {%v}", reply)
	}
}

func TestDecorationsRefs(t *testing.T) {
	d := tbl.Decorations[1]

	st := tbl.Construct(t)
	reply, err := st.Decorations(ctx, &xpb.DecorationsRequest{
		Location:   &xpb.Location{Ticket: d.File.Ticket},
		References: true,
		Filter:     []string{"**"},
	})
	testutil.FatalOnErrT(t, "DecorationsRequest error: %v", err)

	if len(reply.SourceText) != 0 {
		t.Errorf("Unexpected source text: %q", string(reply.SourceText))
	}
	if reply.Encoding != "" {
		t.Errorf("Unexpected encoding: %q", reply.Encoding)
	}

	expected := refs(xrefs.NewNormalizer(d.File.Text), d.Decoration)
	if err := testutil.DeepEqual(expected, reply.Reference); err != nil {
		t.Fatal(err)
	}

	expectedNodes := nodeInfos(tbl.Nodes[9:11], tbl.Nodes[12:13])
	if err := testutil.DeepEqual(expectedNodes, reply.Nodes); err != nil {
		t.Fatal(err)
	}
}

func TestDecorationsDirtyBuffer(t *testing.T) {
	d := tbl.Decorations[1]

	st := tbl.Construct(t)
	// s/empty?/seq/
	dirty := []byte(`(defn map [f coll]
  (if (seq coll)
    []
    (cons (f (first coll)) (map f (rest coll)))))
`)
	reply, err := st.Decorations(ctx, &xpb.DecorationsRequest{
		Location:    &xpb.Location{Ticket: d.File.Ticket},
		DirtyBuffer: dirty,
		References:  true,
		Filter:      []string{"**"},
	})
	testutil.FatalOnErrT(t, "DecorationsRequest error: %v", err)

	if len(reply.SourceText) != 0 {
		t.Errorf("Unexpected source text: %q", string(reply.SourceText))
	}
	if reply.Encoding != "" {
		t.Errorf("Unexpected encoding: %q", reply.Encoding)
	}

	expected := []*xpb.DecorationsReply_Reference{
		{
			// Unpatched anchor for "map"
			SourceTicket: "kythe://c?lang=otpl?path=/a/path#6-9",
			TargetTicket: "kythe://c?lang=otpl?path=/a/path#map",
			Kind:         "/kythe/defines/binding",

			AnchorStart: &xpb.Location_Point{
				ByteOffset:   6,
				LineNumber:   1,
				ColumnOffset: 6,
			},
			AnchorEnd: &xpb.Location_Point{
				ByteOffset:   9,
				LineNumber:   1,
				ColumnOffset: 9,
			},
		},
		// Skipped anchor for "empty?" (inside edit region)
		{
			// Patched anchor for "cons" (moved backwards by 3 bytes)
			SourceTicket: "kythe://c?lang=otpl?path=/a/path#51-55",
			TargetTicket: "kythe://core?lang=otpl#cons",
			Kind:         "/kythe/refs",
			AnchorStart: &xpb.Location_Point{
				ByteOffset:   48,
				LineNumber:   4,
				ColumnOffset: 5,
			},
			AnchorEnd: &xpb.Location_Point{
				ByteOffset:   52,
				LineNumber:   4,
				ColumnOffset: 9,
			},
		},
	}
	if err := testutil.DeepEqual(expected, reply.Reference); err != nil {
		t.Fatal(err)
	}

	// These are a subset of the anchor nodes in tbl.Decorations[1].  tbl.Nodes[10] is missing because
	// it is the target of an anchor in the edited region.
	expectedNodes := nodeInfos([]*srvpb.Node{tbl.Nodes[9], tbl.Nodes[12]})
	if err := testutil.DeepEqual(expectedNodes, reply.Nodes); err != nil {
		t.Fatal(err)
	}
}

func TestDecorationsNotFound(t *testing.T) {
	st := tbl.Construct(t)
	reply, err := st.Decorations(ctx, &xpb.DecorationsRequest{
		Location: &xpb.Location{
			Ticket: "kythe:#someMissingFileTicket",
		},
	})

	if err == nil {
		t.Fatalf("Unexpected DecorationsReply: {%v}", reply)
	} else if err != xrefs.ErrDecorationsNotFound {
		t.Fatalf("Unexpected Decorations error: %v", err)
	}
}

func TestDecorationsEmpty(t *testing.T) {
	st := tbl.Construct(t)
	reply, err := st.Decorations(ctx, &xpb.DecorationsRequest{
		Location: &xpb.Location{
			Ticket: tbl.Decorations[0].File.Ticket,
		},
		References: true,
	})
	testutil.FatalOnErrT(t, "DecorationsRequest error: %v", err)

	if len(reply.Reference) > 0 {
		t.Fatalf("Unexpected DecorationsReply: {%v}", reply)
	}
}

func TestDecorationsSourceText(t *testing.T) {
	expected := tbl.Decorations[0]

	st := tbl.Construct(t)
	reply, err := st.Decorations(ctx, &xpb.DecorationsRequest{
		Location:   &xpb.Location{Ticket: expected.File.Ticket},
		SourceText: true,
	})
	testutil.FatalOnErrT(t, "DecorationsRequest error: %v", err)

	if !bytes.Equal(reply.SourceText, expected.File.Text) {
		t.Errorf("Expected source text %q; found %q", string(expected.File.Text), string(reply.SourceText))
	}
	if reply.Encoding != expected.File.Encoding {
		t.Errorf("Expected source text %q; found %q", expected.File.Encoding, reply.Encoding)
	}
	if len(reply.Reference) > 0 {
		t.Errorf("Unexpected references in DecorationsReply %v", reply.Reference)
	}
}

func TestCrossReferencesNone(t *testing.T) {
	st := tbl.Construct(t)
	reply, err := st.CrossReferences(ctx, &xpb.CrossReferencesRequest{
		Ticket:            []string{"kythe://someCorpus?lang=otpl#sig2"},
		DocumentationKind: xpb.CrossReferencesRequest_ALL_DOCUMENTATION,
		DefinitionKind:    xpb.CrossReferencesRequest_ALL_DEFINITIONS,
		ReferenceKind:     xpb.CrossReferencesRequest_ALL_REFERENCES,
	})
	testutil.FatalOnErrT(t, "CrossReferencesRequest error: %v", err)

	if len(reply.CrossReferences) > 0 || len(reply.Nodes) > 0 {
		t.Fatalf("Expected empty CrossReferencesReply; found %v", reply)
	}
}

func TestCrossReferences(t *testing.T) {
	ticket := "kythe://someCorpus?lang=otpl#signature"

	st := tbl.Construct(t)
	reply, err := st.CrossReferences(ctx, &xpb.CrossReferencesRequest{
		Ticket:         []string{ticket},
		DefinitionKind: xpb.CrossReferencesRequest_BINDING_DEFINITIONS,
		ReferenceKind:  xpb.CrossReferencesRequest_ALL_REFERENCES,

		ExperimentalSignatures: true,
	})
	testutil.FatalOnErrT(t, "CrossReferencesRequest error: %v", err)

	expected := &xpb.CrossReferencesReply_CrossReferenceSet{
		Ticket:      ticket,
		DisplayName: &xpb.Printable{RawText: ""},

		Reference: []*xpb.CrossReferencesReply_RelatedAnchor{{Anchor: &xpb.Anchor{
			Ticket: "kythe:?path=some/utf16/file#0-4",
			Kind:   "/kythe/edge/ref",
			Parent: "kythe://someCorpus?path=some/utf16/file#utf16FTW",

			Start: &xpb.Location_Point{LineNumber: 1},
			End:   &xpb.Location_Point{ByteOffset: 4, LineNumber: 1, ColumnOffset: 4},

			SnippetStart: &xpb.Location_Point{
				LineNumber: 1,
			},
			SnippetEnd: &xpb.Location_Point{
				ByteOffset:   28,
				LineNumber:   1,
				ColumnOffset: 28,
			},
			Snippet: "これはいくつかのテキストです",
		}}, {Anchor: &xpb.Anchor{
			Ticket: "kythe://c?lang=otpl?path=/a/path#51-55",
			Kind:   "/kythe/edge/ref",
			Parent: "kythe://someCorpus?path=some/path#aFileNode",

			Start: &xpb.Location_Point{
				ByteOffset:   51,
				LineNumber:   4,
				ColumnOffset: 15,
			},
			End: &xpb.Location_Point{
				ByteOffset:   55,
				LineNumber:   5,
				ColumnOffset: 2,
			},

			SnippetStart: &xpb.Location_Point{
				ByteOffset: 36,
				LineNumber: 4,
			},
			SnippetEnd: &xpb.Location_Point{
				ByteOffset:   52,
				LineNumber:   4,
				ColumnOffset: 16,
			},
			Snippet: "some random text",
		}}},

		Definition: []*xpb.CrossReferencesReply_RelatedAnchor{{Anchor: &xpb.Anchor{
			Ticket: "kythe://c?lang=otpl?path=/a/path#27-33",
			Kind:   "/kythe/edge/defines/binding",
			Parent: "kythe://someCorpus?path=some/path#aFileNode",

			Start: &xpb.Location_Point{
				ByteOffset:   27,
				LineNumber:   2,
				ColumnOffset: 10,
			},
			End: &xpb.Location_Point{
				ByteOffset:   33,
				LineNumber:   3,
				ColumnOffset: 5,
			},

			SnippetStart: &xpb.Location_Point{
				ByteOffset: 17,
				LineNumber: 2,
			},
			SnippetEnd: &xpb.Location_Point{
				ByteOffset:   27,
				LineNumber:   2,
				ColumnOffset: 10,
			},
			Snippet: "here and  ",
		}}},
	}

	if err := testutil.DeepEqual(&xpb.CrossReferencesReply_Total{
		Definitions: 1,
		References:  2,
	}, reply.Total); err != nil {
		t.Error(err)
	}

	xr := reply.CrossReferences[ticket]
	if xr == nil {
		t.Fatalf("Missing expected CrossReferences; found: %#v", reply)
	}
	sort.Sort(byOffset(xr.Reference))

	if err := testutil.DeepEqual(expected, xr); err != nil {
		t.Fatal(err)
	}
}

func nodeInfos(nss ...[]*srvpb.Node) map[string]*xpb.NodeInfo {
	m := make(map[string]*xpb.NodeInfo)
	for _, ns := range nss {
		for _, n := range ns {
			m[n.Ticket] = nodeInfo(n)
		}
	}
	return m
}

func TestCallers(t *testing.T) {
	st := tbl.Construct(t)
	reply, err := st.Callers(ctx, &xpb.CallersRequest{})
	if reply != nil || err == nil {
		t.Fatalf("Callers expected to fail")
	}
}

func TestDocumentation(t *testing.T) {
	st := tbl.Construct(t)
	reply, err := st.Documentation(ctx, &xpb.DocumentationRequest{})
	if reply != nil || err == nil {
		t.Fatalf("Documentation expected to fail")
	}
}

// byOffset implements the sort.Interface for *xpb.CrossReferencesReply_RelatedAnchors.
type byOffset []*xpb.CrossReferencesReply_RelatedAnchor

// Implement the sort.Interface.
func (s byOffset) Len() int      { return len(s) }
func (s byOffset) Swap(i, j int) { s[i], s[j] = s[j], s[i] }
func (s byOffset) Less(i, j int) bool {
	return s[i].Anchor.Start.ByteOffset < s[j].Anchor.Start.ByteOffset
}

func nodeInfo(n *srvpb.Node) *xpb.NodeInfo {
	ni := &xpb.NodeInfo{Facts: make(map[string][]byte, len(n.Fact))}
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

func mapFacts(n *xpb.NodeInfo, facts map[string]string) {
	for name := range n.Facts {
		if val, ok := facts[name]; ok {
			n.Facts[name] = []byte(val)
		}
	}
}

func edgeSet(kinds []string, pes *srvpb.PagedEdgeSet, pages []*srvpb.EdgePage) *xpb.EdgeSet {
	set := stringset.New(kinds...)

	es := &xpb.EdgeSet{Groups: make(map[string]*xpb.EdgeSet_Group, len(pes.Group))}
	for _, g := range pes.Group {
		if set.Contains(g.Kind) || len(set) == 0 {
			es.Groups[g.Kind] = &xpb.EdgeSet_Group{
				Edge: e2e(g.Edge),
			}
		}
	}
	for _, ep := range pages {
		g := ep.EdgesGroup
		if set.Contains(g.Kind) || len(set) == 0 {
			es.Groups[g.Kind] = &xpb.EdgeSet_Group{
				Edge: e2e(g.Edge),
			}
		}
	}
	return es
}

func refs(norm *xrefs.Normalizer, ds []*srvpb.FileDecorations_Decoration) (refs []*xpb.DecorationsReply_Reference) {
	for _, d := range ds {
		refs = append(refs, decorationToReference(norm, d))
	}
	return
}

type testTable struct {
	Nodes       []*srvpb.Node
	EdgePages   []*srvpb.EdgePage
	EdgeSets    []*srvpb.PagedEdgeSet
	Decorations []*srvpb.FileDecorations
	RefSets     []*srvpb.PagedCrossReferences
	RefPages    []*srvpb.PagedCrossReferences_Page
}

func (tbl *testTable) Construct(t *testing.T) xrefs.Service {
	p := make(testProtoTable)
	tickets := stringset.New()
	for _, n := range tbl.Nodes {
		tickets.Add(n.Ticket)
	}
	for _, es := range tbl.EdgeSets {
		tickets.Remove(es.Source.Ticket)
		testutil.FatalOnErrT(t, "Error writing edge set: %v", p.Put(ctx, EdgeSetKey(mustFix(t, es.Source.Ticket)), es))
	}
	// Fill in EdgeSets for zero-degree nodes
	for ticket := range tickets {
		es := &srvpb.PagedEdgeSet{
			Source: getNode(ticket),
		}
		testutil.FatalOnErrT(t, "Error writing edge set: %v", p.Put(ctx, EdgeSetKey(mustFix(t, es.Source.Ticket)), es))
	}
	for _, ep := range tbl.EdgePages {
		testutil.FatalOnErrT(t, "Error writing edge page: %v", p.Put(ctx, EdgePageKey(ep.PageKey), ep))
	}
	for _, d := range tbl.Decorations {
		testutil.FatalOnErrT(t, "Error writing file decorations: %v", p.Put(ctx, DecorationsKey(mustFix(t, d.File.Ticket)), d))
	}
	for _, cr := range tbl.RefSets {
		testutil.FatalOnErrT(t, "Error writing cross-references: %v", p.Put(ctx, CrossReferencesKey(mustFix(t, cr.SourceTicket)), cr))
	}
	for _, crp := range tbl.RefPages {
		testutil.FatalOnErrT(t, "Error writing cross-references: %v", p.Put(ctx, CrossReferencesPageKey(crp.PageKey), crp))
	}
	return NewCombinedTable(table.ProtoBatchParallel{p})
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

func (t testProtoTable) Buffered() table.BufferedProto { panic("UNIMPLEMENTED") }

func (t testProtoTable) Close(_ context.Context) error { return nil }
