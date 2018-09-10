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
	"strings"
	"testing"

	"kythe.io/kythe/go/services/xrefs"
	"kythe.io/kythe/go/serving/xrefs/columnar"
	"kythe.io/kythe/go/storage/inmemory"
	"kythe.io/kythe/go/storage/keyvalue"
	"kythe.io/kythe/go/util/kytheuri"

	"github.com/google/go-cmp/cmp"

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
		if diff := cmp.Diff(expectedLoc, reply.Location, ignoreProtoXXXFields); diff != "" {
			t.Fatalf("Location differences: (- expected; + found)\n%s", diff)
		}
	})

	t.Run("source_text", makeTestCase(ctx, xs, &xpb.DecorationsRequest{
		Location:   &xpb.Location{Ticket: fileTicket},
		SourceText: true,
	}, &xpb.DecorationsReply{
		Location:   &xpb.Location{Ticket: fileTicket},
		SourceText: []byte(expectedText),
		Encoding:   "ascii",
	}))

	t.Run("references", makeTestCase(ctx, xs, &xpb.DecorationsRequest{
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

	t.Run("referenced_nodes", makeTestCase(ctx, xs, &xpb.DecorationsRequest{
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

	t.Run("target_definitions", makeTestCase(ctx, xs, &xpb.DecorationsRequest{
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

func makeTestCase(ctx context.Context, xs xrefs.Service, req *xpb.DecorationsRequest, expected *xpb.DecorationsReply) func(*testing.T) {
	return func(t *testing.T) {
		reply, err := xs.Decorations(ctx, req)
		if err != nil {
			t.Fatalf("Decorations error: %v", err)
		}
		if diff := cmp.Diff(expected, reply, ignoreProtoXXXFields); diff != "" {
			t.Fatalf("DecorationsReply differences: (- expected; + found)\n%s", diff)
		}
	}
}

var ignoreProtoXXXFields = cmp.FilterPath(func(p cmp.Path) bool {
	for _, s := range p {
		if strings.HasPrefix(s.String(), ".XXX_") {
			return true
		}
	}
	return false
}, cmp.Ignore())
