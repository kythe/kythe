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
	"fmt"
	"io/ioutil"
	"math/rand"
	"os"
	"path/filepath"
	"testing"

	"kythe/go/services/graphstore"

	spb "kythe/proto/storage_proto"
)

const (
	keySize = 5

	smallBatchSize  = 4
	mediumBatchSize = 16
	largeBatchSize  = 64
)

func tempGS() (*DB, string, error) {
	dir, err := ioutil.TempDir("", "sqlite3.benchmark")
	if err != nil {
		return nil, "", err
	}
	db, err := OpenGraphStore(SQLite3, filepath.Join(dir, "db"))
	return db, dir, err
}

func batchGSWriteBenchmark(b *testing.B, batchSize int) {
	gs, dir, err := tempGS()
	fatalOnErr(b, "tempGS error: %v", err)
	defer os.RemoveAll(dir)
	defer func() {
		fatalOnErr(b, "gs close error: %v", gs.Close())
	}()

	buf := make([]byte, keySize)
	updates := make([]spb.WriteRequest_Update, batchSize)
	req := &spb.WriteRequest{
		Source: &spb.VName{},
		Update: make([]*spb.WriteRequest_Update, batchSize),
	}
	for i := 0; i < b.N; i++ {
		randVName(req.Source, buf)

		for j := 0; j < batchSize; j++ {
			randUpdate(&updates[j], buf)
			req.Update[j] = &updates[j]
		}

		fatalOnErr(b, "write error: %v", gs.Write(req))
	}
}

func BenchmarkGSWriteSingleEntry(b *testing.B) { batchGSWriteBenchmark(b, 1) }
func BenchmarkGSWriteBatchSml(b *testing.B)    { batchGSWriteBenchmark(b, smallBatchSize) }
func BenchmarkGSWriteBatchLrg(b *testing.B)    { batchGSWriteBenchmark(b, largeBatchSize) }

func TestGraphStoreOrder(t *testing.T) {
	gs, dir, err := tempGS()
	fatalOnErrT(t, "tempGS error: %v", err)
	defer os.RemoveAll(dir)
	defer func() {
		fatalOnErrT(t, "gs close error: %v", gs.Close())
	}()

	batchSize := smallBatchSize
	buf := make([]byte, keySize)
	updates := make([]spb.WriteRequest_Update, batchSize)
	req := &spb.WriteRequest{
		Source: &spb.VName{},
		Update: make([]*spb.WriteRequest_Update, batchSize),
	}
	for i := 0; i < 10240; i++ {
		randVName(req.Source, buf)

		for j := 0; j < batchSize; j++ {
			randUpdate(&updates[j], buf)
			req.Update[j] = &updates[j]
		}

		fatalOnErrT(t, "write error: %v", gs.Write(req))
	}

	var lastEntry *spb.Entry
	fatalOnErrT(t, "entryLess error: %v",
		graphstore.EachScanEntry(gs, nil, func(entry *spb.Entry) error {
			if !graphstore.EntryLess(lastEntry, entry) {
				return fmt.Errorf("expected {%v} < {%v}", lastEntry, entry)
			}
			return nil
		}))
}

func fatalOnErr(b *testing.B, msg string, err error, args ...interface{}) {
	if err != nil {
		b.Fatalf(msg, append([]interface{}{err}, args...)...)
	}
}

func fatalOnErrT(t *testing.T, msg string, err error, args ...interface{}) {
	if err != nil {
		t.Fatalf(msg, append([]interface{}{err}, args...)...)
	}
}

var factValue = []byte("factValue")

func randUpdate(u *spb.WriteRequest_Update, buf []byte) {
	u.Target = &spb.VName{}
	randVName(u.GetTarget(), buf)
	u.EdgeKind = randStr(buf[:2])
	u.FactName = randStr(buf)
	u.FactValue = factValue
}

func randEntry(entry *spb.Entry, buf []byte) {
	randVName(entry.GetSource(), buf)
	entry.FactName = randStr(buf)
	entry.FactValue = factValue
}

func randVName(v *spb.VName, buf []byte) {
	v.Signature = randStr(buf)
	v.Corpus = randStr(buf)
	v.Root = randStr(buf)
	v.Path = randStr(buf)
	v.Language = randStr(buf)
}

func randStr(buf []byte) *string {
	const chars = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
	randBytes(buf)
	for i, b := range buf {
		buf[i] = chars[b%byte(len(chars))]
	}
	str := string(buf)
	return &str
}

func randBytes(bytes []byte) {
	i := len(bytes) - 1
	for {
		n := rand.Int63()
		for j := 0; j < 8; j++ {
			bytes[i] = byte(n)
			i--
			if i == -1 {
				return
			}
			n >>= 8
		}
	}
}
