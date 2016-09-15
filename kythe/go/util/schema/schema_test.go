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

	"kythe.io/kythe/go/util/schema/edges"
	"kythe.io/kythe/go/util/schema/facts"
	"kythe.io/kythe/go/util/schema/nodes"

	"github.com/golang/protobuf/proto"

	spb "kythe.io/kythe/proto/storage_proto"
)

func TestNodeToEntry(t *testing.T) {
	n := &Node{
		VName: &spb.VName{Signature: "a"},
		Kind:  nodes.Anchor,
		Facts: Facts{
			facts.AnchorStart: "25",
			facts.AnchorEnd:   "37",
			facts.Text:        "apple",
		},
	}
	entries := n.ToEntries()
	t.Logf("Checking entries for node: %+v", n)

	check := func(key, value string) {
		for _, entry := range entries {
			if entry.FactName == key && string(entry.FactValue) == value {
				return
			}
		}
		t.Errorf("Missing entry for %q, wanted %q", key, value)
	}
	check(facts.NodeKind, nodes.Anchor)
	check(facts.AnchorStart, "25")
	check(facts.AnchorEnd, "37")
	check(facts.Text, "apple")
	check(facts.TextEncoding, facts.DefaultTextEncoding)
}

func TestEdgeToEntry(t *testing.T) {
	src := &spb.VName{Signature: "source"}
	tgt := &spb.VName{Signature: "target"}
	e := &Edge{
		Source: src,
		Target: tgt,
		Kind:   edges.ChildOf,
	}
	got := e.ToEntry()
	want := &spb.Entry{
		Source:   src,
		Target:   tgt,
		EdgeKind: edges.ChildOf,
		FactName: "/",
	}
	if !proto.Equal(got, want) {
		t.Errorf("ToEdge(%+v):\n--- got\n%s\n--- want\n%s", e, proto.MarshalTextString(got), proto.MarshalTextString(want))
	}
}
