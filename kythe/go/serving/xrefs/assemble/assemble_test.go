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

func TestEdgeSetBuilder(t *testing.T) {
	tests := []struct {
		src       string
		grp       *srvpb.EdgeSet_Group
		edgeSet   *srvpb.PagedEdgeSet
		edgePages []*srvpb.EdgePage
	}{{
		src: "someSource",
		grp: &srvpb.EdgeSet_Group{
			Kind:         "someEdgeKind",
			TargetTicket: []string{"kythe:#aTarget"},
		},
	}, {
		// flush
		edgeSet: &srvpb.PagedEdgeSet{
			EdgeSet: &srvpb.EdgeSet{
				SourceTicket: "someSource",
				Group: []*srvpb.EdgeSet_Group{{
					Kind:         "someEdgeKind",
					TargetTicket: []string{"kythe:#aTarget"},
				}},
			},
			TotalEdges: 1,
		},
	}, {
		src: "someOtherSource",
		grp: &srvpb.EdgeSet_Group{
			Kind: "someEdgeKind",
			TargetTicket: []string{
				"kythe:#aTarget",
				"kythe:#anotherTarget",
			},
		},
	}, {
		src: "someOtherSource",
		grp: &srvpb.EdgeSet_Group{
			Kind: "someEdgeKind",
			TargetTicket: []string{
				"kythe:#onceMoreWithFeeling",
			},
		},
	}, {
		src: "aThirdSource",
		grp: &srvpb.EdgeSet_Group{
			Kind:         "edgeKind123",
			TargetTicket: []string{"kythe:#aTarget"},
		},

		// forced flush due to new source
		edgeSet: &srvpb.PagedEdgeSet{
			EdgeSet: &srvpb.EdgeSet{
				SourceTicket: "someOtherSource",
				Group: []*srvpb.EdgeSet_Group{{
					Kind: "someEdgeKind",
					TargetTicket: []string{
						"kythe:#aTarget",
						"kythe:#anotherTarget",
						"kythe:#onceMoreWithFeeling",
					},
				}},
			},
			TotalEdges: 3, // fits exactly MaxEdgePageSize
		},
	}, {
		src: "aThirdSource",
		grp: &srvpb.EdgeSet_Group{
			Kind: "edgeKind123",
			TargetTicket: []string{
				"kythe:#bTarget",
				"kythe:#anotherTarget",
				"kythe:#threeTarget",
				"kythe:#fourTarget",
			},
		},

		edgePages: []*srvpb.EdgePage{{
			SourceTicket: "aThirdSource",
			PageKey:      "aThirdSource.0000000000",
			EdgesGroup: &srvpb.EdgeSet_Group{
				Kind: "edgeKind123",
				TargetTicket: []string{
					"kythe:#aTarget",
					"kythe:#bTarget",
					"kythe:#anotherTarget",
				},
			},
		}},
	}, {
		src: "aThirdSource",
		grp: &srvpb.EdgeSet_Group{
			Kind: "edgeKind123",
			TargetTicket: []string{
				"kythe:#five", "kythe:#six", "kythe:#seven", "kythe:#eight", "kythe:#nine",
			},
		},

		edgePages: []*srvpb.EdgePage{{
			SourceTicket: "aThirdSource",
			PageKey:      "aThirdSource.0000000001",
			EdgesGroup: &srvpb.EdgeSet_Group{
				Kind: "edgeKind123",
				TargetTicket: []string{
					"kythe:#threeTarget", "kythe:#fourTarget", "kythe:#five",
				},
			},
		}, {
			SourceTicket: "aThirdSource",
			PageKey:      "aThirdSource.0000000002",
			EdgesGroup: &srvpb.EdgeSet_Group{
				Kind: "edgeKind123",
				TargetTicket: []string{
					"kythe:#six", "kythe:#seven", "kythe:#eight",
				},
			},
		}},
	}, {
		src: "aThirdSource",
		grp: &srvpb.EdgeSet_Group{
			Kind: "edgeKind123",
			TargetTicket: []string{
				"kythe:#ten", "kythe:#eleven",
			},
		},
	}, {
		src: "aThirdSource",
		grp: &srvpb.EdgeSet_Group{
			Kind: "edgeKindFinal",
			TargetTicket: []string{
				"kythe:#ten",
			},
		},

		edgePages: []*srvpb.EdgePage{{
			SourceTicket: "aThirdSource",
			PageKey:      "aThirdSource.0000000003",
			EdgesGroup: &srvpb.EdgeSet_Group{
				Kind: "edgeKind123",
				TargetTicket: []string{
					"kythe:#nine", "kythe:#ten", "kythe:#eleven",
				},
			},
		}},
	}, {
		src: "aThirdSource",
		grp: &srvpb.EdgeSet_Group{
			Kind: "edgeKindFinal",
			TargetTicket: []string{
				"kythe:#two", "kythe:#three",
			},
		},
	}, {
		src: "aThirdSource",
		// flush

		edgeSet: &srvpb.PagedEdgeSet{
			TotalEdges: 15,
			EdgeSet: &srvpb.EdgeSet{
				SourceTicket: "aThirdSource",
				Group: []*srvpb.EdgeSet_Group{{
					Kind: "edgeKindFinal",
					TargetTicket: []string{
						"kythe:#ten", "kythe:#two", "kythe:#three",
					},
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
		if test.grp == nil {
			testutil.FatalOnErrT(t, "Failure to Flush: %v",
				tESB.Flush(ctx))
		} else {
			testutil.FatalOnErrT(t, "Failure to AddGroup: %v",
				tESB.AddGroup(ctx, test.src, test.grp))
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
			} else if found := tESB.EdgePages[edgePages]; !reflect.DeepEqual(test.edgePages[i], found) {
				t.Fatalf("Expected EdgePage: %v; found: %v", test.edgePages[i], found)
			}
			edgePages++
		}
		if edgePages != len(tESB.EdgePages) {
			t.Fatalf("Unexpected EdgePage(s): %v", tESB.EdgePages[edgePages:])
		}
	}
}
