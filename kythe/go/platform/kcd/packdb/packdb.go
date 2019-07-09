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

// Package packdb implements kcd.ReadWriter using an index pack as its backing
// store. See also: http://www.kythe.io/docs/kythe-index-pack.html.
package packdb // import "kythe.io/kythe/go/platform/kcd/packdb"

import (
	"context"
	"errors"
	"fmt"
	"io"
	"io/ioutil"
	"log"
	"os"
	"path/filepath"
	"strings"
	"time"

	"kythe.io/kythe/go/platform/indexpack"
	"kythe.io/kythe/go/platform/kcd"

	"github.com/golang/protobuf/proto"
)

// DB implements kcd.ReadWriter using an index pack as its backing store.
//
// An index pack does not record revision and corpus information, so this
// implementation emulates that with ephemeral data in-memory.  Revision
// markers written will be discarded when the DB is released.
type DB struct {
	Archive   *indexpack.Archive
	FormatKey string                               // default format key
	Revs      []kcd.Revision                       // revision index
	Convert   func(v interface{}) (kcd.Unit, bool) // unit format converter
}

// Revisions implements a method of kcd.Reader.
func (db DB) Revisions(ctx context.Context, want *kcd.RevisionsFilter, f func(kcd.Revision) error) error {
	revisionMatches, err := want.Compile()
	if err != nil {
		return err
	}
	for _, rev := range db.Revs {
		if revisionMatches(rev) {
			if err := f(rev); err != nil {
				return err
			}
		}
	}
	return nil
}

// Find implements a method of kcd.Reader.
func (db DB) Find(ctx context.Context, filter *kcd.FindFilter, f func(string) error) error {
	cf, err := filter.Compile()
	if err != nil {
		return err
	} else if cf == nil {
		return nil
	}

	// Since index packs don't persist index data, we'll assume any revision
	// and corpus match.
	return db.Archive.ReadUnits(ctx, db.FormatKey, func(digest string, v interface{}) error {
		unit, ok := db.Convert(v)
		if !ok {
			log.Printf("WARNING: Unknown compilation unit type %T (%s)", v, digest)
			return nil // nothing to do here
		}
		idx := unit.Index()
		if cf.LanguageMatches(idx.Language) &&
			cf.TargetMatches(idx.Target) &&
			cf.OutputMatches(idx.Output) &&
			cf.SourcesMatch(idx.Sources...) {

			return f(digest)
		}
		return nil
	})
}

// Units implements a method of kcd.Reader.
func (db DB) Units(ctx context.Context, unitDigests []string, f func(digest, key string, data []byte) error) error {
	for _, digest := range unitDigests {
		if err := db.Archive.ReadUnit(ctx, db.FormatKey, digest, func(v interface{}) error {
			switch t := v.(type) {
			case proto.Message:
				data, err := proto.Marshal(v.(proto.Message))
				if err != nil {
					return fmt.Errorf("unmarshaling unit: %v", err)
				}
				return f(digest, db.FormatKey, data)
			case *[]byte:
				return f(digest, db.FormatKey, *t)
			default:
				log.Printf("WARNING: Ignoring peculiar unit of type %T: %v", v, v)
				return nil
			}
		}); err != nil && !os.IsNotExist(err) {
			return err
		}
	}
	return nil
}

// Files implements a method of kcd.Reader.
func (db DB) Files(ctx context.Context, fileDigests []string, f func(string, []byte) error) error {
	for _, digest := range fileDigests {
		data, err := db.Archive.ReadFile(ctx, digest)
		if os.IsNotExist(err) {
			continue
		} else if err != nil {
			return fmt.Errorf("reading file: %v", err)
		}
		if err := f(digest, data); err != nil {
			return err
		}
	}
	return nil
}

// FilesExist implements a method of kcd.Reader.
func (db DB) FilesExist(ctx context.Context, fileDigests []string, f func(string) error) error {
	for _, digest := range fileDigests {
		ok, err := db.Archive.FileExists(ctx, digest)
		if err != nil {
			return fmt.Errorf("locating file: %v", err)
		} else if !ok {
			continue
		}
		if err := f(digest); err != nil {
			return err
		}
	}
	return nil
}

// WriteRevision implements a method of kcd.Writer.  This implementation stores
// revisions only in memory, they are not persisted in the indexpack.
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
		for i, old := range db.Revs {
			if old.Corpus == rev.Corpus && old.Revision == rev.Revision {
				db.Revs[i].Timestamp = rev.Timestamp
				return nil
			}
		}
	}
	db.Revs = append(db.Revs, rev)
	return nil
}

// WriteUnit implements a method of kcd.Writer.
func (db *DB) WriteUnit(ctx context.Context, revision, corpus, formatKey string, unit kcd.Unit) (string, error) {
	if revision == "" {
		return "", errors.New("empty revision marker")
	}
	unit.Canonicalize()
	name, err := db.Archive.WriteUnit(ctx, formatKey, unit)
	if err != nil {
		return "", err
	}
	return trimExt(name), nil
}

// WriteFile implements a method of kcd.Writer.
func (db *DB) WriteFile(ctx context.Context, r io.Reader) (string, error) {
	data, err := ioutil.ReadAll(r)
	if err != nil {
		return "", err
	}
	name, err := db.Archive.WriteFile(ctx, data)
	if err != nil {
		return "", err
	}
	return trimExt(name), nil
}

func trimExt(name string) string { return strings.TrimSuffix(name, filepath.Ext(name)) }
