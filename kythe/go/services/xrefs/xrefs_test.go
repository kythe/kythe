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
	"context"
	"errors"
	"fmt"
	"regexp"
	"testing"

	"kythe.io/kythe/go/util/schema/edges"
	"kythe.io/kythe/go/util/schema/facts"

	"github.com/golang/protobuf/proto"

	cpb "kythe.io/kythe/proto/common_proto"
	gpb "kythe.io/kythe/proto/graph_proto"
	xpb "kythe.io/kythe/proto/xref_proto"
)

func TestFilterRegexp(t *testing.T) {
	tests := []struct {
		filter string
		regexp string
	}{
		{"", ""},

		// Bare glob patterns
		{"?", "[^/]"},
		{"*", "[^/]*"},
		{"**", ".*"},

		// Literal characters
		{facts.NodeKind, facts.NodeKind},
		{`!@#$%^&()-_=+[]{};:'"/<>.,`, regexp.QuoteMeta(`!@#$%^&()-_=+[]{};:'"/<>.,`)},
		{"abcdefghijklmnopqrstuvwxyz", "abcdefghijklmnopqrstuvwxyz"},
		{"ABCDEFGHIJKLMNOPQRSTUVWXYZ", "ABCDEFGHIJKLMNOPQRSTUVWXYZ"},

		{"/kythe/*", "/kythe/[^/]*"},
		{"/kythe/**", "/kythe/.*"},
		{"/array#?", "/array#[^/]"},
		{"/kythe/node?/*/blah/**", "/kythe/node[^/]/[^/]*/blah/.*"},
	}

	for _, test := range tests {
		res := filterToRegexp(test.filter)
		if res.String() != test.regexp {
			t.Errorf(" Filter %q; Got %q; Expected regexp %q", test.filter, res, test.regexp)
		}
	}
}

func TestNormalizerPoint(t *testing.T) {
	const text = `line 1
line 2
last line without newline`

	tests := []struct{ p, expected *cpb.Point }{
		{
			&cpb.Point{},
			&cpb.Point{ByteOffset: 0, LineNumber: 1, ColumnOffset: 0},
		},
		{
			&cpb.Point{LineNumber: 1},
			&cpb.Point{ByteOffset: 0, LineNumber: 1, ColumnOffset: 0},
		},
		{
			&cpb.Point{ByteOffset: 1},
			&cpb.Point{ByteOffset: 1, LineNumber: 1, ColumnOffset: 1},
		},
		{
			&cpb.Point{ByteOffset: 6},
			&cpb.Point{ByteOffset: 6, LineNumber: 1, ColumnOffset: 6},
		},
		{
			&cpb.Point{LineNumber: 1, ColumnOffset: 6},
			&cpb.Point{ByteOffset: 6, LineNumber: 1, ColumnOffset: 6},
		},
		{
			&cpb.Point{ByteOffset: 7},
			&cpb.Point{ByteOffset: 7, LineNumber: 2, ColumnOffset: 0},
		},
		{
			&cpb.Point{LineNumber: 2},
			&cpb.Point{ByteOffset: 7, LineNumber: 2, ColumnOffset: 0},
		},
		{
			&cpb.Point{ByteOffset: 10},
			&cpb.Point{ByteOffset: 10, LineNumber: 2, ColumnOffset: 3},
		},
		{
			&cpb.Point{ByteOffset: 13},
			&cpb.Point{ByteOffset: 13, LineNumber: 2, ColumnOffset: 6},
		},
		{
			&cpb.Point{LineNumber: 2, ColumnOffset: 6},
			&cpb.Point{ByteOffset: 13, LineNumber: 2, ColumnOffset: 6},
		},
		{
			&cpb.Point{LineNumber: 2, ColumnOffset: 7}, // past end of column
			&cpb.Point{ByteOffset: 13, LineNumber: 2, ColumnOffset: 6},
		},
		{
			&cpb.Point{LineNumber: 3},
			&cpb.Point{ByteOffset: 14, LineNumber: 3, ColumnOffset: 0},
		},
		{
			&cpb.Point{LineNumber: 3, ColumnOffset: 5},
			&cpb.Point{ByteOffset: 19, LineNumber: 3, ColumnOffset: 5},
		},
		{
			&cpb.Point{ByteOffset: 39},
			&cpb.Point{ByteOffset: 39, LineNumber: 3, ColumnOffset: 25},
		},
		{
			&cpb.Point{ByteOffset: 40}, // past end of text
			&cpb.Point{ByteOffset: 39, LineNumber: 3, ColumnOffset: 25},
		},
		{
			&cpb.Point{LineNumber: 5, ColumnOffset: 5}, // past end of text
			&cpb.Point{ByteOffset: 39, LineNumber: 3, ColumnOffset: 25},
		},
		{
			&cpb.Point{ByteOffset: -1}, // before start of text
			&cpb.Point{ByteOffset: 0, LineNumber: 1, ColumnOffset: 0},
		},
		{
			&cpb.Point{LineNumber: -1, ColumnOffset: 5}, // before start of text
			&cpb.Point{LineNumber: 1},
		},
	}

	n := NewNormalizer([]byte(text))
	for _, test := range tests {
		if p := n.Point(test.p); !proto.Equal(p, test.expected) {
			t.Errorf("n.Point({%v}): expected {%v}; found {%v}", test.p, test.expected, p)
		}
	}
}

func TestPatcher(t *testing.T) {
	tests := []struct {
		oldText, newText string

		oldSpans []*span
		newSpans []*span
	}{{
		oldText:  "this is some text",
		newText:  "this is some changed text",
		oldSpans: []*span{{0, 5}, {13, 17}, {8, 17}},
		newSpans: []*span{{0, 5}, {21, 25}, nil},
	}, {
		oldText:  "line one\nline two\nline three\nline four\n",
		newText:  "line one\nline three\nline two\nline four\n",
		oldSpans: []*span{{0, 10}, {10, 14}, {16, 19}, {20, 24}, {25, 30}, {29, 38}},
		newSpans: []*span{{0, 10}, {10, 14}, nil, {22, 26}, nil, {29, 38}},
	}}

	for _, test := range tests {
		p := NewPatcher([]byte(test.oldText), []byte(test.newText))
		if len(test.oldSpans) != len(test.newSpans) {
			t.Fatalf("Invalid test: {%v}", test)
		}

		for i, s := range test.oldSpans {
			start, end, exists := p.Patch(s.start, s.end)

			if ns := test.newSpans[i]; ns == nil && exists {
				t.Errorf("Expected span not to exist in new text; received (%d, %d]", start, end)
			} else if ns != nil && !exists {
				t.Errorf("Expected span %v to exist in new text as %v; did not exist", s, ns)
			} else if ns != nil && exists && (start != ns.start || end != ns.end) {
				t.Errorf("Expected %v; received (%d, %d]", ns, start, end)
			}
		}
	}
}

type span struct{ start, end int32 }

func (s span) String() string { return fmt.Sprintf("(%d, %d]", s.start, s.end) }

type mockNode struct {
	ticket, kind, documented, defines, completes, completed, childof, typed, text, defaultParam string
	params, definitionText                                                                      []string
	code                                                                                        *cpb.MarkedSource
}

// mockService implements interface xrefs.Service.
type mockService struct {
	nodes map[string]*cpb.NodeInfo
	esets map[string]*gpb.EdgeSet
	xrefs map[string]*xpb.CrossReferencesReply_CrossReferenceSet
}

func makeMockService(db []mockNode) *mockService {
	s := &mockService{
		nodes: make(map[string]*cpb.NodeInfo),
		esets: make(map[string]*gpb.EdgeSet),
		xrefs: make(map[string]*xpb.CrossReferencesReply_CrossReferenceSet),
	}
	for _, node := range db {
		s.nodes[node.ticket] = &cpb.NodeInfo{
			Facts: map[string][]byte{
				facts.NodeKind: []byte(node.kind),
				facts.Text:     []byte(node.text),
			},
		}
		if node.code != nil {
			p, err := proto.Marshal(node.code)
			if err != nil {
				panic("couldn't set up test: couldn't marshal input proto")
			}
			s.nodes[node.ticket].Facts[facts.Code] = p
		}
		if node.defaultParam != "" {
			s.nodes[node.ticket].Facts[facts.ParamDefault] = []byte(node.defaultParam)
		}
		if node.definitionText != nil {
			set := &xpb.CrossReferencesReply_CrossReferenceSet{Ticket: node.ticket}
			for _, text := range node.definitionText {
				set.Definition = append(set.Definition, &xpb.CrossReferencesReply_RelatedAnchor{Anchor: &xpb.Anchor{Text: text, Ticket: node.ticket + "a"}})
			}
			s.xrefs[node.ticket] = set
		}
		set := &gpb.EdgeSet{Groups: make(map[string]*gpb.EdgeSet_Group)}
		if node.typed != "" {
			set.Groups[edges.Typed] = &gpb.EdgeSet_Group{
				Edge: []*gpb.EdgeSet_Group_Edge{{TargetTicket: node.typed}},
			}
		}
		if node.childof != "" {
			set.Groups[edges.ChildOf] = &gpb.EdgeSet_Group{
				Edge: []*gpb.EdgeSet_Group_Edge{{TargetTicket: node.childof}},
			}
		}
		if node.documented != "" {
			set.Groups[edges.Mirror(edges.Documents)] = &gpb.EdgeSet_Group{
				Edge: []*gpb.EdgeSet_Group_Edge{{TargetTicket: node.documented}},
			}
		}
		if node.completes != "" {
			set.Groups[edges.Completes] = &gpb.EdgeSet_Group{
				Edge: []*gpb.EdgeSet_Group_Edge{{TargetTicket: node.completes}},
			}
		}
		if node.completed != "" {
			set.Groups[edges.Mirror(edges.Completes)] = &gpb.EdgeSet_Group{
				Edge: []*gpb.EdgeSet_Group_Edge{{TargetTicket: node.completed}},
			}
		}
		if node.defines != "" {
			set.Groups[edges.DefinesBinding] = &gpb.EdgeSet_Group{
				Edge: []*gpb.EdgeSet_Group_Edge{{TargetTicket: node.defines}},
			}
		}
		if node.params != nil {
			var groups []*gpb.EdgeSet_Group_Edge
			for i, p := range node.params {
				groups = append(groups, &gpb.EdgeSet_Group_Edge{TargetTicket: p, Ordinal: int32(i)})
			}
			set.Groups[edges.Param] = &gpb.EdgeSet_Group{Edge: groups}
		}
		s.esets[node.ticket] = set
	}
	return s
}

func (s *mockService) getNode(ticket string, facts []string) *cpb.NodeInfo {
	data, found := s.nodes[ticket]
	if !found {
		return nil
	}
	info := &cpb.NodeInfo{Facts: make(map[string][]byte)}
	for _, fact := range facts {
		info.Facts[fact] = data.Facts[fact]
	}
	return info
}

func (s *mockService) Nodes(ctx context.Context, req *gpb.NodesRequest) (*gpb.NodesReply, error) {
	reply := &gpb.NodesReply{Nodes: make(map[string]*cpb.NodeInfo)}
	for _, ticket := range req.Ticket {
		if info := s.getNode(ticket, req.Filter); info != nil {
			reply.Nodes[ticket] = info
		}
	}
	return reply, nil
}

func (s *mockService) Edges(ctx context.Context, req *gpb.EdgesRequest) (*gpb.EdgesReply, error) {
	reply := &gpb.EdgesReply{
		EdgeSets: make(map[string]*gpb.EdgeSet),
		Nodes:    make(map[string]*cpb.NodeInfo),
	}
	for _, ticket := range req.Ticket {
		if data, found := s.esets[ticket]; found {
			set := &gpb.EdgeSet{Groups: make(map[string]*gpb.EdgeSet_Group)}
			reply.EdgeSets[ticket] = set
			nodes := make(map[string]bool)
			for groupKind, group := range data.Groups {
				for _, kind := range req.Kind {
					if groupKind == kind {
						set.Groups[kind] = group
						for _, edge := range group.Edge {
							if !nodes[edge.TargetTicket] {
								nodes[edge.TargetTicket] = true
								if node := s.getNode(edge.TargetTicket, req.Filter); node != nil {
									reply.Nodes[edge.TargetTicket] = node
								}
							}
						}
						break
					}
				}
			}
		}
	}
	return reply, nil
}

func (s *mockService) Decorations(ctx context.Context, req *xpb.DecorationsRequest) (*xpb.DecorationsReply, error) {
	return nil, errors.New("unexpected call to Decorations")
}

func (s *mockService) CrossReferences(ctx context.Context, req *xpb.CrossReferencesRequest) (*xpb.CrossReferencesReply, error) {
	reply := &xpb.CrossReferencesReply{
		CrossReferences: make(map[string]*xpb.CrossReferencesReply_CrossReferenceSet),
	}
	if req.DefinitionKind != xpb.CrossReferencesRequest_BINDING_DEFINITIONS &&
		req.DefinitionKind != xpb.CrossReferencesRequest_ALL_DEFINITIONS ||
		req.ReferenceKind != xpb.CrossReferencesRequest_NO_REFERENCES ||
		req.AnchorText {
		return nil, fmt.Errorf("Unexpected CrossReferences request: %v", req)
	}
	for _, ticket := range req.Ticket {
		if set, found := s.xrefs[ticket]; found {
			reply.CrossReferences[ticket] = set
		}
	}
	return reply, nil
}

func (s *mockService) Documentation(ctx context.Context, req *xpb.DocumentationRequest) (*xpb.DocumentationReply, error) {
	return nil, errors.New("unexpected call to Documentation")
}

func containsString(arr []string, key string) bool {
	for _, i := range arr {
		if i == key {
			return true
		}
	}
	return false
}
