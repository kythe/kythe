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

	gspb "kythe.io/kythe/proto/graph_serving_go_proto"
	scpb "kythe.io/kythe/proto/schema_go_proto"
	spb "kythe.io/kythe/proto/storage_go_proto"
)

func TestEdgesEncodingRoundtrip(t *testing.T) {
	src := &spb.VName{Corpus: "corpus", Root: "root", Path: "path", Signature: "sig"}
	tests := []*gspb.Edges{{
		Source: src,
		Entry: &gspb.Edges_Index_{&gspb.Edges_Index{
			Node: &scpb.Node{Kind: &scpb.Node_KytheKind{scpb.NodeKind_RECORD}},
		}},
	}, {
		Source: src,
		Entry: &gspb.Edges_Edge_{&gspb.Edges_Edge{
			Kind:    &gspb.Edges_Edge_KytheKind{scpb.EdgeKind_CHILD_OF},
			Ordinal: 52,
			Reverse: true,
			Target:  &spb.VName{Signature: "target"},
		}},
	}, {
		Source: src,
		Entry: &gspb.Edges_Target_{&gspb.Edges_Target{
			Node: &scpb.Node{
				Source: &spb.VName{Signature: "target"},
				Kind:   &scpb.Node_KytheKind{scpb.NodeKind_RECORD},
			},
		}},
	}}

	for _, test := range tests {
		t.Run(fmt.Sprintf("%T", test.Entry), func(t *testing.T) {
			kv, err := EncodeEdgesEntry(nil, test)
			if err != nil {
				t.Errorf("Error encoding %T: %v", test.Entry, err)
				return
			}
			var src spb.VName
			key, err := keys.Parse(string(kv.Key), &src)
			if err != nil {
				t.Errorf("Error decoding source for %T: %v", test.Entry, err)
				return
			}
			found, err := DecodeEdgesEntry(&src, string(key), kv.Value)
			if err != nil {
				t.Errorf("Error decoding %T: %v", test.Entry, err)
			} else if diff := compare.ProtoDiff(test, found); diff != "" {
				t.Errorf("%T roundtrip differences: (- expected; + found)\n%s", test.Entry, diff)
			}
		})
	}
}
