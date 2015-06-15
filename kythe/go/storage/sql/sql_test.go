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

package sql

import (
	"io/ioutil"
	"os"
	"path/filepath"
	"testing"

	"kythe.io/kythe/go/test/services/graphstore"
)

const (
	smallBatchSize  = 4
	mediumBatchSize = 16
	largeBatchSize  = 64
)

func tempGS() (graphstore.Service, graphstore.DestroyFunc, error) {
	dir, err := ioutil.TempDir("", "sqlite3.benchmark")
	if err != nil {
		return nil, graphstore.NullDestroy, err
	}
	db, err := Open(SQLite3, filepath.Join(dir, "db"))
	return db, func() error { return os.RemoveAll(dir) }, err
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
