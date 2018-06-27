package explore

import (
	"context"
	"reflect"
	"testing"

	"kythe.io/kythe/go/storage/table"
	"kythe.io/kythe/go/test/testutil"

	"github.com/golang/protobuf/proto"

	epb "kythe.io/kythe/proto/explore_go_proto"
	srvpb "kythe.io/kythe/proto/serving_go_proto"
)

var (
	ctx = context.Background()

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
	svc := Construct(t)

	reply, err := svc.Children(ctx, &epb.ChildrenRequest{
		Tickets: []string{badType},
	})
	if err == nil {
		t.Errorf("Expected Children error for bad data, got: %v", reply)
	}
}

func TestChildren_noData(t *testing.T) {
	svc := Construct(t)

	reply, err := svc.Children(ctx, &epb.ChildrenRequest{
		Tickets: []string{"key with no data"},
	})
	if err != nil {
		testutil.FatalOnErrT(t, "Children error: %v", err)
	}
	if len(reply.InputToChildren) != 0 {
		t.Errorf("Expected empty response for missing key, got: %v", reply)
	}
}

func TestChildren(t *testing.T) {
	svc := Construct(t)
	request := &epb.ChildrenRequest{
		Tickets: []string{p1},
	}

	reply, err := svc.Children(ctx, request)
	testutil.FatalOnErrT(t, "Children error: %v", err)

	expectedInputToChildren := map[string]*epb.Tickets{
		p1: {
			Tickets: []string{p1c1, p1c2},
		},
	}

	if !equalTicketsKeys(reply.InputToChildren, expectedInputToChildren) {
		t.Errorf("Expected results and actual results have different ticket key sets:\n"+
			"expected map: %v\n actual map: %v", expectedInputToChildren, reply.InputToChildren)
	}

	for ticket, expectedChildren := range expectedInputToChildren {
		children := reply.InputToChildren[ticket]
		if !reflect.DeepEqual(listToSet(expectedChildren.Tickets), listToSet(children.Tickets)) {
			t.Errorf("Mismatch for children of ticket %s; expected:\n%v actual:\n%v",
				ticket, expectedChildren, children)
		}
	}
}

func TestParents_badData(t *testing.T) {
	svc := Construct(t)

	reply, err := svc.Parents(ctx, &epb.ParentsRequest{
		Tickets: []string{badType},
	})
	if err == nil {
		t.Errorf("Expected Parents error for bad data, got: %v", reply)
	}
}

func TestParents_noData(t *testing.T) {
	svc := Construct(t)

	reply, err := svc.Parents(ctx, &epb.ParentsRequest{
		Tickets: []string{"key with no data"},
	})
	if err != nil {
		testutil.FatalOnErrT(t, "Parents error: %v", err)
	}
	if len(reply.InputToParents) != 0 {
		t.Errorf("Expected empty response for missing key, got: %v", reply)
	}
}

func TestParents(t *testing.T) {
	svc := Construct(t)
	request := &epb.ParentsRequest{
		Tickets: []string{p1c1},
	}

	reply, err := svc.Parents(ctx, request)
	testutil.FatalOnErrT(t, "Parents error: %v", err)

	expectedInputToParents := map[string]*epb.Tickets{
		p1c1: {
			Tickets: []string{p1},
		},
	}

	if !equalTicketsKeys(reply.InputToParents, expectedInputToParents) {
		t.Errorf("Expected results and actual results have different ticket key sets:\n"+
			"expected map: %v\n actual map: %v", expectedInputToParents, reply.InputToParents)
	}

	for ticket, expectedParents := range expectedInputToParents {
		parents := reply.InputToParents[ticket]
		if !reflect.DeepEqual(listToSet(expectedParents.Tickets), listToSet(parents.Tickets)) {
			t.Errorf("Mismatch for children of ticket %s; expected:\n%v actual:\n%v",
				ticket, expectedParents, parents)
		}
	}
}

// Returns true iff the two maps have the same key sets.
// FIXME: Is there any way to make this function more generic, so that it will work for any value type?
func equalTicketsKeys(m1, m2 map[string]*epb.Tickets) bool {
	if len(m1) != len(m2) {
		return false
	}
	return reflect.DeepEqual(ticketsMapAsSet(m1), ticketsMapAsSet(m2))
}

// Returns the map keys as a "set" (map of keys to booleans).
func ticketsMapAsSet(m map[string]*epb.Tickets) map[string]bool {
	s := make(map[string]bool, len(m))
	for k := range m {
		s[k] = true
	}
	return s
}

func TestCallers_badData(t *testing.T) {
	svc := Construct(t)

	reply, err := svc.Callers(ctx, &epb.CallersRequest{
		Tickets: []string{badType},
	})
	if err == nil {
		t.Errorf("Expected Callers error for bad data, got: %v", reply)
	}
}

func TestCallers_noData(t *testing.T) {
	svc := Construct(t)

	reply, err := svc.Callers(ctx, &epb.CallersRequest{
		Tickets: []string{"key with no data"},
	})
	if err != nil {
		testutil.FatalOnErrT(t, "CallersRequest error: %v", err)
	}
	if len(reply.Graph.Nodes) != 0 {
		t.Errorf("Expected empty response for missing key, got: %v", reply)
	}
}

func TestCallers(t *testing.T) {
	svc := Construct(t)
	request := &epb.CallersRequest{
		Tickets: []string{f1},
	}

	reply, err := svc.Callers(ctx, request)
	testutil.FatalOnErrT(t, "Callers error: %v", err)

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
	svc := Construct(t)
	request := &epb.CallersRequest{
		Tickets: []string{f1, f2, f3},
	}

	reply, err := svc.Callers(ctx, request)
	testutil.FatalOnErrT(t, "Callers error: %v", err)

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
	svc := Construct(t)

	reply, err := svc.Callees(ctx, &epb.CalleesRequest{
		Tickets: []string{badType},
	})
	if err == nil {
		t.Errorf("Expected Callees error for bad data, got: %v", reply)
	}
}

func TestCallees_noData(t *testing.T) {
	svc := Construct(t)

	reply, err := svc.Callees(ctx, &epb.CalleesRequest{
		Tickets: []string{"key with no data"},
	})
	if err != nil {
		testutil.FatalOnErrT(t, "CalleesRequest error: %v", err)
	}
	if len(reply.Graph.Nodes) != 0 {
		t.Errorf("Expected empty response for missing key, got: %v", reply)
	}
}

func TestCallees(t *testing.T) {
	svc := Construct(t)
	request := &epb.CalleesRequest{
		Tickets: []string{fr},
	}

	reply, err := svc.Callees(ctx, request)
	testutil.FatalOnErrT(t, "Callees error: %v", err)

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
	svc := Construct(t)
	request := &epb.CalleesRequest{
		Tickets: []string{f1, f2, f3},
	}

	reply, err := svc.Callees(ctx, request)
	testutil.FatalOnErrT(t, "Callees error: %v", err)

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
		t.Errorf("Unexpected mismatch in graph node counts: expected: %d, actual: %d",
			len(expected.Nodes), len(actual.Nodes))
	}

	if !reflect.DeepEqual(nodeMapAsSet(expected.Nodes), nodeMapAsSet(actual.Nodes)) {
		t.Errorf("Unexpected mismatch in graph node sets:\n expected:\n%v\n actual:\n%v\n",
			nodeMapAsSet(expected.Nodes), nodeMapAsSet(actual.Nodes))
	}

	for ticket, expectedNode := range expected.Nodes {
		node := actual.Nodes[ticket]
		if !reflect.DeepEqual(listToSet(expectedNode.Predecessors), listToSet(node.Predecessors)) {
			t.Errorf("Mismatch in graph node predecessors for %s:\n expected:\n%v\n actual:\n%v\n",
				ticket, expectedNode.Predecessors, node.Predecessors)
		}
		if !reflect.DeepEqual(listToSet(expectedNode.Successors), listToSet(node.Successors)) {
			t.Errorf("Mismatch in graph node successors for %s:\n expected:\n%v\n actual:\n%v\n",
				ticket, expectedNode.Successors, node.Successors)
		}
	}
}

// Returns the map keys as a "set" (map of keys to booleans).
func nodeMapAsSet(m map[string]*epb.GraphNode) map[string]bool {
	s := make(map[string]bool, len(m))
	for k := range m {
		s[k] = true
	}
	return s
}

// Returns the list as a "set" (map of list to booleans).
func listToSet(l []string) map[string]bool {
	s := make(map[string]bool, len(l))
	for _, elt := range l {
		s[elt] = true
	}
	return s
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

func Construct(t *testing.T) *Tables {
	return &Tables{
		ParentToChildren:  parentToChildren,
		ChildToParents:    childToParents,
		FunctionToCallers: functionToCallers,
		FunctionToCallees: functionToCallees,
	}
}
