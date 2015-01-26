/*
 * Copyright 2014 Google Inc. All rights reserved.
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

// Package stringset contains a simple set implementation.
package stringset

import "sort"

// A Set is a compact set of strings.
type Set map[string]struct{}

// New returns a new Set containing the given strings.
func New(init ...string) Set {
	set := make(Set)
	set.Add(init...)
	return set
}

// Slice returns a slice of the strings contained in s.  The slice is ordered
// lexicographically.
func (s Set) Slice() []string {
	res := make([]string, 0, len(s))
	for str := range s {
		res = append(res, str)
	}
	sort.Strings(res)
	return res
}

// Add inserts each element of strs into s.
func (s Set) Add(strs ...string) {
	for _, str := range strs {
		s[str] = struct{}{}
	}
}

// Remove removes str from s, if it is present.
func (s Set) Remove(str string) {
	delete(s, str)
}

// Contains reports whether str is a member of s.
func (s Set) Contains(str string) bool {
	_, ok := s[str]
	return ok
}
