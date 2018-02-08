/*
 * Copyright 2018 Google Inc. All rights reserved.
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

// Package explore provides an implementation of the ExploreService defined in kythe/proto/explore.proto.
package explore

import (
	"context"

	epb "kythe.io/kythe/proto/explore_go_proto"
)

// Service is a struct for binding together the methods implementing the ExploreService.
type Service struct{}

// TODO: implement the methods of Service (signatures below)

// TypeHierarchy returns the hierarchy (supertypes and subtypes, including implementations)
// of a specified type, as a directed acyclic graph.
func (s *Service) TypeHierarchy(ctx context.Context, req *epb.TypeHierarchyRequest) (*epb.TypeHierarchyReply, error) {
	return nil, nil
}

// Callers returns the (recursive) callers of a specified function, as a directed graph.
func (s *Service) Callers(ctx context.Context, req *epb.CallersRequest) (*epb.CallersReply, error) {
	return nil, nil
}

// Callees returns the (recursive) callees of a specified function
// (that is, what functions this function calls), as a directed graph.
func (s *Service) Callees(ctx context.Context, req *epb.CalleesRequest) (*epb.CalleesReply, error) {
	return nil, nil
}

// Parameters returns the parameters of a specified function.
func (s *Service) Parameters(ctx context.Context, req *epb.ParametersRequest) (*epb.ParametersReply, error) {
	return nil, nil
}

// Containers returns the container(s) in which a specified node is found
// (for example, the file for a class, or the class for a function).
// Note: in some cases a node may have more than one container.
func (s *Service) Containers(ctx context.Context, req *epb.ContainersRequest) (*epb.ContainersReply, error) {
	return nil, nil
}

// Contents returns the contents of a given container node
// (for example, the classes contained in a file, or the functions contained in a class).
func (s *Service) Contents(ctx context.Context, req *epb.ContentsRequest) (*epb.ContentsReply, error) {
	return nil, nil
}
