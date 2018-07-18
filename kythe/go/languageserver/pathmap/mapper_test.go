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

package pathmap

import (
	"reflect"
	"testing"
)

type values map[string]string
type failures []string
type successes []struct {
	string
	values
}

type testcase struct {
	successes
	failures
}

var cases = map[string]testcase{
	"": {
		successes{
			{"", values{}},
		},
		failures{"fail"},
	},
	"hello": {
		successes{
			{"hello", values{}},
		},
		failures{"", "notHello"},
	},
	":file": {
		successes{
			{"hello", values{"file": "hello"}},
		},
		failures{"dir/file"},
	},
	"/dir/:file": {
		successes{
			{"/dir/test", values{"file": "test"}},
		},
		failures{"", "dir/", "dir/test", "/dir/trailing/", "/dir/too/long"},
	},
	"/dir/:files*": {
		successes{
			{"/dir/test", values{"files": "test"}},
			{"/dir/test/nested", values{"files": "test/nested"}},
		},
		failures{"/dir/test/", "/dir/"},
	},
	"/dir/:splat*/sep/:file": {
		successes{
			{"/dir/deeply/nested/sep/img.png", values{"splat": "deeply/nested", "file": "img.png"}},
		},
		failures{"/dir//sep/hi"},
	},
}

func TestNewMapper(t *testing.T) {
	for p := range cases {
		_, err := NewMapper(p)
		if err != nil {
			t.Errorf("unexpected error parsing pattern (%s):\n%v", p, err)
		}
	}

	_, err := NewMapper("not a path")
	if err == nil {
		t.Error("expected error parsing non-path pattern")
	}
}

func TestMapperParseGenerate(t *testing.T) {
	for pat, c := range cases {
		m, err := NewMapper(pat)
		if err != nil {
			t.Errorf("unexpected error parsing pattern (%s):\n%v", pat, err)
			continue
		}

		for _, p := range c.successes {
			v, err := m.Parse(p.string)
			if err != nil {
				t.Errorf("unexpected error parsing path (%s) with pattern (%v):\n%v", p.string, m.re, err)
			}
			if !reflect.DeepEqual(values(v), p.values) {
				t.Errorf("incorrect values parsing path (%s) with pattern (%s):\nExpected: %v\nFound: %v", p.string, pat, p.values, v)
			}

			g, err := m.Generate(p.values)
			if err != nil {
				t.Errorf("unexpected error generating path with pattern (%s) and values (%v)", pat, p.values)
			}
			if g != p.string {
				t.Errorf("incorrect generated path with pattern (%s) and values (%v):\nExpected: %s\nFound: %s", pat, p.values, p.string, g)
			}
		}

		for _, p := range c.failures {
			_, err := m.Parse(p)
			if err == nil {
				t.Errorf("expected error parsing path (%s) with pattern (%s)", p, pat)
			}
		}
	}
}
