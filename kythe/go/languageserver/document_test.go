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
	"testing"

	"kythe.io/kythe/go/test/testutil"

	"github.com/sourcegraph/go-langserver/pkg/lsp"
)

func TestRangeMatching(t *testing.T) {
	text := "abcdefg"
	doc := newDocument([]*RefResolution{
		{
			ticket: "longest",
			oldRange: lsp.Range{
				Start: lsp.Position{Line: 0, Character: 0},
				End:   lsp.Position{Line: 0, Character: 5},
			},
		},
		{
			ticket: "shortest",
			oldRange: lsp.Range{
				Start: lsp.Position{Line: 0, Character: 2},
				End:   lsp.Position{Line: 0, Character: 3},
			},
		},
		{
			ticket: "medium",
			oldRange: lsp.Range{
				Start: lsp.Position{Line: 0, Character: 2},
				End:   lsp.Position{Line: 0, Character: 4},
			},
		},
	}, text, text, nil)

	ref := doc.xrefs(lsp.Position{Line: 0, Character: 2})
	if ref == nil {
		t.Fatal("error acquiring xrefs")
	}
	if ref.ticket != "shortest" {
		t.Fatalf("expected shortest match. Found: %s", ref.ticket)
	}
}

type posCase struct {
	res string
	pos lsp.Position
}

type diffTest struct {
	oldText string
	newText string
	refs    []*RefResolution
	cases   []posCase
	errors  []lsp.Position
}

var diffTests = []diffTest{
	{
		"hi there",
		"    hi there",
		[]*RefResolution{{
			ticket: "hi",
			oldRange: lsp.Range{
				Start: lsp.Position{Line: 0, Character: 0},
				End:   lsp.Position{Line: 0, Character: 2},
			},
		}},
		[]posCase{{"hi", lsp.Position{Line: 0, Character: 4}}},
		[]lsp.Position{{Line: 0, Character: 0}},
	},
	{
		"hello there",
		"hello\nextra\nthere",
		[]*RefResolution{
			{
				ticket: "fulltext",
				oldRange: lsp.Range{
					Start: lsp.Position{Line: 0, Character: 0},
					End:   lsp.Position{Line: 0, Character: 11},
				},
			},
			{
				ticket: "there",
				oldRange: lsp.Range{
					Start: lsp.Position{Line: 0, Character: 6},
					End:   lsp.Position{Line: 0, Character: 11},
				},
			},
		},
		[]posCase{{"there", lsp.Position{Line: 2, Character: 0}}},
		[]lsp.Position{
			{Line: 0, Character: 0},
		},
	},
	{
		"hi there friend",
		"hi friend",
		[]*RefResolution{
			{
				ticket: "hi there",
				oldRange: lsp.Range{
					Start: lsp.Position{Line: 0, Character: 0},
					End:   lsp.Position{Line: 0, Character: 8},
				},
			},
			{
				ticket: "friend",
				oldRange: lsp.Range{
					Start: lsp.Position{Line: 0, Character: 9},
					End:   lsp.Position{Line: 0, Character: 15},
				},
			},
		},
		[]posCase{{"friend", lsp.Position{Line: 0, Character: 3}}},
		[]lsp.Position{{Line: 0, Character: 0}},
	},
}

func TestDiffing(t *testing.T) {
	for _, d := range diffTests {
		doc := newDocument(d.refs, d.oldText, d.newText, nil)

		for _, c := range d.cases {
			ref := doc.xrefs(c.pos)

			if ref == nil {
				t.Errorf("no ticket found for at pos %v. Expected '%s'", c.pos, c.res)
			}
			if err := testutil.DeepEqual(ref.ticket, c.res); err != nil {
				t.Errorf("incorrect ticket returned after edit: %v", err)
			}
		}

		for _, p := range d.errors {
			ref := doc.xrefs(p)
			if ref != nil {
				t.Errorf("unexpected ref found at location %v: %v", p, ref)
			}
		}
	}
}

func TestEmpty(t *testing.T) {
	// Verify that generating new references correctly handles the case where
	// there are none. This will panic (and thus fail) if it doesn't.
	doc := newDocument(nil, "nothing", "\nothing", nil)
	doc.generateNewRefs()
}
