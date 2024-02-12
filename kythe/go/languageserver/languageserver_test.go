/*
 * Copyright 2017 The Kythe Authors. All rights reserved.
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
package languageserver

import (
	"context"
	"fmt"
	"reflect"
	"testing"

	"kythe.io/kythe/go/test/testutil"

	cpb "kythe.io/kythe/proto/common_go_proto"
	gpb "kythe.io/kythe/proto/graph_go_proto"
	xpb "kythe.io/kythe/proto/xref_go_proto"

	"github.com/sourcegraph/go-langserver/pkg/lsp"
)

type mockDec struct {
	ticket string
	resp   xpb.DecorationsReply
}
type mockRef struct {
	ticket string
	resp   xpb.CrossReferencesReply
}
type mockDoc struct {
	ticket string
	resp   xpb.DocumentationReply
}
type MockClient struct {
	decRsp []mockDec
	refRsp []mockRef
	docRsp []mockDoc
}

func (MockClient) Close(context.Context) error { return nil }

func (c MockClient) Decorations(_ context.Context, d *xpb.DecorationsRequest) (*xpb.DecorationsReply, error) {
	for _, r := range c.decRsp {
		if r.ticket == d.Location.Ticket {
			return &r.resp, nil
		}
	}
	return nil, fmt.Errorf("no Decorations Found")
}
func (c MockClient) CrossReferences(_ context.Context, x *xpb.CrossReferencesRequest) (*xpb.CrossReferencesReply, error) {
	for _, r := range c.refRsp {
		if r.ticket == x.Ticket[0] {
			return &r.resp, nil
		}
	}
	return nil, fmt.Errorf("no CrossReferences Found")
}
func (c MockClient) Documentation(_ context.Context, x *xpb.DocumentationRequest) (*xpb.DocumentationReply, error) {
	for _, r := range c.docRsp {
		if r.ticket == x.Ticket[0] {
			return &r.resp, nil
		}
	}
	return nil, fmt.Errorf("no CrossReferences Found")
}
func (c MockClient) Edges(_ context.Context, x *gpb.EdgesRequest) (*gpb.EdgesReply, error) {
	return nil, fmt.Errorf("not Implemented")
}
func (c MockClient) Nodes(_ context.Context, x *gpb.NodesRequest) (*gpb.NodesReply, error) {
	return nil, fmt.Errorf("not Implemented")
}
func TestReferences(t *testing.T) {
	const sourceText = "hi\nthere\nhi\nend"
	c := MockClient{
		decRsp: []mockDec{
			{
				ticket: "kythe://corpus?path=file.txt",
				resp: xpb.DecorationsReply{
					SourceText: []byte(sourceText),
					DefinitionLocations: map[string]*xpb.Anchor{
						"kythe://corpus?path=file.txt#def": {
							Ticket: "kythe://corpus?path=file.txt#def",
							Parent: "kythe://corpus?path=file.txt",
							Span: &cpb.Span{
								Start: &cpb.Point{LineNumber: 3, ColumnOffset: 0},
								End:   &cpb.Point{LineNumber: 3, ColumnOffset: 2}}},
						"kythe://corpus?path=file.txt#alt": {
							Ticket: "kythe://corpus?path=file.txt#alt",
							Parent: "kythe://corpus?path=file.txt",
							Span: &cpb.Span{
								Start: &cpb.Point{LineNumber: 4}}},
					},
					Reference: []*xpb.DecorationsReply_Reference{
						{
							TargetDefinition: "kythe://corpus?path=file.txt#def",
							TargetTicket:     "kythe://corpus?path=file.txt#hi",
							Span: &cpb.Span{
								Start: &cpb.Point{LineNumber: 1, ColumnOffset: 0},
								End:   &cpb.Point{LineNumber: 1, ColumnOffset: 2}},
						},
						{
							TargetDefinition: "kythe://corpus?path=file.txt#def",
							TargetTicket:     "kythe://corpus?path=file.txt#hi",
							Span: &cpb.Span{
								Start: &cpb.Point{LineNumber: 3, ColumnOffset: 0},
								End:   &cpb.Point{LineNumber: 3, ColumnOffset: 2}},
						},
						{
							TargetDefinition: "kythe://corpus?path=file.txt#end",
							TargetTicket:     "kythe://corpus?path=file.txt#alt",
							Span: &cpb.Span{
								Start: &cpb.Point{LineNumber: 4}},
						},
					}}}},
		refRsp: []mockRef{
			{
				ticket: "kythe://corpus?path=file.txt#hi",
				resp: xpb.CrossReferencesReply{
					CrossReferences: map[string]*xpb.CrossReferencesReply_CrossReferenceSet{
						"kythe://corpus?path=file.txt#hi": {
							Ticket: "kythe://corpus?path=file.txt#hi",
							Reference: []*xpb.CrossReferencesReply_RelatedAnchor{
								{
									Anchor: &xpb.Anchor{
										Ticket: "kythe://corpus?path=file.txt#hi2",
										Parent: "kythe://corpus?path=file.txt",
										Span: &cpb.Span{
											Start: &cpb.Point{LineNumber: 3, ColumnOffset: 0},
											End:   &cpb.Point{LineNumber: 3, ColumnOffset: 2},
										},
									},
								},
								{
									Anchor: &xpb.Anchor{
										Ticket: "kythe://corpus?path=file.txt#end",
										Parent: "kythe://corpus?path=file.txt",
										Span: &cpb.Span{
											Start: &cpb.Point{LineNumber: 4},
										},
									},
								},
							}}}}}},
		docRsp: []mockDoc{{
			ticket: "kythe://corpus?path=file.txt#hi",
			resp: xpb.DocumentationReply{
				Document: []*xpb.DocumentationReply_Document{{
					Ticket: "kythe://corpus?path=file.txt#hi",
					Text: &xpb.Printable{
						RawText: `a[b]c\[d\]e\\f`,
					},
					MarkedSource: &cpb.MarkedSource{
						PreText:  "<",
						PostText: ">",
						Child: []*cpb.MarkedSource{{
							PreText: "hi",
						}}}}}}}}}

	srv := NewServer(c, &Options{
		NewWorkspace: func(_ lsp.DocumentURI) (Workspace, error) {
			return NewSettingsWorkspace(Settings{
				Root: "/root/dir/",
				Mappings: []MappingConfig{{
					Local: ":path*",
					VName: VNameConfig{
						Path:   ":path*",
						Corpus: "corpus",
					}},
				},
			})
		},
	})

	srv.Initialize(lsp.InitializeParams{})
	u := "file:///root/dir/file.txt"
	err := srv.TextDocumentDidOpen(lsp.DidOpenTextDocumentParams{
		TextDocument: lsp.TextDocumentItem{
			URI:  lsp.DocumentURI(u),
			Text: sourceText,
		},
	})
	if err != nil {
		t.Errorf("Unexpected error opening document (%s): %v", u, err)
	}

	// Make some changes to the file to ensure that Kythe LS store changes correctly.
	err = srv.TextDocumentDidChange(lsp.DidChangeTextDocumentParams{
		TextDocument: lsp.VersionedTextDocumentIdentifier{
			TextDocumentIdentifier: lsp.TextDocumentIdentifier{
				URI: lsp.DocumentURI(u),
			},
		},
		ContentChanges: []lsp.TextDocumentContentChangeEvent{{
			Text: "\n\n\n\n" + sourceText,
		}},
	})
	if err != nil {
		t.Errorf("Unexpected error saving changes to document (%s): %v", u, err)
	}

	locs, err := srv.TextDocumentReferences(lsp.ReferenceParams{
		TextDocumentPositionParams: lsp.TextDocumentPositionParams{
			TextDocument: lsp.TextDocumentIdentifier{
				URI: lsp.DocumentURI(u),
			},
			Position: lsp.Position{
				Line:      4,
				Character: 2,
			},
		},
	})
	if err != nil {
		t.Errorf("Unexpected error finding references: %v", err)
	}

	expected := []lsp.Location{{
		URI: "file:///root/dir/file.txt",
		Range: lsp.Range{
			Start: lsp.Position{Line: 6, Character: 0},
			End:   lsp.Position{Line: 6, Character: 2},
		},
	}, {
		URI: "file:///root/dir/file.txt",
		Range: lsp.Range{
			Start: lsp.Position{Line: 7},
			End:   lsp.Position{Line: 7},
		},
	}}

	if err := testutil.DeepEqual(locs, expected); err != nil {
		t.Errorf("Incorrect references returned\n  Expected: %#v\n  Found:    %#v", expected, locs)
	}

	defs, err := srv.TextDocumentDefinition(lsp.TextDocumentPositionParams{
		TextDocument: lsp.TextDocumentIdentifier{
			URI: lsp.DocumentURI(u),
		},
		Position: lsp.Position{
			Line:      4,
			Character: 2,
		},
	})
	if err != nil {
		t.Error(err)
	}

	expected = []lsp.Location{{
		URI: "file:///root/dir/file.txt",
		Range: lsp.Range{
			Start: lsp.Position{Line: 6, Character: 0},
			End:   lsp.Position{Line: 6, Character: 2},
		},
	}}

	if err := testutil.DeepEqual(defs, expected); err != nil {
		t.Errorf("Incorrect definitions returned\n  Expected: %#v\n  Found:    %#v", expected, defs)
	}

	hover, err := srv.TextDocumentHover(lsp.TextDocumentPositionParams{
		TextDocument: lsp.TextDocumentIdentifier{
			URI: lsp.DocumentURI(u),
		},
		Position: lsp.Position{
			Line:      4,
			Character: 2,
		},
	})

	if err != nil {
		t.Errorf("Unexpect error fetching hover: %v", err)
	}

	hovExpected := lsp.Hover{
		Contents: []lsp.MarkedString{{
			Value: "<hi>",
		}, {
			Value: "abc[d]e\\f",
		}},
		Range: &lsp.Range{
			Start: lsp.Position{Line: 4, Character: 0},
			End:   lsp.Position{Line: 4, Character: 2},
		},
	}

	if !reflect.DeepEqual(hovExpected, hover) {
		t.Errorf("Hover results:\ngot  %+v\nwant %+v", hovExpected, hover)
	}
}
