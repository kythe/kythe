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

// Package leveldb implements a graphstore.Service using a LevelDB backend
// database.
package leveldb

import (
	"bytes"
	"fmt"
	"io"
	"os"

	"kythe.io/kythe/go/services/graphstore"
	"kythe.io/kythe/go/storage/gsutil"
	"kythe.io/kythe/go/storage/keyvalue"

	"github.com/jmhodges/levigo"
)

func init() {
	gsutil.Register("leveldb", func(spec string) (graphstore.Service, error) { return OpenGraphStore(spec, nil) })
	gsutil.RegisterDefault("leveldb")
}

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

	// MustExist ensures that the given database exists before opening it.  If
	// false and the database does not exist, it will be created.
	MustExist bool
}

// ValidDB determines if the given path could be a LevelDB database.
func ValidDB(path string) bool {
	stat, err := os.Stat(path)
	return os.IsNotExist(err) || (err == nil && stat.IsDir())
}

// OpenGraphStore returns a graphstore.Service backed by a LevelDB database at
// the given filepath.  If opts==nil, the DefaultOptions are used.
func OpenGraphStore(path string, opts *Options) (graphstore.Service, error) {
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
	options.SetCreateIfMissing(!opts.MustExist)
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

type snapshot struct {
	db *levigo.DB
	s  *levigo.Snapshot
}

// NewSnapshot implements part of the keyvalue.DB interface.
func (s *levelDB) NewSnapshot() keyvalue.Snapshot {
	return &snapshot{s.db, s.db.NewSnapshot()}
}

// Close implements part of the keyvalue.Snapshot interface.
func (s *snapshot) Close() error {
	s.db.ReleaseSnapshot(s.s)
	return nil
}

// Writer implements part of the keyvalue.DB interface.
func (s *levelDB) Writer() (keyvalue.Writer, error) {
	return &writer{s, levigo.NewWriteBatch()}, nil
}

// Get implements part of the keyvalue.DB interface.
func (s *levelDB) Get(key []byte, opts *keyvalue.Options) ([]byte, error) {
	ro := s.readOptions(opts)
	if ro != s.largeReadOpts && ro != s.readOpts {
		defer ro.Close()
	}
	v, err := s.db.Get(ro, key)
	if err != nil {
		return nil, err
	} else if v == nil {
		return nil, io.EOF
	}
	return v, nil
}

// ScanPrefix implements part of the keyvalue.DB interface.
func (s *levelDB) ScanPrefix(prefix []byte, opts *keyvalue.Options) (keyvalue.Iterator, error) {
	iter, ro := s.iterator(opts)
	if len(prefix) == 0 {
		iter.SeekToFirst()
	} else {
		iter.Seek(prefix)
	}
	return &iterator{iter, ro, prefix, nil}, nil
}

// ScanRange implements part of the keyvalue.DB interface.
func (s *levelDB) ScanRange(r *keyvalue.Range, opts *keyvalue.Options) (keyvalue.Iterator, error) {
	iter, ro := s.iterator(opts)
	iter.Seek(r.Start)
	return &iterator{iter, ro, nil, r}, nil
}

func (s *levelDB) readOptions(opts *keyvalue.Options) *levigo.ReadOptions {
	if snap := opts.GetSnapshot(); snap != nil {
		ro := levigo.NewReadOptions()
		ro.SetSnapshot(snap.(*snapshot).s)
		ro.SetFillCache(!opts.IsLargeRead())
		return ro
	}
	if opts.IsLargeRead() {
		return s.largeReadOpts
	}
	return s.readOpts
}

// iterator creates a new levigo Iterator based on the given options.  It also
// returns any ReadOptions that should be Closed once the Iterator is Closed.
func (s *levelDB) iterator(opts *keyvalue.Options) (*levigo.Iterator, *levigo.ReadOptions) {
	ro := s.readOptions(opts)
	it := s.db.NewIterator(ro)
	if ro == s.largeReadOpts || ro == s.readOpts {
		ro = nil
	}
	return it, ro
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
	it   *levigo.Iterator
	opts *levigo.ReadOptions

	prefix []byte
	r      *keyvalue.Range
}

// Close implements part of the keyvalue.Iterator interface.
func (i iterator) Close() error {
	if i.opts != nil {
		i.opts.Close()
	}
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
	if (i.r == nil && !bytes.HasPrefix(key, i.prefix)) || (i.r != nil && bytes.Compare(key, i.r.End) >= 0) {
		return nil, nil, io.EOF
	}
	i.it.Next()
	return key, val, nil
}
