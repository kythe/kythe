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

package columnar

import (
	"fmt"
	"testing"

	"kythe.io/kythe/go/util/compare"
	"kythe.io/kythe/go/util/keys"

	cpb "kythe.io/kythe/proto/common_go_proto"
	scpb "kythe.io/kythe/proto/schema_go_proto"
	srvpb "kythe.io/kythe/proto/serving_go_proto"
	spb "kythe.io/kythe/proto/storage_go_proto"
	xspb "kythe.io/kythe/proto/xref_serving_go_proto"
)

func TestDecorationsEncodingRoundtrip(t *testing.T) {
	file := &spb.VName{Corpus: "corpus", Root: "root", Path: "path"}
	tests := []*xspb.FileDecorations{{
		File: file,
		Entry: &xspb.FileDecorations_Index_{&xspb.FileDecorations_Index{
			TextEncoding: "some_encoding",
		}},
	}, {
		File: file,
		Entry: &xspb.FileDecorations_Text_{&xspb.FileDecorations_Text{
			Text: []byte("some file text\n"),
		}},
	}, {
		File: file,
		Entry: &xspb.FileDecorations_Target_{&xspb.FileDecorations_Target{
			StartOffset: 64,
			EndOffset:   128,
			Kind:        &xspb.FileDecorations_Target_GenericKind{"generic"},
			Target:      &spb.VName{Corpus: "c", Root: "r", Signature: "target"},
		}},
	}, {
		File: file,
		Entry: &xspb.FileDecorations_Target_{&xspb.FileDecorations_Target{
			StartOffset: 64,
			EndOffset:   128,
			Kind:        &xspb.FileDecorations_Target_KytheKind{scpb.EdgeKind_CHILD_OF},
			Target:      &spb.VName{Signature: "target"},
		}},
	}, {
		File: file,
		Entry: &xspb.FileDecorations_TargetOverride_{&xspb.FileDecorations_TargetOverride{
			Kind:       srvpb.FileDecorations_Override_EXTENDS,
			Overridden: &spb.VName{Signature: "overridden"},
			Overriding: &spb.VName{Signature: "overriding"},
		}},
	}, {
		File: file,
		Entry: &xspb.FileDecorations_TargetNode_{&xspb.FileDecorations_TargetNode{
			Node: &scpb.Node{
				Source: &spb.VName{Signature: "nodeSignature"},
			},
		}},
	}, {
		File: file,
		Entry: &xspb.FileDecorations_TargetDefinition_{&xspb.FileDecorations_TargetDefinition{
			Target:     &spb.VName{Signature: "target"},
			Definition: &spb.VName{Signature: "def"},
		}},
	}, {
		File: file,
		Entry: &xspb.FileDecorations_DefinitionLocation_{&xspb.FileDecorations_DefinitionLocation{
			Location: &srvpb.ExpandedAnchor{
				Ticket: "kythe://corpus#someSignature",
				Span: &cpb.Span{
					Start: &cpb.Point{ByteOffset: 32},
					End:   &cpb.Point{ByteOffset: 256},
				},
				Text: "some anchor text",
			},
		}},
	}, {
		File: file,
		Entry: &xspb.FileDecorations_Override_{&xspb.FileDecorations_Override{
			Override: &spb.VName{Signature: "target"},
			MarkedSource: &cpb.MarkedSource{
				Kind:    cpb.MarkedSource_IDENTIFIER,
				PreText: "ident",
			},
		}},
	}, {
		File: file,
		Entry: &xspb.FileDecorations_Diagnostic_{&xspb.FileDecorations_Diagnostic{
			Diagnostic: &cpb.Diagnostic{
				Message: "diagnostic message",
			},
		}},
	}, {
		File: file,
		Entry: &xspb.FileDecorations_Diagnostic_{&xspb.FileDecorations_Diagnostic{
			Diagnostic: &cpb.Diagnostic{
				Span: &cpb.Span{
					Start: &cpb.Point{ByteOffset: 32},
					End:   &cpb.Point{ByteOffset: 256},
				},
				Message: "span diagnostic message",
			},
		}},
	}}

	for _, test := range tests {
		t.Run(fmt.Sprintf("%T", test.Entry), func(t *testing.T) {
			kv, err := EncodeDecorationsEntry(nil, test)
			if err != nil {
				t.Errorf("Error encoding %T: %v", test.Entry, err)
				return
			}
			var file spb.VName
			key, err := keys.Parse(string(kv.Key), &file)
			if err != nil {
				t.Errorf("Error decoding file for %T: %v", test.Entry, err)
				return
			}
			found, err := DecodeDecorationsEntry(&file, string(key), kv.Value)
			if err != nil {
				t.Errorf("Error decoding %T: %v", test.Entry, err)
			} else if diff := compare.ProtoDiff(test, found); diff != "" {
				t.Errorf("%T roundtrip differences: (- expected; + found)\n%s", test.Entry, diff)
			}
		})
	}
}

func TestCrossReferencesEncodingRoundtrip(t *testing.T) {
	src := &spb.VName{Corpus: "corpus", Root: "root", Path: "path", Signature: "sig"}
	tests := []*xspb.CrossReferences{{
		Source: src,
		Entry: &xspb.CrossReferences_Index_{&xspb.CrossReferences_Index{
			Node:         &scpb.Node{},
			MarkedSource: &cpb.MarkedSource{Kind: cpb.MarkedSource_IDENTIFIER},
			MergeWith:    []*spb.VName{{Signature: "anything"}},
		}},
	}, {
		Source: src,
		Entry: &xspb.CrossReferences_Reference_{&xspb.CrossReferences_Reference{
			Kind: &xspb.CrossReferences_Reference_KytheKind{scpb.EdgeKind_REF},
			Location: &srvpb.ExpandedAnchor{
				Ticket: "kythe:#reference",
				Text:   "ref",
				Span: &cpb.Span{
					Start: &cpb.Point{ByteOffset: 16},
					End:   &cpb.Point{ByteOffset: 128},
				},
			},
		}},
	}, {
		Source: src,
		Entry: &xspb.CrossReferences_Relation_{&xspb.CrossReferences_Relation{
			Node:    &spb.VName{Signature: "relatedNode"},
			Kind:    &xspb.CrossReferences_Relation_KytheKind{scpb.EdgeKind_EXTENDS},
			Ordinal: 4,
		}},
	}, {
		Source: src,
		Entry: &xspb.CrossReferences_Relation_{&xspb.CrossReferences_Relation{
			Node:    &spb.VName{Signature: "relatedNode"},
			Kind:    &xspb.CrossReferences_Relation_KytheKind{scpb.EdgeKind_EXTENDS},
			Ordinal: 2,
			Reverse: true,
		}},
	}, {
		Source: src,
		Entry: &xspb.CrossReferences_Caller_{&xspb.CrossReferences_Caller{
			Caller: &spb.VName{Signature: "caller"},
		}},
	}, {
		Source: src,
		Entry: &xspb.CrossReferences_Callsite_{&xspb.CrossReferences_Callsite{
			Caller: &spb.VName{Signature: "caller"},
			Location: &srvpb.ExpandedAnchor{
				Ticket: "kythe:#callsite",
				Text:   "callsite",
				Span: &cpb.Span{
					Start: &cpb.Point{ByteOffset: 32},
					End:   &cpb.Point{ByteOffset: 256},
				},
			},
		}},
	}, {
		Source: src,
		Entry: &xspb.CrossReferences_Callsite_{&xspb.CrossReferences_Callsite{
			Caller: &spb.VName{Signature: "caller"},
			Kind:   xspb.CrossReferences_Callsite_OVERRIDE,
			Location: &srvpb.ExpandedAnchor{
				Ticket: "kythe:#callsite2",
				Span: &cpb.Span{
					Start: &cpb.Point{ByteOffset: 32},
					End:   &cpb.Point{ByteOffset: 256},
				},
			},
		}},
	}, {
		Source: src,
		Entry: &xspb.CrossReferences_RelatedNode_{&xspb.CrossReferences_RelatedNode{
			Node: &scpb.Node{
				Source: &spb.VName{Signature: "relatedNode"},
			},
		}},
	}, {
		Source: src,
		Entry: &xspb.CrossReferences_NodeDefinition_{&xspb.CrossReferences_NodeDefinition{
			Node: &spb.VName{Signature: "relatedNode"},
			Location: &srvpb.ExpandedAnchor{
				Ticket: "kythe:#relatedNodeDef",
				Span: &cpb.Span{
					Start: &cpb.Point{ByteOffset: 32},
					End:   &cpb.Point{ByteOffset: 256},
				},
			},
		}},
	}}

	for _, test := range tests {
		t.Run(fmt.Sprintf("%T", test.Entry), func(t *testing.T) {
			kv, err := EncodeCrossReferencesEntry(nil, test)
			if err != nil {
				t.Errorf("Error encoding %T: %v", test.Entry, err)
				return
			}
			var src spb.VName
			key, err := keys.Parse(string(kv.Key), &src)
			if err != nil {
				t.Errorf("Error decoding file for %T: %v", test.Entry, err)
				return
			}
			found, err := DecodeCrossReferencesEntry(&src, string(key), kv.Value)
			if err != nil {
				t.Errorf("Error decoding %T: %v", test.Entry, err)
			} else if diff := compare.ProtoDiff(test, found); diff != "" {
				t.Errorf("%T roundtrip differences: (- expected; + found)\n%s", test.Entry, diff)
			}
		})
	}
}
