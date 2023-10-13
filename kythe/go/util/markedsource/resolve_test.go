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

package markedsource

import (
	"strconv"
	"testing"

	"kythe.io/kythe/go/test/testutil"
	"kythe.io/kythe/go/util/compare"
	"kythe.io/kythe/go/util/kytheuri"
	"kythe.io/kythe/go/util/schema/edges"
	"kythe.io/kythe/go/util/schema/facts"

	"google.golang.org/protobuf/encoding/prototext"
	"google.golang.org/protobuf/proto"

	cpb "kythe.io/kythe/proto/common_go_proto"
	spb "kythe.io/kythe/proto/storage_go_proto"
)

func TestResolve(t *testing.T) {
	tests := []struct {
		Entries  []*spb.Entry
		Ticket   string
		Expected string
	}{
		{
			[]*spb.Entry{code(t, "kythe:#test", `kind: IDENTIFIER pre_text: "simple"`)},
			"kythe:#test",
			`kind: IDENTIFIER pre_text: "simple"`,
		},
		{
			[]*spb.Entry{
				code(t, "kythe:#test", `kind: BOX child: { kind: LOOKUP_BY_PARAM lookup_index: 0 }`),
				edge(t, "kythe:#test", edges.ParamIndex(0), "kythe:#param"),
				code(t, "kythe:#param", `kind: IDENTIFIER pre_text: "param"`),
			},
			"kythe:#test",
			`kind: BOX child: { kind: PARAMETER child: { kind: IDENTIFIER pre_text: "param" } }`,
		},
		{
			[]*spb.Entry{
				code(t, "kythe:#test", `kind: BOX child: { kind: LOOKUP_BY_PARAM lookup_index: 0 }`),
				edge(t, "kythe:#test", edges.ParamIndex(0), "kythe:#paramLevel0"),
				code(t, "kythe:#paramLevel0", `kind: LOOKUP_BY_PARAM lookup_index: 1`),
				edge(t, "kythe:#paramLevel0", edges.ParamIndex(1), "kythe:#paramLevel1"),
				code(t, "kythe:#paramLevel1", `kind: IDENTIFIER pre_text: "deep"`),
			},
			"kythe:#test",
			`kind: BOX child: { kind: PARAMETER child: { kind: IDENTIFIER pre_text: "deep" } }`,
		},
		{
			[]*spb.Entry{
				code(t, "kythe:#test", `kind: PARAMETER_LOOKUP_BY_PARAM post_child_text: ", "`),
				edge(t, "kythe:#test", edges.ParamIndex(0), "kythe:#param0"),
				edge(t, "kythe:#test", edges.ParamIndex(1), "kythe:#param1"),
				edge(t, "kythe:#test", edges.ParamIndex(2), "kythe:#param2"),
				code(t, "kythe:#param0", `kind: IDENTIFIER pre_text: "0"`),
				code(t, "kythe:#param1", `kind: IDENTIFIER pre_text: "1"`),
				code(t, "kythe:#param2", `kind: IDENTIFIER pre_text: "2"`),
			},
			"kythe:#test",
			`kind: PARAMETER post_child_text: ", " child: { kind: IDENTIFIER pre_text: "0" } child: { kind: IDENTIFIER pre_text: "1" } child: { kind: IDENTIFIER pre_text: "2" }`,
		},
	}

	for i, test := range tests {
		t.Run(strconv.Itoa(i), func(t *testing.T) {
			r, err := NewResolver(test.Entries)
			testutil.Fatalf(t, "NewResolver: %v", err)
			v := parseVName(t, test.Ticket)
			found := r.Resolve(v)
			expected := parseMarkedSource(t, test.Expected)

			if diff := compare.ProtoDiff(expected, found); diff != "" {
				t.Fatalf("(- expected; + found)\n%s", diff)
			}
		})
	}
}

func code(t testing.TB, src, code string) *spb.Entry {
	t.Helper()
	v := parseVName(t, src)
	ms := parseMarkedSource(t, code)
	rec, err := proto.Marshal(ms)
	testutil.Fatalf(t, "proto.Marshal: %v", err)
	return &spb.Entry{
		Source:    v,
		FactName:  facts.Code,
		FactValue: rec,
	}
}

func edge(t testing.TB, src, kind, tgt string) *spb.Entry {
	t.Helper()
	return &spb.Entry{
		Source:   parseVName(t, src),
		EdgeKind: kind,
		Target:   parseVName(t, tgt),
	}
}

func parseVName(t testing.TB, s string) *spb.VName {
	t.Helper()
	v, err := kytheuri.ToVName(s)
	testutil.Fatalf(t, "kytheuri.ParseVName: %v", err)
	return v
}

func parseMarkedSource(t testing.TB, s string) *cpb.MarkedSource {
	t.Helper()
	var ms cpb.MarkedSource
	testutil.Fatalf(t, "prototext.Unmarshal: %v", prototext.Unmarshal([]byte(s), &ms))
	return &ms
}

func e(t testing.TB, s string) *spb.Entry {
	t.Helper()
	var e spb.Entry
	testutil.Fatalf(t, "prototext.Unmarshal: %v", prototext.Unmarshal([]byte(s), &e))
	return &e
}
