/*
 * Copyright 2018 The Kythe Authors. All rights reserved.
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

// Package kzipdb implements kcd.Reader using a kzip file as its backing
// store. See also: http://www.kythe.io/docs/kythe-kzip.html.
package kzipdb // import "kythe.io/kythe/go/platform/kcd/kzipdb"

import (
	"context"

	"google.golang.org/protobuf/proto"
	"kythe.io/kythe/go/platform/kcd"
	"kythe.io/kythe/go/platform/kcd/kythe"
	"kythe.io/kythe/go/platform/kzip"
	"google3/util/task/go/status"
)

// DB implements kcd.Reader using a kzip.Reader as its backing store.
type DB struct {
	Reader *kzip.Reader
}

// Revisions implements a method of kcd.Reader.
func (db DB) Revisions(ctx context.Context, want *kcd.RevisionsFilter, f func(kcd.Revision) error) error {
	revisionMatches, err := want.Compile()
	if err != nil {
		return err
	}
	seen := make(map[kcd.Revision]bool)
	return db.Reader.Scan(func(unit *kzip.Unit) error {
		corpus := unit.Proto.GetVName().GetCorpus()
		for _, revision := range unit.Index.GetRevisions() {
			rev := kcd.Revision{Revision: revision, Corpus: corpus}
			if !seen[rev] && revisionMatches(rev) {
				if err := f(rev); err != nil {
					return err
				}
				seen[rev] = true
			}
		}
		return nil
	})
}

// Find implements a method of kcd.Reader.
func (db DB) Find(ctx context.Context, filter *kcd.FindFilter, f func(string) error) error {
	cf, err := filter.Compile()
	if err != nil {
		return err
	} else if cf == nil {
		return nil
	}

	return db.Reader.Scan(func(unit *kzip.Unit) error {
		idx := kythe.Unit{unit.Proto}.Index()

		if cf.RevisionMatches(unit.Index.GetRevisions()...) &&
			cf.LanguageMatches(idx.Language) &&
			cf.TargetMatches(idx.Target) &&
			cf.OutputMatches(idx.Output) &&
			cf.SourcesMatch(idx.Sources...) {
			return f(unit.Digest)
		}
		return nil
	})
}

// Units implements a method of kcd.Reader.
func (db DB) Units(ctx context.Context, unitDigests []string, f func(digest, key string, data []byte) error) error {
	for _, digest := range unitDigests {
		unit, err := db.Reader.Lookup(digest)
		if err != nil {
			return err
		}
		data, err := proto.Marshal(unit.Proto)
		if err != nil {
			return err
		}
		if err := f(unit.Digest, kythe.Format, data); err != nil {
			return err
		}
	}
	return nil
}

// Files implements a method of kcd.Reader.
func (db DB) Files(ctx context.Context, fileDigests []string, f func(string, []byte) error) error {
	for _, digest := range fileDigests {
		data, err := db.Reader.ReadAll(digest)
		if err == kzip.ErrDigestNotFound {
			continue
		} else if err != nil {
			return err
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
		rc, err := db.Reader.Open(digest)
		if err == kzip.ErrDigestNotFound {
			continue
		} else if err != nil {
			return err
		}
		rc.Close()
		return f(digest)
	}
	return nil
}

// FetchCUSelector implements a method of kcd.Reader. This method is not supported for kzipdb.
func (db DB) FetchCUSelector(ctx context.Context, filter *kcd.FetchCUSelectorFilter, f func(digest string, target string) error) error {
	return status.ErrUnimplemented
}
