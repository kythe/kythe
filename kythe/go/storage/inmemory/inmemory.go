/*
 * Copyright 2014 The Kythe Authors. All rights reserved.
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

// Package inmemory provides a in-memory implementation of graphstore.Service
// and keyvalue.DB.
package inmemory // import "kythe.io/kythe/go/storage/inmemory"

import (
	"context"
	"fmt"
	"io"
	"sort"
	"strings"
	"sync"

	"kythe.io/kythe/go/services/graphstore"
	"kythe.io/kythe/go/storage/keyvalue"
	"kythe.io/kythe/go/util/compare"

	"google.golang.org/protobuf/proto"

	spb "kythe.io/kythe/proto/storage_go_proto"
)

// GraphStore implements the graphstore.Service interface. A zero of this type
// is ready for use, and is safe for access by concurrent goroutines.
type GraphStore struct {
	mu      sync.RWMutex
	entries []*spb.Entry
}

// Close implements io.Closer. It never returns an error.
func (*GraphStore) Close(ctx context.Context) error { return nil }

// Write implements part of the graphstore.Service interface.
func (s *GraphStore) Write(ctx context.Context, req *spb.WriteRequest) error {
	s.mu.Lock()
	defer s.mu.Unlock()
	for _, u := range req.Update {
		s.insert(proto.Clone(&spb.Entry{
			Source:    req.Source,
			EdgeKind:  u.EdgeKind,
			Target:    u.Target,
			FactName:  u.FactName,
			FactValue: u.FactValue,
		}).(*spb.Entry))
	}
	return nil
}

func (s *GraphStore) insert(e *spb.Entry) {
	i := sort.Search(len(s.entries), func(i int) bool {
		return compare.Entries(e, s.entries[i]) == compare.LT
	})
	if i == len(s.entries) {
		s.entries = append(s.entries, e)
	} else if i < len(s.entries) && compare.EntriesEqual(e, s.entries[i]) {
		s.entries[i] = e
	} else if i == 0 {
		s.entries = append([]*spb.Entry{e}, s.entries...)
	} else {
		s.entries = append(s.entries[:i], append([]*spb.Entry{e}, s.entries[i:]...)...)
	}
}

// Read implements part of the graphstore.Service interface.
func (s *GraphStore) Read(ctx context.Context, req *spb.ReadRequest, f graphstore.EntryFunc) error {
	s.mu.RLock()
	defer s.mu.RUnlock()
	start := sort.Search(len(s.entries), func(i int) bool {
		comp := compare.VNames(s.entries[i].Source, req.Source)
		return comp != compare.LT && (comp == compare.GT || req.EdgeKind == "*" || s.entries[i].EdgeKind >= req.EdgeKind)
	})
	end := sort.Search(len(s.entries), func(i int) bool {
		comp := compare.VNames(s.entries[i].Source, req.Source)
		return comp == compare.GT || (comp != compare.LT && req.EdgeKind != "*" && s.entries[i].EdgeKind > req.EdgeKind)
	})
	for i := start; i < end; i++ {
		if err := f(s.entries[i]); err == io.EOF {
			return nil
		} else if err != nil {
			return err
		}
	}
	return nil
}

// Scan implements part of the graphstore.Service interface.
func (s *GraphStore) Scan(ctx context.Context, req *spb.ScanRequest, f graphstore.EntryFunc) error {
	s.mu.RLock()
	defer s.mu.RUnlock()

	for _, e := range s.entries {
		if !graphstore.EntryMatchesScan(req, e) {
			continue
		} else if err := f(e); err == io.EOF {
			return nil
		} else if err != nil {
			return err
		}
	}
	return nil
}

// NewKeyValueDB returns a keyvalue.DB backed by an in-memory data structure.
func NewKeyValueDB() *KeyValueDB {
	return &KeyValueDB{
		db: make(map[string][]byte),
	}
}

// KeyValueDB implements the keyvalue.DB interface backed by an in-memory map.
type KeyValueDB struct {
	mu   sync.RWMutex
	db   map[string][]byte
	keys []string
}

var _ keyvalue.DB = &KeyValueDB{}

// Get implements part of the keyvalue.DB interface.
func (k *KeyValueDB) Get(ctx context.Context, key []byte, opts *keyvalue.Options) ([]byte, error) {
	k.mu.RLock()
	defer k.mu.RUnlock()
	val, ok := k.db[string(key)]
	if !ok {
		return nil, io.EOF
	}
	return val, nil
}

type kvPrefixIterator struct {
	db     *KeyValueDB
	prefix string
	idx    int
}

// Next implements part of the keyvalue.Iterator interface.
func (p *kvPrefixIterator) Next() (key, val []byte, err error) {
	if p.idx >= len(p.db.keys) || !strings.HasPrefix(p.db.keys[p.idx], p.prefix) {
		return nil, nil, io.EOF
	}

	k := p.db.keys[p.idx]
	v := p.db.db[k]
	p.idx++
	return []byte(k), []byte(v), nil
}

// Seek implements part of the keyvalue.Iterator interface.
func (p *kvPrefixIterator) Seek(k []byte) error {
	s := string(k)
	i := sort.Search(len(p.db.keys), func(i int) bool { return strings.Compare(p.db.keys[i], s) >= 0 })
	if i < p.idx {
		return fmt.Errorf("given key before current iterator position: %q", k)
	}
	p.idx = i
	return nil
}

// Close implements part of the keyvalue.Iterator interface.
func (p *kvPrefixIterator) Close() error {
	p.db.mu.RUnlock()
	return nil
}

// ScanPrefix implements part of the keyvalue.DB interface.
func (k *KeyValueDB) ScanPrefix(ctx context.Context, prefix []byte, opts *keyvalue.Options) (keyvalue.Iterator, error) {
	k.mu.RLock()
	p := string(prefix)
	i := sort.Search(len(k.keys), func(i int) bool { return strings.Compare(k.keys[i], p) >= 0 })
	return &kvPrefixIterator{k, p, i}, nil
}

type kvRangeIterator struct {
	db  *KeyValueDB
	end *string
	idx int
}

// Next implements part of the keyvalue.Iterator interface.
func (p *kvRangeIterator) Next() (key, val []byte, err error) {
	if p.idx >= len(p.db.keys) || (p.end != nil && strings.Compare(p.db.keys[p.idx], *p.end) >= 0) {
		return nil, nil, io.EOF
	}

	k := p.db.keys[p.idx]
	v := p.db.db[k]
	p.idx++
	return []byte(k), []byte(v), nil
}

// Seek implements part of the keyvalue.Iterator interface.
func (p *kvRangeIterator) Seek(k []byte) error {
	s := string(k)
	i := sort.Search(len(p.db.keys), func(i int) bool { return strings.Compare(p.db.keys[i], s) >= 0 })
	if i < p.idx {
		return fmt.Errorf("given key before current iterator position: %q", k)
	}
	p.idx = i
	return nil
}

// Close implements part of the keyvalue.Iterator interface.
func (p *kvRangeIterator) Close() error {
	p.db.mu.RUnlock()
	return nil
}

// ScanRange implements part of the keyvalue.DB interface.
func (k *KeyValueDB) ScanRange(ctx context.Context, r *keyvalue.Range, opts *keyvalue.Options) (keyvalue.Iterator, error) {
	k.mu.RLock()
	var start int
	if r != nil && len(r.Start) != 0 {
		s := string(r.Start)
		start = sort.Search(len(k.keys), func(i int) bool { return strings.Compare(k.keys[i], s) >= 0 })
	}
	var end *string
	if r != nil && r.End != nil {
		e := string(r.End)
		end = &e
	}
	return &kvRangeIterator{k, end, start}, nil
}

type kvWriter struct{ db *KeyValueDB }

// Write implements part of the keyvalue.Writer interface.
func (w kvWriter) Write(key, val []byte) error {
	k := string(key)
	i := sort.Search(len(w.db.keys), func(i int) bool { return strings.Compare(w.db.keys[i], k) >= 0 })
	if i == len(w.db.keys) {
		w.db.keys = append(w.db.keys, k)
	} else if w.db.keys[i] != k {
		w.db.keys = append(w.db.keys[:i], append([]string{k}, w.db.keys[i:]...)...)
	}
	w.db.db[k] = val
	return nil
}

// Close implements part of the keyvalue.Writer interface.
func (w kvWriter) Close() error {
	w.db.mu.Unlock()
	return nil
}

// Writer implements part of the keyvalue.DB interface.
func (k *KeyValueDB) Writer(ctx context.Context) (keyvalue.Writer, error) {
	k.mu.Lock()
	return kvWriter{k}, nil
}

// NewSnapshot implements part of the keyvalue.DB interface.
func (k *KeyValueDB) NewSnapshot(ctx context.Context) keyvalue.Snapshot { return nil }

// Close implements part of the keyvalue.DB interface.
func (k *KeyValueDB) Close(context.Context) error { return nil }
