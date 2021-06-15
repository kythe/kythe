/*
 * Copyright 2021 The Kythe Authors. All rights reserved.
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

package beamio

import (
	"testing"

	"github.com/apache/beam/sdks/go/pkg/beam"
	"github.com/apache/beam/sdks/go/pkg/beam/testing/passert"
	"github.com/apache/beam/sdks/go/pkg/beam/testing/ptest"
	"github.com/apache/beam/sdks/go/pkg/beam/transforms/stats"
)

type shardValue struct {
	Num    int
	Values []int
}

type collectShards struct{}

func (c collectShards) CreateAccumulator() []int { return []int{} }
func (c collectShards) AddInput(accum []int, input KeyValue) []int {
	return append(accum, int(input.Key[0]))
}
func (c collectShards) MergeAccumulators(accum, other []int) []int {
	return append(accum, other...)
}
func makeShardValue(shard int, accum []int) shardValue {
	return shardValue{Num: shard, Values: accum}
}

func TestComputeShards(t *testing.T) {
	const testElements = 8
	kvs := make([]KeyValue, 0, testElements)
	for i := 0; i < testElements; i++ {
		kvs = append(kvs, KeyValue{
			Key:   []byte{byte(i)},
			Value: []byte{},
		})
	}
	expected := []shardValue{
		{Num: 0, Values: []int{0, 1}},
		{Num: 1, Values: []int{2, 3, 4}},
		{Num: 2, Values: []int{5, 6, 7}},
	}
	p, s, col, pExpected := ptest.CreateList2(kvs, expected)
	shardedElementsKv := ComputeShards(s, col, stats.Opts{K: 10, NumQuantiles: 3})
	collectedShards := beam.ParDo(s, makeShardValue, beam.CombinePerKey(s, collectShards{}, shardedElementsKv))

	passert.Equals(s, pExpected, collectedShards)
	if err := ptest.Run(p); err != nil {
		t.Errorf("ComputeShards failed: %v", err)
	}
}
