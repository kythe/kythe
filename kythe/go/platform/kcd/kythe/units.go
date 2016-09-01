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

// Package kythe implements the kcd.Unit interface for Kythe compilations.
package kythe

import (
	"encoding/json"

	"kythe.io/kythe/go/platform/kcd"

	apb "kythe.io/kythe/proto/analysis_proto"
	spb "kythe.io/kythe/proto/storage_proto"

	"github.com/golang/protobuf/proto"
)

// Format is the format key used to denote Kythe compilations, stored
// as kythe.proto.CompilationUnit messages.
const Format = "kythe"

// Unit implements the kcd.Unit interface for Kythe compilations.
type Unit struct{ Proto *apb.CompilationUnit }

// MarshalBinary satisfies the encoding.BinaryMarshaler interface.
func (u Unit) MarshalBinary() ([]byte, error) { return proto.Marshal(u.Proto) }

// MarshalJSON satisfies the json.Marshaler interface.
func (u Unit) MarshalJSON() ([]byte, error) { return json.Marshal(u.Proto) }

// Index satisfies part of the kcd.Unit interface.
func (u Unit) Index() kcd.Index {
	v := u.Proto.GetVName()
	if v == nil {
		v = new(spb.VName)
	}
	idx := kcd.Index{
		Language: v.Language,
		Output:   u.Proto.OutputKey,
		Sources:  u.Proto.SourceFile,
		Target:   v.Signature,
	}
	for _, ri := range u.Proto.RequiredInput {
		if info := ri.Info; info != nil {
			idx.Inputs = append(idx.Inputs, info.Digest)
		}
	}
	return idx
}

// ConvertUnit reports whether v can be converted to a Kythe kcd.Unit, and if
// so returns the appropriate implementation.
func ConvertUnit(v interface{}) (kcd.Unit, bool) {
	if u, ok := v.(*apb.CompilationUnit); ok {
		return Unit{u}, true
	}
	return nil, false
}
