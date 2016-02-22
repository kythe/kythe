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
	"reflect"
	"regexp"
	"testing"

	"kythe.io/kythe/go/test/testutil"
	"kythe.io/kythe/go/util/schema"

	"golang.org/x/net/context"

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
		{schema.NodeKindFact, schema.NodeKindFact},
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
	NodesFn           func(*xpb.NodesRequest) (*xpb.NodesReply, error)
	EdgesFn           func(*xpb.EdgesRequest) (*xpb.EdgesReply, error)
	DecorationsFn     func(*xpb.DecorationsRequest) (*xpb.DecorationsReply, error)
	CrossReferencesFn func(*xpb.CrossReferencesRequest) (*xpb.CrossReferencesReply, error)
	CallersFn         func(*xpb.CallersRequest) (*xpb.CallersReply, error)
}

func (s *mockService) Nodes(ctx context.Context, req *xpb.NodesRequest) (*xpb.NodesReply, error) {
	return s.NodesFn(req)
}

func (s *mockService) Edges(ctx context.Context, req *xpb.EdgesRequest) (*xpb.EdgesReply, error) {
	return s.EdgesFn(req)
}

func (s *mockService) Decorations(ctx context.Context, req *xpb.DecorationsRequest) (*xpb.DecorationsReply, error) {
	return s.DecorationsFn(req)
}

func (s *mockService) CrossReferences(ctx context.Context, req *xpb.CrossReferencesRequest) (*xpb.CrossReferencesReply, error) {
	return s.CrossReferencesFn(req)
}

func (s *mockService) Callers(ctx context.Context, req *xpb.CallersRequest) (*xpb.CallersReply, error) {
	return s.CallersFn(req)
}

func containsString(arr []string, key string) bool {
	for _, i := range arr {
		if i == key {
			return true
		}
	}
	return false
}

func TestSlowCallers(t *testing.T) {
	service := &mockService{
		NodesFn: func(req *xpb.NodesRequest) (*xpb.NodesReply, error) {
			t.Fatalf("Unexpected Nodes request: %v", req)
			return nil, nil
		},
		EdgesFn: func(req *xpb.EdgesRequest) (*xpb.EdgesReply, error) {
			if len(req.Ticket) == 1 && req.Ticket[0] == "kythe://test#f" {
				if containsString(req.Kind, schema.CallableAsEdge) {
					return &xpb.EdgesReply{
						EdgeSet: []*xpb.EdgeSet{&xpb.EdgeSet{
							SourceTicket: req.Ticket[0],
							Group: []*xpb.EdgeSet_Group{&xpb.EdgeSet_Group{
								TargetTicket: []string{"kythe://test#c"},
							}},
						}},
					}, nil
				}
				return &xpb.EdgesReply{}, nil
			} else if len(req.Ticket) == 1 && req.Ticket[0] == "kythe://test#acall" {
				if containsString(req.Kind, schema.ChildOfEdge) {
					return &xpb.EdgesReply{
						EdgeSet: []*xpb.EdgeSet{&xpb.EdgeSet{
							SourceTicket: req.Ticket[0],
							Group: []*xpb.EdgeSet_Group{&xpb.EdgeSet_Group{
								TargetTicket: []string{"kythe://test#g"},
							}},
						}},
					}, nil
				}
			}
			t.Fatalf("Unexpected Edges request: %v", req)
			return nil, nil
		},
		DecorationsFn: func(req *xpb.DecorationsRequest) (*xpb.DecorationsReply, error) {
			t.Fatalf("Unexpected Decorations request: %v", req)
			return nil, nil
		},
		CrossReferencesFn: func(req *xpb.CrossReferencesRequest) (*xpb.CrossReferencesReply, error) {
			if len(req.Ticket) == 1 && req.Ticket[0] == "kythe://test#c" {
				if req.ReferenceKind == xpb.CrossReferencesRequest_ALL_REFERENCES {
					return &xpb.CrossReferencesReply{
						CrossReferences: map[string]*xpb.CrossReferencesReply_CrossReferenceSet{
							req.Ticket[0]: &xpb.CrossReferencesReply_CrossReferenceSet{
								Ticket: req.Ticket[0],
								Reference: []*xpb.Anchor{
									&xpb.Anchor{
										Ticket: "kythe://test#acall",
										Kind:   schema.RefCallEdge,
									},
								},
							},
						},
					}, nil
				}
			} else if len(req.Ticket) == 2 && containsString(req.Ticket, "kythe://test#f") && containsString(req.Ticket, "kythe://test#g") {
				if req.DefinitionKind == xpb.CrossReferencesRequest_BINDING_DEFINITIONS {
					return &xpb.CrossReferencesReply{
						CrossReferences: map[string]*xpb.CrossReferencesReply_CrossReferenceSet{
							"kythe://test#f": &xpb.CrossReferencesReply_CrossReferenceSet{
								Ticket: "kythe://test#f",
								Definition: []*xpb.Anchor{
									&xpb.Anchor{
										Ticket: "kythe://test#afdef",
										Text:   "f",
									},
								},
							},
							"kythe://test#g": &xpb.CrossReferencesReply_CrossReferenceSet{
								Ticket: "kythe://test#g",
								Definition: []*xpb.Anchor{
									&xpb.Anchor{
										Ticket: "kythe://test#agdef",
										Text:   "g",
									},
								},
							},
						},
					}, nil
				}
			}
			t.Fatalf("Unexpected CrossReferences request: %v", req)
			return nil, nil
		},
		CallersFn: func(req *xpb.CallersRequest) (*xpb.CallersReply, error) {
			t.Fatalf("Unexpected Callers request: %v", req)
			return nil, nil
		},
	}
	creq := &xpb.CallersRequest{
		IncludeOverrides: true,
		SemanticObject:   []string{"kythe://test#f"},
	}
	creply, err := SlowCallers(nil, service, creq)
	if err != nil {
		t.Fatalf("SlowCallers error: %v", err)
	}
	if creply == nil {
		t.Fatalf("SlowCallers returned nil response")
	}
	expected := &xpb.CallersReply{
		Callee: []*xpb.CallersReply_CallableDetail{
			&xpb.CallersReply_CallableDetail{
				SemanticObject:         "kythe://test#f",
				SemanticObjectCallable: "kythe://test#c",
				Definition: &xpb.Anchor{
					Ticket: "kythe://test#afdef",
					Text:   "f",
				},
				Identifier:  "f",
				DisplayName: "f",
			},
		},
		Caller: []*xpb.CallersReply_Caller{
			&xpb.CallersReply_Caller{
				Detail: &xpb.CallersReply_CallableDetail{
					SemanticObject: "kythe://test#g",
					Definition: &xpb.Anchor{
						Ticket: "kythe://test#agdef",
						Text:   "g",
					},
					Identifier:  "g",
					DisplayName: "g",
				},
				CallSite: []*xpb.CallersReply_Caller_CallSite{
					&xpb.CallersReply_Caller_CallSite{
						Anchor: &xpb.Anchor{
							Ticket: "kythe://test#acall",
							Kind:   "/kythe/edge/ref/call",
						},
					},
				},
			},
		},
	}
	if err := testutil.DeepEqual(expected, creply); err != nil {
		t.Fatal(err)
	}
}
