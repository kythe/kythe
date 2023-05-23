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

// Package span implements utilities to resolve byte offsets within a file to
// line and column numbers.
package span // import "kythe.io/kythe/go/util/span"

import (
	"bytes"
	"errors"
	"fmt"
	"sort"

	"kythe.io/kythe/go/util/log"

	"github.com/sergi/go-diff/diffmatchpatch"
	"google.golang.org/protobuf/proto"

	cpb "kythe.io/kythe/proto/common_go_proto"
	srvpb "kythe.io/kythe/proto/serving_go_proto"
	xpb "kythe.io/kythe/proto/xref_go_proto"
)

// InBounds reports whether [start,end) is bounded by the specified [startBoundary,endBoundary) span.
func InBounds(kind xpb.DecorationsRequest_SpanKind, start, end, startBoundary, endBoundary int32) bool {
	switch kind {
	case xpb.DecorationsRequest_WITHIN_SPAN:
		return start >= startBoundary && end <= endBoundary
	case xpb.DecorationsRequest_AROUND_SPAN:
		return start <= startBoundary && end >= endBoundary
	default:
		log.Warningf("unknown DecorationsRequest_SpanKind: %v", kind)
	}
	return false
}

// Patcher uses a computed diff between two texts to map spans from the original
// text to the new text.
type Patcher struct {
	spans []diff
}

// NewPatcher returns a Patcher based on the diff between oldText and newText.
func NewPatcher(oldText, newText []byte) (p *Patcher, err error) {
	defer func() {
		// dmp may panic on some large requests; catch it and return an error instead
		if r := recover(); r != nil {
			err = fmt.Errorf("diffmatchpatch panic: %v", r)
		}
	}()
	dmp := diffmatchpatch.New()
	diff := dmp.DiffCleanupEfficiency(dmp.DiffMain(string(oldText), string(newText), false))
	return &Patcher{mapToOffsets(diff)}, nil
}

// Marshal encodes the Patcher into a packed binary format.
func (p *Patcher) Marshal() ([]byte, error) {
	db := &srvpb.Diff{
		SpanLength:       make([]int32, len(p.spans)),
		SpanType:         make([]srvpb.Diff_Type, len(p.spans)),
		SpanNewlines:     make([]int32, len(p.spans)),
		SpanFirstNewline: make([]int32, len(p.spans)),
		SpanLastNewline:  make([]int32, len(p.spans)),
	}
	for i, d := range p.spans {
		db.SpanLength[i] = d.Length
		db.SpanNewlines[i] = d.Newlines
		db.SpanFirstNewline[i] = d.FirstNewline
		db.SpanLastNewline[i] = d.LastNewline
		switch d.Type {
		case eq:
			db.SpanType[i] = srvpb.Diff_EQUAL
		case ins:
			db.SpanType[i] = srvpb.Diff_INSERT
		case del:
			db.SpanType[i] = srvpb.Diff_DELETE
		default:
			return nil, fmt.Errorf("unknown diff type: %s", d.Type)
		}
	}
	return proto.Marshal(db)
}

// Unmarshal decodes a Patcher from its packed binary format.
func Unmarshal(rec []byte) (*Patcher, error) {
	var db srvpb.Diff
	if err := proto.Unmarshal(rec, &db); err != nil {
		return nil, err
	}
	if len(db.SpanLength) != len(db.SpanType) {
		return nil, fmt.Errorf("length of span_length does not match length of span_type: %d vs %d", len(db.SpanLength), len(db.SpanType))
	} else if len(db.SpanLength) != len(db.SpanNewlines) {
		return nil, fmt.Errorf("length of span_length does not match length of span_newlines: %d vs %d", len(db.SpanLength), len(db.SpanNewlines))
	} else if len(db.SpanLength) != len(db.SpanFirstNewline) {
		return nil, fmt.Errorf("length of span_length does not match length of span_first_newline: %d vs %d", len(db.SpanLength), len(db.SpanFirstNewline))
	} else if len(db.SpanLength) != len(db.SpanLastNewline) {
		return nil, fmt.Errorf("length of span_length does not match length of span_last_newline: %d vs %d", len(db.SpanLength), len(db.SpanLastNewline))
	}
	spans := make([]diff, len(db.SpanLength))
	for i, l := range db.SpanLength {
		spans[i] = diff{
			Length:       l,
			Newlines:     db.SpanNewlines[i],
			FirstNewline: db.SpanFirstNewline[i],
			LastNewline:  db.SpanLastNewline[i],
		}
		switch db.SpanType[i] {
		case srvpb.Diff_EQUAL:
			spans[i].Type = eq
		case srvpb.Diff_INSERT:
			spans[i].Type = ins
		case srvpb.Diff_DELETE:
			spans[i].Type = del
		default:
			return nil, fmt.Errorf("unknown diff type: %s", db.SpanType[i])
		}
		if i != 0 {
			updatePrefix(&spans[i-1], &spans[i])
		}
	}
	return &Patcher{spans}, nil
}

func updatePrefix(prev, d *diff) {
	d.oldPrefix = prev.oldPrefix
	d.newPrefix = prev.newPrefix
	d.oldPrefix.Type = del
	d.newPrefix.Type = ins
	d.oldPrefix.Update(*prev)
	d.newPrefix.Update(*prev)
}

type diff struct {
	Length int32
	Type   diffmatchpatch.Operation

	Newlines     int32
	FirstNewline int32
	LastNewline  int32

	oldPrefix, newPrefix offsetTracker
}

const (
	eq  = diffmatchpatch.DiffEqual
	ins = diffmatchpatch.DiffInsert
	del = diffmatchpatch.DiffDelete
)

func mapToOffsets(ds []diffmatchpatch.Diff) []diff {
	res := make([]diff, len(ds))
	for i, d := range ds {
		l := len(d.Text)
		var newlines int
		var first, last int = -1, -1
		for j := 0; j < l; j++ {
			if d.Text[j] != '\n' {
				continue
			}
			newlines++
			if first == -1 {
				first = j
			}
			last = j
		}
		res[i] = diff{
			Length:       int32(l),
			Type:         d.Type,
			Newlines:     int32(newlines),
			FirstNewline: int32(first),
			LastNewline:  int32(last),
		}
		if i != 0 {
			updatePrefix(&res[i-1], &res[i])
		}
	}
	return res
}

type offsetTracker struct {
	Type diffmatchpatch.Operation

	Offset       int32
	Lines        int32
	ColumnOffset int32
}

func (t *offsetTracker) Update(d diff) {
	if d.Type != eq && d.Type != t.Type {
		return
	}
	t.Offset += d.Length
	t.Lines += d.Newlines
	if d.LastNewline == -1 {
		t.ColumnOffset += d.Length
	} else {
		t.ColumnOffset = d.Length - d.LastNewline - 1
	}
}

// PatchSpan returns the resulting Span of mapping the given Span from the
// Patcher's constructed oldText to its newText.  If the span no longer exists
// in newText or is invalid, the returned bool will be false.  As a convenience,
// if p==nil, the original span will be returned.
func (p *Patcher) PatchSpan(s *cpb.Span) (span *cpb.Span, exists bool) {
	spanStart, spanEnd := ByteOffsets(s)
	if spanStart > spanEnd {
		return nil, false
	} else if p == nil || s == nil {
		return s, true
	}

	// Find the diff span that contains the starting offset.
	idx := sort.Search(len(p.spans), func(i int) bool {
		return spanStart < p.spans[i].oldPrefix.Offset
	}) - 1
	if idx < 0 {
		return nil, false
	}

	d := p.spans[idx]
	if d.Type != eq || spanEnd > d.oldPrefix.Offset+d.Length {
		return nil, false
	}

	lineDiff := d.newPrefix.Lines - d.oldPrefix.Lines
	colDiff := d.newPrefix.ColumnOffset - d.oldPrefix.ColumnOffset
	if d.FirstNewline != -1 && spanStart-d.oldPrefix.Offset >= d.FirstNewline {
		// The given span is past the first newline so it has no column diff.
		colDiff = 0
	}
	return &cpb.Span{
		Start: &cpb.Point{
			ByteOffset:   d.newPrefix.Offset + (spanStart - d.oldPrefix.Offset),
			ColumnOffset: s.GetStart().GetColumnOffset() + colDiff,
			LineNumber:   s.GetStart().GetLineNumber() + lineDiff,
		},
		End: &cpb.Point{
			ByteOffset:   d.newPrefix.Offset + (spanEnd - d.oldPrefix.Offset),
			ColumnOffset: s.GetEnd().GetColumnOffset() + colDiff,
			LineNumber:   s.GetEnd().GetLineNumber() + lineDiff,
		},
	}, true
}

// ByteOffsets returns the starting and ending byte offsets of the Span.
func ByteOffsets(s *cpb.Span) (int32, int32) {
	return s.GetStart().GetByteOffset(), s.GetEnd().GetByteOffset()
}

// Patch returns the resulting span of mapping the given span from the Patcher's
// constructed oldText to its newText.  If the span no longer exists in newText
// or is invalid, the returned bool will be false.  As a convenience, if p==nil,
// the original span will be returned.
func (p *Patcher) Patch(spanStart, spanEnd int32) (newStart, newEnd int32, exists bool) {
	if spanStart > spanEnd {
		return 0, 0, false
	} else if p == nil {
		return spanStart, spanEnd, true
	}

	if spanStart == spanEnd {
		// Give zero-width span a positive length for the below algorithm; then fix
		// the length on return.
		spanEnd++
		defer func() { newEnd = newStart }()
	}

	var old, new int32
	for _, d := range p.spans {
		l := d.Length
		if old > spanStart {
			return 0, 0, false
		}
		switch d.Type {
		case eq:
			if old <= spanStart && spanEnd <= old+l {
				newStart = new + (spanStart - old)
				newEnd = new + (spanEnd - old)
				exists = true
				return
			}
			old += l
			new += l
		case del:
			old += l
		case ins:
			new += l
		}
	}

	return 0, 0, false
}

// Normalizer fixes xref.Locations within a given source text so that each point
// has consistent byte_offset, line_number, and column_offset fields within the
// range of text's length and its line lengths.
type Normalizer struct {
	textLen   int32
	lineLen   []int32
	prefixLen []int32
}

// NewNormalizer returns a Normalizer for Locations within text.
func NewNormalizer(text []byte) *Normalizer {
	lines := bytes.Split(text, lineEnd)
	lineLen := make([]int32, len(lines))
	prefixLen := make([]int32, len(lines))
	for i := 1; i < len(lines); i++ {
		lineLen[i-1] = int32(len(lines[i-1]) + len(lineEnd))
		prefixLen[i] = prefixLen[i-1] + lineLen[i-1]
	}
	lineLen[len(lines)-1] = int32(len(lines[len(lines)-1]) + len(lineEnd))
	return &Normalizer{int32(len(text)), lineLen, prefixLen}
}

// Location returns a normalized location within the Normalizer's text.
// Normalized FILE locations have no start/end points.  Normalized SPAN
// locations have fully populated start/end points clamped in the range [0,
// len(text)).
func (n *Normalizer) Location(loc *xpb.Location) (*xpb.Location, error) {
	nl := &xpb.Location{}
	if loc == nil {
		return nl, nil
	}
	nl.Ticket = loc.Ticket
	nl.Kind = loc.Kind
	if loc.Kind == xpb.Location_FILE {
		return nl, nil
	}

	if loc.Span == nil {
		return nil, errors.New("invalid SPAN: missing span")
	} else if loc.Span.Start == nil {
		return nil, errors.New("invalid SPAN: missing span start point")
	} else if loc.Span.End == nil {
		return nil, errors.New("invalid SPAN: missing span end point")
	}

	nl.Span = n.Span(loc.Span)

	start, end := nl.Span.Start.ByteOffset, nl.Span.End.ByteOffset
	if start > end {
		return nil, fmt.Errorf("invalid SPAN: start (%d) is after end (%d)", start, end)
	}
	return nl, nil
}

// Span returns a Span with its start and end normalized.
func (n *Normalizer) Span(s *cpb.Span) *cpb.Span {
	if s == nil {
		return nil
	}
	return &cpb.Span{
		Start: n.Point(s.Start),
		End:   n.Point(s.End),
	}
}

// SpanOffsets returns a Span based on normalized start and end byte offsets.
func (n *Normalizer) SpanOffsets(start, end int32) *cpb.Span {
	return &cpb.Span{
		Start: n.ByteOffset(start),
		End:   n.ByteOffset(end),
	}
}

var lineEnd = []byte("\n")

// Point returns a normalized point within the Normalizer's text.  A normalized
// point has all of its fields set consistently and clamped within the range
// [0,len(text)).
func (n *Normalizer) Point(p *cpb.Point) *cpb.Point {
	if p == nil {
		return nil
	}

	if p.ByteOffset > 0 {
		return n.ByteOffset(p.ByteOffset)
	} else if p.LineNumber > 0 {
		np := &cpb.Point{
			LineNumber:   p.LineNumber,
			ColumnOffset: p.ColumnOffset,
		}

		if totalLines := int32(len(n.lineLen)); p.LineNumber > totalLines {
			np.LineNumber = totalLines
			np.ColumnOffset = n.lineLen[np.LineNumber-1] - 1
		}
		if np.ColumnOffset < 0 {
			np.ColumnOffset = 0
		} else if np.ColumnOffset > 0 {
			if lineLen := n.lineLen[np.LineNumber-1] - 1; p.ColumnOffset > lineLen {
				np.ColumnOffset = lineLen
			}
		}

		np.ByteOffset = n.prefixLen[np.LineNumber-1] + np.ColumnOffset

		return np
	}

	return &cpb.Point{LineNumber: 1}
}

// ByteOffset returns a normalized point based on the given offset within the
// Normalizer's text.  A normalized point has all of its fields set consistently
// and clamped within the range [0,len(text)).
func (n *Normalizer) ByteOffset(offset int32) *cpb.Point {
	np := &cpb.Point{ByteOffset: offset}
	if np.ByteOffset > n.textLen {
		np.ByteOffset = n.textLen
	}

	np.LineNumber = int32(sort.Search(len(n.lineLen), func(i int) bool {
		return n.prefixLen[i] > np.ByteOffset
	}))
	np.ColumnOffset = np.ByteOffset - n.prefixLen[np.LineNumber-1]

	return np
}
