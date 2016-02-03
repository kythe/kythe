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

// Package filetree implements a lookup table for files in a tree structure.
//
// Table format:
//   dirs:<corpus>\n<root>\n<path> -> srvpb.FileDirectory
//   dirs:corpusRoots              -> srvpb.CorpusRoots
package filetree

import (
	"errors"
	"fmt"
	"strings"

	"kythe.io/kythe/go/storage/table"

	"golang.org/x/net/context"

	ftpb "kythe.io/kythe/proto/filetree_proto"
	srvpb "kythe.io/kythe/proto/serving_proto"
)

const (
	// DirTablePrefix is used as the prefix of the keys of a combined serving
	// table.  CorpusRootsPrefixedKey and PrefixedDirKey use this prefix to
	// construct their keys.  Table uses this prefix when PrefixedKeys is true.
	DirTablePrefix = "dirs:"

	dirKeySep = "\n"
)

// CorpusRootsKey is the filetree lookup key for the tree's srvpb.CorpusRoots.
var CorpusRootsKey = []byte("corpusRoots")

// CorpusRootsPrefixedKey is the filetree lookup key for the tree's
// srvpb.CorpusRoots when using PrefixedKeys.
var CorpusRootsPrefixedKey = []byte(DirTablePrefix + "corpusRoots")

// Table implements the FileTree interface using a static lookup table.
type Table struct {
	table.Proto

	// PrefixedKeys indicates whether all keys are prefixed by DirTablePrefix
	// (i.e. when using a combined serving table).
	PrefixedKeys bool
}

// Directory implements part of the filetree Service interface.
func (t *Table) Directory(ctx context.Context, req *ftpb.DirectoryRequest) (*ftpb.DirectoryReply, error) {
	var key []byte
	if t.PrefixedKeys {
		key = PrefixedDirKey(req.Corpus, req.Root, req.Path)
	} else {
		key = DirKey(req.Corpus, req.Root, req.Path)
	}
	var d srvpb.FileDirectory
	if err := t.Lookup(ctx, key, &d); err == table.ErrNoSuchKey {
		return &ftpb.DirectoryReply{}, nil
	} else if err != nil {
		return nil, fmt.Errorf("lookup error: %v", err)
	}
	return &ftpb.DirectoryReply{
		Subdirectory: d.Subdirectory,
		File:         d.FileTicket,
	}, nil
}

// CorpusRoots implements part of the filetree Service interface.
func (t *Table) CorpusRoots(ctx context.Context, req *ftpb.CorpusRootsRequest) (*ftpb.CorpusRootsReply, error) {
	key := CorpusRootsKey
	if t.PrefixedKeys {
		key = CorpusRootsPrefixedKey
	}
	var cr srvpb.CorpusRoots
	if err := t.Lookup(ctx, key, &cr); err == table.ErrNoSuchKey {
		return nil, errors.New("internal error: missing corpusRoots in table")
	} else if err != nil {
		return nil, fmt.Errorf("corpusRoots lookup error: %v", err)
	}

	reply := &ftpb.CorpusRootsReply{
		Corpus: make([]*ftpb.CorpusRootsReply_Corpus, len(cr.Corpus), len(cr.Corpus)),
	}

	for i, corpus := range cr.Corpus {
		reply.Corpus[i] = &ftpb.CorpusRootsReply_Corpus{
			Name: corpus.Corpus,
			Root: corpus.Root,
		}
	}

	return reply, nil
}

// DirKey returns the filetree lookup table key for the given corpus path.
func DirKey(corpus, root, path string) []byte {
	return []byte(strings.Join([]string{corpus, root, path}, dirKeySep))
}

// PrefixedDirKey returns the filetree lookup table key for the given corpus
// path, prefixed by DirTablePrefix.
func PrefixedDirKey(corpus, root, path string) []byte {
	return []byte(DirTablePrefix + strings.Join([]string{corpus, root, path}, dirKeySep))
}
