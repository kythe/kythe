/*
 * Copyright 2014 Google Inc. All rights reserved.
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

// Package tables provides an xrefs.Service implementation backed by static
// serving tables located on disk.  It also provides a way to Fill the tables
// using an existing GraphStore.
//
// Currently there are 4 separate lookup tables:
//   decorations: ticket -> [DecorationReply_Reference]
//   nodes: ticket -> {factName: []byte(factValue)}
//   edges: ticket -> {edgeKind: [ticket]}
//   dirs: ticket -> {"dirs": [dirName], "files": [VName]}
package tables

import (
	"bytes"
	"encoding/json"
	"errors"
	"fmt"
	"io"
	"os"
	"os/exec"
	"path/filepath"
	"sync"

	"kythe/go/serving/xrefs"
	"kythe/go/storage"
	"kythe/go/storage/filetree"
	"kythe/go/storage/keyvalue"
	"kythe/go/storage/leveldb"
	"kythe/go/util/kytheuri"
	"kythe/go/util/schema"
	"kythe/go/util/stringset"

	spb "kythe/proto/storage_proto"
	xpb "kythe/proto/xref_proto"

	"code.google.com/p/goprotobuf/proto"
)

// Tables is a set of static lookup tables that can serve as an xrefs.Service
// and a filetree.FileTree.
type Tables struct {
	// TODO(schroederc): use a single namespaced keyvalue.DB (LevelDB)
	decorations *jsonLookupTable
	nodes       *jsonLookupTable
	edges       *jsonLookupTable
	dirs        *jsonLookupTable

	// TODO(schroederc): add/use xrefs table
	//   ticket -> {edgeKind: [{"anchor": ticket, "file": VName, "start": offset, "end": offset}]
}

// Open opens the set of lookup tables located in the given directory.
func Open(dirPath string) (*Tables, error) {
	err := os.MkdirAll(dirPath, os.ModePerm)
	if err != nil {
		return nil, fmt.Errorf("error making %q: %v", dirPath, err)
	}

	tbls := new(Tables)
	tbls.decorations, err = openTable(filepath.Join(dirPath, "decorations"))
	if err != nil {
		return nil, err
	}
	tbls.nodes, err = openTable(filepath.Join(dirPath, "nodes"))
	if err != nil {
		return nil, err
	}
	tbls.edges, err = openTable(filepath.Join(dirPath, "edges"))
	if err != nil {
		return nil, err
	}
	tbls.dirs, err = openTable(filepath.Join(dirPath, "dirs"))
	if err != nil {
		return nil, err
	}

	return tbls, nil
}

// XRefs returns an xrefs.Service backed by the Tables data.
func (t *Tables) XRefs() xrefs.Service { return t }

// FileTree returns a filetree.FileTree backed by the Tables data.
func (t *Tables) FileTree() filetree.FileTree { return t }

// Nodes implements part of the xrefs.Service interface.
func (t *Tables) Nodes(req *xpb.NodesRequest) (*xpb.NodesReply, error) {
	patterns := xrefs.ConvertFilters(req.Filter)
	reply := new(xpb.NodesReply)
	for _, ticket := range req.Ticket {
		var facts nodeFacts
		if err := t.nodes.get(ticket, &facts); err != nil {
			return nil, fmt.Errorf("error getting facts for %q: %v", ticket, err)
		}

		info := &xpb.NodeInfo{Ticket: proto.String(ticket)}
		for name, val := range facts {
			if len(patterns) == 0 || xrefs.MatchesAny(name, patterns) {
				info.Fact = append(info.Fact, &xpb.Fact{
					Name:  proto.String(name),
					Value: val,
				})
			}
		}
		if len(info.Fact) > 0 {
			reply.Node = append(reply.Node, info)
		}
	}
	return reply, nil
}

// Edges implements part of the xrefs.Service interface.
func (t *Tables) Edges(req *xpb.EdgesRequest) (*xpb.EdgesReply, error) {
	if req.GetPageToken() != "" {
		return nil, errors.New("UNIMPLEMENTED: page_token")
	}

	allowedKinds := stringset.New(req.Kind...)
	nodeSet := stringset.New()
	reply := new(xpb.EdgesReply)

	for _, ticket := range req.Ticket {
		var edges nodeEdges
		if err := t.edges.get(ticket, &edges); err != nil {
			return nil, fmt.Errorf("error getting edges for %q: %v", ticket, err)
		}

		es := &xpb.EdgeSet{SourceTicket: proto.String(ticket)}
		for kind, targets := range edges {
			if len(allowedKinds) == 0 || allowedKinds.Contains(kind) {
				es.Group = append(es.Group, &xpb.EdgeSet_Group{
					Kind:         proto.String(kind),
					TargetTicket: targets,
				})
				nodeSet.Add(targets...)
			}
		}
		if len(es.Group) > 0 {
			nodeSet.Add(ticket)
			reply.EdgeSet = append(reply.EdgeSet, es)
		}
	}

	nodesReply, err := t.Nodes(&xpb.NodesRequest{
		Ticket: nodeSet.Slice(),
		Filter: req.Filter,
	})
	if err != nil {
		return nil, err
	}
	reply.Node = nodesReply.Node

	return reply, nil
}

// Decorations implements part of the xrefs.Service interface.
func (t *Tables) Decorations(req *xpb.DecorationsRequest) (*xpb.DecorationsReply, error) {
	if len(req.DirtyBuffer) > 0 {
		return nil, errors.New("UNIMPLEMENTED: patching")
	} else if req.Location.GetKind() != xpb.Location_FILE {
		return nil, errors.New("UNIMPLEMENTED: span locations")
	}

	ticket := req.Location.GetTicket()
	reply := &xpb.DecorationsReply{Location: req.Location}
	if req.GetSourceText() {
		var file nodeFacts
		if err := t.nodes.get(ticket, &file); err != nil {
			return nil, fmt.Errorf("error getting source text: %v", err)
		}

		if val := file[schema.FileEncodingFact]; val != nil {
			reply.Encoding = proto.String(string(val))
		}
		reply.SourceText = file[schema.FileTextFact]
	}

	if req.GetReferences() {
		var refs []*xpb.DecorationsReply_Reference
		if err := t.decorations.get(ticket, &refs); err != nil {
			return nil, fmt.Errorf("error getting references: %v", err)
		}
		reply.Reference = refs

		nodes := stringset.New()
		for _, ref := range refs {
			nodes.Add(ref.GetSourceTicket(), ref.GetTargetTicket())
		}

		nodesReply, err := t.Nodes(&xpb.NodesRequest{Ticket: nodes.Slice()})
		if err != nil {
			return nil, err
		}

		reply.Node = nodesReply.Node
	}

	return reply, nil
}

const corporaRootsKey = "__corpora_roots"

// CorporaRoots implements part of the filetree.FileTree interface.
func (t *Tables) CorporaRoots() (map[string][]string, error) {
	var corporaRoots map[string][]string
	return corporaRoots, t.dirs.get(corporaRootsKey, &corporaRoots)
}

// Dir implements part of the filetree.FileTree interface.
func (t *Tables) Dir(corpus, root, path string) (*filetree.Directory, error) {
	uri := &kytheuri.URI{
		Corpus: corpus,
		Root:   root,
		Path:   path,
	}
	var dir filetree.Directory
	return &dir, t.dirs.get(uri.String(), &dir)
}

// DumpToText writes .txt files for each of the xrefs tables
func (t *Tables) DumpToText() error {
	if err := t.nodes.dumpToText(); err != nil {
		return fmt.Errorf("error dumping nodes: %v", err)
	}
	if err := t.edges.dumpToText(); err != nil {
		return fmt.Errorf("error dumping edges: %v", err)
	}
	if err := t.decorations.dumpToText(); err != nil {
		return fmt.Errorf("error dumping decorations: %v", err)
	}
	if err := t.dirs.dumpToText(); err != nil {
		return fmt.Errorf("error dumping dirs: %v", err)
	}
	return nil
}

// Fill constructs entries in the xrefs Tables with the contents of gs.
func (t *Tables) Fill(gs storage.GraphStore) (err error) {
	xs := xrefs.NewGraphStoreService(gs)

	entries := make(chan *spb.Entry)
	var wg sync.WaitGroup
	wg.Add(1)
	go func() {
		defer wg.Done()
		defer func() {
			if pErr := recover(); pErr != nil && err == nil {
				switch pErr := pErr.(type) {
				case error:
					err = pErr
				default:
					err = fmt.Errorf("panic error: %v", pErr)
				}
			}
		}()
		// In this routine, we panic on any error, immediately recover from it
		// above, and then propagate it to Fill's return error variable without
		// clobbering any existing error.

		tree := filetree.NewMap()
		for node := range collectFacts(entries) {
			uri := kytheuri.FromVName(node.VName).String()
			panicOnErr(t.nodes.put(uri, node.Facts))
			panicOnErr(t.edges.put(uri, node.Edges))

			if string(node.Facts[schema.NodeKindFact]) == schema.FileKind {
				tree.AddFile(node.VName)
				reply, err := xs.Decorations(&xpb.DecorationsRequest{
					Location:   &xpb.Location{Ticket: &uri},
					References: proto.Bool(true),
				})
				panicOnErr(err)
				panicOnErr(t.decorations.put(uri, reply.Reference))
			}
		}

		for corpus, roots := range tree.M {
			for root, dirs := range roots {
				for path, dir := range dirs {
					uri := &kytheuri.URI{
						Corpus: corpus,
						Root:   root,
						Path:   path,
					}
					panicOnErr(t.dirs.put(uri.String(), dir))
				}
			}
		}
		corporaRoots, err := tree.CorporaRoots()
		panicOnErr(err)
		panicOnErr(t.dirs.put(corporaRootsKey, corporaRoots))
	}()
	err = gs.Scan(&spb.ScanRequest{}, entries)
	close(entries)
	wg.Wait()
	return
}

type jsonLookupTable struct {
	keyvalue.DB
	path string
}

func openTable(path string) (*jsonLookupTable, error) {
	path, err := filepath.Abs(path)
	if err != nil {
		return nil, fmt.Errorf("path error: %v", err)
	}
	db, err := leveldb.Open(path, nil)
	if err != nil {
		return nil, err
	}
	return &jsonLookupTable{db, path}, nil
}

func (t *jsonLookupTable) put(key string, val interface{}) (err error) {
	bytes, err := json.Marshal(val)
	if err != nil {
		return fmt.Errorf("error marshaling: %v", err)
	}
	wr, err := t.Writer()
	if err != nil {
		return fmt.Errorf("db writer error: %v", err)
	}
	defer func() {
		if cErr := wr.Close(); err == nil {
			err = cErr
		}
	}()
	if err := wr.Write([]byte(key), bytes); err != nil {
		return fmt.Errorf("write error: %v", err)
	}
	return nil
}

var errNoSuchKey = errors.New("no such key")

func (t *jsonLookupTable) get(key string, val interface{}) error {
	bKey := []byte(key)
	rd, err := t.Reader(bKey, nil)
	if err != nil {
		return fmt.Errorf("db reader error: %v", err)
	}
	defer rd.Close()

	k, v, err := rd.Next()
	if err == io.EOF {
		return errNoSuchKey
	} else if err != nil {
		return fmt.Errorf("read error: %v", err)
	} else if !bytes.Equal(bKey, k) {
		return errNoSuchKey
	}

	return json.Unmarshal(v, val)
}

func (t *jsonLookupTable) dumpToText() (err error) {
	path := t.path + ".txt"
	f, err := os.Create(path)
	if err != nil {
		return fmt.Errorf("error creating %q: %v", path, err)
	}
	defer func() {
		if cErr := f.Close(); err == nil {
			err = cErr
			return
		}
		if sErr := exec.Command("sort", "-k1", f.Name(), "-o", f.Name()).Run(); err == nil {
			err = sErr
		}
	}()

	it, err := t.Reader(nil, nil)
	if err != nil {
		return fmt.Errorf("db scanner error: %v", err)
	}
	defer it.Close()

	for {
		k, v, err := it.Next()
		if err == io.EOF {
			break
		} else if err != nil {
			return fmt.Errorf("read error: %v", err)
		}

		if _, err := fmt.Fprintln(f, string(k), string(v)); err != nil {
			return fmt.Errorf("write error: %v", err)
		}
	}

	return nil
}

type nodeFacts map[string][]byte
type nodeEdges map[string][]string

type node struct {
	VName *spb.VName
	Facts nodeFacts
	Edges nodeEdges
}

func collectFacts(entries <-chan *spb.Entry) <-chan *node {
	ch := make(chan *node)
	go func() {
		defer close(ch)
		var n *node
		for entry := range entries {
			if n != nil && !storage.VNameEqual(n.VName, entry.GetSource()) {
				ch <- n
				n = nil
			}

			if n == nil {
				n = &node{
					VName: entry.GetSource(),
					Facts: make(map[string][]byte),
					Edges: make(map[string][]string),
				}
			}

			edgeKind := entry.GetEdgeKind()
			if edgeKind == "" {
				n.Facts[entry.GetFactName()] = entry.FactValue
			} else {
				n.Edges[edgeKind] = append(n.Edges[edgeKind], kytheuri.FromVName(entry.GetTarget()).String())
			}
		}
		if n != nil {
			ch <- n
		}
	}()
	return ch
}

func panicOnErr(err error) {
	if err != nil {
		panic(err)
	}
}
