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

import "testing"

func TestLocalFromURI(t *testing.T) {
	p := PathConfig{
		Root:   "/root/dir/",
		Corpus: "corpus",
	}
	badURIs := []string{
		"",
		"malformed",
		"wrong://protocol",
		"file:///absolutely/outside/root",
		"file://relatively/outside/root",
	}
	for _, u := range badURIs {
		l, err := p.localFromURI(u)
		if err == nil {
			t.Errorf("Expected error converting URI (%s) to local\n  Found: %s", u, l)
		}
	}

	goodURIs := []string{
		"file:///root/dir/topLevel.file",
		"file:///root/dir/very/deeply/nested.file",
	}

	for _, u := range goodURIs {
		_, err := p.localFromURI(u)

		if err != nil {
			t.Errorf("Error parsing URI (%s)", u)
		}
	}
}
