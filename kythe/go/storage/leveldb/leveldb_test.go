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
	"math/rand"
	"os"
	"testing"

	"kythe/go/services/graphstore"
	"kythe/go/storage/keyvalue"

	spb "kythe/proto/storage_proto"
)

const (
	keySize = 16
	valSize = 32

	smallBatchSize  = 4
	mediumBatchSize = 16
	largeBatchSize  = 64
)

func tempDB() (keyvalue.DB, string, error) {
	path, err := ioutil.TempDir(os.TempDir(), "levelDB.benchmark")
	if err != nil {
		return nil, path, err
	}
	db, err := Open(path, nil)
	return db, path, err
}

func batchWriteBenchmark(b *testing.B, batchSize int) {
	db, dir, err := tempDB()
	fatalOnErr(b, "tempDB error: %v", err)
	defer os.RemoveAll(dir)
	defer func() {
		fatalOnErr(b, "db close error: %v", db.Close())
	}()

	keyBuf := make([]byte, keySize)
	valBuf := make([]byte, valSize)
	for i := 0; i < b.N; i++ {
		wr, err := db.Writer()
		fatalOnErr(b, "writer error: %v", err)
		for j := 0; j < batchSize; j++ {
			randBytes(keyBuf)
			randBytes(valBuf)
			fatalOnErr(b, "write error: %v", wr.Write(keyBuf, valBuf))
		}
		fatalOnErr(b, "writer close error: %v", wr.Close())
	}
}

func batchWriteParallelBenchmark(b *testing.B, batchSize int) {
	db, dir, err := tempDB()
	fatalOnErr(b, "tempDB error: %v", err)
	defer os.RemoveAll(dir)
	defer func() {
		fatalOnErr(b, "db close error: %v", db.Close())
	}()

	b.RunParallel(func(pb *testing.PB) {
		keyBuf := make([]byte, keySize)
		valBuf := make([]byte, valSize)
		for pb.Next() {
			wr, err := db.Writer()
			fatalOnErr(b, "writer error: %v", err)
			for j := 0; j < batchSize; j++ {
				randBytes(keyBuf)
				randBytes(valBuf)
				fatalOnErr(b, "write error: %v", wr.Write(keyBuf, valBuf))
			}
			fatalOnErr(b, "writer close error: %v", wr.Close())
		}
	})
}

func BenchmarkWriteSingle(b *testing.B)   { batchWriteBenchmark(b, 1) }
func BenchmarkWriteBatchSml(b *testing.B) { batchWriteBenchmark(b, smallBatchSize) }
func BenchmarkWriteBatchMed(b *testing.B) { batchWriteBenchmark(b, mediumBatchSize) }
func BenchmarkWriteBatchLrg(b *testing.B) { batchWriteBenchmark(b, largeBatchSize) }

func BenchmarkWriteParallelSingle(b *testing.B)   { batchWriteParallelBenchmark(b, 1) }
func BenchmarkWriteParallelBatchLrg(b *testing.B) { batchWriteParallelBenchmark(b, largeBatchSize) }

func tempGS() (graphstore.Service, string, error) {
	db, path, err := tempDB()
	if err != nil {
		return nil, "", fmt.Errorf("error creating temporary DB: %v", err)
	}
	return keyvalue.NewGraphStore(db), path, err
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

	batchSize := largeBatchSize
	buf := make([]byte, keySize)
	updates := make([]spb.WriteRequest_Update, batchSize)
	req := &spb.WriteRequest{
		Source: &spb.VName{},
		Update: make([]*spb.WriteRequest_Update, batchSize),
	}
	for i := 0; i < 1024; i++ {
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
