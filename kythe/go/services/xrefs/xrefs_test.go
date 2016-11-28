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
	"errors"
	"fmt"
	"log"
	"reflect"
	"regexp"
	"testing"

	"kythe.io/kythe/go/test/testutil"
	"kythe.io/kythe/go/util/schema/edges"
	"kythe.io/kythe/go/util/schema/facts"

	"golang.org/x/net/context"

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

	tests := []struct{ p, expected *xpb.Location_Point }{
		{
			&xpb.Location_Point{},
			&xpb.Location_Point{ByteOffset: 0, LineNumber: 1, ColumnOffset: 0},
		},
		{
			&xpb.Location_Point{LineNumber: 1},
			&xpb.Location_Point{ByteOffset: 0, LineNumber: 1, ColumnOffset: 0},
		},
		{
			&xpb.Location_Point{ByteOffset: 1},
			&xpb.Location_Point{ByteOffset: 1, LineNumber: 1, ColumnOffset: 1},
		},
		{
			&xpb.Location_Point{ByteOffset: 6},
			&xpb.Location_Point{ByteOffset: 6, LineNumber: 1, ColumnOffset: 6},
		},
		{
			&xpb.Location_Point{LineNumber: 1, ColumnOffset: 6},
			&xpb.Location_Point{ByteOffset: 6, LineNumber: 1, ColumnOffset: 6},
		},
		{
			&xpb.Location_Point{ByteOffset: 7},
			&xpb.Location_Point{ByteOffset: 7, LineNumber: 2, ColumnOffset: 0},
		},
		{
			&xpb.Location_Point{LineNumber: 2},
			&xpb.Location_Point{ByteOffset: 7, LineNumber: 2, ColumnOffset: 0},
		},
		{
			&xpb.Location_Point{ByteOffset: 10},
			&xpb.Location_Point{ByteOffset: 10, LineNumber: 2, ColumnOffset: 3},
		},
		{
			&xpb.Location_Point{ByteOffset: 13},
			&xpb.Location_Point{ByteOffset: 13, LineNumber: 2, ColumnOffset: 6},
		},
		{
			&xpb.Location_Point{LineNumber: 2, ColumnOffset: 6},
			&xpb.Location_Point{ByteOffset: 13, LineNumber: 2, ColumnOffset: 6},
		},
		{
			&xpb.Location_Point{LineNumber: 2, ColumnOffset: 7}, // past end of column
			&xpb.Location_Point{ByteOffset: 13, LineNumber: 2, ColumnOffset: 6},
		},
		{
			&xpb.Location_Point{LineNumber: 3},
			&xpb.Location_Point{ByteOffset: 14, LineNumber: 3, ColumnOffset: 0},
		},
		{
			&xpb.Location_Point{LineNumber: 3, ColumnOffset: 5},
			&xpb.Location_Point{ByteOffset: 19, LineNumber: 3, ColumnOffset: 5},
		},
		{
			&xpb.Location_Point{ByteOffset: 39},
			&xpb.Location_Point{ByteOffset: 39, LineNumber: 3, ColumnOffset: 25},
		},
		{
			&xpb.Location_Point{ByteOffset: 40}, // past end of text
			&xpb.Location_Point{ByteOffset: 39, LineNumber: 3, ColumnOffset: 25},
		},
		{
			&xpb.Location_Point{LineNumber: 5, ColumnOffset: 5}, // past end of text
			&xpb.Location_Point{ByteOffset: 39, LineNumber: 3, ColumnOffset: 25},
		},
		{
			&xpb.Location_Point{ByteOffset: -1}, // before start of text
			&xpb.Location_Point{ByteOffset: 0, LineNumber: 1, ColumnOffset: 0},
		},
		{
			&xpb.Location_Point{LineNumber: -1, ColumnOffset: 5}, // before start of text
			&xpb.Location_Point{LineNumber: 1},
		},
	}

	n := NewNormalizer([]byte(text))
	for _, test := range tests {
		if p := n.Point(test.p); !reflect.DeepEqual(p, test.expected) {
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

type mockService struct {
	NodesFn           func(*gpb.NodesRequest) (*gpb.NodesReply, error)
	EdgesFn           func(*gpb.EdgesRequest) (*gpb.EdgesReply, error)
	DecorationsFn     func(*xpb.DecorationsRequest) (*xpb.DecorationsReply, error)
	CrossReferencesFn func(*xpb.CrossReferencesRequest) (*xpb.CrossReferencesReply, error)
	DocumentationFn   func(*xpb.DocumentationRequest) (*xpb.DocumentationReply, error)
}

func (s *mockService) Nodes(ctx context.Context, req *gpb.NodesRequest) (*gpb.NodesReply, error) {
	if s.NodesFn == nil {
		return nil, errors.New("unexpected call to Nodes")
	}
	return s.NodesFn(req)
}

func (s *mockService) Edges(ctx context.Context, req *gpb.EdgesRequest) (*gpb.EdgesReply, error) {
	if s.EdgesFn == nil {
		return nil, errors.New("unexpected call to Edges")
	}
	return s.EdgesFn(req)
}

func (s *mockService) Decorations(ctx context.Context, req *xpb.DecorationsRequest) (*xpb.DecorationsReply, error) {
	if s.DecorationsFn == nil {
		return nil, errors.New("unexpected call to Decorations")
	}
	return s.DecorationsFn(req)
}

func (s *mockService) CrossReferences(ctx context.Context, req *xpb.CrossReferencesRequest) (*xpb.CrossReferencesReply, error) {
	if s.CrossReferencesFn == nil {
		return nil, errors.New("unexpected call to CrossReferences")
	}
	return s.CrossReferencesFn(req)
}

func (s *mockService) Documentation(ctx context.Context, req *xpb.DocumentationRequest) (*xpb.DocumentationReply, error) {
	if s.DocumentationFn == nil {
		return nil, errors.New("unexpected call to Documentation")
	}
	return s.DocumentationFn(req)
}

func containsString(arr []string, key string) bool {
	for _, i := range arr {
		if i == key {
			return true
		}
	}
	return false
}

func TestSlowSignature(t *testing.T) {
	db := []struct {
		ticket, format, kind, typed, childof string
		params                               []string
	}{
		{ticket: "kythe://test#ident", format: "IDENT", kind: "etc"},
		{ticket: "kythe:?lang=b#etc%23meta", format: "META", kind: "meta"},
		{ticket: "kythe://test?lang=b#meta", kind: "etc"},
		{ticket: "kythe://test#escparen", format: "%%", kind: "etc"},
		{ticket: "kythe://test#escparentext", format: "a%%b%%c", kind: "etc"},
		{ticket: "kythe://test#p0", format: "%[^bad%]%[iP0%]", kind: "etc"},
		{ticket: "kythe://test#p1", format: "P1", kind: "etc"},
		{ticket: "kythe://test#p2", format: "P2", kind: "etc"},
		{ticket: "kythe://test#pselect", format: "%0.%1.", kind: "etc", params: []string{"kythe://test#p0", "kythe://test#p1"}},
		{ticket: "kythe://test#parent", format: "%^::%[iC%]", kind: "etc", childof: "kythe://test#p0"},
		{ticket: "kythe://test#parents", format: "%^::D", kind: "etc", childof: "kythe://test#parent"},
		{ticket: "kythe://test#recurse", format: "%^", kind: "etc", childof: "kythe://test#recurse"},
		{ticket: "kythe://test#list", format: "%[p%0,%]", kind: "etc", params: []string{"kythe://test#p0", "kythe://test#p1", "kythe://test#p2"}},
		{ticket: "kythe://test#listofs1", format: "%1,", kind: "etc", params: []string{"kythe://test#p0", "kythe://test#p1", "kythe://test#p2"}},
		{ticket: "kythe://test#listofs2", format: "%2,", kind: "etc", params: []string{"kythe://test#p0", "kythe://test#p1", "kythe://test#p2"}},
		{ticket: "kythe://test#listofs3", format: "%3,", kind: "etc", params: []string{"kythe://test#p0", "kythe://test#p1", "kythe://test#p2"}},
		{ticket: "kythe://test#typeselect", format: "%1`", kind: "etc", typed: "kythe://test#list"},
		{ticket: "kythe:?lang=b#tapp%23meta", format: "m%2.m", kind: "meta"},
		{ticket: "kythe://test#lhs", format: "l%1.l", kind: "tapp"},
		{ticket: "kythe://test?lang=b#tappmeta", kind: "tapp", params: []string{"kythe://test#missing", "kythe://test#p0", "kythe://test#p1"}},
		{ticket: "kythe://test?lang=b#tapplhs", kind: "tapp", params: []string{"kythe://test#lhs", "kythe://test#p0"}},
	}
	tests := []struct {
		ticket, reply string
		kinds         []xpb.Link_Kind
	}{
		{ticket: "kythe://test#ident", reply: "IDENT"},
		{ticket: "kythe://test?lang=b#meta", reply: "META"},
		{ticket: "kythe://test#escparen", reply: "%"},
		{ticket: "kythe://test#escparentext", reply: "a%b%c"},
		{ticket: "kythe://test#pselect", reply: "P0P1"},
		{ticket: "kythe://test#recurse", reply: "..."},
		{ticket: "kythe://test#parent", reply: "P0::[C]", kinds: []xpb.Link_Kind{xpb.Link_IMPORTANT}},
		{ticket: "kythe://test#parents", reply: "P0::C::D"},
		{ticket: "kythe://test#list", reply: "[[P0], [P1], [P2]]", kinds: []xpb.Link_Kind{xpb.Link_LIST, xpb.Link_LIST_ITEM, xpb.Link_LIST_ITEM, xpb.Link_LIST_ITEM}},
		{ticket: "kythe://test#listofs1", reply: "[P1], [P2]", kinds: []xpb.Link_Kind{xpb.Link_LIST_ITEM, xpb.Link_LIST_ITEM}},
		{ticket: "kythe://test#listofs2", reply: "[P2]", kinds: []xpb.Link_Kind{xpb.Link_LIST_ITEM}},
		{ticket: "kythe://test#listofs3", reply: ""},
		{ticket: "kythe://test#typeselect", reply: "P1"},
		{ticket: "kythe://test?lang=b#tappmeta", reply: "mP1m"},
		{ticket: "kythe://test?lang=b#tapplhs", reply: "lP0l"},
	}
	ninfo := make(map[string]*cpb.NodeInfo)
	esets := make(map[string]*gpb.EdgeSet)
	for _, node := range db {
		ninfo[node.ticket] = &cpb.NodeInfo{
			Facts: map[string][]byte{
				facts.NodeKind: []byte(node.kind),
				facts.Format:   []byte(node.format),
			},
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
		if node.params != nil {
			var groups []*gpb.EdgeSet_Group_Edge
			for i, p := range node.params {
				groups = append(groups, &gpb.EdgeSet_Group_Edge{TargetTicket: p, Ordinal: int32(i)})
			}
			set.Groups[edges.Param] = &gpb.EdgeSet_Group{Edge: groups}
		}
		esets[node.ticket] = set
	}
	getNode := func(ticket string, facts []string) *cpb.NodeInfo {
		data, found := ninfo[ticket]
		if !found {
			return nil
		}
		info := &cpb.NodeInfo{Facts: make(map[string][]byte)}
		for _, fact := range facts {
			info.Facts[fact] = data.Facts[fact]
		}
		return info
	}
	service := &mockService{
		NodesFn: func(req *gpb.NodesRequest) (*gpb.NodesReply, error) {
			log.Printf("NodesFn: %v", req)
			if len(req.Ticket) != 1 {
				t.Fatalf("Unexpected Nodes request: %v", req)
				return nil, nil
			}
			reply := &gpb.NodesReply{Nodes: make(map[string]*cpb.NodeInfo)}
			if info := getNode(req.Ticket[0], req.Filter); info != nil {
				reply.Nodes[req.Ticket[0]] = info
			}
			return reply, nil
		},
		EdgesFn: func(req *gpb.EdgesRequest) (*gpb.EdgesReply, error) {
			log.Printf("EdgesFn: %v", req)
			if len(req.Ticket) != 1 {
				t.Fatalf("Unexpected Edges request: %v", req)
				return nil, nil
			}
			reply := &gpb.EdgesReply{
				EdgeSets: make(map[string]*gpb.EdgeSet),
				Nodes:    make(map[string]*cpb.NodeInfo),
			}
			if data, found := esets[req.Ticket[0]]; found {
				set := &gpb.EdgeSet{Groups: make(map[string]*gpb.EdgeSet_Group)}
				reply.EdgeSets[req.Ticket[0]] = set
				seen := make(map[string]bool)
				for groupKind, group := range data.Groups {
					for _, kind := range req.Kind {
						if groupKind == kind {
							set.Groups[kind] = group
							for _, edge := range group.Edge {
								if !seen[edge.TargetTicket] {
									seen[edge.TargetTicket] = true
									if node := getNode(edge.TargetTicket, req.Filter); node != nil {
										reply.Nodes[edge.TargetTicket] = node
									}
								}
							}
							break
						}
					}
				}
			}
			log.Printf(" => %v", reply)
			return reply, nil
		},
	}
	_, err := SlowSignature(nil, service, "kythe://test#missing")
	if err == nil {
		t.Fatal("SlowSignature should return an error for a missing ticket")
	}
	for _, test := range tests {
		reply, err := SlowSignature(nil, service, test.ticket)
		if err != nil {
			t.Fatalf("SlowSignature error for %s: %v", test.ticket, err)
		}
		var links []*xpb.Link
		for _, k := range test.kinds {
			links = append(links, &xpb.Link{Kind: k})
		}
		if err := testutil.DeepEqual(&xpb.Printable{RawText: test.reply, Link: links}, reply); err != nil {
			t.Fatal(err)
		}
	}
}

func TestSlowDocumentation(t *testing.T) {
	db := []struct {
		ticket, kind, documented, defines, completes, completed, childof, typed, text, format string
		params, definitionText                                                                []string
	}{
		{ticket: "kythe://test#a", kind: "etc", documented: "kythe://test#adoc", format: "asig"},
		{ticket: "kythe://test#adoc", kind: "doc", text: "atext"},
		{ticket: "kythe://test#fdoc", kind: "doc", text: "ftext"},
		{ticket: "kythe://test#fdecl", kind: "function", documented: "kythe://test#fdoc", format: "fsig"},
		{ticket: "kythe://test#fdefn", kind: "function", completed: "kythe://test#fbind", format: "fsig", definitionText: []string{"fdeftext"}},
		{ticket: "kythe://test#fbind", kind: "anchor", defines: "kythe://test#fdefn", completes: "kythe://test#fdecl"},
		{ticket: "kythe://test#l", kind: "etc", documented: "kythe://test#ldoc"},
		{ticket: "kythe://test#ldoc", kind: "doc", text: "ltext", params: []string{"kythe://test#l1", "kythe://test#l2"}},
		{ticket: "kythe://test#l1", kind: "etc", definitionText: []string{"deftext1"}},
		{ticket: "kythe://test#l2", kind: "etc", definitionText: []string{"deftext2"}},
		{ticket: "kythe://test#l", kind: "etc", documented: "kythe://test#ldoc", format: "lsig"},
	}
	mkPr := func(text string, linkTicket ...string) *xpb.Printable {
		links := make([]*xpb.Link, len(linkTicket))
		for i, link := range linkTicket {
			links[i] = &xpb.Link{Definition: []string{link}}
		}
		return &xpb.Printable{RawText: text, Link: links}
	}
	type returnNode struct{ ticket, kind, anchorText string }
	tests := []struct {
		ticket string
		reply  *xpb.DocumentationReply_Document
		nodes  []returnNode
	}{
		{ticket: "kythe://test#a", reply: &xpb.DocumentationReply_Document{Signature: mkPr("asig"), Text: mkPr("atext")}, nodes: []returnNode{{ticket: "kythe://test#a", kind: "etc"}}},
		// Note that SlowDefinitions doesn't handle completions.
		{ticket: "kythe://test#fdecl", reply: &xpb.DocumentationReply_Document{Signature: mkPr("fsig"), Text: mkPr("ftext")}, nodes: []returnNode{{ticket: "kythe://test#fdecl", kind: "function"}}},
		{ticket: "kythe://test#fdefn", reply: &xpb.DocumentationReply_Document{Signature: mkPr("fsig"), Text: mkPr("ftext")}, nodes: []returnNode{{ticket: "kythe://test#fdefn", kind: "function", anchorText: "fdeftext"}}},
		{ticket: "kythe://test#l", reply: &xpb.DocumentationReply_Document{Signature: mkPr("lsig"), Text: mkPr("ltext", "kythe://test#l1", "kythe://test#l2")},
			nodes: []returnNode{{ticket: "kythe://test#l1", kind: "etc", anchorText: "deftext1"}, {ticket: "kythe://test#l2", kind: "etc", anchorText: "deftext2"}, {ticket: "kythe://test#l", kind: "etc"}}},
	}
	nodes := make(map[string]*cpb.NodeInfo)
	esets := make(map[string]*gpb.EdgeSet)
	xrefs := make(map[string]*xpb.CrossReferencesReply_CrossReferenceSet)
	for _, node := range db {
		nodes[node.ticket] = &cpb.NodeInfo{
			Facts: map[string][]byte{
				facts.NodeKind: []byte(node.kind),
				facts.Text:     []byte(node.text),
				facts.Format:   []byte(node.format),
			},
		}
		if node.definitionText != nil {
			set := &xpb.CrossReferencesReply_CrossReferenceSet{Ticket: node.ticket}
			for _, text := range node.definitionText {
				set.Definition = append(set.Definition, &xpb.CrossReferencesReply_RelatedAnchor{Anchor: &xpb.Anchor{Text: text, Ticket: node.ticket + "a"}})
			}
			xrefs[node.ticket] = set
		}
		set := &gpb.EdgeSet{Groups: make(map[string]*gpb.EdgeSet_Group)}
		if node.typed != "" {
			set.Groups[edges.Typed] = &gpb.EdgeSet_Group{
				Edge: []*gpb.EdgeSet_Group_Edge{&gpb.EdgeSet_Group_Edge{TargetTicket: node.typed}},
			}
		}
		if node.childof != "" {
			set.Groups[edges.ChildOf] = &gpb.EdgeSet_Group{
				Edge: []*gpb.EdgeSet_Group_Edge{&gpb.EdgeSet_Group_Edge{TargetTicket: node.childof}},
			}
		}
		if node.documented != "" {
			set.Groups[edges.Mirror(edges.Documents)] = &gpb.EdgeSet_Group{
				Edge: []*gpb.EdgeSet_Group_Edge{&gpb.EdgeSet_Group_Edge{TargetTicket: node.documented}},
			}
		}
		if node.completes != "" {
			set.Groups[edges.Completes] = &gpb.EdgeSet_Group{
				Edge: []*gpb.EdgeSet_Group_Edge{&gpb.EdgeSet_Group_Edge{TargetTicket: node.completes}},
			}
		}
		if node.completed != "" {
			set.Groups[edges.Mirror(edges.Completes)] = &gpb.EdgeSet_Group{
				Edge: []*gpb.EdgeSet_Group_Edge{&gpb.EdgeSet_Group_Edge{TargetTicket: node.completed}},
			}
		}
		if node.defines != "" {
			set.Groups[edges.DefinesBinding] = &gpb.EdgeSet_Group{
				Edge: []*gpb.EdgeSet_Group_Edge{&gpb.EdgeSet_Group_Edge{TargetTicket: node.defines}},
			}
		}
		if node.params != nil {
			var groups []*gpb.EdgeSet_Group_Edge
			for i, p := range node.params {
				groups = append(groups, &gpb.EdgeSet_Group_Edge{TargetTicket: p, Ordinal: int32(i)})
			}
			set.Groups[edges.Param] = &gpb.EdgeSet_Group{Edge: groups}
		}
		esets[node.ticket] = set
	}
	getNode := func(ticket string, facts []string) *cpb.NodeInfo {
		data, found := nodes[ticket]
		if !found {
			return nil
		}
		info := &cpb.NodeInfo{Facts: make(map[string][]byte)}
		for _, fact := range facts {
			info.Facts[fact] = data.Facts[fact]
		}
		return info
	}
	service := &mockService{
		NodesFn: func(req *gpb.NodesRequest) (*gpb.NodesReply, error) {
			reply := &gpb.NodesReply{Nodes: make(map[string]*cpb.NodeInfo)}
			for _, ticket := range req.Ticket {
				if info := getNode(ticket, req.Filter); info != nil {
					reply.Nodes[ticket] = info
				}
			}
			return reply, nil
		},
		EdgesFn: func(req *gpb.EdgesRequest) (*gpb.EdgesReply, error) {
			reply := &gpb.EdgesReply{
				EdgeSets: make(map[string]*gpb.EdgeSet),
				Nodes:    make(map[string]*cpb.NodeInfo),
			}
			for _, ticket := range req.Ticket {
				if data, found := esets[ticket]; found {
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
										if node := getNode(edge.TargetTicket, req.Filter); node != nil {
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
		},
		CrossReferencesFn: func(req *xpb.CrossReferencesRequest) (*xpb.CrossReferencesReply, error) {
			reply := &xpb.CrossReferencesReply{
				CrossReferences: make(map[string]*xpb.CrossReferencesReply_CrossReferenceSet),
			}
			if req.DefinitionKind != xpb.CrossReferencesRequest_BINDING_DEFINITIONS &&
				req.DefinitionKind != xpb.CrossReferencesRequest_ALL_DEFINITIONS ||
				req.ReferenceKind != xpb.CrossReferencesRequest_NO_REFERENCES ||
				req.DocumentationKind != xpb.CrossReferencesRequest_NO_DOCUMENTATION ||
				req.AnchorText {
				t.Fatalf("Unexpected CrossReferences request: %v", req)
			}
			for _, ticket := range req.Ticket {
				if set, found := xrefs[ticket]; found {
					reply.CrossReferences[ticket] = set
				}
			}
			return reply, nil
		},
	}
	for _, test := range tests {
		reply, err := SlowDocumentation(nil, service, &xpb.DocumentationRequest{Ticket: []string{test.ticket}, Filter: []string{facts.NodeKind}})
		if err != nil {
			t.Fatalf("SlowDocumentation error for %s: %v", test.ticket, err)
		}
		test.reply.Ticket = test.ticket
		expectedDoc := &xpb.DocumentationReply{Document: []*xpb.DocumentationReply_Document{test.reply}}
		for _, node := range test.nodes {
			anchorTicket := ""
			if node.anchorText != "" {
				if expectedDoc.DefinitionLocations == nil {
					expectedDoc.DefinitionLocations = make(map[string]*xpb.Anchor)
				}
				anchorTicket = node.ticket + "a"
				expectedDoc.DefinitionLocations[anchorTicket] = &xpb.Anchor{Ticket: anchorTicket, Text: node.anchorText}
			}
			if expectedDoc.Nodes == nil {
				expectedDoc.Nodes = make(map[string]*cpb.NodeInfo)
			}
			expectedDoc.Nodes[node.ticket] = &cpb.NodeInfo{Definition: anchorTicket, Facts: map[string][]byte{"/kythe/node/kind": []byte(node.kind)}}
		}
		if err := testutil.DeepEqual(expectedDoc, reply); err != nil {
			t.Fatal(err)
		}
	}
}
