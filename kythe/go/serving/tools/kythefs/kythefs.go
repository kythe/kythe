/*
 * Copyright 2019 The Kythe Authors. All rights reserved.
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

// KytheFS exposes file content stored in Kythe as a virtual filesystem.
package main

import (
	"context"
	"flag"
	"fmt"
	"log"
	"os"
	"path/filepath"
	"strings"

	"kythe.io/kythe/go/services/filetree"
	"kythe.io/kythe/go/services/graph"
	"kythe.io/kythe/go/services/xrefs"
	"kythe.io/kythe/go/util/flagutil"
	"kythe.io/kythe/go/util/kytheuri"
	ftpb "kythe.io/kythe/proto/filetree_go_proto"
	gpb "kythe.io/kythe/proto/graph_go_proto"
	xpb "kythe.io/kythe/proto/xref_go_proto"

	"github.com/hanwen/go-fuse/fuse"
	"github.com/hanwen/go-fuse/fuse/nodefs"
	"github.com/hanwen/go-fuse/fuse/pathfs"
)

var (
	serverAddr = flag.String("server", "http://localhost:8080",
		"The address of the Kythe service to use. For example http://localhost:8080.")
	mountPoint = flag.String("mountpoint", "",
		"Path to existing directory to mount KytheFS at.")
)

func init() {
	flag.Usage = flagutil.SimpleUsage("Mounts file content stored in Kythe as a virtual filesystem.",
		"The files are laid out on a path <corpus>/<root>/<path>.",
		"(--mountpoint MOUNT_PATH)",
		"[--server SERVER_ADDRESS]")
}

type KytheFs struct {
	pathfs.FileSystem
	XRefs    xrefs.Service
	FileTree filetree.Service
	Graph    graph.Service
	Context  context.Context
}

// Only one of the fields has a non-default value, depending on the resolution.
type FilepathResolution struct {
	// Present if the filepath was resolved to a Kythe URI.
	//
	// For example, given corpus "foo" and root "bar/baz", KytheUri will resolve
	// as follows:
	//
	//     filepath to resolve | KytheUri
	//     "foo/bar/baz"       | "kythe://foo?root=bar/baz?path="
	//     "foo/bar/baz/quux"  | "kythe://foo?root=bar/baz?path=quux"
	//
	KytheURI *kytheuri.URI

	// Present if the filepath is a prefix of a given corpus+root.
	// Marks the next filepath component on the corpus+root filepath.
	//
	// For example, given corpus "foo" and root "bar/baz", NextDirComponent will
	// resolve as follows:
	//
	//     filepath to resolve | NextDirComponent
	//     ""                  | "foo"
	//     "foo"               | "bar"
	//     "foo/bar"           | "baz"
	//
	NextDirComponent string
}

// True if 'rs' contains a 'NextDirComponent' resolution.
func hasDirComponent(rs []FilepathResolution) bool {
	for _, r := range rs {
		if r.NextDirComponent != "" {
			return true
		}
	}
	return false
}

// Returns alternative resolutions if the path is ambiguous.
// Ambiguity comes from two sources:
//
//     a) The queried path points into a prefix of some corpus+root vfs path.
//
//     b) There are overlapping corpus+root+path vfs paths. If happens, you
//        likely need to adjust the extractor's vname mapping config.
//
func (me *KytheFs) ResolveFilepath(path string) ([]FilepathResolution, error) {
	req := ftpb.CorpusRootsRequest{}
	cr, err := me.FileTree.CorpusRoots(me.Context, &req)
	if err != nil {
		return nil, err
	}

	// Set to dedup next-dirs, as they can be common against multiple corpus+root
	// pairs.
	var nextDirs map[string]bool = make(map[string]bool)

	// Among multiple possible corpus+root pairs, prefer the longest ones.
	// We could alternatively check existence of the path with API calls, but
	// this situation shouldn't normally arise unless extraction config is broken.
	longestCorpusRootForTicket := ""
	var ticketResolution *FilepathResolution

	sep := string(os.PathSeparator)

	for _, corpus := range cr.Corpus {
		for _, root := range corpus.Root {
			crPath := filepath.Join(corpus.Name, root)
			if path == "" {
				// List root dirs, which are the first components of corpus names.
				parts := filepath.SplitList(corpus.Name)
				if len(parts) > 0 {
					nextDirs[parts[0]] = true
				}
			} else {
				crRemain := strings.TrimPrefix(crPath, path)
				vfsRemain := strings.TrimPrefix(path, crPath)
				if len(vfsRemain) < len(path) &&
					(vfsRemain == "" || vfsRemain[:1] == sep) &&
					len(crPath) > len(longestCorpusRootForTicket) {

					longestCorpusRootForTicket = crPath
					// Points inside corpus+root, match using the ticket.
					var p string = "" // Exact corpus+root match.
					if vfsRemain != "" {
						p = vfsRemain[1:] // Additional kythe path.
					}
					ticketResolution = &FilepathResolution{KytheURI: &kytheuri.URI{
						Corpus: corpus.Name,
						Root:   root,
						Path:   p,
					}}
				} else if len(crRemain) < len(crPath) && crRemain[:1] == sep {
					// Queried path is a proper prefix.
					parts := strings.Split(crRemain[1:], sep)
					if len(parts) > 0 {
						nextDirs[parts[0]] = true
					}
				}
			}
		}
	}

	var results []FilepathResolution
	for k, _ := range nextDirs {
		res := FilepathResolution{NextDirComponent: k}
		results = append(results, res)
	}
	if ticketResolution != nil {
		results = append(results, *ticketResolution)
	}
	return results, nil
}

// True if the given Kythe URI corresponds to a directory.
func (me *KytheFs) IsDirectory(uri kytheuri.URI) (bool, error) {
	ticket := uri.String()
	nodeKind := "/kythe/node/kind"
	req := &gpb.NodesRequest{
		Ticket: []string{ticket},
		Filter: []string{nodeKind},
	}
	res, err := me.Graph.Nodes(me.Context, req)
	if err != nil {
		return false, nil
	}
	for k, n := range res.Nodes {
		if k == ticket {
			for factKey, factValue := range n.Facts {
				if factKey == nodeKind && string(factValue) == "file" {
					return false, nil
				}
			}
		}
	}
	// Note: Directory entries don't have any facts usually.
	return true, nil
}

func (me *KytheFs) FetchSourceForURI(uri kytheuri.URI) ([]byte, error) {
	ticket := uri.String()
	dec, err := me.XRefs.Decorations(me.Context, &xpb.DecorationsRequest{
		Location:          &xpb.Location{Ticket: ticket},
		References:        false,
		TargetDefinitions: false,
		SourceText:        true,
	})

	if err != nil {
		return []byte{}, err
	}
	return dec.SourceText, nil
}

func (me *KytheFs) FetchSource(path string) ([]byte, error) {
	resolutions, err := me.ResolveFilepath(path)
	if err != nil {
		return []byte{}, err
	}

	for _, r := range resolutions {
		if r.KytheURI != nil {
			src, err := me.FetchSourceForURI(*r.KytheURI)

			if err != nil {
				return []byte{}, fmt.Errorf(
					"No xrefs for [%v] (resolved to ticket [%v]): [%v]",
					path, r.KytheURI.String(), err)
			}

			return src, nil
		}
	}

	return []byte{}, fmt.Errorf("Couldn't resolve path [%v] to a ticket", path)
}

//
// go-fuse stubs
//

func (me *KytheFs) GetAttr(path string, context *fuse.Context) (*fuse.Attr, fuse.Status) {
	resolutions, err := me.ResolveFilepath(path)

	if err != nil {
		log.Printf("Resolution error for [%v]: %v", path, err)
		return nil, fuse.ENOENT
	}

	if hasDirComponent(resolutions) {
		return &fuse.Attr{
			Mode: fuse.S_IFDIR | 0755,
		}, fuse.OK
	}

	for _, r := range resolutions {
		if r.KytheURI != nil {
			isDir, err := me.IsDirectory(*r.KytheURI)
			if err != nil {
				return nil, fuse.ENOENT
			}
			if isDir {
				return &fuse.Attr{
					Mode: fuse.S_IFDIR | 0755,
				}, fuse.OK
			} else {
				src, err := me.FetchSourceForURI(*r.KytheURI)
				if err != nil {
					return nil, fuse.ENOENT
				}
				return &fuse.Attr{
					Mode: fuse.S_IFREG | 0644, Size: uint64(len(src)),
				}, fuse.OK
			}
		}
	}
	return nil, fuse.ENOENT
}

func (me *KytheFs) OpenDir(path string, context *fuse.Context) (c []fuse.DirEntry, code fuse.Status) {
	resolutions, err := me.ResolveFilepath(path)

	if err != nil {
		log.Printf("Resolution error for [%v]: %v", path, err)
		return nil, fuse.ENOENT
	}

	// Key by path component, since when a corpus+root segment overlaps with
	// an actual deep path from an other corpus+root, we could get duplicate
	// components otherwise.
	ents := make(map[string]fuse.DirEntry)
	for _, r := range resolutions {
		if r.NextDirComponent != "" {
			ents[r.NextDirComponent] = fuse.DirEntry{
				Name: r.NextDirComponent,
				Mode: fuse.S_IFDIR,
			}
		} else if r.KytheURI != nil {
			req := &ftpb.DirectoryRequest{
				Corpus: r.KytheURI.Corpus,
				Root:   r.KytheURI.Root,
				Path:   r.KytheURI.Path,
			}
			dir, err := me.FileTree.Directory(me.Context, req)

			if err != nil {
				log.Printf("Error fetching dir contents for [%v] (ticket [%v]): %v",
					path, r.KytheURI.String(), err)
				return nil, fuse.ENOENT
			}

			for _, d := range dir.Subdirectory {
				uri, err := kytheuri.Parse(d)
				if err != nil {
					log.Printf("warning: received invalid directory uri %v: %v", d, err)
					continue
				}
				component := filepath.Base(uri.Path)
				ents[component] = fuse.DirEntry{
					Name: component,
					Mode: fuse.S_IFDIR,
				}
			}
			for _, f := range dir.File {
				uri, err := kytheuri.Parse(f)
				if err != nil {
					log.Printf("warning: received invalid file uri %v: %v", f, err)
					continue
				}
				component := filepath.Base(uri.Path)
				ents[component] = fuse.DirEntry{
					Name: component,
					Mode: fuse.S_IFREG,
				}
			}
		} else {
			log.Fatalf(
				"Programming error: resoultion is neither dir part nor uri for [%v]",
				path)
		}
	}
	var result []fuse.DirEntry
	for _, v := range ents {
		result = append(result, v)
	}
	return result, fuse.OK
}

func (me *KytheFs) Open(path string, flags uint32, context *fuse.Context) (file nodefs.File, code fuse.Status) {
	// Read-only filesystem.
	if flags&fuse.O_ANYWRITE != 0 {
		return nil, fuse.EPERM
	}

	src, err := me.FetchSource(path)
	if err != nil {
		log.Printf("Error fetching source for [%v]: %v", path, err)
		return nil, fuse.ENOENT
	}

	return nodefs.NewDataFile(src), fuse.OK
}

//
// Main
//

func main() {
	flag.Parse()
	if *serverAddr == "" {
		log.Fatal("You must provide --server address")
	}
	if *mountPoint == "" {
		log.Fatal("You must provide --mountpoint")
	}

	var xrefClient xrefs.Service = xrefs.WebClient(*serverAddr)
	var filetreeClient filetree.Service = filetree.WebClient(*serverAddr)
	var graphClient graph.Service = graph.WebClient(*serverAddr)

	ctx := context.Background()

	nfs := pathfs.NewPathNodeFs(&KytheFs{
		FileSystem: pathfs.NewDefaultFileSystem(),
		XRefs:      xrefClient,
		FileTree:   filetreeClient,
		Graph:      graphClient,
		Context:    ctx,
	}, nil)

	server, _, err := nodefs.MountRoot(*mountPoint, nfs.Root(), nil)

	if err != nil {
		log.Fatalf("Mounting failed: %v", err)
	}

	server.Serve()
}
