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
	"strings"
	"testing"

	"kythe.io/kythe/go/util/keys"

	"github.com/google/go-cmp/cmp"

	cpb "kythe.io/kythe/proto/common_go_proto"
	scpb "kythe.io/kythe/proto/schema_go_proto"
	srvpb "kythe.io/kythe/proto/serving_go_proto"
	spb "kythe.io/kythe/proto/storage_go_proto"
	xspb "kythe.io/kythe/proto/xref_serving_go_proto"
)

func TestEncodingRoundtrip(t *testing.T) {
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
			Kind:       xspb.FileDecorations_TargetOverride_EXTENDS,
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
		kv, err := EncodeDecorationsEntry(nil, test)
		if err != nil {
			t.Errorf("Error encoding %T: %v", test.Entry, err)
			continue
		}
		var file spb.VName
		key, err := keys.Parse(string(kv.Key), &file)
		if err != nil {
			t.Errorf("Error decoding file for %T: %v", test.Entry, err)
			continue
		}
		found, err := DecodeDecorationsEntry(&file, string(key), kv.Value)
		if err != nil {
			t.Errorf("Error decoding %T: %v", test.Entry, err)
		} else if diff := cmp.Diff(test, found, ignoreProtoXXXFields); diff != "" {
			t.Errorf("%T roundtrip differences: (- expected; + found)\n%s", test.Entry, diff)
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
