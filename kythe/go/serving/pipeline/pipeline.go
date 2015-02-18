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
	"io/ioutil"
	"log"
	"os"
	"strconv"
	"sync"

	"kythe/go/services/filetree"
	"kythe/go/services/graphstore"
	"kythe/go/services/graphstore/compare"
	"kythe/go/services/xrefs"
	ftsrv "kythe/go/serving/filetree"
	"kythe/go/serving/search"
	xsrv "kythe/go/serving/xrefs"
	"kythe/go/storage/keyvalue"
	"kythe/go/storage/leveldb"
	"kythe/go/storage/table"
	"kythe/go/util/kytheuri"
	"kythe/go/util/schema"

	srvpb "kythe/proto/serving_proto"
	spb "kythe/proto/storage_proto"
	xpb "kythe/proto/xref_proto"

	"github.com/golang/protobuf/proto"
)

// maxIndexedFactValueSize is the maximum length in bytes of fact values to
// write to the inverted index search table.
const maxIndexedFactValueSize = 512

// Run writes the xrefs and filetree serving tables to db based on the given
// graphstore.Service.
func Run(gs graphstore.Service, db keyvalue.DB) error {
	log.Println("Starting serving pipeline")
	tbl := &table.KVProto{db}

	// TODO(schroederc): for large corpora, this won't fit in memory
	var files []string

	entries := make(chan *spb.Entry)
	ftIn, nIn, eIn := make(chan *spb.VName), make(chan *spb.Entry), make(chan *spb.Entry)
	go func() {
		for entry := range entries {
			if entry.GetEdgeKind() == "" {
				nIn <- entry
				if entry.GetFactName() == schema.NodeKindFact && string(entry.FactValue) == "file" {
					ftIn <- entry.Source
					files = append(files, kytheuri.ToString(entry.Source))
				}
			} else {
				eIn <- entry
			}
		}
		close(ftIn)
		close(nIn)
		close(eIn)
	}()
	log.Println("Scanning GraphStore")
	var sErr error
	go func() {
		sErr = gs.Scan(&spb.ScanRequest{}, func(e *spb.Entry) error {
			entries <- e
			return nil
		})
		close(entries)
	}()

	var (
		ftErr, nErr, eErr error
		ftWG, edgeNodeWG  sync.WaitGroup
	)
	ftWG.Add(1)
	go func() {
		defer ftWG.Done()
		ftErr = writeFileTree(tbl, ftIn)
		log.Println("Wrote FileTree")
	}()
	edgeNodeWG.Add(2)
	nodes := make(chan *srvpb.Node)
	go func() {
		defer edgeNodeWG.Done()
		nErr = writeNodes(tbl, nIn, nodes)
		log.Println("Wrote Nodes")
	}()
	go func() {
		defer edgeNodeWG.Done()
		eErr = writeEdges(tbl, eIn)
		log.Println("Wrote Edges")
	}()

	var (
		idxWG  sync.WaitGroup
		idxErr error
	)
	idxWG.Add(1)
	go func() {
		defer idxWG.Done()
		idxErr = writeIndex(&table.KVInverted{db}, nodes)
		log.Println("Wrote Search Index")
	}()

	edgeNodeWG.Wait()
	if eErr != nil {
		return eErr
	} else if nErr != nil {
		return nErr
	}

	es := xrefs.NodesEdgesService(&xsrv.Table{tbl})
	if err := writeDecorations(tbl, es, files); err != nil {
		return err
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

func writeFileTree(t table.Proto, files <-chan *spb.VName) error {
	tree := filetree.NewMap()
	for f := range files {
		tree.AddFile(f)
		// TODO(schroederc): evict finished directories (based on GraphStore order)
	}

	for corpus, roots := range tree.M {
		for root, dirs := range roots {
			for path, dir := range dirs {
				if err := t.Put(ftsrv.DirKey(corpus, root, path), dir); err != nil {
					return err
				}
			}
		}
	}
	cr, err := tree.CorporaRoots()
	if err != nil {
		return err
	}
	return t.Put(ftsrv.CorpusRootsKey, cr)
}

func writeNodes(t table.Proto, nodeEntries <-chan *spb.Entry, nodes chan<- *srvpb.Node) error {
	defer close(nodes)
	for node := range collectNodes(nodeEntries) {
		nodes <- node
		if err := t.Put(xsrv.NodeKey(node.GetTicket()), node); err != nil {
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
				node = &srvpb.Node{Ticket: &ticket}
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

func writeEdges(t table.Proto, edges <-chan *spb.Entry) error {
	tempDir, err := ioutil.TempDir("", "reverse.edges")
	if err != nil {
		return fmt.Errorf("failed to create temporary directory: %v", err)
	}
	defer func() {
		log.Println("Removing temporary edges table", tempDir)
		if err := os.RemoveAll(tempDir); err != nil {
			log.Printf("Failed to remove temporary directory %q: %v", tempDir, err)
		}
	}()
	gs, err := leveldb.OpenGraphStore(tempDir, nil)
	if err != nil {
		return fmt.Errorf("failed to create temporary GraphStore: %v", err)
	}
	defer gs.Close()

	log.Println("Writing temporary reverse edges table")
	var writeReq *spb.WriteRequest
	for e := range edges {
		if writeReq != nil && !compare.VNamesEqual(e.Source, writeReq.Source) {
			if err := writeWithReverses(gs, writeReq); err != nil {
				return err
			}
			writeReq = nil
		}
		if writeReq == nil {
			writeReq = &spb.WriteRequest{Source: e.Source}
		}
		writeReq.Update = append(writeReq.Update, &spb.WriteRequest_Update{
			Target:    e.Target,
			EdgeKind:  e.EdgeKind,
			FactName:  e.FactName,
			FactValue: e.FactValue,
		})
	}
	if writeReq != nil {
		if err := writeWithReverses(gs, writeReq); err != nil {
			return err
		}
	}

	return writeEdgePages(t, gs)
}

func writeEdgePages(t table.Proto, gs graphstore.Service) error {
	// TODO(schroederc): spill large PagedEdgeSets into EdgePages
	log.Println("Writing EdgeSets")
	var (
		lastSrc  *spb.VName
		pes      *srvpb.PagedEdgeSet
		grp      *srvpb.EdgeSet_Group
		pesTotal int
	)
	if err := gs.Scan(nil, func(e *spb.Entry) error {
		if e.GetEdgeKind() == "" {
			panic("non-edge entry")
		}

		if pes != nil && !compare.VNamesEqual(lastSrc, e.Source) {
			if grp != nil {
				pes.EdgeSet.Group = append(pes.EdgeSet.Group, grp)
				pesTotal += len(grp.TargetTicket)
			}
			pes.TotalEdges = proto.Int(pesTotal)
			if err := t.Put(xsrv.EdgeSetKey(pes.EdgeSet.GetSourceTicket()), pes); err != nil {
				return err
			}
			pes = nil
			grp = nil
			pesTotal = 0
		}
		if pes == nil {
			pes = &srvpb.PagedEdgeSet{
				EdgeSet: &srvpb.EdgeSet{
					SourceTicket: proto.String(kytheuri.ToString(e.Source)),
				},
			}
		}

		if grp != nil && grp.GetKind() != e.GetEdgeKind() {
			pes.EdgeSet.Group = append(pes.EdgeSet.Group, grp)
			pesTotal += len(grp.TargetTicket)
			grp = nil
		}
		if grp == nil {
			grp = &srvpb.EdgeSet_Group{
				Kind: e.EdgeKind,
			}
		}

		grp.TargetTicket = append(grp.TargetTicket, kytheuri.ToString(e.Target))
		lastSrc = e.Source
		return nil
	}); err != nil {
		return err
	}
	if pes != nil {
		if grp != nil {
			pes.EdgeSet.Group = append(pes.EdgeSet.Group, grp)
			pesTotal += len(grp.TargetTicket)
		}
		pes.TotalEdges = proto.Int(pesTotal)
		if err := t.Put(xsrv.EdgeSetKey(pes.EdgeSet.GetSourceTicket()), pes); err != nil {
			return err
		}
	}
	return nil
}

func writeWithReverses(gs graphstore.Service, req *spb.WriteRequest) error {
	if err := gs.Write(req); err != nil {
		return fmt.Errorf("error writing edges: %v", err)
	}
	for _, u := range req.Update {
		if err := gs.Write(&spb.WriteRequest{
			Source: u.Target,
			Update: []*spb.WriteRequest_Update{{
				Target:    req.Source,
				EdgeKind:  proto.String(schema.MirrorEdge(u.GetEdgeKind())),
				FactName:  u.FactName,
				FactValue: u.FactValue,
			}},
		}); err != nil {
			return fmt.Errorf("error writing rev edge: %v", err)
		}
	}
	return nil
}

var revChildOfEdgeKind = schema.MirrorEdge(schema.ChildOfEdge)

func writeDecorations(t table.Proto, es xrefs.NodesEdgesService, files []string) error {
	log.Println("Writing Decorations")

	edges := make(chan *xpb.EdgesReply)
	var eErr error
	go func() {
		eErr = readEdges(es, files, edges,
			decorationFilters, []string{revChildOfEdgeKind})
		close(edges)
	}()

	for e := range edges {
		decor := &srvpb.FileDecorations{}
		if len(e.EdgeSet) == 0 {
			if len(e.Node) != 1 {
				log.Println("ERROR: missing node for non-decoration file")
				continue
			}
			decor.FileTicket = e.Node[0].Ticket
		} else if len(e.EdgeSet) != 1 {
			log.Println("ERROR: invalid number of decoration EdgeSets:", len(e.EdgeSet))
			continue
		} else {
			decor.FileTicket = e.EdgeSet[0].SourceTicket
		}

		for _, n := range e.Node {
			if n.GetTicket() == decor.GetFileTicket() {
				for _, f := range n.Fact {
					switch f.GetName() {
					case schema.FileTextFact:
						decor.SourceText = f.Value
					case schema.FileEncodingFact:
						decor.Encoding = proto.String(string(f.GetValue()))
					}
				}
			} else {
				ds, err := getDecorations(es, n)
				if err != nil {
					return err
				}
				decor.Decoration = append(decor.Decoration, ds...)
			}
		}

		if err := t.Put(xsrv.DecorationsKey(decor.GetFileTicket()), decor); err != nil {
			return err
		}
	}

	return eErr
}

func getDecorations(es xrefs.EdgesService, anchor *xpb.NodeInfo) ([]*srvpb.FileDecorations_Decoration, error) {
	var (
		isAnchor   bool
		start, end int
		err        error
	)
	for _, f := range anchor.Fact {
		switch f.GetName() {
		case schema.NodeKindFact:
			if string(f.Value) == schema.AnchorKind {
				isAnchor = true
			}
		case schema.AnchorStartFact:
			start, err = strconv.Atoi(string(f.Value))
			if err != nil {
				return nil, fmt.Errorf("invalid anchor %q start offset: %q", anchor.GetTicket(), string(f.Value))
			}
		case schema.AnchorEndFact:
			end, err = strconv.Atoi(string(f.Value))
			if err != nil {
				return nil, fmt.Errorf("invalid anchor %q end offset: %q", anchor.GetTicket(), string(f.Value))
			}
		}
	}
	if !isAnchor {
		return nil, nil
	}

	edges, err := es.Edges(&xpb.EdgesRequest{Ticket: []string{anchor.GetTicket()}})
	if err != nil {
		return nil, err
	}
	if len(edges.EdgeSet) != 1 {
		return nil, fmt.Errorf("invalid number of EdgeSets returned for anchor: %d", len(edges.EdgeSet))
	}

	a := &srvpb.FileDecorations_Decoration_Anchor{
		Ticket:      anchor.Ticket,
		StartOffset: proto.Int(start),
		EndOffset:   proto.Int(end),
	}
	var ds []*srvpb.FileDecorations_Decoration
	for _, grp := range edges.EdgeSet[0].Group {
		if schema.EdgeDirection(grp.GetKind()) == schema.Forward && grp.GetKind() != schema.ChildOfEdge {
			for _, target := range grp.TargetTicket {
				ds = append(ds, &srvpb.FileDecorations_Decoration{
					Anchor:       a,
					Kind:         grp.Kind,
					TargetTicket: proto.String(target),
				})
			}
		}
	}
	return ds, nil
}

var decorationFilters = []string{
	schema.NodeKindFact,
	schema.FileTextFact,
	schema.FileEncodingFact,
	schema.AnchorLocFilter,
}

func readEdges(es xrefs.NodesEdgesService, files []string, edges chan<- *xpb.EdgesReply, filters []string, kinds []string) error {
	var eErr error
	for _, file := range files {
		if eErr == nil {
			reply, err := es.Edges(&xpb.EdgesRequest{
				Ticket: []string{file},
				Filter: filters,
				Kind:   kinds,
			})
			if err != nil {
				eErr = err
				continue
			}
			if len(reply.EdgeSet) == 0 {
				// File does not have any decorations, but we still want the source text/encoding.
				nodeReply, err := es.Nodes(&xpb.NodesRequest{
					Ticket: []string{file},
					Filter: filters,
				})
				if err != nil {
					return fmt.Errorf("error getting file node: %v", err)
				}
				reply.Node = nodeReply.Node
			}
			edges <- reply
		}
	}
	return eErr
}

func writeIndex(t table.Inverted, nodes <-chan *srvpb.Node) error {
	for n := range nodes {
		uri, err := kytheuri.Parse(n.GetTicket())
		if err != nil {
			return err
		}
		key := []byte(n.GetTicket())

		if uri.Signature != "" {
			if err := t.Put(key, search.VNameVal("signature", uri.Signature)); err != nil {
				return err
			}
		}
		if uri.Corpus != "" {
			if err := t.Put(key, search.VNameVal("corpus", uri.Corpus)); err != nil {
				return err
			}
		}
		if uri.Root != "" {
			if err := t.Put(key, search.VNameVal("root", uri.Root)); err != nil {
				return err
			}
		}
		if uri.Path != "" {
			if err := t.Put(key, search.VNameVal("path", uri.Path)); err != nil {
				return err
			}
		}
		if uri.Language != "" {
			if err := t.Put(key, search.VNameVal("language", uri.Language)); err != nil {
				return err
			}
		}

		for _, f := range n.Fact {
			if len(f.Value) <= maxIndexedFactValueSize {
				if err := t.Put(key, search.FactVal(f.GetName(), f.Value)); err != nil {
					return err
				}
			}
		}
	}
	return nil
}
