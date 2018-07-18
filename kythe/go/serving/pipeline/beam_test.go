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
	"github.com/golang/protobuf/proto"

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

	p, s, nodes := ptest.CreateList(testNodes)
	refs := FromNodes(s, nodes).References()
	debug.Print(s, refs)
	passert.Equals(s, refs, beam.CreateList(s, expected))

	if err := ptest.Run(p); err != nil {
		t.Fatalf("Pipeline error: %+v", err)
	}
}

func TestDecorations_targetNode(t *testing.T) {
	testNodes := []*ppb.Node{{
		Source: &spb.VName{Path: "path", Signature: "anchor1"},
		Kind:   &ppb.Node_KytheKind{scpb.NodeKind_ANCHOR},
		Fact: []*ppb.Fact{{
			Name:  &ppb.Fact_KytheName{scpb.FactName_LOC_START},
			Value: []byte("5"),
		}, {
			Name:  &ppb.Fact_KytheName{scpb.FactName_LOC_END},
			Value: []byte("9"),
		}},
		Edge: []*ppb.Edge{{
			Kind:   &ppb.Edge_KytheKind{scpb.EdgeKind_REF},
			Target: &spb.VName{Signature: "node1"},
		}},
	}, {
		Source: &spb.VName{Path: "path"},
		Kind:   &ppb.Node_KytheKind{scpb.NodeKind_FILE},
		Fact: []*ppb.Fact{{
			Name:  &ppb.Fact_KytheName{scpb.FactName_TEXT},
			Value: []byte("some text\n"),
		}},
	}, {
		Source: &spb.VName{Signature: "node1"},
		Kind:   &ppb.Node_KytheKind{scpb.NodeKind_RECORD},
		Fact: []*ppb.Fact{{
			Name:  &ppb.Fact_GenericName{"/unknown/fact/name"},
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

	p, s, nodes := ptest.CreateList(testNodes)
	decor := FromNodes(s, nodes).Decorations()
	debug.Print(s, decor)
	passert.Equals(s, beam.DropKey(s, decor), beam.CreateList(s, expected))

	if err := ptest.Run(p); err != nil {
		t.Fatalf("Pipeline error: %+v", err)
	}
}

func TestDecorations_decoration(t *testing.T) {
	testNodes := []*ppb.Node{{
		Source: &spb.VName{Path: "path", Signature: "anchor1"},
		Kind:   &ppb.Node_KytheKind{scpb.NodeKind_ANCHOR},
		Fact: []*ppb.Fact{{
			Name:  &ppb.Fact_KytheName{scpb.FactName_LOC_START},
			Value: []byte("5"),
		}, {
			Name:  &ppb.Fact_KytheName{scpb.FactName_LOC_END},
			Value: []byte("9"),
		}},
		Edge: []*ppb.Edge{{
			Kind:   &ppb.Edge_KytheKind{scpb.EdgeKind_REF},
			Target: &spb.VName{Signature: "node1"},
		}},
	}, {
		Source: &spb.VName{Path: "path"},
		Kind:   &ppb.Node_KytheKind{scpb.NodeKind_FILE},
		Fact: []*ppb.Fact{{
			Name:  &ppb.Fact_KytheName{scpb.FactName_TEXT},
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

	p, s, nodes := ptest.CreateList(testNodes)
	decor := FromNodes(s, nodes).Decorations()
	debug.Print(s, decor)
	passert.Equals(s, beam.DropKey(s, decor), beam.CreateList(s, expected))

	if err := ptest.Run(p); err != nil {
		t.Fatalf("Pipeline error: %+v", err)
	}
}

func TestDecorations_targetDefinition(t *testing.T) {
	testNodes := []*ppb.Node{{
		Source: &spb.VName{Path: "path", Signature: "anchor1"},
		Kind:   &ppb.Node_KytheKind{scpb.NodeKind_ANCHOR},
		Fact: []*ppb.Fact{{
			Name:  &ppb.Fact_KytheName{scpb.FactName_LOC_START},
			Value: []byte("5"),
		}, {
			Name:  &ppb.Fact_KytheName{scpb.FactName_LOC_END},
			Value: []byte("9"),
		}},
		Edge: []*ppb.Edge{{
			Kind:   &ppb.Edge_KytheKind{scpb.EdgeKind_REF},
			Target: &spb.VName{Signature: "node1"},
		}},
	}, {
		Source: &spb.VName{Path: "path"},
		Kind:   &ppb.Node_KytheKind{scpb.NodeKind_FILE},
		Fact: []*ppb.Fact{{
			Name:  &ppb.Fact_KytheName{scpb.FactName_TEXT},
			Value: []byte("some text\n"),
		}},
	}, {
		Source: &spb.VName{Path: "path2", Signature: "def1"},
		Kind:   &ppb.Node_KytheKind{scpb.NodeKind_ANCHOR},
		Fact: []*ppb.Fact{{
			Name:  &ppb.Fact_KytheName{scpb.FactName_LOC_START},
			Value: []byte("5"),
		}, {
			Name:  &ppb.Fact_KytheName{scpb.FactName_LOC_END},
			Value: []byte("8"),
		}},
		Edge: []*ppb.Edge{{
			Kind:   &ppb.Edge_KytheKind{scpb.EdgeKind_DEFINES_BINDING},
			Target: &spb.VName{Signature: "node1"},
		}},
	}, {
		Source: &spb.VName{Path: "path2"},
		Kind:   &ppb.Node_KytheKind{scpb.NodeKind_FILE},
		Fact: []*ppb.Fact{{
			Name:  &ppb.Fact_KytheName{scpb.FactName_TEXT},
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

	p, s, nodes := ptest.CreateList(testNodes)
	decor := FromNodes(s, nodes).Decorations()
	debug.Print(s, decor)
	passert.Equals(s, beam.DropKey(s, decor), beam.CreateList(s, expected))

	if err := ptest.Run(p); err != nil {
		t.Fatalf("Pipeline error: %+v", err)
	}
}

func TestCrossReferences(t *testing.T) {
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
	}}

	p, s, refs := ptest.CreateList(testRefs)
	k := &KytheBeam{s: s, refs: refs}
	sets, _ := k.CrossReferences()
	debug.Print(s, sets)
	passert.Equals(s, beam.DropKey(s, sets), beam.CreateList(s, expectedSets))

	if err := ptest.Run(p); err != nil {
		t.Fatalf("Pipeline error: %+v", err)
	}
}

func TestEdges_grouping(t *testing.T) {
	testNodes := []*ppb.Node{{
		Source: &spb.VName{Signature: "node1"},
		Edge: []*ppb.Edge{{
			Kind:   &ppb.Edge_KytheKind{scpb.EdgeKind_PARAM},
			Target: &spb.VName{Signature: "node1"},
		}, {
			Kind:    &ppb.Edge_KytheKind{scpb.EdgeKind_PARAM},
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

	p, s, nodes := ptest.CreateList(testNodes)
	k := FromNodes(s, nodes)
	sets, _ := k.Edges()
	debug.Print(s, sets)
	passert.Equals(s, beam.DropKey(s, sets), beam.CreateList(s, expectedSets))

	if err := ptest.Run(p); err != nil {
		t.Fatalf("Pipeline error: %+v", err)
	}
}

func TestEdges_reverses(t *testing.T) {
	testNodes := []*ppb.Node{{
		Source: &spb.VName{Signature: "node1"},
		Kind:   &ppb.Node_KytheKind{scpb.NodeKind_RECORD},
		Edge: []*ppb.Edge{{
			Kind:   &ppb.Edge_KytheKind{scpb.EdgeKind_CHILD_OF},
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

	p, s, nodes := ptest.CreateList(testNodes)
	k := FromNodes(s, nodes)
	sets, _ := k.Edges()
	debug.Print(s, sets)
	passert.Equals(s, beam.DropKey(s, sets), beam.CreateList(s, expectedSets))

	if err := ptest.Run(p); err != nil {
		t.Fatalf("Pipeline error: %+v", err)
	}
}

func TestFileTree_registrations(t *testing.T) {
	testNodes := []*ppb.Node{{}}
	p, s, nodes := ptest.CreateList(testNodes)
	k := FromNodes(s, nodes)
	k.CorpusRoots()
	k.Directories()
	if err := beamtest.CheckRegistrations(p); err != nil {
		t.Fatal(err)
	}
}

func TestDocuments_text(t *testing.T) {
	testNodes := []*ppb.Node{{
		Source: &spb.VName{Signature: "doc1"},
		Kind:   &ppb.Node_KytheKind{scpb.NodeKind_DOC},
		Fact: []*ppb.Fact{{
			Name:  &ppb.Fact_KytheName{scpb.FactName_TEXT},
			Value: []byte("raw document text"),
		}},
		Edge: []*ppb.Edge{{
			Kind:   &ppb.Edge_KytheKind{scpb.EdgeKind_DOCUMENTS},
			Target: &spb.VName{Signature: "node1"},
		}},
	}}
	expectedDocs := []*srvpb.Document{{
		Ticket:  "kythe:#node1",
		RawText: "raw document text",
	}}

	p, s, nodes := ptest.CreateList(testNodes)
	docs := FromNodes(s, nodes).Documents()
	debug.Print(s, docs)
	passert.Equals(s, beam.DropKey(s, docs), beam.CreateList(s, expectedDocs))

	if err := ptest.Run(p); err != nil {
		t.Fatalf("Pipeline error: %+v", err)
	}
}

func TestDocuments_children(t *testing.T) {
	testNodes := []*ppb.Node{{
		Source: &spb.VName{Signature: "child1"},
		Edge: []*ppb.Edge{{
			Kind:   &ppb.Edge_KytheKind{scpb.EdgeKind_CHILD_OF},
			Target: &spb.VName{Signature: "node1"},
		}},
	}, {
		Source: &spb.VName{Signature: "doc1"},
		Kind:   &ppb.Node_KytheKind{scpb.NodeKind_DOC},
		Edge: []*ppb.Edge{{
			Kind:   &ppb.Edge_KytheKind{scpb.EdgeKind_DOCUMENTS},
			Target: &spb.VName{Signature: "node1"},
		}},
	}}
	expectedDocs := []*srvpb.Document{{
		Ticket:      "kythe:#node1",
		ChildTicket: []string{"kythe:#child1"},
	}}

	p, s, nodes := ptest.CreateList(testNodes)
	docs := FromNodes(s, nodes).Documents()
	debug.Print(s, docs)
	passert.Equals(s, beam.DropKey(s, docs), beam.CreateList(s, expectedDocs))

	if err := ptest.Run(p); err != nil {
		t.Fatalf("Pipeline error: %+v", err)
	}
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

	testNodes := []*ppb.Node{{
		Source: &spb.VName{Signature: "node1"},
		Fact: []*ppb.Fact{{
			Name:  &ppb.Fact_KytheName{scpb.FactName_CODE},
			Value: rec,
		}},
	}, {
		Source: &spb.VName{Signature: "doc1"},
		Kind:   &ppb.Node_KytheKind{scpb.NodeKind_DOC},
		Edge: []*ppb.Edge{{
			Kind:   &ppb.Edge_KytheKind{scpb.EdgeKind_DOCUMENTS},
			Target: &spb.VName{Signature: "node1"},
		}},
	}}
	expectedDocs := []*srvpb.Document{{
		Ticket:       "kythe:#node1",
		MarkedSource: ms,
	}}

	p, s, nodes := ptest.CreateList(testNodes)
	docs := FromNodes(s, nodes).Documents()
	debug.Print(s, docs)
	passert.Equals(s, beam.DropKey(s, docs), beam.CreateList(s, expectedDocs))

	if err := ptest.Run(p); err != nil {
		t.Fatalf("Pipeline error: %+v", err)
	}
}

func TestDecorations_registrations(t *testing.T) {
	testNodes := []*ppb.Node{{}}
	p, s, nodes := ptest.CreateList(testNodes)
	FromNodes(s, nodes).Decorations()
	if err := beamtest.CheckRegistrations(p); err != nil {
		t.Fatal(err)
	}
}

func TestCrossReferences_registrations(t *testing.T) {
	testNodes := []*ppb.Node{{}}
	p, s, nodes := ptest.CreateList(testNodes)
	FromNodes(s, nodes).CrossReferences()
	if err := beamtest.CheckRegistrations(p); err != nil {
		t.Fatal(err)
	}
}

func TestEdges_registrations(t *testing.T) {
	testNodes := []*ppb.Node{{}}
	p, s, nodes := ptest.CreateList(testNodes)
	FromNodes(s, nodes).Edges()
	if err := beamtest.CheckRegistrations(p); err != nil {
		t.Fatal(err)
	}
}

func TestDocuments_registrations(t *testing.T) {
	testNodes := []*ppb.Node{{}}
	p, s, nodes := ptest.CreateList(testNodes)
	FromNodes(s, nodes).Documents()
	if err := beamtest.CheckRegistrations(p); err != nil {
		t.Fatal(err)
	}
}
