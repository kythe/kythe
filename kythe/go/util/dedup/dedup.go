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

// Package dedup implements a utility to determine if a record has not been seen
// before (whether it's unique).
package dedup // import "kythe.io/kythe/go/util/dedup"

import (
	"crypto/sha512"
	"fmt"
)

// HashSize is the size of hash used to determine uniqueness in Deduper.
const HashSize = sha512.Size384

// Deduper determines if a data record has been seen before by checking its
// size-limited cache of hashes.
type Deduper struct {
	pri, sec map[[HashSize]byte]struct{}
	maxSize  int // maximum number of entries in each half of the hash cache

	duplicates, unique uint64
}

// New returns a new Deduper with the given cache size (in bytes).  maxSize must be at least HashSize/2.
func New(maxSize int) (*Deduper, error) {
	max := maxSize / HashSize / 2
	if max <= 0 {
		return nil, fmt.Errorf("invalid cache size: %d (must be at least %d)", maxSize, 2*HashSize)
	}
	return &Deduper{
		pri:     make(map[[HashSize]byte]struct{}),
		sec:     make(map[[HashSize]byte]struct{}),
		maxSize: max,
	}, nil
}

// Unique returns the number of unique records seen so far.
func (d *Deduper) Unique() uint64 {
	if d == nil {
		return 0
	}
	return d.unique
}

// Duplicates returns the number of duplicate records seen so far.
func (d *Deduper) Duplicates() uint64 {
	if d == nil {
		return 0
	}
	return d.duplicates
}

// IsUnique determines if the given data record has not been seen before.
func (d *Deduper) IsUnique(data []byte, rest ...[]byte) bool {
	if d == nil {
		return true
	}

	hash := d.hash(data, rest...)
	if d.seen(hash) {
		d.duplicates++
		return false
	}
	d.unique++

	if len(d.sec) == d.maxSize {
		d.pri = d.sec
		d.sec = make(map[[HashSize]byte]struct{})
	}
	if len(d.pri) == d.maxSize {
		d.sec[hash] = struct{}{}
	} else {
		d.pri[hash] = struct{}{}
	}

	return true
}

func (d *Deduper) hash(data []byte, rest ...[]byte) (hash [HashSize]byte) {
	h := sha512.New384()
	h.Write(data)
	for _, d := range rest {
		h.Write(d)
	}
	s := h.Sum(nil)
	copy(hash[:], s[:HashSize])
	return
}

func (d *Deduper) seen(hash [HashSize]byte) bool {
	if _, ok := d.pri[hash]; ok {
		return true
	}
	_, ok := d.sec[hash]
	return ok
}
