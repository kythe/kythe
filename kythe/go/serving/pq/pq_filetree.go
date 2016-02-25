/*
 * Copyright 2016 Google Inc. All rights reserved.
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

package pq

import (
	"fmt"
	"path/filepath"

	"golang.org/x/net/context"

	ftpb "kythe.io/kythe/proto/filetree_proto"
)

// Directory implements part of the filetree.Interface.
func (d *DB) Directory(ctx context.Context, req *ftpb.DirectoryRequest) (*ftpb.DirectoryReply, error) {
	if d.selectDirectory == nil {
		var err error
		d.selectDirectory, err = d.Prepare("SELECT ticket, file FROM Files WHERE corpus = $1 AND root = $2 AND path = $3;")
		if err != nil {
			return nil, fmt.Errorf("error preparing statement: %v", err)
		}
	}

	path := filepath.Join("/", req.Path)
	if path != "/" {
		path += "/"
	}

	rs, err := d.selectDirectory.Query(req.Corpus, req.Root, path)
	if err != nil {
		return nil, fmt.Errorf("query error: %v", err)
	}

	reply := &ftpb.DirectoryReply{}
	for rs.Next() {
		var ticket string
		var file bool
		if err := rs.Scan(&ticket, &file); err != nil {
			return nil, fmt.Errorf("scan error: %v", err)
		}

		if file {
			reply.File = append(reply.File, ticket)
		} else {
			reply.Subdirectory = append(reply.Subdirectory, ticket)
		}
	}

	return reply, nil
}

// CorpusRoots implements part of the filetree.Interface.
func (d *DB) CorpusRoots(ctx context.Context, req *ftpb.CorpusRootsRequest) (*ftpb.CorpusRootsReply, error) {
	if d.selectCorpusRoots == nil {
		var err error
		d.selectCorpusRoots, err = d.Prepare("SELECT DISTINCT corpus, root FROM Files;")
		if err != nil {
			return nil, fmt.Errorf("error preparing statement: %v", err)
		}
	}

	rs, err := d.selectCorpusRoots.Query()
	if err != nil {
		return nil, fmt.Errorf("error select corpus roots: %v", err)
	}

	crs := make(map[string][]string)
	for rs.Next() {
		var corpus, root string
		if err := rs.Scan(&corpus, &root); err != nil {
			return nil, fmt.Errorf("scan error: %v", err)
		}

		crs[corpus] = append(crs[corpus], root)
	}

	reply := &ftpb.CorpusRootsReply{}
	for corpus, roots := range crs {
		reply.Corpus = append(reply.Corpus, &ftpb.CorpusRootsReply_Corpus{
			Name: corpus,
			Root: roots,
		})
	}

	return reply, nil
}
