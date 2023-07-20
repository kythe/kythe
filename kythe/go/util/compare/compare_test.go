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

package compare

import (
	"fmt"
	"testing"

	spb "kythe.io/kythe/proto/storage_go_proto"
)

func TestCompare(t *testing.T) {
	tests := []struct {
		a, b     any
		opts     []Option
		expected Order
	}{
		{1, 3, nil, LT},
		{3, 3, nil, EQ},
		{3, 1, nil, GT},
		{1, 3, []Option{Reversed()}, GT},
		{3, 3, []Option{Reversed()}, EQ},
		{3, 1, []Option{Reversed()}, LT},
		{false, false, nil, EQ},
		{false, true, nil, LT},
		{true, true, nil, EQ},
		{true, false, nil, GT},
		{"a", "b", nil, LT},
		{[]byte("a"), []byte("b"), nil, LT},
		{&spb.VName{Signature: "abc"}, &spb.VName{Signature: "bcd"}, []Option{ByVNameSignature}, LT},
		{&spb.VName{Signature: "abc"}, &spb.VName{}, []Option{ByVNameSignature}, GT},
		{&spb.VName{}, &spb.VName{}, []Option{ByVNameSignature}, EQ},
	}

	for _, test := range tests {
		if found := Compare(test.a, test.b, test.opts...); found != test.expected {
			t.Errorf("Compare(%#v, %#v) == %v; expected %v", test.a, test.b, found, test.expected)
		}
	}
}

func TestSeq(t *testing.T) {
	tests := []struct {
		a, b     *spb.VName
		opts     []Option
		expected Order
	}{
		{nil, nil, []Option{ByVNameSignature, ByVNameCorpus, ByVNameRoot}, EQ},
		{&spb.VName{Signature: "abc"}, &spb.VName{Signature: "bcd"}, []Option{ByVNameCorpus, ByVNameRoot}, EQ},
		{&spb.VName{Signature: "abc"}, &spb.VName{Signature: "bcd"}, []Option{ByVNameCorpus, ByVNameRoot, ByVNameSignature}, LT},
		{&spb.VName{Corpus: "a", Root: "b"}, &spb.VName{Corpus: "b", Root: "a"}, []Option{ByVNameCorpus, ByVNameRoot}, LT},
		{&spb.VName{Corpus: "a", Root: "b"}, &spb.VName{Corpus: "b", Root: "a"}, []Option{ByVNameSignature, ByVNameRoot, ByVNameCorpus}, GT},
	}

	for _, test := range tests {
		if found := Seq(test.a, test.b, test.opts...); found != test.expected {
			t.Errorf("Seq(%#v, %#v, %+v) == %v; expected %v", test.a, test.b, test.opts, found, test.expected)
		}
	}
}

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

func TestInts(t *testing.T) {
	tests := []struct {
		a, b     int
		expected Order
	}{
		{1, 1, EQ},
		{1, 2, LT},
		{2, 1, GT},
		{10, 1, GT},
		{1, 10, LT},
		{42, 42, EQ},
		{-5, 5, LT},
		{5, -5, GT},
	}

	for _, test := range tests {
		if found := Ints(test.a, test.b); found != test.expected {
			t.Errorf("Ints(%d, %d) == %v; expected %v", test.a, test.b, found, test.expected)
		}
	}
}
