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

package reduce

import (
	"context"
	"errors"
	"fmt"
	"io"

	"kythe.io/kythe/go/util/disksort"

	"google.golang.org/protobuf/proto"

	ipb "kythe.io/kythe/proto/internal_go_proto"
)

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
	iter, err := sorter.Iterator()
	if err != nil {
		return nil, err
	}
	return &sortSplitInput{iter: iter}, nil
}

var errEndSplit = errors.New("END OF SPLIT")

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

	addToSorter := OutFunc(func(_ context.Context, i any) error {
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

type sortSplitInput struct {
	iter disksort.Iterator

	curKey     string
	first      *ipb.SortedKeyValue
	endOfSplit bool
}

func (s *sortSplitInput) Next() (any, error) {
	if s.endOfSplit {
		return nil, io.EOF
	} else if s.first != nil {
		kv := s.first
		s.curKey, s.first = kv.Key, nil
		return kv, nil
	}
	v, err := s.iter.Next()
	if err != nil {
		s.iter.Close()
		return nil, err
	}
	kv := v.(*ipb.SortedKeyValue)
	if kv.Key != s.curKey {
		s.endOfSplit = true
		s.first = kv
		return nil, io.EOF
	}
	return v, nil
}

func (s *sortSplitInput) NextSplit() (Input, error) {
	s.endOfSplit = false
	if s.first != nil {
		return s, nil
	}
	v, err := s.iter.Next()
	if err != nil {
		s.iter.Close()
		return nil, err
	}
	s.first = v.(*ipb.SortedKeyValue)
	return s, nil
}

type keyValueSortUtil struct{}

func (keyValueSortUtil) Less(a, b any) bool {
	x, y := a.(*ipb.SortedKeyValue), b.(*ipb.SortedKeyValue)
	if x.Key == y.Key {
		return x.SortKey < y.SortKey
	}
	return x.Key < y.Key
}

func (keyValueSortUtil) Marshal(x any) ([]byte, error) {
	return proto.Marshal(x.(proto.Message))
}

func (keyValueSortUtil) Unmarshal(rec []byte) (any, error) {
	var kv ipb.SortedKeyValue
	return &kv, proto.Unmarshal(rec, &kv)
}
