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

// Package disksort implements sorting algorithms for sets of data too large to
// fit fully in-memory.  If the number of elements becomes to large, data are
// paged onto the disk.
package disksort

import (
	"bufio"
	"container/heap"
	"errors"
	"fmt"
	"io"
	"io/ioutil"
	"os"
	"path/filepath"
	"sort"

	"kythe.io/kythe/go/platform/delimited"
	"kythe.io/kythe/go/util/sortutil"

	"github.com/golang/snappy"
)

// Interface is the standard interface for disk sorting algorithms.  Each
// element in the set of data to be sorted is added to the sorter with Add.
// Once all elements are added, Read can then be called to retrieve each element
// sequentially in sorted order.  Once Read is called, no other operations on
// the sorter are allowed.
type Interface interface {
	// Add adds a new element to the set of data to be sorted.
	Add(i interface{}) error

	// Read calls f on each elements previously added the set of data to be
	// sorted.  If f returns an error, it is returned immediately and f is no
	// longer called.  Once Read is called, no more data may be added to the
	// sorter.  Read may only be called once.
	Read(f func(interface{}) error) error
}

// Marshaler is an interface to functions that can binary encode/decode
// elements.
type Marshaler interface {
	// Marshal binary encodes the given element.
	Marshal(interface{}) ([]byte, error)

	// Unmarshal decodes the given encoding of an element.
	Unmarshal([]byte) (interface{}, error)
}

type mergeSorter struct {
	opts MergeOptions

	buffer  *sortutil.ByLesser
	workDir string
	shards  []string
}

// DefaultMaxInMemory is the default number of elements to keep in-memory during
// a merge sort.
const DefaultMaxInMemory = 32000

// DefaultIOBufferSize is the default size of the reading/writing buffers for
// the temporary file shards.
const DefaultIOBufferSize = 2 << 13

// MergeOptions specifies how to sort elements.
type MergeOptions struct {
	// Lesser is the comparison function for sorting the given elements.
	Lesser sortutil.Lesser
	// Marshaler is used for encoding/decoding elements in temporary file shards.
	Marshaler Marshaler

	// WorkDir is the directory used for writing temporary file shards.  If empty,
	// the default directory for temporary files is used.
	WorkDir string

	// MaxInMemory is the maximum number of elements to keep in-memory before
	// paging them to a temporary file shard.  If non-positive, DefaultMaxInMemory
	// is used.
	MaxInMemory int

	// CompressShards determines whether the temporary file shards should be
	// compressed.
	CompressShards bool

	// IOBufferSize is the size of the reading/writing buffers for the temporary
	// file shards.  If non-positive, DefaultIOBufferSize is used.
	IOBufferSize int
}

// NewMergeSorter returns a new disk sorter using a mergesort algorithm.
func NewMergeSorter(opts MergeOptions) (Interface, error) {
	if opts.Lesser == nil {
		return nil, errors.New("missing Lesser")
	} else if opts.Marshaler == nil {
		return nil, errors.New("missing Marshaler")
	}

	dir, err := ioutil.TempDir(opts.WorkDir, "external.merge.sort")
	if err != nil {
		return nil, fmt.Errorf("error creating temporary work directory: %v", err)
	}

	if opts.MaxInMemory <= 0 {
		opts.MaxInMemory = DefaultMaxInMemory
	}
	if opts.IOBufferSize <= 0 {
		opts.IOBufferSize = DefaultIOBufferSize
	}

	return &mergeSorter{
		opts:    opts,
		buffer:  &sortutil.ByLesser{Lesser: opts.Lesser},
		workDir: dir,
	}, nil
}

var (
	// ErrAlreadyFinalized is returned from Interface#Add when Interface#Read has
	// already been called, freezing the sort's inputs.
	ErrAlreadyFinalized = errors.New("sorter already finalized")

	// ErrAlreadyRead is returned from Interface#Read when Interface#Read has
	// already been called.
	ErrAlreadyRead = errors.New("sorter already read")
)

// Add implements part of the Interface interface.
func (m *mergeSorter) Add(i interface{}) error {
	if m.buffer == nil {
		return ErrAlreadyFinalized
	}

	m.buffer.Push(i)
	if m.buffer.Len() > m.opts.MaxInMemory {
		return m.dumpShard()
	}
	return nil
}

// Read implements part of the Interface interface.
func (m *mergeSorter) Read(f func(i interface{}) error) (err error) {
	if m.buffer == nil {
		return ErrAlreadyRead
	}

	if len(m.shards) == 0 {
		// Fast path for a single, in-memory shard
		defer func() {
			m.buffer = nil // signal that further operations should fail
		}()
		sort.Sort(m.buffer)
		for _, x := range m.buffer.Slice {
			if err := f(x); err != nil {
				return err
			}
		}
		return nil
	}

	// Ensure that the working directory is always cleaned up.
	defer func() {
		cleanupErr := os.RemoveAll(m.workDir)
		if err == nil {
			err = cleanupErr
		}
	}()

	if m.buffer.Len() != 0 {
		// To make the merging algorithm simpler, dump the last shard to disk.
		if err := m.dumpShard(); err != nil {
			return fmt.Errorf("error dumping final shard: %v", err)
		}
	}
	m.buffer = nil // signal that further operations should fail

	// This is a heap storing the head of each shard.
	merger := &sortutil.ByLesser{
		Lesser: &mergeElementLesser{Lesser: m.opts.Lesser},
	}

	defer func() {
		// Try to cleanup on errors
		for merger.Len() != 0 {
			x := heap.Pop(merger).(*mergeElement)
			_ = x.f.Close() // ignore errors (file is only open for reading)
		}
	}()

	// Initialize the merger heap by reading the first element of each shard.
	for _, shard := range m.shards {
		f, err := os.Open(shard)
		if err != nil {
			return fmt.Errorf("error opening shard %q: %v", shard, err)
		}
		var r io.Reader = f
		if m.opts.CompressShards {
			r = snappy.NewReader(r)
		}
		rd := delimited.NewReader(bufio.NewReaderSize(r, m.opts.IOBufferSize))
		first, err := rd.Next()
		if err != nil {
			f.Close()
			return fmt.Errorf("error reading beginning of shard %q: %v", shard, err)
		}
		el, err := m.opts.Marshaler.Unmarshal(first)
		if err != nil {
			f.Close()
			return fmt.Errorf("error unmarshaling beginning of shard %q: %v", shard, err)
		}
		heap.Push(merger, &mergeElement{el: el, rd: rd, f: f})
	}

	// While the merger heap is non-empty:
	//   el := pop the head of the heap
	//   pass it to the user-specific function
	//   push the next element el.rd to the merger heap
	for merger.Len() != 0 {
		x := heap.Pop(merger).(*mergeElement)

		// Give the value to the user-supplied function
		if err := f(x.el); err != nil {
			return err
		}

		// Read and parse the next value on the same shard
		rec, err := x.rd.Next()
		if err != nil {
			_ = x.f.Close() // ignore errors (file is only open for reading)
			if err == io.EOF {
				continue
			} else {
				return fmt.Errorf("error reading shard: %v", err)
			}
		}
		next, err := m.opts.Marshaler.Unmarshal(rec)
		if err != nil {
			return fmt.Errorf("error unmarshaling element: %v", err)
		}

		// Reuse mergeElement, push it back onto the merger heap with the next value
		x.el = next
		heap.Push(merger, x)
	}

	return nil
}

func (m *mergeSorter) dumpShard() (err error) {
	defer m.buffer.Clear()

	// Create a new shard file
	shardPath := filepath.Join(m.workDir, fmt.Sprintf("shard.%.6d", len(m.shards)))
	file, err := os.OpenFile(shardPath, os.O_WRONLY|os.O_CREATE, 0666)
	if err != nil {
		return fmt.Errorf("error creating shard: %v", err)
	}
	defer func() {
		replaceErrIfNil(&err, "error closing shard: %v", file.Close())
	}()

	var w io.Writer = file
	if m.opts.CompressShards {
		w = snappy.NewWriter(w)
	}

	// Buffer writing to the shard
	buf := bufio.NewWriterSize(w, m.opts.IOBufferSize)
	defer func() {
		replaceErrIfNil(&err, "error flushing shard: %v", buf.Flush())
	}()

	// Sort the in-memory buffer of elements
	sort.Sort(m.buffer)

	// Write each element of the in-memory to shard file, in sorted order
	wr := delimited.NewWriter(buf)
	for _, x := range m.buffer.Slice {
		rec, err := m.opts.Marshaler.Marshal(x)
		if err != nil {
			return fmt.Errorf("marshaling error: %v", err)
		}
		if _, err := wr.Write(rec); err != nil {
			return fmt.Errorf("writing error: %v", err)
		}
	}

	m.shards = append(m.shards, shardPath)
	return nil
}

func replaceErrIfNil(err *error, s string, newError error) {
	if newError != nil && *err == nil {
		*err = fmt.Errorf(s, newError)
	}
}

type mergeElement struct {
	el interface{}
	rd delimited.Reader
	f  *os.File
}

type mergeElementLesser struct{ sortutil.Lesser }

// Less implements the sortutil.Lesser interface.
func (m *mergeElementLesser) Less(a, b interface{}) bool {
	x, y := a.(*mergeElement), b.(*mergeElement)
	return m.Lesser.Less(x.el, y.el)
}
