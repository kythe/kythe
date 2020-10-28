/*
 * Copyright 2016 The Kythe Authors. All rights reserved.
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

// Package ptypes is a thin wrapper around the golang.org/protobuf/ptypes
// package that adds support for Kythe message types, and handles some type
// format conversions.
package ptypes // import "kythe.io/kythe/go/util/ptypes"

import (
	"sort"
	"strings"

	"github.com/golang/protobuf/proto"
	"github.com/golang/protobuf/ptypes"

	anypb "github.com/golang/protobuf/ptypes/any"
)

// Any is an alias for the protocol buffer Any message type.
type Any = anypb.Any

// MarshalAny converts pb to a google.protobuf.Any message, fixing the URLs of
// Kythe protobuf types as needed.
func MarshalAny(pb proto.Message) (*anypb.Any, error) {
	// The ptypes package vendors generated code for the Any type, so we have
	// to convert the type. The pointers are convertible, but since we need to
	// do surgery on the URL anyway, we just construct the output separately.
	internalAny, err := ptypes.MarshalAny(pb)
	if err != nil {
		return nil, err
	}

	// Fix up messages in the Kythe namespace.
	url := internalAny.TypeUrl
	if name, _ := ptypes.AnyMessageName(internalAny); strings.HasPrefix(name, "kythe.") {
		url = "kythe.io/proto/" + name
	}
	return &anypb.Any{
		TypeUrl: url,
		Value:   internalAny.Value,
	}, nil
}

// UnmarshalAny unmarshals a google.protobuf.Any message into pb.
// This is an alias for ptypes.UnmarshalAny to save an import.
func UnmarshalAny(any *anypb.Any, pb proto.Message) error {
	return ptypes.UnmarshalAny(any, pb)
}

// SortByTypeURL orders a slice of Any messages by their type URL, modifying
// the argument slice in-place.
func SortByTypeURL(msgs []*anypb.Any) { sort.Sort(byTypeURL(msgs)) }

type byTypeURL []*anypb.Any

func (b byTypeURL) Len() int           { return len(b) }
func (b byTypeURL) Less(i, j int) bool { return b[i].TypeUrl < b[j].TypeUrl }
func (b byTypeURL) Swap(i, j int)      { b[i], b[j] = b[j], b[i] }
