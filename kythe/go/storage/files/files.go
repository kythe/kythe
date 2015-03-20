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

// Package files declares the FileStore interface and implements a single
// in-memory FileStore
package files

import (
	"bytes"
	"errors"
	"fmt"

	apb "kythe.io/kythe/proto/analysis_proto"
)

// FileStore refers to an open Kythe file storage server.
type FileStore interface {
	// FileData returns the data for the file described by the given path and
	// digest. Returns a NotFound error if the file data could not be found.
	FileData(path, digest string) ([]byte, error)
}

// ErrNotFound is an error indicating that a requested file's data could not be
// found in a FileStore.
var ErrNotFound = errors.New("file data not found")

// InMemoryStore is a FileStore that serves files based on an in-memory map from
// digest to file contents.
type InMemoryStore map[string][]byte

// InMemory returns an empty InMemoryStore
func InMemory() InMemoryStore {
	return make(InMemoryStore)
}

// AddData adds each FileData to the InMemoryStore based on their digests.
func (s InMemoryStore) AddData(files ...*apb.FileData) error {
	for _, file := range files {
		digest := file.GetInfo().Digest
		if digest == "" {
			return fmt.Errorf("empty digest for %v", file)
		}
		if data, exists := s[digest]; exists {
			if bytes.Equal(data, file.Content) {
				continue
			}
			return fmt.Errorf("different contents for same digest %q", digest)
		}
		s[digest] = file.Content
	}
	return nil
}

// ClearData removes all file data contained within the InMemoryStore.
func (s InMemoryStore) ClearData() {
	for key := range s {
		delete(s, key)
	}
}

// FileData implements a FileStore for an InMemoryStore.
func (s InMemoryStore) FileData(path, digest string) ([]byte, error) {
	data, exists := s[digest]
	if !exists {
		return nil, ErrNotFound
	}
	return data, nil
}
