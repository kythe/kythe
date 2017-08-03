/*
 * Copyright 2017 Google Inc. All rights reserved.
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
	"testing"

	"kythe.io/kythe/go/test/testutil"

	cpb "kythe.io/kythe/proto/common_proto"
	gpb "kythe.io/kythe/proto/graph_proto"
	xpb "kythe.io/kythe/proto/xref_proto"

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

type MockClient struct {
	decRsp []mockDec
	refRsp []mockRef
}

func (c MockClient) Decorations(_ context.Context, d *xpb.DecorationsRequest) (*xpb.DecorationsReply, error) {
	for _, r := range c.decRsp {
		fmt.Printf("%s !== %s\n", r.ticket, d.Location.Ticket)

		if r.ticket == d.Location.Ticket {
			return &r.resp, nil
		}
	}
	return nil, fmt.Errorf("No Decorations Found")
}
func (c MockClient) CrossReferences(_ context.Context, x *xpb.CrossReferencesRequest) (*xpb.CrossReferencesReply, error) {
	for _, r := range c.refRsp {
		if r.ticket == x.Ticket[0] {
			return &r.resp, nil
		}
	}
	return nil, fmt.Errorf("No CrossReferences Found")
}
func (c MockClient) Documentation(_ context.Context, x *xpb.DocumentationRequest) (*xpb.DocumentationReply, error) {
	return nil, fmt.Errorf("Not Implemented")
}
func (c MockClient) Edges(_ context.Context, x *gpb.EdgesRequest) (*gpb.EdgesReply, error) {
	return nil, fmt.Errorf("Not Implemented")
}
func (c MockClient) Nodes(_ context.Context, x *gpb.NodesRequest) (*gpb.NodesReply, error) {
	return nil, fmt.Errorf("Not Implemented")
}
func TestReferences(t *testing.T) {
	p := PathConfig{
		Root:   "/root/dir/",
		Corpus: "corpus",
	}

	c := MockClient{
		decRsp: []mockDec{
			{
				ticket: "kythe://corpus?path=file.txt",
				resp: xpb.DecorationsReply{
					SourceText: []byte("hi\nthere\nhi"),
					Reference: []*xpb.DecorationsReply_Reference{
						{
							TargetTicket: "kythe://corpus?path=file.txt?signature=hi",
							Span: &cpb.Span{
								Start: &cpb.Point{LineNumber: 1, ColumnOffset: 0},
								End:   &cpb.Point{LineNumber: 1, ColumnOffset: 3}}}}}}},
		refRsp: []mockRef{
			{
				ticket: "kythe://corpus?path=file.txt?signature=hi",
				resp: xpb.CrossReferencesReply{
					CrossReferences: map[string]*xpb.CrossReferencesReply_CrossReferenceSet{
						"kythe://corpus?path=file.txt?signature=hi": {
							Ticket: "kythe://corpus?path=file.txt?signature=hi",
							Reference: []*xpb.CrossReferencesReply_RelatedAnchor{
								{
									Anchor: &xpb.Anchor{
										Ticket: "kythe://corpus?path=file.txt?signature=hi2",
										Parent: "kythe://corpus?path=file.txt",
										Span: &cpb.Span{
											Start: &cpb.Point{LineNumber: 3, ColumnOffset: 0},
											End:   &cpb.Point{LineNumber: 3, ColumnOffset: 3}}}}}}}}}}}

	srv := NewServer(p, c)
	u := "file:///root/dir/file.txt"
	err := srv.TextDocumentDidOpen(lsp.DidOpenTextDocumentParams{
		TextDocument: lsp.TextDocumentItem{
			URI: lsp.DocumentURI(u),
		},
	})
	if err != nil {
		t.Errorf("Unexpected error opening document (%s): %v", u, err)
	}

	locs, err := srv.TextDocumentReferences(lsp.ReferenceParams{
		TextDocumentPositionParams: lsp.TextDocumentPositionParams{
			TextDocument: lsp.TextDocumentIdentifier{
				URI: lsp.DocumentURI(u),
			},
			Position: lsp.Position{
				Line:      0,
				Character: 3,
			},
		},
	})
	if err != nil {
		t.Errorf("Unexpected error finding references: %v", err)
	}

	expected := []lsp.Location{lsp.Location{
		URI: "file:///root/dir/file.txt",
		Range: lsp.Range{
			Start: lsp.Position{Line: 2, Character: 0},
			End:   lsp.Position{Line: 2, Character: 3}}}}

	if err := testutil.DeepEqual(locs, expected); err != nil {
		t.Errorf("Incorrect references returned\n  Expected: %#v\n  Found:    %#v", locs, expected)
	}
}
