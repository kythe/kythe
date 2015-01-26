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

// Package leveldb implements a GraphStore using a LevelDB backend database.
package leveldb

import (
	"bytes"
	"fmt"
	"io"
	"os"

	"kythe/go/storage"
	"kythe/go/storage/keyvalue"

	"github.com/jmhodges/levigo"
)

// levelDB is a wrapper around a levigo.DB that implements keyvalue.DB
type levelDB struct {
	db    *levigo.DB
	cache *levigo.Cache

	// save options to reduce number of allocations during high load
	readOpts      *levigo.ReadOptions
	largeReadOpts *levigo.ReadOptions
	writeOpts     *levigo.WriteOptions
}

// DefaultOptions is the default Options struct passed to Open when not
// otherwise given one.
var DefaultOptions = &Options{
	CacheCapacity:   512 * 1024 * 1024, // 512mb
	WriteBufferSize: 60 * 1024 * 1024,  // 60mb
}

// Options for customizing a LevelDB backend.
type Options struct {
	// CacheCapacity is the caching capacity (in bytes) used for the LevelDB.
	CacheCapacity int

	// CacheLargeReads determines whether to use the cache for large reads. This
	// is usually discouraged but may be useful when the entire LevelDB is known
	// to fit into the cache.
	CacheLargeReads bool

	// WriteBufferSize is the number of bytes the database will build up in memory
	// (backed by a disk log) before writing to the on-disk table.
	WriteBufferSize int
}

// ValidDB determines if the given path could be a LevelDB database.
func ValidDB(path string) bool {
	stat, err := os.Stat(path)
	return os.IsNotExist(err) || (err == nil && stat.IsDir())
}

// OpenGraphStore returns a GraphStore backed by a LevelDB database at the given
// filepath.  If opts==nil, the DefaultOptions are used.
func OpenGraphStore(path string, opts *Options) (storage.GraphStore, error) {
	db, err := Open(path, opts)
	if err != nil {
		return nil, err
	}
	return keyvalue.NewGraphStore(db), nil
}

// Open returns a keyvalue DB backed by a LevelDB database at the given
// filepath.  If opts==nil, the DefaultOptions are used.
func Open(path string, opts *Options) (keyvalue.DB, error) {
	if opts == nil {
		opts = DefaultOptions
	}

	options := levigo.NewOptions()
	defer options.Close()
	cache := levigo.NewLRUCache(opts.CacheCapacity)
	options.SetCache(cache)
	options.SetCreateIfMissing(true)
	if opts.WriteBufferSize > 0 {
		options.SetWriteBufferSize(opts.WriteBufferSize)
	}
	db, err := levigo.Open(path, options)
	if err != nil {
		return nil, fmt.Errorf("could not open LevelDB at %q: %v", path, err)
	}
	largeReadOpts := levigo.NewReadOptions()
	largeReadOpts.SetFillCache(opts.CacheLargeReads)
	return &levelDB{
		db:            db,
		cache:         cache,
		readOpts:      levigo.NewReadOptions(),
		largeReadOpts: largeReadOpts,
		writeOpts:     levigo.NewWriteOptions(),
	}, nil
}

// Close will close the underlying LevelDB database.
func (s *levelDB) Close() error {
	s.db.Close()
	s.cache.Close()
	s.readOpts.Close()
	s.largeReadOpts.Close()
	s.writeOpts.Close()
	return nil
}

// Writer implements part of the keyvalue.DB interface.
func (s *levelDB) Writer() (keyvalue.Writer, error) {
	return &writer{s, levigo.NewWriteBatch()}, nil
}

// Reader implements part of the keyvalue.DB interface.
func (s *levelDB) Reader(prefix []byte, opts *keyvalue.Options) (keyvalue.Iterator, error) {
	var iter *levigo.Iterator
	if opts.IsLargeRead() {
		iter = s.db.NewIterator(s.largeReadOpts)
	} else {
		iter = s.db.NewIterator(s.readOpts)
	}
	iter.Seek(prefix)
	return &iterator{iter, prefix}, nil
}

type writer struct {
	s *levelDB
	*levigo.WriteBatch
}

// Write implements part of the keyvalue.Writer interface.
func (w *writer) Write(key, val []byte) error {
	w.Put(key, val)
	return nil
}

// Close implements part of the keyvalue.Writer interface.
func (w *writer) Close() error {
	if err := w.s.db.Write(w.s.writeOpts, w.WriteBatch); err != nil {
		return err
	}
	w.WriteBatch.Close()
	return nil
}

type iterator struct {
	it     *levigo.Iterator
	prefix []byte
}

// Close implements part of the keyvalue.Iterator interface.
func (i iterator) Close() error {
	i.it.Close()
	return nil
}

// Next implements part of the keyvalue.Iterator interface.
func (i iterator) Next() ([]byte, []byte, error) {
	if !i.it.Valid() {
		if err := i.it.GetError(); err != nil {
			return nil, nil, err
		}
		return nil, nil, io.EOF
	}
	key, val := i.it.Key(), i.it.Value()
	if !bytes.HasPrefix(key, i.prefix) {
		return nil, nil, io.EOF
	}
	i.it.Next()
	return key, val, nil
}
