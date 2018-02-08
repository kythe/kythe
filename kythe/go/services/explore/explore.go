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

// Package explore defines the ExploreService interface.
package explore

import (
	"context"

	epb "kythe.io/kythe/proto/explore_go_proto"
)

// Service defines the interface for the ExploreService defined in kythe/proto/explore.proto.
type Service interface {
	// Returns the hierarchy (supertypes and subtypes, including implementations)
	// of a specified type, as a directed acyclic graph.
	// NOT YET IMPLEMENTED
	TypeHierarchy(context.Context, *epb.TypeHierarchyRequest) (*epb.TypeHierarchyReply, error)

	// Returns the (recursive) callers of a specified function, as a directed
	// graph.
	// The Callers/Callees functions are distinct from XrefService.CrossReferences
	// in that these functions capture the semantic relationships between methods,
	// rather than the locations in the code base where a method is called.
	// NOT YET IMPLEMENTED
	Callers(context.Context, *epb.CallersRequest) (*epb.CallersReply, error)

	// Returns the (recursive) callees of a specified function (that is, what
	// functions this function calls), as a directed graph.
	// NOT YET IMPLEMENTED
	Callees(context.Context, *epb.CalleesRequest) (*epb.CalleesReply, error)

	// Returns the parameters of a specified function.
	// NOT YET IMPLEMENTED
	Parameters(context.Context, *epb.ParametersRequest) (*epb.ParametersReply, error)

	// Returns the container(s) in which a specified node is found (for example,
	// the file for a class, or the class for a function).
	// Note that in some cases a node may have more than one container.
	// NOT YET IMPLEMENTED
	Containers(context.Context, *epb.ContainersRequest) (*epb.ContainersReply, error)

	// Returns the contents of a given container node (for example, the classes
	// contained in a file, or the functions contained in a class).
	// NOT YET IMPLEMENTED
	Contents(context.Context, *epb.ContentsRequest) (*epb.ContentsReply, error)
}
