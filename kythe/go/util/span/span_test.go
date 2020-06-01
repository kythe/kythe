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
	"testing"

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
		oldSpans: []*span{{0, 5}, {13, 17}, {8, 17}},
		newSpans: []*span{{0, 5}, {21, 25}, nil},
	}, {
		oldText:  "line one\nline two\nline three\nline four\n",
		newText:  "line one\nline three\nline two\nline four\n",
		oldSpans: []*span{{0, 10}, {10, 14}, {16, 19}, {20, 24}, {25, 30}, {29, 38}},
		newSpans: []*span{{0, 10}, {10, 14}, nil, {22, 26}, nil, {29, 38}},
	}}

	for _, test := range tests {
		p := NewPatcher([]byte(test.oldText), []byte(test.newText))
		if len(test.oldSpans) != len(test.newSpans) {
			t.Fatalf("Invalid test: {%v}", test)
		}

		for i, s := range test.oldSpans {
			start, end, exists := p.Patch(s.start, s.end)

			if ns := test.newSpans[i]; ns == nil && exists {
				t.Errorf("Expected span not to exist in new text; received (%d, %d]", start, end)
			} else if ns != nil && !exists {
				t.Errorf("Expected span %v to exist in new text as %v; did not exist", s, ns)
			} else if ns != nil && exists && (start != ns.start || end != ns.end) {
				t.Errorf("Expected %v; received (%d, %d]", ns, start, end)
			}
		}
	}
}

type span struct{ start, end int32 }

func (s span) String() string { return fmt.Sprintf("(%d, %d]", s.start, s.end) }
