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

// Package disksort implements sorting algorithms for sets of data too large to
// fit fully in-memory.  If the number of elements becomes to large, data are
// paged onto the disk.
package disksort // import "kythe.io/kythe/go/util/disksort"

import (
	"bufio"
	"container/heap"
	"errors"
	"fmt"
	"io"
	"io/ioutil"
	"os"
	"path/filepath"
	"strings"

	"kythe.io/kythe/go/platform/delimited"
	"kythe.io/kythe/go/util/log"
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
	Add(i any) error

	// Iterator returns an Iterator to read each of the elements previously added
	// to the set of data to be sorted.  Once Iterator is called, no more data may
	// be added to the sorter.  This is a lower-level version of Read.  Iterator
	// and Read may only be called once.
	Iterator() (Iterator, error)

	// Read calls f on each elements previously added the set of data to be
	// sorted.  If f returns an error, it is returned immediately and f is no
	// longer called.  Once Read is called, no more data may be added to the
	// sorter.  Iterator and Read may only be called once.
	Read(f func(any) error) error
}

// An Iterator reads each element, in order, in a sorted dataset.
type Iterator interface {
	// Next returns the next ordered element.  If none exist, an io.EOF error is
	// returned.
	Next() (any, error)

	// Close releases all of the Iterator's used resources.  Each Iterator must be
	// closed after the client's last call to Next or stray temporary files may be
	// left on disk.
	Close() error
}

// Marshaler is an interface to functions that can binary encode/decode
// elements.
type Marshaler interface {
	// Marshal binary encodes the given element.
	Marshal(any) ([]byte, error)

	// Unmarshal decodes the given encoding of an element.
	Unmarshal([]byte) (any, error)
}

type mergeSorter struct {
	opts MergeOptions

	buffer  []any
	workDir string
	shards  []string

	bufferSize int

	finalized bool
}

// DefaultMaxInMemory is the default number of elements to keep in-memory during
// a merge sort.
const DefaultMaxInMemory = 32000

// DefaultMaxBytesInMemory is the default maximum total size of elements to keep
// in-memory during a merge sort.
const DefaultMaxBytesInMemory = 1024 * 1024 * 256

// MergeOptions specifies how to sort elements.
type MergeOptions struct {
	// Name is optionally used as part of the path for temporary file shards.
	Name string

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

	// MaxBytesInMemory is the maximum total size of elements to keep in-memory
	// before paging them to a temporary file shard.  An element's size is
	// determined by its `Size() int` method. If non-positive,
	// DefaultMaxBytesInMemory is used.
	MaxBytesInMemory int

	// CompressShards determines whether the temporary file shards should be
	// compressed.
	CompressShards bool
}

type sizer interface{ Size() int }

// NewMergeSorter returns a new disk sorter using a mergesort algorithm.
func NewMergeSorter(opts MergeOptions) (Interface, error) {
	if opts.Lesser == nil {
		return nil, errors.New("missing Lesser")
	} else if opts.Marshaler == nil {
		return nil, errors.New("missing Marshaler")
	}

	name := strings.Replace(opts.Name, string(filepath.Separator), ".", -1)
	if name == "" {
		name = "external.merge.sort"
	}
	dir, err := ioutil.TempDir(opts.WorkDir, name)
	if err != nil {
		return nil, fmt.Errorf("error creating temporary work directory: %v", err)
	}

	if opts.MaxInMemory <= 0 {
		opts.MaxInMemory = DefaultMaxInMemory
	}
	if opts.MaxBytesInMemory <= 0 {
		opts.MaxBytesInMemory = DefaultMaxBytesInMemory
	}

	return &mergeSorter{
		opts:    opts,
		buffer:  make([]any, 0, opts.MaxInMemory),
		workDir: dir,
	}, nil
}

var (
	// ErrAlreadyFinalized is returned from Interface#Add and Interface#Read when
	// Interface#Read has already been called, freezing the sort's inputs/outputs.
	ErrAlreadyFinalized = errors.New("sorter already finalized")
)

// Add implements part of the Interface interface.
func (m *mergeSorter) Add(i any) error {
	if m.finalized {
		return ErrAlreadyFinalized
	}

	m.buffer = append(m.buffer, i)
	if sizer, ok := i.(sizer); ok {
		m.bufferSize += sizer.Size()
	}

	if len(m.buffer) >= m.opts.MaxInMemory || m.bufferSize >= m.opts.MaxBytesInMemory {
		return m.dumpShard()
	}
	return nil
}

type mergeIterator struct {
	buffer []any

	merger    *sortutil.ByLesser
	marshaler Marshaler
	workDir   string
}

const ioBufferSize = 2 << 15

// Iterator implements part of the Interface interface.
func (m *mergeSorter) Iterator() (iter Iterator, err error) {
	if m.finalized {
		return nil, ErrAlreadyFinalized
	}
	m.finalized = true // signal that further operations should fail

	it := &mergeIterator{workDir: m.workDir, marshaler: m.opts.Marshaler}

	if len(m.shards) == 0 {
		// Fast path for a single, in-memory shard
		it.buffer, m.buffer = m.buffer, nil
		sortutil.Sort(m.opts.Lesser, it.buffer)
		return it, nil
	}

	// This is a heap storing the head of each shard.
	merger := &sortutil.ByLesser{
		Lesser: &mergeElementLesser{Lesser: m.opts.Lesser},
	}
	it.merger = merger

	defer func() {
		// Try to cleanup on errors
		if err != nil {
			if cErr := it.Close(); cErr != nil {
				log.Warningf("error closing Iterator after error: %v", cErr)
			}
		}
	}()

	// Push all of the in-memory elements into the merger heap.
	for _, el := range m.buffer {
		heap.Push(merger, &mergeElement{el: el})
	}
	m.buffer = nil

	// Initialize the merger heap by reading the first element of each shard.
	for _, shard := range m.shards {
		f, err := os.OpenFile(shard, os.O_RDONLY, shardFileMode)
		if err != nil {
			return nil, fmt.Errorf("error opening shard %q: %v", shard, err)
		}

		var r io.Reader
		if m.opts.CompressShards {
			r = snappy.NewReader(f)
		} else {
			r = bufio.NewReaderSize(f, ioBufferSize)
		}

		rd := delimited.NewReader(r)
		first, err := rd.Next()
		if err != nil {
			f.Close()
			return nil, fmt.Errorf("error reading beginning of shard %q: %v", shard, err)
		}
		el, err := m.opts.Marshaler.Unmarshal(first)
		if err != nil {
			f.Close()
			return nil, fmt.Errorf("error unmarshaling beginning of shard %q: %v", shard, err)
		}

		heap.Push(merger, &mergeElement{el: el, rd: rd, f: f})
	}

	return it, nil
}

// Next implements part of the Iterator interface.
func (i *mergeIterator) Next() (any, error) {
	if i.merger == nil {
		// Fast path for a single, in-memory shard
		if len(i.buffer) == 0 {
			return nil, io.EOF
		}
		val := i.buffer[0]
		i.buffer = i.buffer[1:]
		return val, nil
	}

	if i.merger.Len() == 0 {
		return nil, io.EOF
	}

	// While the merger heap is non-empty:
	//   x := peek the head of the heap
	//   pass x.el to the user-specific function
	//   read the next element in x.rd; fix the merger heap order
	x := i.merger.Slice[0].(*mergeElement)
	el := x.el

	if x.rd == nil {
		heap.Pop(i.merger)
	} else {
		// Read and parse the next value on the same shard
		rec, err := x.rd.Next()
		if err != nil {
			_ = x.f.Close()           // ignore errors (file is only open for reading)
			_ = os.Remove(x.f.Name()) // ignore errors (os.RemoveAll used in Close)
			heap.Pop(i.merger)
			if err != io.EOF {
				return nil, fmt.Errorf("error reading shard: %v", err)
			}
		} else {
			next, err := i.marshaler.Unmarshal(rec)
			if err != nil {
				return nil, fmt.Errorf("error unmarshaling element: %v", err)
			}

			// Reuse mergeElement, reorder it in the merger heap with the next value
			x.el = next
			heap.Fix(i.merger, 0)
		}
	}

	return el, nil
}

// Close implements part of the Iterator interface.
func (i *mergeIterator) Close() error {
	i.buffer = nil
	if i.merger != nil {
		for _, x := range i.merger.Slice {
			el := x.(*mergeElement)
			if el.f != nil {
				el.f.Close() // ignore errors (file is only open for reading)
			}
		}
		i.merger = nil
	}
	if rmErr := os.RemoveAll(i.workDir); rmErr != nil {
		return fmt.Errorf("error removing temporary directory %q: %v", i.workDir, rmErr)
	}
	return nil
}

// Read implements part of the Interface interface.
func (m *mergeSorter) Read(f func(i any) error) (err error) {
	it, err := m.Iterator()
	if err != nil {
		return err
	}
	defer func() {
		if cErr := it.Close(); cErr != nil {
			if err == nil {
				err = cErr
			} else {
				log.Warningf("error closing Iterator: %v", cErr)
			}
		}
	}()
	for {
		val, err := it.Next()
		if err == io.EOF {
			return nil
		} else if err != nil {
			return err
		}
		if err := f(val); err != nil {
			return err
		}
	}
}

const shardFileMode = 0600 | os.ModeExclusive | os.ModeAppend | os.ModeTemporary | os.ModeSticky

func (m *mergeSorter) dumpShard() (err error) {
	defer func() {
		m.buffer = make([]any, 0, m.opts.MaxInMemory)
		m.bufferSize = 0
	}()

	// Create a new shard file
	shardPath := filepath.Join(m.workDir, fmt.Sprintf("shard.%.6d", len(m.shards)))
	file, err := os.OpenFile(shardPath, os.O_WRONLY|os.O_CREATE|os.O_EXCL, shardFileMode)
	if err != nil {
		return fmt.Errorf("error creating shard: %v", err)
	}
	defer func() {
		replaceErrIfNil(&err, "error closing shard: %v", file.Close())
	}()

	// Buffer writing to the shard
	var buf interface {
		io.Writer
		Flush() error
	}
	if m.opts.CompressShards {
		buf = snappy.NewBufferedWriter(file)
	} else {
		buf = bufio.NewWriterSize(file, ioBufferSize)
	}

	defer func() {
		replaceErrIfNil(&err, "error flushing shard: %v", buf.Flush())
	}()

	// Sort the in-memory buffer of elements
	sortutil.Sort(m.opts.Lesser, m.buffer)

	// Write each element of the in-memory to shard file, in sorted order
	wr := delimited.NewWriter(buf)
	for len(m.buffer) > 0 {
		rec, err := m.opts.Marshaler.Marshal(m.buffer[0])
		if err != nil {
			return fmt.Errorf("marshaling error: %v", err)
		}
		if _, err := wr.WriteRecord(rec); err != nil {
			return fmt.Errorf("writing error: %v", err)
		}
		m.buffer = m.buffer[1:]
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
	el any
	rd *delimited.Reader
	f  *os.File
}

type mergeElementLesser struct{ sortutil.Lesser }

// Less implements the sortutil.Lesser interface.
func (m *mergeElementLesser) Less(a, b any) bool {
	x, y := a.(*mergeElement), b.(*mergeElement)
	return m.Lesser.Less(x.el, y.el)
}
