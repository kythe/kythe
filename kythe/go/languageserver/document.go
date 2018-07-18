/*
 * Copyright 2017 The Kythe Authors. All rights reserved.
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
	"sort"
	"strings"

	"github.com/sergi/go-diff/diffmatchpatch"
	"github.com/sourcegraph/go-langserver/pkg/lsp"
)

// document encapsulates all information required to map file locations to Kythe tickets
type document struct {
	refs      []*RefResolution
	oldSrc    string
	newSrc    string
	staleRefs bool
	defLocs   map[string]*lsp.Location
}

func newDocument(refs []*RefResolution, oldSrc string, newSrc string, defLocs map[string]*lsp.Location) *document {
	d := &document{
		refs:      refs,
		oldSrc:    oldSrc,
		newSrc:    newSrc,
		staleRefs: true,
		defLocs:   defLocs,
	}

	sort.Slice(d.refs, func(i, j int) bool {
		return posLess(d.refs[i].oldRange.Start, d.refs[j].oldRange.Start)
	})

	return d
}

// xrefs produces a Kythe ticket corresponding to the entity at a given
// position in the file
func (doc *document) xrefs(pos lsp.Position) *RefResolution {
	if doc.staleRefs {
		doc.generateNewRefs()
	}

	var smallestValidRef *RefResolution
	for _, r := range doc.refs {
		if r.newRange != nil &&
			rangeContains(*r.newRange, pos) &&
			(smallestValidRef == nil || smaller(*r.newRange, *smallestValidRef.newRange)) {
			smallestValidRef = r
		}
	}
	return smallestValidRef
}

// updateSource accepts new file contents to be used for diffing when next
// required This invalidates the previous diff
func (doc *document) updateSource(newSrc string) {
	doc.newSrc = newSrc
	doc.staleRefs = true
}

// rangeInNewSource takes in a range representing a ref in the oldSrc
// and returns that refs range in the newSrc if it exists
func (doc *document) rangeInNewSource(r lsp.Range) *lsp.Range {
	if doc.staleRefs {
		doc.generateNewRefs()
	}

	for _, ref := range doc.refs {
		if ref.oldRange == r {
			return ref.newRange
		}
	}

	return nil
}

// generateNewRefs generates refs by diffing the current file contents against
// the old contents
func (doc *document) generateNewRefs() {
	defer func() { doc.staleRefs = false }()

	// Short circuit if there are no references
	if len(doc.refs) == 0 {
		return
	}
	// Invalidate all previously calculated ranges
	for _, r := range doc.refs {
		r.newRange = nil
	}

	dmp := diffmatchpatch.New()
	diffs := dmp.DiffCleanupSemanticLossless(dmp.DiffMain(doc.oldSrc, doc.newSrc, true))

	// oldPos & newPos track progress through the oldSrc and newSrc respectively
	var oldPos, newPos lsp.Position
	// refIdx represents progress through the refs slice
	refIdx := 0

diffLoop:
	for _, d := range diffs {
		// We know that if our position in the old file is greater than
		// start of the ref then the current diff cannot be used to produce
		// a new range. Because refs is sorted by the start location of the
		// oldRange, we can loop forward until the current ref is past oldPos,
		// invalidating the refs along the way
		for posLess(doc.refs[refIdx].oldRange.Start, oldPos) {
			refIdx++
			// If all refs have been adjusted, nothing else needs to be done
			if len(doc.refs) <= refIdx {
				break diffLoop
			}
		}

		// Because refs are stored as lsp locations which are essentially
		// (line, character) tuples, knowing the lines contained within the
		// diff segment will be relevant
		dLines := strings.Split(d.Text, "\n")
		// If there's no newline we can't say the diff spans any lines so we
		// subtract 1
		dLineLen := len(dLines) - 1
		dNewLine := dLineLen != 0
		// dOffset determines the amount of characters the last line of the diff
		// contains
		dOffset := len(dLines[dLineLen])

		switch d.Type {
		// If text was deleted, we "move past" it in the oldSrc so we move the
		// oldPos forward
		case diffmatchpatch.DiffDelete:
			// If there was new line in the diff, then we moved forward that many lines
			// and are now at the character offset of the last line. Otherwise, we've moved
			// further through the same line
			if dNewLine {
				oldPos.Line += dLineLen
				oldPos.Character = dOffset
			} else {
				oldPos.Character += dOffset
			}

		// If text was inserted, we have "move past" it in the newSrc so we move the
		// newPos forward
		case diffmatchpatch.DiffInsert:
			if dNewLine {
				newPos.Line += dLineLen
				newPos.Character = dOffset
			} else {
				newPos.Character += dOffset
			}

		// DiffEqual is the only case where can actually map refs because we know all
		// refs contained within the equality are preserved
		case diffmatchpatch.DiffEqual:
			var oldEndChar int
			if dNewLine {
				oldEndChar = dOffset
			} else {
				oldEndChar = oldPos.Character + dOffset
			}

			// This range represents the span of the diff in the oldSrc
			diffRange := lsp.Range{
				Start: oldPos,
				End: lsp.Position{
					Line:      oldPos.Line + dLineLen,
					Character: oldEndChar,
				},
			}

			// Declare iteration variables outside because i will become the new refIdx
			// when we break or finish
			var (
				ref *RefResolution
				i   int
			)
			// Loop over the remaining unprocessed refs
			for i, ref = range doc.refs[refIdx:] {
				// When the start position is past the diffRange we know all
				// refs within the equality have been updated
				if !rangeContains(diffRange, ref.oldRange.Start) {
					break
				}

				// If the ref extends beyond the diffRange, we know the ref will
				// be invalidated
				if !rangeContains(diffRange, ref.oldRange.End) {
					continue
				}

				var (
					refStartLine  = newPos.Line + (ref.oldRange.Start.Line - oldPos.Line)
					refOnDiffLine = refStartLine != newPos.Line
					refLineLength = ref.oldRange.End.Line - ref.oldRange.Start.Line
					refCharLength = ref.oldRange.End.Character - ref.oldRange.Start.Character
					refStartChar  int
					refEndChar    int
				)

				// If the ref is on a different line than newPos, we know a newline occurs
				// between newPos and the start of the ref, meaning the start character will
				// be unchanged. Otherwise we add the offset of the ref from oldPos to newPos
				if refOnDiffLine {
					refStartChar = ref.oldRange.Start.Character
				} else {
					refStartChar = newPos.Character + ref.oldRange.Start.Character - oldPos.Character
				}

				if refLineLength > 0 {
					refEndChar = ref.oldRange.End.Character
				} else {
					refEndChar = refStartChar + refCharLength
				}

				ref.newRange = &lsp.Range{
					Start: lsp.Position{Line: refStartLine, Character: refStartChar},
					End: lsp.Position{
						Line:      refStartLine + refLineLength,
						Character: refEndChar,
					},
				}
			}

			// We know there's no reason to go back to any refs
			// within the diffRange
			refIdx += i

			// On an equality we have to move both oldPos and newPos forward
			if dNewLine {
				oldPos.Line += dLineLen
				oldPos.Character = dOffset
				newPos.Line += dLineLen
				newPos.Character = dOffset
			} else {
				oldPos.Character += dOffset
				newPos.Character += dOffset
			}
		}
	}
}

// RefResolution represents the mapping from a location in a document to a
// Kythe ticket
type RefResolution struct {
	ticket   string
	def      string     // the target definition anchor ticket
	markup   string     // a rendering of marked source
	comment  string     // if available, a comment
	lang     string     // a language label
	oldRange lsp.Range  // the range indexed
	newRange *lsp.Range // the range after patching (if viable)
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
