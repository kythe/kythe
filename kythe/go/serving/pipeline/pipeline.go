/*
 * Copyright 2015 The Kythe Authors. All rights reserved.
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

// Package pipeline implements an in-process pipeline to create a combined
// filetree and xrefs serving table from a stream of GraphStore-ordered entries.
package pipeline // import "kythe.io/kythe/go/serving/pipeline"

import (
	"bytes"
	"context"
	"errors"
	"fmt"
	"sort"
	"sync"

	"kythe.io/kythe/go/services/filetree"
	"kythe.io/kythe/go/services/graphstore"
	ftsrv "kythe.io/kythe/go/serving/filetree"
	gsrv "kythe.io/kythe/go/serving/graph"
	xsrv "kythe.io/kythe/go/serving/xrefs"
	"kythe.io/kythe/go/serving/xrefs/assemble"
	"kythe.io/kythe/go/storage/keyvalue"
	"kythe.io/kythe/go/storage/stream"
	"kythe.io/kythe/go/storage/table"
	"kythe.io/kythe/go/util/disksort"
	"kythe.io/kythe/go/util/log"
	"kythe.io/kythe/go/util/schema/edges"
	"kythe.io/kythe/go/util/schema/facts"
	"kythe.io/kythe/go/util/schema/nodes"
	"kythe.io/kythe/go/util/sortutil"
	"kythe.io/kythe/go/util/span"

	"google.golang.org/protobuf/proto"

	ftpb "kythe.io/kythe/proto/filetree_go_proto"
	ipb "kythe.io/kythe/proto/internal_go_proto"
	srvpb "kythe.io/kythe/proto/serving_go_proto"
	spb "kythe.io/kythe/proto/storage_go_proto"
)

// Options controls the behavior of pipeline.Run.
type Options struct {
	// Verbose determines whether to emit extra, and possibly excessive, log messages.
	Verbose bool

	// MaxPageSize is maximum number of edges/cross-references that are allowed in
	// PagedEdgeSets, CrossReferences, EdgePages, and CrossReferences_Pages.  If
	// MaxPageSize <= 0, no paging is attempted.
	MaxPageSize int

	// CompressShards determines whether intermediate data written to disk should
	// be compressed.
	CompressShards bool

	// MaxShardSize is the maximum number of elements to keep in-memory before
	// flushing an intermediary data shard to disk.
	MaxShardSize int
}

func (o *Options) diskSorter(l sortutil.Lesser, m disksort.Marshaler) (disksort.Interface, error) {
	return disksort.NewMergeSorter(disksort.MergeOptions{
		Lesser:         l,
		Marshaler:      m,
		MaxInMemory:    o.MaxShardSize,
		CompressShards: o.CompressShards,
	})
}

const chBuf = 512

type servingOutput struct {
	xs table.Proto
}

// Run writes the xrefs and filetree serving tables to db based on the given
// entries (in GraphStore-order).
func Run(ctx context.Context, rd stream.EntryReader, db keyvalue.DB, opts *Options) error {
	if opts == nil {
		opts = new(Options)
	}

	log.Info("Starting serving pipeline")

	out := &servingOutput{
		xs: &table.KVProto{DB: db},
	}
	rd = filterReverses(rd)

	var cErr error
	var wg sync.WaitGroup
	var sortedEdges disksort.Interface
	wg.Add(1)
	go func() {
		sortedEdges, cErr = combineNodesAndEdges(ctx, opts, out, rd)
		if cErr != nil {
			cErr = fmt.Errorf("error combining nodes and edges: %v", cErr)
		}
		wg.Done()
	}()

	wg.Wait()
	if cErr != nil {
		return cErr
	}

	pesIn, dIn := make(chan *srvpb.Edge, chBuf), make(chan *srvpb.Edge, chBuf)
	var pErr, fErr error
	wg.Add(2)
	go func() {
		defer wg.Done()
		if err := writePagedEdges(ctx, pesIn, out.xs, opts); err != nil {
			pErr = fmt.Errorf("error writing paged edge sets: %v", err)
		}
	}()
	go func() {
		defer wg.Done()
		if err := writeDecorAndRefs(ctx, opts, dIn, out); err != nil {
			fErr = fmt.Errorf("error writing file decorations: %v", err)
		}
	}()

	err := sortedEdges.Read(func(x any) error {
		e := x.(*srvpb.Edge)
		pesIn <- e
		dIn <- e
		return nil
	})
	close(pesIn)
	close(dIn)
	if err != nil {
		return fmt.Errorf("error reading edges table: %v", err)
	}

	wg.Wait()
	if pErr != nil {
		return pErr
	}
	return fErr
}

func combineNodesAndEdges(ctx context.Context, opts *Options, out *servingOutput, rdIn stream.EntryReader) (disksort.Interface, error) {
	log.Info("Writing partial edges")

	tree := filetree.NewMap()
	rd := func(f func(*spb.Entry) error) error {
		return rdIn(func(e *spb.Entry) error {
			if e.FactName == facts.NodeKind && string(e.FactValue) == nodes.File {
				tree.AddFile(e.Source)
				// TODO(schroederc): evict finished directories (based on GraphStore order)
			}
			return f(e)
		})
	}

	partialSorter, err := opts.diskSorter(edgeLesser{}, edgeMarshaler{})
	if err != nil {
		return nil, err
	}

	if err := assemble.Sources(rd, func(src *ipb.Source) error {
		return writePartialEdges(ctx, partialSorter, src)
	}); err != nil {
		return nil, err
	}

	if err := writeFileTree(ctx, tree, out.xs); err != nil {
		return nil, fmt.Errorf("error writing file tree: %v", err)
	}
	tree = nil

	log.Info("Writing complete edges")

	cSorter, err := opts.diskSorter(edgeLesser{}, edgeMarshaler{})
	if err != nil {
		return nil, err
	}

	var n *srvpb.Node
	if err := partialSorter.Read(func(i any) error {
		e := i.(*srvpb.Edge)
		if n == nil || n.Ticket != e.Source.Ticket {
			n = e.Source
			if e.Target != nil {
				if opts.Verbose {
					log.Warningf("missing node facts for: %q", e.Source.Ticket)
				}
				// This is needed to satisfy later parts of the pipeline that look for targetless edges
				// to signify new nodes.
				if err := cSorter.Add(&srvpb.Edge{Source: &srvpb.Node{Ticket: e.Source.Ticket}}); err != nil {
					return fmt.Errorf("error writing complete edge: %v", err)
				}
			}
		}
		if e.Target == nil {
			// pass-through self-edges
			return cSorter.Add(e)
		}
		e.Source = n
		if err := writeCompletedEdges(ctx, cSorter, e); err != nil {
			return fmt.Errorf("error writing complete edge: %v", err)
		}
		return nil
	}); err != nil {
		return nil, fmt.Errorf("error reading/writing edges: %v", err)
	}

	return cSorter, nil
}

func writeFileTree(ctx context.Context, tree *filetree.Map, out table.Proto) error {
	buffer := out.Buffered()
	for corpus, roots := range tree.M {
		for root, dirs := range roots {
			for path, dir := range dirs {
				fd := &srvpb.FileDirectory{}
				for _, e := range dir.Entry {
					kind := srvpb.FileDirectory_UNKNOWN
					switch e.Kind {
					case ftpb.DirectoryReply_FILE:
						kind = srvpb.FileDirectory_FILE
					case ftpb.DirectoryReply_DIRECTORY:
						kind = srvpb.FileDirectory_DIRECTORY
					}
					fd.Entry = append(fd.Entry, &srvpb.FileDirectory_Entry{
						Kind: kind,
						Name: e.Name,
					})
				}
				if err := buffer.Put(ctx, ftsrv.PrefixedDirKey(corpus, root, path), fd); err != nil {
					return err
				}
			}
		}
	}
	cr, err := tree.CorpusRoots(ctx, &ftpb.CorpusRootsRequest{})
	if err != nil {
		return err
	}
	if err := buffer.Put(ctx, ftsrv.CorpusRootsPrefixedKey, cr); err != nil {
		return err
	}
	return buffer.Flush(ctx)
}

func filterReverses(rd stream.EntryReader) stream.EntryReader {
	return func(f func(*spb.Entry) error) error {
		return rd(func(e *spb.Entry) error {
			if graphstore.IsNodeFact(e) || edges.IsForward(e.EdgeKind) {
				return f(e)
			}
			return nil
		})
	}
}

func writePartialEdges(ctx context.Context, sorter disksort.Interface, src *ipb.Source) error {
	edges := assemble.PartialReverseEdges(src)
	for _, pe := range edges {
		if err := sorter.Add(pe); err != nil {
			return err
		}
	}
	return nil
}

func writeCompletedEdges(ctx context.Context, output disksort.Interface, e *srvpb.Edge) error {
	if err := output.Add(&srvpb.Edge{
		Source:  &srvpb.Node{Ticket: e.Source.Ticket},
		Kind:    e.Kind,
		Ordinal: e.Ordinal,
		Target:  e.Target,
	}); err != nil {
		return fmt.Errorf("error writing complete edge: %v", err)
	}
	if err := output.Add(&srvpb.Edge{
		Source:  &srvpb.Node{Ticket: e.Target.Ticket},
		Kind:    edges.Mirror(e.Kind),
		Ordinal: e.Ordinal,
		Target:  assemble.FilterTextFacts(e.Source),
	}); err != nil {
		return fmt.Errorf("error writing complete edge mirror: %v", err)
	}
	return nil
}

func writePagedEdges(ctx context.Context, edges <-chan *srvpb.Edge, out table.Proto, opts *Options) error {
	buffer := out.Buffered()
	log.Info("Writing EdgeSets")
	esb := &assemble.EdgeSetBuilder{
		MaxEdgePageSize: opts.MaxPageSize,
		Output: func(ctx context.Context, pes *srvpb.PagedEdgeSet) error {
			return buffer.Put(ctx, gsrv.EdgeSetKey(pes.Source.Ticket), pes)
		},
		OutputPage: func(ctx context.Context, ep *srvpb.EdgePage) error {
			return buffer.Put(ctx, gsrv.EdgePageKey(ep.PageKey), ep)
		},
	}

	var grp *srvpb.EdgeGroup
	for e := range edges {
		if grp != nil && (e.Target == nil || grp.Kind != e.Kind) {
			if err := esb.AddGroup(ctx, grp); err != nil {
				for range edges {
				} // drain input channel
				return err
			}
			grp = nil
		}

		if e.Target == nil {
			// Head-only edge: signals a new set of edges with the same Source
			if err := esb.StartEdgeSet(ctx, e.Source); err != nil {
				return err
			}
		} else if grp == nil {
			grp = &srvpb.EdgeGroup{
				Kind: e.Kind,
				Edge: []*srvpb.EdgeGroup_Edge{e2e(e)},
			}
		} else {
			grp.Edge = append(grp.Edge, e2e(e))
		}
	}

	if grp != nil {
		if err := esb.AddGroup(ctx, grp); err != nil {
			return err
		}
	}

	if err := esb.Flush(ctx); err != nil {
		return err
	}
	return buffer.Flush(ctx)
}

func e2e(e *srvpb.Edge) *srvpb.EdgeGroup_Edge {
	return &srvpb.EdgeGroup_Edge{
		Target:  e.Target,
		Ordinal: e.Ordinal,
	}
}

// TODO(schroederc): use ipb.CrossReference for fragments
type decorationFragment struct {
	fileTicket string
	decoration *srvpb.FileDecorations
}

type fragmentLesser struct{}

func (fragmentLesser) Less(a, b any) bool {
	x, y := a.(*decorationFragment), b.(*decorationFragment)
	if x.fileTicket == y.fileTicket {
		if len(x.decoration.Decoration) == 0 || len(y.decoration.Decoration) == 0 {
			return len(x.decoration.Decoration) == 0
		}
		return x.decoration.Decoration[0].Anchor.Ticket < y.decoration.Decoration[0].Anchor.Ticket
	}
	return x.fileTicket < y.fileTicket
}

func createDecorationFragments(ctx context.Context, edges <-chan *srvpb.Edge, fragments disksort.Interface) error {
	fdb := &assemble.DecorationFragmentBuilder{
		Output: func(ctx context.Context, file string, fragment *srvpb.FileDecorations) error {
			return fragments.Add(&decorationFragment{fileTicket: file, decoration: fragment})
		},
	}

	for e := range edges {
		if err := fdb.AddEdge(ctx, e); err != nil {
			for range edges { // drain input channel
			}
			return err
		}
	}

	return fdb.Flush(ctx)
}

func writeDecorAndRefs(ctx context.Context, opts *Options, edges <-chan *srvpb.Edge, out *servingOutput) error {
	fragments, err := opts.diskSorter(fragmentLesser{}, fragmentMarshaler{})
	if err != nil {
		return err
	}

	log.Info("Writing decoration fragments")
	if err := createDecorationFragments(ctx, edges, fragments); err != nil {
		return err
	}

	log.Info("Writing completed FileDecorations")

	// refSorter stores a *ipb.CrossReference for each Decoration from fragments
	refSorter, err := opts.diskSorter(refLesser{}, refMarshaler{})
	if err != nil {
		return fmt.Errorf("error creating sorter: %v", err)
	}

	buffer := out.xs.Buffered()
	var (
		curFile string
		file    *srvpb.File
		norm    *span.Normalizer
		decor   *srvpb.FileDecorations
		targets map[string]*srvpb.Node
	)
	if err := fragments.Read(func(x any) error {
		df := x.(*decorationFragment)
		fileTicket := df.fileTicket
		fragment := df.decoration

		if decor != nil && curFile != fileTicket {
			if decor.File != nil {
				if err := writeDecor(ctx, buffer, decor, targets); err != nil {
					return err
				}
				file = nil
			}
			decor = nil
		}
		curFile = fileTicket
		if decor == nil {
			decor = &srvpb.FileDecorations{}
			targets = make(map[string]*srvpb.Node)
		}

		if fragment.File == nil {
			decor.Decoration = append(decor.Decoration, fragment.Decoration...)
			for _, n := range fragment.Target {
				targets[n.Ticket] = n
			}
			if file == nil {
				log.Infof("Warning: no file set for anchor. fileTicket:[%v] curFile:[%v] fragment:[%v]", fileTicket, curFile, fragment)
				return nil
			}

			// Reverse each fragment.Decoration to create a *ipb.CrossReference
			for _, d := range fragment.Decoration {
				cr, err := assemble.CrossReference(file, norm, d, targets[d.Target])
				if err != nil {
					if opts.Verbose {
						log.Warningf("error assembling cross-reference: %v", err)
					}
					continue
				}
				if err := refSorter.Add(cr); err != nil {
					return fmt.Errorf("error adding CrossReference to sorter: %v", err)
				}

				// Snippet offsets aren't needed for the actual FileDecorations; they
				// were only needed for the above CrossReference construction
				d.Anchor.SnippetStart, d.Anchor.SnippetEnd = 0, 0
			}
		} else {
			decor.File = fragment.File
			file = fragment.File
			norm = span.NewNormalizer(file.Text)
		}

		return nil
	}); err != nil {
		return fmt.Errorf("error reading decoration fragments: %v", err)
	}

	if decor != nil && decor.File != nil {
		if err := writeDecor(ctx, buffer, decor, targets); err != nil {
			return err
		}
	}

	log.Info("Writing CrossReferences")

	xb := &assemble.CrossReferencesBuilder{
		MaxPageSize: opts.MaxPageSize,
		Output: func(ctx context.Context, s *srvpb.PagedCrossReferences) error {
			return buffer.Put(ctx, xsrv.CrossReferencesKey(s.SourceTicket), s)
		},
		OutputPage: func(ctx context.Context, p *srvpb.PagedCrossReferences_Page) error {
			return buffer.Put(ctx, xsrv.CrossReferencesPageKey(p.PageKey), p)
		},
	}
	var curTicket string
	if err := refSorter.Read(func(i any) error {
		cr := i.(*ipb.CrossReference)

		if curTicket != cr.Referent.Ticket {
			curTicket = cr.Referent.Ticket
			if err := xb.StartSet(ctx, cr.Referent); err != nil {
				return fmt.Errorf("error starting cross-references set: %v", err)
			}
		}

		g := &srvpb.PagedCrossReferences_Group{
			Kind:   cr.TargetAnchor.Kind,
			Anchor: []*srvpb.ExpandedAnchor{cr.TargetAnchor},
		}
		if err := xb.AddGroup(ctx, g); err != nil {
			return fmt.Errorf("error adding cross-reference: %v", err)
		}

		return nil
	}); err != nil {
		return fmt.Errorf("error reading xrefs: %v", err)
	}

	if err := xb.Flush(ctx); err != nil {
		return fmt.Errorf("error flushing cross-references: %v", err)
	}

	return buffer.Flush(ctx)
}

func writeDecor(ctx context.Context, t table.BufferedProto, decor *srvpb.FileDecorations, targets map[string]*srvpb.Node) error {
	for _, n := range targets {
		decor.Target = append(decor.Target, n)
	}
	sort.Sort(assemble.ByOffset(decor.Decoration))
	sort.Sort(assemble.ByTicket(decor.Target))
	sort.Sort(assemble.ByAnchorTicket(decor.TargetDefinitions))
	return t.Put(ctx, xsrv.DecorationsKey(decor.File.Ticket), decor)
}

type edgeLesser struct{}

func (edgeLesser) Less(a, b any) bool {
	x, y := a.(*srvpb.Edge), b.(*srvpb.Edge)
	if x.Source.Ticket == y.Source.Ticket {
		if x.Target == nil || y.Target == nil {
			return x.Target == nil
		}
		if x.Kind == y.Kind {
			if x.Ordinal == y.Ordinal {
				return x.Target.Ticket < y.Target.Ticket
			}
			return x.Ordinal < y.Ordinal
		}
		return x.Kind < y.Kind
	}
	return x.Source.Ticket < y.Source.Ticket
}

type edgeMarshaler struct{}

func (edgeMarshaler) Marshal(x any) ([]byte, error) { return proto.Marshal(x.(proto.Message)) }

func (edgeMarshaler) Unmarshal(rec []byte) (any, error) {
	var e srvpb.Edge
	return &e, proto.Unmarshal(rec, &e)
}

type fragmentMarshaler struct{}

func (fragmentMarshaler) Marshal(x any) ([]byte, error) {
	f := x.(*decorationFragment)
	rec, err := proto.Marshal(f.decoration)
	if err != nil {
		return nil, err
	}
	return bytes.Join([][]byte{[]byte(f.fileTicket), rec}, []byte("\000")), nil
}

func (fragmentMarshaler) Unmarshal(rec []byte) (any, error) {
	ss := bytes.SplitN(rec, []byte("\000"), 2)
	if len(ss) != 2 {
		return nil, errors.New("invalid decorationFragment encoding")
	}
	var d srvpb.FileDecorations
	if err := proto.Unmarshal(ss[1], &d); err != nil {
		return nil, err
	}
	return &decorationFragment{
		fileTicket: string(ss[0]),
		decoration: &d,
	}, nil
}

type refMarshaler struct{}

func (refMarshaler) Marshal(x any) ([]byte, error) { return proto.Marshal(x.(proto.Message)) }

func (refMarshaler) Unmarshal(rec []byte) (any, error) {
	var e ipb.CrossReference
	return &e, proto.Unmarshal(rec, &e)
}

type refLesser struct{}

func (refLesser) Less(a, b any) bool {
	x, y := a.(*ipb.CrossReference), b.(*ipb.CrossReference)
	if x.Referent.Ticket == y.Referent.Ticket {
		if x.TargetAnchor == nil || y.TargetAnchor == nil {
			return x.TargetAnchor == nil
		} else if x.TargetAnchor.Kind == y.TargetAnchor.Kind {
			if x.TargetAnchor.Span.Start.ByteOffset == y.TargetAnchor.Span.Start.ByteOffset {
				if x.TargetAnchor.Span.End.ByteOffset == y.TargetAnchor.Span.End.ByteOffset {
					return x.TargetAnchor.SnippetSpan.End.ByteOffset < y.TargetAnchor.SnippetSpan.End.ByteOffset
				}
				return x.TargetAnchor.Span.End.ByteOffset < y.TargetAnchor.Span.End.ByteOffset
			}
			return x.TargetAnchor.Span.Start.ByteOffset < y.TargetAnchor.Span.Start.ByteOffset
		}
		return x.TargetAnchor.Kind < y.TargetAnchor.Kind
	}
	return x.Referent.Ticket < y.Referent.Ticket
}
