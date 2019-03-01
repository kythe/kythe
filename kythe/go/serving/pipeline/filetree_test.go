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

	"github.com/apache/beam/sdks/go/pkg/beam"
	"github.com/apache/beam/sdks/go/pkg/beam/testing/passert"
	"github.com/apache/beam/sdks/go/pkg/beam/testing/ptest"
	"github.com/apache/beam/sdks/go/pkg/beam/x/debug"

	scpb "kythe.io/kythe/proto/schema_go_proto"
	srvpb "kythe.io/kythe/proto/serving_go_proto"
	spb "kythe.io/kythe/proto/storage_go_proto"
)

func TestCorpusRoots(t *testing.T) {
	testNodes := []*scpb.Node{{
		Source: &spb.VName{Corpus: "corpus", Root: "root", Path: "path"},
		Kind:   &scpb.Node_KytheKind{scpb.NodeKind_FILE},
	}, {
		Source: &spb.VName{Corpus: "corpus", Root: "root", Path: "path2"},
		Kind:   &scpb.Node_KytheKind{scpb.NodeKind_FILE},
	}, {
		Source: &spb.VName{Corpus: "corpus", Root: "root", Path: "path3"},
		Kind:   &scpb.Node_KytheKind{scpb.NodeKind_FILE},
	}, {
		Source: &spb.VName{Corpus: "corpus", Path: "p/to/file.go"},
		Kind:   &scpb.Node_KytheKind{scpb.NodeKind_FILE},
	}, {
		Source: &spb.VName{Corpus: "corpus2", Path: "p/to/file.go"},
		Kind:   &scpb.Node_KytheKind{scpb.NodeKind_FILE},
	}, {
		Source: &spb.VName{Corpus: "corpus", Path: "path", Signature: "a0"},
		Kind:   &scpb.Node_KytheKind{scpb.NodeKind_ANCHOR},
	}, {
		Source: &spb.VName{Corpus: "corpus", Path: "path", Signature: "a1"},
		Kind:   &scpb.Node_KytheKind{scpb.NodeKind_ANCHOR},
		Fact: []*scpb.Fact{{
			Name:  &scpb.Fact_KytheName{scpb.FactName_BUILD_CONFIG},
			Value: []byte("test-build-config"),
		}},
	}, {
		Source: &spb.VName{Corpus: "corpus", Path: "p/to/file.go", Signature: "a0"},
		Kind:   &scpb.Node_KytheKind{scpb.NodeKind_ANCHOR},
		Fact: []*scpb.Fact{{
			Name:  &scpb.Fact_KytheName{scpb.FactName_BUILD_CONFIG},
			Value: []byte("test-build-config"),
		}},
	}}

	expected := []*srvpb.CorpusRoots{{
		Corpus: []*srvpb.CorpusRoots_Corpus{{
			Corpus:      "corpus",
			Root:        []string{"", "root"},
			BuildConfig: []string{"", "test-build-config"},
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
	testNodes := []*scpb.Node{{
		Source: &spb.VName{Corpus: "corpus", Root: "root", Path: "path"},
		Kind:   &scpb.Node_KytheKind{scpb.NodeKind_FILE},
	}, {
		Source: &spb.VName{Corpus: "corpus", Root: "root", Path: "path", Signature: "a0"},
		Kind:   &scpb.Node_KytheKind{scpb.NodeKind_ANCHOR},
		Fact: []*scpb.Fact{{
			Name:  &scpb.Fact_KytheName{scpb.FactName_BUILD_CONFIG},
			Value: []byte("test-build-config"),
		}},
	}, {
		Source: &spb.VName{Corpus: "corpus", Root: "root", Path: "path", Signature: "a1"},
		Kind:   &scpb.Node_KytheKind{scpb.NodeKind_ANCHOR},
		Fact: []*scpb.Fact{{
			Name:  &scpb.Fact_KytheName{scpb.FactName_BUILD_CONFIG},
			Value: []byte("test-build-config2"),
		}},
	}, {
		Source: &spb.VName{Corpus: "corpus", Root: "root", Path: "path2"},
		Kind:   &scpb.Node_KytheKind{scpb.NodeKind_FILE},
	}, {
		Source: &spb.VName{Corpus: "corpus", Root: "root", Path: "path2", Signature: "a0"},
		Kind:   &scpb.Node_KytheKind{scpb.NodeKind_ANCHOR},
		// no build-config
	}, {
		Source: &spb.VName{Corpus: "corpus", Root: "root", Path: "path3"},
		Kind:   &scpb.Node_KytheKind{scpb.NodeKind_FILE},
	}, {
		Source: &spb.VName{Corpus: "corpus", Path: "p/to/file.go"},
		Kind:   &scpb.Node_KytheKind{scpb.NodeKind_FILE},
	}, {
		Source: &spb.VName{Corpus: "corpus", Path: "p/to/file.go", Signature: "a0"},
		Kind:   &scpb.Node_KytheKind{scpb.NodeKind_ANCHOR},
		Fact: []*scpb.Fact{{
			Name:  &scpb.Fact_KytheName{scpb.FactName_BUILD_CONFIG},
			Value: []byte("test-build-config"),
		}},
	}, {
		Source: &spb.VName{Corpus: "corpus2", Path: "p/to/file.go"},
		Kind:   &scpb.Node_KytheKind{scpb.NodeKind_FILE},
	}}

	expected := []*srvpb.FileDirectory{{
		Entry: []*srvpb.FileDirectory_Entry{{
			Kind: srvpb.FileDirectory_DIRECTORY,
			Name: "p",
		}},
	}, {
		Entry: []*srvpb.FileDirectory_Entry{{
			Kind: srvpb.FileDirectory_DIRECTORY,
			Name: "to",
		}},
	}, {
		Entry: []*srvpb.FileDirectory_Entry{{
			Kind: srvpb.FileDirectory_FILE,
			Name: "file.go",
		}},
	}, {
		Entry: []*srvpb.FileDirectory_Entry{{
			Kind:        srvpb.FileDirectory_DIRECTORY,
			Name:        "p",
			BuildConfig: []string{"test-build-config"},
		}},
	}, {
		Entry: []*srvpb.FileDirectory_Entry{{
			Kind:        srvpb.FileDirectory_DIRECTORY,
			Name:        "to",
			BuildConfig: []string{"test-build-config"},
		}},
	}, {
		Entry: []*srvpb.FileDirectory_Entry{{
			Kind:        srvpb.FileDirectory_FILE,
			Name:        "file.go",
			BuildConfig: []string{"test-build-config"},
		}},
	}, {
		Entry: []*srvpb.FileDirectory_Entry{{
			Kind:        srvpb.FileDirectory_FILE,
			Name:        "path",
			BuildConfig: []string{"test-build-config", "test-build-config2"},
		}, {
			Kind:        srvpb.FileDirectory_FILE,
			Name:        "path2",
			BuildConfig: []string{""},
		}, {
			Kind: srvpb.FileDirectory_FILE,
			Name: "path3",
		}},
	}}

	p, s, nodes := ptest.CreateList(testNodes)
	dirs := beam.DropKey(s, FromNodes(s, nodes).Directories())
	debug.Print(s, dirs)
	passert.Equals(s, dirs, beam.CreateList(s, expected))

	if err := ptest.Run(p); err != nil {
		t.Fatalf("Pipeline error: %+v", err)
	}
}
