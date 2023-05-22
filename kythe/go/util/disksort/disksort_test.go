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

package disksort

import (
	"fmt"
	"math/rand"
	"strconv"
	"testing"
)

type numLesser struct{}

// Less implements the Lesser interface.
func (numLesser) Less(a, b any) bool { return a.(int) < b.(int) }

type numMarshaler struct{}

// Marshal implements part of the Marshaler interface.
func (numMarshaler) Marshal(x any) ([]byte, error) {
	return []byte(strconv.Itoa(x.(int))), nil
}

// Unmarshal implements part of the Marshaler interface.
func (numMarshaler) Unmarshal(rec []byte) (any, error) {
	return strconv.Atoi(string(rec))
}

func TestMergeSorter(t *testing.T) {
	// Sort 1M numbers in chunks of 750 (~1.3k shards)
	const n = 1000000
	const max = 750

	rand.Seed(120875)

	sorter, err := NewMergeSorter(MergeOptions{
		Lesser:      numLesser{},
		Marshaler:   numMarshaler{},
		MaxInMemory: max,
	})
	if err != nil {
		t.Fatalf("error creating MergeSorter: %v", err)
	}

	nums := make([]int, n)
	for i := 0; i < n; i++ {
		nums[i] = i
	}

	// Randomize order for Add
	for i := 0; i < n; i++ {
		o := rand.Int() % n
		nums[i], nums[o] = nums[o], nums[i]
	}

	for _, n := range nums {
		if err := sorter.Add(n); err != nil {
			t.Fatalf("error adding %d to sorter: %v", n, err)
		}
	}

	var expected int
	if err := sorter.Read(func(i any) error {
		x, ok := i.(int)
		if !ok {
			return fmt.Errorf("expected int; found %T", i)
		} else if expected != x {
			return fmt.Errorf("expected %d; found %d", expected, x)
		}
		expected++
		return nil
	}); err != nil {
		t.Fatalf("read error: %v", err)
	}

	if expected != n {
		t.Fatalf("Expected %d total; found %d", n, expected)
	}
}
