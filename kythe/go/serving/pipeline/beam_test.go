/*
 * Copyright 2018 Google Inc. All rights reserved.
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

package pipeline

import (
	"testing"

	"github.com/apache/beam/sdks/go/pkg/beam"
	"github.com/apache/beam/sdks/go/pkg/beam/testing/passert"
	"github.com/apache/beam/sdks/go/pkg/beam/testing/ptest"
	"github.com/apache/beam/sdks/go/pkg/beam/x/debug"

	cpb "kythe.io/kythe/proto/common_go_proto"
	ppb "kythe.io/kythe/proto/pipeline_go_proto"
	scpb "kythe.io/kythe/proto/schema_go_proto"
	srvpb "kythe.io/kythe/proto/serving_go_proto"
	spb "kythe.io/kythe/proto/storage_go_proto"
)

func TestReferences(t *testing.T) {
	testNodes := []*ppb.Node{{
		Source: &spb.VName{Path: "path", Signature: "anchor1"},
		Kind:   &ppb.Node_KytheKind{scpb.NodeKind_ANCHOR},
		Fact: []*ppb.Fact{{
			Name:  &ppb.Fact_KytheName{scpb.FactName_LOC_START},
			Value: []byte("0"),
		}, {
			Name:  &ppb.Fact_KytheName{scpb.FactName_LOC_END},
			Value: []byte("4"),
		}},
		Edge: []*ppb.Edge{{
			Kind:   &ppb.Edge_KytheKind{scpb.EdgeKind_REF},
			Target: &spb.VName{Signature: "node1"},
		}},
	}, {
		Source: &spb.VName{Path: "path", Signature: "anchor2"},
		Kind:   &ppb.Node_KytheKind{scpb.NodeKind_ANCHOR},
		Fact: []*ppb.Fact{{
			Name:  &ppb.Fact_KytheName{scpb.FactName_LOC_START},
			Value: []byte("5"),
		}, {
			Name:  &ppb.Fact_KytheName{scpb.FactName_LOC_END},
			Value: []byte("9"),
		}},
		Edge: []*ppb.Edge{{
			Kind:   &ppb.Edge_KytheKind{scpb.EdgeKind_CHILD_OF},
			Target: &spb.VName{Path: "path", Signature: "anchor2_parent"},
		}, {
			Kind:   &ppb.Edge_KytheKind{scpb.EdgeKind_REF_CALL},
			Target: &spb.VName{Signature: "node2"},
		}},
	}, {
		Source: &spb.VName{Path: "path"},
		Kind:   &ppb.Node_KytheKind{scpb.NodeKind_FILE},
		Fact: []*ppb.Fact{{
			Name:  &ppb.Fact_KytheName{scpb.FactName_TEXT},
			Value: []byte("some text\n"),
		}},
	}}

	expected := []*ppb.Reference{{
		Source: &spb.VName{Signature: "node1"},
		Kind:   &ppb.Reference_KytheKind{scpb.EdgeKind_REF},
		Anchor: &srvpb.ExpandedAnchor{
			Text: "some",
			Span: &cpb.Span{
				Start: &cpb.Point{
					LineNumber: 1,
				},
				End: &cpb.Point{
					ByteOffset:   4,
					LineNumber:   1,
					ColumnOffset: 4,
				},
			},
			Snippet: "some text",
			SnippetSpan: &cpb.Span{
				Start: &cpb.Point{
					LineNumber: 1,
				},
				End: &cpb.Point{
					ByteOffset:   9,
					LineNumber:   1,
					ColumnOffset: 9,
				},
			},
		},
	}, {
		Source: &spb.VName{Signature: "node2"},
		Kind:   &ppb.Reference_KytheKind{scpb.EdgeKind_REF_CALL},
		Scope:  &spb.VName{Path: "path", Signature: "anchor2_parent"},
		Anchor: &srvpb.ExpandedAnchor{
			Text: "text",
			Span: &cpb.Span{
				Start: &cpb.Point{
					ByteOffset:   5,
					LineNumber:   1,
					ColumnOffset: 5,
				},
				End: &cpb.Point{
					ByteOffset:   9,
					LineNumber:   1,
					ColumnOffset: 9,
				},
			},
			Snippet: "some text",
			SnippetSpan: &cpb.Span{
				Start: &cpb.Point{
					LineNumber: 1,
				},
				End: &cpb.Point{
					ByteOffset:   9,
					LineNumber:   1,
					ColumnOffset: 9,
				},
			},
		},
	}}

	p, s, nodes := ptest.CreateList(testNodes)
	refs := FromNodes(s, nodes).References()
	debug.Print(s, refs)
	passert.Equals(s, refs, beam.CreateList(s, expected))

	if err := ptest.Run(p); err != nil {
		t.Fatalf("Pipeline error: %+v", err)
	}
}
