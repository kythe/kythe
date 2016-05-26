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

package paths

import (
	"fmt"
	"io"
	"testing"

	"kythe.io/kythe/go/test/testutil"
	"kythe.io/kythe/go/util/reduce"
	"kythe.io/kythe/go/util/schema"

	"golang.org/x/net/context"

	cpb "kythe.io/kythe/proto/common_proto"
	ipb "kythe.io/kythe/proto/internal_proto"
	srvpb "kythe.io/kythe/proto/serving_proto"
)

var ctx = context.Background()

func testInput(ps []*ipb.Path) reduce.SplitInput {
	sorter, err := reduce.KeyValueSorter()
	if err != nil {
		panic(err)
	}

	for _, p := range ps {
		kv, err := KeyValue("", p)
		if err != nil {
			panic(err)
		}
		if err := sorter.Add(kv); err != nil {
			panic(err)
		}
	}

	si, err := reduce.SplitSortedKeyValues(sorter)
	if err != nil {
		panic(err)
	}
	return si
}

func TestReducer_EmptySplit(t *testing.T) {
	expectedPivot := testNode("blah", "test", "subkind", "input")

	// Input for a single node (pivot header only)
	in := testInput([]*ipb.Path{{
		Pivot: expectedPivot,
	}})

	var found *ipb.Path_Node
	out, err := reduce.Sort(ctx, in, &Reducer{
		Func: func(ctx context.Context, pivot *ipb.Path_Node, rio ReducerIO) error {
			sortKey, p, err := rio.Next()
			if err != io.EOF {
				t.Errorf("Unexpected return values: %q %v %v", sortKey, p, err)
			}
			if found != nil {
				t.Errorf("Already found pivot: %v", found)
			}
			found = pivot
			return nil
		},
	})
	if err != nil {
		t.Fatal(err)
	}

	if err := testutil.DeepEqual(expectedPivot, found); err != nil {
		t.Error(err)
	}

	if s, err := out.NextSplit(); err != io.EOF {
		t.Fatalf("Unexpected output/err: %v / %v", s, err)
	}
}

func TestReducer_Input(t *testing.T) {
	in := testInput([]*ipb.Path{{
		Pivot: testNode("blah", "UNKNOWN"), // will be replaced by expectedPivot (below)
		Edges: testEdges("edgeKind", "target"),
	}, {
		Pivot: testNode("blah", "UNKNOWN2"), // will also be replaced
		Edges: testEdges("e1", "t1", "e2", "t2"),
	}, {
		// pivot-only edge will be sorted before anything else
		Pivot: testNode("blah", "test", "subkind", "input"),
	}, {
		Pivot: testNode("blah", "still UNKNOWN"), // replaced
		Edges: testEdges("e0", "t3"),
	}})

	var found []*ipb.Path
	empty, err := reduce.Sort(ctx, in, &Reducer{
		Func: func(ctx context.Context, pivot *ipb.Path_Node, rio ReducerIO) error {
			if len(found) > 0 {
				t.Fatalf("Already read paths: %v", found)
			}
			for {
				sortKey, p, err := rio.Next()
				if err == io.EOF {
					return nil
				} else if err != nil {
					t.Fatal(err)
				} else if sortKey != "" {
					t.Errorf("Unexpected sort key: %q", sortKey)
				} else if err := testutil.DeepEqual(pivot, p.Pivot); err != nil {
					t.Errorf("mismatched pivot: %v", err)
				}

				found = append(found, p)
			}
		},
	})
	if err != nil {
		t.Fatal(err)
	}

	expectedPivot := testNode("blah", "test", "subkind", "input")
	// Ordered the same as input
	expectedPaths := []*ipb.Path{{
		Pivot: expectedPivot,
		Edges: testEdges("edgeKind", "target"),
	}, {
		Pivot: expectedPivot,
		Edges: testEdges("e1", "t1", "e2", "t2"),
	}, {
		Pivot: expectedPivot,
		Edges: testEdges("e0", "t3"),
	}}

	if err := testutil.DeepEqual(expectedPaths, found); err != nil {
		t.Error(err)
	}

	if s, err := empty.NextSplit(); err != io.EOF {
		t.Fatalf("Unexpected output/err: %v / %v", s, err)
	}
}

func TestReducer_Output(t *testing.T) {
	in := testInput([]*ipb.Path{{
		Pivot: testNode("blah", "UNKNOWN"),
		Edges: testEdges("edgeKind", "target"),
	}, {
		Pivot: testNode("blah", "UNKNOWN2"),
		Edges: testEdges("e1", "t1", "e2", "t2"),
	}, {
		Pivot: testNode("blah", "test", "subkind", "input"),
	}, {
		Pivot: testNode("blah", "still UNKNOWN"),
		Edges: testEdges("e0", "t3"),
	}})

	out, err := reduce.Sort(ctx, in, &Reducer{
		OutputSort: func(p *ipb.Path) string {
			if len(p.Edges) == 0 {
				return ""
			}
			// Order by first edge's kind
			return p.Edges[0].Kind
		},
		Func: func(ctx context.Context, pivot *ipb.Path_Node, rio ReducerIO) error {
			// Pass-through the pivot node
			if err := rio.Emit(ctx, &ipb.Path{Pivot: pivot}); err != nil {
				t.Fatal(err)
			}
			for {
				_, p, err := rio.Next()
				if err == io.EOF {
					return nil
				} else if err != nil {
					t.Fatal(err)
				}
				// Pass-through each Path
				if err := rio.Emit(ctx, p); err != nil {
					t.Fatal(err)
				}
			}
		},
	})
	if err != nil {
		t.Fatal(err)
	}

	var found []*ipb.Path
	empty, err := reduce.Sort(ctx, out, &Reducer{
		Func: func(ctx context.Context, pivot *ipb.Path_Node, rio ReducerIO) error {
			if len(found) > 0 {
				t.Fatalf("Already read paths: %v", found)
			}
			for {
				_, p, err := rio.Next()
				if err == io.EOF {
					return nil
				} else if err != nil {
					t.Fatal(err)
				}
				found = append(found, p)
			}
		},
	})

	expectedPivot := testNode("blah", "test", "subkind", "input")

	// Ordered by first edge's kind
	expectedPaths := []*ipb.Path{{
		Pivot: expectedPivot,
		Edges: testEdges("e0", "t3"),
	}, {
		Pivot: expectedPivot,
		Edges: testEdges("e1", "t1", "e2", "t2"),
	}, {
		Pivot: expectedPivot,
		Edges: testEdges("edgeKind", "target"),
	}}

	if err := testutil.DeepEqual(expectedPaths, found); err != nil {
		t.Error(err)
	}

	if s, err := empty.NextSplit(); err != io.EOF {
		t.Fatalf("Unexpected output/err: %v / %v", s, err)
	}
}

func BenchmarkReducer_Input(b *testing.B) {
	ps := []*ipb.Path{{
		Pivot: testNode("blah", "test", "subkind", "input"),
	}}

	for i := 0; i < b.N; i++ {
		ps = append(ps, &ipb.Path{
			Pivot: testNode("blah", "UNKNOWN"),
			Edges: testEdges(fmt.Sprintf("edgeKind.%d", i), fmt.Sprintf("target.%d", i)),
		})
	}

	in := testInput(ps)

	var readSplit bool
	b.ResetTimer()
	empty, err := reduce.Sort(ctx, in, &Reducer{
		OutputSort: func(p *ipb.Path) string {
			if len(p.Edges) == 0 {
				return ""
			}
			// Order by first edge's kind
			return p.Edges[0].Kind
		},
		Func: func(ctx context.Context, pivot *ipb.Path_Node, rio ReducerIO) error {
			if readSplit {
				b.Fatal("Multiple splits")
			}
			readSplit = true
			for {
				if _, _, err := rio.Next(); err == io.EOF {
					return nil
				} else if err != nil {
					return err
				}
			}
		},
	})
	if err != nil {
		b.Fatal(err)
	}

	if s, err := empty.NextSplit(); err != io.EOF {
		b.Fatalf("Unexpected output/err: %v / %v", s, err)
	}
}

func BenchmarkReducer_Output(b *testing.B) {
	in := testInput([]*ipb.Path{{
		Pivot: testNode("blah", "test", "subkind", "input"),
	}})

	var readSplit bool
	out, err := reduce.Sort(ctx, in, &Reducer{
		OutputSort: func(p *ipb.Path) string {
			if len(p.Edges) == 0 {
				return ""
			}
			// Order by first edge's kind
			return p.Edges[0].Kind
		},
		Func: func(ctx context.Context, pivot *ipb.Path_Node, rio ReducerIO) error {
			if readSplit {
				b.Fatal("Multiple splits")
			}
			readSplit = true
			// Pass-through the pivot node
			if err := rio.Emit(ctx, &ipb.Path{Pivot: pivot}); err != nil {
				b.Fatal(err)
			}
			if sortKey, p, err := rio.Next(); err != io.EOF {
				b.Fatalf("Unexpected input/err: %q %v %v", sortKey, p, err)
			}

			for i := 0; i < b.N; i++ {
				if err := rio.Emit(ctx, &ipb.Path{
					Pivot: pivot,
					Edges: testEdges(fmt.Sprintf("edgeKind.%d", i), fmt.Sprintf("target.%d", i)),
				}); err != nil {
					b.Fatal(err)
				}
			}
			return nil
		},
	})
	if err != nil {
		b.Fatal(err)
	}

	readSplit = false
	empty, err := reduce.Sort(ctx, out, &Reducer{
		Func: func(ctx context.Context, pivot *ipb.Path_Node, rio ReducerIO) error {
			if readSplit {
				b.Fatal("Multiple splits")
			}
			readSplit = true
			for {
				_, _, err := rio.Next()
				if err == io.EOF {
					return nil
				} else if err != nil {
					b.Fatal(err)
				}
			}
		},
	})

	if s, err := empty.NextSplit(); err != io.EOF {
		b.Fatalf("Unexpected output/err: %v / %v", s, err)
	}
}

func testNode(ticket, kind string, facts ...string) *ipb.Path_Node {
	if len(facts)%2 != 0 {
		panic("uneven number of fact-values")
	}

	n := &ipb.Path_Node{
		Ticket:   ticket,
		NodeKind: kind,
		Original: &srvpb.Node{
			Ticket: ticket,
			Fact: []*cpb.Fact{{
				Name:  schema.NodeKindFact,
				Value: []byte(kind),
			}},
		},
	}

	for i := 0; i < len(facts); i += 2 {
		n.Original.Fact = append(n.Original.Fact, &cpb.Fact{
			Name:  facts[i],
			Value: []byte(facts[i+1]),
		})
	}

	return n
}

func testEdges(kindTargets ...string) []*ipb.Path_Edge {
	if len(kindTargets)%2 != 0 {
		panic("uneven number of kind-targets")
	}
	var es []*ipb.Path_Edge
	for i := 0; i < len(kindTargets); i += 2 {
		es = append(es, &ipb.Path_Edge{
			Kind:   kindTargets[i],
			Target: testNode(kindTargets[i+1], "UNKNOWN"),
		})
	}
	return es
}
