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
	"kythe.io/kythe/go/util/kytheuri"
	"kythe.io/kythe/go/util/schema"
	"kythe.io/kythe/go/util/stringset"

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

// Run writes the xrefs and filetree serving tables to db based on the given
// graphstore.Service.
func Run(ctx context.Context, gs graphstore.Service, db keyvalue.DB, opts *Options) error {
	if opts == nil {
		opts = new(Options)
	}

	log.Println("Starting serving pipeline")
	tbl := table.ProtoBatchParallel{&table.KVProto{db}}

	entries := make(chan *spb.Entry, chBuf)

	ftIn := make(chan *spb.VName, chBuf)
	nIn, eIn, dIn := make(chan *spb.Entry, chBuf), make(chan *spb.Entry, chBuf), make(chan *spb.Entry, chBuf)

	go func() {
		for entry := range entries {
			if graphstore.IsNodeFact(entry) {
				nIn <- entry
				if entry.FactName == schema.NodeKindFact && string(entry.FactValue) == "file" {
					ftIn <- entry.Source
				}
			} else {
				eIn <- entry
			}
			dIn <- entry
		}
		close(ftIn)
		close(nIn)
		close(eIn)
		close(dIn)
	}()
	log.Println("Scanning GraphStore")
	var sErr error
	go func() {
		sErr = gs.Scan(ctx, &spb.ScanRequest{}, func(e *spb.Entry) error {
			entries <- e
			return nil
		})
		if sErr != nil {
			sErr = fmt.Errorf("error scanning GraphStore: %v", sErr)
		}
		close(entries)
	}()

	var (
		ftErr, nErr, eErr, dErr error
		ftWG, xrefsWG           sync.WaitGroup
	)
	ftWG.Add(1)
	go func() {
		defer ftWG.Done()
		ftErr = writeFileTree(ctx, tbl, ftIn)
		if ftErr != nil {
			ftErr = fmt.Errorf("error writing FileTree: %v", ftErr)
		} else {
			log.Println("Wrote FileTree")
		}
	}()
	xrefsWG.Add(3)
	nodes := make(chan *srvpb.Node)
	go func() {
		defer xrefsWG.Done()
		nErr = writeNodes(ctx, tbl, nIn, nodes)
		if nErr != nil {
			nErr = fmt.Errorf("error writing Nodes: %v", nErr)
		} else {
			log.Println("Wrote Nodes")
		}
	}()
	go func() {
		defer xrefsWG.Done()
		eErr = writeEdges(ctx, tbl, eIn, opts.MaxEdgePageSize)
		if eErr != nil {
			eErr = fmt.Errorf("error writing Edges: %v", eErr)
		} else {
			log.Println("Wrote Edges")
		}
	}()
	go func() {
		defer xrefsWG.Done()
		dErr = writeDecorations(ctx, tbl, dIn)
		if dErr != nil {
			dErr = fmt.Errorf("error writing FileDecorations: %v", dErr)
		} else {
			log.Println("Wrote Decorations")
		}
	}()

	var (
		idxWG  sync.WaitGroup
		idxErr error
	)
	idxWG.Add(1)
	go func() {
		defer idxWG.Done()
		idxErr = writeIndex(ctx, &table.KVInverted{db}, nodes)
		if idxErr != nil {
			idxErr = fmt.Errorf("error writing Search Index: %v", idxErr)
		} else {
			log.Println("Wrote Search Index")
		}
	}()

	xrefsWG.Wait()
	if eErr != nil {
		return eErr
	} else if nErr != nil {
		return nErr
	} else if dErr != nil {
		return dErr
	}

	ftWG.Wait()
	if ftErr != nil {
		return ftErr
	}
	idxWG.Wait()
	if idxErr != nil {
		return idxErr
	}

	return sErr
}

func writeFileTree(ctx context.Context, t table.Proto, files <-chan *spb.VName) error {
	tree := filetree.NewMap()
	for f := range files {
		tree.AddFile(f)
		// TODO(schroederc): evict finished directories (based on GraphStore order)
	}

	for corpus, roots := range tree.M {
		for root, dirs := range roots {
			for path, dir := range dirs {
				if err := t.Put(ctx, ftsrv.DirKey(corpus, root, path), dir); err != nil {
					return err
				}
			}
		}
	}
	cr, err := tree.CorpusRoots(ctx, &ftpb.CorpusRootsRequest{})
	if err != nil {
		return err
	}
	return t.Put(ctx, ftsrv.CorpusRootsKey, cr)
}

func writeNodes(ctx context.Context, t table.Proto, nodeEntries <-chan *spb.Entry, nodes chan<- *srvpb.Node) error {
	defer close(nodes)
	defer drainEntries(nodeEntries) // ensure channel is drained on errors
	for node := range collectNodes(nodeEntries) {
		nodes <- node
		if err := t.Put(ctx, xrefs.NodeKey(node.Ticket), node); err != nil {
			return err
		}
	}
	return nil
}

func collectNodes(nodeEntries <-chan *spb.Entry) <-chan *srvpb.Node {
	nodes := make(chan *srvpb.Node)
	go func() {
		var (
			node  *srvpb.Node
			vname *spb.VName
		)
		for e := range nodeEntries {
			if node != nil && !compare.VNamesEqual(e.Source, vname) {
				nodes <- node
				node = nil
				vname = nil
			}
			if node == nil {
				vname = e.Source
				ticket := kytheuri.ToString(vname)
				node = &srvpb.Node{Ticket: ticket}
			}
			node.Fact = append(node.Fact, assemble.NodeFact(e))
		}
		if node != nil {
			nodes <- node
		}
		close(nodes)
	}()
	return nodes
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

func writeEdges(ctx context.Context, t table.Proto, edges <-chan *spb.Entry, maxEdgePageSize int) error {
	defer drainEntries(edges) // ensure channel is drained on errors

	temp, err := tempTable("edge.groups")
	if err != nil {
		return fmt.Errorf("failed to create temporary table: %v", err)
	}
	edgeGroups := &table.KVProto{temp}
	defer func() {
		if err := edgeGroups.Close(ctx); err != nil {
			log.Println("Error closing edge groups table: %v", err)
		}
	}()

	log.Println("Writing temporary edges table")

	var (
		src     *spb.VName
		kind    string
		targets stringset.Set
	)
	for e := range edges {
		if src != nil && (!compare.VNamesEqual(e.Source, src) || kind != e.EdgeKind) {
			if err := writeWithReverses(ctx, edgeGroups, kytheuri.ToString(src), kind, targets.Slice()); err != nil {
				return err
			}
			src = nil
		}
		if src == nil {
			src = e.Source
			kind = e.EdgeKind
			targets = stringset.New()
		}
		targets.Add(kytheuri.ToString(e.Target))
	}
	if src != nil {
		if err := writeWithReverses(ctx, edgeGroups, kytheuri.ToString(src), kind, targets.Slice()); err != nil {
			return err
		}
	}

	return writeEdgePages(ctx, t, edgeGroups, maxEdgePageSize)
}

func writeEdgePages(ctx context.Context, t table.Proto, edgeGroups *table.KVProto, maxEdgePageSize int) error {
	// TODO(schroederc): spill large PagedEdgeSets into EdgePages

	it, err := edgeGroups.ScanPrefix(nil, &keyvalue.Options{LargeRead: true})
	if err != nil {
		return err
	}
	defer it.Close()

	log.Println("Writing EdgeSets")
	esb := &assemble.EdgeSetBuilder{
		MaxEdgePageSize: maxEdgePageSize,
		Output: func(ctx context.Context, pes *srvpb.PagedEdgeSet) error {
			return t.Put(ctx, xrefs.EdgeSetKey(pes.EdgeSet.SourceTicket), pes)
		},
		OutputPage: func(ctx context.Context, ep *srvpb.EdgePage) error {
			return t.Put(ctx, xrefs.EdgePageKey(ep.PageKey), ep)
		},
	}
	for {
		k, v, err := it.Next()
		if err == io.EOF {
			break
		} else if err != nil {
			return fmt.Errorf("error scanning edge groups table: %v", err)
		}

		ss := strings.Split(string(k), tempTableKeySep)
		if len(ss) != 3 {
			return fmt.Errorf("invalid edge groups table key: %q", string(k))
		}
		src := ss[0]

		var eg srvpb.EdgeSet_Group
		if err := proto.Unmarshal(v, &eg); err != nil {
			return fmt.Errorf("invalid edge groups table value: %v", err)
		}

		if err := esb.AddGroup(ctx, src, &eg); err != nil {
			return err
		}
	}
	return esb.Flush(ctx)
}

func writeWithReverses(ctx context.Context, tbl *table.KVProto, src, kind string, targets []string) error {
	if err := tbl.Put(ctx, []byte(src+tempTableKeySep+kind+tempTableKeySep), &srvpb.EdgeSet_Group{
		Kind:         kind,
		TargetTicket: targets,
	}); err != nil {
		return fmt.Errorf("error writing edges group: %v", err)
	}
	revGroup := &srvpb.EdgeSet_Group{
		Kind:         schema.MirrorEdge(kind),
		TargetTicket: []string{src},
	}
	for _, tgt := range targets {
		if err := tbl.Put(ctx, []byte(tgt+tempTableKeySep+revGroup.Kind+tempTableKeySep+src), revGroup); err != nil {
			return fmt.Errorf("error writing rev edges group: %v", err)
		}
	}
	return nil
}

var revChildOfEdgeKind = schema.MirrorEdge(schema.ChildOfEdge)

const tempTableKeySep = "\000"

func createFragmentsTable(ctx context.Context, entries <-chan *spb.Entry) (t *table.KVProto, err error) {
	log.Println("Writing decoration fragments")

	defer drainEntries(entries) // ensure channel is drained on errors

	temp, err := tempTable("decor.fragments")
	if err != nil {
		return nil, fmt.Errorf("failed to create temporary table: %v", err)
	}
	fragments := &table.KVProto{temp}
	defer func() {
		if err != nil {
			if err := fragments.Close(ctx); err != nil {
				log.Println("Error closing fragments table: %v", err)
			}
		}
	}()

	for src := range assemble.Sources(entries) {
		for _, fragment := range assemble.DecorationFragments(src) {
			fileTicket := fragment.FileTicket
			var anchorTicket string
			if len(fragment.Decoration) != 0 {
				// don't keep decor.FileTicket for anchors; we aren't sure we have a file yet
				fragment.FileTicket = ""
				anchorTicket = fragment.Decoration[0].Anchor.Ticket
			}
			if err := fragments.Put(ctx, []byte(fileTicket+tempTableKeySep+anchorTicket), fragment); err != nil {
				return nil, err
			}
		}
	}
	return fragments, nil
}

func writeDecorations(ctx context.Context, t table.Proto, entries <-chan *spb.Entry) error {
	fragments, err := createFragmentsTable(ctx, entries)
	if err != nil {
		return fmt.Errorf("error creating fragments table: %v", err)
	}
	defer fragments.Close(ctx)

	it, err := fragments.ScanPrefix(nil, &keyvalue.Options{LargeRead: true})
	if err != nil {
		return err
	}
	defer it.Close()

	log.Println("Writing Decorations")

	var curFile string
	var decor *srvpb.FileDecorations
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
				if err := writeDecor(ctx, t, decor); err != nil {
					return err
				}
			}
			decor = nil
		}
		curFile = fileTicket
		if decor == nil {
			decor = &srvpb.FileDecorations{}
		}

		var fragment srvpb.FileDecorations
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
		if err := writeDecor(ctx, t, decor); err != nil {
			return err
		}
	}

	return nil
}

func writeDecor(ctx context.Context, t table.Proto, decor *srvpb.FileDecorations) error {
	sort.Sort(assemble.ByOffset(decor.Decoration))
	return t.Put(ctx, xrefs.DecorationsKey(decor.FileTicket), decor)
}

func writeIndex(ctx context.Context, t table.Inverted, nodes <-chan *srvpb.Node) error {
	for n := range nodes {
		if err := search.IndexNode(ctx, t, n); err != nil {
			return err
		}
	}
	return nil
}

func drainEntries(entries <-chan *spb.Entry) {
	for _ = range entries {
	}
}
