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
	"strconv"
	"strings"
	"sync"

	"kythe.io/kythe/go/services/filetree"
	"kythe.io/kythe/go/services/graphstore"
	"kythe.io/kythe/go/services/graphstore/compare"
	ftsrv "kythe.io/kythe/go/serving/filetree"
	"kythe.io/kythe/go/serving/search"
	"kythe.io/kythe/go/serving/xrefs"
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

const chBuf = 512

// Run writes the xrefs and filetree serving tables to db based on the given
// graphstore.Service.
func Run(ctx context.Context, gs graphstore.Service, db keyvalue.DB) error {
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
		eErr = writeEdges(ctx, tbl, eIn)
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
			node.Fact = append(node.Fact, &srvpb.Node_Fact{
				Name:  e.FactName,
				Value: e.FactValue,
			})
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

func writeEdges(ctx context.Context, t table.Proto, edges <-chan *spb.Entry) error {
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

	return writeEdgePages(ctx, t, edgeGroups)
}

func writeEdgePages(ctx context.Context, t table.Proto, edgeGroups *table.KVProto) error {
	// TODO(schroederc): spill large PagedEdgeSets into EdgePages

	it, err := edgeGroups.ScanPrefix(nil, &keyvalue.Options{LargeRead: true})
	if err != nil {
		return err
	}
	defer it.Close()

	log.Println("Writing EdgeSets")
	var (
		lastSrc  string
		pes      *srvpb.PagedEdgeSet
		grp      *srvpb.EdgeSet_Group
		pesTotal int
	)
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
		curSrc := ss[0]

		if pes != nil && lastSrc != curSrc {
			if grp != nil {
				pes.EdgeSet.Group = append(pes.EdgeSet.Group, grp)
				pesTotal += len(grp.TargetTicket)
			}
			pes.TotalEdges = int32(pesTotal)
			if err := t.Put(ctx, xrefs.EdgeSetKey(pes.EdgeSet.SourceTicket), pes); err != nil {
				return err
			}
			pes = nil
			grp = nil
			pesTotal = 0
		}
		if pes == nil {
			pes = &srvpb.PagedEdgeSet{
				EdgeSet: &srvpb.EdgeSet{
					SourceTicket: curSrc,
				},
			}
		}

		var cur srvpb.EdgeSet_Group
		if err := proto.Unmarshal(v, &cur); err != nil {
			return fmt.Errorf("invalid edge groups table value: %v", err)
		}

		if grp != nil && grp.Kind != cur.Kind {
			pes.EdgeSet.Group = append(pes.EdgeSet.Group, grp)
			pesTotal += len(grp.TargetTicket)
			grp = nil
		}
		if grp == nil {
			grp = &cur
		} else {
			grp.TargetTicket = append(grp.TargetTicket, cur.TargetTicket...)
		}
		lastSrc = curSrc
	}
	if pes != nil {
		if grp != nil {
			pes.EdgeSet.Group = append(pes.EdgeSet.Group, grp)
			pesTotal += len(grp.TargetTicket)
		}
		pes.TotalEdges = int32(pesTotal)
		if err := t.Put(ctx, xrefs.EdgeSetKey(pes.EdgeSet.SourceTicket), pes); err != nil {
			return err
		}
	}
	return nil
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

	for src := range collectSources(entries) {
		for _, fragment := range decorationFragments(src) {
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
	sort.Sort(byOffset(decor.Decoration))
	return t.Put(ctx, xrefs.DecorationsKey(decor.FileTicket), decor)
}

type byOffset []*srvpb.FileDecorations_Decoration

func (s byOffset) Len() int      { return len(s) }
func (s byOffset) Swap(i, j int) { s[i], s[j] = s[j], s[i] }
func (s byOffset) Less(i, j int) bool {
	if s[i].Anchor.StartOffset < s[j].Anchor.StartOffset {
		return true
	} else if s[i].Anchor.StartOffset > s[j].Anchor.StartOffset {
		return false
	} else if s[i].Anchor.EndOffset == s[j].Anchor.EndOffset {
		if s[i].Kind == s[j].Kind {
			return s[i].TargetTicket < s[j].TargetTicket
		}
		return s[i].Kind < s[j].Kind
	}
	return s[i].Anchor.EndOffset < s[j].Anchor.EndOffset
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

// Source is a collection of facts and edges with a common source.
type Source struct {
	Ticket string

	Facts map[string][]byte
	Edges map[string][]string
}

func collectSources(entries <-chan *spb.Entry) <-chan *Source {
	ch := make(chan *Source, 1)

	go func() {
		defer close(ch)

		var es []*spb.Entry
		for entry := range entries {
			if len(es) > 0 && compare.VNames(es[0].Source, entry.Source) != compare.EQ {
				ch <- toSource(es)
				es = nil
			}

			es = append(es, entry)
		}
		if len(es) > 0 {
			ch <- toSource(es)
		}
	}()

	return ch
}

func toSource(entries []*spb.Entry) *Source {
	if len(entries) == 0 {
		return nil
	}

	src := &Source{
		Ticket: kytheuri.ToString(entries[0].Source),
		Facts:  make(map[string][]byte),
		Edges:  make(map[string][]string),
	}

	edgeTargets := make(map[string]stringset.Set)

	for _, e := range entries {
		if graphstore.IsEdge(e) {
			tgts, ok := edgeTargets[e.EdgeKind]
			if !ok {
				tgts = stringset.New()
				edgeTargets[e.EdgeKind] = tgts
			}
			tgts.Add(kytheuri.ToString(e.Target))
		} else {
			src.Facts[e.FactName] = e.FactValue
		}
	}
	for kind, targets := range edgeTargets {
		src.Edges[kind] = targets.Slice()
		sort.Strings(src.Edges[kind])
	}

	return src
}

func decorationFragments(src *Source) []*srvpb.FileDecorations {
	switch string(src.Facts[schema.NodeKindFact]) {
	default:
		return nil
	case schema.FileKind:
		return []*srvpb.FileDecorations{{
			FileTicket: src.Ticket,
			SourceText: src.Facts[schema.TextFact],
			Encoding:   string(src.Facts[schema.TextEncodingFact]),
		}}
	case schema.AnchorKind:
		anchorStart, err := strconv.Atoi(string(src.Facts[schema.AnchorStartFact]))
		if err != nil {
			log.Printf("Error parsing anchor start offset %q: %v", string(src.Facts[schema.AnchorStartFact]), err)
			return nil
		}
		anchorEnd, err := strconv.Atoi(string(src.Facts[schema.AnchorEndFact]))
		if err != nil {
			log.Printf("Error parsing anchor end offset %q: %v", string(src.Facts[schema.AnchorEndFact]), err)
			return nil
		}

		anchor := &srvpb.FileDecorations_Decoration_Anchor{
			Ticket:      src.Ticket,
			StartOffset: int32(anchorStart),
			EndOffset:   int32(anchorEnd),
		}

		kinds := make([]string, len(src.Edges)-1)
		for kind := range src.Edges {
			if kind == schema.ChildOfEdge {
				continue
			}
			kinds = append(kinds, kind)
		}
		sort.Strings(kinds) // to ensure consistency

		var decor []*srvpb.FileDecorations_Decoration
		for _, kind := range kinds {
			for _, tgt := range src.Edges[kind] {
				decor = append(decor, &srvpb.FileDecorations_Decoration{
					Anchor:       anchor,
					Kind:         kind,
					TargetTicket: tgt,
				})
			}
		}
		if len(decor) == 0 {
			return nil
		}

		// Assume each anchor parent is a file
		parents := src.Edges[schema.ChildOfEdge]
		dm := make([]*srvpb.FileDecorations, len(parents))

		for i, parent := range parents {
			dm[i] = &srvpb.FileDecorations{
				FileTicket: parent,
				Decoration: decor,
			}
		}
		return dm
	}
}
