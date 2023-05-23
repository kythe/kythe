/*
 * Copyright 2015 The Kythe Authors. All rights reserved.
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

// Package sortutil implements utilities for sorting.
package sortutil // import "kythe.io/kythe/go/util/sortutil"

import "sort"

// Lesser is an interface to a comparison function.
type Lesser interface {
	// Less returns true if a < b.
	Less(a, b any) bool
}

// Sort uses l to sort the given slice.
func Sort(l Lesser, a []any) {
	sort.Sort(&ByLesser{
		Lesser: l,
		Slice:  a,
	})
}

// ByLesser implements the heap.Interface using a Lesser over a slice.
type ByLesser struct {
	Lesser Lesser
	Slice  []any
}

// Clear removes all elements from the underlying slice.
func (s *ByLesser) Clear() { s.Slice = nil }

// Len implements part of the sort.Interface
func (s ByLesser) Len() int { return len(s.Slice) }

// Swap implements part of the sort.Interface
func (s ByLesser) Swap(i, j int) { s.Slice[i], s.Slice[j] = s.Slice[j], s.Slice[i] }

// Less implements part of the sort.Interface
func (s ByLesser) Less(i, j int) bool { return s.Lesser.Less(s.Slice[i], s.Slice[j]) }

// Push implements part of the heap.Interface
func (s *ByLesser) Push(v any) { s.Slice = append(s.Slice, v) }

// Pop implements part of the heap.Interface
func (s *ByLesser) Pop() any {
	n := len(s.Slice) - 1
	out := s.Slice[n]
	s.Slice = s.Slice[:n]
	return out
}

// Peek returns the least element in the heap.  nil is returned if the heap is
// empty.
func (s ByLesser) Peek() any {
	if len(s.Slice) == 0 {
		return nil
	}
	return s.Slice[0]
}

// LesserFunc implements the Lesser interface using a function.
type LesserFunc func(a, b any) bool

// Less implements the Lesser interface.
func (f LesserFunc) Less(a, b any) bool { return f(a, b) }
