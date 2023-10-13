/*
 * Copyright 2019 The Kythe Authors. All rights reserved.
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

package govname

import (
	"fmt"
	"go/types"

	spb "kythe.io/kythe/proto/storage_go_proto"
)

// Builtin returns the VName for a named builtin Go type.
func Builtin(name string) *spb.VName {
	return &spb.VName{Corpus: GolangCorpus, Language: Language, Signature: name + "#builtin"}
}

// BasicType returns the VName for a basic builtin Go type.
func BasicType(b *types.Basic) *spb.VName { return Builtin(b.Name()) }

// FunctionConstructorType returns the VName for the builtin Go function type constructor.
func FunctionConstructorType() *spb.VName { return Builtin("fn") }

// TupleConstructorType returns the VName for the builtin Go tuple type constructor.
func TupleConstructorType() *spb.VName { return Builtin("tuple") }

// MapConstructorType returns the VName for the builtin Go map type constructor.
func MapConstructorType() *spb.VName { return Builtin("map") }

// ArrayConstructorType returns the VName for the builtin Go array type
// constructor of a given length.
func ArrayConstructorType(length int64) *spb.VName { return Builtin(fmt.Sprintf("array%d", length)) }

// SliceConstructorType returns the VName for the builtin Go slice type constructor.
func SliceConstructorType() *spb.VName { return Builtin("slice") }

// PointerConstructorType returns the VName for the builtin Go pointer type constructor.
func PointerConstructorType() *spb.VName { return Builtin("pointer") }

// ChanConstructorType returns the VName for the builtin Go chan type
// constructor of the given direction.
func ChanConstructorType(dir types.ChanDir) *spb.VName {
	var chanType string
	switch dir {
	case types.SendOnly:
		chanType = "chan<-"
	case types.RecvOnly:
		chanType = "<-chan"
	default:
		chanType = "chan"
	}
	return Builtin(chanType)
}

// VariadicConstructorType returns the VName for the builtin Go variadic type constructor.
func VariadicConstructorType() *spb.VName { return Builtin("variadic") }
