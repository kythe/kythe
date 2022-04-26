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

// Package graphstore contains common utilities for testing GraphStore
// implementations.
package graphstore // import "kythe.io/kythe/go/test/services/graphstore"

import (
	"context"
	"fmt"
	"testing"

	"kythe.io/kythe/go/services/graphstore"
	"kythe.io/kythe/go/test/testutil"
	"kythe.io/kythe/go/util/compare"

	spb "kythe.io/kythe/proto/storage_go_proto"
)

// Service re-exports graphstore.Service for tests
type Service graphstore.Service

// CreateFunc creates a temporary graphstore.Service with a corresponding
// function to destroy it completely.
type CreateFunc func() (Service, DestroyFunc, error)

// DestroyFunc destroys its corresponding graphstore.Service returned
// from a CreateFunc.
type DestroyFunc func() error

// NullDestroy does nothing.
func NullDestroy() error { return nil }

const keySize = 16 // bytes

var ctx = context.Background()

// BatchWriteBenchmark benchmarks the Write method of the given
// graphstore.Service.  The number of updates per write is configured with
// batchSize.
func BatchWriteBenchmark(b *testing.B, create CreateFunc, batchSize int) {
	b.StopTimer()
	gs, destroy, err := create()
	testutil.Fatalf(b, "CreateFunc error: %v", err)
	defer func() {
		testutil.Fatalf(b, "gs close error: %v", gs.Close(ctx))
		testutil.Fatalf(b, "DestroyFunc error: %v", destroy())
	}()

	updates := make([]spb.WriteRequest_Update, batchSize)
	req := &spb.WriteRequest{
		Source: &spb.VName{},
		Update: make([]*spb.WriteRequest_Update, batchSize),
	}
	for i := 0; i < b.N; i++ {
		randVName(req.Source, keySize)

		for j := 0; j < batchSize; j++ {
			randUpdate(&updates[j], keySize)
			req.Update[j] = &updates[j]
		}

		testutil.Fatalf(b, "write error: %v", gs.Write(ctx, req))
	}
}

// OrderTest tests the ordering of the streamed entries while reading from the
// CreateFunc created graphstore.Service.
func OrderTest(t *testing.T, create CreateFunc, batchSize int) {
	gs, destroy, err := create()
	testutil.Fatalf(t, "CreateFunc error: %v", err)
	defer func() {
		testutil.Fatalf(t, "gs close error: %v", gs.Close(ctx))
		testutil.Fatalf(t, "DestroyFunc error: %v", destroy())
	}()

	updates := make([]spb.WriteRequest_Update, batchSize)
	req := &spb.WriteRequest{
		Source: &spb.VName{},
		Update: make([]*spb.WriteRequest_Update, batchSize),
	}
	for i := 0; i < 1024; i++ {
		randVName(req.Source, keySize)

		for j := 0; j < batchSize; j++ {
			randUpdate(&updates[j], keySize)
			req.Update[j] = &updates[j]
		}

		testutil.Fatalf(t, "write error: %v", gs.Write(ctx, req))
	}

	var lastEntry *spb.Entry
	testutil.Fatalf(t, "entryLess error: %v",
		gs.Scan(ctx, new(spb.ScanRequest), func(entry *spb.Entry) error {
			if compare.Entries(lastEntry, entry) != compare.LT {
				return fmt.Errorf("expected {%v} < {%v}", lastEntry, entry)
			}
			return nil
		}))
}

var factValue = []byte("factValue")

func randUpdate(u *spb.WriteRequest_Update, size int) {
	u.Target = &spb.VName{}
	randVName(u.GetTarget(), size)
	u.EdgeKind = testutil.RandStr(2)
	u.FactName = testutil.RandStr(size)
	u.FactValue = factValue
}

func randVName(v *spb.VName, size int) {
	v.Signature = testutil.RandStr(size)
	v.Corpus = testutil.RandStr(size)
	v.Root = testutil.RandStr(size)
	v.Path = testutil.RandStr(size)
	v.Language = testutil.RandStr(size)
}
