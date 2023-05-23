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

// Binary KytheFS exposes file content stored in Kythe as a virtual filesystem.
//
// Example usage:
//
//	bazel build kythe/go/serving/tools/kythefs
//	# Blocks until unmounted:
//	./bazel-bin/kythe/go/serving/tools/kythefs/kythefs --mountpoint vfs_dir
//
//	# To unmount:
//	fusermount -u vfs_dir
package main

import (
	"context"
	"flag"
	"fmt"
	"os"
	"path/filepath"
	"strings"

	"kythe.io/kythe/go/serving/api"
	"kythe.io/kythe/go/util/flagutil"
	"kythe.io/kythe/go/util/kytheuri"
	"kythe.io/kythe/go/util/log"
	"kythe.io/kythe/go/util/schema/facts"

	"github.com/hanwen/go-fuse/fuse"
	"github.com/hanwen/go-fuse/fuse/nodefs"
	"github.com/hanwen/go-fuse/fuse/pathfs"

	ftpb "kythe.io/kythe/proto/filetree_go_proto"
	gpb "kythe.io/kythe/proto/graph_go_proto"
	xpb "kythe.io/kythe/proto/xref_go_proto"
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

type kytheFS struct {
	pathfs.FileSystem
	Context context.Context
	API     api.Interface

	WarnedEmptyCorpus       bool
	WarnedOverlappingPrefix bool
}

// A FilepathResolution is the result of mapping a vfs path into a Kythe uri
// component or a proper KytheUri. The mapping happens in the context of a given
// corpus+root.
//
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

// hasDirComponent returns true if 'rs' contains a 'NextDirComponent' resolution.
func hasDirComponent(rs []FilepathResolution) bool {
	for _, r := range rs {
		if r.NextDirComponent != "" {
			return true
		}
	}
	return false
}

// ResolveFilepath returns alternative resolutions of a given vfs path.
// The multiple resolutions are due to ambiguity, which occurs due to:
//
//	a) The queried path pointing into a prefix of some corpus+root vfs path.
//
//	b) Overlapping corpus+root+path vfs paths. If happens, you likely need to
//	   adjust the extractor's vname mapping config.
func (me *kytheFS) ResolveFilepath(path string) ([]FilepathResolution, error) {
	var req ftpb.CorpusRootsRequest
	cr, err := me.API.CorpusRoots(me.Context, &req)
	if err != nil {
		return nil, err
	}

	// Set to dedup next-dirs, as they can be common against multiple corpus+root
	// pairs.
	nextDirs := make(map[string]bool)

	var ticketResolution *FilepathResolution

	sep := string(os.PathSeparator)

	if path == "" {
		// List top-level vfs dirs, which are the first components of corpus names.
		for _, corpus := range me.NonEmptyCorpora(cr.Corpus) {
			parts := strings.SplitN(corpus.Name, sep, 2)
			nextDirs[parts[0]] = true
		}
	} else {
		// Given a vfs directory path, collect the listings visible in that
		// directory. These are the following directory components and maybe a
		// resolved/ Kythe URI of a matching file.

		// If a path could resolve to multiple URIs, due to an overlap in
		// corpus+root+path_prefix, we arbitrary prefer the one matching the one
		// with the longer corpus+root.
		//
		// We could alternatively check existence of the path with API calls, but
		// this situation shouldn't normally arise unless extraction config is broken.
		var longestCorpusRootForTicket string

		for _, corpus := range me.NonEmptyCorpora(cr.Corpus) {
			for _, root := range corpus.Root {
				crPath := filepath.Join(corpus.Name, root)
				crRemain := strings.TrimPrefix(crPath, path)
				vfsRemain := strings.TrimPrefix(path, crPath)
				if len(vfsRemain) < len(path) &&
					(vfsRemain == "" || vfsRemain[:1] == sep) &&
					len(crPath) > len(longestCorpusRootForTicket) {

					if longestCorpusRootForTicket != "" && !me.WarnedOverlappingPrefix {
						log.Warningf("There is at least one overlap in corpus+root+path_prefix. "+
							"Path: %q (corpus+root %q), corpus+root of conflicting path: %q ",
							path, crPath, longestCorpusRootForTicket)
						me.WarnedOverlappingPrefix = true
					}

					longestCorpusRootForTicket = crPath
					// Points inside corpus+root, match using the ticket.
					var p string // Exact corpus+root match.
					if vfsRemain != "" {
						p = vfsRemain[1:] // Additional kythe path.
					}
					ticketResolution = &FilepathResolution{
						KytheURI: &kytheuri.URI{
							Corpus: corpus.Name,
							Root:   root,
							Path:   p,
						},
					}
				} else if len(crRemain) < len(crPath) && crRemain[:1] == sep {
					// Queried path is a proper prefix.
					parts := strings.SplitN(crRemain[1:], sep, 2)
					nextDirs[parts[0]] = true
				}
			}
		}
	}

	var results []FilepathResolution
	for k := range nextDirs {
		res := FilepathResolution{NextDirComponent: k}
		results = append(results, res)
	}
	if ticketResolution != nil {
		results = append(results, *ticketResolution)
	}
	return results, nil
}

// NonEmptyCorpora returns the non-empty named corpuses, and warns the first time
// an empty corpus name is encountered.
//
// Empty corpus names are not worth the trouble for special handling, given
// that naming corpora comes without drawbacks and is a good practice.
func (me *kytheFS) NonEmptyCorpora(cs []*ftpb.CorpusRootsReply_Corpus) []*ftpb.CorpusRootsReply_Corpus {
	var res []*ftpb.CorpusRootsReply_Corpus
	for _, c := range cs {
		if c.Name != "" {
			res = append(res, c)
		} else if !me.WarnedEmptyCorpus {
			log.Warningf("found empty corpus name, skipping mapping! " +
				"Please set a corpus when extracting or indexing.")
			me.WarnedEmptyCorpus = true
		}
	}
	return res
}

// IsDirectory returns true if the given Kythe URI corresponds to a directory.
//
// Actually it checks that the path is not a known file. Could also use the
// filetree api to check contents of the parent. But I expect this code to
// change when caching is added, then we will determine directory-ness from
// a local cache (and it will be a separate concern how we fill that cache).
func (me *kytheFS) IsDirectory(uri *kytheuri.URI) (bool, error) {
	ticket := uri.String()
	req := &gpb.NodesRequest{
		Ticket: []string{ticket},
		// Minimize amount of data returned, ask for NodeKind only.
		Filter: []string{facts.NodeKind},
	}
	res, err := me.API.Nodes(me.Context, req)
	if err != nil {
		return false, nil
	}
	for k, n := range res.Nodes {
		if k != ticket {
			continue
		}
		if len(n.Facts) > 0 {
			// Directory entries don't have any facts.
			return false, nil
		}
	}
	return true, nil
}

func (me *kytheFS) fetchSourceForURI(uri *kytheuri.URI) ([]byte, error) {
	ticket := uri.String()
	dec, err := me.API.Decorations(me.Context, &xpb.DecorationsRequest{
		Location:   &xpb.Location{Ticket: ticket},
		SourceText: true,
	})
	if err != nil {
		return nil, err
	}
	return dec.SourceText, nil
}

func (me *kytheFS) fetchSource(path string) ([]byte, error) {
	resolutions, err := me.ResolveFilepath(path)
	if err != nil {
		return nil, err
	}

	for _, r := range resolutions {
		if r.KytheURI == nil {
			continue
		}
		src, err := me.fetchSourceForURI(r.KytheURI)
		if err != nil {
			return nil, fmt.Errorf(
				"no xrefs for %q (resolved to ticket %q): %v",
				path, r.KytheURI.String(), err)
		}

		return src, nil
	}

	return nil, fmt.Errorf("couldn't resolve path %q to a ticket", path)
}

// GetAttr implements a go-fuse stub.
func (me *kytheFS) GetAttr(path string, context *fuse.Context) (*fuse.Attr, fuse.Status) {
	resolutions, err := me.ResolveFilepath(path)
	if err != nil {
		log.Errorf("resolution error for %q: %v", path, err)
		return nil, fuse.ENOENT
	}

	if hasDirComponent(resolutions) {
		return &fuse.Attr{
			Mode: fuse.S_IFDIR | 0755,
		}, fuse.OK
	}

	for _, r := range resolutions {
		if r.KytheURI == nil {
			continue
		}

		isDir, err := me.IsDirectory(r.KytheURI)
		if err != nil {
			return nil, fuse.ENOENT
		}

		if isDir {
			return &fuse.Attr{
				Mode: fuse.S_IFDIR | 0755,
			}, fuse.OK
		}

		src, err := me.fetchSourceForURI(r.KytheURI)
		if err != nil {
			return nil, fuse.ENOENT
		}
		return &fuse.Attr{
			Mode: fuse.S_IFREG | 0644, Size: uint64(len(src)),
		}, fuse.OK
	}
	return nil, fuse.ENOENT
}

// OpenDir implements a go-fuse stub.
func (me *kytheFS) OpenDir(path string, context *fuse.Context) (c []fuse.DirEntry, code fuse.Status) {
	resolutions, err := me.ResolveFilepath(path)
	if err != nil {
		log.Errorf("resolution error for %q: %v", path, err)
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
			dir, err := me.API.Directory(me.Context, req)
			if err != nil {
				log.Errorf("error fetching dir contents for %q (ticket %q): %v",
					path, r.KytheURI.String(), err)
				return nil, fuse.ENOENT
			}

			for _, e := range dir.Entry {
				de := fuse.DirEntry{Name: e.Name}
				switch e.Kind {
				case ftpb.DirectoryReply_FILE:
					de.Mode = fuse.S_IFREG
				case ftpb.DirectoryReply_DIRECTORY:
					de.Mode = fuse.S_IFDIR
				default:
					log.Warningf("received invalid directory entry: %v", e)
					continue
				}
				ents[e.Name] = de
			}
		} else {
			log.Fatalf(
				"Programming error: resoultion is neither dir part nor uri for %q",
				path)
		}
	}
	var result []fuse.DirEntry
	for _, v := range ents {
		result = append(result, v)
	}
	return result, fuse.OK
}

// Open implements a go-fuse stub.
func (me *kytheFS) Open(path string, flags uint32, context *fuse.Context) (file nodefs.File, code fuse.Status) {
	// Read-only filesystem.
	if flags&fuse.O_ANYWRITE != 0 {
		return nil, fuse.EPERM
	}

	src, err := me.fetchSource(path)
	if err != nil {
		log.Errorf("error fetching source for %q: %v", path, err)
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

	kytheAPI, err := api.ParseSpec(*serverAddr)
	if err != nil {
		log.Fatal("Failed to parse server address!", *serverAddr)
	}

	ctx := context.Background()
	defer kytheAPI.Close(ctx)

	nfs := pathfs.NewPathNodeFs(&kytheFS{
		FileSystem: pathfs.NewDefaultFileSystem(),
		Context:    ctx,
		API:        kytheAPI,
	}, nil)

	server, _, err := nodefs.MountRoot(*mountPoint, nfs.Root(), nil)
	if err != nil {
		log.Fatalf("Mounting failed: %v", err)
	}

	server.Serve()
}
