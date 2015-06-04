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
	"reflect"
	"sort"
	"testing"

	"kythe.io/kythe/go/services/graphstore"
	"kythe.io/kythe/go/storage/inmemory"
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
		}},
		{testAnchorVName, facts(schema.NodeKindFact, schema.AnchorKind), map[string][]*spb.VName{
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
	if !reflect.DeepEqual(sortInfos(reply.Node), expected) {
		t.Errorf("Got %v; Expected %v", reply.Node, expected)
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
	if !reflect.DeepEqual(sortEdgeSets(reply.EdgeSet), expectedEdges) {
		t.Errorf("Got %v; Expected edgeSets %v", reply.EdgeSet, expectedEdges)
	}

	nodesWithEdges := testNodes[1:]
	expectedInfos := nodesToInfos(nodesWithEdges)
	if !reflect.DeepEqual(sortInfos(reply.Node), expectedInfos) {
		t.Errorf("Got %v; Expected nodes %v", reply.Node, expectedInfos)
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
		},
	}
	if !reflect.DeepEqual(sortRefs(reply.Reference), sortRefs(expectedRefs)) {
		t.Errorf("Got %v; Expected references %v", reply.Reference, expectedRefs)
	}

	refNodes := testNodes[4:6]
	expectedNodes := nodesToInfos(refNodes)
	if !reflect.DeepEqual(sortInfos(reply.Node), expectedNodes) {
		t.Errorf("Got %v; Expected nodes %v", reply.Node, expectedNodes)
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
		Ticket: kytheuri.ToString(n.Source),
	}
	for name, val := range n.Facts {
		info.Fact = append(info.Fact, &xpb.Fact{
			Name:  name,
			Value: []byte(val),
		})
	}
	return info
}

func (n *node) EdgeSet() *xpb.EdgeSet {
	var groups []*xpb.EdgeSet_Group
	for kind, targets := range n.Edges {
		var tickets []string
		for _, target := range targets {
			tickets = append(tickets, kytheuri.ToString(target))
		}
		groups = append(groups, &xpb.EdgeSet_Group{
			Kind:         kind,
			TargetTicket: tickets,
		})
	}
	return &xpb.EdgeSet{
		SourceTicket: kytheuri.ToString(n.Source),
		Group:        groups,
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
			for _, target := range targets {
				entries = append(entries, edgeFact(n.Source, edgeKind, target))
			}
		}
	}
	return entries
}

func nodesToInfos(nodes []*node) []*xpb.NodeInfo {
	var infos []*xpb.NodeInfo
	for _, n := range nodes {
		infos = append(infos, n.Info())
	}
	return sortInfos(infos)
}

func nodesToEdgeSets(nodes []*node) []*xpb.EdgeSet {
	var sets []*xpb.EdgeSet
	for _, n := range nodes {
		set := n.EdgeSet()
		if len(set.Group) > 0 {
			sets = append(sets, set)
		}
	}
	return sortEdgeSets(sets)
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

func edgeFact(source *spb.VName, kind string, target *spb.VName) *spb.Entry {
	return &spb.Entry{
		Source:    source,
		Target:    target,
		EdgeKind:  kind,
		FactName:  "/",
		FactValue: []byte{},
	}
}

////// Everything below is for sorting results to ensure order doesn't matter

type sortedFacts []*xpb.Fact

func (h sortedFacts) Len() int { return len(h) }
func (h sortedFacts) Less(i, j int) bool {
	return h[i].Name < h[j].Name
}
func (h sortedFacts) Swap(i, j int) {
	h[i], h[j] = h[j], h[i]
}

type sortedGroups []*xpb.EdgeSet_Group

func (h sortedGroups) Len() int { return len(h) }
func (h sortedGroups) Less(i, j int) bool {
	return h[i].Kind < h[j].Kind
}
func (h sortedGroups) Swap(i, j int) {
	h[i], h[j] = h[j], h[i]
}

type sortedEdgeSets []*xpb.EdgeSet

func (h sortedEdgeSets) Len() int { return len(h) }
func (h sortedEdgeSets) Less(i, j int) bool {
	return h[i].SourceTicket < h[j].SourceTicket
}
func (h sortedEdgeSets) Swap(i, j int) {
	h[i], h[j] = h[j], h[i]
}

type sortedNodeInfos []*xpb.NodeInfo

func (h sortedNodeInfos) Len() int { return len(h) }
func (h sortedNodeInfos) Less(i, j int) bool {
	return h[i].Ticket < h[j].Ticket
}
func (h sortedNodeInfos) Swap(i, j int) {
	h[i], h[j] = h[j], h[i]
}

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

func sortEdgeSets(sets []*xpb.EdgeSet) []*xpb.EdgeSet {
	sort.Sort(sortedEdgeSets(sets))
	for _, set := range sets {
		sort.Sort(sortedGroups(set.Group))
	}
	return sets
}

func sortInfos(infos []*xpb.NodeInfo) []*xpb.NodeInfo {
	sort.Sort(sortedNodeInfos(infos))
	for _, info := range infos {
		sort.Sort(sortedFacts(info.Fact))
	}
	return infos
}

func sortRefs(refs []*xpb.DecorationsReply_Reference) []*xpb.DecorationsReply_Reference {
	sort.Sort(sortedReferences(refs))
	return refs
}
