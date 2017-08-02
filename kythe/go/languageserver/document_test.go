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
	"testing"

	"github.com/sourcegraph/go-langserver/pkg/lsp"
)

func TestRangeMatching(t *testing.T) {
	text := "abcdefg"
	doc := newDocument([]*RefResolution{
		&RefResolution{
			ticket: "longest",
			oldRange: lsp.Range{
				Start: lsp.Position{Line: 0, Character: 0},
				End:   lsp.Position{Line: 0, Character: 5},
			},
		},
		&RefResolution{
			ticket: "shortest",
			oldRange: lsp.Range{
				Start: lsp.Position{Line: 0, Character: 2},
				End:   lsp.Position{Line: 0, Character: 3},
			},
		},
		&RefResolution{
			ticket: "medium",
			oldRange: lsp.Range{
				Start: lsp.Position{Line: 0, Character: 2},
				End:   lsp.Position{Line: 0, Character: 4},
			},
		},
	}, text)

	ticket, err := doc.xrefs(lsp.Position{Line: 0, Character: 2})
	if err != nil {
		t.Fatalf("Error acquiring xrefs: %v", err)
	}
	if *ticket != "shortest" {
		t.Fatalf("Expected shortest match. Found: %s", *ticket)
	}
}
