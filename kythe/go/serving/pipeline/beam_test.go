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

package pipeline

import (
	"testing"

	"kythe.io/kythe/go/serving/pipeline/beamtest"

	"github.com/apache/beam/sdks/go/pkg/beam"
	"github.com/apache/beam/sdks/go/pkg/beam/testing/passert"
	"github.com/apache/beam/sdks/go/pkg/beam/testing/ptest"
	"github.com/apache/beam/sdks/go/pkg/beam/x/debug"
	"github.com/google/go-cmp/cmp"
	"github.com/google/go-cmp/cmp/cmpopts"
	"google.golang.org/protobuf/proto"

	cpb "kythe.io/kythe/proto/common_go_proto"
	ppb "kythe.io/kythe/proto/pipeline_go_proto"
	scpb "kythe.io/kythe/proto/schema_go_proto"
	srvpb "kythe.io/kythe/proto/serving_go_proto"
	spb "kythe.io/kythe/proto/storage_go_proto"
)

func TestReferences(t *testing.T) {
	testNodes := []*scpb.Node{{
		Source: &spb.VName{Path: "path", Signature: "anchor1"},
		Kind:   &scpb.Node_KytheKind{scpb.NodeKind_ANCHOR},
		Fact: []*scpb.Fact{{
			Name:  &scpb.Fact_KytheName{scpb.FactName_LOC_START},
			Value: []byte("0"),
		}, {
			Name:  &scpb.Fact_KytheName{scpb.FactName_LOC_END},
			Value: []byte("4"),
		}, {
			Name:  &scpb.Fact_KytheName{scpb.FactName_BUILD_CONFIG},
			Value: []byte("test-build-config"),
		}},
		Edge: []*scpb.Edge{{
			Kind:   &scpb.Edge_KytheKind{scpb.EdgeKind_REF},
			Target: &spb.VName{Signature: "node1"},
		}},
	}, {
		Source: &spb.VName{Path: "path", Signature: "anchor2"},
		Kind:   &scpb.Node_KytheKind{scpb.NodeKind_ANCHOR},
		Fact: []*scpb.Fact{{
			Name:  &scpb.Fact_KytheName{scpb.FactName_LOC_START},
			Value: []byte("5"),
		}, {
			Name:  &scpb.Fact_KytheName{scpb.FactName_LOC_END},
			Value: []byte("9"),
		}},
		Edge: []*scpb.Edge{{
			Kind:   &scpb.Edge_KytheKind{scpb.EdgeKind_CHILD_OF},
			Target: &spb.VName{Path: "path", Signature: "anchor2_parent"},
		}, {
			Kind:   &scpb.Edge_KytheKind{scpb.EdgeKind_REF_CALL},
			Target: &spb.VName{Signature: "node2"},
		}},
	}, {
		Source: &spb.VName{Path: "path"},
		Kind:   &scpb.Node_KytheKind{scpb.NodeKind_FILE},
		Fact: []*scpb.Fact{{
			Name:  &scpb.Fact_KytheName{scpb.FactName_TEXT},
			Value: []byte("some text\n"),
		}},
	}}

	expected := []*ppb.Reference{{
		Source: &spb.VName{Signature: "node1"},
		Kind:   &ppb.Reference_KytheKind{scpb.EdgeKind_REF},
		Anchor: &srvpb.ExpandedAnchor{
			Ticket: "kythe:?path=path#anchor1",
			Text:   "some",
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
			BuildConfiguration: "test-build-config",
		},
	}, {
		Source: &spb.VName{Signature: "node2"},
		Kind:   &ppb.Reference_KytheKind{scpb.EdgeKind_REF_CALL},
		Scope:  &spb.VName{Path: "path", Signature: "anchor2_parent"},
		Anchor: &srvpb.ExpandedAnchor{
			Ticket: "kythe:?path=path#anchor2",
			Text:   "text",
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

	beam.Init()
	p, s, nodes := ptest.CreateList(testNodes)
	refs := FromNodes(s, nodes).References()
	debug.Print(s, refs)
	passert.Equals(s, refs, beam.CreateList(s, expected))

	ptest.RunAndValidate(t, p)
}

func TestDecorations_targetNode(t *testing.T) {
	testNodes := []*scpb.Node{{
		Source: &spb.VName{Path: "path", Signature: "anchor1"},
		Kind:   &scpb.Node_KytheKind{scpb.NodeKind_ANCHOR},
		Fact: []*scpb.Fact{{
			Name:  &scpb.Fact_KytheName{scpb.FactName_LOC_START},
			Value: []byte("5"),
		}, {
			Name:  &scpb.Fact_KytheName{scpb.FactName_LOC_END},
			Value: []byte("9"),
		}},
		Edge: []*scpb.Edge{{
			Kind:   &scpb.Edge_KytheKind{scpb.EdgeKind_REF},
			Target: &spb.VName{Signature: "node1"},
		}},
	}, {
		Source: &spb.VName{Path: "path"},
		Kind:   &scpb.Node_KytheKind{scpb.NodeKind_FILE},
		Fact: []*scpb.Fact{{
			Name:  &scpb.Fact_KytheName{scpb.FactName_TEXT},
			Value: []byte("some text\n"),
		}},
	}, {
		Source: &spb.VName{Signature: "node1"},
		Kind:   &scpb.Node_KytheKind{scpb.NodeKind_RECORD},
		Fact: []*scpb.Fact{{
			Name:  &scpb.Fact_GenericName{"/unknown/fact/name"},
			Value: []byte("something"),
		}},
	}}

	expected := []*srvpb.FileDecorations{{
		File: &srvpb.File{
			Text: []byte("some text\n"),
		},
		Decoration: []*srvpb.FileDecorations_Decoration{{
			Anchor: &srvpb.RawAnchor{
				StartOffset: 5,
				EndOffset:   9,
			},
			Kind:   "/kythe/edge/ref",
			Target: "kythe:#node1",
		}},
		Target: []*srvpb.Node{{
			Ticket: "kythe:#node1",
			Fact: []*cpb.Fact{{
				Name:  "/kythe/node/kind",
				Value: []byte("record"),
			}, {
				Name:  "/unknown/fact/name",
				Value: []byte("something"),
			}},
		}},
	}}

	beam.Init()
	p, s, nodes := ptest.CreateList(testNodes)
	decor := FromNodes(s, nodes).Decorations()
	debug.Print(s, decor)
	passert.Equals(s, beam.DropKey(s, decor), beam.CreateList(s, expected))

	ptest.RunAndValidate(t, p)
}

func TestDecorations_decoration(t *testing.T) {
	testNodes := []*scpb.Node{{
		Source: &spb.VName{Path: "path", Signature: "anchor1"},
		Kind:   &scpb.Node_KytheKind{scpb.NodeKind_ANCHOR},
		Fact: []*scpb.Fact{{
			Name:  &scpb.Fact_KytheName{scpb.FactName_LOC_START},
			Value: []byte("5"),
		}, {
			Name:  &scpb.Fact_KytheName{scpb.FactName_LOC_END},
			Value: []byte("9"),
		}},
		Edge: []*scpb.Edge{{
			Kind:   &scpb.Edge_KytheKind{scpb.EdgeKind_REF},
			Target: &spb.VName{Signature: "node1"},
		}},
	}, {
		Source: &spb.VName{Path: "path"},
		Kind:   &scpb.Node_KytheKind{scpb.NodeKind_FILE},
		Fact: []*scpb.Fact{{
			Name:  &scpb.Fact_KytheName{scpb.FactName_TEXT},
			Value: []byte("some text\n"),
		}},
	}}

	expected := []*srvpb.FileDecorations{{
		File: &srvpb.File{
			Text: []byte("some text\n"),
		},
		Decoration: []*srvpb.FileDecorations_Decoration{{
			Anchor: &srvpb.RawAnchor{
				StartOffset: 5,
				EndOffset:   9,
			},
			Kind:   "/kythe/edge/ref",
			Target: "kythe:#node1",
		}},
	}}

	beam.Init()
	p, s, nodes := ptest.CreateList(testNodes)
	decor := FromNodes(s, nodes).Decorations()
	debug.Print(s, decor)
	passert.Equals(s, beam.DropKey(s, decor), beam.CreateList(s, expected))

	ptest.RunAndValidate(t, p)
}

func TestDecorations_diagnostics(t *testing.T) {
	testNodes := []*scpb.Node{{
		Source: &spb.VName{Path: "path", Signature: "anchor1"},
		Kind:   &scpb.Node_KytheKind{scpb.NodeKind_ANCHOR},
		Fact: []*scpb.Fact{{
			Name:  &scpb.Fact_KytheName{scpb.FactName_LOC_START},
			Value: []byte("5"),
		}, {
			Name:  &scpb.Fact_KytheName{scpb.FactName_LOC_END},
			Value: []byte("9"),
		}},
		Edge: []*scpb.Edge{{
			Kind:   &scpb.Edge_KytheKind{scpb.EdgeKind_TAGGED},
			Target: &spb.VName{Signature: "diagnostic"},
		}},
	}, {
		Source: &spb.VName{Path: "path"},
		Kind:   &scpb.Node_KytheKind{scpb.NodeKind_FILE},
		Fact: []*scpb.Fact{{
			Name:  &scpb.Fact_KytheName{scpb.FactName_TEXT},
			Value: []byte("some text\n"),
		}},
		Edge: []*scpb.Edge{{
			Kind:   &scpb.Edge_KytheKind{scpb.EdgeKind_TAGGED},
			Target: &spb.VName{Signature: "diagnostic"},
		}},
	}, {
		Source: &spb.VName{Signature: "diagnostic"},
		Kind:   &scpb.Node_KytheKind{scpb.NodeKind_DIAGNOSTIC},
		Fact: []*scpb.Fact{{
			Name:  &scpb.Fact_KytheName{scpb.FactName_MESSAGE},
			Value: []byte("msg"),
		}, {
			Name:  &scpb.Fact_KytheName{scpb.FactName_DETAILS},
			Value: []byte("deets"),
		}, {
			Name:  &scpb.Fact_KytheName{scpb.FactName_CONTEXT_URL},
			Value: []byte("https://kythe.io"),
		}},
	}}

	expected := []*srvpb.FileDecorations{{
		File: &srvpb.File{Text: []byte("some text\n")},
		Diagnostic: []*cpb.Diagnostic{{
			Message:    "msg",
			Details:    "deets",
			ContextUrl: "https://kythe.io",
		}, {
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
			Message:    "msg",
			Details:    "deets",
			ContextUrl: "https://kythe.io",
		}},
	}}

	beam.Init()
	p, s, nodes := ptest.CreateList(testNodes)
	decor := FromNodes(s, nodes).Decorations()
	debug.Print(s, decor)
	passert.Equals(s, beam.DropKey(s, decor), beam.CreateList(s, expected))

	ptest.RunAndValidate(t, p)
}

func TestDecorations_targetDefinition(t *testing.T) {
	testNodes := []*scpb.Node{{
		Source: &spb.VName{Path: "path", Signature: "anchor1"},
		Kind:   &scpb.Node_KytheKind{scpb.NodeKind_ANCHOR},
		Fact: []*scpb.Fact{{
			Name:  &scpb.Fact_KytheName{scpb.FactName_LOC_START},
			Value: []byte("5"),
		}, {
			Name:  &scpb.Fact_KytheName{scpb.FactName_LOC_END},
			Value: []byte("9"),
		}},
		Edge: []*scpb.Edge{{
			Kind:   &scpb.Edge_KytheKind{scpb.EdgeKind_REF},
			Target: &spb.VName{Signature: "node1"},
		}},
	}, {
		Source: &spb.VName{Path: "path"},
		Kind:   &scpb.Node_KytheKind{scpb.NodeKind_FILE},
		Fact: []*scpb.Fact{{
			Name:  &scpb.Fact_KytheName{scpb.FactName_TEXT},
			Value: []byte("some text\n"),
		}},
	}, {
		Source: &spb.VName{Path: "path2", Signature: "def1"},
		Kind:   &scpb.Node_KytheKind{scpb.NodeKind_ANCHOR},
		Fact: []*scpb.Fact{{
			Name:  &scpb.Fact_KytheName{scpb.FactName_LOC_START},
			Value: []byte("5"),
		}, {
			Name:  &scpb.Fact_KytheName{scpb.FactName_LOC_END},
			Value: []byte("8"),
		}},
		Edge: []*scpb.Edge{{
			Kind:   &scpb.Edge_KytheKind{scpb.EdgeKind_DEFINES_BINDING},
			Target: &spb.VName{Signature: "node1"},
		}},
	}, {
		Source: &spb.VName{Path: "path2"},
		Kind:   &scpb.Node_KytheKind{scpb.NodeKind_FILE},
		Fact: []*scpb.Fact{{
			Name:  &scpb.Fact_KytheName{scpb.FactName_TEXT},
			Value: []byte("some def\n"),
		}},
	}}

	def := &srvpb.ExpandedAnchor{
		Ticket: "kythe:?path=path2#def1",
		Text:   "def",
		Span: &cpb.Span{
			Start: &cpb.Point{
				ByteOffset:   5,
				LineNumber:   1,
				ColumnOffset: 5,
			},
			End: &cpb.Point{
				ByteOffset:   8,
				LineNumber:   1,
				ColumnOffset: 8,
			},
		},
		Snippet: "some def",
		SnippetSpan: &cpb.Span{
			Start: &cpb.Point{
				LineNumber: 1,
			},
			End: &cpb.Point{
				ByteOffset:   8,
				LineNumber:   1,
				ColumnOffset: 8,
			},
		},
	}
	expected := []*srvpb.FileDecorations{{
		File: &srvpb.File{Text: []byte("some text\n")},
		Decoration: []*srvpb.FileDecorations_Decoration{{
			Anchor: &srvpb.RawAnchor{
				StartOffset: 5,
				EndOffset:   9,
			},
			Kind:             "/kythe/edge/ref",
			Target:           "kythe:#node1",
			TargetDefinition: "kythe:?path=path2#def1",
		}},
		TargetDefinitions: []*srvpb.ExpandedAnchor{def},
	}, {
		File: &srvpb.File{Text: []byte("some def\n")},
		Decoration: []*srvpb.FileDecorations_Decoration{{
			Anchor: &srvpb.RawAnchor{
				StartOffset: 5,
				EndOffset:   8,
			},
			Kind:   "/kythe/edge/defines/binding",
			Target: "kythe:#node1",

			// TODO(schroederc): ellide TargetDefinition for actual definitions
			TargetDefinition: "kythe:?path=path2#def1",
		}},
		TargetDefinitions: []*srvpb.ExpandedAnchor{def},
	}}

	beam.Init()
	p, s, nodes := ptest.CreateList(testNodes)
	decor := FromNodes(s, nodes).Decorations()
	debug.Print(s, decor)
	passert.Equals(s, beam.DropKey(s, decor), beam.CreateList(s, expected))

	ptest.RunAndValidate(t, p)
}

func TestDecorations_overrides(t *testing.T) {
	testNodes := []*scpb.Node{{
		Source: &spb.VName{Path: "path"},
		Kind:   &scpb.Node_KytheKind{scpb.NodeKind_FILE},
		Fact: []*scpb.Fact{{
			Name:  &scpb.Fact_KytheName{scpb.FactName_TEXT},
			Value: []byte("some text\n"),
		}},
	}, {
		Source: &spb.VName{Path: "path2", Signature: "def1"},
		Kind:   &scpb.Node_KytheKind{scpb.NodeKind_ANCHOR},
		Fact: []*scpb.Fact{{
			Name:  &scpb.Fact_KytheName{scpb.FactName_LOC_START},
			Value: []byte("5"),
		}, {
			Name:  &scpb.Fact_KytheName{scpb.FactName_LOC_END},
			Value: []byte("8"),
		}},
		Edge: []*scpb.Edge{{
			Kind:   &scpb.Edge_KytheKind{scpb.EdgeKind_DEFINES_BINDING},
			Target: &spb.VName{Signature: "node1"},
		}},
	}, {
		Source: &spb.VName{Path: "path2", Signature: "def2"},
		Kind:   &scpb.Node_KytheKind{scpb.NodeKind_ANCHOR},
		Fact: []*scpb.Fact{{
			Name:  &scpb.Fact_KytheName{scpb.FactName_LOC_START},
			Value: []byte("4"),
		}, {
			Name:  &scpb.Fact_KytheName{scpb.FactName_LOC_END},
			Value: []byte("7"),
		}},
		Edge: []*scpb.Edge{{
			Kind:   &scpb.Edge_KytheKind{scpb.EdgeKind_DEFINES_BINDING},
			Target: &spb.VName{Signature: "node2"},
		}},
	}, {
		Source: &spb.VName{Signature: "node2"},
		Kind:   &scpb.Node_KytheKind{scpb.NodeKind_FUNCTION},
		Edge: []*scpb.Edge{{
			Kind:   &scpb.Edge_KytheKind{scpb.EdgeKind_OVERRIDES},
			Target: &spb.VName{Signature: "node1"},
		}},
	}, {
		Source: &spb.VName{Path: "path2"},
		Kind:   &scpb.Node_KytheKind{scpb.NodeKind_FILE},
		Fact: []*scpb.Fact{{
			Name:  &scpb.Fact_KytheName{scpb.FactName_TEXT},
			Value: []byte("some def\n"),
		}},
	}}

	expected := []*srvpb.FileDecorations{{
		File: &srvpb.File{Text: []byte("some def\n")},
		Decoration: []*srvpb.FileDecorations_Decoration{{
			Anchor:           &srvpb.RawAnchor{StartOffset: 4, EndOffset: 7},
			Kind:             "/kythe/edge/defines/binding",
			Target:           "kythe:#node2",
			TargetDefinition: "kythe:?path=path2#def2",
		}, {
			Anchor:           &srvpb.RawAnchor{StartOffset: 5, EndOffset: 8},
			Kind:             "/kythe/edge/defines/binding",
			Target:           "kythe:#node1",
			TargetDefinition: "kythe:?path=path2#def1",
		}},
		Target: []*srvpb.Node{{
			Ticket: "kythe:#node2",
			Fact:   []*cpb.Fact{{Name: "/kythe/node/kind", Value: []byte("function")}},
		}},
		TargetDefinitions: []*srvpb.ExpandedAnchor{{
			Ticket: "kythe:?path=path2#def1",
			Text:   "def",
			Span: &cpb.Span{
				Start: &cpb.Point{ByteOffset: 5, LineNumber: 1, ColumnOffset: 5},
				End:   &cpb.Point{ByteOffset: 8, LineNumber: 1, ColumnOffset: 8},
			},
			Snippet: "some def",
			SnippetSpan: &cpb.Span{
				Start: &cpb.Point{LineNumber: 1},
				End:   &cpb.Point{ByteOffset: 8, LineNumber: 1, ColumnOffset: 8},
			},
		}, {
			Ticket: "kythe:?path=path2#def2",
			Text:   " de",
			Span: &cpb.Span{
				Start: &cpb.Point{ByteOffset: 4, LineNumber: 1, ColumnOffset: 4},
				End:   &cpb.Point{ByteOffset: 7, LineNumber: 1, ColumnOffset: 7},
			},
			Snippet: "some def",
			SnippetSpan: &cpb.Span{
				Start: &cpb.Point{LineNumber: 1},
				End:   &cpb.Point{ByteOffset: 8, LineNumber: 1, ColumnOffset: 8},
			},
		}},
		TargetOverride: []*srvpb.FileDecorations_Override{{
			Overriding:           "kythe:#node2",
			Overridden:           "kythe:#node1",
			OverriddenDefinition: "kythe:?path=path2#def1",
		}},
	}, {
		File: &srvpb.File{Text: []byte("some text\n")},
	}}

	beam.Init()
	p, s, nodes := ptest.CreateList(testNodes)
	decor := FromNodes(s, nodes).Decorations()
	debug.Print(s, decor)
	passert.Equals(s, beam.DropKey(s, decor), beam.CreateList(s, expected))

	ptest.RunAndValidate(t, p)
}

func TestCrossReferences(t *testing.T) {
	testNodes := []*scpb.Node{{
		Source: &spb.VName{Signature: "node1"},
	}}
	testRefs := []*ppb.Reference{{
		Source: &spb.VName{Signature: "node1"},
		Kind:   &ppb.Reference_KytheKind{scpb.EdgeKind_REF},
		Anchor: &srvpb.ExpandedAnchor{
			Ticket: "kythe:?path=path#anchor1",
			Text:   "some",
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
		Source: &spb.VName{Signature: "node1"},
		Kind:   &ppb.Reference_KytheKind{scpb.EdgeKind_REF},
		Anchor: &srvpb.ExpandedAnchor{
			Ticket: "kythe:?path=path2#anchor3",
			Span: &cpb.Span{
				Start: &cpb.Point{ByteOffset: 42},
				End:   &cpb.Point{ByteOffset: 45},
			},
		},
	}, {
		Source: &spb.VName{Signature: "node2"},
		Kind:   &ppb.Reference_KytheKind{scpb.EdgeKind_REF_CALL},
		Scope:  &spb.VName{Path: "path", Signature: "anchor2_parent"},
		Anchor: &srvpb.ExpandedAnchor{
			Ticket: "kythe:?path=path#anchor2",
			Text:   "text",
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
	}, {
		Source: &spb.VName{Path: "path", Signature: "anchor2_parent"},
		Kind:   &ppb.Reference_KytheKind{scpb.EdgeKind_DEFINES_BINDING},
		Anchor: &srvpb.ExpandedAnchor{
			Ticket: "kythe:?path=path#anchor3",
			Text:   "text",
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
	expectedSets := []*srvpb.PagedCrossReferences{{
		SourceTicket: "kythe:#node1",
		Group: []*srvpb.PagedCrossReferences_Group{{
			Kind: "/kythe/edge/ref",
			Anchor: []*srvpb.ExpandedAnchor{{
				Ticket: "kythe:?path=path#anchor1",
				Text:   "some",
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
			}, {
				Ticket: "kythe:?path=path2#anchor3",
				Span: &cpb.Span{
					Start: &cpb.Point{ByteOffset: 42},
					End:   &cpb.Point{ByteOffset: 45},
				},
			}},
		}},
	}, {
		SourceTicket: "kythe:#node2",
		Group: []*srvpb.PagedCrossReferences_Group{{
			Kind: "#internal/ref/call/direct",
			Caller: []*srvpb.PagedCrossReferences_Caller{{
				SemanticCaller: "kythe:?path=path#anchor2_parent",
				Caller: &srvpb.ExpandedAnchor{
					Ticket: "kythe:?path=path#anchor3",
					Text:   "text",
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
				Callsite: []*srvpb.ExpandedAnchor{{
					Ticket: "kythe:?path=path#anchor2",
					Text:   "text",
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
				}},
			}},
		}, {
			Kind: "/kythe/edge/ref/call",
			Anchor: []*srvpb.ExpandedAnchor{{
				Ticket: "kythe:?path=path#anchor2",
				Text:   "text",
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
			}},
		}},
	}, {
		SourceTicket: "kythe:?path=path#anchor2_parent",
		Group: []*srvpb.PagedCrossReferences_Group{{
			Kind: "/kythe/edge/defines/binding",
			Anchor: []*srvpb.ExpandedAnchor{{
				Ticket: "kythe:?path=path#anchor3",
				Text:   "text",
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
			}},
		}},
	}}

	beam.Init()
	p, s, refs, nodes := ptest.CreateList2(testRefs, testNodes)
	k := &KytheBeam{s: s, refs: refs, nodes: nodes}
	sets, _ := k.CrossReferences()
	debug.Print(s, sets)
	passert.Equals(s, beam.DropKey(s, sets), beam.CreateList(s, expectedSets))

	ptest.RunAndValidate(t, p)
}

func TestEdges_grouping(t *testing.T) {
	testNodes := []*scpb.Node{{
		Source: &spb.VName{Signature: "node1"},
		Edge: []*scpb.Edge{{
			Kind:   &scpb.Edge_KytheKind{scpb.EdgeKind_PARAM},
			Target: &spb.VName{Signature: "node1"},
		}, {
			Kind:    &scpb.Edge_KytheKind{scpb.EdgeKind_PARAM},
			Ordinal: 1,
			Target:  &spb.VName{Signature: "node1"},
		}},
	}}
	expectedSets := []*srvpb.PagedEdgeSet{{
		Source: &srvpb.Node{Ticket: "kythe:#node1"},
		Group: []*srvpb.EdgeGroup{{
			Kind: "%/kythe/edge/param",
			Edge: []*srvpb.EdgeGroup_Edge{{
				Target: &srvpb.Node{Ticket: "kythe:#node1"},
			}, {
				Target:  &srvpb.Node{Ticket: "kythe:#node1"},
				Ordinal: 1,
			}},
		}, {
			Kind: "/kythe/edge/param",
			Edge: []*srvpb.EdgeGroup_Edge{{
				Target: &srvpb.Node{Ticket: "kythe:#node1"},
			}, {
				Target:  &srvpb.Node{Ticket: "kythe:#node1"},
				Ordinal: 1,
			}},
		}},
	}}

	beam.Init()
	p, s, nodes := ptest.CreateList(testNodes)
	k := FromNodes(s, nodes)
	sets, _ := k.Edges()
	debug.Print(s, sets)
	passert.Equals(s, beam.DropKey(s, sets), beam.CreateList(s, expectedSets))

	ptest.RunAndValidate(t, p)
}

func TestEdges_reverses(t *testing.T) {
	testNodes := []*scpb.Node{{
		Source: &spb.VName{Signature: "node1"},
		Kind:   &scpb.Node_KytheKind{scpb.NodeKind_RECORD},
		Edge: []*scpb.Edge{{
			Kind:   &scpb.Edge_KytheKind{scpb.EdgeKind_CHILD_OF},
			Target: &spb.VName{Signature: "node2"},
		}},
	}}
	expectedSets := []*srvpb.PagedEdgeSet{{
		Source: &srvpb.Node{
			Ticket: "kythe:#node1",
			Fact: []*cpb.Fact{{
				Name:  "/kythe/node/kind",
				Value: []byte("record"),
			}},
		},
		Group: []*srvpb.EdgeGroup{{
			Kind: "/kythe/edge/childof",
			Edge: []*srvpb.EdgeGroup_Edge{{
				Target: &srvpb.Node{Ticket: "kythe:#node2"},
			}},
		}},
	}, {
		Source: &srvpb.Node{Ticket: "kythe:#node2"},
		Group: []*srvpb.EdgeGroup{{
			Kind: "%/kythe/edge/childof",
			Edge: []*srvpb.EdgeGroup_Edge{{
				Target: &srvpb.Node{
					Ticket: "kythe:#node1",
					Fact: []*cpb.Fact{{
						Name:  "/kythe/node/kind",
						Value: []byte("record"),
					}},
				},
			}},
		}},
	}}

	beam.Init()
	p, s, nodes := ptest.CreateList(testNodes)
	k := FromNodes(s, nodes)
	sets, _ := k.Edges()
	debug.Print(s, sets)
	passert.Equals(s, beam.DropKey(s, sets), beam.CreateList(s, expectedSets))

	ptest.RunAndValidate(t, p)
}

func TestDocuments_text(t *testing.T) {
	testNodes := []*scpb.Node{{
		Source: &spb.VName{Signature: "doc1"},
		Kind:   &scpb.Node_KytheKind{scpb.NodeKind_DOC},
		Fact: []*scpb.Fact{{
			Name:  &scpb.Fact_KytheName{scpb.FactName_TEXT},
			Value: []byte("raw document text"),
		}},
		Edge: []*scpb.Edge{{
			Kind:   &scpb.Edge_KytheKind{scpb.EdgeKind_DOCUMENTS},
			Target: &spb.VName{Signature: "node1"},
		}},
	}}
	expectedDocs := []*srvpb.Document{{
		Ticket:  "kythe:#node1",
		RawText: "raw document text",
	}}

	beam.Init()
	p, s, nodes := ptest.CreateList(testNodes)
	docs := FromNodes(s, nodes).Documents()
	debug.Print(s, docs)
	passert.Equals(s, beam.DropKey(s, docs), beam.CreateList(s, expectedDocs))

	ptest.RunAndValidate(t, p)
}

func TestDocuments_children(t *testing.T) {
	testNodes := []*scpb.Node{{
		Source: &spb.VName{Signature: "child1"},
		Edge: []*scpb.Edge{{
			Kind:   &scpb.Edge_KytheKind{scpb.EdgeKind_CHILD_OF},
			Target: &spb.VName{Signature: "node1"},
		}},
	}, {
		Source: &spb.VName{Signature: "doc1"},
		Kind:   &scpb.Node_KytheKind{scpb.NodeKind_DOC},
		Edge: []*scpb.Edge{{
			Kind:   &scpb.Edge_KytheKind{scpb.EdgeKind_DOCUMENTS},
			Target: &spb.VName{Signature: "node1"},
		}},
	}}
	expectedDocs := []*srvpb.Document{{
		Ticket:      "kythe:#node1",
		ChildTicket: []string{"kythe:#child1"},
	}}

	beam.Init()
	p, s, nodes := ptest.CreateList(testNodes)
	docs := FromNodes(s, nodes).Documents()
	debug.Print(s, docs)
	passert.Equals(s, beam.DropKey(s, docs), beam.CreateList(s, expectedDocs))

	ptest.RunAndValidate(t, p)
}

func TestDocuments_markedSource(t *testing.T) {
	ms := &cpb.MarkedSource{
		Kind:    cpb.MarkedSource_IDENTIFIER,
		PreText: "Some_MarkedSource_identifier",
	}
	rec, err := proto.Marshal(ms)
	if err != nil {
		t.Fatal(err)
	}

	testNodes := []*scpb.Node{{
		Source: &spb.VName{Signature: "node1"},
		Fact: []*scpb.Fact{{
			Name:  &scpb.Fact_KytheName{scpb.FactName_CODE},
			Value: rec,
		}},
	}, {
		Source: &spb.VName{Signature: "doc1"},
		Kind:   &scpb.Node_KytheKind{scpb.NodeKind_DOC},
		Edge: []*scpb.Edge{{
			Kind:   &scpb.Edge_KytheKind{scpb.EdgeKind_DOCUMENTS},
			Target: &spb.VName{Signature: "node1"},
		}},
	}}
	expectedDocs := []*srvpb.Document{{
		Ticket:       "kythe:#node1",
		MarkedSource: ms,
	}}

	beam.Init()
	p, s, nodes := ptest.CreateList(testNodes)
	docs := FromNodes(s, nodes).Documents()
	debug.Print(s, docs)
	passert.Equals(s, beam.DropKey(s, docs), beam.CreateList(s, expectedDocs))

	ptest.RunAndValidate(t, p)
}

func TestFileTree_registrations(t *testing.T) {
	testNodes := []*scpb.Node{{}}
	p, s, nodes := ptest.CreateList(testNodes)
	k := FromNodes(s, nodes)
	k.CorpusRoots()
	k.Directories()
	beamtest.CheckRegistrations(t, p)
}

func TestDecorations_registrations(t *testing.T) {
	testNodes := []*scpb.Node{{}}
	p, s, nodes := ptest.CreateList(testNodes)
	k := FromNodes(s, nodes)
	k.Decorations()
	k.SplitDecorations()
	beamtest.CheckRegistrations(t, p)
}

func TestCrossReferences_registrations(t *testing.T) {
	testNodes := []*scpb.Node{{}}
	p, s, nodes := ptest.CreateList(testNodes)
	k := FromNodes(s, nodes)
	k.CrossReferences()
	k.SplitCrossReferences()
	beamtest.CheckRegistrations(t, p)
}

func TestEdges_registrations(t *testing.T) {
	testNodes := []*scpb.Node{{}}
	p, s, nodes := ptest.CreateList(testNodes)
	k := FromNodes(s, nodes)
	k.Edges()
	k.SplitEdges()
	beamtest.CheckRegistrations(t, p)
}

func TestDocuments_registrations(t *testing.T) {
	testNodes := []*scpb.Node{{}}
	p, s, nodes := ptest.CreateList(testNodes)
	FromNodes(s, nodes).Documents()
	beamtest.CheckRegistrations(t, p)
}

func TestCombineDecorPieces_mergeAccumulators(t *testing.T) {
	accum := &srvpb.FileDecorations{
		Decoration: []*srvpb.FileDecorations_Decoration{{
			Target: "target1",
		}},
		Target: []*srvpb.Node{{
			Ticket: "ticket1",
		}},
		TargetDefinitions: []*srvpb.ExpandedAnchor{{
			Ticket: "ticket1",
		}},
		Diagnostic: []*cpb.Diagnostic{{
			Message: "msg1",
		}},
	}

	other := &srvpb.FileDecorations{
		Decoration: []*srvpb.FileDecorations_Decoration{{
			Target: "target2",
		}},
		Target: []*srvpb.Node{{
			Ticket: "ticket2",
		}},
		TargetDefinitions: []*srvpb.ExpandedAnchor{{
			Ticket: "ticket2",
		}},
		Diagnostic: []*cpb.Diagnostic{{
			Message: "msg2",
		}},
		File: &srvpb.File{
			Text: []byte("some text\n"),
		},
	}

	expected := &srvpb.FileDecorations{
		Decoration: []*srvpb.FileDecorations_Decoration{{
			Target: "target1",
		}, {
			Target: "target2",
		}},
		Target: []*srvpb.Node{{
			Ticket: "ticket1",
		}, {
			Ticket: "ticket2",
		}},
		TargetDefinitions: []*srvpb.ExpandedAnchor{{
			Ticket: "ticket1",
		}, {
			Ticket: "ticket2",
		}},
		Diagnostic: []*cpb.Diagnostic{{
			Message: "msg1",
		}, {
			Message: "msg2",
		}},
		File: &srvpb.File{
			Text: []byte("some text\n"),
		},
	}

	actual := (&combineDecorPieces{}).MergeAccumulators(accum, other)
	if d := cmp.Diff(actual, expected, cmpopts.IgnoreUnexported(cpb.Diagnostic{}, srvpb.ExpandedAnchor{}, srvpb.Node{}, srvpb.FileDecorations_Decoration{}, srvpb.File{}, srvpb.FileDecorations{})); d != "" {
		t.Errorf("Expected %v but was %v. Diff: %v", expected, actual, d)
	}
}
