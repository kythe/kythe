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

	"kythe/go/storage/table"

	srvpb "kythe/proto/serving_proto"
)

const (
	dirTablePrefix = "dirs:"
	dirKeySep      = "\n"
)

// CorpusRootsKey is the filetree lookup key for the tree's srvpb.CorpusRoots.
var CorpusRootsKey = []byte(dirTablePrefix + "corpusRoots")

// Table implements the FileTree interface using a static lookup table.
type Table struct{ table.Proto }

// Dir implements part of the filetree Service interface.
func (t *Table) Dir(corpus, root, path string) (*srvpb.FileDirectory, error) {
	var d srvpb.FileDirectory
	if err := t.Lookup(DirKey(corpus, root, path), &d); err == table.ErrNoSuchKey {
		return nil, nil
	} else if err != nil {
		return nil, fmt.Errorf("lookup error: %v", err)
	}
	return &d, nil
}

// CorporaRoots implements part of the filetree Service interface.
func (t *Table) CorporaRoots() (*srvpb.CorpusRoots, error) {
	var cr srvpb.CorpusRoots
	if err := t.Lookup(CorpusRootsKey, &cr); err == table.ErrNoSuchKey {
		return nil, errors.New("missing corpusRoots in table")
	} else if err != nil {
		return nil, fmt.Errorf("corpusRoots lookup error: %v", err)
	}
	return &cr, nil
}

// DirKey returns the filetree lookup table key for the given corpus path.
func DirKey(corpus, root, path string) []byte {
	return []byte(dirTablePrefix + strings.Join([]string{corpus, root, path}, dirKeySep))
}
