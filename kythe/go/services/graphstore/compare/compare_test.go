/*
 * Copyright 2015 Google Inc. All rights reserved.
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

package compare

import (
	"fmt"
	"testing"

	spb "kythe.io/kythe/proto/storage_proto"
)

func TestCompareVNames(t *testing.T) {
	var ordered []*spb.VName
	for i := 0; i < 100000; i += 307 {
		key := fmt.Sprintf("%05d", i)
		ordered = append(ordered, &spb.VName{
			Signature: key[0:1],
			Corpus:    key[1:2],
			Root:      key[2:3],
			Path:      key[3:4],
			Language:  key[4:5],
		})
	}

	for i, fst := range ordered {
		for j, snd := range ordered {
			want := LT
			if i == j {
				want = EQ
			} else if i > j {
				want = GT
			}

			got := VNames(fst, snd)
			if got != want {
				t.Errorf("Comparison failed: got %v, want %v\n\tlhs %+v\n\trhs %+v",
					got, want, fst, snd)
			}
		}
	}
}

func TestCompareEntries(t *testing.T) {
	a := "a"
	b := "b"
	e := "eq"
	na := &spb.VName{Signature: a}
	nb := &spb.VName{Signature: b}
	ne := &spb.VName{Signature: e}
	root := "/"
	fa := "/a"
	fb := "/b"
	fe := "/eq"
	va := []byte("a")
	vb := []byte("b")
	ve := []byte("eq")

	// A sequence of entries in increasing order.
	ordered := []*spb.Entry{
		{FactValue: va},
		{FactValue: vb},
		{FactValue: ve},
		{FactName: root, Target: na, FactValue: vb},
		{FactName: root, Target: nb, FactValue: va},
		{FactName: root, Target: ne},
		{FactName: fa, FactValue: vb},
		{FactName: fb, FactValue: va},
		{FactName: fe},
		{EdgeKind: a, FactName: root, FactValue: vb},
		{EdgeKind: b, FactName: root, FactValue: va},
		{EdgeKind: e, FactName: root},
		{Source: na, FactName: root, FactValue: vb},
		{Source: nb, FactName: root, FactValue: va},
		{Source: ne, FactName: root},
	}

	for i, fst := range ordered {
		for j, snd := range ordered {
			want := LT
			if i == j {
				want = EQ
			} else if i > j {
				want = GT
			}

			got := ValueEntries(fst, snd)
			if got != want {
				t.Errorf("Comparison failed: got %v, want %v\n\tlhs %+v\n\trhs %+v",
					got, want, fst, snd)
			}
		}
	}
}
