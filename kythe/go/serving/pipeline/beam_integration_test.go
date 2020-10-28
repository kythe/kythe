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
	"context"
	"testing"

	"kythe.io/kythe/go/services/graph"
	"kythe.io/kythe/go/services/xrefs"
	gsrv "kythe.io/kythe/go/serving/graph"
	xsrv "kythe.io/kythe/go/serving/xrefs"
	"kythe.io/kythe/go/storage/inmemory"
	"kythe.io/kythe/go/storage/keyvalue"
	"kythe.io/kythe/go/util/compare"
	"kythe.io/kythe/go/util/kytheuri"
	"kythe.io/kythe/go/util/schema/edges"
	"kythe.io/kythe/go/util/schema/facts"
	"kythe.io/kythe/go/util/schema/nodes"

	"github.com/apache/beam/sdks/go/pkg/beam"
	"github.com/apache/beam/sdks/go/pkg/beam/testing/ptest"
	"google.golang.org/protobuf/proto"

	cpb "kythe.io/kythe/proto/common_go_proto"
	gpb "kythe.io/kythe/proto/graph_go_proto"
	scpb "kythe.io/kythe/proto/schema_go_proto"
	spb "kythe.io/kythe/proto/storage_go_proto"
	xpb "kythe.io/kythe/proto/xref_go_proto"
)

var ctx = context.Background()

func encodeMarkedSource(ms *cpb.MarkedSource) []byte {
	rec, err := proto.Marshal(ms)
	if err != nil {
		panic(err)
	}
	return rec
}

func TestServingSimpleDecorations(t *testing.T) {
	file := &spb.VName{Path: "path"}
	const expectedText = "some text\n"
	testNodes := []*scpb.Node{{
		Source: file,
		Kind:   &scpb.Node_KytheKind{scpb.NodeKind_FILE},
		Fact: []*scpb.Fact{{
			Name:  &scpb.Fact_KytheName{scpb.FactName_TEXT},
			Value: []byte(expectedText),
		}, {
			Name:  &scpb.Fact_KytheName{scpb.FactName_TEXT_ENCODING},
			Value: []byte("ascii"),
		}},
		Edge: []*scpb.Edge{{
			Kind:   &scpb.Edge_KytheKind{scpb.EdgeKind_TAGGED},
			Target: &spb.VName{Signature: "diagnostic"},
		}},
	}, {
		Source: &spb.VName{Path: "path", Signature: "anchor1"},
		Kind:   &scpb.Node_KytheKind{scpb.NodeKind_ANCHOR},
		Fact: []*scpb.Fact{{
			Name:  &scpb.Fact_KytheName{scpb.FactName_LOC_START},
			Value: []byte("0"),
		}, {
			Name:  &scpb.Fact_KytheName{scpb.FactName_LOC_END},
			Value: []byte("4"),
		}},
		Edge: []*scpb.Edge{{
			Kind:   &scpb.Edge_KytheKind{scpb.EdgeKind_REF},
			Target: &spb.VName{Signature: "simpleDecor"},
		}},
	}, {
		Source: &spb.VName{Signature: "simpleDecor"},
		Kind:   &scpb.Node_KytheKind{scpb.NodeKind_RECORD},
	}, {
		Source: &spb.VName{Path: "path", Signature: "anchor2"},
		Kind:   &scpb.Node_KytheKind{scpb.NodeKind_ANCHOR},
		Fact: []*scpb.Fact{{
			Name:  &scpb.Fact_KytheName{scpb.FactName_LOC_START},
			Value: []byte("5"),
		}, {
			Name:  &scpb.Fact_KytheName{scpb.FactName_LOC_END},
			Value: []byte("9"),
		}, {
			Name:  &scpb.Fact_KytheName{scpb.FactName_BUILD_CONFIG},
			Value: []byte("test-build-config"),
		}},
		Edge: []*scpb.Edge{{
			Kind:   &scpb.Edge_KytheKind{scpb.EdgeKind_REF},
			Target: &spb.VName{Signature: "decorWithDef"},
		}},
	}, {
		Source: &spb.VName{Signature: "def1"},
		Kind:   &scpb.Node_KytheKind{scpb.NodeKind_ANCHOR},
		Fact: []*scpb.Fact{{
			Name:  &scpb.Fact_KytheName{scpb.FactName_LOC_START},
			Value: []byte("0"),
		}, {
			Name:  &scpb.Fact_KytheName{scpb.FactName_LOC_END},
			Value: []byte("3"),
		}},
		Edge: []*scpb.Edge{{
			Kind:   &scpb.Edge_KytheKind{scpb.EdgeKind_DEFINES},
			Target: &spb.VName{Signature: "decorWithDef"},
		}},
	}, {
		Source: &spb.VName{},
		Kind:   &scpb.Node_KytheKind{scpb.NodeKind_FILE},
		Fact: []*scpb.Fact{{
			Name:  &scpb.Fact_KytheName{scpb.FactName_TEXT},
			Value: []byte("def\n"),
		}},
	}, {
		Source: &spb.VName{Signature: "diagnostic"},
		Kind:   &scpb.Node_KytheKind{scpb.NodeKind_DIAGNOSTIC},
		Fact: []*scpb.Fact{{
			Name:  &scpb.Fact_KytheName{scpb.FactName_MESSAGE},
			Value: []byte("msg"),
		}, {
			Name:  &scpb.Fact_KytheName{scpb.FactName_DETAILS},
			Value: []byte("dtails"),
		}, {
			Name:  &scpb.Fact_KytheName{scpb.FactName_CONTEXT_URL},
			Value: []byte("https://kythe.io/schema"),
		}},
	}}

	p, s, nodes := ptest.CreateList(testNodes)
	decor := FromNodes(s, nodes).SplitDecorations()

	db := inmemory.NewKeyValueDB()
	w, err := db.Writer(ctx)
	if err != nil {
		t.Fatal(err)
	}

	// Mark table as columnar
	if err := w.Write([]byte(xsrv.ColumnarTableKeyMarker), []byte{}); err != nil {
		t.Fatal(err)
	}
	// Write columnar data to inmemory.KeyValueDB
	beam.ParDo(s, &writeTo{w}, decor)

	if err := ptest.Run(p); err != nil {
		t.Fatalf("Pipeline error: %+v", err)
	} else if err := w.Close(); err != nil {
		t.Fatal(err)
	}

	xs := xsrv.NewService(ctx, db)
	fileTicket := kytheuri.ToString(file)

	t.Run("source_text", makeDecorTestCase(ctx, xs, &xpb.DecorationsRequest{
		Location:   &xpb.Location{Ticket: fileTicket},
		SourceText: true,
	}, &xpb.DecorationsReply{
		Location:   &xpb.Location{Ticket: fileTicket},
		SourceText: []byte(expectedText),
		Encoding:   "ascii",
	}))

	t.Run("references", makeDecorTestCase(ctx, xs, &xpb.DecorationsRequest{
		Location:   &xpb.Location{Ticket: fileTicket},
		References: true,
	}, &xpb.DecorationsReply{
		Location: &xpb.Location{Ticket: fileTicket},
		Reference: []*xpb.DecorationsReply_Reference{{
			Span: &cpb.Span{
				Start: &cpb.Point{
					LineNumber: 1,
				},
				End: &cpb.Point{
					ByteOffset:   4,
					ColumnOffset: 4,
					LineNumber:   1,
				},
			},
			Kind:         "/kythe/edge/ref",
			TargetTicket: "kythe:#simpleDecor",
		}, {
			Span: &cpb.Span{
				Start: &cpb.Point{
					ByteOffset:   5,
					ColumnOffset: 5,
					LineNumber:   1,
				},
				End: &cpb.Point{
					ByteOffset:   9,
					ColumnOffset: 9,
					LineNumber:   1,
				},
			},
			BuildConfig:  "test-build-config",
			Kind:         "/kythe/edge/ref",
			TargetTicket: "kythe:#decorWithDef",
			// TargetDefinition: explicitly not requested
		}},
		// Nodes: not requested
		// DefinitionLocations: not requested
	}))

	t.Run("referenced_nodes", makeDecorTestCase(ctx, xs, &xpb.DecorationsRequest{
		Location:   &xpb.Location{Ticket: fileTicket},
		References: true,
		Filter:     []string{"**"},
	}, &xpb.DecorationsReply{
		Location: &xpb.Location{Ticket: fileTicket},
		Reference: []*xpb.DecorationsReply_Reference{{
			Span: &cpb.Span{
				Start: &cpb.Point{
					LineNumber: 1,
				},
				End: &cpb.Point{
					ByteOffset:   4,
					ColumnOffset: 4,
					LineNumber:   1,
				},
			},
			Kind:         "/kythe/edge/ref",
			TargetTicket: "kythe:#simpleDecor",
		}, {
			Span: &cpb.Span{
				Start: &cpb.Point{
					ByteOffset:   5,
					ColumnOffset: 5,
					LineNumber:   1,
				},
				End: &cpb.Point{
					ByteOffset:   9,
					ColumnOffset: 9,
					LineNumber:   1,
				},
			},
			BuildConfig:  "test-build-config",
			Kind:         "/kythe/edge/ref",
			TargetTicket: "kythe:#decorWithDef",
			// TargetDefinition: explicitly not requested
		}},
		Nodes: map[string]*cpb.NodeInfo{
			"kythe:#simpleDecor": {
				Facts: map[string][]byte{
					"/kythe/node/kind": []byte("record"),
				},
			},
		},
		// DefinitionLocations: not requested
	}))

	t.Run("target_definitions", makeDecorTestCase(ctx, xs, &xpb.DecorationsRequest{
		Location:          &xpb.Location{Ticket: fileTicket},
		References:        true,
		TargetDefinitions: true,
	}, &xpb.DecorationsReply{
		Location: &xpb.Location{Ticket: fileTicket},
		Reference: []*xpb.DecorationsReply_Reference{{
			Span: &cpb.Span{
				Start: &cpb.Point{
					LineNumber: 1,
				},
				End: &cpb.Point{
					ByteOffset:   4,
					ColumnOffset: 4,
					LineNumber:   1,
				},
			},
			Kind:         "/kythe/edge/ref",
			TargetTicket: "kythe:#simpleDecor",
		}, {
			Span: &cpb.Span{
				Start: &cpb.Point{
					ByteOffset:   5,
					ColumnOffset: 5,
					LineNumber:   1,
				},
				End: &cpb.Point{
					ByteOffset:   9,
					ColumnOffset: 9,
					LineNumber:   1,
				},
			},
			BuildConfig:      "test-build-config",
			Kind:             "/kythe/edge/ref",
			TargetTicket:     "kythe:#decorWithDef",
			TargetDefinition: "kythe:#def1", // expected definition
		}},
		// Nodes: not requested
		DefinitionLocations: map[string]*xpb.Anchor{
			"kythe:#def1": {
				Ticket: "kythe:#def1",
				Parent: "kythe:",
				Span: &cpb.Span{
					Start: &cpb.Point{
						LineNumber: 1,
					},
					End: &cpb.Point{
						ByteOffset:   3,
						ColumnOffset: 3,
						LineNumber:   1,
					},
				},
				Snippet: "def",
				SnippetSpan: &cpb.Span{
					Start: &cpb.Point{
						LineNumber: 1,
					},
					End: &cpb.Point{
						ByteOffset:   3,
						ColumnOffset: 3,
						LineNumber:   1,
					},
				},
			},
		},
	}))

	// TODO(schroederc): test diagnostics (w/ span)
	t.Run("diagnostics", makeDecorTestCase(ctx, xs, &xpb.DecorationsRequest{
		Location:    &xpb.Location{Ticket: fileTicket},
		Diagnostics: true,
	}, &xpb.DecorationsReply{
		Location: &xpb.Location{Ticket: fileTicket},
		Diagnostic: []*cpb.Diagnostic{{
			Message:    "msg",
			Details:    "dtails",
			ContextUrl: "https://kythe.io/schema",
		}},
	}))

	// TODO(schroederc): test split file contents
	// TODO(schroederc): test overrides
}

func TestServingSimpleCrossReferences(t *testing.T) {
	src := &spb.VName{Path: "path", Signature: "signature"}
	ms := &cpb.MarkedSource{
		Kind:    cpb.MarkedSource_IDENTIFIER,
		PreText: "identifier",
	}
	testNodes := []*scpb.Node{{
		Source: &spb.VName{Path: "path"},
		Kind:   &scpb.Node_KytheKind{scpb.NodeKind_FILE},
		Fact: []*scpb.Fact{{
			Name:  &scpb.Fact_KytheName{scpb.FactName_TEXT},
			Value: []byte("blah blah\n"),
		}, {
			Name:  &scpb.Fact_KytheName{scpb.FactName_TEXT_ENCODING},
			Value: []byte("ascii"),
		}},
	}, {
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
			Kind:   &scpb.Edge_KytheKind{scpb.EdgeKind_DEFINES_BINDING},
			Target: &spb.VName{Signature: "caller"},
		}, {
			Kind:   &scpb.Edge_KytheKind{scpb.EdgeKind_DEFINES_BINDING},
			Target: &spb.VName{Signature: "interface"},
		}, {
			Kind:   &scpb.Edge_KytheKind{scpb.EdgeKind_REF},
			Target: src,
		}},
	}, {
		Source: &spb.VName{Path: "path", Signature: "anchor2"},
		Kind:   &scpb.Node_KytheKind{scpb.NodeKind_ANCHOR},
		Fact: []*scpb.Fact{{
			Name:  &scpb.Fact_KytheName{scpb.FactName_LOC_START},
			Value: []byte("0"),
		}, {
			Name:  &scpb.Fact_KytheName{scpb.FactName_LOC_END},
			Value: []byte("4"),
		}},
		Edge: []*scpb.Edge{{
			Kind:   &scpb.Edge_KytheKind{scpb.EdgeKind_CHILD_OF},
			Target: &spb.VName{Signature: "caller"},
		}, {
			Kind:   &scpb.Edge_KytheKind{scpb.EdgeKind_REF_CALL},
			Target: src,
		}},
	}, {
		Source: src,
		Kind:   &scpb.Node_KytheKind{scpb.NodeKind_RECORD},
		Fact: []*scpb.Fact{{
			Name:  &scpb.Fact_KytheName{scpb.FactName_CODE},
			Value: encodeMarkedSource(ms),
		}},
		Edge: []*scpb.Edge{{
			Kind:   &scpb.Edge_KytheKind{scpb.EdgeKind_EXTENDS},
			Target: &spb.VName{Signature: "interface"},
		}},
	}, {
		Source: &spb.VName{Signature: "interface"},
		Kind:   &scpb.Node_KytheKind{scpb.NodeKind_INTERFACE},
	}}

	p, s, rawNodes := ptest.CreateList(testNodes)
	xrefs := FromNodes(s, rawNodes).SplitCrossReferences()

	db := inmemory.NewKeyValueDB()
	w, err := db.Writer(ctx)
	if err != nil {
		t.Fatal(err)
	}

	// Mark table as columnar
	if err := w.Write([]byte(xsrv.ColumnarTableKeyMarker), []byte{}); err != nil {
		t.Fatal(err)
	}
	// Write columnar data to inmemory.KeyValueDB
	beam.ParDo(s, &writeTo{w}, xrefs)

	if err := ptest.Run(p); err != nil {
		t.Fatalf("Pipeline error: %+v", err)
	} else if err := w.Close(); err != nil {
		t.Fatal(err)
	}
	xs := xsrv.NewService(ctx, db)

	ticket := kytheuri.ToString(src)

	t.Run("requested_node", makeXRefTestCase(ctx, xs, &xpb.CrossReferencesRequest{
		Ticket:          []string{ticket},
		Filter:          []string{"**"},
		RelatedNodeKind: []string{"NONE"},
	}, &xpb.CrossReferencesReply{
		CrossReferences: map[string]*xpb.CrossReferencesReply_CrossReferenceSet{
			ticket: {
				Ticket:       ticket,
				MarkedSource: ms,
			},
		},
		Nodes: map[string]*cpb.NodeInfo{
			ticket: {
				Facts: map[string][]byte{
					"/kythe/node/kind": []byte("record"),

					// TODO(schroederc): ellide; MarkedSource already included
					"/kythe/code": encodeMarkedSource(ms),
				},
			},
		},
	}))

	t.Run("non_call_refs", makeXRefTestCase(ctx, xs, &xpb.CrossReferencesRequest{
		Ticket:        []string{ticket},
		ReferenceKind: xpb.CrossReferencesRequest_NON_CALL_REFERENCES,
	}, &xpb.CrossReferencesReply{
		CrossReferences: map[string]*xpb.CrossReferencesReply_CrossReferenceSet{
			ticket: {
				Ticket:       ticket,
				MarkedSource: ms,
				Reference: []*xpb.CrossReferencesReply_RelatedAnchor{{
					Anchor: &xpb.Anchor{
						Parent: "kythe:?path=path",
						Span: &cpb.Span{
							Start: &cpb.Point{
								ByteOffset:   5,
								ColumnOffset: 5,
								LineNumber:   1,
							},
							End: &cpb.Point{
								ByteOffset:   9,
								ColumnOffset: 9,
								LineNumber:   1,
							},
						},
						Snippet: "blah blah",
						SnippetSpan: &cpb.Span{
							Start: &cpb.Point{
								LineNumber: 1,
							},
							End: &cpb.Point{
								ByteOffset:   9,
								ColumnOffset: 9,
								LineNumber:   1,
							},
						},
					},
				}},
			},
		},
	}))

	t.Run("related_nodes", makeXRefTestCase(ctx, xs, &xpb.CrossReferencesRequest{
		Ticket: []string{ticket},
		Filter: []string{"**"},
	}, &xpb.CrossReferencesReply{
		CrossReferences: map[string]*xpb.CrossReferencesReply_CrossReferenceSet{
			ticket: {
				Ticket:       ticket,
				MarkedSource: ms,
				RelatedNode: []*xpb.CrossReferencesReply_RelatedNode{{
					Ticket:       "kythe:#interface",
					RelationKind: edges.Extends,
				}},
			},
		},
		Nodes: map[string]*cpb.NodeInfo{
			ticket: {
				Facts: map[string][]byte{
					facts.NodeKind: []byte(nodes.Record),
					facts.Code:     encodeMarkedSource(ms),
				},
			},
			"kythe:#interface": {
				Facts: map[string][]byte{
					facts.NodeKind: []byte(nodes.Interface),
				},
			},
		},
	}))

	t.Run("node_definitions", makeXRefTestCase(ctx, xs, &xpb.CrossReferencesRequest{
		Ticket:          []string{ticket},
		Filter:          []string{facts.NodeKind},
		NodeDefinitions: true,
	}, &xpb.CrossReferencesReply{
		CrossReferences: map[string]*xpb.CrossReferencesReply_CrossReferenceSet{
			ticket: {
				Ticket:       ticket,
				MarkedSource: ms,
				RelatedNode: []*xpb.CrossReferencesReply_RelatedNode{{
					Ticket:       "kythe:#interface",
					RelationKind: edges.Extends,
				}},
			},
		},
		Nodes: map[string]*cpb.NodeInfo{
			ticket: {Facts: map[string][]byte{facts.NodeKind: []byte(nodes.Record)}},
			"kythe:#interface": {
				Facts:      map[string][]byte{facts.NodeKind: []byte(nodes.Interface)},
				Definition: "kythe:?path=path#anchor1",
			},
		},
		DefinitionLocations: map[string]*xpb.Anchor{
			"kythe:?path=path#anchor1": {
				Ticket: "kythe:?path=path#anchor1",
				Parent: "kythe:?path=path",
				Span: &cpb.Span{
					Start: &cpb.Point{
						ByteOffset:   5,
						ColumnOffset: 5,
						LineNumber:   1,
					},
					End: &cpb.Point{
						ByteOffset:   9,
						ColumnOffset: 9,
						LineNumber:   1,
					},
				},
				Snippet: "blah blah",
				SnippetSpan: &cpb.Span{
					Start: &cpb.Point{
						LineNumber: 1,
					},
					End: &cpb.Point{
						ByteOffset:   9,
						ColumnOffset: 9,
						LineNumber:   1,
					},
				},
			},
		},
	}))

	t.Run("call_refs", makeXRefTestCase(ctx, xs, &xpb.CrossReferencesRequest{
		Ticket:        []string{ticket},
		ReferenceKind: xpb.CrossReferencesRequest_CALL_REFERENCES,
	}, &xpb.CrossReferencesReply{
		CrossReferences: map[string]*xpb.CrossReferencesReply_CrossReferenceSet{
			ticket: {
				Ticket:       ticket,
				MarkedSource: ms,
				Reference: []*xpb.CrossReferencesReply_RelatedAnchor{{
					Anchor: &xpb.Anchor{
						Parent: "kythe:?path=path",
						Span: &cpb.Span{
							Start: &cpb.Point{
								ByteOffset:   0,
								ColumnOffset: 0,
								LineNumber:   1,
							},
							End: &cpb.Point{
								ByteOffset:   4,
								ColumnOffset: 4,
								LineNumber:   1,
							},
						},
						Snippet: "blah blah",
						SnippetSpan: &cpb.Span{
							Start: &cpb.Point{
								LineNumber: 1,
							},
							End: &cpb.Point{
								ByteOffset:   9,
								ColumnOffset: 9,
								LineNumber:   1,
							},
						},
					},
				}},
			},
		},
	}))

	t.Run("direct_callers", makeXRefTestCase(ctx, xs, &xpb.CrossReferencesRequest{
		Ticket:     []string{ticket},
		CallerKind: xpb.CrossReferencesRequest_DIRECT_CALLERS,
	}, &xpb.CrossReferencesReply{
		CrossReferences: map[string]*xpb.CrossReferencesReply_CrossReferenceSet{
			ticket: {
				Ticket:       ticket,
				MarkedSource: ms,
				Caller: []*xpb.CrossReferencesReply_RelatedAnchor{{
					Ticket: "kythe:#caller",
					Anchor: &xpb.Anchor{
						Parent: "kythe:?path=path",
						Span: &cpb.Span{
							Start: &cpb.Point{
								ByteOffset:   5,
								ColumnOffset: 5,
								LineNumber:   1,
							},
							End: &cpb.Point{
								ByteOffset:   9,
								ColumnOffset: 9,
								LineNumber:   1,
							},
						},
						Snippet: "blah blah",
						SnippetSpan: &cpb.Span{
							Start: &cpb.Point{
								LineNumber: 1,
							},
							End: &cpb.Point{
								ByteOffset:   9,
								ColumnOffset: 9,
								LineNumber:   1,
							},
						},
					},
					Site: []*xpb.Anchor{{
						Parent: "kythe:?path=path",
						Span: &cpb.Span{
							Start: &cpb.Point{
								ByteOffset:   0,
								ColumnOffset: 0,
								LineNumber:   1,
							},
							End: &cpb.Point{
								ByteOffset:   4,
								ColumnOffset: 4,
								LineNumber:   1,
							},
						},
						Snippet: "blah blah",
						SnippetSpan: &cpb.Span{
							Start: &cpb.Point{
								LineNumber: 1,
							},
							End: &cpb.Point{
								ByteOffset:   9,
								ColumnOffset: 9,
								LineNumber:   1,
							},
						},
					}},
				}},
			},
		},
	}))

	// TODO(schroederc): add override caller
	t.Run("override_callers", makeXRefTestCase(ctx, xs, &xpb.CrossReferencesRequest{
		Ticket:     []string{ticket},
		CallerKind: xpb.CrossReferencesRequest_OVERRIDE_CALLERS,
	}, &xpb.CrossReferencesReply{
		CrossReferences: map[string]*xpb.CrossReferencesReply_CrossReferenceSet{
			ticket: {
				Ticket:       ticket,
				MarkedSource: ms,
				Caller: []*xpb.CrossReferencesReply_RelatedAnchor{{
					Ticket: "kythe:#caller",
					Anchor: &xpb.Anchor{
						Parent: "kythe:?path=path",
						Span: &cpb.Span{
							Start: &cpb.Point{
								ByteOffset:   5,
								ColumnOffset: 5,
								LineNumber:   1,
							},
							End: &cpb.Point{
								ByteOffset:   9,
								ColumnOffset: 9,
								LineNumber:   1,
							},
						},
						Snippet: "blah blah",
						SnippetSpan: &cpb.Span{
							Start: &cpb.Point{
								LineNumber: 1,
							},
							End: &cpb.Point{
								ByteOffset:   9,
								ColumnOffset: 9,
								LineNumber:   1,
							},
						},
					},
					Site: []*xpb.Anchor{{
						Parent: "kythe:?path=path",
						Span: &cpb.Span{
							Start: &cpb.Point{
								ByteOffset:   0,
								ColumnOffset: 0,
								LineNumber:   1,
							},
							End: &cpb.Point{
								ByteOffset:   4,
								ColumnOffset: 4,
								LineNumber:   1,
							},
						},
						Snippet: "blah blah",
						SnippetSpan: &cpb.Span{
							Start: &cpb.Point{
								LineNumber: 1,
							},
							End: &cpb.Point{
								ByteOffset:   9,
								ColumnOffset: 9,
								LineNumber:   1,
							},
						},
					}},
				}},
			},
		},
	}))
}

func TestServingSimpleEdges(t *testing.T) {
	src := &spb.VName{Path: "path", Signature: "signature"}
	testNodes := []*scpb.Node{{
		Source: &spb.VName{Path: "path"},
		Kind:   &scpb.Node_KytheKind{scpb.NodeKind_FILE},
		Fact: []*scpb.Fact{{
			Name:  &scpb.Fact_KytheName{scpb.FactName_TEXT},
			Value: []byte("blah blah\n"),
		}, {
			Name:  &scpb.Fact_KytheName{scpb.FactName_TEXT_ENCODING},
			Value: []byte("ascii"),
		}},
	}, {
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
			Target: src,
		}},
	}, {
		Source:  src,
		Kind:    &scpb.Node_KytheKind{scpb.NodeKind_RECORD},
		Subkind: &scpb.Node_KytheSubkind{scpb.Subkind_CLASS},
		Edge: []*scpb.Edge{{
			Kind:   &scpb.Edge_KytheKind{scpb.EdgeKind_EXTENDS},
			Target: &spb.VName{Signature: "interface1"},
		}, {
			Kind:   &scpb.Edge_KytheKind{scpb.EdgeKind_EXTENDS},
			Target: &spb.VName{Signature: "interface2"},
		}},
	}, {
		Source:  &spb.VName{Signature: "child"},
		Kind:    &scpb.Node_KytheKind{scpb.NodeKind_RECORD},
		Subkind: &scpb.Node_KytheSubkind{scpb.Subkind_CLASS},
		Edge: []*scpb.Edge{{
			Kind:   &scpb.Edge_KytheKind{scpb.EdgeKind_CHILD_OF},
			Target: src,
		}},
	}, {
		Source: &spb.VName{Signature: "interface"},
		Kind:   &scpb.Node_KytheKind{scpb.NodeKind_INTERFACE},
	}}

	p, s, rawNodes := ptest.CreateList(testNodes)
	edgeEntries := FromNodes(s, rawNodes).SplitEdges()

	db := inmemory.NewKeyValueDB()
	w, err := db.Writer(ctx)
	if err != nil {
		t.Fatal(err)
	}

	// Mark table as columnar
	if err := w.Write([]byte(gsrv.ColumnarTableKeyMarker), []byte{}); err != nil {
		t.Fatal(err)
	}
	// Write columnar data to inmemory.KeyValueDB
	beam.ParDo(s, &writeTo{w}, edgeEntries)

	if err := ptest.Run(p); err != nil {
		t.Fatalf("Pipeline error: %+v", err)
	} else if err := w.Close(); err != nil {
		t.Fatal(err)
	}

	gs := gsrv.NewService(ctx, db)
	ticket := kytheuri.ToString(src)

	t.Run("source_node", makeEdgesTestCase(ctx, gs, &gpb.EdgesRequest{
		Ticket: []string{ticket},
		Kind:   []string{"non_existent_kind"},
		Filter: []string{"**"},
	}, &gpb.EdgesReply{
		Nodes: map[string]*cpb.NodeInfo{
			ticket: &cpb.NodeInfo{
				Facts: map[string][]byte{
					facts.NodeKind: []byte(nodes.Record),
					facts.Subkind:  []byte(nodes.Class),
				},
			},
		},
	}))

	t.Run("edges", makeEdgesTestCase(ctx, gs, &gpb.EdgesRequest{
		Ticket: []string{ticket},
	}, &gpb.EdgesReply{
		EdgeSets: map[string]*gpb.EdgeSet{
			ticket: {
				Groups: map[string]*gpb.EdgeSet_Group{
					edges.Extends: {
						Edge: []*gpb.EdgeSet_Group_Edge{{
							TargetTicket: "kythe:#interface1",
						}, {
							TargetTicket: "kythe:#interface2",
						}},
					},
					"%" + edges.ChildOf: {
						Edge: []*gpb.EdgeSet_Group_Edge{{
							TargetTicket: "kythe:#child",
						}},
					},
				},
			},
		},
	}))

	t.Run("edge_targets", makeEdgesTestCase(ctx, gs, &gpb.EdgesRequest{
		Ticket: []string{ticket},
		Filter: []string{facts.NodeKind},
	}, &gpb.EdgesReply{
		EdgeSets: map[string]*gpb.EdgeSet{
			ticket: {
				Groups: map[string]*gpb.EdgeSet_Group{
					edges.Extends: {
						Edge: []*gpb.EdgeSet_Group_Edge{{
							TargetTicket: "kythe:#interface1",
						}, {
							TargetTicket: "kythe:#interface2",
						}},
					},
					"%" + edges.ChildOf: {
						Edge: []*gpb.EdgeSet_Group_Edge{{
							TargetTicket: "kythe:#child",
						}},
					},
				},
			},
		},
		Nodes: map[string]*cpb.NodeInfo{
			ticket:         &cpb.NodeInfo{Facts: map[string][]byte{facts.NodeKind: []byte(nodes.Record)}},
			"kythe:#child": &cpb.NodeInfo{Facts: map[string][]byte{facts.NodeKind: []byte(nodes.Record)}},
		},
	}))
}

type writeTo struct{ w keyvalue.Writer }

func (p *writeTo) ProcessElement(ctx context.Context, k, v []byte, emit func([]byte)) error {
	return p.w.Write(k, v)
}

func makeDecorTestCase(ctx context.Context, xs xrefs.Service, req *xpb.DecorationsRequest, expected *xpb.DecorationsReply) func(*testing.T) {
	return func(t *testing.T) {
		reply, err := xs.Decorations(ctx, req)
		if err != nil {
			t.Fatalf("Decorations error: %v", err)
		}
		if diff := compare.ProtoDiff(expected, reply); diff != "" {
			t.Fatalf("DecorationsReply differences: (- expected; + found)\n%s", diff)
		}
	}
}

func makeXRefTestCase(ctx context.Context, xs xrefs.Service, req *xpb.CrossReferencesRequest, expected *xpb.CrossReferencesReply) func(*testing.T) {
	return func(t *testing.T) {
		reply, err := xs.CrossReferences(ctx, req)
		if err != nil {
			t.Fatalf("CrossReferences error: %v", err)
		}
		if diff := compare.ProtoDiff(expected, reply); diff != "" {
			t.Fatalf("CrossReferencesReply differences: (- expected; + found)\n%s", diff)
		}
	}
}

func makeEdgesTestCase(ctx context.Context, gs graph.Service, req *gpb.EdgesRequest, expected *gpb.EdgesReply) func(*testing.T) {
	return func(t *testing.T) {
		reply, err := gs.Edges(ctx, req)
		if err != nil {
			t.Fatalf("Edges error: %v", err)
		}
		if diff := compare.ProtoDiff(expected, reply); diff != "" {
			t.Fatalf("EdgesReply differences: (- expected; + found)\n%s", diff)
		}
	}
}
