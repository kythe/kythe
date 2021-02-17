// Licensed to the Apache Software Foundation (ASF) under one or more
// contributor license agreements.  See the NOTICE file distributed with
// this work for additional information regarding copyright ownership.
// The ASF licenses this file to You under the Apache License, Version 2.0
// (the "License"); you may not use this file except in compliance with
// the License.  You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

package disksort

import (
	"context"
	"testing"

	"github.com/apache/beam/sdks/go/pkg/beam"
	"github.com/apache/beam/sdks/go/pkg/beam/testing/passert"
	"github.com/apache/beam/sdks/go/pkg/beam/testing/ptest"
	"github.com/apache/beam/sdks/go/pkg/beam/x/debug"
)

func TestGBK(t *testing.T) {
	p, s, nums := ptest.CreateList([]int{1, 2, 3, 4, 2, 3, 3})

	kv := beam.ParDo(s, extendToKey, nums)
	sums := beam.DropKey(s, beam.ParDo(s, sum, beam.GroupByKey(s, kv)))
	debug.Print(s, sums)

	expected := []int{1, 4, 9, 4}
	passert.Equals(s, sums, beam.CreateList(s, expected))

	if _, err := Execute(context.Background(), p); err != nil {
		t.Fatal(err)
	}
}

func extendToKey(v beam.T) (beam.T, beam.T) { return v, v }

func sum(key beam.T, els func(*int) bool) (beam.T, int) {
	var sum, n int
	for els(&n) {
		sum += n
	}
	return key, sum
}

func TestCoGBK(t *testing.T) {
	p, s, nums := ptest.CreateList([]int{1, 2, 3, 4})

	twos := beam.ParDo(s, &multBy{2}, nums)
	threes := beam.ParDo(s, &multBy{3}, nums)

	sums := beam.DropKey(s, beam.ParDo(s, sum2, beam.CoGroupByKey(s, twos, threes)))
	debug.Print(s, sums)

	expected := []int{5, 10, 15, 20}
	passert.Equals(s, sums, beam.CreateList(s, expected))

	if _, err := Execute(context.Background(), p); err != nil {
		t.Fatal(err)
	}
}

type multBy struct{ X int }

func (m *multBy) ProcessElement(x int) (int, int) { return x, m.X * x }

func sum2(key beam.T, left func(*int) bool, right func(*int) bool) (beam.T, int) {
	var sum, n int
	for left(&n) {
		sum += n
	}
	for right(&n) {
		sum += n
	}
	return key, sum
}

func TestCombinePerKey(t *testing.T) {
	p, s, nums := ptest.CreateList([]int{1, 2, 3, 4, 2, 3, 3})

	kv := beam.ParDo(s, extendToKey, nums)
	sums := beam.DropKey(s, beam.CombinePerKey(s, &sumCombine{}, kv))
	debug.Print(s, sums)

	expected := []int{1, 4, 9, 4}
	passert.Equals(s, sums, beam.CreateList(s, expected))

	if _, err := Execute(context.Background(), p); err != nil {
		t.Fatal(err)
	}
}

type sumCombine struct{}

func (sumCombine) MergeAccumulators(x, y int) int { return x + y }

func TestCombine(t *testing.T) {
	p, s, nums := ptest.CreateList([]int{1, 2, 3, 4})

	sum := beam.Combine(s, &sumCombine{}, nums)
	debug.Print(s, sum)

	expected := []int{10}
	passert.Equals(s, sum, beam.CreateList(s, expected))

	if _, err := Execute(context.Background(), p); err != nil {
		t.Fatal(err)
	}
}
