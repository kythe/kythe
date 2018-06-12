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

	ppb "kythe.io/kythe/proto/pipeline_go_proto"
	scpb "kythe.io/kythe/proto/schema_go_proto"
	srvpb "kythe.io/kythe/proto/serving_go_proto"
	spb "kythe.io/kythe/proto/storage_go_proto"
)

func TestCorpusRoots(t *testing.T) {
	testNodes := []*ppb.Node{{
		Source: &spb.VName{Corpus: "corpus", Root: "root", Path: "path"},
		Kind:   &ppb.Node_KytheKind{scpb.NodeKind_FILE},
	}, {
		Source: &spb.VName{Corpus: "corpus", Root: "root", Path: "path2"},
		Kind:   &ppb.Node_KytheKind{scpb.NodeKind_FILE},
	}, {
		Source: &spb.VName{Corpus: "corpus", Root: "root", Path: "path3"},
		Kind:   &ppb.Node_KytheKind{scpb.NodeKind_FILE},
	}, {
		Source: &spb.VName{Corpus: "corpus", Path: "p/to/file.go"},
		Kind:   &ppb.Node_KytheKind{scpb.NodeKind_FILE},
	}, {
		Source: &spb.VName{Corpus: "corpus2", Path: "p/to/file.go"},
		Kind:   &ppb.Node_KytheKind{scpb.NodeKind_FILE},
	}}

	expected := []*srvpb.CorpusRoots{{
		Corpus: []*srvpb.CorpusRoots_Corpus{{
			Corpus: "corpus",
			Root:   []string{"", "root"},
		}, {
			Corpus: "corpus2",
			Root:   []string{""},
		}},
	}}

	p, s, nodes := ptest.CreateList(testNodes)
	cr := beam.DropKey(s, FromNodes(s, nodes).CorpusRoots())
	debug.Print(s, cr)
	passert.Equals(s, cr, beam.CreateList(s, expected))

	if err := ptest.Run(p); err != nil {
		t.Fatalf("Pipeline error: %+v", err)
	}
}

func TestDirectories(t *testing.T) {
	testNodes := []*ppb.Node{{
		Source: &spb.VName{Corpus: "corpus", Root: "root", Path: "path"},
		Kind:   &ppb.Node_KytheKind{scpb.NodeKind_FILE},
	}, {
		Source: &spb.VName{Corpus: "corpus", Root: "root", Path: "path2"},
		Kind:   &ppb.Node_KytheKind{scpb.NodeKind_FILE},
	}, {
		Source: &spb.VName{Corpus: "corpus", Root: "root", Path: "path3"},
		Kind:   &ppb.Node_KytheKind{scpb.NodeKind_FILE},
	}, {
		Source: &spb.VName{Corpus: "corpus", Path: "p/to/file.go"},
		Kind:   &ppb.Node_KytheKind{scpb.NodeKind_FILE},
	}, {
		Source: &spb.VName{Corpus: "corpus2", Path: "p/to/file.go"},
		Kind:   &ppb.Node_KytheKind{scpb.NodeKind_FILE},
	}}

	expected := []*srvpb.FileDirectory{{
		Subdirectory: []string{"kythe://corpus?path=p"},
	}, {
		Subdirectory: []string{"kythe://corpus?path=p/to"},
	}, {
		FileTicket: []string{"kythe://corpus?path=p/to/file.go"},
	}, {
		Subdirectory: []string{"kythe://corpus2?path=p"},
	}, {
		Subdirectory: []string{"kythe://corpus2?path=p/to"},
	}, {
		FileTicket: []string{"kythe://corpus2?path=p/to/file.go"},
	}, {
		FileTicket: []string{
			"kythe://corpus?path=path2?root=root",
			"kythe://corpus?path=path3?root=root",
			"kythe://corpus?path=path?root=root",
		},
	}}

	p, s, nodes := ptest.CreateList(testNodes)
	dirs := beam.DropKey(s, FromNodes(s, nodes).Directories())
	debug.Print(s, dirs)
	passert.Equals(s, dirs, beam.CreateList(s, expected))

	if err := ptest.Run(p); err != nil {
		t.Fatalf("Pipeline error: %+v", err)
	}
}
