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

package link

import (
	"context"
	"errors"
	"testing"

	"google.golang.org/grpc/codes"
	"google.golang.org/grpc/status"
	"google.golang.org/protobuf/proto"

	cpb "kythe.io/kythe/proto/common_go_proto"
	ipb "kythe.io/kythe/proto/identifier_go_proto"
	linkpb "kythe.io/kythe/proto/link_go_proto"
	xpb "kythe.io/kythe/proto/xref_go_proto"
)

type fakeClient struct {
	found *ipb.FindReply
	xrefs *xpb.CrossReferencesReply
}

func (f fakeClient) CrossReferences(_ context.Context, req *xpb.CrossReferencesRequest) (*xpb.CrossReferencesReply, error) {
	if f.xrefs == nil {
		return nil, errNoRefs
	}
	return f.xrefs, nil
}

func (f fakeClient) Find(_ context.Context, req *ipb.FindRequest) (*ipb.FindReply, error) {
	if f.found == nil {
		return nil, errNoID
	}
	return f.found, nil
}

var (
	errNoRefs = errors.New("no xrefs")
	errNoID   = errors.New("no identifiers")
)

func TestErrors(t *testing.T) {
	// None of these tests should make it to calling the service.
	res := &Resolver{Client: fakeClient{}}
	tests := []struct {
		req  *linkpb.LinkRequest
		want codes.Code
	}{
		// Various missing parameters should report INVALID_ARGUMENT.
		{new(linkpb.LinkRequest), codes.InvalidArgument},
		{&linkpb.LinkRequest{
			Identifier: "foo",
			Include: []*linkpb.LinkRequest_Location{{
				Path: "(", // bogus regexp
			}},
		}, codes.InvalidArgument},
		{&linkpb.LinkRequest{
			Identifier: "foo",
			Exclude: []*linkpb.LinkRequest_Location{{
				Root: "?", // bogus regexp
			}},
		}, codes.InvalidArgument},
	}
	ctx := context.Background()
	for _, test := range tests {
		_, err := res.Resolve(ctx, test.req)
		got := status.Code(err)
		if got != test.want {
			t.Errorf("Resolve %+v: got code %v, want %v [%v]", test.req, got, test.want, err)
		}
	}
}

func TestResolve(t *testing.T) {
	tests := []struct {
		req    *linkpb.LinkRequest
		client fakeClient
		check  func(*linkpb.LinkReply, error)
	}{
		{
			// If there are no ID matches, the request should fail.
			req:    &linkpb.LinkRequest{Identifier: "foo"},
			client: fakeClient{},
			check: func(rsp *linkpb.LinkReply, err error) {
				if err != errNoID {
					t.Errorf("Resolve(foo): got %v, want %v", err, errNoID)
				}
			},
		},
		{
			// If there are no cross-references, the request should fail.
			req: &linkpb.LinkRequest{Identifier: "bar"},
			client: fakeClient{
				found: &ipb.FindReply{
					Matches: []*ipb.FindReply_Match{{Ticket: "T"}},
				},
			},
			check: func(rsp *linkpb.LinkReply, err error) {
				if err != errNoRefs {
					t.Errorf("Resolve(bar): got %v, want %v", err, errNoRefs)
				}
			},
		},
		{
			// If there are cross-references, we should get a result.
			req: &linkpb.LinkRequest{Identifier: "baz"},
			client: fakeClient{
				found: &ipb.FindReply{
					Matches: []*ipb.FindReply_Match{
						{Ticket: "kythe://test?lang=qq#X"},
						{Ticket: "kythe://test?lang=rr#Y"},
					},
				},
				xrefs: &xpb.CrossReferencesReply{
					CrossReferences: map[string]*xpb.CrossReferencesReply_CrossReferenceSet{
						"kythe://test?lang=qq#X": {
							Ticket: "kythe://test?lang=qq#X",
							Definition: []*xpb.CrossReferencesReply_RelatedAnchor{{
								Anchor: &xpb.Anchor{
									Ticket: "kythe://test?lang=qq?path=foo#blah",
									Kind:   "/kythe/edge/defines/binding",
									Parent: "kythe://test?path=foo",
									Span: &cpb.Span{
										Start: &cpb.Point{LineNumber: 10},
										End:   &cpb.Point{LineNumber: 11},
									},
								},
							}},
						},
					},
				},
			},
			check: func(rsp *linkpb.LinkReply, err error) {
				if err != nil {
					t.Errorf("Resolve(baz): unexpected error: %v", err)
					return
				}
				want := &linkpb.LinkReply{
					Links: []*linkpb.Link{{
						FileTicket: "kythe://test?path=foo",
						Span: &cpb.Span{
							Start: &cpb.Point{LineNumber: 10},
							End:   &cpb.Point{LineNumber: 11},
						},
					}},
				}
				if !proto.Equal(rsp, want) {
					t.Errorf("Resolve(baz):\ngot  %+v\nwant %+v", rsp, want)
				}
			},
		},
		{
			// If there are no suitable definitions, we should get no result.
			req: &linkpb.LinkRequest{Identifier: "frob"},
			client: fakeClient{
				found: &ipb.FindReply{
					Matches: []*ipb.FindReply_Match{{Ticket: "kythe://test?lang=qq#X"}},
				},
				xrefs: &xpb.CrossReferencesReply{
					CrossReferences: map[string]*xpb.CrossReferencesReply_CrossReferenceSet{
						"kythe://test?lang=qq#X": {
							Ticket: "kythe://test?lang=qq#X",
							Reference: []*xpb.CrossReferencesReply_RelatedAnchor{{
								Anchor: new(xpb.Anchor),
							}},
						},
					},
				},
			},
			check: func(rsp *linkpb.LinkReply, err error) {
				if err == nil {
					t.Errorf("Resolve(frob): expected error, but got %+v", rsp)
				}
			},
		},
	}
	ctx := context.Background()
	for _, test := range tests {
		res := &Resolver{Client: test.client}
		test.check(res.Resolve(ctx, test.req))
	}
}
