/*
 * Copyright 2014 Google Inc. All rights reserved.
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

package leveldb

import (
	"fmt"
	"io/ioutil"
	"os"
	"testing"

	"kythe.io/kythe/go/test/services/graphstore"
	"kythe.io/kythe/go/test/storage/keyvalue"
)

const (
	smallBatchSize  = 4
	mediumBatchSize = 16
	largeBatchSize  = 64
)

func tempDB() (keyvalue.DB, keyvalue.DestroyFunc, error) {
	path, err := ioutil.TempDir(os.TempDir(), "levelDB.benchmark")
	if err != nil {
		return nil, keyvalue.NullDestroy, err
	}
	db, err := Open(path, nil)
	return db, func() error { return os.RemoveAll(path) }, err
}

func tempGS() (graphstore.Service, graphstore.DestroyFunc, error) {
	db, destroy, err := tempDB()
	if err != nil {
		return nil, graphstore.DestroyFunc(destroy), fmt.Errorf("error creating temporary DB: %v", err)
	}
	return keyvalue.NewGraphStore(db), graphstore.DestroyFunc(destroy), err
}

func destroy(i interface{}) error { return os.RemoveAll(i.(string)) }

func BenchmarkWriteSingle(b *testing.B) { keyvalue.BatchWriteBenchmark(b, tempDB, 1) }
func BenchmarkWriteBatchSml(b *testing.B) {
	keyvalue.BatchWriteBenchmark(b, tempDB, smallBatchSize)
}
func BenchmarkWriteBatchMed(b *testing.B) {
	keyvalue.BatchWriteBenchmark(b, tempDB, mediumBatchSize)
}
func BenchmarkWriteBatchLrg(b *testing.B) {
	keyvalue.BatchWriteBenchmark(b, tempDB, largeBatchSize)
}

func BenchmarkWriteParallelSingle(b *testing.B) {
	keyvalue.BatchWriteParallelBenchmark(b, tempDB, 1)
}
func BenchmarkWriteParallelBatchLrg(b *testing.B) {
	keyvalue.BatchWriteParallelBenchmark(b, tempDB, largeBatchSize)
}

func BenchmarkGSWriteSingleEntry(b *testing.B) {
	graphstore.BatchWriteBenchmark(b, tempGS, 1)
}
func BenchmarkGSWriteBatchSml(b *testing.B) {
	graphstore.BatchWriteBenchmark(b, tempGS, smallBatchSize)
}
func BenchmarkGSWriteBatchLrg(b *testing.B) {
	graphstore.BatchWriteBenchmark(b, tempGS, largeBatchSize)
}

func TestOrder(t *testing.T) {
	graphstore.OrderTest(t, tempGS, largeBatchSize)
}
