/*
 * Copyright 2015 Google Inc. All rights reserved.
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
// filetree and xrefs serving table from a graphstore Service.
package pipeline

import (
	"bytes"
	"errors"
	"fmt"
	"log"
	"sort"
	"sync"

	"kythe.io/kythe/go/services/filetree"
	"kythe.io/kythe/go/services/graphstore"
	"kythe.io/kythe/go/services/graphstore/compare"
	ftsrv "kythe.io/kythe/go/serving/filetree"
	"kythe.io/kythe/go/serving/search"
	"kythe.io/kythe/go/serving/xrefs"
	"kythe.io/kythe/go/serving/xrefs/assemble"
	"kythe.io/kythe/go/storage/keyvalue"
	"kythe.io/kythe/go/storage/table"
	"kythe.io/kythe/go/util/disksort"
	"kythe.io/kythe/go/util/schema"

	"github.com/golang/protobuf/proto"
	"golang.org/x/net/context"

	ftpb "kythe.io/kythe/proto/filetree_proto"
	srvpb "kythe.io/kythe/proto/serving_proto"
	spb "kythe.io/kythe/proto/storage_proto"
)

// Options controls the behavior of pipeline.Run.
type Options struct {
	// MaxEdgePageSize is maximum number of edges that are allowed in the
	// PagedEdgeSet and any EdgePage.  If MaxEdgePageSize <= 0, no paging is
	// attempted.
	MaxEdgePageSize int
}

const chBuf = 512

type servingOutput struct {
	xs  table.Proto
	idx table.Inverted
}

// Run writes the xrefs and filetree serving tables to db based on the given
// graphstore.Service.
func Run(ctx context.Context, gs graphstore.Service, db keyvalue.DB, opts *Options) error {
	if opts == nil {
		opts = new(Options)
	}

	log.Println("Starting serving pipeline")

	out := &servingOutput{
		xs:  table.ProtoBatchParallel{&table.KVProto{DB: db}},
		idx: &table.KVInverted{DB: db},
	}
	entries := make(chan *spb.Entry, chBuf)

	var cErr error
	var wg sync.WaitGroup
	var sortedEdges disksort.Interface
	wg.Add(1)
	go func() {
		sortedEdges, cErr = combineNodesAndEdges(ctx, out, entries)
		if cErr != nil {
			cErr = fmt.Errorf("error combining nodes and edges: %v", cErr)
		}
		wg.Done()
	}()

	err := gs.Scan(ctx, &spb.ScanRequest{}, func(e *spb.Entry) error {
		if graphstore.IsNodeFact(e) || schema.EdgeDirection(e.EdgeKind) == schema.Forward {
			entries <- e
		}
		return nil
	})
	close(entries)
	if err != nil {
		return fmt.Errorf("error scanning GraphStore: %v", err)
	}

	wg.Wait()
	if cErr != nil {
		return cErr
	}

	pesIn, dIn := make(chan *srvpb.Edge, chBuf), make(chan *srvpb.Edge, chBuf)
	var pErr, fErr error
	wg.Add(2)
	go func() {
		defer wg.Done()
		if err := writePagedEdges(ctx, pesIn, out.xs, opts.MaxEdgePageSize); err != nil {
			pErr = fmt.Errorf("error writing paged edge sets: %v", err)
		}
	}()
	go func() {
		defer wg.Done()
		if err := writeFileDecorations(ctx, dIn, out); err != nil {
			fErr = fmt.Errorf("error writing file decorations: %v", err)
		}
	}()

	err = sortedEdges.Read(func(x interface{}) error {
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
func combineNodesAndEdges(ctx context.Context, out *servingOutput, gsEntries <-chan *spb.Entry) (disksort.Interface, error) {
	log.Println("Writing partial edges")

	tree := filetree.NewMap()

	partialSorter, err := disksort.NewMergeSorter(edgeSorterOptions)
	if err != nil {
		return nil, err
	}

	bIdx := out.idx.Buffered()
	var src *spb.VName
	var entries []*spb.Entry
	for e := range gsEntries {
		if e.FactName == schema.NodeKindFact && string(e.FactValue) == schema.FileKind {
			tree.AddFile(e.Source)
			// TODO(schroederc): evict finished directories (based on GraphStore order)
		}

		if src == nil {
			src = e.Source
		} else if !compare.VNamesEqual(e.Source, src) {
			if err := writePartialEdges(ctx, partialSorter, bIdx, assemble.SourceFromEntries(entries)); err != nil {
				drainEntries(gsEntries)
				return nil, err
			}
			src = e.Source
			entries = nil
		}

		entries = append(entries, e)
	}
	if len(entries) > 0 {
		if err := writePartialEdges(ctx, partialSorter, bIdx, assemble.SourceFromEntries(entries)); err != nil {
			return nil, err
		}
	}
	if err := bIdx.Flush(ctx); err != nil {
		return nil, err
	}

	if err := writeFileTree(ctx, tree, out.xs); err != nil {
		return nil, fmt.Errorf("error writing file tree: %v", err)
	}
	tree = nil

	log.Println("Writing complete edges")

	cSorter, err := disksort.NewMergeSorter(edgeSorterOptions)
	if err != nil {
		return nil, err
	}

	var n *srvpb.Node
	if err := partialSorter.Read(func(i interface{}) error {
		e := i.(*srvpb.Edge)
		if n == nil || n.Ticket != e.Source.Ticket {
			n = e.Source
			return cSorter.Add(e)
		} else if e.Target != nil {
			e.Source = n
			if err := writeCompletedEdges(ctx, cSorter, e); err != nil {
				return fmt.Errorf("error writing complete edge: %v", err)
			}
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
				if err := buffer.Put(ctx, ftsrv.PrefixedDirKey(corpus, root, path), dir); err != nil {
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

func writePartialEdges(ctx context.Context, sorter disksort.Interface, idx table.BufferedInverted, src *assemble.Source) error {
	edges := assemble.PartialReverseEdges(src)
	for _, pe := range edges {
		if err := sorter.Add(pe); err != nil {
			return err
		}
	}
	if err := search.IndexNode(ctx, idx, edges[0].Source); err != nil {
		return err
	}
	return nil
}

func writeCompletedEdges(ctx context.Context, edges disksort.Interface, e *srvpb.Edge) error {
	if err := edges.Add(&srvpb.Edge{
		Source: &srvpb.Node{Ticket: e.Source.Ticket},
		Kind:   e.Kind,
		Target: e.Target,
	}); err != nil {
		return fmt.Errorf("error writing complete edge: %v", err)
	}
	if err := edges.Add(&srvpb.Edge{
		Source: &srvpb.Node{Ticket: e.Target.Ticket},
		Kind:   schema.MirrorEdge(e.Kind),
		Target: assemble.FilterTextFacts(e.Source),
	}); err != nil {
		return fmt.Errorf("error writing complete edge mirror: %v", err)
	}
	return nil
}

func writePagedEdges(ctx context.Context, edges <-chan *srvpb.Edge, out table.Proto, maxEdgePageSize int) error {
	buffer := out.Buffered()
	log.Println("Writing EdgeSets")
	esb := &assemble.EdgeSetBuilder{
		MaxEdgePageSize: maxEdgePageSize,
		Output: func(ctx context.Context, pes *srvpb.PagedEdgeSet) error {
			return buffer.Put(ctx, xrefs.EdgeSetKey(pes.EdgeSet.Source.Ticket), pes)
		},
		OutputPage: func(ctx context.Context, ep *srvpb.EdgePage) error {
			return buffer.Put(ctx, xrefs.EdgePageKey(ep.PageKey), ep)
		},
	}

	var grp *srvpb.EdgeSet_Group
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
			grp = &srvpb.EdgeSet_Group{
				Kind:   e.Kind,
				Target: []*srvpb.Node{e.Target},
			}
		} else {
			grp.Target = append(grp.Target, e.Target)
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

// TODO(schroederc): use srvpb.CrossReference for fragments
type decorationFragment struct {
	fileTicket string
	decoration *srvpb.FileDecorations
}

type fragmentLesser struct{}

func (fragmentLesser) Less(a, b interface{}) bool {
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

func writeFileDecorations(ctx context.Context, edges <-chan *srvpb.Edge, out *servingOutput) error {
	fragments, err := disksort.NewMergeSorter(disksort.MergeOptions{
		Lesser:      fragmentLesser{},
		Marshaler:   fragmentMarshaler{},
		MaxInMemory: 64000,
	})
	if err != nil {
		return err
	}

	log.Println("Writing decoration fragments")
	if err := createDecorationFragments(ctx, edges, fragments); err != nil {
		return err
	}

	log.Println("Writing completed FileDecorations")

	buffer := out.xs.Buffered()
	var curFile string
	var decor *srvpb.FileDecorations
	if err := fragments.Read(func(x interface{}) error {
		df := x.(*decorationFragment)
		fileTicket := df.fileTicket
		fragment := df.decoration

		if decor != nil && curFile != fileTicket {
			if decor.File != nil {
				if err := writeDecor(ctx, buffer, decor); err != nil {
					return err
				}
			}
			decor = nil
		}
		curFile = fileTicket
		if decor == nil {
			decor = &srvpb.FileDecorations{}
		}

		if fragment.File == nil {
			decor.Decoration = append(decor.Decoration, fragment.Decoration...)
		} else {
			decor.File = fragment.File
		}

		return nil
	}); err != nil {
		return fmt.Errorf("error reading decoration fragments: %v", err)
	}

	if decor != nil && decor.File != nil {
		if err := writeDecor(ctx, buffer, decor); err != nil {
			return err
		}
	}

	return buffer.Flush(ctx)
}

func writeDecor(ctx context.Context, t table.BufferedProto, decor *srvpb.FileDecorations) error {
	sort.Sort(assemble.ByOffset(decor.Decoration))
	return t.Put(ctx, xrefs.DecorationsKey(decor.File.Ticket), decor)
}

func drainEntries(entries <-chan *spb.Entry) {
	for _ = range entries {
	}
}

type edgeLesser struct{}

func (edgeLesser) Less(a, b interface{}) bool {
	x, y := a.(*srvpb.Edge), b.(*srvpb.Edge)
	if x.Source.Ticket == y.Source.Ticket {
		if x.Kind == y.Kind {
			if x.Target == nil || y.Target == nil {
				return x.Target != nil
			}
			return x.Target.Ticket < y.Target.Ticket
		}
		return x.Kind < y.Kind
	}
	return x.Source.Ticket < y.Source.Ticket
}

type edgeMarshaler struct{}

func (edgeMarshaler) Marshal(x interface{}) ([]byte, error) { return proto.Marshal(x.(proto.Message)) }

func (edgeMarshaler) Unmarshal(rec []byte) (interface{}, error) {
	var e srvpb.Edge
	return &e, proto.Unmarshal(rec, &e)
}

var edgeSorterOptions = disksort.MergeOptions{
	Lesser:      edgeLesser{},
	Marshaler:   edgeMarshaler{},
	MaxInMemory: 16000,
}

type fragmentMarshaler struct{}

func (fragmentMarshaler) Marshal(x interface{}) ([]byte, error) {
	f := x.(*decorationFragment)
	rec, err := proto.Marshal(f.decoration)
	if err != nil {
		return nil, err
	}
	return bytes.Join([][]byte{[]byte(f.fileTicket), rec}, []byte("\000")), nil
}

func (fragmentMarshaler) Unmarshal(rec []byte) (interface{}, error) {
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
