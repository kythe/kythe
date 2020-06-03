/*
 * Copyright 2018 The Kythe Authors. All rights reserved.
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

package bazel

import (
	"io/ioutil"
	"os"
	"path/filepath"
	"testing"

	"github.com/golang/protobuf/proto"

	xapb "kythe.io/third_party/bazel/extra_actions_base_go_proto"
)

func TestFromSettings(t *testing.T) {
	const testVJ = `[{"pattern":"x","vname":{"corpus":"y"}}]`

	var xa xapb.ExtraActionInfo
	if err := proto.UnmarshalText(`
owner: "//target"
mnemonic: "some action"
[blaze.SpawnInfo.spawn_info]: <
  argument: "a"
  argument: "b"
  input_file: "infile"
  output_file: "outfile"
>`, &xa); err != nil {
		t.Fatalf("Unmarshaling test XA: %v", err)
	}

	tmp, err := ioutil.TempDir("", "settings")
	if err != nil {
		t.Fatalf("Setting up tempdir: %v", err)
	}
	defer os.RemoveAll(tmp)

	xaPath := filepath.Join(tmp, "test.xa")
	if bits, err := proto.Marshal(&xa); err != nil {
		t.Fatalf("Marshaling XA proto: %v", err)
	} else if err := ioutil.WriteFile(xaPath, bits, 0644); err != nil {
		t.Fatalf("Writing XA file: %v", err)
	}

	vjPath := filepath.Join(tmp, "vnames.json")
	if err := ioutil.WriteFile(vjPath, []byte(testVJ), 0644); err != nil {
		t.Fatalf("Writing vnames file: %v", err)
	}

	failures := []Settings{
		// Each of these should fail validation.
		{},
		{Corpus: "foo"},
		{Corpus: "foo", Language: "bar"},
		{Corpus: "foo", Language: "bar", ExtraAction: "baz"},
		{Corpus: "foo", Language: "bar", SourceFiles: "frob"},
		{Corpus: "foo", Language: "bar", SourceArgs: "frob"},

		// Each of these should fail construction.
		{Corpus: "foo", Language: "bar", SourceFiles: "quux",
			ExtraAction: "nonesuch", VNameRules: "nonesuch"},
		{Corpus: "foo", Language: "bar", SourceArgs: "brock",
			ExtraAction: xaPath, VNameRules: "nonesuch"},
		{Corpus: "foo", Language: "bar", SourceFiles: "zort",
			ExtraAction: "nonesuch", VNameRules: vjPath},
		{Corpus: "foo", Language: "bar", SourceArgs: "blub",
			SourceFiles: "ging", ExtraAction: xaPath,
			VNameRules: vjPath, ProtoFormat: "notcorrect"},
	}
	for _, test := range failures {
		t.Logf("Test settings: %+v", test)
		got, info, err := NewFromSettings(test)
		if err != nil {
			t.Logf("NewFromSettings: got error (expected): %v", err)
		} else {
			t.Errorf("NewFromSettings: got %+v, wanted error\ninfo is %+v", got, info)
		}
	}

	// When all the stars align, the fates allow us grace.
	got, info, err := NewFromSettings(Settings{
		Corpus:      "foo",
		Language:    "bar",
		SourceArgs:  "blub",
		SourceFiles: "ging",
		ExtraAction: xaPath,
		VNameRules:  vjPath,
		ProtoFormat: "json",
	})
	if err != nil {
		t.Fatalf("NewFromSettings: unexpected error: %v", err)
	}
	t.Logf("NewFromSettings config OK:\n%+v", got)
	t.Logf("NewFromSettings info OK:\n%s", proto.MarshalTextString(info))
}
