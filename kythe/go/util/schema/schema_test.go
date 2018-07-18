/*
 * Copyright 2016 The Kythe Authors. All rights reserved.
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

	"kythe.io/kythe/go/util/schema/facts"
	"kythe.io/kythe/go/util/schema/nodes"

	"github.com/golang/protobuf/proto"

	scpb "kythe.io/kythe/proto/schema_go_proto"
	spb "kythe.io/kythe/proto/storage_go_proto"
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
		Kind:   "childof",
	}
	got := e.ToEntry()
	want := &spb.Entry{
		Source:   src,
		Target:   tgt,
		EdgeKind: "childof",
		FactName: "/",
	}
	if !proto.Equal(got, want) {
		t.Errorf("ToEdge(%+v):\n--- got\n%s\n--- want\n%s", e, proto.MarshalTextString(got), proto.MarshalTextString(want))
	}
}

func TestNodeKindEnum(t *testing.T) {
	known := []struct {
		Name string
		Enum scpb.NodeKind
	}{
		{"anchor", scpb.NodeKind_ANCHOR},
		{"record", scpb.NodeKind_RECORD},
	}

	for _, k := range known {
		if found := NodeKind(k.Name); found != k.Enum {
			t.Errorf("NodeKind(%q) == %v; expected: %v", k.Name, found, k.Enum)
		}
	}
}

func TestFactNameEnum(t *testing.T) {
	known := []struct {
		Name string
		Enum scpb.FactName
	}{
		{"/kythe/loc/start", scpb.FactName_LOC_START},
		{"/kythe/node/kind", scpb.FactName_NODE_KIND},
		{"/kythe/subkind", scpb.FactName_SUBKIND},
	}

	for _, k := range known {
		if found := FactName(k.Name); found != k.Enum {
			t.Errorf("FactName(%q) == %v; expected: %v", k.Name, found, k.Enum)
		}
	}
}

func TestEdgeKindEnum(t *testing.T) {
	known := []struct {
		Name string
		Enum scpb.EdgeKind
	}{
		{"/kythe/edge/annotatedby", scpb.EdgeKind_ANNOTATED_BY},
		{"/kythe/edge/childof", scpb.EdgeKind_CHILD_OF},
		{"/kythe/edge/ref", scpb.EdgeKind_REF},
		{"/kythe/edge/ref/implicit", scpb.EdgeKind_REF_IMPLICIT},
	}

	for _, k := range known {
		if found := EdgeKind(k.Name); found != k.Enum {
			t.Errorf("EdgeKind(%q) == %v; expected: %v", k.Name, found, k.Enum)
		}
	}
}

func TestSubkindEnum(t *testing.T) {
	known := []struct {
		Name string
		Enum scpb.Subkind
	}{
		{"class", scpb.Subkind_CLASS},
		{"implicit", scpb.Subkind_IMPLICIT},
		{"enumClass", scpb.Subkind_ENUM_CLASS},
	}

	for _, k := range known {
		if found := Subkind(k.Name); found != k.Enum {
			t.Errorf("Subkind(%q) == %v; expected: %v", k.Name, found, k.Enum)
		}
	}
}

func TestEdgeKindStrings(t *testing.T) {
	for _, i := range proto.EnumValueMap("kythe.proto.schema.EdgeKind") {
		enum := scpb.EdgeKind(i)
		if enum == scpb.EdgeKind_UNKNOWN_EDGE_KIND {
			continue
		}
		str := EdgeKindString(enum)
		if str == "" {
			t.Errorf("Missing string value for %v", enum)
		} else if found := EdgeKind(str); found != enum {
			t.Errorf("Mismatched EdgeKind: found: %v; expected: %v", found, enum)
		}
	}
}

func TestNodeKindStrings(t *testing.T) {
	for _, i := range proto.EnumValueMap("kythe.proto.schema.NodeKind") {
		enum := scpb.NodeKind(i)
		if enum == scpb.NodeKind_UNKNOWN_NODE_KIND {
			continue
		}
		str := NodeKindString(enum)
		if str == "" {
			t.Errorf("Missing string value for %v", enum)
		} else if found := NodeKind(str); found != enum {
			t.Errorf("Mismatched NodeKind: found: %v; expected: %v", found, enum)
		}
	}
}

func TestFactNameStrings(t *testing.T) {
	for _, i := range proto.EnumValueMap("kythe.proto.schema.FactName") {
		enum := scpb.FactName(i)
		if enum == scpb.FactName_UNKNOWN_FACT_NAME {
			continue
		}
		str := FactNameString(enum)
		if str == "" {
			t.Errorf("Missing string value for %v", enum)
		} else if found := FactName(str); found != enum {
			t.Errorf("Mismatched FactName: found: %v; expected: %v", found, enum)
		}
	}
}

func TestSubkindStrings(t *testing.T) {
	for _, i := range proto.EnumValueMap("kythe.proto.schema.Subkind") {
		enum := scpb.Subkind(i)
		if enum == scpb.Subkind_UNKNOWN_SUBKIND {
			continue
		}
		str := SubkindString(enum)
		if str == "" {
			t.Errorf("Missing string value for %v", enum)
		} else if found := Subkind(str); found != enum {
			t.Errorf("Mismatched Subkind: found: %v; expected: %v", found, enum)
		}
	}
}
