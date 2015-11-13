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
	"fmt"
	"io"
	"io/ioutil"
	"log"
	"os"
	"sort"
	"strings"
	"sync"

	"kythe.io/kythe/go/services/filetree"
	"kythe.io/kythe/go/services/graphstore"
	"kythe.io/kythe/go/services/graphstore/compare"
	ftsrv "kythe.io/kythe/go/serving/filetree"
	"kythe.io/kythe/go/serving/search"
	"kythe.io/kythe/go/serving/xrefs"
	"kythe.io/kythe/go/serving/xrefs/assemble"
	"kythe.io/kythe/go/storage/keyvalue"
	"kythe.io/kythe/go/storage/leveldb"
	"kythe.io/kythe/go/storage/table"
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

	completeEdges *table.KVProto
}

// Run writes the xrefs and filetree serving tables to db based on the given
// graphstore.Service.
func Run(ctx context.Context, gs graphstore.Service, db keyvalue.DB, opts *Options) error {
	if opts == nil {
		opts = new(Options)
	}

	log.Println("Starting serving pipeline")

	edges, err := tempTable("complete.edges")
	if err != nil {
		return err
	}
	defer func() {
		if err := edges.Close(); err != nil {
			log.Printf("Error closing edges table: %v", err)
		}
	}()

	out := &servingOutput{
		xs:  table.ProtoBatchParallel{&table.KVProto{db}},
		idx: &table.KVInverted{db},

		completeEdges: &table.KVProto{edges},
	}
	entries := make(chan *spb.Entry, chBuf)

	var cErr error
	var wg sync.WaitGroup
	wg.Add(1)
	go func() {
		cErr = combineNodesAndEdges(ctx, out, entries)
		if cErr != nil {
			cErr = fmt.Errorf("error combining nodes and edges: %v", cErr)
		}
		wg.Done()
	}()

	err = gs.Scan(ctx, &spb.ScanRequest{}, func(e *spb.Entry) error {
		entries <- e
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

	if err := readCompletedEdges(ctx, out.completeEdges, pesIn, dIn); err != nil {
		return fmt.Errorf("error reading edges table: %v", err)
	}

	wg.Wait()
	if pErr != nil {
		return pErr
	}
	return fErr
}

func combineNodesAndEdges(ctx context.Context, out *servingOutput, gsEntries <-chan *spb.Entry) error {
	log.Println("Writing partial edges")

	tree := filetree.NewMap()

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
			if err := writePartialEdges(ctx, out, assemble.SourceFromEntries(entries)); err != nil {
				drainEntries(gsEntries)
				return err
			}
			src = e.Source
			entries = nil
		}

		entries = append(entries, e)
	}
	if len(entries) > 0 {
		if err := writePartialEdges(ctx, out, assemble.SourceFromEntries(entries)); err != nil {
			return err
		}
	}

	if err := writeFileTree(ctx, tree, out.xs); err != nil {
		return fmt.Errorf("error writing file tree: %v", err)
	}
	tree = nil

	log.Println("Writing complete edges")
	snapshot := out.completeEdges.NewSnapshot()
	defer snapshot.Close()
	it, err := out.completeEdges.ScanPrefix(nil, &keyvalue.Options{
		LargeRead: true,
		Snapshot:  snapshot,
	})
	if err != nil {
		return err
	}
	defer it.Close()

	var n *srvpb.Node
	var e srvpb.Edge
	for {
		k, v, err := it.Next()
		if err == io.EOF {
			break
		} else if err != nil {
			return fmt.Errorf("error scanning partial edges table: %v", err)
		}

		ss := strings.Split(string(k), tempTableKeySep)
		if len(ss) != 3 {
			return fmt.Errorf("invalid partial edge table key: %q", string(k))
		}

		if err := proto.Unmarshal(v, &e); err != nil {
			return fmt.Errorf("invalid partial edge table value: %v", err)
		}

		if n == nil || n.Ticket != ss[0] {
			n = e.Source
		} else if e.Target != nil {
			e.Source = n
			if err := writeCompletedEdges(ctx, out.completeEdges, &e); err != nil {
				return fmt.Errorf("error writing complete edge: %v", err)
			}
		}
	}

	return nil
}

func writeFileTree(ctx context.Context, tree *filetree.Map, out table.Proto) error {
	for corpus, roots := range tree.M {
		for root, dirs := range roots {
			for path, dir := range dirs {
				if err := out.Put(ctx, ftsrv.DirKey(corpus, root, path), dir); err != nil {
					return err
				}
			}
		}
	}
	cr, err := tree.CorpusRoots(ctx, &ftpb.CorpusRootsRequest{})
	if err != nil {
		return err
	}
	return out.Put(ctx, ftsrv.CorpusRootsKey, cr)
}

func writePartialEdges(ctx context.Context, out *servingOutput, src *assemble.Source) error {
	edges := assemble.PartialReverseEdges(src)
	for _, pe := range edges {
		var target string
		if pe.Target != nil {
			target = pe.Target.Ticket
		}
		if err := out.completeEdges.Put(ctx,
			[]byte(pe.Source.Ticket+tempTableKeySep+pe.Kind+tempTableKeySep+target), pe); err != nil {
			return fmt.Errorf("error writing partial edge: %v", err)
		}
	}
	if err := search.IndexNode(ctx, out.idx, edges[0].Source); err != nil {
		return err
	}
	return nil
}

func writeCompletedEdges(ctx context.Context, edges *table.KVProto, e *srvpb.Edge) error {
	if err := writeEdge(ctx, edges, &srvpb.Edge{
		Source: &srvpb.Node{Ticket: e.Source.Ticket},
		Kind:   e.Kind,
		Target: e.Target,
	}); err != nil {
		return fmt.Errorf("error writing complete edge: %v", err)
	}
	if err := writeEdge(ctx, edges, &srvpb.Edge{
		Source: &srvpb.Node{Ticket: e.Target.Ticket},
		Kind:   schema.MirrorEdge(e.Kind),
		Target: assemble.FilterTextFacts(e.Source),
	}); err != nil {
		return fmt.Errorf("error writing complete edge mirror: %v", err)
	}
	return nil
}

func writeEdge(ctx context.Context, edges *table.KVProto, e *srvpb.Edge) error {
	return edges.Put(ctx, []byte(e.Source.Ticket+tempTableKeySep+e.Kind+tempTableKeySep+e.Target.Ticket), e)
}

func readCompletedEdges(ctx context.Context, edges *table.KVProto, outs ...chan<- *srvpb.Edge) error {
	defer func() {
		for _, out := range outs {
			close(out)
		}
	}()

	snapshot := edges.NewSnapshot()
	defer snapshot.Close()
	it, err := edges.ScanPrefix(nil, &keyvalue.Options{
		LargeRead: true,
		Snapshot:  snapshot,
	})
	if err != nil {
		return err
	}
	defer it.Close()

	for {
		_, v, err := it.Next()
		if err == io.EOF {
			break
		} else if err != nil {
			return fmt.Errorf("error scanning edges table: %v", err)
		}

		var e srvpb.Edge
		if err := proto.Unmarshal(v, &e); err != nil {
			return fmt.Errorf("invalid edge table value: %v", err)
		}

		for _, out := range outs {
			out <- &e
		}
	}

	return nil
}

func writePagedEdges(ctx context.Context, edges <-chan *srvpb.Edge, out table.Proto, maxEdgePageSize int) error {
	log.Println("Writing EdgeSets")
	esb := &assemble.EdgeSetBuilder{
		MaxEdgePageSize: maxEdgePageSize,
		Output: func(ctx context.Context, pes *srvpb.PagedEdgeSet) error {
			return out.Put(ctx, xrefs.EdgeSetKey(pes.EdgeSet.Source.Ticket), pes)
		},
		OutputPage: func(ctx context.Context, ep *srvpb.EdgePage) error {
			return out.Put(ctx, xrefs.EdgePageKey(ep.PageKey), ep)
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

	return esb.Flush(ctx)
}

func createDecorationFragments(ctx context.Context, edges <-chan *srvpb.Edge, fragments *table.KVProto) error {
	fdb := &assemble.DecorationFragmentBuilder{
		Output: func(ctx context.Context, file string, fragment *srvpb.FileDecorations) error {
			key := file + tempTableKeySep
			if len(fragment.Decoration) != 0 {
				key += fragment.Decoration[0].Anchor.Ticket
			}
			return fragments.Put(ctx, []byte(key), fragment)
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
	temp, err := tempTable("decor.fragments")
	if err != nil {
		return fmt.Errorf("failed to create temporary table: %v", err)
	}
	fragments := &table.KVProto{temp}
	defer func() {
		if err := fragments.Close(ctx); err != nil {
			log.Printf("Error closing fragments table: %v", err)
		}
	}()

	log.Println("Writing decoration fragments")
	if err := createDecorationFragments(ctx, edges, fragments); err != nil {
		return err
	}

	log.Println("Writing completed FileDecorations")

	it, err := fragments.ScanPrefix(nil, &keyvalue.Options{LargeRead: true})
	if err != nil {
		return err
	}
	defer it.Close()

	log.Println("Writing Decorations")

	var curFile string
	var decor *srvpb.FileDecorations
	var fragment srvpb.FileDecorations
	for {
		k, v, err := it.Next()
		if err == io.EOF {
			break
		} else if err != nil {
			return fmt.Errorf("error scanning temporary table: %v", err)
		}

		ss := strings.Split(string(k), tempTableKeySep)
		if len(ss) != 2 {
			return fmt.Errorf("invalid temporary table key: %q", string(k))
		}
		fileTicket := ss[0]

		if decor != nil && curFile != fileTicket {
			if decor.FileTicket != "" {
				if err := writeDecor(ctx, out.xs, decor); err != nil {
					return err
				}
			}
			decor = nil
		}
		curFile = fileTicket
		if decor == nil {
			decor = &srvpb.FileDecorations{}
		}

		if err := proto.Unmarshal(v, &fragment); err != nil {
			return fmt.Errorf("invalid temporary table value: %v", err)
		}

		if fragment.FileTicket == "" {
			decor.Decoration = append(decor.Decoration, fragment.Decoration...)
		} else {
			decor.FileTicket = fragment.FileTicket
			decor.SourceText = fragment.SourceText
			decor.Encoding = fragment.Encoding
		}
	}

	if decor != nil && decor.FileTicket != "" {
		if err := writeDecor(ctx, out.xs, decor); err != nil {
			return err
		}
	}

	return nil
}

type deleteOnClose struct {
	keyvalue.DB
	path string
}

// Close implements part of the keyvalue.DB interface.
func (d *deleteOnClose) Close() error {
	err := d.DB.Close()
	log.Println("Removing temporary table", d.path)
	if err := os.RemoveAll(d.path); err != nil {
		log.Printf("Failed to remove temporary directory %q: %v", d.path, err)
	}
	return err
}

func tempTable(name string) (keyvalue.DB, error) {
	tempDir, err := ioutil.TempDir("", "kythe.pipeline."+name)
	if err != nil {
		return nil, fmt.Errorf("failed to create temporary directory: %v", err)
	}
	tbl, err := leveldb.Open(tempDir, nil)
	if err != nil {
		return nil, fmt.Errorf("failed to create temporary table: %v", err)
	}
	return &deleteOnClose{tbl, tempDir}, nil
}

const tempTableKeySep = "\000"

func writeDecor(ctx context.Context, t table.Proto, decor *srvpb.FileDecorations) error {
	sort.Sort(assemble.ByOffset(decor.Decoration))
	return t.Put(ctx, xrefs.DecorationsKey(decor.FileTicket), decor)
}

func drainEntries(entries <-chan *spb.Entry) {
	for _ = range entries {
	}
}
