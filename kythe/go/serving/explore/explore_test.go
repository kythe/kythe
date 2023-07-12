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

package explore

import (
	"context"
	"fmt"
	"testing"

	"kythe.io/kythe/go/storage/table"
	"kythe.io/kythe/go/test/testutil"

	"bitbucket.org/creachadair/stringset"
	"google.golang.org/protobuf/proto"

	epb "kythe.io/kythe/proto/explore_go_proto"
	srvpb "kythe.io/kythe/proto/serving_go_proto"
)

const (
	p1       = "kythe:#parent1"
	p2       = "kythe:#parent2"
	p1c1     = "kythe:#p1child1"
	p1c2     = "kythe:#p1child2"
	badType  = "kythe:#baddatatype"
	dontcare = "kythe:#dontcare"
	f1       = "kythe:#function1"
	f2       = "kythe:#function2"
	f1r1     = "kythe:#f1caller1"
	fr       = "kythe:#fcaller"
	f2r1     = "kythe:#f2caller1"
	f3       = "kythe:#function3_recursive"
	dne      = "kythe:#does_not_exist"
)

var (
	ctx              = context.Background()
	parentToChildren = &protoTable{
		p1: &srvpb.Relatives{
			Tickets: []string{p1c1, p1c2},
			Type:    srvpb.Relatives_CHILDREN,
		},
		badType: &srvpb.Relatives{
			Tickets: []string{dontcare},
			Type:    srvpb.Relatives_PARENTS,
		},
	}

	childToParents = &protoTable{
		p1c1: &srvpb.Relatives{
			Tickets: []string{p1},
			Type:    srvpb.Relatives_PARENTS,
		},
		p1c2: &srvpb.Relatives{
			Tickets: []string{p2},
			Type:    srvpb.Relatives_PARENTS,
		},
		badType: &srvpb.Relatives{
			Tickets: []string{dontcare},
			Type:    srvpb.Relatives_CHILDREN,
		},
	}

	functionToCallers = &protoTable{
		f1: &srvpb.Callgraph{
			Tickets: []string{f1r1, fr},
			Type:    srvpb.Callgraph_CALLER,
		},
		f2: &srvpb.Callgraph{
			Tickets: []string{f2r1, fr},
			Type:    srvpb.Callgraph_CALLER,
		},
		f3: &srvpb.Callgraph{
			Tickets: []string{f1, fr, f3},
			Type:    srvpb.Callgraph_CALLER,
		},
		fr: &srvpb.Callgraph{
			Tickets: []string{f2},
			Type:    srvpb.Callgraph_CALLER,
		},
		badType: &srvpb.Callgraph{
			Tickets: []string{dontcare},
			Type:    srvpb.Callgraph_CALLEE,
		},
	}

	functionToCallees = &protoTable{
		f1: &srvpb.Callgraph{
			Tickets: []string{f3},
			Type:    srvpb.Callgraph_CALLEE,
		},
		f2: &srvpb.Callgraph{
			Tickets: []string{fr},
			Type:    srvpb.Callgraph_CALLEE,
		},
		f3: &srvpb.Callgraph{
			Tickets: []string{f3},
			Type:    srvpb.Callgraph_CALLEE,
		},
		fr: &srvpb.Callgraph{
			Tickets: []string{f1, f2, f3},
			Type:    srvpb.Callgraph_CALLEE,
		},
		f1r1: &srvpb.Callgraph{
			Tickets: []string{f1},
			Type:    srvpb.Callgraph_CALLEE,
		},
		f2r1: &srvpb.Callgraph{
			Tickets: []string{f2},
			Type:    srvpb.Callgraph_CALLEE,
		},
		badType: &srvpb.Callgraph{
			Tickets: []string{dontcare},
			Type:    srvpb.Callgraph_CALLER,
		},
	}
)

func TestChildren_badData(t *testing.T) {
	svc := construct(t)

	reply, err := svc.Children(ctx, &epb.ChildrenRequest{
		Tickets: []string{badType},
	})
	if err == nil {
		t.Errorf("Expected Children error for bad data, got: %v", reply)
	}
}

func TestChildren_noData(t *testing.T) {
	svc := construct(t)

	reply, err := svc.Children(ctx, &epb.ChildrenRequest{
		Tickets: []string{dne},
	})
	testutil.Fatalf(t, "Children error: %v", err)
	if len(reply.InputToChildren) != 0 {
		t.Errorf("Expected empty response for missing key, got: %v", reply)
	}
}

func TestChildren(t *testing.T) {
	svc := construct(t)
	request := &epb.ChildrenRequest{
		Tickets: []string{p1},
	}

	reply, err := svc.Children(ctx, request)
	testutil.Fatalf(t, "Children error: %v", err)

	expectedInputToChildren := map[string]*epb.Tickets{
		p1: {
			Tickets: []string{p1c1, p1c2},
		},
	}

	expectedInputKeys := stringset.FromKeys(expectedInputToChildren)
	actualInputKeys := stringset.FromKeys(reply.InputToChildren)
	if !expectedInputKeys.Equals(actualInputKeys) {
		t.Errorf("Expected results and actual results have different ticket key sets:\n"+
			"expected: %v\n actual: %v", expectedInputKeys, actualInputKeys)
	}

	for ticket, expectedChildren := range expectedInputToChildren {
		children := reply.InputToChildren[ticket]
		checkEquivalentLists(t, expectedChildren.Tickets, children.Tickets, "children")
	}
}

func TestParents_badData(t *testing.T) {
	svc := construct(t)

	reply, err := svc.Parents(ctx, &epb.ParentsRequest{
		Tickets: []string{badType},
	})
	if err == nil {
		t.Errorf("Expected Parents error for bad data, got: %v", reply)
	}
}

func TestParents_noData(t *testing.T) {
	svc := construct(t)

	reply, err := svc.Parents(ctx, &epb.ParentsRequest{
		Tickets: []string{dne},
	})
	testutil.Fatalf(t, "Parents error: %v", err)
	if len(reply.InputToParents) != 0 {
		t.Errorf("Expected empty response for missing key, got: %v", reply)
	}
}

func TestParents(t *testing.T) {
	svc := construct(t)
	request := &epb.ParentsRequest{
		Tickets: []string{p1c1},
	}

	reply, err := svc.Parents(ctx, request)
	testutil.Fatalf(t, "Parents error: %v", err)

	expectedInputToParents := map[string]*epb.Tickets{
		p1c1: {
			Tickets: []string{p1},
		},
	}

	expectedInputKeys := stringset.FromKeys(expectedInputToParents)
	actualInputKeys := stringset.FromKeys(reply.InputToParents)
	if !expectedInputKeys.Equals(actualInputKeys) {
		t.Errorf("Expected results and actual results have different ticket key sets:\n"+
			"expected: %v\n actual: %v", expectedInputKeys, actualInputKeys)
	}

	for ticket, expectedParents := range expectedInputToParents {
		parents := reply.InputToParents[ticket]
		checkEquivalentLists(t, expectedParents.Tickets, parents.Tickets,
			fmt.Sprintf("children of %s", ticket))
	}
}

func TestCallers_badData(t *testing.T) {
	svc := construct(t)

	reply, err := svc.Callers(ctx, &epb.CallersRequest{
		Tickets: []string{badType},
	})
	if err == nil {
		t.Errorf("Expected Callers error for bad data, got: %v", reply)
	}
}

func TestCallers_noData(t *testing.T) {
	svc := construct(t)

	reply, err := svc.Callers(ctx, &epb.CallersRequest{
		Tickets: []string{dne},
	})
	testutil.Fatalf(t, "CallersRequest error: %v", err)
	if len(reply.Graph.Nodes) != 0 {
		t.Errorf("Expected empty response for missing key, got: %v", reply)
	}
}

func TestCallers(t *testing.T) {
	svc := construct(t)
	request := &epb.CallersRequest{
		Tickets: []string{f1},
	}

	reply, err := svc.Callers(ctx, request)
	testutil.Fatalf(t, "Callers error: %v", err)

	expectedGraph := &epb.Graph{
		Nodes: map[string]*epb.GraphNode{
			f1: {
				Predecessors: []string{f1r1, fr},
			},
			f1r1: {
				Successors: []string{f1},
			},
			fr: {
				Successors: []string{f1},
			},
		},
	}

	checkEqualGraphs(t, expectedGraph, reply.Graph)
}

func TestCallers_multipleInputs(t *testing.T) {
	svc := construct(t)
	request := &epb.CallersRequest{
		Tickets: []string{f1, f2, f3},
	}

	reply, err := svc.Callers(ctx, request)
	testutil.Fatalf(t, "Callers error: %v", err)

	// The edge f2->fr is not in the reply because we're asking for neither
	// the callers of fr nor the callees of f2.
	expectedGraph := &epb.Graph{
		Nodes: map[string]*epb.GraphNode{
			f1: {
				Predecessors: []string{f1r1, fr},
				Successors:   []string{f3},
			},
			f2: {
				Predecessors: []string{f2r1, fr},
			},
			f3: {
				Predecessors: []string{f1, fr, f3},
				Successors:   []string{f3},
			},
			f1r1: {
				Successors: []string{f1},
			},
			f2r1: {
				Successors: []string{f2},
			},
			fr: {
				Successors: []string{f1, f2, f3},
			},
		},
	}

	checkEqualGraphs(t, expectedGraph, reply.Graph)
}

func TestCallees_badData(t *testing.T) {
	svc := construct(t)

	reply, err := svc.Callees(ctx, &epb.CalleesRequest{
		Tickets: []string{badType},
	})
	if err == nil {
		t.Errorf("Expected Callees error for bad data, got: %v", reply)
	}
}

func TestCallees_noData(t *testing.T) {
	svc := construct(t)

	reply, err := svc.Callees(ctx, &epb.CalleesRequest{
		Tickets: []string{dne},
	})
	testutil.Fatalf(t, "CalleesRequest error: %v", err)
	if len(reply.Graph.Nodes) != 0 {
		t.Errorf("Expected empty response for missing key, got: %v", reply)
	}
}

func TestCallees(t *testing.T) {
	svc := construct(t)
	request := &epb.CalleesRequest{
		Tickets: []string{fr},
	}

	reply, err := svc.Callees(ctx, request)
	testutil.Fatalf(t, "Callees error: %v", err)

	expectedGraph := &epb.Graph{
		Nodes: map[string]*epb.GraphNode{
			fr: {
				Successors: []string{f1, f2, f3},
			},
			f1: {
				Predecessors: []string{fr},
			},
			f2: {
				Predecessors: []string{fr},
			},
			f3: {
				Predecessors: []string{fr},
			},
		},
	}

	checkEqualGraphs(t, expectedGraph, reply.Graph)
}

func TestCallees_multipleInputs(t *testing.T) {
	svc := construct(t)
	request := &epb.CalleesRequest{
		Tickets: []string{f1, f2, f3},
	}

	reply, err := svc.Callees(ctx, request)
	testutil.Fatalf(t, "Callees error: %v", err)

	// The following edges are present in the stored callgraph
	// but intentionally not present in the response:
	// f1r1->f1, fr->f1; f2r1->f2, fr->f2; fr->f3
	// (because the request doesn't ask for the callers of f1, f2, f3).
	expectedGraph := &epb.Graph{
		Nodes: map[string]*epb.GraphNode{
			f1: {
				Successors: []string{f3},
			},
			f2: {
				Successors: []string{fr},
			},
			f3: {
				Predecessors: []string{f1, f3},
				Successors:   []string{f3},
			},
			fr: {
				Predecessors: []string{f2},
			},
		},
	}

	checkEqualGraphs(t, expectedGraph, reply.Graph)
}

func checkEqualGraphs(t *testing.T, expected, actual *epb.Graph) {
	if len(expected.Nodes) != len(actual.Nodes) {
		t.Errorf("Mismatch in graph node counts: expected: %d, actual: %d",
			len(expected.Nodes), len(actual.Nodes))
	}

	expectedNodeSet := stringset.FromKeys(expected.Nodes)
	actualNodeSet := stringset.FromKeys(actual.Nodes)
	if !expectedNodeSet.Equals(actualNodeSet) {
		t.Errorf("Unexpected mismatch in graph node sets:\n expected:\n%v\n actual:\n%v\n",
			expectedNodeSet, actualNodeSet)
	}

	for ticket, expectedNode := range expected.Nodes {
		node := actual.Nodes[ticket]
		checkEquivalentLists(t, expectedNode.Predecessors, node.Predecessors,
			fmt.Sprintf("predecessors for %s", ticket))
		checkEquivalentLists(t, expectedNode.Successors, node.Successors,
			fmt.Sprintf("successors for %s", ticket))
	}
}

// Checks whether the lists are equivalent.
func checkEquivalentLists(t *testing.T, expected, actual []string, tag string) {
	if len(expected) != len(actual) {
		t.Errorf("Mismatch in counts for %s; expected:\n%v actual:\n%v",
			tag, expected, actual)
	}

	if !stringset.FromKeys(expected).Equals(stringset.FromKeys(actual)) {
		t.Errorf("Mismatch for %s; expected:\n%v actual:\n%v",
			tag, expected, actual)
	}
}

type protoTable map[string]proto.Message

func (t protoTable) Lookup(_ context.Context, key []byte, msg proto.Message) error {
	m, ok := t[string(key)]
	if !ok {
		return table.ErrNoSuchKey
	}
	proto.Merge(msg, m)
	return nil
}

func (t protoTable) LookupValues(_ context.Context, key []byte, m proto.Message, f func(proto.Message) error) error {
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

func construct(t *testing.T) *Tables {
	return &Tables{
		ParentToChildren:  parentToChildren,
		ChildToParents:    childToParents,
		FunctionToCallers: functionToCallers,
		FunctionToCallees: functionToCallees,
	}
}
