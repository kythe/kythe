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

// Package keys implements orderedcode encodings for Kythe types.
package keys

import (
	"github.com/google/orderedcode"

	spb "kythe.io/kythe/proto/storage_go_proto"
)

// Append appends the encoded representations of items to the key buffer.
//
// In addition to the types supported by the orderedcode library, the following
// Kythe types can be handled:
//
//  - *spb.VName
//  - int32/int
//
// More detail at: https://godoc.org/github.com/google/orderedcode#Append
func Append(key []byte, items ...interface{}) ([]byte, error) {
	expanded := make([]interface{}, 0, len(items))
	for _, item := range items {
		switch x := item.(type) {
		case *spb.VName:
			expanded = append(expanded, x.Corpus, x.Language, x.Path, x.Root, x.Signature)
		case int32:
			expanded = append(expanded, int64(x))
		case int:
			expanded = append(expanded, int64(x))
		default:
			expanded = append(expanded, x)
		}
	}
	return orderedcode.Append(key, expanded...)
}

// Parse parses the next len(items) from the encoded key.
//
// In addition to the types supported by the orderedcode library, the following
// Kythe types can be handled:
//
//  - *spb.VName
//  - int32/int
//
// More detail at: https://godoc.org/github.com/google/orderedcode#Parse
func Parse(key string, items ...interface{}) (remaining string, err error) {
	expanded := make([]interface{}, 0, len(items))
	for _, item := range items {
		switch x := item.(type) {
		case *spb.VName:
			expanded = append(expanded, &x.Corpus, &x.Language, &x.Path, &x.Root, &x.Signature)
		case *int32:
			var n int64
			defer func() { *x = int32(n) }()
			expanded = append(expanded, &n)
		case *int:
			var n int64
			defer func() { *x = int(n) }()
			expanded = append(expanded, &n)
		default:
			expanded = append(expanded, x)
		}
	}
	return orderedcode.Parse(key, expanded...)
}
