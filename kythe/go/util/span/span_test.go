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

package span

import (
	"fmt"
	"strconv"
	"testing"

	"github.com/google/go-cmp/cmp/cmpopts"
	"kythe.io/kythe/go/test/testutil"
	"kythe.io/kythe/go/util/compare"

	"google.golang.org/protobuf/proto"

	cpb "kythe.io/kythe/proto/common_go_proto"
)

func TestNormalizerPoint(t *testing.T) {
	const text = `line 1
line 2
last line without newline`

	tests := []struct{ p, expected *cpb.Point }{
		{
			&cpb.Point{},
			&cpb.Point{ByteOffset: 0, LineNumber: 1, ColumnOffset: 0},
		},
		{
			&cpb.Point{LineNumber: 1},
			&cpb.Point{ByteOffset: 0, LineNumber: 1, ColumnOffset: 0},
		},
		{
			&cpb.Point{ByteOffset: 1},
			&cpb.Point{ByteOffset: 1, LineNumber: 1, ColumnOffset: 1},
		},
		{
			&cpb.Point{ByteOffset: 6},
			&cpb.Point{ByteOffset: 6, LineNumber: 1, ColumnOffset: 6},
		},
		{
			&cpb.Point{LineNumber: 1, ColumnOffset: 6},
			&cpb.Point{ByteOffset: 6, LineNumber: 1, ColumnOffset: 6},
		},
		{
			&cpb.Point{ByteOffset: 7},
			&cpb.Point{ByteOffset: 7, LineNumber: 2, ColumnOffset: 0},
		},
		{
			&cpb.Point{LineNumber: 2},
			&cpb.Point{ByteOffset: 7, LineNumber: 2, ColumnOffset: 0},
		},
		{
			&cpb.Point{ByteOffset: 10},
			&cpb.Point{ByteOffset: 10, LineNumber: 2, ColumnOffset: 3},
		},
		{
			&cpb.Point{ByteOffset: 13},
			&cpb.Point{ByteOffset: 13, LineNumber: 2, ColumnOffset: 6},
		},
		{
			&cpb.Point{LineNumber: 2, ColumnOffset: 6},
			&cpb.Point{ByteOffset: 13, LineNumber: 2, ColumnOffset: 6},
		},
		{
			&cpb.Point{LineNumber: 2, ColumnOffset: 7}, // past end of column
			&cpb.Point{ByteOffset: 13, LineNumber: 2, ColumnOffset: 6},
		},
		{
			&cpb.Point{LineNumber: 3},
			&cpb.Point{ByteOffset: 14, LineNumber: 3, ColumnOffset: 0},
		},
		{
			&cpb.Point{LineNumber: 3, ColumnOffset: 5},
			&cpb.Point{ByteOffset: 19, LineNumber: 3, ColumnOffset: 5},
		},
		{
			&cpb.Point{ByteOffset: 39},
			&cpb.Point{ByteOffset: 39, LineNumber: 3, ColumnOffset: 25},
		},
		{
			&cpb.Point{ByteOffset: 40}, // past end of text
			&cpb.Point{ByteOffset: 39, LineNumber: 3, ColumnOffset: 25},
		},
		{
			&cpb.Point{LineNumber: 5, ColumnOffset: 5}, // past end of text
			&cpb.Point{ByteOffset: 39, LineNumber: 3, ColumnOffset: 25},
		},
		{
			&cpb.Point{ByteOffset: -1}, // before start of text
			&cpb.Point{ByteOffset: 0, LineNumber: 1, ColumnOffset: 0},
		},
		{
			&cpb.Point{LineNumber: -1, ColumnOffset: 5}, // before start of text
			&cpb.Point{LineNumber: 1},
		},
	}

	n := NewNormalizer([]byte(text))
	for _, test := range tests {
		if p := n.Point(test.p); !proto.Equal(p, test.expected) {
			t.Errorf("n.Point({%v}): expected {%v}; found {%v}", test.p, test.expected, p)
		}
	}
}

func TestPatcher(t *testing.T) {
	tests := []struct {
		oldText, newText string

		oldSpans []*span
		newSpans []*span
	}{{
		oldText:  "this is some text",
		newText:  "this is some changed text",
		oldSpans: []*span{{0, 5}, {13, 17}, {13, 13}, {8, 17}},
		newSpans: []*span{{0, 5}, {21, 25}, {21, 21}, nil},
	}, {
		oldText:  "line one\nline two\nline three\nline four\n",
		newText:  "line one\nline three\nline two\nline four\n",
		oldSpans: []*span{{0, 10}, {10, 14}, {16, 19}, {20, 24}, {25, 30}, {29, 38}},
		newSpans: []*span{{0, 10}, {10, 14}, nil, {22, 26}, nil, {29, 38}},
	}, {
		oldText: "line three\n",
		newText: "line one\ntwo\nthree\n",
		oldSpans: []*span{
			{0, 4},
			{5, 5},
			{5, 10},
		},
		newSpans: []*span{
			{0, 4},
			{13, 13},
			{13, 18},
		},
	}}

	for i, test := range tests {
		t.Run(strconv.Itoa(i), func(t *testing.T) {
			p, err := NewPatcher([]byte(test.oldText), []byte(test.newText))
			if err != nil {
				t.Fatal(err)
			}
			if len(test.oldSpans) != len(test.newSpans) {
				t.Fatalf("Invalid test: {%v}", test)
			}

			for i, s := range test.oldSpans {
				start, end, exists := p.Patch(s.Start, s.End)

				if ns := test.newSpans[i]; ns == nil && exists {
					t.Errorf("Expected span not to exist in new text; received (%d, %d]", start, end)
				} else if ns != nil && !exists {
					t.Errorf("Expected span %v to exist in new text as %v; did not exist", s, ns)
				} else if ns != nil && exists && (start != ns.Start || end != ns.End) {
					t.Errorf("Expected %v; received (%d, %d]", ns, start, end)
				}
			}
		})
	}
}

func p(bo, ln, co int32) *cpb.Point {
	return &cpb.Point{ByteOffset: bo, LineNumber: ln, ColumnOffset: co}
}

func sp(start, end *cpb.Point) *cpb.Span { return &cpb.Span{Start: start, End: end} }

func TestPatchSpan(t *testing.T) {
	tests := []struct {
		oldText, newText string

		oldSpans []*cpb.Span
		newSpans []*cpb.Span
	}{{
		oldText: "this is some text\nsecond line\n",
		newText: "this is some text\nsecond line\n",

		oldSpans: []*cpb.Span{
			sp(p(0, 1, 0), p(4, 1, 4)),
			sp(p(13, 1, 13), p(17, 1, 17)),
		},
		newSpans: []*cpb.Span{
			sp(p(0, 1, 0), p(4, 1, 4)),
			sp(p(13, 1, 13), p(17, 1, 17)),
		},
	}, {
		oldText: "this is some text",
		newText: "this is some changed text",

		oldSpans: []*cpb.Span{
			sp(p(0, 1, 0), p(4, 1, 4)),
			sp(p(13, 1, 13), p(17, 1, 17)),
			nil,
		},
		newSpans: []*cpb.Span{
			sp(p(0, 1, 0), p(4, 1, 4)),
			sp(p(21, 1, 21), p(25, 1, 25)),
			nil,
		},
	}, {
		oldText: "line one\nline two\nline three\nline four\n",
		newText: "line one\nline three\nline two\npre line four\n",

		oldSpans: []*cpb.Span{
			sp(p(0, 1, 0), p(5, 1, 5)),
			sp(p(10, 2, 0), p(14, 2, 4)),
			sp(p(15, 2, 6), p(18, 2, 11)),
			sp(p(30, 4, 0), p(34, 4, 4)),
		},
		newSpans: []*cpb.Span{
			sp(p(0, 1, 0), p(5, 1, 5)),
			sp(p(10, 2, 0), p(14, 2, 4)),
			nil,
			sp(p(34, 4, 4), p(38, 4, 8)),
		},
	}, {
		oldText: "line one\nmoved\n",
		newText: "line one\ninsert\nmoved\n",

		oldSpans: []*cpb.Span{
			sp(p(5, 1, 5), p(8, 1, 8)),
			sp(p(10, 2, 0), p(15, 2, 5)),
		},
		newSpans: []*cpb.Span{
			sp(p(5, 1, 5), p(8, 1, 8)),
			sp(p(17, 3, 0), p(22, 3, 5)),
		},
	}, {
		oldText: "line\ntwo\nthree\n",
		newText: "line one\nline two\nthee\n",

		oldSpans: []*cpb.Span{
			sp(p(0, 1, 0), p(0, 1, 0)),
			sp(p(0, 1, 0), p(4, 1, 4)),
			sp(p(5, 2, 0), p(8, 2, 3)),
			sp(p(9, 3, 0), p(14, 3, 5)),
		},
		newSpans: []*cpb.Span{
			sp(p(0, 1, 0), p(0, 1, 0)),
			sp(p(0, 1, 0), p(4, 1, 4)),
			sp(p(14, 2, 5), p(17, 2, 8)),
			nil,
		},
	}, {
		oldText: "line three\n",
		newText: "line one\ntwo\nthree\n",

		oldSpans: []*cpb.Span{
			sp(p(0, 1, 0), p(4, 1, 4)),
			sp(p(5, 1, 5), p(5, 1, 5)),
			sp(p(5, 1, 5), p(10, 1, 10)),
		},
		newSpans: []*cpb.Span{
			sp(p(0, 1, 0), p(4, 1, 4)),
			sp(p(13, 3, 0), p(13, 3, 0)),
			sp(p(13, 3, 0), p(18, 3, 5)),
		},
	}, {
		oldText: "line three\nfour\n",
		newText: "line one\ntwo\nthree\nfour\n",

		oldSpans: []*cpb.Span{
			sp(p(0, 1, 0), p(4, 1, 4)),
			sp(p(5, 1, 5), p(10, 1, 10)),
			sp(p(11, 2, 0), p(15, 2, 4)),
		},
		newSpans: []*cpb.Span{
			sp(p(0, 1, 0), p(4, 1, 4)),
			sp(p(13, 3, 0), p(18, 3, 5)),
			sp(p(19, 4, 0), p(23, 4, 4)),
		},
	}, {
		oldText: "線\n動\n",
		newText: "線\n入\n動\n",

		oldSpans: []*cpb.Span{
			sp(p(0, 1, 0), p(3, 1, 3)),
			sp(p(0, 1, 0), p(7, 2, 3)),
			sp(p(4, 2, 0), p(7, 2, 3)),
		},
		newSpans: []*cpb.Span{
			sp(p(0, 1, 0), p(3, 1, 3)),
			nil,
			sp(p(8, 3, 0), p(11, 3, 3)),
		},
	}}

	for i, test := range tests {
		t.Run(strconv.Itoa(i), func(t *testing.T) {
			if len(test.oldSpans) == 0 || len(test.oldSpans) != len(test.newSpans) {
				t.Fatalf("Invalid test: {%v}", test)
			}

			p, err := NewPatcher([]byte(test.oldText), []byte(test.newText))
			if err != nil {
				t.Fatal(err)
			}

			for i, s := range test.oldSpans {
				t.Run(strconv.Itoa(i), func(t *testing.T) {
					found, exists := p.PatchSpan(s)

					if ns := test.newSpans[i]; s == nil && ns == nil {
						if found != nil || !exists {
							t.Errorf("Expected nil span to exist as nil span in new text; received %s %v", found, exists)
						}
					} else if ns == nil && exists {
						t.Errorf("Expected span not to exist in new text; received %s", found)
					} else if ns != nil && !exists {
						t.Errorf("Expected span %s to exist in new text as %s; did not exist", s, ns)
					} else if ns != nil && exists && !proto.Equal(found, ns) {
						t.Errorf("(-expected; +found)\n%s", compare.ProtoDiff(ns, found))
					}
				})
			}
		})
	}
}

func TestMarshal(t *testing.T) {
	expected := &Patcher{
		[]diff{
			{Length: 2, Type: ins, Newlines: 1, FirstNewline: 2, LastNewline: 5},
			{Length: 10, Type: del, Newlines: 5, FirstNewline: 0, LastNewline: 9},
			{Length: 3, Type: eq, Newlines: 0, FirstNewline: -1, LastNewline: -1},
			{Length: 5, Type: ins, Newlines: 1, FirstNewline: 2, LastNewline: 2},
		},
	}

	rec, err := expected.Marshal()
	testutil.Fatalf(t, "Marshal: %v", err)

	found, err := Unmarshal(rec)
	testutil.Fatalf(t, "Unmarshal: %v", err)

	// Check that the unmarshalled Patcher has correct prefix sums
	ignoreTypeField := cmpopts.IgnoreFields(offsetTracker{}, "Type")
	oldT := offsetTracker{Type: del}
	newT := offsetTracker{Type: ins}
	for _, d := range found.spans {
		if diff := compare.ProtoDiff(oldT, d.oldPrefix, ignoreTypeField); diff != "" {
			t.Fatalf("Old prefix sum incorrect (-expected; +found)\n%s", diff)
		}
		if diff := compare.ProtoDiff(newT, d.newPrefix, ignoreTypeField); diff != "" {
			t.Fatalf("New prefix sum incorrect (-expected; +found)\n%s", diff)
		}

		oldT.Update(d)
		newT.Update(d)
	}

	if diff := compare.ProtoDiff(expected.spans, found.spans, cmpopts.IgnoreUnexported(diff{})); diff != "" {
		t.Errorf("(-expected; +found)\n%s", diff)
	}
}

func TestLargeDiff(t *testing.T) {
	var before, after string
	for i := 0; i < 60; i++ {
		before += fmt.Sprintf("%d\n", i)
		after += fmt.Sprintf("%d\n", i/2)
	}

	patcher, err := NewPatcher([]byte(before), []byte(after))
	testutil.Fatalf(t, "NewPatcher: %v", err)

	s := sp(p(26, 12, 1), p(28, 12, 3))
	expected := sp(p(55, 25, 1), p(57, 25, 3))

	found, exists := patcher.PatchSpan(s)
	if !exists {
		t.Fatal("Patched span does not exist")
	}

	if diff := compare.ProtoDiff(expected, found); diff != "" {
		t.Errorf("(-expected; +found)\n%s", diff)
	}
}

type span struct{ Start, End int32 }

func (s span) String() string { return fmt.Sprintf("(%d, %d]", s.Start, s.End) }
