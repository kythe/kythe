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

// Package cache implements a simple in-memory file cache and provides a simple
// Fetcher wrapper that uses the cache for its Fetch operations.
package cache // import "kythe.io/kythe/go/platform/cache"

import (
	"container/heap"
	"fmt"
	"sync"

	"kythe.io/kythe/go/platform/analysis"
)

type cachedFetcher struct {
	cache *Cache
	analysis.Fetcher
}

// Fetch implements the corresponding method of analysis.Fetcher by reading
// through the cache.
func (c cachedFetcher) Fetch(path, digest string) ([]byte, error) {
	key := fmt.Sprintf("%s\x00%s", path, digest)
	if data := c.cache.Get(key); data != nil {
		return data, nil
	}
	data, err := c.Fetcher.Fetch(path, digest)
	if err == nil {
		c.cache.Put(key, data)
	}
	return data, err
}

// Fetcher creates an analysis.Fetcher that implements fetches through the
// cache, and delegates all other operations to f.  If cache == nil, f is
// returned unmodified.  The returned value is safe for concurrent use if f is.
func Fetcher(f analysis.Fetcher, cache *Cache) analysis.Fetcher {
	if cache == nil {
		return f
	}
	return cachedFetcher{
		cache:   cache,
		Fetcher: f,
	}
}

// New returns a new empty cache with a capacity of maxBytes.
// Returns nil if maxBytes <= 0.
func New(maxBytes int) *Cache {
	if maxBytes <= 0 {
		return nil
	}
	return &Cache{
		maxBytes: maxBytes,
		data:     make(map[string]*entry),
	}
}

// A Cache implements a limited-size cache of key-value pairs, where keys are
// strings and values are byte slices.  Entries are evicted from the cache
// using a least-frequently used policy, based on how many times a given key
// has been fetched with Get.  A *Cache is safe for concurrent use.
type Cache struct {
	mu sync.RWMutex

	curBytes int               // Size of resident data.
	maxBytes int               // Total allowed capacity of cache.
	data     map[string]*entry // Cached entries.
	usage    countHeap         // Count-ordered heap for eviction.

	hits, misses int
}

// Has returns whether the specified key is resident in the cache.  This does
// not affect the usage count of the key for purposes of cache eviction.
func (c *Cache) Has(key string) bool {
	c.mu.RLock()
	defer c.mu.RUnlock()
	return c.data[key] != nil
}

// Get fetches the specified key from the cache, returning nil if the key is
// not present.  A successful fetch counts as a usage for the purposes of the
// cache eviction policy.
func (c *Cache) Get(key string) []byte {
	c.mu.Lock()
	defer c.mu.Unlock()

	if e := c.data[key]; e != nil {
		increment(&c.usage, e)
		c.hits++
		return e.data
	}
	c.misses++
	return nil
}

// Stats returns usage statistics for the cache.
func (c *Cache) Stats() (residentBytes, numHits, numMisses int) {
	if c == nil {
		return 0, 0, 0
	}
	c.mu.RLock()
	defer c.mu.RUnlock()
	return c.curBytes, c.hits, c.misses
}

// Put adds the specified key and data to the cache if it is not already
// present.  If necessary, existing keys are evicted to maintain size.
func (c *Cache) Put(key string, data []byte) {
	c.mu.Lock()
	defer c.mu.Unlock()

	// Quick return if this key is already recorded, or if data itself exceeds
	// the cache's total capacity.  There's no point in evicting other keys in
	// that case.
	if c.data[key] != nil || len(data) > c.maxBytes {
		return
	}

	// At this point we know that there is room for the data, save that we may
	// need to evict some of the existing entries (if there are no entries, we
	// have enough room by construction).
	newBytes := c.curBytes + len(data)
	for newBytes > c.maxBytes {
		goat := heap.Pop(&c.usage).(*entry)
		delete(c.data, goat.key)
		newBytes -= len(goat.data)
	}

	e := &entry{
		key:  key,
		data: data,
	}
	c.data[key] = e
	heap.Push(&c.usage, e)
	c.curBytes = newBytes
}

type entry struct {
	key   string
	data  []byte
	index int // Entry's position in the heap.
	count int // Number of fetches on this key.
}

// countHeap implements a min-heap of entries based on their count value.
// This permits an efficient implementation of a least-frequently used cache
// eviction policy.
type countHeap []*entry

// Len implements a method of heap.Interface.
func (h countHeap) Len() int { return len(h) }

// Less implements a method of heap.Interface.
func (h countHeap) Less(i, j int) bool { return h[i].count < h[j].count }

// Swap implements a method of heap.Interface.
func (h countHeap) Swap(i, j int) {
	h[i], h[j] = h[j], h[i]
	h[i].index = i
	h[j].index = j
}

// Push implements a method of heap.Interface.
func (h *countHeap) Push(item any) {
	if e, ok := item.(*entry); ok {
		n := len(*h)
		e.index = n
		*h = append(*h, e)
	}
}

// Pop implements a method of heap.Interface.
func (h *countHeap) Pop() any {
	if n := len(*h) - 1; n >= 0 {
		e := (*h)[n]
		e.index = -1
		*h = (*h)[:n]
		return e
	}
	return nil
}

// increment increases the count of e by 1 and updates its location in the heap.
func increment(h *countHeap, e *entry) {
	heap.Remove(h, e.index)
	e.count++
	heap.Push(h, e)
}

// A ByteSize implements the flag.Value interface to allow specifying a cache
// size on the command line.  Legal size values are as parsed by ParseByteSize.
type ByteSize int

// String implements a method of the flag.Value interface.
func (b *ByteSize) String() string { return fmt.Sprintf("%d", *b) }

// Get implements a method of the flag.Value interface.
func (b *ByteSize) Get() any { return *b }

// Set implements a method of the flag.Value interface.
func (b *ByteSize) Set(s string) error {
	v, err := ParseByteSize(s)
	if err != nil {
		return err
	}
	*b = ByteSize(v)
	return nil
}

// ParseByteSize parses a string specifying a possibly-fractional number with
// units and returns an equivalent value in bytes.  Fractions are rounded down.
// Returns -1 and an error in case of invalid format.
//
// The supported unit labels are:
//
//	B    * 2^0  bytes
//	K    * 2^10 bytes
//	M    * 2^20 bytes
//	G    * 2^30 bytes
//	T    * 2^40 bytes
//
// The labels are case-insensitive ("10g" is the same as "10G")
//
// Examples:
//
//	ParseByteSize("25")    ==> 25
//	ParseByteSize("1k")    ==> 1024
//	ParseByteSize("2.5G")  ==> 2684354560
//	ParseByteSize("10.3k") ==> 10547
//	ParseByteSize("-45xx") ==> -1 [error]
func ParseByteSize(s string) (int, error) {
	var value float64
	var unit string

	n, err := fmt.Sscanf(s, "%f%s", &value, &unit)
	if err != nil && n != 1 {
		return -1, fmt.Errorf("invalid byte size: %q", s)
	}
	if value < 0 {
		return -1, fmt.Errorf("invalid byte size: %q", s)
	}
	switch unit {
	case "", "b", "B":
		break
	case "k", "K":
		value *= 1 << 10
	case "m", "M":
		value *= 1 << 20
	case "g", "G":
		value *= 1 << 30
	case "t", "T":
		value *= 1 << 40
	default:
		return -1, fmt.Errorf("invalid byte size: %q", s)
	}
	return int(value), nil
}
