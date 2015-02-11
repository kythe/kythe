/*
 * Copyright 2015 Google Inc. All rights reserved.
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

// Package graphstore defines the Service and Sharded interfaces, and provides
// some useful utility functions.
package graphstore

import (
	"container/heap"
	"io"
	"strings"
	"sync"

	"kythe/go/services/graphstore/compare"

	spb "kythe/proto/storage_proto"
)

// An EntryFunc is a callback from the implementation of a Service to deliver
// entry messages. If the callback returns an error, the operation stops.  If
// the error is io.EOF, the operation returns nil; otherwise it returns the
// error value from the callback.
type EntryFunc func(*spb.Entry) error

// Service refers to an open Kythe graph store.
type Service interface {
	// Read calls f with each entry having the ReadRequest's given source
	// VName, subject to the following rules:
	//
	//  |----------+---------------------------------------------------------|
	//  | EdgeKind | Result                                                  |
	//  |----------+---------------------------------------------------------|
	//  | ø        | All entries with kind and target empty (node entries).  |
	//  | "*"      | All entries (node and edge, regardless of kind/target). |
	//  | "kind"   | All edge entries with the given edge kind.              |
	//  |----------+---------------------------------------------------------|
	//
	// Read returns when there are no more entries to send. The Read operation should be
	// implemented with time complexity proportional to the size of the return set.
	Read(req *spb.ReadRequest, f EntryFunc) error

	// Scan calls f with each entries having the specified target VName, kind,
	// and fact label prefix. If a field is empty, any entry value for that
	// field matches and will be returned. Scan returns when there are no more
	// entries to send. Scan is similar to Read, but with no time complexity
	// restrictions.
	Scan(req *spb.ScanRequest, f EntryFunc) error

	// Write atomically inserts or updates a collection of entries into the store.
	// Each update is a tuple of the form (kind, target, fact, value). For each such
	// update, an entry (source, kind, target, fact, value) is written into the store,
	// replacing any existing entry (source, kind, target, fact, value') that may
	// exist. Note that this operation cannot delete any data from the store; entries are
	// only ever inserted or updated. Apart from acting atomically, no other constraints
	// are placed on the implementation.
	Write(req *spb.WriteRequest) error

	// Close and release any underlying resources used by the store.
	// No operations may be used on the store after this has been called.
	Close() error
}

// Sharded represents a store that can be arbitrarily sharded for parallel
// processing.  Depending on the implementation, these methods may not return
// consistent results when the store is being written to.  Shards are indexed
// from 0.
type Sharded interface {
	Service

	// Count returns the number of entries in the given shard.
	Count(req *spb.CountRequest) (int64, error)

	// Shard calls f with each entry in the given shard.
	Shard(req *spb.ShardRequest, f EntryFunc) error
}

type proxyService struct {
	stores []Service
}

// NewProxy returns a Service that forwards Reads, Writes, and Scans to a set
// of stores in parallel, and merges their results.
func NewProxy(stores ...Service) Service { return &proxyService{stores} }

// Read implements Service and forwards the request to the proxied stores.
func (p *proxyService) Read(req *spb.ReadRequest, f EntryFunc) error {
	return p.invoke(func(svc Service, cb EntryFunc) error {
		return svc.Read(req, cb)
	}, f)
}

// Scan implements part of graphstore.Service by forwarding the request to the
// proxied stores.
func (p *proxyService) Scan(req *spb.ScanRequest, f EntryFunc) error {
	return p.invoke(func(svc Service, cb EntryFunc) error {
		return svc.Scan(req, cb)
	}, f)
}

// Write implements part of graphstore.Service by forwarding the request to the
// proxied stores.
func (p *proxyService) Write(req *spb.WriteRequest) error {
	return waitErr(p.foreach(func(i int, s Service) error {
		return s.Write(req)
	}))
}

// Close implements part of graphstore.Service by calling Close on each proxied
// store.  All the stores are given an opportunity to close, even in case of
// error, but only one error is returned.
func (p *proxyService) Close() error {
	return waitErr(p.foreach(func(i int, s Service) error {
		return s.Close()
	}))
}

// waitErr reads values from errc until it closes, then returns the first
// non-nil error it received (if any).
func waitErr(errc <-chan error) error {
	var err error
	for e := range errc {
		if e != nil && err == nil {
			err = e
		}
	}
	return err
}

// foreach concurrently invokes f(i, p.stores[i]) for each proxied store.  The
// return value from each invocation is delivered to the error channel that is
// returned, which will be closed once all the calls are complete.  The channel
// is unbuffered, so the caller must drain the channel to avoid deadlock.
func (p *proxyService) foreach(f func(int, Service) error) <-chan error {
	errc := make(chan error)
	var wg sync.WaitGroup
	wg.Add(len(p.stores))
	for i, s := range p.stores {
		i, s := i, s
		go func() {
			defer wg.Done()
			errc <- f(i, s)
		}()
	}
	go func() { wg.Wait(); close(errc) }()
	return errc
}

// entryHeap is a min-heap of entries, ordered by compare.Entries.
type entryHeap []*spb.Entry

func (h entryHeap) Len() int           { return len(h) }
func (h entryHeap) Less(i, j int) bool { return compare.Entries(h[i], h[j]) == compare.LT }
func (h entryHeap) Swap(i, j int)      { h[i], h[j] = h[j], h[i] }

func (h *entryHeap) Push(v interface{}) { *h = append(*h, v.(*spb.Entry)) }

func (h *entryHeap) Pop() interface{} {
	n := h.Len() - 1
	out := (*h)[n]
	*h = (*h)[:n]
	return out
}

// EntryMatchesScan reports whether entry belongs in the result set for req.
func EntryMatchesScan(req *spb.ScanRequest, entry *spb.Entry) bool {
	return (req.GetTarget() == nil || compare.VNamesEqual(entry.Target, req.Target)) &&
		(req.GetEdgeKind() == "" || entry.GetEdgeKind() == req.GetEdgeKind()) &&
		strings.HasPrefix(entry.GetFactName(), req.GetFactPrefix())
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

			if req != nil && (!compare.VNamesEqual(req.Source, entry.Source) || len(req.Update) >= maxSize) {
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

// invoke calls req concurrently for each delegated service in p, merges the
// results, and delivers them to f.
func (p *proxyService) invoke(req func(Service, EntryFunc) error, f EntryFunc) error {
	stop := make(chan struct{}) // Closed to signal cancellation

	// Create a channel for each delegated request, and a callback that
	// delivers results to that channel.  The callback will handle cancellation
	// signaled by a close of the stop channel, and exit early.

	rcv := make([]EntryFunc, len(p.stores))       // callbacks
	chs := make([]chan *spb.Entry, len(p.stores)) // channels
	for i := range p.stores {
		ch := make(chan *spb.Entry)
		chs[i] = ch
		rcv[i] = func(e *spb.Entry) error {
			select {
			case <-stop: // cancellation has been signalled
				return nil
			case ch <- e:
				return nil
			}
		}
	}

	// Invoke the requests for each service, using the corresponding callback.
	errc := p.foreach(func(i int, s Service) error {
		err := req(s, rcv[i])
		close(chs[i])
		return err
	})

	// Accumulate and merge the results.  This is a straightforward round-robin
	// n-finger merge of the values from the delegated requests.

	var h entryHeap     // used to preserve stream order
	var last *spb.Entry // used to deduplicate entries
	var perr error      // error while accumulating
	go func() {
		defer close(stop)
		for {
			hit := false // are any requests still pending?

			// Give each channel a chance to produce a value, round-robin to
			// preserve the global ordering.
			for _, ch := range chs {
				if e, ok := <-ch; ok {
					hit = true
					heap.Push(&h, e)
				}
			}

			// If there are any values pending, deliver one to the consumer.
			// If not, and there are no more values coming, we're finished.
			if h.Len() != 0 {
				entry := heap.Pop(&h).(*spb.Entry)
				if last == nil || !compare.EntriesEqual(last, entry) {
					last = entry
					if err := f(entry); err != nil {
						if err != io.EOF {
							perr = err
						}
						return
					}
				}
			} else if !hit {
				return // no more work to do
			}
		}
	}()
	err := waitErr(errc) // wait for all receives to complete
	<-stop               // wait for all sends to complete
	if perr != nil {
		return perr
	}
	return err
}
