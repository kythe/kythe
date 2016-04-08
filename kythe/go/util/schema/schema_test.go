/*
 * Copyright 2016 Google Inc. All rights reserved.
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

package schema

import (
	"testing"

	"kythe.io/kythe/go/test/testutil"
)

type OrdinalTest struct {
	Input string

	Kind       string
	Ordinal    int
	HasOrdinal bool
}

func TestParseOrdinal(t *testing.T) {
	tests := []OrdinalTest{
		{"/kythe/edge/defines", "/kythe/edge/defines", 0, false},
		{"/kythe/edge/kind.here", "/kythe/edge/kind.here", 0, false},
		{"/kythe/edge/kind1", "/kythe/edge/kind1", 0, false},
		{"kind.-1", "kind.-1", 0, false},

		{"kind.3", "kind", 3, true},
		{"/kythe/edge/param.0", "/kythe/edge/param", 0, true},
		{"/kythe/edge/param.1", "/kythe/edge/param", 1, true},
		{"%/kythe/edge/param.1", "%/kythe/edge/param", 1, true},
		{"/kythe/edge/kind.1930", "/kythe/edge/kind", 1930, true},
	}

	for _, test := range tests {
		kind, ord, ok := ParseOrdinal(test.Input)
		if err := testutil.DeepEqual(&test, &OrdinalTest{test.Input, kind, ord, ok}); err != nil {
			t.Error(err)
		}
	}
}
