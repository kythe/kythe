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
//
// TODO(fromberger): Move the ordering-related code to a separate package.
package graphstore

import (
	"bytes"
	"container/heap"
	"io"
	"strings"
	"sync"

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
	// and fact label prefix. If any field is empty, any Entry value for that
	// fields matches and will be returned. Scan returns when there are no more
	// entries to send. Scan is similar to Read, but with no time complexity
	// restrictions.
	Scan(req *spb.ScanRequest, f EntryFunc) error

	// Write atomically inserts or updates a collection of entries into the
	// Each update is a tuple of the form (kind, target, fact, value). For each such
	// update, the entry (source, kind, target, fact, value) is written into the store,
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

// entryHeap is a min-heap of entries, orderd by EntryLess.
type entryHeap []*spb.Entry

func (h entryHeap) Len() int           { return len(h) }
func (h entryHeap) Less(i, j int) bool { return EntryLess(h[i], h[j]) }
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
	return (req.GetTarget() == nil || VNameEqual(entry.Target, req.Target)) &&
		(req.GetEdgeKind() == "" || entry.GetEdgeKind() == req.GetEdgeKind()) &&
		strings.HasPrefix(entry.GetFactName(), req.GetFactPrefix())
}

// EntryLess reports whether if i precedes j in entry order.
func EntryLess(i, j *spb.Entry) bool { return EntryCompare(i, j) == LT }

// EntryCompare reports whether i is LT, GT, or EQ to j in entry order.
// The ordering for entries is defined by lexicographic comparison of
// [source, edge kind, fact name, target].
func EntryCompare(i, j *spb.Entry) Order {
	if i == j {
		return EQ
	}
	if c := VNameCompare(i.GetSource(), j.GetSource()); c != EQ {
		return c
	} else if c := stringComp(i.GetEdgeKind(), j.GetEdgeKind()); c != EQ {
		return c
	} else if c := stringComp(i.GetFactName(), j.GetFactName()); c != EQ {
		return c
	}
	return VNameCompare(i.GetTarget(), j.GetTarget())
}

// Order represents a total order for values.
type Order int

// LT, EQ, and GT are the standard values for an Order.
const (
	LT Order = -1 // lhs < rhs
	EQ Order = 0  // lhs == rhs
	GT Order = 1  // lhs > rhs
)

// stringComp returns LT if s < t, EQ if s == t, or GT if s > t.
func stringComp(s, t string) Order {
	switch {
	case s < t:
		return LT
	case s > t:
		return GT
	default:
		return EQ
	}
}

// VNameCompare returns LT if i precedes j, EQ if i and j are equal, or GT if i
// follows j, in standard order.  The ordering for VNames is defined by
// lexicographic comparison of [signature, corpus, root, path, language].
func VNameCompare(i, j *spb.VName) Order {
	if i == j {
		return EQ
	} else if c := stringComp(i.GetSignature(), j.GetSignature()); c != EQ {
		return c
	} else if c := stringComp(i.GetCorpus(), j.GetCorpus()); c != EQ {
		return c
	} else if c := stringComp(i.GetRoot(), j.GetRoot()); c != EQ {
		return c
	} else if c := stringComp(i.GetPath(), j.GetPath()); c != EQ {
		return c
	}
	return stringComp(i.GetLanguage(), j.GetLanguage())
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
				if last == nil || !EntryEqual(last, entry) {
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
