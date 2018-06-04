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
	"fmt"
	"reflect"
	"sort"
	"strconv"

	"kythe.io/kythe/go/services/graphstore/compare"
	"kythe.io/kythe/go/services/xrefs"
	"kythe.io/kythe/go/serving/pipeline/nodes"
	"kythe.io/kythe/go/serving/xrefs/assemble"
	"kythe.io/kythe/go/util/kytheuri"
	"kythe.io/kythe/go/util/schema"
	"kythe.io/kythe/go/util/schema/facts"
	kinds "kythe.io/kythe/go/util/schema/nodes"

	"github.com/apache/beam/sdks/go/pkg/beam"

	cpb "kythe.io/kythe/proto/common_go_proto"
	ppb "kythe.io/kythe/proto/pipeline_go_proto"
	scpb "kythe.io/kythe/proto/schema_go_proto"
	srvpb "kythe.io/kythe/proto/serving_go_proto"
	spb "kythe.io/kythe/proto/storage_go_proto"
)

func init() {
	beam.RegisterFunction(keyByPath)
	beam.RegisterFunction(moveSourceToKey)
	beam.RegisterFunction(toEnclosingFile)
	beam.RegisterFunction(toFiles)
	beam.RegisterFunction(toRefs)

	beam.RegisterFunction(fileToDecorPiece)
	beam.RegisterFunction(nodeToDecorPiece)
	beam.RegisterFunction(refToDecorPiece)

	beam.RegisterType(reflect.TypeOf((*combineDecorPieces)(nil)).Elem())
	beam.RegisterType(reflect.TypeOf((*ticketKey)(nil)).Elem())
}

// KytheBeam controls the lifetime and generation of PCollections in the Kythe
// pipeline.
type KytheBeam struct {
	s beam.Scope

	nodes beam.PCollection // *ppb.Node
	files beam.PCollection // *srvpb.File
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

// Decorations returns a Kythe file decorations table derived from the Kythe
// input graph.  The beam.PCollection has elements of type
// KV<string, *srvpb.FileDecorations>.
func (k *KytheBeam) Decorations() beam.PCollection {
	s := k.s.Scope("Decorations")

	targets := beam.ParDo(s, toEnclosingFile, k.References())
	bareNodes := beam.ParDo(s, &nodes.Filter{IncludeEdges: []string{}}, k.nodes)

	decor := beam.ParDo(s, refToDecorPiece, k.References())
	files := beam.ParDo(s, fileToDecorPiece, k.getFiles())
	nodes := beam.ParDo(s, nodeToDecorPiece,
		beam.CoGroupByKey(s, beam.ParDo(s, moveSourceToKey, bareNodes), targets))
	// TODO(schroederc): definitions
	// TODO(schroederc): overrides

	pieces := beam.Flatten(s, decor, files, nodes)
	return beam.ParDo(s, &ticketKey{"decor:"}, beam.CombinePerKey(s, &combineDecorPieces{}, pieces))
}

type ticketKey struct{ Prefix string }

func (t *ticketKey) ProcessElement(key *spb.VName, val beam.T) (string, beam.T) {
	return t.Prefix + kytheuri.ToString(key), val
}

func toEnclosingFile(r *ppb.Reference) (*spb.VName, *spb.VName, error) {
	anchor, err := kytheuri.ToVName(r.Anchor.Ticket)
	if err != nil {
		return nil, nil, err
	}
	file := fileVName(anchor)
	return r.Source, file, nil
}

// combineDecorPieces :: *ppb.DecorationPiece -> *srvpb.FileDecorations
type combineDecorPieces struct{}

func (c *combineDecorPieces) CreateAccumulator() *srvpb.FileDecorations {
	return &srvpb.FileDecorations{}
}

func (c *combineDecorPieces) MergeAccumulators(accum, n *srvpb.FileDecorations) *srvpb.FileDecorations {
	return accum
}

func (c *combineDecorPieces) AddInput(accum *srvpb.FileDecorations, p *ppb.DecorationPiece) *srvpb.FileDecorations {
	if ref := p.GetReference(); ref != nil {
		var kind string
		if k := ref.GetKytheKind(); k == scpb.EdgeKind_UNKNOWN_EDGE_KIND {
			kind = ref.GetGenericKind()
		} else {
			kind = schema.EdgeKindString(k)
		}
		accum.Decoration = append(accum.Decoration, &srvpb.FileDecorations_Decoration{
			Anchor: &srvpb.RawAnchor{
				StartOffset: ref.Anchor.Span.Start.ByteOffset,
				EndOffset:   ref.Anchor.Span.End.ByteOffset,
			},
			Kind:   kind,
			Target: kytheuri.ToString(ref.Source),
		})
	} else if file := p.GetFile(); file != nil {
		accum.File = file
	} else if node := p.GetNode(); node != nil {
		n := &srvpb.Node{Ticket: kytheuri.ToString(node.Source)}
		if kind := nodes.Kind(node); kind != "" {
			n.Fact = append(n.Fact, &cpb.Fact{
				Name:  facts.NodeKind,
				Value: []byte(kind),
			})
		}
		if subkind := nodes.Subkind(node); subkind != "" {
			n.Fact = append(n.Fact, &cpb.Fact{
				Name:  facts.Subkind,
				Value: []byte(subkind),
			})
		}
		for _, f := range node.Fact {
			n.Fact = append(n.Fact, &cpb.Fact{
				Name:  nodes.FactName(f),
				Value: f.Value,
			})
		}
		sort.Slice(n.Fact, func(i, j int) bool { return n.Fact[i].Name < n.Fact[j].Name })
		accum.Target = append(accum.Target, n)
	}
	return accum
}

func (c *combineDecorPieces) ExtractOutput(fd *srvpb.FileDecorations) *srvpb.FileDecorations {
	sort.Slice(fd.Decoration, func(i, j int) bool {
		if c := compare.Ints(int(fd.Decoration[i].Anchor.StartOffset), int(fd.Decoration[j].Anchor.StartOffset)); c != compare.EQ {
			return c == compare.LT
		} else if c := compare.Ints(int(fd.Decoration[i].Anchor.EndOffset), int(fd.Decoration[j].Anchor.EndOffset)); c != compare.EQ {
			return c == compare.LT
		} else if c := compare.Strings(fd.Decoration[i].Kind, fd.Decoration[j].Kind); c != compare.EQ {
			return c == compare.LT
		}
		return fd.Decoration[i].Target < fd.Decoration[j].Target
	})
	sort.Slice(fd.Target, func(i, j int) bool { return fd.Target[i].Ticket < fd.Target[j].Ticket })
	return fd
}

func fileToDecorPiece(src *spb.VName, f *srvpb.File) (*spb.VName, *ppb.DecorationPiece) {
	return src, &ppb.DecorationPiece{Piece: &ppb.DecorationPiece_File{f}}
}

func refToDecorPiece(r *ppb.Reference) (*spb.VName, *ppb.DecorationPiece, error) {
	_, file, err := toEnclosingFile(r)
	if err != nil {
		return nil, nil, err
	}
	return file, &ppb.DecorationPiece{
		Piece: &ppb.DecorationPiece_Reference{&ppb.Reference{
			Source: r.Source,
			Kind:   r.Kind,
			Anchor: r.Anchor,
		}},
	}, nil
}

func fileVName(anchor *spb.VName) *spb.VName {
	return &spb.VName{
		Corpus: anchor.Corpus,
		Root:   anchor.Root,
		Path:   anchor.Path,
	}
}

func nodeToDecorPiece(key *spb.VName, node func(**ppb.Node) bool, file func(**spb.VName) bool, emit func(*spb.VName, *ppb.DecorationPiece)) {
	var n, singleNode *ppb.Node
	for node(&n) {
		singleNode = n
	}
	if singleNode == nil {
		return
	}

	piece := &ppb.DecorationPiece{
		Piece: &ppb.DecorationPiece_Node{&ppb.Node{
			Source:  key,
			Kind:    singleNode.Kind,
			Subkind: singleNode.Subkind,
			Fact:    singleNode.Fact,
			Edge:    singleNode.Edge,
		}},
	}

	var f *spb.VName
	for file(&f) {
		emit(f, piece)
	}
}

// Nodes returns all *ppb.Nodes from the Kythe input graph.
func (k *KytheBeam) Nodes() beam.PCollection { return k.nodes }

// References returns all derived *ppb.References from the Kythe input graph.
func (k *KytheBeam) References() beam.PCollection {
	if k.refs.IsValid() {
		return k.refs
	}
	s := k.s.Scope("References")
	anchors := beam.ParDo(s, keyByPath, beam.ParDo(s,
		&nodes.Filter{
			FilterByKind: []string{kinds.Anchor},
			IncludeFacts: []string{
				facts.AnchorStart, facts.AnchorEnd,
				facts.SnippetStart, facts.SnippetEnd,
			},
		}, k.nodes))
	k.refs = beam.ParDo(s, toRefs, beam.CoGroupByKey(s, k.getFiles(), anchors))
	return k.refs
}

func (k *KytheBeam) getFiles() beam.PCollection {
	if !k.files.IsValid() {
		fileNodes := beam.ParDo(k.s,
			&nodes.Filter{
				FilterByKind: []string{kinds.File},
				IncludeFacts: []string{facts.Text, facts.TextEncoding},
			}, k.nodes)
		k.files = beam.ParDo(k.s, toFiles, fileNodes)
	}
	return k.files
}

func keyByPath(n *ppb.Node) (*spb.VName, *ppb.Node) {
	return &spb.VName{Corpus: n.Source.Corpus, Root: n.Source.Root, Path: n.Source.Path}, n
}

func toRefs(p *spb.VName, file func(**srvpb.File) bool, anchor func(**ppb.Node) bool, emit func(*ppb.Reference)) error {
	var f *srvpb.File
	if !file(&f) {
		return nil
	}
	return normalizeAnchors(f, anchor, emit)
}

func toFiles(n *ppb.Node) (*spb.VName, *srvpb.File) {
	var f srvpb.File
	for _, fact := range n.Fact {
		switch fact.GetKytheName() {
		case scpb.FactName_TEXT:
			f.Text = fact.Value
		case scpb.FactName_TEXT_ENCODING:
			f.Encoding = string(fact.Value)
		}
	}
	return n.Source, &f
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
			if k := e.GetKytheKind(); k == scpb.EdgeKind_UNKNOWN_EDGE_KIND {
				ref.Kind = &ppb.Reference_GenericKind{e.GetGenericKind()}
			} else {
				ref.Kind = &ppb.Reference_KytheKind{k}
			}
			emit(ref)
		}
	}
	return nil
}

func toRawAnchor(n *ppb.Node) (*srvpb.RawAnchor, error) {
	var a srvpb.RawAnchor
	for _, f := range n.Fact {
		i, err := strconv.Atoi(string(f.Value))
		if err != nil {
			return nil, fmt.Errorf("invalid integer fact value for %q: %v", f.GetKytheName(), err)
		}
		n := int32(i)

		switch f.GetKytheName() {
		case scpb.FactName_LOC_START:
			a.StartOffset = n
		case scpb.FactName_LOC_END:
			a.EndOffset = n
		case scpb.FactName_SNIPPET_START:
			a.SnippetStart = n
		case scpb.FactName_SNIPPET_END:
			a.SnippetEnd = n
		default:
			return nil, fmt.Errorf("unhandled fact: %v", f)
		}
	}
	a.Ticket = kytheuri.ToString(n.Source)
	return &a, nil
}

func moveSourceToKey(n *ppb.Node) (*spb.VName, *ppb.Node) {
	return n.Source, &ppb.Node{
		Kind:    n.Kind,
		Subkind: n.Subkind,
		Fact:    n.Fact,
		Edge:    n.Edge,
	}
}
