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

package identifiers

import (
	"context"
	"testing"

	"github.com/golang/protobuf/proto"

	"kythe.io/kythe/go/storage/table"
	"kythe.io/kythe/go/test/testutil"
	ipb "kythe.io/kythe/proto/identifier_proto"
	srvpb "kythe.io/kythe/proto/serving_proto"
)

var matchTable = Table{testProtoTable{
	"foo::bar": &srvpb.IdentifierMatch{
		Node: []*srvpb.IdentifierMatch_Node{
			{"kythe://corpus?lang=c++", "record", "class"},
			{"kythe://corpus?lang=rust", "record", "struct"},
		},
		BaseName:      "bar",
		QualifiedName: "foo::bar",
	},

	"com.java.package.Interface": &srvpb.IdentifierMatch{
		Node: []*srvpb.IdentifierMatch_Node{
			{"kythe://habeas?lang=java", "record", "interface"},
		},
		BaseName:      "Interface",
		QualifiedName: "com.java.package.Interface",
	},
}}

var tests = []testCase{
	{
		ipb.FindRequest{"foo::bar", nil, nil},
		[]*ipb.FindReply_Match{
			{"kythe://corpus?lang=c++", "record", "class", "bar", "foo::bar"},
			{"kythe://corpus?lang=rust", "record", "struct", "bar", "foo::bar"},
		},
	},
	{
		ipb.FindRequest{"foo::bar", nil, []string{"rust"}},
		[]*ipb.FindReply_Match{
			{"kythe://corpus?lang=rust", "record", "struct", "bar", "foo::bar"},
		},
	},
	{
		ipb.FindRequest{"com.java.package.Interface", []string{"habeas"}, nil},
		[]*ipb.FindReply_Match{
			{"kythe://habeas?lang=java", "record", "interface", "Interface", "com.java.package.Interface"},
		},
	},
	{
		ipb.FindRequest{"com.java.package.Interface", []string{"corpus"}, nil},
		[]*ipb.FindReply_Match{},
	},
}

func TestFind(t *testing.T) {
	for _, test := range tests {
		reply, err := matchTable.Find(context.TODO(), &test.FindRequest)
		if err != nil {
			t.Errorf("unexpected error for request %v: %v", test.FindRequest, err)
		}

		if err := testutil.DeepEqual(test.Matches, reply.Matches); err != nil {
			t.Error(err)
		}
	}
}

type testCase struct {
	ipb.FindRequest
	Matches []*ipb.FindReply_Match
}

type testProtoTable map[string]proto.Message

func (t testProtoTable) Put(_ context.Context, key []byte, val proto.Message) error {
	t[string(key)] = val
	return nil
}

func (t testProtoTable) Lookup(_ context.Context, key []byte, msg proto.Message) error {
	m, ok := t[string(key)]
	if !ok {
		return table.ErrNoSuchKey
	}
	proto.Merge(msg, m)
	return nil
}

func (t testProtoTable) Buffered() table.BufferedProto { panic("UNIMPLEMENTED") }

func (t testProtoTable) Close(_ context.Context) error { return nil }
