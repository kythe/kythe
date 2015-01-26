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

// Package filetree implements an in-memory filetree Map and Directory model
// used to store VName-based file paths.
package filetree

import (
	"fmt"
	"path/filepath"

	"kythe/go/storage"
	"kythe/go/util/schema"

	spb "kythe/proto/storage_proto"

	"code.google.com/p/goprotobuf/proto"
)

// FileTree provides an interface to explore a tree of VName files.
type FileTree interface {
	// Dir returns the Directory for the given corpus/root/path. nil is returned
	// if one is not found.
	Dir(corpus, root, path string) (*Directory, error)

	// CorporaRoots returns a map from corpus to known roots.
	CorporaRoots() (map[string][]string, error)
}

// Map is a FileTree backed by an in-memory map.
type Map struct {
	// corpus -> root -> dirPath -> Directory
	M map[string]map[string]map[string]*Directory
}

// A Directory is a named container for other Directories and VName-based files.
type Directory struct {
	// Set of sub-directories basenames
	Dirs []string `json:"dirs"`

	// Map of files within the Directory key'd by their basename.  Note: the value
	// is a slice because there may be different versions of file's signature
	// (its digest)
	Files map[string][]*spb.VName `json:"files"`
}

// NewMap returns an empty filetree Map
func NewMap() *Map {
	return &Map{make(map[string]map[string]map[string]*Directory)}
}

// Populate adds each file node in the GraphStore to the FileTree
func (m *Map) Populate(gs storage.GraphStore) error {
	if err := storage.EachScanEntry(gs, &spb.ScanRequest{
		FactPrefix: proto.String(schema.NodeKindFact),
	}, func(entry *spb.Entry) error {
		if entry.GetFactName() == schema.NodeKindFact && string(entry.GetFactValue()) == schema.FileKind {
			m.AddFile(entry.Source)
		}
		return nil
	}); err != nil {
		return fmt.Errorf("failed to Scan GraphStore for directory structure: %v", err)
	}
	return nil
}

// AddFile adds the given VName file to the FileTree.
func (m *Map) AddFile(file *spb.VName) {
	path := filepath.Join("/", file.GetPath())
	dir := m.ensureDir(file.GetCorpus(), file.GetRoot(), filepath.Dir(path))
	dir.Files[filepath.Base(path)] = append(dir.Files[filepath.Base(path)], file)
}

// CorporaRoots implements part of the FileTree interface.
func (m *Map) CorporaRoots() (map[string][]string, error) {
	corporaRoots := make(map[string][]string)
	for corpus, rootDirs := range m.M {
		var roots []string
		for root := range rootDirs {
			roots = append(roots, root)
		}
		corporaRoots[corpus] = roots
	}
	return corporaRoots, nil
}

// Dir implements part of the FileTree interface.
func (m *Map) Dir(corpus, root, path string) (*Directory, error) {
	roots := m.M[corpus]
	if roots == nil {
		return nil, nil
	}
	dirs := roots[root]
	if dirs == nil {
		return nil, nil
	}
	return dirs[path], nil
}

func (m *Map) ensureCorpusRoot(corpus, root string) map[string]*Directory {
	roots := m.M[corpus]
	if roots == nil {
		roots = make(map[string]map[string]*Directory)
		m.M[corpus] = roots
	}

	dirs := roots[root]
	if dirs == nil {
		dirs = make(map[string]*Directory)
		roots[root] = dirs
	}
	return dirs
}

func (m *Map) ensureDir(corpus, root, path string) *Directory {
	dirs := m.ensureCorpusRoot(corpus, root)
	dir := dirs[path]
	if dir == nil {
		dir = &Directory{[]string{}, make(map[string][]*spb.VName)}
		dirs[path] = dir

		if path != "/" {
			parent := m.ensureDir(corpus, root, filepath.Dir(path))
			parent.Dirs = addToSet(parent.Dirs, filepath.Base(path))
		}
	}
	return dir
}

func addToSet(strs []string, str string) []string {
	for _, s := range strs {
		if s == str {
			return strs
		}
	}
	return append(strs, str)
}
