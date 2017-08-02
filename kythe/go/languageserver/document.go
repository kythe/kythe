/*
 * Copyright 2017 Google Inc. All rights reserved.
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

package languageserver

import (
	"errors"
	"sort"

	"github.com/sourcegraph/go-langserver/pkg/lsp"
)

// document encapsulates all information required to map file locations to Kythe tickets
type document struct {
	refs      []*RefResolution
	oldSrc    string
	newSrc    string
	staleRefs bool
}

func newDocument(refs []*RefResolution, oldSrc string) *document {
	d := new(document)
	d.refs = refs
	sort.Slice(d.refs, func(i, j int) bool {
		return posLess(d.refs[i].oldRange.Start, d.refs[j].oldRange.Start)
	})

	d.staleRefs = true
	return d
}

// xrefs produces a Kythe ticket corresponding to the entity at a given
// position in the file
func (d *document) xrefs(pos lsp.Position) (*string, error) {
	if d.staleRefs {
		d.generateNewRefs()
	}

	var smallestValidRef *RefResolution
	for _, r := range d.refs {
		if r.newRange != nil &&
			rangeContains(*r.newRange, pos) &&
			(smallestValidRef == nil || smaller(*r.newRange, *smallestValidRef.newRange)) {
			smallestValidRef = r
		}
	}
	if smallestValidRef != nil {
		return &smallestValidRef.ticket, nil
	}
	return nil, errors.New("No xref found")
}

// updateSrc accepts new file contents to be used for diffing when next
// required. This invalidates the previous diff.
func (d *document) updateSrc(newSrc string) {
	d.newSrc = newSrc
	d.staleRefs = true
}

// generateNewRefs generates refs by diffing the current file contents against
// the old contents
func (d *document) generateNewRefs() {
	// TODO(djrenren): Diff the file to produce new refs
	for _, ref := range d.refs {
		ref.newRange = &ref.oldRange
	}

	d.staleRefs = false
}

// RefResolution represents the mapping from a location in a document to a
// Kythe ticket
type RefResolution struct {
	ticket   string
	oldRange lsp.Range
	newRange *lsp.Range
}

func posLess(a, b lsp.Position) bool {
	if a.Line == b.Line {
		return a.Character < b.Character
	}
	return a.Line < b.Line
}

func posLeq(a, b lsp.Position) bool {
	if a.Line == b.Line {
		return a.Character <= b.Character
	}
	return a.Line < b.Line
}

// rangeContains checks whether a Range contains a Position. This
// check is inclusive because the position represents a cursor location.
// Consider an identifier "hi" at offset 0. Cursor positions 0-2 inclusive
// should match it.
func rangeContains(r lsp.Range, pos lsp.Position) bool {
	return posLeq(r.Start, pos) && posLeq(pos, r.End)
}

func smaller(r1, r2 lsp.Range) bool {
	r1lines := r1.End.Line - r1.Start.Line
	r2lines := r2.End.Line - r2.Start.Line

	if r1lines == r2lines {
		r1chars := r1.End.Character - r1.Start.Character
		r2chars := r2.End.Character - r2.Start.Character
		return r1chars < r2chars
	}
	return r1lines < r2lines
}
