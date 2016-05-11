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

package assemble

import (
	"reflect"
	"testing"

	"kythe.io/kythe/go/test/testutil"

	"golang.org/x/net/context"

	ipb "kythe.io/kythe/proto/internal_proto"
	srvpb "kythe.io/kythe/proto/serving_proto"
	spb "kythe.io/kythe/proto/storage_proto"
)

func fact(name, value string) *spb.Entry {
	return &spb.Entry{
		FactName:  name,
		FactValue: []byte(value),
	}
}

func edge(kind, targetSig string) *spb.Entry {
	return &spb.Entry{
		EdgeKind: kind,
		Target: &spb.VName{
			Signature: targetSig,
		},
		FactName: "/",
	}
}

func TestAppendEntry(t *testing.T) {
	tests := []struct {
		entries  []*spb.Entry
		expected *ipb.Source
	}{{
		entries: []*spb.Entry{fact("fact", "value")},
		expected: &ipb.Source{
			Facts: map[string][]byte{"fact": []byte("value")},
		},
	}, {
		entries: []*spb.Entry{edge("kind", "target")},
		expected: &ipb.Source{
			EdgeGroups: map[string]*ipb.Source_EdgeGroup{
				"kind": {
					Edges: []*ipb.Source_Edge{{Ticket: "kythe:#target"}},
				},
			},
		},
	}, {
		entries: []*spb.Entry{
			fact("kind", "first"),
			fact("kind", "second"),
			edge("edgeKind", "firstTarget"),
			edge("edgeKind", "secondTarget"),
			fact("blah", "blah"),
		},
		expected: &ipb.Source{
			Facts: map[string][]byte{
				"kind": []byte("second"),
				"blah": []byte("blah"),
			},
			EdgeGroups: map[string]*ipb.Source_EdgeGroup{
				"edgeKind": &ipb.Source_EdgeGroup{
					Edges: []*ipb.Source_Edge{{
						Ticket: "kythe:#firstTarget",
					}, {
						Ticket: "kythe:#secondTarget",
					}},
				},
			},
		},
	}}

	for i, test := range tests {
		src := &ipb.Source{
			Facts:      make(map[string][]byte),
			EdgeGroups: make(map[string]*ipb.Source_EdgeGroup),
		}

		for _, e := range test.entries {
			AppendEntry(src, e)
		}

		if err := testutil.DeepEqual(test.expected, src); err != nil {
			t.Errorf("tests[%d] error: %v", i, err)
		}
	}
}

var ctx = context.Background()

type testESB struct {
	*EdgeSetBuilder

	PagedEdgeSets []*srvpb.PagedEdgeSet
	EdgePages     []*srvpb.EdgePage
}

func newTestESB(esb *EdgeSetBuilder) *testESB {
	if esb == nil {
		esb = new(EdgeSetBuilder)
	}

	t := &testESB{
		EdgeSetBuilder: esb,
	}
	t.Output = func(_ context.Context, pes *srvpb.PagedEdgeSet) error {
		t.PagedEdgeSets = append(t.PagedEdgeSets, pes)
		return nil
	}
	t.OutputPage = func(_ context.Context, pes *srvpb.EdgePage) error {
		t.EdgePages = append(t.EdgePages, pes)
		return nil
	}
	return t
}

func makeNodes() map[string]*srvpb.Node {
	m := make(map[string]*srvpb.Node)
	// TODO(schroederc): fill
	return m
}

// TODO(schroederc): add some facts for each node
var nodes = makeNodes()

func getEdgeTargets(tickets ...string) []*srvpb.EdgeGroup_Edge {
	es := make([]*srvpb.EdgeGroup_Edge, len(tickets))
	for i, t := range tickets {
		es[i] = &srvpb.EdgeGroup_Edge{
			Target: getNode(t),
		}
	}
	return es
}

func getNode(t string) *srvpb.Node {
	n, ok := nodes[t]
	if !ok {
		n = &srvpb.Node{Ticket: t}
	}
	return n
}

func TestEdgeSetBuilder(t *testing.T) {
	tests := []struct {
		src       *srvpb.Node
		grp       *srvpb.EdgeGroup
		edgeSet   *srvpb.PagedEdgeSet
		edgePages []*srvpb.EdgePage
	}{{
		src: getNode("someSource"),
	}, {
		grp: &srvpb.EdgeGroup{
			Kind: "someEdgeKind",
			Edge: getEdgeTargets("kythe:#aTarget"),
		},
	}, {
		// flush
		edgeSet: &srvpb.PagedEdgeSet{
			Source: getNode("someSource"),
			Group: []*srvpb.EdgeGroup{{
				Kind: "someEdgeKind",
				Edge: getEdgeTargets("kythe:#aTarget"),
			}},
			TotalEdges: 1,
		},
	}, {
		src: getNode("someOtherSource"),
	}, {
		grp: &srvpb.EdgeGroup{
			Kind: "someEdgeKind",
			Edge: getEdgeTargets(
				"kythe:#aTarget",
				"kythe:#anotherTarget",
			),
		},
	}, {
		grp: &srvpb.EdgeGroup{
			Kind: "someEdgeKind",
			Edge: getEdgeTargets(
				"kythe:#onceMoreWithFeeling",
			),
		},
	}, {
		src: getNode("aThirdSource"),

		// forced flush due to new source
		edgeSet: &srvpb.PagedEdgeSet{
			Source: getNode("someOtherSource"),
			Group: []*srvpb.EdgeGroup{{
				Kind: "someEdgeKind",
				Edge: getEdgeTargets(
					"kythe:#aTarget",
					"kythe:#anotherTarget",
					"kythe:#onceMoreWithFeeling",
				),
			}},

			TotalEdges: 3, // fits exactly MaxEdgePageSize
		},
	}, {
		grp: &srvpb.EdgeGroup{
			Kind: "edgeKind123",
			Edge: getEdgeTargets("kythe:#aTarget"),
		},
	}, {
		grp: &srvpb.EdgeGroup{
			Kind: "edgeKind123",
			Edge: getEdgeTargets(
				"kythe:#bTarget",
				"kythe:#anotherTarget",
				"kythe:#threeTarget",
				"kythe:#fourTarget",
			),
		},

		edgePages: []*srvpb.EdgePage{{
			SourceTicket: "aThirdSource",
			PageKey:      "aThirdSource.0000000000",
			EdgesGroup: &srvpb.EdgeGroup{
				Kind: "edgeKind123",
				Edge: getEdgeTargets(
					"kythe:#aTarget",
					"kythe:#bTarget",
					"kythe:#anotherTarget",
				),
			},
		}},
	}, {
		grp: &srvpb.EdgeGroup{
			Kind: "edgeKind123",
			Edge: getEdgeTargets(
				"kythe:#five", "kythe:#six", "kythe:#seven", "kythe:#eight", "kythe:#nine",
			),
		},

		edgePages: []*srvpb.EdgePage{{
			SourceTicket: "aThirdSource",
			PageKey:      "aThirdSource.0000000001",
			EdgesGroup: &srvpb.EdgeGroup{
				Kind: "edgeKind123",
				Edge: getEdgeTargets(
					"kythe:#threeTarget", "kythe:#fourTarget", "kythe:#five",
				),
			},
		}, {
			SourceTicket: "aThirdSource",
			PageKey:      "aThirdSource.0000000002",
			EdgesGroup: &srvpb.EdgeGroup{
				Kind: "edgeKind123",
				Edge: getEdgeTargets(
					"kythe:#six", "kythe:#seven", "kythe:#eight",
				),
			},
		}},
	}, {
		grp: &srvpb.EdgeGroup{
			Kind: "edgeKind123",
			Edge: getEdgeTargets(
				"kythe:#ten", "kythe:#eleven",
			),
		},
	}, {
		grp: &srvpb.EdgeGroup{
			Kind: "edgeKindFinal",
			Edge: getEdgeTargets(
				"kythe:#ten",
			),
		},

		edgePages: []*srvpb.EdgePage{{
			SourceTicket: "aThirdSource",
			PageKey:      "aThirdSource.0000000003",
			EdgesGroup: &srvpb.EdgeGroup{
				Kind: "edgeKind123",
				Edge: getEdgeTargets(
					"kythe:#nine", "kythe:#ten", "kythe:#eleven",
				),
			},
		}},
	}, {
		grp: &srvpb.EdgeGroup{
			Kind: "edgeKindFinal",
			Edge: getEdgeTargets(
				"kythe:#two", "kythe:#three",
			),
		},
	}, {
		// flush

		edgeSet: &srvpb.PagedEdgeSet{
			TotalEdges: 15,

			Source: getNode("aThirdSource"),
			Group: []*srvpb.EdgeGroup{{
				Kind: "edgeKindFinal",
				Edge: getEdgeTargets(
					"kythe:#ten", "kythe:#two", "kythe:#three",
				),
			}},

			PageIndex: []*srvpb.PageIndex{{
				PageKey:   "aThirdSource.0000000000",
				EdgeKind:  "edgeKind123",
				EdgeCount: 3,
			}, {
				PageKey:   "aThirdSource.0000000001",
				EdgeKind:  "edgeKind123",
				EdgeCount: 3,
			}, {
				PageKey:   "aThirdSource.0000000002",
				EdgeKind:  "edgeKind123",
				EdgeCount: 3,
			}, {
				PageKey:   "aThirdSource.0000000003",
				EdgeKind:  "edgeKind123",
				EdgeCount: 3,
			}},
		},
	}}

	tESB := newTestESB(&EdgeSetBuilder{
		MaxEdgePageSize: 3,
	})
	var edgeSets, edgePages int
	for _, test := range tests {
		if test.src != nil {
			testutil.FatalOnErrT(t, "Failure to StartEdgeSet: %v",
				tESB.StartEdgeSet(ctx, test.src))
		} else if test.grp != nil {
			testutil.FatalOnErrT(t, "Failure to AddGroup: %v",
				tESB.AddGroup(ctx, test.grp))
		} else {
			testutil.FatalOnErrT(t, "Failure to Flush: %v",
				tESB.Flush(ctx))
		}

		if test.edgeSet != nil {
			// Expected a new PagedEdgeSet
			if edgeSets+1 != len(tESB.PagedEdgeSets) {
				t.Fatalf("Missing expected PagedEdgeSet: %v", test.edgeSet)
			} else if found := tESB.PagedEdgeSets[len(tESB.PagedEdgeSets)-1]; !reflect.DeepEqual(test.edgeSet, found) {
				t.Errorf("Expected PagedEdgeSet: %v; found: %v", test.edgeSet, found)
			}
			edgeSets++
		} else if edgeSets != len(tESB.PagedEdgeSets) {
			t.Fatalf("Unexpected PagedEdgeSet: %v", tESB.PagedEdgeSets[len(tESB.PagedEdgeSets)-1])
		}

		// Expected new EdgePage(s)
		for i := 0; i < len(test.edgePages); i++ {
			if edgePages >= len(tESB.EdgePages) {
				t.Fatalf("Missing expected EdgePages: %v", test.edgePages[i])
			} else if err := testutil.DeepEqual(test.edgePages[i], tESB.EdgePages[edgePages]); err != nil {
				t.Error(err)
			}
			edgePages++
		}
		if edgePages != len(tESB.EdgePages) {
			t.Fatalf("Unexpected EdgePage(s): %v", tESB.EdgePages[edgePages:])
		}
	}
}
