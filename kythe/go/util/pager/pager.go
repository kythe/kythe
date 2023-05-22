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

// Package pager implements a generic SetPager that splits a stream of Groups
// into a single Set and one-or-more associated Pages.  Useful for constructing
// paged serving data.
package pager // import "kythe.io/kythe/go/util/pager"

import (
	"container/heap"
	"context"
	"errors"
	"fmt"

	"kythe.io/kythe/go/util/sortutil"
)

// A Head signals the start of a new Set.
type Head any

// A Group is part of a Set.
type Group any

// A Set is a set of Groups.
type Set any

// SetPager constructs a set of Sets and Pages from a sequence of Heads and
// Groups.  For each set of Groups with the same Head, a call to StartSet must
// precede.  All Groups for the same Head are then assumed to be given
// sequentially to AddGroup.  Flush must be called after the final call to
// AddGroup.
type SetPager struct {
	// MaxPageSize is the maximum size of a Set or Page, as calculated by the
	// given Size function.
	MaxPageSize int

	// SkipEmpty determines whether empty Sets/Pages will be emitted.
	SkipEmpty bool

	// OutputSet should output the given Set and Groups not previously emitted by
	// OutputPage.  The total size of all Groups is given.
	OutputSet func(context.Context, int, Set, []Group) error
	// OutputPage should output the given Group as an individual Page.  The Set
	// currently being built is given for any necessary mutations.
	OutputPage func(context.Context, Set, Group) error

	// NewSet returns a new Set for the given Head.
	NewSet func(Head) Set
	// Combine possibly merges two Groups with the Head together.  If not
	// possible, nil should be returned.
	//
	//   Constraints (if g != nil):
	//     Combine(l, r) == Split(Size(l), g)
	Combine func(l, r Group) (g Group)
	// Split splits the given Group into a Group of the given size and a Group
	// with any leftovers.
	//
	//   Constraints:
	//     Size(l) == total
	//     Size(r) == Size(g) - total
	//     g == Combine(l, r)
	Split func(total int, g Group) (l, r Group)
	// Size returns the size of the given Group.
	//
	// Constraints:
	//   Size(l) + Size(r) == Size(Combine(l, r))
	Size func(Group) int

	curSet          Set
	curGrp          Group
	groups          *sortutil.ByLesser // heap sorted by Size
	resident, total int
}

// StartSet begins a new Set for the given Head, possibly emitting a previous
// Set.  Each following call to AddGroup adds the group to this new Set until
// another call to StartSet is made.
func (p *SetPager) StartSet(ctx context.Context, hd Head) error {
	if p.curSet != nil {
		if err := p.Flush(ctx); err != nil {
			return fmt.Errorf("error flushing previous set: %v", err)
		}
	}

	p.curSet = p.NewSet(hd)
	p.groups = &sortutil.ByLesser{
		Lesser: sortutil.LesserFunc(func(a, b any) bool {
			// Sort larger Groups first.
			return p.Size(a) > p.Size(b)
		}),
	}

	return nil
}

// AddGroup adds a Group to current Set being built, possibly emitting a new Set
// and/or Page.  StartSet must be called before any calls to this method.  See
// SetPager's documentation for the assumed order of the groups and this
// method's relation to StartSet.
func (p *SetPager) AddGroup(ctx context.Context, g Group) error {
	if p.curSet == nil {
		return errors.New("no Set currently being built")
	}

	// Setup p.curGrp; ensuring it is non-nil
	sz := p.Size(g)
	if p.SkipEmpty && sz == 0 {
		return nil
	} else if p.curGrp == nil {
		p.curGrp = g
	} else if c := p.Combine(p.curGrp, g); c != nil {
		p.curGrp = c
	} else {
		// We can't combine the current group with g.  Push the current group onto
		// the heap and make g the new current group.
		heap.Push(p.groups, p.curGrp)
		p.curGrp = g
	}
	// Update group size counters
	p.resident += sz
	p.total += sz

	// Handle creation of pages when # of resident elements passes config value
	for p.MaxPageSize > 0 && p.resident > p.MaxPageSize {
		var eviction Group
		// p.curGrp can be nil if we evicted it in a previous loop iteration
		if p.curGrp != nil {
			if p.Size(p.curGrp) > p.MaxPageSize {
				// Split the large page; evict page exactly sized b.MaxPageSize
				eviction, p.curGrp = p.Split(p.MaxPageSize, p.curGrp)
			} else if p.groups.Len() == 0 || p.Size(p.curGrp) > p.Size(p.groups.Peek()) {
				// Evict p.curGrp, it's larger than any other group we have
				eviction, p.curGrp = p.curGrp, nil
			}
		}
		if eviction == nil {
			// Evict the largest group we have
			eviction = heap.Pop(p.groups)
		}

		p.resident -= p.Size(eviction)
		if err := p.OutputPage(ctx, p.curSet, eviction); err != nil {
			return err
		}
	}

	return nil
}

// Flush signals the end of the current Set being built, flushing it, and its
// Groups to the output function.  This should be called after the final call to
// AddGroup.  Manually calling Flush at any other time is unnecessary.
func (p *SetPager) Flush(ctx context.Context) error {
	if p == nil || p.curSet == nil {
		return nil
	} else if p.curGrp != nil {
		p.groups.Push(p.curGrp) // the order of this last group doesn't matter
	}

	// grps := p.groups.Slice.([]Group)
	grps := make([]Group, len(p.groups.Slice))
	for i, g := range p.groups.Slice {
		grps[i] = g
	}

	var err error
	if !p.SkipEmpty || p.total > 0 {
		err = p.OutputSet(ctx, p.total, p.curSet, grps)
	}
	p.curSet, p.curGrp, p.groups, p.resident, p.total = nil, nil, nil, 0, 0
	return err
}
