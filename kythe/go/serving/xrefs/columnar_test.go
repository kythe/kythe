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

package xrefs

import (
	"context"
	"testing"

	"kythe.io/kythe/go/services/xrefs"
	"kythe.io/kythe/go/serving/xrefs/columnar"
	"kythe.io/kythe/go/storage/inmemory"
	"kythe.io/kythe/go/storage/keyvalue"
	"kythe.io/kythe/go/util/compare"
	"kythe.io/kythe/go/util/kytheuri"
	"kythe.io/kythe/go/util/schema/facts"

	cpb "kythe.io/kythe/proto/common_go_proto"
	scpb "kythe.io/kythe/proto/schema_go_proto"
	srvpb "kythe.io/kythe/proto/serving_go_proto"
	spb "kythe.io/kythe/proto/storage_go_proto"
	xpb "kythe.io/kythe/proto/xref_go_proto"
	xspb "kythe.io/kythe/proto/xref_serving_go_proto"
)

func mustWriteDecor(t *testing.T, w keyvalue.Writer, fd *xspb.FileDecorations) {
	kv, err := columnar.EncodeDecorationsEntry(columnar.DecorationsKeyPrefix, fd)
	if err != nil {
		t.Fatal(err)
	}
	mustWrite(t, w, kv.Key, kv.Value)
}

func mustWriteXRef(t *testing.T, w keyvalue.Writer, fd *xspb.CrossReferences) {
	kv, err := columnar.EncodeCrossReferencesEntry(columnar.CrossReferencesKeyPrefix, fd)
	if err != nil {
		t.Fatal(err)
	}
	mustWrite(t, w, kv.Key, kv.Value)
}

func mustWrite(t *testing.T, w keyvalue.Writer, key, val []byte) {
	if err := w.Write(key, val); err != nil {
		t.Fatal(err)
	}
}

func TestServingDecorations(t *testing.T) {
	ctx := context.Background()
	db := inmemory.NewKeyValueDB()
	w, err := db.Writer(ctx)
	if err != nil {
		t.Fatal(err)
	}

	// Mark table as columnar
	mustWrite(t, w, []byte(ColumnarTableKeyMarker), []byte{})

	file := &spb.VName{Path: "path"}
	const expectedText = "some text\n"
	decor := []*xspb.FileDecorations{{
		File: file,
		Entry: &xspb.FileDecorations_Index_{&xspb.FileDecorations_Index{
			TextEncoding: "ascii",
		}},
	}, {
		File: file,
		Entry: &xspb.FileDecorations_Text_{&xspb.FileDecorations_Text{
			Text: []byte(expectedText),
		}},
	}, {
		File: file,
		Entry: &xspb.FileDecorations_Target_{&xspb.FileDecorations_Target{
			StartOffset: 0,
			EndOffset:   4,
			Kind: &xspb.FileDecorations_Target_KytheKind{
				scpb.EdgeKind_REF,
			},
			Target: &spb.VName{Signature: "simpleDecor"},
		}},
	}, {
		File: file,
		Entry: &xspb.FileDecorations_TargetNode_{&xspb.FileDecorations_TargetNode{
			Node: &scpb.Node{
				Source: &spb.VName{Signature: "simpleDecor"},
				Kind:   &scpb.Node_KytheKind{scpb.NodeKind_RECORD},
			},
		}},
	}, {
		File: file,
		Entry: &xspb.FileDecorations_Target_{&xspb.FileDecorations_Target{
			StartOffset: 5,
			EndOffset:   9,
			Kind: &xspb.FileDecorations_Target_KytheKind{
				scpb.EdgeKind_REF,
			},
			Target: &spb.VName{Signature: "decorWithDef"},
		}},
	}, {
		File: file,
		Entry: &xspb.FileDecorations_TargetDefinition_{&xspb.FileDecorations_TargetDefinition{
			Target:     &spb.VName{Signature: "decorWithDef"},
			Definition: &spb.VName{Signature: "def1"},
		}},
	}, {
		File: file,
		Entry: &xspb.FileDecorations_DefinitionLocation_{&xspb.FileDecorations_DefinitionLocation{
			Location: &srvpb.ExpandedAnchor{
				Ticket: "kythe:#def1",
			},
		}},
	}}
	for _, fd := range decor {
		mustWriteDecor(t, w, fd)
	}

	if err := w.Close(); err != nil {
		t.Fatal(err)
	}

	xs := NewService(ctx, db)

	fileTicket := kytheuri.ToString(file)
	t.Run("file_location", func(t *testing.T) {
		reply, err := xs.Decorations(ctx, &xpb.DecorationsRequest{
			Location:   &xpb.Location{Ticket: fileTicket},
			SourceText: true,
		})
		if err != nil {
			t.Fatalf("Decorations error: %v", err)
		}

		expectedLoc := &xpb.Location{Ticket: fileTicket}
		if diff := compare.ProtoDiff(expectedLoc, reply.Location); diff != "" {
			t.Fatalf("Location differences: (- expected; + found)\n%s", diff)
		}
	})

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
			Kind:         "/kythe/edge/ref",
			TargetTicket: "kythe:#decorWithDef",
			// TargetDefinition: explicitly not requested
		}},
		// Nodes: not requested
		// DefinitionLocations: not requested
	}))

	t.Run("references_dirty", makeDecorTestCase(ctx, xs, &xpb.DecorationsRequest{
		Location:    &xpb.Location{Ticket: fileTicket},
		References:  true,
		DirtyBuffer: []byte("\n " + expectedText),
	}, &xpb.DecorationsReply{
		Location: &xpb.Location{Ticket: fileTicket},
		Reference: []*xpb.DecorationsReply_Reference{{
			Span: &cpb.Span{
				Start: &cpb.Point{
					ByteOffset:   0 + 2,
					ColumnOffset: 0 + 1,
					LineNumber:   1 + 1,
				},
				End: &cpb.Point{
					ByteOffset:   4 + 2,
					ColumnOffset: 4 + 1,
					LineNumber:   1 + 1,
				},
			},
			Kind:         "/kythe/edge/ref",
			TargetTicket: "kythe:#simpleDecor",
		}, {
			Span: &cpb.Span{
				Start: &cpb.Point{
					ByteOffset:   5 + 2,
					ColumnOffset: 5 + 1,
					LineNumber:   1 + 1,
				},
				End: &cpb.Point{
					ByteOffset:   9 + 2,
					ColumnOffset: 9 + 1,
					LineNumber:   1 + 1,
				},
			},
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
			Kind:             "/kythe/edge/ref",
			TargetTicket:     "kythe:#decorWithDef",
			TargetDefinition: "kythe:#def1", // expected definition
		}},
		// Nodes: not requested
		DefinitionLocations: map[string]*xpb.Anchor{
			"kythe:#def1": {
				Ticket: "kythe:#def1",
				Parent: "kythe:",
			},
		},
	}))

	// TODO(schroederc): test split file contents
	// TODO(schroederc): test overrides
	// TODO(schroederc): test diagnostics (w/ or w/o span)
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

func TestServingCrossReferences(t *testing.T) {
	ctx := context.Background()
	db := inmemory.NewKeyValueDB()
	w, err := db.Writer(ctx)
	if err != nil {
		t.Fatal(err)
	}

	// Mark table as columnar
	mustWrite(t, w, []byte(ColumnarTableKeyMarker), []byte{})

	src := &spb.VName{Path: "path", Signature: "signature"}
	span := &cpb.Span{
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
	}
	ms := &cpb.MarkedSource{
		Kind:    cpb.MarkedSource_IDENTIFIER,
		PreText: "identifier",
	}
	xrefs := []*xspb.CrossReferences{{
		Source: src,
		Entry: &xspb.CrossReferences_Index_{&xspb.CrossReferences_Index{
			Node: &scpb.Node{
				Kind: &scpb.Node_KytheKind{scpb.NodeKind_RECORD},
			},
			MarkedSource: ms,
		}},
	}, {
		Source: src,
		Entry: &xspb.CrossReferences_Reference_{&xspb.CrossReferences_Reference{
			Kind: &xspb.CrossReferences_Reference_KytheKind{scpb.EdgeKind_REF},
			Location: &srvpb.ExpandedAnchor{
				Ticket: "kythe:?path=path1#ref1",
				Span:   span,
			},
		}},
	}, {
		Source: src,
		Entry: &xspb.CrossReferences_Reference_{&xspb.CrossReferences_Reference{
			Kind: &xspb.CrossReferences_Reference_KytheKind{scpb.EdgeKind_REF_CALL},
			Location: &srvpb.ExpandedAnchor{
				Ticket: "kythe:?path=path2#ref2",
				Span:   span,
			},
		}},
	}, {
		Source: src,
		Entry: &xspb.CrossReferences_Reference_{&xspb.CrossReferences_Reference{
			Kind: &xspb.CrossReferences_Reference_KytheKind{scpb.EdgeKind_DEFINES},
			Location: &srvpb.ExpandedAnchor{
				Ticket: "kythe:?path=path1#def1",
				Span:   span,
			},
		}},
	}, {
		Source: src,
		Entry: &xspb.CrossReferences_Reference_{&xspb.CrossReferences_Reference{
			Kind: &xspb.CrossReferences_Reference_KytheKind{scpb.EdgeKind_DEFINES_BINDING},
			Location: &srvpb.ExpandedAnchor{
				Ticket: "kythe:?path=path2#def2",
				Span:   span,
			},
		}},
	}, {
		Source: src,
		Entry: &xspb.CrossReferences_Reference_{&xspb.CrossReferences_Reference{
			Kind: &xspb.CrossReferences_Reference_GenericKind{"#internal/ref/declare"},
			Location: &srvpb.ExpandedAnchor{
				Ticket: "kythe:?path=path1#decl1",
				Span:   span,
			},
		}},
	}, {
		Source: src,
		Entry: &xspb.CrossReferences_Reference_{&xspb.CrossReferences_Reference{
			Kind: &xspb.CrossReferences_Reference_GenericKind{"#internal/ref/declare"},
			Location: &srvpb.ExpandedAnchor{
				Ticket: "kythe:?path=path2#decl2",
				Span:   span,
			},
		}},
	}, {
		Source: src,
		Entry: &xspb.CrossReferences_Caller_{&xspb.CrossReferences_Caller{
			Caller: &spb.VName{Signature: "caller"},
			Location: &srvpb.ExpandedAnchor{
				Ticket: "kythe:?path=path#caller",
				Span:   span,
			},
			MarkedSource: &cpb.MarkedSource{Kind: cpb.MarkedSource_IDENTIFIER, PreText: "caller"},
		}},
	}, {
		Source: src,
		Entry: &xspb.CrossReferences_Callsite_{&xspb.CrossReferences_Callsite{
			Caller: &spb.VName{Signature: "caller"},
			Kind:   xspb.CrossReferences_Callsite_DIRECT,
			Location: &srvpb.ExpandedAnchor{
				Ticket: "kythe:?path=path2#callsite",
				Span:   span,
			},
		}},
	}, {
		Source: src,
		Entry: &xspb.CrossReferences_Callsite_{&xspb.CrossReferences_Callsite{
			Caller: &spb.VName{Signature: "caller"},
			Kind:   xspb.CrossReferences_Callsite_OVERRIDE,
			Location: &srvpb.ExpandedAnchor{
				Ticket: "kythe:?path=path3#callsite_override",
				Span:   span,
			},
		}},
	}, {
		Source: src,
		Entry: &xspb.CrossReferences_Relation_{&xspb.CrossReferences_Relation{
			Node:    &spb.VName{Signature: "relatedNode"},
			Kind:    &xspb.CrossReferences_Relation_KytheKind{scpb.EdgeKind_CHILD_OF},
			Reverse: true,
		}},
	}, {
		Source: src,
		Entry: &xspb.CrossReferences_RelatedNode_{&xspb.CrossReferences_RelatedNode{
			Node: &scpb.Node{
				Source: &spb.VName{Signature: "relatedNode"},
				Kind:   &scpb.Node_KytheKind{scpb.NodeKind_FUNCTION},
			},
		}},
	}, {
		Source: src,
		Entry: &xspb.CrossReferences_NodeDefinition_{&xspb.CrossReferences_NodeDefinition{
			Node: &spb.VName{Signature: "relatedNode"},
			Location: &srvpb.ExpandedAnchor{
				Ticket: "kythe:#relatedNodeDef",
				Span:   span,
			},
		}},
	}}
	for _, xr := range xrefs {
		mustWriteXRef(t, w, xr)
	}
	if err := w.Close(); err != nil {
		t.Fatal(err)
	}
	xs := NewService(ctx, db)

	refs := []*xpb.CrossReferencesReply_RelatedAnchor{{
		Anchor: &xpb.Anchor{
			Parent: "kythe:?path=path1",
			Span:   span,
		},
	}, {
		Anchor: &xpb.Anchor{
			Parent: "kythe:?path=path2",
			Span:   span,
		},
	}}

	defs := []*xpb.CrossReferencesReply_RelatedAnchor{{
		Anchor: &xpb.Anchor{
			Parent: "kythe:?path=path1",
			Span:   span,
		},
	}, {
		Anchor: &xpb.Anchor{
			Parent: "kythe:?path=path2",
			Span:   span,
		},
	}}

	decls := []*xpb.CrossReferencesReply_RelatedAnchor{{
		Anchor: &xpb.Anchor{
			Parent: "kythe:?path=path1",
			Span:   span,
		},
	}, {
		Anchor: &xpb.Anchor{
			Parent: "kythe:?path=path2",
			Span:   span,
		},
	}}

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
				},
			},
		},
	}))

	t.Run("refs", makeXRefTestCase(ctx, xs, &xpb.CrossReferencesRequest{
		Ticket:        []string{ticket},
		ReferenceKind: xpb.CrossReferencesRequest_ALL_REFERENCES,
	}, &xpb.CrossReferencesReply{
		CrossReferences: map[string]*xpb.CrossReferencesReply_CrossReferenceSet{
			ticket: {
				Ticket:       ticket,
				MarkedSource: ms,
				Reference:    refs,
			},
		},
	}))

	t.Run("decls", makeXRefTestCase(ctx, xs, &xpb.CrossReferencesRequest{
		Ticket:          []string{ticket},
		DeclarationKind: xpb.CrossReferencesRequest_ALL_DECLARATIONS,
	}, &xpb.CrossReferencesReply{
		CrossReferences: map[string]*xpb.CrossReferencesReply_CrossReferenceSet{
			ticket: {
				Ticket:       ticket,
				MarkedSource: ms,
				Declaration:  decls,
			},
		},
	}))

	t.Run("defs", makeXRefTestCase(ctx, xs, &xpb.CrossReferencesRequest{
		Ticket:         []string{ticket},
		DefinitionKind: xpb.CrossReferencesRequest_ALL_DEFINITIONS,
	}, &xpb.CrossReferencesReply{
		CrossReferences: map[string]*xpb.CrossReferencesReply_CrossReferenceSet{
			ticket: {
				Ticket:       ticket,
				MarkedSource: ms,
				Definition:   defs,
			},
		},
	}))

	t.Run("full_defs", makeXRefTestCase(ctx, xs, &xpb.CrossReferencesRequest{
		Ticket:         []string{ticket},
		DefinitionKind: xpb.CrossReferencesRequest_FULL_DEFINITIONS,
	}, &xpb.CrossReferencesReply{
		CrossReferences: map[string]*xpb.CrossReferencesReply_CrossReferenceSet{
			ticket: {
				Ticket:       ticket,
				MarkedSource: ms,
				Definition:   defs[0:1],
			},
		},
	}))

	t.Run("bindings", makeXRefTestCase(ctx, xs, &xpb.CrossReferencesRequest{
		Ticket:         []string{ticket},
		DefinitionKind: xpb.CrossReferencesRequest_BINDING_DEFINITIONS,
	}, &xpb.CrossReferencesReply{
		CrossReferences: map[string]*xpb.CrossReferencesReply_CrossReferenceSet{
			ticket: {
				Ticket:       ticket,
				MarkedSource: ms,
				Definition:   defs[1:2],
			},
		},
	}))

	t.Run("related_nodes", makeXRefTestCase(ctx, xs, &xpb.CrossReferencesRequest{
		Ticket:        []string{ticket},
		ReferenceKind: xpb.CrossReferencesRequest_ALL_REFERENCES,
		Filter:        []string{"**"},
	}, &xpb.CrossReferencesReply{
		CrossReferences: map[string]*xpb.CrossReferencesReply_CrossReferenceSet{
			ticket: {
				Ticket:       ticket,
				MarkedSource: ms,
				Reference:    refs,
				RelatedNode: []*xpb.CrossReferencesReply_RelatedNode{{
					Ticket:       "kythe:#relatedNode",
					RelationKind: "%/kythe/edge/childof",
				}},
			},
		},
		Nodes: map[string]*cpb.NodeInfo{
			ticket: {
				Facts: map[string][]byte{
					"/kythe/node/kind": []byte("record"),
				},
			},
			"kythe:#relatedNode": {
				Facts: map[string][]byte{
					"/kythe/node/kind": []byte("function"),
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
					Ticket:       "kythe:#relatedNode",
					RelationKind: "%/kythe/edge/childof",
				}},
			},
		},
		Nodes: map[string]*cpb.NodeInfo{
			ticket: {Facts: map[string][]byte{"/kythe/node/kind": []byte("record")}},
			"kythe:#relatedNode": {
				Facts:      map[string][]byte{"/kythe/node/kind": []byte("function")},
				Definition: "kythe:#relatedNodeDef",
			},
		},
		DefinitionLocations: map[string]*xpb.Anchor{
			"kythe:#relatedNodeDef": {
				Ticket: "kythe:#relatedNodeDef",
				Parent: "kythe:",
				Span:   span,
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
				Reference:    refs[0:1],
			},
		},
	}))

	t.Run("callsites", makeXRefTestCase(ctx, xs, &xpb.CrossReferencesRequest{
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
						Span:   span,
					},
					MarkedSource: &cpb.MarkedSource{Kind: cpb.MarkedSource_IDENTIFIER, PreText: "caller"},
					Site: []*xpb.Anchor{{
						Parent: "kythe:?path=path2",
						Span:   span,
					}, {
						Parent: "kythe:?path=path3",
						Span:   span,
					}},
				}},
			},
		},
	}))

	t.Run("direct_callsites", makeXRefTestCase(ctx, xs, &xpb.CrossReferencesRequest{
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
						Span:   span,
					},
					MarkedSource: &cpb.MarkedSource{Kind: cpb.MarkedSource_IDENTIFIER, PreText: "caller"},
					Site: []*xpb.Anchor{{
						Parent: "kythe:?path=path2",
						Span:   span,
					}},
				}},
			},
		},
	}))
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
