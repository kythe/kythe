/*
 * Copyright 2016 The Kythe Authors. All rights reserved.
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

// Package memdb implements kcd.ReadWriter with an in-memory representation,
// suitable for testing or ephemeral service-based collections.
package memdb // import "kythe.io/kythe/go/platform/kcd/memdb"

import (
	"context"
	"errors"
	"io"
	"io/ioutil"
	"os"
	"time"

	"kythe.io/kythe/go/platform/kcd"
)

// DB implements kcd.Reader, and *DB implements kcd.ReadWriter and kcd.Deleter.
// Records are stored in exported fields, to assist in testing.  The zero value
// is ready for use as an empty database.
type DB struct {
	Rev   []kcd.Revision
	Unit  map[string]Unit   // :: unit digest → compilation record
	File  map[string]string // :: file digest → file content
	Index map[string]Index  // :: unit digest → index entries
}

// A Unit represents a stored compilation unit.
type Unit struct {
	FormatKey string // may be empty
	Data      []byte // the raw data in wire format
}

// An Index is a mapping from index keys (e.g., "corpus", "source") to distinct
// values for those keys.
type Index map[string][]string

// String tags for index keys matching the fields of a kcd.FindFilter.
const (
	RevisionKey   = "revision"
	CorpusKey     = "corpus"
	UnitCorpusKey = "unit_corpus"
	OutputKey     = "output"
	LanguageKey   = "language"
	TargetKey     = "target"
	SourceKey     = "source"
)

// SetIndex sets an index term for the given digest.
func (db *DB) SetIndex(digest, key, value string) {
	if db.Index == nil {
		db.Index = make(map[string]Index)
	}
	idx := db.Index[digest]
	for _, v := range idx[key] {
		if v == value {
			return
		}
	}
	if idx == nil {
		db.Index[digest] = Index{key: []string{value}}
	} else {
		idx[key] = append(idx[key], value)
	}
}

// Revisions implements a method of kcd.Reader.
func (db DB) Revisions(_ context.Context, want *kcd.RevisionsFilter, f func(kcd.Revision) error) error {
	revisionMatches, err := want.Compile()
	if err != nil {
		return err
	}
	for _, rev := range db.Rev {
		if revisionMatches(rev) {
			if err := f(rev); err != nil {
				return err
			}
		}
	}
	return nil
}

// Find implements a method of kcd.Reader.
func (db DB) Find(_ context.Context, filter *kcd.FindFilter, f func(string) error) error {
	cf, err := filter.Compile()
	if err != nil {
		return err
	} else if cf == nil {
		return nil
	}

	for digest, index := range db.Index {
		if cf.RevisionMatches(index[RevisionKey]...) &&
			cf.BuildCorpusMatches(index[CorpusKey]...) &&
			cf.UnitCorpusMatches(index[UnitCorpusKey]...) &&
			cf.LanguageMatches(index[LanguageKey]...) &&
			cf.TargetMatches(index[TargetKey]...) &&
			cf.OutputMatches(index[OutputKey]...) &&
			cf.SourcesMatch(index[SourceKey]...) {

			if err := f(digest); err != nil {
				return err
			}
		}
	}
	return nil
}

// Units implements a method of kcd.Reader.
func (db DB) Units(_ context.Context, unitDigests []string, f func(digest, key string, data []byte) error) error {
	for _, ud := range unitDigests {
		if unit, ok := db.Unit[ud]; ok {
			if err := f(ud, unit.FormatKey, unit.Data); err != nil {
				return err
			}
		}
	}
	return nil
}

// Files implements a method of kcd.Reader.
func (db DB) Files(_ context.Context, fileDigests []string, f func(string, []byte) error) error {
	for _, fd := range fileDigests {
		if s, ok := db.File[fd]; ok {
			if err := f(fd, []byte(s)); err != nil {
				return err
			}
		}
	}
	return nil
}

// FilesExist implements a method of kcd.Reader.
func (db DB) FilesExist(_ context.Context, fileDigests []string, f func(string) error) error {
	for _, fd := range fileDigests {
		if _, ok := db.File[fd]; ok {
			if err := f(fd); err != nil {
				return err
			}
		}
	}
	return nil
}

// WriteRevision implements a method of kcd.Writer.
func (db *DB) WriteRevision(_ context.Context, rev kcd.Revision, replace bool) error {
	if rev.Revision == "" {
		return errors.New("missing revision marker")
	} else if rev.Corpus == "" {
		return errors.New("missing corpus label")
	}
	if rev.Timestamp.IsZero() {
		rev.Timestamp = time.Now()
	}
	rev.Timestamp = rev.Timestamp.In(time.UTC)
	if replace {
		for i, old := range db.Rev {
			if old.Corpus == rev.Corpus && old.Revision == rev.Revision {
				db.Rev[i].Timestamp = rev.Timestamp
				return nil
			}
		}
	}
	db.Rev = append(db.Rev, rev)
	return nil
}

// WriteUnit implements a method of kcd.Writer.  On success, the returned
// digest is the kcd.HexDigest of whatever unit.MarshalBinary returned.
func (db *DB) WriteUnit(_ context.Context, rev kcd.Revision, formatKey string, unit kcd.Unit) (string, error) {
	revision, corpus := rev.Revision, rev.Corpus
	if revision == "" {
		return "", errors.New("empty revision marker")
	}
	unit.Canonicalize()
	bits, err := unit.MarshalBinary()
	if err != nil {
		return "", err
	}
	digest := unit.Digest()
	if db.Unit == nil {
		db.Unit = make(map[string]Unit)
	}
	db.Unit[digest] = Unit{FormatKey: formatKey, Data: bits}

	// Update the index.
	db.SetIndex(digest, RevisionKey, revision)
	if corpus != "" {
		db.SetIndex(digest, CorpusKey, corpus)
	}
	idx := unit.Index()
	if idx.Corpus != "" {
		db.SetIndex(digest, UnitCorpusKey, idx.Corpus)
	}
	if idx.Language != "" {
		db.SetIndex(digest, LanguageKey, idx.Language)
	}
	if idx.Output != "" {
		db.SetIndex(digest, OutputKey, idx.Output)
	}
	for _, src := range idx.Sources {
		db.SetIndex(digest, SourceKey, src)
	}
	if idx.Target != "" {
		db.SetIndex(digest, TargetKey, idx.Target)
	}
	return digest, nil
}

// WriteFile implements a method of kcd.Writer.
func (db *DB) WriteFile(_ context.Context, r io.Reader) (string, error) {
	bits, err := ioutil.ReadAll(r)
	if err != nil {
		return "", err
	}
	digest := kcd.HexDigest(bits)
	if db.File == nil {
		db.File = make(map[string]string)
	}
	db.File[digest] = string(bits)
	return digest, nil
}

// DeleteUnit implements a method of kcd.Deleter.
func (db *DB) DeleteUnit(_ context.Context, unitDigest string) error {
	if _, ok := db.Unit[unitDigest]; !ok {
		return os.ErrNotExist
	}
	delete(db.Unit, unitDigest)
	delete(db.Index, unitDigest)
	return nil
}

// DeleteFile implements a method of kcd.Deleter.
func (db *DB) DeleteFile(_ context.Context, fileDigest string) error {
	if _, ok := db.File[fileDigest]; !ok {
		return os.ErrNotExist
	}
	delete(db.File, fileDigest)
	return nil
}

// revisionsEqual reports whether r1 and r2 are equal, diregarding timestamp.
func revisionsEqual(r1, r2 kcd.Revision) bool {
	return r1.Revision == r2.Revision && r1.Corpus == r2.Corpus
}

// DeleteRevision implements a method of kcd.Deleter.
func (db *DB) DeleteRevision(_ context.Context, revision, corpus string) error {
	rev := kcd.Revision{Revision: revision, Corpus: corpus}
	if err := rev.IsValid(); err != nil {
		return err
	}

	found := false
	for i := len(db.Rev) - 1; i >= 0; i-- {
		if revisionsEqual(db.Rev[i], rev) {
			found = true
			db.Rev = append(db.Rev[:i], db.Rev[i+1:]...)
		}
	}
	if found {
		return nil
	}
	return os.ErrNotExist
}
