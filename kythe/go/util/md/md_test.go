/*
 * Copyright 2023 The Kythe Authors. All rights reserved.
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

package md

import (
	"testing"

	"github.com/google/go-cmp/cmp"
)

func TestEscape(t *testing.T) {
	tests := []struct {
		content     string
		destination string
		want        string
	}{{
		"foo",
		"dest",
		"[foo](dest)",
	}, {
		"f<b>oo&lt;/b&gt;",
		"dest",
		"[f<b>oo&lt;/b&gt;](dest)",
	}, {
		"f\\[oo\\]",
		"dest",
		"[f\\[oo\\]](dest)",
	}, {
		"foo",
		"d(est)",
		"[foo](d(est))",
	}}

	for _, tc := range tests {
		got := Link(tc.content, tc.destination)
		if diff := cmp.Diff(tc.want, got); diff != "" {
			t.Errorf("(-want, +got): %v", diff)
		}

	}
}

func TestProcessLinks(t *testing.T) {
	tests := []struct {
		content string
		links   []LinkInfo
		want    string
	}{{
		"No links",
		[]LinkInfo{},
		"No links",
	}, {
		"Link",
		[]LinkInfo{
			LinkInfo{0, 4, "http://test"},
		},
		"[Link](http://test)",
	}, {
		"Link Here",
		[]LinkInfo{
			LinkInfo{0, 9, "http://test"},
		},
		"[Link Here](http://test)",
	}, {
		"Link here but not here",
		[]LinkInfo{
			LinkInfo{5, 4, "http://test"},
		},
		"Link [here](http://test) but not here",
	}, {
		"Two links",
		[]LinkInfo{
			LinkInfo{0, 3, "http://test"},
			LinkInfo{4, 5, "http://test2"},
		},
		"[Two](http://test) [links](http://test2)",
	}, {
		"Link out of bounds",
		[]LinkInfo{
			LinkInfo{18, 1, "http://test"},
		},
		"Link out of bounds",
	}}

	for _, tc := range tests {
		got := ProcessLinks(tc.content, tc.links)
		if diff := cmp.Diff(tc.want, got); diff != "" {
			t.Errorf("TestConvert(%s, %#v): got unexpected diff (-want, +got): %v", tc.content, tc.links, diff)
		}
	}
}
