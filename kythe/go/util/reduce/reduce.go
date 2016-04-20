/*
 * Copyright 2016 Google Inc. All rights reserved.
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

// Package reduce provides a simple interface for transforming a stream of
// inputs.  Currently, other than utility functions/interfaces, the only
// transformation available is Sort.
package reduce

import (
	"errors"
	"fmt"
	"io"
	"sync"

	"kythe.io/kythe/go/util/disksort"

	"github.com/golang/protobuf/proto"
	"golang.org/x/net/context"

	ipb "kythe.io/kythe/proto/internal_proto"
)

// A Reducer transforms one stream of values into another stream of values.
// Reduce may be called multiple times (once per input shard).  Start and End
// will be called once before any call to Reduce and once after every call to
// Reduce, respectively.
type Reducer interface {
	Start(context.Context) error
	Reduce(context.Context, IO) error
	End(context.Context) error
}

// A Func implements the Reducer interface with no-op Start/End methods.
type Func func(context.Context, IO) error

// Start implements part of the Reducer interface.
func (r Func) Start(_ context.Context) error { return nil }

// End implements part of the Reducer interface.
func (r Func) End(_ context.Context) error { return nil }

// Reduce implements part of the Reducer interface.
func (r Func) Reduce(ctx context.Context, io IO) error { return r(ctx, io) }

// A Input yields input to a Reducer.
type Input interface {
	Next() (interface{}, error)
}

// A Output allows a Reducer to emit values.
type Output interface {
	Emit(context.Context, interface{}) error
}

// IO composes Input and Output.
type IO interface {
	Input
	Output
}

// An OutFunc implements the Output interface.
type OutFunc func(context.Context, interface{}) error

// Emit implements the Output interface.
func (o OutFunc) Emit(ctx context.Context, i interface{}) error { return o(ctx, i) }

// An InFunc implements the Input interface.
type InFunc func(context.Context) (interface{}, error)

// Next implements the Input interface.
func (f InFunc) Next(ctx context.Context) (interface{}, error) { return f(ctx) }

// ChannelInput implements the Input interface, yielding each channel
// value when Next is called.
type ChannelInput <-chan ChannelInputValue

// ChannelInputValue represents the return value of Input#Next.
type ChannelInputValue struct {
	Value interface{}
	Err   error
}

// Next implements the Input interface.
func (c ChannelInput) Next() (interface{}, error) {
	v, ok := <-c
	if !ok {
		return nil, io.EOF
	}
	if v.Err != nil {
		return nil, v.Err
	}
	return v.Value, nil
}

// IOStruct reifies the IO interface.
type IOStruct struct {
	Input
	Output
}

var errEndSplit = errors.New("END OF SPLIT")

// SplitInput represents an input that is sharded.
type SplitInput interface {
	NextSplit() (Input, error)
}

// Sort applies r to each separate input in splits.  r should be a Reducer that accepts the
// same input type that splits contains and MUST output *ipb.SortedKeyValues.  The resulting
// SplitInput will be the set of outputs from r, split on groups sharing the same
// SortedKeyValue.Key and sorted within a group by SortedKeyValue.SortKey (see
// SplitSortedKeyValues).  The Reducer's Start method will be called once before any call to Reduce
// and the Reducer's End method will be called once after the final Reduce call is completed.
//
// Sort will return on the first error it encounters.  This may cause some of the input to not
// be read and Start/Reduce/End may not be called depending on when the error occurs.
func Sort(ctx context.Context, splits SplitInput, r Reducer) (SplitInput, error) {
	sorter, err := KeyValueSorter()
	if err != nil {
		return nil, err
	}

	if err := r.Start(ctx); err != nil {
		return nil, err
	}

	addToSorter := OutFunc(func(_ context.Context, i interface{}) error {
		_, ok := i.(*ipb.SortedKeyValue)
		if !ok {
			return fmt.Errorf("given non-SortedKeyValue: %T", i)
		}
		return sorter.Add(i)
	})

	for {
		if err := func() error {
			in, err := splits.NextSplit()
			if err == io.EOF {
				return errEndSplit
			} else if err != nil {
				return err
			} else if in == nil {
				return errors.New("received nil Input from SplitInput")
			}
			defer func() { // drain input if not read by Reducer
				var err error
				for err == nil {
					_, err = in.Next()
				}
			}()

			return r.Reduce(ctx, IOStruct{in, addToSorter})
		}(); err == errEndSplit {
			break
		} else if err != nil {
			return nil, err
		}
	}

	if err := r.End(ctx); err != nil {
		return nil, err
	}

	return SplitSortedKeyValues(sorter)
}

// ChannelSplitInput implements the SplitInput interface over a channel of
// ReduceInputs to return on each call to NextSplit.
type ChannelSplitInput <-chan ChannelSplitInputValue

// NextSplit implements the SplitInput interface.
func (c ChannelSplitInput) NextSplit() (Input, error) {
	ri, ok := <-c
	if !ok {
		return nil, io.EOF
	}
	if ri.Err != nil {
		return nil, ri.Err
	}
	return ri.Input, nil
}

// ChannelSplitInputValue represents the return values of
// SplitInput#NextSplit.
type ChannelSplitInputValue struct {
	Input Input
	Err   error
}

// KeyValueSorter returns a disksort for arbitrary *ipb.SortedKeyValues.
func KeyValueSorter() (disksort.Interface, error) {
	return disksort.NewMergeSorter(disksort.MergeOptions{
		Lesser:    keyValueSortUtil{},
		Marshaler: keyValueSortUtil{},
	})
}

// SplitSortedKeyValues constructs a SplitInput that returns a Input
// for each set of *ipb.SortedKeyValues with the same key.
func SplitSortedKeyValues(sorter disksort.Interface) (SplitInput, error) {
	splitCh := make(chan ChannelSplitInputValue)

	go func() {
		defer close(splitCh)

		var (
			curKey string
			curCh  chan ChannelInputValue
		)

		if err := sorter.Read(func(i interface{}) error {
			kv := i.(*ipb.SortedKeyValue)
			if curCh != nil && curKey != kv.Key {
				close(curCh)
				curKey, curCh = "", nil
			}
			if curCh == nil {
				curKey = kv.Key
				curCh = make(chan ChannelInputValue)
				splitCh <- ChannelSplitInputValue{Input: ChannelInput(curCh)}
			}
			curCh <- ChannelInputValue{Value: kv}
			return nil
		}); err != nil {
			splitCh <- ChannelSplitInputValue{Err: err}
		}
		if curCh != nil {
			close(curCh)
		}
	}()

	return ChannelSplitInput(splitCh), nil
}

// CombinedReducers allows multiple Reducers to consume the same ReducerInput
// and output to the same ReducerOutput.
type CombinedReducers struct {
	Reducers []Reducer
}

// Start implements part of the Reducer interface.
func (c *CombinedReducers) Start(ctx context.Context) error {
	for _, r := range c.Reducers {
		if err := r.Start(ctx); err != nil {
			return err
		}
	}
	return nil
}

// End implements part of the Reducer interface.
func (c *CombinedReducers) End(ctx context.Context) error {
	for _, r := range c.Reducers {
		if err := r.Start(ctx); err != nil {
			return err
		}
	}
	return nil
}

// Reduce implements part of the Reducer interface.
func (c *CombinedReducers) Reduce(ctx context.Context, rio IO) error {
	chs := make([]chan ChannelInputValue, len(c.Reducers))
	errs := make([]error, len(c.Reducers))
	var wg sync.WaitGroup
	wg.Add(len(c.Reducers))
	for i := range c.Reducers {
		chs[i] = make(chan ChannelInputValue, 10)
		go func(i int) {
			defer wg.Done()
			errs[i] = c.Reducers[i].Reduce(ctx, &IOStruct{
				Input:  ChannelInput(chs[i]),
				Output: rio,
			})

			// Drain channel if Reducer hasn't already
			for range chs[i] {
			}
		}(i)
	}

	for {
		i, err := rio.Next()
		if err == io.EOF {
			break
		} else if err != nil {
			for _, ch := range chs {
				ch <- ChannelInputValue{Err: err}
			}
			break
		}

		for _, ch := range chs {
			ch <- ChannelInputValue{Value: i}
		}
	}

	for _, ch := range chs {
		close(ch)
	}
	wg.Wait()

	for _, err := range errs {
		if err != nil {
			return err
		}
	}
	return nil
}

type keyValueSortUtil struct{}

func (keyValueSortUtil) Less(a, b interface{}) bool {
	x, y := a.(*ipb.SortedKeyValue), b.(*ipb.SortedKeyValue)
	if x.Key == y.Key {
		return x.SortKey < y.SortKey
	}
	return x.Key < y.Key
}

func (keyValueSortUtil) Marshal(x interface{}) ([]byte, error) {
	return proto.Marshal(x.(proto.Message))
}

func (keyValueSortUtil) Unmarshal(rec []byte) (interface{}, error) {
	var kv ipb.SortedKeyValue
	return &kv, proto.Unmarshal(rec, &kv)
}
