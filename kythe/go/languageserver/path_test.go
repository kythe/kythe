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

	"kythe.io/kythe/go/test/testutil"
	"kythe.io/kythe/go/util/kytheuri"

	"github.com/sourcegraph/go-langserver/pkg/lsp"
)

func TestLocalFromURI(t *testing.T) {
	p, err := NewSettingsPathConfig(Settings{
		Root: "/root/dir",
		Mappings: []MappingConfig{{
			Local: ":path*",
			VName: VNameConfig{
				Path:   ":path*",
				Corpus: "corpus",
			},
		}},
	})

	if err != nil {
		t.Fatal(err)
	}

	badURIs := []lsp.DocumentURI{
		"",
		"malformed",
		"wrong://protocol",
		"file:///absolutely/outside/root",
		"file://relatively/outside/root",
	}
	for _, u := range badURIs {
		l, err := p.LocalFromURI(u)
		if err == nil {
			t.Errorf("Expected error converting URI (%s) to local\n  Found: %s", u, l)
		}
	}

	goodURIs := []lsp.DocumentURI{
		"file:///root/dir/topLevel.file",
		"file:///root/dir/very/deeply/nested.file",
	}

	for _, u := range goodURIs {
		_, err := p.LocalFromURI(u)

		if err != nil {
			t.Errorf("Error parsing URI (%s)", u)
		}
	}
}

func TestGeneration(t *testing.T) {
	p, err := NewSettingsPathConfig(Settings{
		Root: "/root/dir",
		Mappings: []MappingConfig{{
			Local: ":corpus/:path*/:root",
			VName: VNameConfig{
				Path:   ":path*",
				Corpus: ":corpus",
				Root:   ":root",
			},
		}},
	})
	if err != nil {
		t.Fatal(err)
	}

	l := "/root/dir/myCorpus/deeply/nested/myRoot"
	u := kytheuri.URI{
		Path:   "deeply/nested",
		Root:   "myRoot",
		Corpus: "myCorpus",
	}

	// Test local -> URI
	gu, err := p.KytheURIFromLocal(l)
	if err != nil {
		t.Errorf("error generating Kythe URI from local (%s):\n%v", l, err)
	}
	if err := testutil.DeepEqual(*gu, u); err != nil {
		t.Errorf("incorrect Kythe URI generated from local (%s):\n%v", l, err)
	}

	// Test URI -> local
	gl, err := p.LocalFromKytheURI(u)
	if err != nil {
		t.Errorf("error generating local from Kythe URI (%v):\n%v", u, err)
	}
	if err := testutil.DeepEqual(gl, l); err != nil {
		t.Errorf("incorrect local generated from Kythe URI (%v):\n%v", u, err)
	}
}
