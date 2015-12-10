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

	srvpb "kythe.io/kythe/proto/serving_proto"
)

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

func getNodes(tickets ...string) []*srvpb.Node {
	ns := make([]*srvpb.Node, len(tickets))
	for i, t := range tickets {
		ns[i] = getNode(t)
	}
	return ns
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
		grp       *srvpb.EdgeSet_Group
		edgeSet   *srvpb.PagedEdgeSet
		edgePages []*srvpb.EdgePage
	}{{
		src: getNode("someSource"),
	}, {
		grp: &srvpb.EdgeSet_Group{
			Kind:   "someEdgeKind",
			Target: getNodes("kythe:#aTarget"),
		},
	}, {
		// flush
		edgeSet: &srvpb.PagedEdgeSet{
			EdgeSet: &srvpb.EdgeSet{
				Source: getNode("someSource"),
				Group: []*srvpb.EdgeSet_Group{{
					Kind:   "someEdgeKind",
					Target: getNodes("kythe:#aTarget"),
				}},
			},
			TotalEdges: 1,
		},
	}, {
		src: getNode("someOtherSource"),
	}, {
		grp: &srvpb.EdgeSet_Group{
			Kind: "someEdgeKind",
			Target: getNodes(
				"kythe:#aTarget",
				"kythe:#anotherTarget",
			),
		},
	}, {
		grp: &srvpb.EdgeSet_Group{
			Kind: "someEdgeKind",
			Target: getNodes(
				"kythe:#onceMoreWithFeeling",
			),
		},
	}, {
		src: getNode("aThirdSource"),

		// forced flush due to new source
		edgeSet: &srvpb.PagedEdgeSet{
			EdgeSet: &srvpb.EdgeSet{
				Source: getNode("someOtherSource"),
				Group: []*srvpb.EdgeSet_Group{{
					Kind: "someEdgeKind",
					Target: getNodes(
						"kythe:#aTarget",
						"kythe:#anotherTarget",
						"kythe:#onceMoreWithFeeling",
					),
				}},
			},
			TotalEdges: 3, // fits exactly MaxEdgePageSize
		},
	}, {
		grp: &srvpb.EdgeSet_Group{
			Kind:   "edgeKind123",
			Target: getNodes("kythe:#aTarget"),
		},
	}, {
		grp: &srvpb.EdgeSet_Group{
			Kind: "edgeKind123",
			Target: getNodes(
				"kythe:#bTarget",
				"kythe:#anotherTarget",
				"kythe:#threeTarget",
				"kythe:#fourTarget",
			),
		},

		edgePages: []*srvpb.EdgePage{{
			SourceTicket: "aThirdSource",
			PageKey:      "aThirdSource.0000000000",
			EdgesGroup: &srvpb.EdgeSet_Group{
				Kind: "edgeKind123",
				Target: getNodes(
					"kythe:#aTarget",
					"kythe:#bTarget",
					"kythe:#anotherTarget",
				),
			},
		}},
	}, {
		grp: &srvpb.EdgeSet_Group{
			Kind: "edgeKind123",
			Target: getNodes(
				"kythe:#five", "kythe:#six", "kythe:#seven", "kythe:#eight", "kythe:#nine",
			),
		},

		edgePages: []*srvpb.EdgePage{{
			SourceTicket: "aThirdSource",
			PageKey:      "aThirdSource.0000000001",
			EdgesGroup: &srvpb.EdgeSet_Group{
				Kind: "edgeKind123",
				Target: getNodes(
					"kythe:#threeTarget", "kythe:#fourTarget", "kythe:#five",
				),
			},
		}, {
			SourceTicket: "aThirdSource",
			PageKey:      "aThirdSource.0000000002",
			EdgesGroup: &srvpb.EdgeSet_Group{
				Kind: "edgeKind123",
				Target: getNodes(
					"kythe:#six", "kythe:#seven", "kythe:#eight",
				),
			},
		}},
	}, {
		grp: &srvpb.EdgeSet_Group{
			Kind: "edgeKind123",
			Target: getNodes(
				"kythe:#ten", "kythe:#eleven",
			),
		},
	}, {
		grp: &srvpb.EdgeSet_Group{
			Kind: "edgeKindFinal",
			Target: getNodes(
				"kythe:#ten",
			),
		},

		edgePages: []*srvpb.EdgePage{{
			SourceTicket: "aThirdSource",
			PageKey:      "aThirdSource.0000000003",
			EdgesGroup: &srvpb.EdgeSet_Group{
				Kind: "edgeKind123",
				Target: getNodes(
					"kythe:#nine", "kythe:#ten", "kythe:#eleven",
				),
			},
		}},
	}, {
		grp: &srvpb.EdgeSet_Group{
			Kind: "edgeKindFinal",
			Target: getNodes(
				"kythe:#two", "kythe:#three",
			),
		},
	}, {
		// flush

		edgeSet: &srvpb.PagedEdgeSet{
			TotalEdges: 15,
			EdgeSet: &srvpb.EdgeSet{
				Source: getNode("aThirdSource"),
				Group: []*srvpb.EdgeSet_Group{{
					Kind: "edgeKindFinal",
					Target: getNodes(
						"kythe:#ten", "kythe:#two", "kythe:#three",
					),
				}},
			},
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
