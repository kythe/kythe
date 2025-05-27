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

// Package locked implements a delegating wrapper that protects each method of
// the resulting kcd.Reader or kcd.ReadWriter with a mutex, so that the result
// is safe for concurrent use by multiple goroutines.
package locked // import "kythe.io/kythe/go/platform/kcd/locked"

import (
	"context"
	"errors"
	"io"
	"sync"

	"kythe.io/kythe/go/platform/kcd"
)

type locker struct {
	μ   sync.Mutex
	rd  kcd.Reader
	wr  kcd.Writer
	del kcd.Deleter
}

// Reader returns a kcd.Reader that delegates to r with method calls protected
// by a mutex.
func Reader(r kcd.Reader) kcd.Reader { return &locker{rd: r} }

// ReadWriter returns a kcd.ReadWriter that delegates to rw with method calls
// protected by a mutex.
func ReadWriter(rw kcd.ReadWriter) kcd.ReadWriter { return &locker{rd: rw, wr: rw} }

// ReadWriteDeleter returns a kcd.ReadWriteDeleter that delegates to rwd with
// method calls protected by a mutex.
func ReadWriteDeleter(rwd kcd.ReadWriteDeleter) kcd.ReadWriteDeleter {
	return &locker{rd: rwd, wr: rwd, del: rwd}
}

// ErrNotSupported is returned by the write methods if no Writer is available.
var ErrNotSupported = errors.New("write operation not supported")

// Revisions implements a method of kcd.Reader.
func (db *locker) Revisions(ctx context.Context, want *kcd.RevisionsFilter, f func(kcd.Revision) error) error {
	db.μ.Lock()
	defer db.μ.Unlock()
	return db.rd.Revisions(ctx, want, f)
}

// Find implements a method of kcd.Reader.
func (db *locker) Find(ctx context.Context, filter *kcd.FindFilter, f func(string) error) error {
	db.μ.Lock()
	defer db.μ.Unlock()
	return db.rd.Find(ctx, filter, f)
}

// Units implements a method of kcd.Reader.
func (db *locker) Units(ctx context.Context, unitDigests []string, f func(digest, key string, data []byte) error) error {
	db.μ.Lock()
	defer db.μ.Unlock()
	return db.rd.Units(ctx, unitDigests, f)
}

// Files implements a method of kcd.Reader.
func (db *locker) Files(ctx context.Context, fileDigests []string, f func(string, []byte) error) error {
	db.μ.Lock()
	defer db.μ.Unlock()
	return db.rd.Files(ctx, fileDigests, f)
}

// FilesExist implements a method of kcd.Reader.
func (db *locker) FilesExist(ctx context.Context, fileDigests []string, f func(string) error) error {
	db.μ.Lock()
	defer db.μ.Unlock()
	return db.rd.FilesExist(ctx, fileDigests, f)
}

// FetchCUForAnalysis implements a method of kcd.Reader.
func (db *locker) FindCUMetadatas(ctx context.Context, filter *kcd.FetchCUSelectorFilter, f func(cuMetaData *kcd.CUMetaData) error) error {
	db.μ.Lock()
	defer db.μ.Unlock()
	return db.rd.FindCUMetadatas(ctx, filter, f)
}

// WriteRevision implements a method of kcd.Writer.
func (db *locker) WriteRevision(ctx context.Context, rev kcd.Revision, replace bool) error {
	if db.wr == nil {
		return ErrNotSupported
	}
	db.μ.Lock()
	defer db.μ.Unlock()
	return db.wr.WriteRevision(ctx, rev, replace)
}

// WriteUnit implements a method of kcd.Writer.
func (db *locker) WriteUnit(ctx context.Context, rev kcd.Revision, formatKey string, unit kcd.Unit) (string, error) {
	if db.wr == nil {
		return "", ErrNotSupported
	}
	db.μ.Lock()
	defer db.μ.Unlock()
	return db.wr.WriteUnit(ctx, rev, formatKey, unit)
}

// WriteFile implements a method of kcd.Writer.
func (db *locker) WriteFile(ctx context.Context, r io.Reader) (string, error) {
	if db.wr == nil {
		return "", ErrNotSupported
	}
	db.μ.Lock()
	defer db.μ.Unlock()
	return db.wr.WriteFile(ctx, r)
}

// DeleteUnit implements a method of kcd.Deleter.
func (db *locker) DeleteUnit(ctx context.Context, unitDigest string) error {
	if db.del == nil {
		return ErrNotSupported
	}
	db.μ.Lock()
	defer db.μ.Unlock()
	return db.del.DeleteUnit(ctx, unitDigest)
}

// DeleteFile implements a method of kcd.Deleter.
func (db *locker) DeleteFile(ctx context.Context, fileDigest string) error {
	if db.del == nil {
		return ErrNotSupported
	}
	db.μ.Lock()
	defer db.μ.Unlock()
	return db.del.DeleteFile(ctx, fileDigest)
}

// DeleteRevision implements a method of kcd.Deleter.
func (db *locker) DeleteRevision(ctx context.Context, revision, corpus string) error {
	if db.del == nil {
		return ErrNotSupported
	}
	db.μ.Lock()
	defer db.μ.Unlock()
	return db.del.DeleteRevision(ctx, revision, corpus)
}
