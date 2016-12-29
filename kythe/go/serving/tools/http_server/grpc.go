/*
 * Copyright 2016 Google Inc. All rights reserved.
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

package main

import (
	"kythe.io/kythe/go/services/filetree"
	"kythe.io/kythe/go/services/xrefs"

	netcontext "golang.org/x/net/context"

	ftpb "kythe.io/kythe/proto/filetree_proto"
	gpb "kythe.io/kythe/proto/graph_proto"
	xpb "kythe.io/kythe/proto/xref_proto"
)

type grpcXRefServiceServer struct{ xrefs.Service }

func (s grpcXRefServiceServer) CrossReferences(ctx netcontext.Context, req *xpb.CrossReferencesRequest) (*xpb.CrossReferencesReply, error) {
	return s.Service.CrossReferences(ctx, req)
}
func (s grpcXRefServiceServer) Decorations(ctx netcontext.Context, req *xpb.DecorationsRequest) (*xpb.DecorationsReply, error) {
	return s.Service.Decorations(ctx, req)
}
func (s grpcXRefServiceServer) Documentation(ctx netcontext.Context, req *xpb.DocumentationRequest) (*xpb.DocumentationReply, error) {
	return s.Service.Documentation(ctx, req)
}

type grpcGraphServiceServer struct{ xrefs.Service }

func (s grpcGraphServiceServer) Nodes(ctx netcontext.Context, req *gpb.NodesRequest) (*gpb.NodesReply, error) {
	return s.Service.Nodes(ctx, req)
}
func (s grpcGraphServiceServer) Edges(ctx netcontext.Context, req *gpb.EdgesRequest) (*gpb.EdgesReply, error) {
	return s.Service.Edges(ctx, req)
}

type grpcFileTreeServiceServer struct{ filetree.Service }

func (s grpcFileTreeServiceServer) CorpusRoots(ctx netcontext.Context, req *ftpb.CorpusRootsRequest) (*ftpb.CorpusRootsReply, error) {
	return s.Service.CorpusRoots(ctx, req)
}
func (s grpcFileTreeServiceServer) Directory(ctx netcontext.Context, req *ftpb.DirectoryRequest) (*ftpb.DirectoryReply, error) {
	return s.Service.Directory(ctx, req)
}
