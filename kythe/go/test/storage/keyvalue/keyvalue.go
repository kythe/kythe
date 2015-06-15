/*
 * Copyright 2015 Google Inc. All rights reserved.
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

// Package keyvalue contains utilities to test keyvalue DB implementations.
package keyvalue

import (
	"testing"

	"kythe.io/kythe/go/storage/keyvalue"
	"kythe.io/kythe/go/test/testutil"
)

// DB re-exports keyvalue.DB for tests
type DB keyvalue.DB

// NewGraphStore re-exports keyvalue.NewGraphStore for tests
var NewGraphStore = keyvalue.NewGraphStore

// CreateFunc creates a temporary keyvalue.DB with a corresponding function to
// destroy it completely.
type CreateFunc func() (DB, DestroyFunc, error)

// DestroyFunc destroys its corresponding keyvalue.DB returned from a
// CreateFunc.
type DestroyFunc func() error

// NullDestroy does nothing.
func NullDestroy() error { return nil }

const (
	keySize = 16 // bytes
	valSize = 32 // bytes
)

// BatchWriteBenchmark benchmarks the Write method of the given keyvalue.DB.
// The number of updates per write is configured with batchSize.
func BatchWriteBenchmark(b *testing.B, create CreateFunc, batchSize int) {
	db, destroy, err := create()
	testutil.FatalOnErr(b, "CreateFunc error: %v", err)
	defer func() {
		testutil.FatalOnErr(b, "db close error: %v", db.Close())
		testutil.FatalOnErr(b, "DestroyFunc error: %v", destroy())
	}()

	keyBuf := make([]byte, keySize)
	valBuf := make([]byte, valSize)
	for i := 0; i < b.N; i++ {
		wr, err := db.Writer()
		testutil.FatalOnErr(b, "writer error: %v", err)
		for j := 0; j < batchSize; j++ {
			testutil.RandBytes(keyBuf)
			testutil.RandBytes(valBuf)
			testutil.FatalOnErr(b, "write error: %v", wr.Write(keyBuf, valBuf))
		}
		testutil.FatalOnErr(b, "writer close error: %v", wr.Close())
	}
}

// BatchWriteParallelBenchmark benchmarks the Write method of the given
// keyvalue.DB in parallel.  The number of updates per write is configured with
// batchSize.
func BatchWriteParallelBenchmark(b *testing.B, create CreateFunc, batchSize int) {
	db, destroy, err := create()
	testutil.FatalOnErr(b, "CreateFunc error: %v", err)
	defer func() {
		testutil.FatalOnErr(b, "db close error: %v", db.Close())
		testutil.FatalOnErr(b, "DestroyFunc error: %v", destroy())
	}()

	b.RunParallel(func(pb *testing.PB) {
		keyBuf := make([]byte, keySize)
		valBuf := make([]byte, valSize)
		for pb.Next() {
			wr, err := db.Writer()
			testutil.FatalOnErr(b, "writer error: %v", err)
			for j := 0; j < batchSize; j++ {
				testutil.RandBytes(keyBuf)
				testutil.RandBytes(valBuf)
				testutil.FatalOnErr(b, "write error: %v", wr.Write(keyBuf, valBuf))
			}
			testutil.FatalOnErr(b, "writer close error: %v", wr.Close())
		}
	})
}
