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
	"fmt"
	"reflect"
	"sort"
	"strconv"

	"kythe.io/kythe/go/serving/pipeline/nodes"
	"kythe.io/kythe/go/serving/xrefs/assemble"
	"kythe.io/kythe/go/util/compare"
	"kythe.io/kythe/go/util/kytheuri"
	"kythe.io/kythe/go/util/schema"
	"kythe.io/kythe/go/util/schema/edges"
	"kythe.io/kythe/go/util/schema/facts"
	kinds "kythe.io/kythe/go/util/schema/nodes"
	"kythe.io/kythe/go/util/span"

	"github.com/apache/beam/sdks/go/pkg/beam"
	"github.com/golang/protobuf/proto"

	cpb "kythe.io/kythe/proto/common_go_proto"
	ppb "kythe.io/kythe/proto/pipeline_go_proto"
	scpb "kythe.io/kythe/proto/schema_go_proto"
	srvpb "kythe.io/kythe/proto/serving_go_proto"
	spb "kythe.io/kythe/proto/storage_go_proto"
)

func init() {
	beam.RegisterFunction(completeDocument)
	beam.RegisterFunction(defToDecorPiece)
	beam.RegisterFunction(fileToDecorPiece)
	beam.RegisterFunction(groupCrossRefs)
	beam.RegisterFunction(groupEdges)
	beam.RegisterFunction(keyByPath)
	beam.RegisterFunction(keyRef)
	beam.RegisterFunction(moveSourceToKey)
	beam.RegisterFunction(nodeToChildren)
	beam.RegisterFunction(nodeToDecorPiece)
	beam.RegisterFunction(nodeToDocs)
	beam.RegisterFunction(nodeToEdges)
	beam.RegisterFunction(nodeToReverseEdges)
	beam.RegisterFunction(parseMarkedSource)
	beam.RegisterFunction(refToDecorPiece)
	beam.RegisterFunction(reverseEdge)
	beam.RegisterFunction(toDefinition)
	beam.RegisterFunction(toEnclosingFile)
	beam.RegisterFunction(toFiles)
	beam.RegisterFunction(toRefs)

	beam.RegisterType(reflect.TypeOf((*combineDecorPieces)(nil)).Elem())
	beam.RegisterType(reflect.TypeOf((*ticketKey)(nil)).Elem())

	beam.RegisterType(reflect.TypeOf((*cpb.MarkedSource)(nil)).Elem())
	beam.RegisterType(reflect.TypeOf((*ppb.DecorationPiece)(nil)).Elem())
	beam.RegisterType(reflect.TypeOf((*scpb.Edge)(nil)).Elem())
	beam.RegisterType(reflect.TypeOf((*scpb.Node)(nil)).Elem())
	beam.RegisterType(reflect.TypeOf((*ppb.Reference)(nil)).Elem())
	beam.RegisterType(reflect.TypeOf((*spb.Entry)(nil)).Elem())
	beam.RegisterType(reflect.TypeOf((*spb.VName)(nil)).Elem())
	beam.RegisterType(reflect.TypeOf((*srvpb.CorpusRoots)(nil)).Elem())
	beam.RegisterType(reflect.TypeOf((*srvpb.Document)(nil)).Elem())
	beam.RegisterType(reflect.TypeOf((*srvpb.EdgePage)(nil)).Elem())
	beam.RegisterType(reflect.TypeOf((*srvpb.ExpandedAnchor)(nil)).Elem())
	beam.RegisterType(reflect.TypeOf((*srvpb.File)(nil)).Elem())
	beam.RegisterType(reflect.TypeOf((*srvpb.FileDecorations)(nil)).Elem())
	beam.RegisterType(reflect.TypeOf((*srvpb.FileDirectory)(nil)).Elem())
	beam.RegisterType(reflect.TypeOf((*srvpb.PagedCrossReferences)(nil)).Elem())
	beam.RegisterType(reflect.TypeOf((*srvpb.PagedCrossReferences_Page)(nil)).Elem())
	beam.RegisterType(reflect.TypeOf((*srvpb.PagedEdgeSet)(nil)).Elem())
}

// KytheBeam controls the lifetime and generation of PCollections in the Kythe
// pipeline.
type KytheBeam struct {
	s beam.Scope

	fileVNames beam.PCollection // *spb.VName
	nodes      beam.PCollection // *scpb.Node
	files      beam.PCollection // *srvpb.File
	refs       beam.PCollection // *ppb.Reference
}

// FromNodes creates a KytheBeam pipeline from an input collection of
// *spb.Nodes.
func FromNodes(s beam.Scope, nodes beam.PCollection) *KytheBeam { return &KytheBeam{s: s, nodes: nodes} }

// FromEntries creates a KytheBeam pipeline from an input collection of
// *spb.Entry messages.
func FromEntries(s beam.Scope, entries beam.PCollection) *KytheBeam {
	return FromNodes(s, nodes.FromEntries(s, entries))
}

// CrossReferences returns a Kythe file decorations table derived from the Kythe
// input graph.  The beam.PCollections have elements of type
// KV<string, *srvpb.PagedCrossReferences> and
// KV<string, *srvpb.PagedCrossReferences_Page>, respectively.
func (k *KytheBeam) CrossReferences() (sets, pages beam.PCollection) {
	s := k.s.Scope("CrossReferences")
	refs := beam.GroupByKey(s, beam.ParDo(s, keyRef, k.References()))
	// TODO(schroederc): related nodes
	// TODO(schroederc): callers
	// TODO(schroederc): MarkedSource
	// TODO(schroederc): source_node
	return beam.ParDo2(s, groupCrossRefs, refs)
}

// groupCrossRefs emits *srvpb.PagedCrossReferences and *srvpb.PagedCrossReferences_Pages for a
// single node's collection of *ppb.References.
func groupCrossRefs(key *spb.VName, refStream func(**ppb.Reference) bool, emitSet func(string, *srvpb.PagedCrossReferences), emitPage func(string, *srvpb.PagedCrossReferences_Page)) {
	set := &srvpb.PagedCrossReferences{SourceTicket: kytheuri.ToString(key)}
	// TODO(schroederc): add paging

	groups := make(map[string]*srvpb.PagedCrossReferences_Group)

	var ref *ppb.Reference
	for refStream(&ref) {
		kind := refKind(ref)
		g, ok := groups[kind]
		if !ok {
			g = &srvpb.PagedCrossReferences_Group{Kind: kind}
			groups[kind] = g
			set.Group = append(set.Group, g)
		}
		g.Anchor = append(g.Anchor, ref.Anchor)
	}

	sort.Slice(set.Group, func(i, j int) bool { return set.Group[i].Kind < set.Group[j].Kind })
	for _, g := range set.Group {
		sort.Slice(g.Anchor, func(i, j int) bool { return g.Anchor[i].Ticket < g.Anchor[j].Ticket })
	}

	emitSet("xrefs:"+set.SourceTicket, set)
}

func keyRef(r *ppb.Reference) (*spb.VName, *ppb.Reference) {
	return r.Source, &ppb.Reference{
		Kind:   r.Kind,
		Anchor: r.Anchor,
	}
}

func (k *KytheBeam) decorationPieces(s beam.Scope) beam.PCollection {
	targets := beam.ParDo(s, toEnclosingFile, k.References())
	bareNodes := beam.ParDo(s, &nodes.Filter{IncludeEdges: []string{}}, k.nodes)

	decor := beam.ParDo(s, refToDecorPiece, k.References())
	files := beam.ParDo(s, fileToDecorPiece, k.getFiles())
	nodes := beam.ParDo(s, nodeToDecorPiece,
		beam.CoGroupByKey(s, beam.ParDo(s, moveSourceToKey, bareNodes), targets))
	defs := beam.ParDo(s, defToDecorPiece,
		beam.CoGroupByKey(s, k.directDefinitions(), targets))
	// TODO(schroederc): overrides
	// TODO(schroederc): diagnostics

	return beam.Flatten(s, decor, files, nodes, defs)
}

// SplitDecorations returns a columnar Kythe file decorations table derived from
// the Kythe input graph.  The beam.PCollection has elements of type
// KV<[]byte, []byte>.
func (k *KytheBeam) SplitDecorations() beam.PCollection {
	s := k.s.Scope("SplitDecorations")
	return beam.ParDo(s, encodeDecorPiece, k.decorationPieces(s))
}

// Decorations returns a Kythe file decorations table derived from the Kythe
// input graph.  The beam.PCollection has elements of type
// KV<string, *srvpb.FileDecorations>.
func (k *KytheBeam) Decorations() beam.PCollection {
	s := k.s.Scope("Decorations")
	pieces := k.decorationPieces(s)
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

// combineDecorPieces combines *ppb.DecorationPieces into a single *srvpb.FileDecorations.
type combineDecorPieces struct{}

func (c *combineDecorPieces) CreateAccumulator() *srvpb.FileDecorations {
	return &srvpb.FileDecorations{}
}

func (c *combineDecorPieces) MergeAccumulators(accum, n *srvpb.FileDecorations) *srvpb.FileDecorations {
	return accum
}

func (c *combineDecorPieces) AddInput(accum *srvpb.FileDecorations, p *ppb.DecorationPiece) *srvpb.FileDecorations {
	switch p := p.Piece.(type) {
	case *ppb.DecorationPiece_Reference:
		ref := p.Reference
		accum.Decoration = append(accum.Decoration, &srvpb.FileDecorations_Decoration{
			Anchor: &srvpb.RawAnchor{
				StartOffset: ref.Anchor.Span.Start.ByteOffset,
				EndOffset:   ref.Anchor.Span.End.ByteOffset,
			},
			Kind:   refKind(ref),
			Target: kytheuri.ToString(ref.Source),
		})
	case *ppb.DecorationPiece_File:
		accum.File = p.File
	case *ppb.DecorationPiece_Node:
		accum.Target = append(accum.Target, convertPipelineNode(p.Node))
	case *ppb.DecorationPiece_Definition_:
		// TODO(schroederc): redesign *srvpb.FileDecorations to not need invasive
		// changes to add a node's definition
		def := p.Definition
		accum.TargetDefinitions = append(accum.TargetDefinitions, def.Definition)
		// Add a marker to associate the definition and node.  ExtractOutput will
		// later embed the definition within accum.Target/accum.TargetOverride.
		accum.Target = append(accum.Target, &srvpb.Node{
			Ticket:             kytheuri.ToString(def.Node),
			DefinitionLocation: &srvpb.ExpandedAnchor{Ticket: def.Definition.Ticket},
		})
	default:
		panic(fmt.Errorf("unhandled DecorationPiece: %T", p))
	}
	return accum
}

func convertPipelineNode(node *scpb.Node) *srvpb.Node {
	n := &srvpb.Node{Ticket: kytheuri.ToString(node.Source)}
	if kind := schema.GetNodeKind(node); kind != "" {
		n.Fact = append(n.Fact, &cpb.Fact{
			Name:  facts.NodeKind,
			Value: []byte(kind),
		})
	}
	if subkind := schema.GetSubkind(node); subkind != "" {
		n.Fact = append(n.Fact, &cpb.Fact{
			Name:  facts.Subkind,
			Value: []byte(subkind),
		})
	}
	for _, f := range node.Fact {
		n.Fact = append(n.Fact, &cpb.Fact{
			Name:  schema.GetFactName(f),
			Value: f.Value,
		})
	}
	sort.Slice(n.Fact, func(i, j int) bool { return n.Fact[i].Name < n.Fact[j].Name })
	return n
}

func (c *combineDecorPieces) ExtractOutput(fd *srvpb.FileDecorations) *srvpb.FileDecorations {
	// Embed definitions for Decorations and Overrides
	for i := len(fd.Target) - 1; i >= 0; i-- {
		if fd.Target[i].DefinitionLocation == nil {
			continue
		}
		node, def := fd.Target[i].Ticket, fd.Target[i].DefinitionLocation.Ticket
		fd.Target = append(fd.Target[:i], fd.Target[i+1:]...)

		for _, d := range fd.Decoration {
			if d.Target == node {
				d.TargetDefinition = def
			}
		}
		for _, o := range fd.TargetOverride {
			if o.Overridden == node {
				o.OverriddenDefinition = def
			}
		}
	}

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

func nodeToDecorPiece(key *spb.VName, node func(**scpb.Node) bool, file func(**spb.VName) bool, emit func(*spb.VName, *ppb.DecorationPiece)) {
	var n, singleNode *scpb.Node
	for node(&n) {
		singleNode = n
	}
	if singleNode == nil {
		return
	}

	piece := &ppb.DecorationPiece{
		Piece: &ppb.DecorationPiece_Node{&scpb.Node{
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

func defToDecorPiece(node *spb.VName, defs func(**srvpb.ExpandedAnchor) bool, file func(**spb.VName) bool, emit func(*spb.VName, *ppb.DecorationPiece)) {
	var def *srvpb.ExpandedAnchor
	for defs(&def) {
		// TODO(schroederc): select ambiguous definition better
		break // pick first known definition
	}
	if def == nil {
		return
	}
	piece := &ppb.DecorationPiece{
		Piece: &ppb.DecorationPiece_Definition_{&ppb.DecorationPiece_Definition{
			Node:       node,
			Definition: def,
		}},
	}
	var f *spb.VName
	for file(&f) {
		emit(f, piece)
	}
}

// Nodes returns all *scpb.Nodes from the Kythe input graph.
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

func keyByPath(n *scpb.Node) (*spb.VName, *scpb.Node) {
	return &spb.VName{Corpus: n.Source.Corpus, Root: n.Source.Root, Path: n.Source.Path}, n
}

func toRefs(p *spb.VName, file func(**srvpb.File) bool, anchor func(**scpb.Node) bool, emit func(*ppb.Reference)) error {
	var f *srvpb.File
	if !file(&f) {
		return nil
	}
	return normalizeAnchors(f, anchor, emit)
}

func toFiles(n *scpb.Node) (*spb.VName, *srvpb.File) {
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

func normalizeAnchors(file *srvpb.File, anchor func(**scpb.Node) bool, emit func(*ppb.Reference)) error {
	norm := span.NewNormalizer(file.Text)
	var n *scpb.Node
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

func toRawAnchor(n *scpb.Node) (*srvpb.RawAnchor, error) {
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

func moveSourceToKey(n *scpb.Node) (*spb.VName, *scpb.Node) {
	return n.Source, &scpb.Node{
		Kind:    n.Kind,
		Subkind: n.Subkind,
		Fact:    n.Fact,
		Edge:    n.Edge,
	}
}

func (k *KytheBeam) directDefinitions() beam.PCollection {
	s := k.s.Scope("DirectDefinitions")
	return beam.ParDo(s, toDefinition, k.References())
}

func toDefinition(r *ppb.Reference, emit func(*spb.VName, *srvpb.ExpandedAnchor)) error {
	if edges.IsVariant(refKind(r), edges.Defines) {
		emit(r.Source, r.Anchor)
	}
	return nil
}

func refKind(r *ppb.Reference) string {
	if k := r.GetKytheKind(); k != scpb.EdgeKind_UNKNOWN_EDGE_KIND {
		return schema.EdgeKindString(k)
	}
	return r.GetGenericKind()
}

// Edges returns a Kythe edges table derived from the Kythe input graph.  The beam.PCollections have
// elements of type KV<string, *srvpb.PagedEdgeSet> and KV<string, *srvpb.EdgePage>, respectively.
func (k *KytheBeam) Edges() (beam.PCollection, beam.PCollection) {
	s := k.s.Scope("Edges")

	nodes := beam.ParDo(s, moveSourceToKey, k.nodes)
	edges := beam.ParDo(s, reverseEdge, beam.CoGroupByKey(s, nodes, beam.ParDo(s, nodeToEdges, k.nodes)))
	rev := beam.ParDo(s, nodeToReverseEdges, k.nodes)

	return beam.ParDo2(s, groupEdges, beam.CoGroupByKey(s, nodes, edges, rev))
}

// nodeToReverseEdges emits an *scpb.Edge with its SourceNode populated for each of n's edges.  The
// key for each *scpb.Edge is its Target VName.
func nodeToReverseEdges(n *scpb.Node, emit func(*spb.VName, *scpb.Edge)) {
	for _, e := range n.Edge {
		emit(e.Target, &scpb.Edge{
			SourceNode: n,
			Target:     e.Target,
			Kind:       e.Kind,
			Ordinal:    e.Ordinal,
		})
	}
}

// nodeToEdges emits an *scpb.Edge for each of n's edges.  The key for each *scpb.Edge is its Target
// VName.
func nodeToEdges(n *scpb.Node, emit func(*spb.VName, *scpb.Edge)) {
	for _, e := range n.Edge {
		emit(e.Target, &scpb.Edge{
			Source:  n.Source,
			Target:  e.Target,
			Kind:    e.Kind,
			Ordinal: e.Ordinal,
		})
	}
}

// reverseEdge emits the reverse of each *scpb.Edge, embedding the associated TargetNode.
func reverseEdge(src *spb.VName, nodeStream func(**scpb.Node) bool, edgeStream func(**scpb.Edge) bool, emit func(*spb.VName, *scpb.Edge)) {
	var node *scpb.Node
	if !nodeStream(&node) {
		node = &scpb.Node{Source: src}
	}

	var e *scpb.Edge
	for edgeStream(&e) {
		emit(e.Source, &scpb.Edge{
			Source:     e.Source,
			TargetNode: node,
			Kind:       e.Kind,
			Ordinal:    e.Ordinal,
		})
	}
}

// groupEdges emits *srvpb.PagedEdgeSets and *srvpb.EdgePages for a node and its forward/reverse
// edges.
func groupEdges(src *spb.VName, nodeStream func(**scpb.Node) bool, edgeStream, revStream func(**scpb.Edge) bool, emitSet func(string, *srvpb.PagedEdgeSet), emitPage func(string, *srvpb.EdgePage)) {
	set := &srvpb.PagedEdgeSet{}
	// TODO(schroederc): paging

	var node *scpb.Node
	if nodeStream(&node) {
		node.Source = src
		set.Source = convertPipelineNode(node)
	} else {
		set.Source = &srvpb.Node{Ticket: kytheuri.ToString(src)}
	}

	groups := make(map[string]*srvpb.EdgeGroup)

	var edge *scpb.Edge
	for edgeStream(&edge) {
		kind := schema.GetEdgeKind(edge)
		g, ok := groups[kind]
		if !ok {
			g = &srvpb.EdgeGroup{Kind: kind}
			groups[kind] = g
			set.Group = append(set.Group, g)
		}
		g.Edge = append(g.Edge, &srvpb.EdgeGroup_Edge{
			Target:  convertPipelineNode(edge.TargetNode),
			Ordinal: edge.Ordinal,
		})
	}
	for revStream(&edge) {
		kind := "%" + schema.GetEdgeKind(edge) // encode reverse edge kind
		g, ok := groups[kind]
		if !ok {
			g = &srvpb.EdgeGroup{Kind: kind}
			groups[kind] = g
			set.Group = append(set.Group, g)
		}
		g.Edge = append(g.Edge, &srvpb.EdgeGroup_Edge{
			Target:  convertPipelineNode(edge.SourceNode),
			Ordinal: edge.Ordinal,
		})
	}

	sort.Slice(set.Group, func(i, j int) bool { return set.Group[i].Kind < set.Group[j].Kind })
	for _, g := range set.Group {
		sort.Slice(g.Edge, func(i, j int) bool {
			return compare.Compare(g.Edge[i].Ordinal, g.Edge[j].Ordinal).
				AndThen(g.Edge[i].Target.Ticket, g.Edge[j].Target.Ticket) == compare.LT
		})
	}

	emitSet("edgeSets:"+set.Source.Ticket, set)
}

// Documents returns a Kythe documentation table derived from the Kythe input
// graph.  The beam.PCollection has elements of type KV<string,
// *srvpb.Document>.
func (k *KytheBeam) Documents() beam.PCollection {
	s := k.s.Scope("Documents")

	docs := beam.Seq(s, k.nodes, &nodes.Filter{
		FilterByKind: []string{kinds.Doc},
		IncludeFacts: []string{facts.Text},
		IncludeEdges: []string{edges.Documents},
	}, nodeToDocs)
	markedSources := beam.Seq(s, k.nodes, &nodes.Filter{
		IncludeFacts: []string{facts.Code},
		IncludeEdges: []string{},
	}, parseMarkedSource)
	children := beam.Seq(s, k.nodes, &nodes.Filter{
		IncludeFacts: []string{},
		IncludeEdges: []string{edges.ChildOf},
	}, nodeToChildren)

	return beam.ParDo(s, completeDocument, beam.CoGroupByKey(s, docs, markedSources, children))
}

// completeDocument emits a single *srvpb.Document per *spb.VName source.
func completeDocument(key *spb.VName, docStream func(**srvpb.Document) bool, msStream func(**cpb.MarkedSource) bool, childStream func(**spb.VName) bool, emit func(string, *srvpb.Document)) {
	var doc *srvpb.Document
	if !docStream(&doc) {
		return
	}
	doc.Ticket = kytheuri.ToString(key)

	msStream(&doc.MarkedSource) // embed MarkedSource, if available

	var child *spb.VName
	for childStream(&child) {
		doc.ChildTicket = append(doc.ChildTicket, kytheuri.ToString(child))
	}
	sort.Strings(doc.ChildTicket)

	// TODO(schroederc): add definition Links
	emit("docs:"+doc.Ticket, doc)
}

// nodeToDocs emits a (*spb.VName, *srvpb.Document) pair for each
// /kythe/edge/documents edges from the given `doc` *scpb.Node.
func nodeToDocs(n *scpb.Node, emit func(*spb.VName, *srvpb.Document)) {
	d := &srvpb.Document{}
	for _, f := range n.Fact {
		if f.GetKytheName() == scpb.FactName_TEXT {
			d.RawText = string(f.Value)
			break
		}
	}

	for _, e := range n.Edge {
		if e.GetKytheKind() == scpb.EdgeKind_DOCUMENTS {
			emit(e.Target, d)
		}
	}
}

// parseMarkedSource parses the /kythe/code fact for each *scpb.Node.
func parseMarkedSource(n *scpb.Node, emit func(*spb.VName, *cpb.MarkedSource)) error {
	for _, f := range n.Fact {
		if f.GetKytheName() == scpb.FactName_CODE {
			var ms cpb.MarkedSource
			if err := proto.Unmarshal(f.Value, &ms); err != nil {
				return err
			}
			emit(n.Source, &ms)
			break
		}
	}
	return nil
}

// nodeToChildren emits a (parent, child) pair for each /kythe/edge/childof edge
// per *scpb.Node.
func nodeToChildren(n *scpb.Node, emit func(*spb.VName, *spb.VName)) {
	for _, e := range n.Edge {
		if e.GetKytheKind() == scpb.EdgeKind_CHILD_OF {
			emit(e.Target, n.Source) // parent -> child
		}
	}
}
