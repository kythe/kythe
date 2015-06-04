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

// Package inmemory implements a simple in-memory graphstore.Service.
package inmemory

import (
	"io"
	"sort"
	"sync"

	"kythe.io/kythe/go/services/graphstore"
	"kythe.io/kythe/go/services/graphstore/compare"

	spb "kythe.io/kythe/proto/storage_proto"

	"github.com/golang/protobuf/proto"
	"golang.org/x/net/context"
)

type store struct {
	entries []*spb.Entry
	mu      sync.RWMutex
}

// Create returns a new in-memory graphstore.Service
func Create() graphstore.Service { return &store{} }

// Close implements part of the graphstore.Service interface.
func (*store) Close(ctx context.Context) error { return nil }

// Write implements part of the graphstore.Service interface.
func (s *store) Write(ctx context.Context, req *spb.WriteRequest) error {
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

func (s *store) insert(e *spb.Entry) {
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
func (s *store) Read(ctx context.Context, req *spb.ReadRequest, f graphstore.EntryFunc) error {
	s.mu.RLock()
	defer s.mu.RUnlock()
	start := sort.Search(len(s.entries), func(i int) bool {
		comp := compare.VNames(s.entries[i].Source, req.Source)
		return comp != compare.LT && (comp == compare.GT || req.EdgeKind == "*" || s.entries[i].EdgeKind >= req.EdgeKind)
	})
	end := sort.Search(len(s.entries), func(i int) bool {
		comp := compare.VNames(s.entries[i].Source, req.Source)
		return comp == compare.GT || (req.EdgeKind != "*" && s.entries[i].EdgeKind > req.EdgeKind)
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
func (s *store) Scan(ctx context.Context, req *spb.ScanRequest, f graphstore.EntryFunc) error {
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
