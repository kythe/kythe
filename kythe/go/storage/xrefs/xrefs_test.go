/*
 * Copyright 2014 Google Inc. All rights reserved.
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
	"fmt"
	"sort"
	"testing"

	"kythe.io/kythe/go/services/graphstore"
	"kythe.io/kythe/go/services/xrefs"
	"kythe.io/kythe/go/storage/inmemory"
	"kythe.io/kythe/go/test/testutil"
	"kythe.io/kythe/go/util/kytheuri"
	"kythe.io/kythe/go/util/schema"

	"golang.org/x/net/context"

	spb "kythe.io/kythe/proto/storage_proto"
	xpb "kythe.io/kythe/proto/xref_proto"
)

var (
	ctx = context.Background()

	testFileVName    = sig("testFileNode")
	testFileContent  = "file_content"
	testFileEncoding = "UTF-8"

	testAnchorVName       = sig("testAnchor")
	testAnchorTargetVName = sig("someSemanticNode")

	testNodes = []*node{
		{sig("orphanedNode"), facts(schema.NodeKindFact, "orphan"), nil},
		{testFileVName, facts(
			schema.NodeKindFact, schema.FileKind,
			schema.TextFact, testFileContent,
			schema.TextEncodingFact, testFileEncoding), map[string][]*spb.VName{
			revChildOfEdgeKind: {testAnchorVName},
		}},
		{sig("sig2"), facts(schema.NodeKindFact, "test"), map[string][]*spb.VName{
			"someEdgeKind": {sig("signature")},
		}},
		{sig("signature"), facts(schema.NodeKindFact, "test"), map[string][]*spb.VName{
			schema.MirrorEdge("someEdgeKind"): {sig("sig2")},
			schema.ParamEdge:                  {sig("sig2"), sig("someParameter")},
		}},
		{testAnchorVName, facts(
			schema.AnchorEndFact, "4",
			schema.AnchorStartFact, "1",
			schema.NodeKindFact, schema.AnchorKind,
		), map[string][]*spb.VName{
			schema.ChildOfEdge: {testFileVName},
			schema.RefEdge:     {testAnchorTargetVName},
		}},
		{testAnchorTargetVName, facts(schema.NodeKindFact, "record"), map[string][]*spb.VName{
			schema.MirrorEdge(schema.RefEdge): {testAnchorVName},
		}},
	}
	testEntries = nodesToEntries(testNodes)
)

func TestNodes(t *testing.T) {
	xs := newService(t, testEntries)

	reply, err := xs.Nodes(ctx, &xpb.NodesRequest{
		Ticket: nodesToTickets(testNodes),
	})
	if err != nil {
		t.Fatalf("Error fetching nodes for %+v: %v", nodesToTickets(testNodes), err)
	}
	expected := nodesToInfos(testNodes)
	if err := testutil.DeepEqual(expected, reply.Nodes); err != nil {
		t.Fatal(err)
	}
}

func TestEdges(t *testing.T) {
	xs := newService(t, testEntries)

	reply, err := xs.Edges(ctx, &xpb.EdgesRequest{
		Ticket: nodesToTickets(testNodes),
		Filter: []string{"**"}, // every fact
	})
	if err != nil {
		t.Fatalf("Error fetching edges for %+v: %v", nodesToTickets(testNodes), err)
	}

	expectedEdges := nodesToEdgeSets(testNodes)
	if err := testutil.DeepEqual(expectedEdges, sortEdgeSets(reply.EdgeSets)); err != nil {
		t.Error(err)
	}

	nodesWithEdges := testNodes[1:]
	expectedInfos := nodesToInfos(nodesWithEdges)
	if err := testutil.DeepEqual(expectedInfos, reply.Nodes); err != nil {
		t.Error(err)
	}
}

func TestDecorations(t *testing.T) {
	xs := newService(t, testEntries)

	reply, err := xs.Decorations(ctx, &xpb.DecorationsRequest{
		Location: &xpb.Location{
			Ticket: kytheuri.ToString(testFileVName),
		},
		SourceText: true,
		References: true,
		Filter:     []string{"**"},
	})
	if err != nil {
		t.Fatalf("Error fetching decorations for %+v: %v", testFileVName, err)
	}

	if string(reply.SourceText) != testFileContent {
		t.Errorf("Incorrect file content: %q; Expected: %q", string(reply.SourceText), testFileContent)
	}
	if reply.Encoding != testFileEncoding {
		t.Errorf("Incorrect file encoding: %q; Expected: %q", reply.Encoding, testFileEncoding)
	}

	expectedRefs := []*xpb.DecorationsReply_Reference{
		{
			SourceTicket: kytheuri.ToString(testAnchorVName),
			TargetTicket: kytheuri.ToString(testAnchorTargetVName),
			Kind:         schema.RefEdge,
			AnchorStart: &xpb.Location_Point{
				ByteOffset:   1,
				LineNumber:   1,
				ColumnOffset: 1,
			},
			AnchorEnd: &xpb.Location_Point{
				ByteOffset:   4,
				LineNumber:   1,
				ColumnOffset: 4,
			},
		},
	}
	if err := testutil.DeepEqual(sortRefs(expectedRefs), sortRefs(reply.Reference)); err != nil {
		t.Error(err)
	}

	expectedNodes := nodesToInfos(testNodes[4:6])
	if err := testutil.DeepEqual(expectedNodes, reply.Nodes); err != nil {
		t.Error(err)
	}
}

func TestCallers(t *testing.T) {
	xs := newService(t, testEntries)

	reply, err := xs.Callers(ctx, &xpb.CallersRequest{})
	if reply != nil || err == nil {
		t.Fatalf("Callers expected to fail")
	}
}

func TestDocumentation(t *testing.T) {
	xs := newService(t, testEntries)

	reply, err := xs.Documentation(ctx, &xpb.DocumentationRequest{})
	if reply != nil || err == nil {
		t.Fatalf("Documentation expected to fail")
	}
}

func newService(t *testing.T, entries []*spb.Entry) *GraphStoreService {
	gs := inmemory.Create()

	for req := range graphstore.BatchWrites(channelEntries(entries), 64) {
		if err := gs.Write(ctx, req); err != nil {
			t.Fatalf("Failed to write entries: %v", err)
		}
	}
	return NewGraphStoreService(gs)
}

func channelEntries(entries []*spb.Entry) <-chan *spb.Entry {
	ch := make(chan *spb.Entry)
	go func() {
		defer close(ch)
		for _, entry := range entries {
			ch <- entry
		}
	}()
	return ch
}

type node struct {
	Source *spb.VName
	// FactName -> FactValue
	Facts map[string]string
	// EdgeKind -> Targets
	Edges map[string][]*spb.VName
}

func (n *node) Info() *xpb.NodeInfo {
	info := &xpb.NodeInfo{
		Facts: make(map[string][]byte, len(n.Facts)),
	}
	for name, val := range n.Facts {
		info.Facts[name] = []byte(val)
	}
	return info
}

func (n *node) EdgeSet() *xpb.EdgeSet {
	groups := make(map[string]*xpb.EdgeSet_Group)
	for kind, targets := range n.Edges {
		var edges []*xpb.EdgeSet_Group_Edge
		for ordinal, target := range targets {
			edges = append(edges, &xpb.EdgeSet_Group_Edge{
				TargetTicket: kytheuri.ToString(target),
				Ordinal:      int32(ordinal),
			})
		}
		groups[kind] = &xpb.EdgeSet_Group{
			Edge: edges,
		}
	}
	return &xpb.EdgeSet{
		Groups: groups,
	}
}

func nodesToTickets(nodes []*node) []string {
	var tickets []string
	for _, n := range nodes {
		tickets = append(tickets, kytheuri.ToString(n.Source))
	}
	return tickets
}

func nodesToEntries(nodes []*node) []*spb.Entry {
	var entries []*spb.Entry
	for _, n := range nodes {
		for fact, val := range n.Facts {
			entries = append(entries, nodeFact(n.Source, fact, val))
		}
		for edgeKind, targets := range n.Edges {
			for ordinal, target := range targets {
				entries = append(entries, edgeFact(n.Source, edgeKind, ordinal, target))
			}
		}
	}
	return entries
}

func nodesToInfos(nodes []*node) map[string]*xpb.NodeInfo {
	m := make(map[string]*xpb.NodeInfo)
	for _, n := range nodes {
		m[kytheuri.ToString(n.Source)] = n.Info()
	}
	return m
}

func nodesToEdgeSets(nodes []*node) map[string]*xpb.EdgeSet {
	sets := make(map[string]*xpb.EdgeSet)
	for _, n := range nodes {
		set := n.EdgeSet()
		if len(set.Groups) > 0 {
			sets[kytheuri.ToString(n.Source)] = set
		}
	}
	return sets
}

func sig(sig string) *spb.VName {
	return &spb.VName{Signature: sig}
}

func facts(keyVals ...string) map[string]string {
	facts := make(map[string]string)
	for i := 0; i < len(keyVals); i += 2 {
		facts[keyVals[i]] = keyVals[i+1]
	}
	return facts
}

func nodeFact(vname *spb.VName, fact, val string) *spb.Entry {
	return &spb.Entry{
		Source:    vname,
		FactName:  fact,
		FactValue: []byte(val),
	}
}

func edgeFact(source *spb.VName, kind string, ordinal int, target *spb.VName) *spb.Entry {
	if ordinal > 0 {
		kind = fmt.Sprintf("%s.%d", kind, ordinal)
	}
	return &spb.Entry{
		Source:    source,
		Target:    target,
		EdgeKind:  kind,
		FactName:  "/",
		FactValue: nil,
	}
}

////// Everything below is for sorting results to ensure order doesn't matter

type sortedReferences []*xpb.DecorationsReply_Reference

func (h sortedReferences) Len() int { return len(h) }
func (h sortedReferences) Less(i, j int) bool {
	switch {
	case h[i].SourceTicket < h[j].SourceTicket:
		return true
	case h[i].SourceTicket > h[j].SourceTicket:
		return false
	case h[i].Kind < h[j].Kind:
		return true
	case h[i].Kind > h[j].Kind:
		return false
	}
	return h[i].TargetTicket < h[j].TargetTicket
}
func (h sortedReferences) Swap(i, j int) {
	h[i], h[j] = h[j], h[i]
}

func sortEdgeSets(sets map[string]*xpb.EdgeSet) map[string]*xpb.EdgeSet {
	for _, set := range sets {
		for _, g := range set.Groups {
			sort.Sort(xrefs.ByOrdinal(g.Edge))
		}
	}
	return sets
}

func sortRefs(refs []*xpb.DecorationsReply_Reference) []*xpb.DecorationsReply_Reference {
	sort.Sort(sortedReferences(refs))
	return refs
}
