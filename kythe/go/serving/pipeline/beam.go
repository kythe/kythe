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
	"strconv"

	"kythe.io/kythe/go/services/xrefs"
	"kythe.io/kythe/go/serving/pipeline/nodes"
	"kythe.io/kythe/go/serving/xrefs/assemble"
	"kythe.io/kythe/go/util/schema/facts"
	kinds "kythe.io/kythe/go/util/schema/nodes"

	"github.com/apache/beam/sdks/go/pkg/beam"

	ppb "kythe.io/kythe/proto/pipeline_go_proto"
	scpb "kythe.io/kythe/proto/schema_go_proto"
	srvpb "kythe.io/kythe/proto/serving_go_proto"
	spb "kythe.io/kythe/proto/storage_go_proto"
)

func init() {
	beam.RegisterFunction(toRefs)
	beam.RegisterFunction(keyByPath)
}

// KytheBeam controls the lifetime and generation of PCollections in the Kythe
// pipeline.
type KytheBeam struct {
	s beam.Scope

	nodes beam.PCollection // *ppb.Node
	refs  beam.PCollection // *ppb.Reference
}

// FromNodes creates a KytheBeam pipeline from an input collection of
// *spb.Nodes.
func FromNodes(s beam.Scope, nodes beam.PCollection) *KytheBeam { return &KytheBeam{s: s, nodes: nodes} }

// FromEntries creates a KytheBeam pipeline from an input collection of
// *spb.Entry messages.
func FromEntries(s beam.Scope, entries beam.PCollection) *KytheBeam {
	return FromNodes(s, nodes.FromEntries(s, entries))
}

// Nodes returns all *ppb.Nodes from the Kythe input graph.
func (k *KytheBeam) Nodes() beam.PCollection { return k.nodes }

// References returns all derived *ppb.References from the Kythe input graph.
func (k *KytheBeam) References() beam.PCollection {
	if !k.refs.IsValid() {
		s := k.s.Scope("References")
		anchors := beam.ParDo(s, keyByPath, beam.ParDo(s,
			&nodes.Filter{
				FilterByKind: []string{kinds.Anchor},
				IncludeFacts: []string{
					facts.AnchorStart, facts.AnchorEnd,
					facts.SnippetStart, facts.SnippetEnd,
				},
			}, k.nodes))
		files := beam.ParDo(s, keyByPath, beam.ParDo(s,
			&nodes.Filter{
				FilterByKind: []string{kinds.File},
				IncludeFacts: []string{facts.Text},
			}, k.nodes))

		k.refs = beam.ParDo(s, toRefs, beam.CoGroupByKey(s, files, anchors))
	}
	return k.refs
}

func keyByPath(n *ppb.Node) (*spb.VName, *ppb.Node) {
	return &spb.VName{Corpus: n.Source.Corpus, Root: n.Source.Root, Path: n.Source.Path}, n
}

func toRefs(p *spb.VName, file func(**ppb.Node) bool, anchor func(**ppb.Node) bool, emit func(*ppb.Reference)) error {
	var fileNode *ppb.Node
	if !file(&fileNode) {
		return nil
	}
	var f srvpb.File
	for _, fact := range fileNode.Fact {
		if fact.GetKytheName() == scpb.FactName_TEXT {
			f.Text = fact.Value
			break
		}
	}
	return normalizeAnchors(&f, anchor, emit)
}

func normalizeAnchors(file *srvpb.File, anchor func(**ppb.Node) bool, emit func(*ppb.Reference)) error {
	norm := xrefs.NewNormalizer(file.Text)
	var n *ppb.Node
	for anchor(&n) {
		raw, err := toRawAnchor(n)
		if err != nil {
			return err
		}
		a, err := assemble.ExpandAnchor(raw, file, norm, "")
		if err != nil {
			return err
		}

		var parent *spb.VName
		for _, e := range n.Edge {
			if e.GetKytheKind() == scpb.EdgeKind_CHILD_OF {
				parent = e.Target
				break
			}
		}

		for _, e := range n.Edge {
			if e.GetKytheKind() == scpb.EdgeKind_CHILD_OF {
				continue
			}
			ref := &ppb.Reference{
				Source: e.Target,
				Anchor: a,
				Scope:  parent,
			}
			if k := e.GetKytheKind(); k != scpb.EdgeKind_UNKNOWN_EDGE_KIND {
				ref.Kind = &ppb.Reference_KytheKind{k}
			} else {
				ref.Kind = &ppb.Reference_GenericKind{e.GetGenericKind()}
			}
			emit(ref)
		}
	}
	return nil
}

func toRawAnchor(n *ppb.Node) (*srvpb.RawAnchor, error) {
	var a srvpb.RawAnchor
	for _, f := range n.Fact {
		switch f.GetKytheName() {
		case scpb.FactName_LOC_START:
			n, err := strconv.Atoi(string(f.Value))
			if err != nil {
				return nil, err
			}
			a.StartOffset = int32(n)
		case scpb.FactName_LOC_END:
			n, err := strconv.Atoi(string(f.Value))
			if err != nil {
				return nil, err
			}
			a.EndOffset = int32(n)
		case scpb.FactName_SNIPPET_START:
			n, err := strconv.Atoi(string(f.Value))
			if err != nil {
				return nil, err
			}
			a.SnippetStart = int32(n)
		case scpb.FactName_SNIPPET_END:
			n, err := strconv.Atoi(string(f.Value))
			if err != nil {
				return nil, err
			}
			a.SnippetEnd = int32(n)
		}
	}
	return &a, nil
}
