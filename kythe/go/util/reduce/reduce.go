/*
 * Copyright 2016 The Kythe Authors. All rights reserved.
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
package reduce // import "kythe.io/kythe/go/util/reduce"

import (
	"context"
	"io"
	"sync"
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
	Next() (any, error)
}

// A Output allows a Reducer to emit values.
type Output interface {
	Emit(context.Context, any) error
}

// IO composes Input and Output.
type IO interface {
	Input
	Output
}

// An OutFunc implements the Output interface.
type OutFunc func(context.Context, any) error

// Emit implements the Output interface.
func (o OutFunc) Emit(ctx context.Context, i any) error { return o(ctx, i) }

// An InFunc implements the Input interface.
type InFunc func(context.Context) (any, error)

// Next implements the Input interface.
func (f InFunc) Next(ctx context.Context) (any, error) { return f(ctx) }

// ChannelInput implements the Input interface, yielding each channel
// value when Next is called.
type ChannelInput <-chan ChannelInputValue

// ChannelInputValue represents the return value of Input#Next.
type ChannelInputValue struct {
	Value any
	Err   error
}

// Next implements the Input interface.
func (c ChannelInput) Next() (any, error) {
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

// SplitInput represents an input that is sharded.
type SplitInput interface {
	NextSplit() (Input, error)
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
