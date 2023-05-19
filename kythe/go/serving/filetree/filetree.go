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

// Package filetree implements a lookup table for files in a tree structure.
//
// Table format:
//
//	dirs:<corpus>\n<root>\n<path> -> srvpb.FileDirectory
//	dirs:corpusRoots              -> srvpb.CorpusRoots
package filetree // import "kythe.io/kythe/go/serving/filetree"

import (
	"context"
	"errors"
	"fmt"
	"path/filepath"
	"strings"

	"kythe.io/kythe/go/storage/table"
	"kythe.io/kythe/go/util/kytheuri"
	"kythe.io/kythe/go/util/log"

	ftpb "kythe.io/kythe/proto/filetree_go_proto"
	srvpb "kythe.io/kythe/proto/serving_go_proto"
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
	entries := make([]*ftpb.DirectoryReply_Entry, 0, len(d.Entry))
	for _, e := range d.Entry {
		if !req.GetIncludeFilesMissingText() && e.GetMissingText() {
			continue
		}
		re := &ftpb.DirectoryReply_Entry{
			Name:        e.Name,
			BuildConfig: e.BuildConfig,
			Generated:   e.GetGenerated(),
			MissingText: e.GetMissingText(),
		}
		switch e.Kind {
		case srvpb.FileDirectory_FILE:
			re.Kind = ftpb.DirectoryReply_FILE
		case srvpb.FileDirectory_DIRECTORY:
			re.Kind = ftpb.DirectoryReply_DIRECTORY
		default:
			log.Warningf("unknown directory entry type: %T", e)
			continue
		}
		entries = append(entries, re)
	}
	entries, err := parseLegacyEntries(entries, ftpb.DirectoryReply_FILE, d.FileTicket)
	if err != nil {
		return nil, err
	}
	entries, err = parseLegacyEntries(entries, ftpb.DirectoryReply_DIRECTORY, d.Subdirectory)
	if err != nil {
		return nil, err
	}
	return &ftpb.DirectoryReply{
		Corpus: req.Corpus,
		Root:   req.Root,
		Path:   req.Path,
		Entry:  entries,
	}, nil
}

func parseLegacyEntries(entries []*ftpb.DirectoryReply_Entry, kind ftpb.DirectoryReply_Kind, tickets []string) ([]*ftpb.DirectoryReply_Entry, error) {
	for _, ticket := range tickets {
		uri, err := kytheuri.Parse(ticket)
		if err != nil {
			return nil, fmt.Errorf("invalid serving data: %v", err)
		}
		entries = append(entries, &ftpb.DirectoryReply_Entry{
			Kind:      kind,
			Name:      filepath.Base(uri.Path),
			Generated: uri.Root != "",
		})
	}
	return entries, nil
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
		Corpus: make([]*ftpb.CorpusRootsReply_Corpus, len(cr.Corpus)),
	}

	for i, corpus := range cr.Corpus {
		reply.Corpus[i] = &ftpb.CorpusRootsReply_Corpus{
			Name:        corpus.Corpus,
			Root:        corpus.Root,
			BuildConfig: corpus.BuildConfig,
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
