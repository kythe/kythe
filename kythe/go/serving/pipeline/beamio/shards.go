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
	"bytes"
	"reflect"
	"sort"

	"github.com/apache/beam/sdks/go/pkg/beam"
	"github.com/apache/beam/sdks/go/pkg/beam/core/util/reflectx"
	"github.com/apache/beam/sdks/go/pkg/beam/transforms/stats"
)

func init() {
	beam.RegisterFunction(computeShard)
	beam.RegisterFunction(assignKeyWeights)
	beam.RegisterType(reflect.TypeOf((*computeMinKvSize)(nil)).Elem())
	reflectx.RegisterFunc(reflect.ValueOf(bytesLessFn).Type(), func(_ any) reflectx.Func {
		return newBytesLess()
	})
}

// Not used, needed for the signature.
func bytesLessFn(a, b []byte) bool {
	return bytes.Compare(a, b) < 0
}

type bytesLess struct {
	name string
	t    reflect.Type
}

func newBytesLess() *bytesLess {
	return &bytesLess{
		name: reflectx.FunctionName(reflect.ValueOf(bytesLessFn).Interface()),
		t:    reflect.ValueOf(bytesLessFn).Type(),
	}
}

func (i *bytesLess) Name() string {
	return i.name
}
func (i *bytesLess) Type() reflect.Type {
	return i.t
}
func (i *bytesLess) Call(args []any) []any {
	return []any{bytesLessFn(args[0].([]byte), args[1].([]byte))}
}

// Input is PCollection of beamio.KeyValue<[]byte, []byte>, output is PCollection of *ppb.KeyWeights.
func computeAndAssignKeyWeights(s beam.Scope, kv beam.PCollection, shards int) beam.PCollection {
	minKvSize := beam.Combine(s, computeMinKvSize{}, kv)
	return beam.ParDo(s, assignKeyWeights, kv, beam.SideInput{Input: minKvSize})
}

// ComputeShards assigns shards to KeyValues. Input is a PCollection of beamio.KeyValue, output is (int, beamio.KeyValue).
func ComputeShards(s beam.Scope, kv beam.PCollection, opts stats.Opts) beam.PCollection {
	// Allows us to "weight" keys by how big their values are. This helps us estimate quantiles with similar sizes of data
	weightedKeys := computeAndAssignKeyWeights(s, kv, opts.NumQuantiles)
	ssp := stats.ApproximateWeightedQuantiles(s, weightedKeys, bytesLessFn, opts)
	return beam.ParDo(s, computeShard, kv, beam.SideInput{Input: ssp})
}

type computeMinKvSize struct{}

func (computeMinKvSize) AddInput(accum int, input KeyValue) int {
	inputSize := len(input.Value) + len(input.Key)
	if accum < inputSize && accum != 0 {
		return accum
	}
	return inputSize
}

func (computeMinKvSize) MergeAccumulators(accum, other int) int {
	if accum < other {
		return accum
	}
	return other
}

func computeShard(kv KeyValue, ssp [][]byte) (int, KeyValue) {
	shard := sort.Search(len(ssp), func(i int) bool { return bytes.Compare(kv.Key, ssp[i]) < 0 })
	return shard, kv
}

func assignKeyWeights(kv KeyValue, minKvSize int) (int, []byte) {
	kvSize := len(kv.Key) + len(kv.Value)
	weight := kvSize / minKvSize
	return weight, kv.Key
}
