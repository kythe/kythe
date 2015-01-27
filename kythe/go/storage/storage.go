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

// Package storage declares the GraphStore interface and exposes utility functions for
// key/value-based GraphStore implementations.
package storage

import (
	"bytes"
	"container/heap"
	"fmt"
	"log"
	"strings"
	"sync"

	spb "kythe/proto/storage_proto"
)

// GraphStore refers to an open Kythe graph storage server.
type GraphStore interface {
	// Read sends to stream all entries with the ReadRequest's given source VName, subject
	// to the following rules:
	//
	// |----------+---------------------------------------------------------|
	// | EdgeKind | Result                                                  |
	// |----------+---------------------------------------------------------|
	// | ø        | All entries with kind and target empty (node entries).  |
	// | "*"      | All entries (node and edge, regardless of kind/target). |
	// | "kind"   | All edge entries with the given edge kind.              |
	// |----------+---------------------------------------------------------|
	//
	// Read returns when there are no more entries to send. The Read operation should be
	// implemented with time complexity proportional to the size of the return set.
	Read(req *spb.ReadRequest, stream chan<- *spb.Entry) error

	// Scan sends to stream all entries with the specified target VName, kind, and fact
	// label prefix. If any field is empty, any Entry value for that fields matches and
	// will be returned. Scan returns when there are no more entries to send. Scan is
	// similar to Read, but with no time complexity restrictions.
	Scan(req *spb.ScanRequest, stream chan<- *spb.Entry) error

	// Write atomically inserts or updates a collection of entries into the GraphStore.
	// Each update is a tuple of the form (kind, target, fact, value). For each such
	// update, the entry (source, kind, target, fact, value) is written into the store,
	// replacing any existing entry (source, kind, target, fact, value') that may
	// exist. Note that this operation cannot delete any data from the store; entries are
	// only ever inserted or updated. Apart from acting atomically, no other constraints
	// are placed on the implementation.
	Write(req *spb.WriteRequest) error

	// Close and release any underlying resources used by the GraphStore.
	// No operations should be used on the GraphStore after this has been called.
	Close() error
}

// ShardedGraphStore is a GraphStore that can be arbitrarily sharded for
// parallel processing.  Depending on the implementation, these methods may not
// return consistent results when the GraphStore is being written to.
type ShardedGraphStore interface {
	GraphStore

	// Count returns the number of entries in the given shard.
	Count(req *spb.CountRequest) (int64, error)

	// Shard sends to stream all of the entries within the given shard.
	Shard(req *spb.ShardRequest, stream chan<- *spb.Entry) error
}

// EachScanEntry calls f for each Entry in the GraphStore matching the ScanRequest.
func EachScanEntry(gs GraphStore, req *spb.ScanRequest, f func(*spb.Entry) error) error {
	var wg sync.WaitGroup
	wg.Add(1)
	entries := make(chan *spb.Entry)
	var iterErr error
	go func() {
		defer wg.Done()
		for entry := range entries {
			if iterErr != nil {
				continue
			}
			iterErr = f(entry)
		}
	}()

	if req == nil {
		req = &spb.ScanRequest{}
	}
	err := gs.Scan(req, entries)
	close(entries)
	wg.Wait()
	if iterErr != nil {
		return iterErr
	}
	if err != nil {
		return fmt.Errorf("scan error: %v", err)
	}
	return nil
}

// EachReadEntry calls f for each Entry in the GraphStore matching the ReadRequest.
func EachReadEntry(gs GraphStore, req *spb.ReadRequest, f func(*spb.Entry) error) error {
	var wg sync.WaitGroup
	wg.Add(1)
	entries := make(chan *spb.Entry)
	var iterErr error
	go func() {
		defer wg.Done()
		for entry := range entries {
			if iterErr != nil {
				continue
			}
			iterErr = f(entry)
		}
	}()

	if req == nil {
		req = &spb.ReadRequest{}
	}
	err := gs.Read(req, entries)
	close(entries)
	wg.Wait()
	if iterErr != nil {
		return iterErr
	}
	if err != nil {
		return fmt.Errorf("read error: %v", err)
	}
	return nil
}

// EachShardEntry calls f for each Entry in the given GraphStore shard
func EachShardEntry(gs ShardedGraphStore, req *spb.ShardRequest, f func(*spb.Entry) error) error {
	var wg sync.WaitGroup
	wg.Add(1)
	entries := make(chan *spb.Entry)
	var iterErr error
	go func() {
		defer wg.Done()
		for entry := range entries {
			if iterErr != nil {
				continue
			}
			iterErr = f(entry)
		}
	}()

	err := gs.Shard(req, entries)
	close(entries)
	wg.Wait()
	if iterErr != nil {
		return iterErr
	}
	if err != nil {
		return fmt.Errorf("read error: %v", err)
	}
	return nil
}

// EntryMatchesScan returns whether the Entry should be in the result set for the ScanRequest.
func EntryMatchesScan(req *spb.ScanRequest, entry *spb.Entry) bool {
	return (req.Target == nil || VNameEqual(entry.Target, req.Target)) &&
		(req.GetEdgeKind() == "" || entry.GetEdgeKind() == req.GetEdgeKind()) &&
		strings.HasPrefix(entry.GetFactName(), req.GetFactPrefix())
}

// EntryLess returns true if i is sorted before j in GraphStore results.
func EntryLess(i, j *spb.Entry) bool {
	return EntryCompare(i, j) == LT
}

// EntryCompare whether i is LT, GT, or EQ to j in GraphStore results.
// EntryCompare compares the following fields, in order, until a difference is
// found:
//   - Source
//   - EdgeKind
//   - FactName
//   - Target
func EntryCompare(i, j *spb.Entry) Order {
	if i == j {
		return EQ
	}
	sourceComp := VNameCompare(i.GetSource(), j.GetSource())
	if sourceComp == LT {
		return LT
	} else if sourceComp == GT {
		return GT
	} else if i.GetEdgeKind() < j.GetEdgeKind() {
		return LT
	} else if i.GetEdgeKind() > j.GetEdgeKind() {
		return GT
	} else if i.GetFactName() < j.GetFactName() {
		return LT
	} else if i.GetFactName() > j.GetFactName() {
		return GT
	}
	return VNameCompare(i.GetTarget(), j.GetTarget())
}

// Order represents a total order for values.
type Order int

// LT, EQ, and GT (each self-explanatory) are the only acceptable Order values.
const (
	LT Order = -1
	EQ Order = 0
	GT Order = 1
)

// VNameCompare returns EQ if i and j are equal, LT if i is sorted before j in a
// GraphStore, and GT otherwise. VNameCompare compares the VName field in the
// following order until a difference is found:
//   - Signature
//   - Corpus
//   - Root
//   - Path
//   - Language
func VNameCompare(i, j *spb.VName) Order {
	if i == j {
		return EQ
	} else if i.GetSignature() < j.GetSignature() {
		return LT
	} else if i.GetSignature() > j.GetSignature() {
		return GT
	} else if i.GetCorpus() < j.GetCorpus() {
		return LT
	} else if i.GetCorpus() > j.GetCorpus() {
		return GT
	} else if i.GetRoot() < j.GetRoot() {
		return LT
	} else if i.GetRoot() > j.GetRoot() {
		return GT
	} else if i.GetPath() < j.GetPath() {
		return LT
	} else if i.GetPath() > j.GetPath() {
		return GT
	} else if i.GetLanguage() < j.GetLanguage() {
		return LT
	} else if i.GetLanguage() > j.GetLanguage() {
		return GT
	}
	return EQ
}

type proxyGraphStore struct {
	clients []GraphStore
}

// NewProxy returns a GraphStore that forwards Reads, Writes, and Scans to a set
// of GraphStores.
func NewProxy(clients ...GraphStore) GraphStore {
	return &proxyGraphStore{clients}
}

// Read implements a GraphStore and forwards the ReadRequest to the proxied
// GraphStores.
func (p *proxyGraphStore) Read(req *spb.ReadRequest, stream chan<- *spb.Entry) error {
	errors := make([]error, len(p.clients))
	entries := make([]chan *spb.Entry, len(p.clients))
	for idx, client := range p.clients {
		entries[idx] = make(chan *spb.Entry)
		go func(idx int, client GraphStore) {
			defer close(entries[idx])
			errors[idx] = client.Read(req, entries[idx])
		}(idx, client)
	}
	mergeEntries(stream, entries)
	return lastError(proxyErrorPrefix, errors)
}

// Scan implements a GraphStore and forwards the ReadRequest to the proxied
// GraphStores.
func (p *proxyGraphStore) Scan(req *spb.ScanRequest, stream chan<- *spb.Entry) error {
	errors := make([]error, len(p.clients))
	entries := make([]chan *spb.Entry, len(p.clients))
	for idx, client := range p.clients {
		entries[idx] = make(chan *spb.Entry)
		go func(idx int, client GraphStore) {
			defer close(entries[idx])
			errors[idx] = client.Scan(req, entries[idx])
		}(idx, client)
	}
	mergeEntries(stream, entries)
	return lastError(proxyErrorPrefix, errors)
}

// Write implements a GraphStore and forwards the ReadRequest to the proxied
// GraphStores.
func (p *proxyGraphStore) Write(req *spb.WriteRequest) error {
	errors := make([]error, len(p.clients))
	wg := new(sync.WaitGroup)
	wg.Add(len(p.clients))
	for idx, client := range p.clients {
		go func(idx int, client GraphStore) {
			defer wg.Done()
			errors[idx] = client.Write(req)
		}(idx, client)
	}
	wg.Wait()
	return lastError(proxyErrorPrefix, errors)
}

// Close implements a GraphStore and calls Close on each proxied GraphStore.
func (p *proxyGraphStore) Close() error {
	errors := make([]error, len(p.clients))
	wg := new(sync.WaitGroup)
	wg.Add(len(p.clients))
	for idx, client := range p.clients {
		go func(idx int, client GraphStore) {
			defer wg.Done()
			errors[idx] = client.Close()
		}(idx, client)
	}
	wg.Wait()
	return lastError(proxyErrorPrefix, errors)
}

const proxyErrorPrefix = "proxyGraphStore: client GraphStore error"

func lastError(prefix string, errors []error) error {
	var lastErr error
	for _, e := range errors {
		if e != nil {
			if lastErr != nil {
				log.Printf("%s: %v", prefix, e)
			}
			lastErr = e
		}
	}
	return lastErr
}

type entryItem struct {
	entry  *spb.Entry
	stream <-chan *spb.Entry
}

func mergeEntries(entries chan<- *spb.Entry, streams []chan *spb.Entry) {
	merge := &mergedEntries{}
	for _, stream := range streams {
		entry := <-stream
		if entry != nil {
			*merge = append(*merge, &entryItem{entry, stream})
		}
	}
	heap.Init(merge)
	var lastEntry *spb.Entry
	for merge.Len() > 0 {
		item := heap.Pop(merge).(*entryItem)
		if lastEntry == nil || !EntryEqual(item.entry, lastEntry) {
			entries <- item.entry
		}
		newEntry := <-item.stream
		if newEntry != nil {
			item.entry = newEntry
			heap.Push(merge, item)
		}
	}
}

// mergedEntries is a Heap of entryItems (sorted by the entries).
type mergedEntries []*entryItem

func (m mergedEntries) Len() int { return len(m) }
func (m mergedEntries) Less(i, j int) bool {
	return EntryLess(m[i].entry, m[j].entry)
}
func (m mergedEntries) Swap(i, j int) {
	m[i], m[j] = m[j], m[i]
}
func (m *mergedEntries) Push(x interface{}) {
	*m = append(*m, x.(*entryItem))
}
func (m *mergedEntries) Pop() interface{} {
	old := *m
	n := len(old)
	item := old[n-1]
	*m = old[0 : n-1]
	return item
}

// BatchWrites returns a channel of WriteRequests for the given entries.
// Consecutive entries with the same Source will be collected in the same
// WriteRequest, with each request containing up to maxSize updates.
func BatchWrites(entries <-chan *spb.Entry, maxSize int) <-chan *spb.WriteRequest {
	ch := make(chan *spb.WriteRequest)
	go func() {
		defer close(ch)
		var req *spb.WriteRequest
		for entry := range entries {
			update := &spb.WriteRequest_Update{
				EdgeKind:  entry.EdgeKind,
				Target:    entry.Target,
				FactName:  entry.FactName,
				FactValue: entry.FactValue,
			}

			if req != nil && (!VNameEqual(req.Source, entry.Source) || len(req.Update) >= maxSize) {
				ch <- req
				req = nil
			}

			if req == nil {
				req = &spb.WriteRequest{
					Source: entry.Source,
					Update: []*spb.WriteRequest_Update{update},
				}
			} else {
				req.Update = append(req.Update, update)
			}
		}
		if req != nil {
			ch <- req
		}
	}()
	return ch
}

// EntryEqual determines if two Entry protos are equivalent
func EntryEqual(e1, e2 *spb.Entry) bool {
	return (e1 == e2) || (e1.GetFactName() == e2.GetFactName() && e1.GetEdgeKind() == e2.GetEdgeKind() && VNameEqual(e1.Target, e2.Target) && VNameEqual(e1.Source, e2.Source) && bytes.Equal(e1.FactValue, e2.FactValue))
}

// VNameEqual determines if two VNames are equivalent
func VNameEqual(v1, v2 *spb.VName) bool {
	return (v1 == v2) || (v1.GetSignature() == v2.GetSignature() && v1.GetCorpus() == v2.GetCorpus() && v1.GetRoot() == v2.GetRoot() && v1.GetPath() == v2.GetPath() && v1.GetLanguage() == v2.GetLanguage())
}
