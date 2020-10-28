/*
 * Copyright 2015 The Kythe Authors. All rights reserved.
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

// Package proxy defines a proxy graphstore.Service that delegates requests to
// other service implementations.
package proxy // import "kythe.io/kythe/go/services/graphstore/proxy"

import (
	"container/heap"
	"context"
	"errors"
	"fmt"
	"io"
	"strings"
	"sync"

	"kythe.io/kythe/go/services/graphstore"
	"kythe.io/kythe/go/storage/gsutil"
	"kythe.io/kythe/go/util/compare"

	spb "kythe.io/kythe/proto/storage_go_proto"
)

func init() {
	gsutil.Register("proxy", proxyHandler)
}

func proxyHandler(spec string) (graphstore.Service, error) {
	var stores []graphstore.Service
	for _, s := range strings.Split(spec, ",") {
		gs, err := gsutil.ParseGraphStore(s)
		if err != nil {
			return nil, fmt.Errorf("proxy GraphStore error for %q: %v", s, err)
		}
		stores = append(stores, gs)
	}
	if len(stores) == 0 {
		return nil, errors.New("no proxy GraphStores specified")
	}
	return New(stores...), nil
}

type proxyService struct {
	stores []graphstore.Service
}

// New returns a graphstore.Service that forwards Reads, Writes, and Scans to a
// set of stores in parallel, and merges their results.
func New(stores ...graphstore.Service) graphstore.Service { return &proxyService{stores} }

// Read implements graphstore.Service and forwards the request to the proxied stores.
func (p *proxyService) Read(ctx context.Context, req *spb.ReadRequest, f graphstore.EntryFunc) error {
	return p.invoke(func(svc graphstore.Service, cb graphstore.EntryFunc) error {
		return svc.Read(ctx, req, cb)
	}, f)
}

// Scan implements part of graphstore.Service by forwarding the request to the
// proxied stores.
func (p *proxyService) Scan(ctx context.Context, req *spb.ScanRequest, f graphstore.EntryFunc) error {
	return p.invoke(func(svc graphstore.Service, cb graphstore.EntryFunc) error {
		return svc.Scan(ctx, req, cb)
	}, f)
}

// Write implements part of graphstore.Service by forwarding the request to the
// proxied stores.
func (p *proxyService) Write(ctx context.Context, req *spb.WriteRequest) error {
	return waitErr(p.foreach(func(i int, s graphstore.Service) error {
		return s.Write(ctx, req)
	}))
}

// Close implements part of graphstore.Service by calling Close on each proxied
// store.  All the stores are given an opportunity to close, even in case of
// error, but only one error is returned.
func (p *proxyService) Close(ctx context.Context) error {
	return waitErr(p.foreach(func(i int, s graphstore.Service) error {
		return s.Close(ctx)
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
func (p *proxyService) foreach(f func(int, graphstore.Service) error) <-chan error {
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

// invoke calls req concurrently for each delegated service in p, merges the
// results, and delivers them to f.
func (p *proxyService) invoke(req func(graphstore.Service, graphstore.EntryFunc) error, f graphstore.EntryFunc) error {
	stop := make(chan struct{}) // Closed to signal cancellation

	// Create a channel for each delegated request, and a callback that
	// delivers results to that channel.  The callback will handle cancellation
	// signaled by a close of the stop channel, and exit early.

	rcv := make([]graphstore.EntryFunc, len(p.stores)) // callbacks
	chs := make([]chan *spb.Entry, len(p.stores))      // channels
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
	errc := p.foreach(func(i int, s graphstore.Service) error {
		err := req(s, rcv[i])
		close(chs[i])
		return err
	})

	// Accumulate and merge the results.  This is a straightforward round-robin
	// n-finger merge of the values from the delegated requests.

	var h compare.ByEntries // used to preserve stream order
	var last *spb.Entry     // used to deduplicate entries
	var perr error          // error while accumulating
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
