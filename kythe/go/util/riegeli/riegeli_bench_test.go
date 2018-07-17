/*
 * Copyright 2018 Google Inc. All rights reserved.
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

package riegeli

import (
	"bytes"
	"encoding/binary"
	"io"
	"io/ioutil"
	"math/rand"
	"testing"
)

var nullRecord = bytes.Repeat([]byte{0}, maxRandRecordSize)

func genNulls(b *testing.B) []byte { return nullRecord }

const maxRandRecordSize = 1024 * 1024 * 8

var buf = make([]byte, maxRandRecordSize+8)

func genRand(seed int64) func(b *testing.B) []byte {
	rand := rand.New(rand.NewSource(seed))
	return func(b *testing.B) []byte {
		b.Helper()
		b.StopTimer()
		l := rand.Int() % maxRandRecordSize
		for i := 0; i < l; i += 8 {
			binary.LittleEndian.PutUint64(buf[i:], rand.Uint64())
		}
		b.StartTimer()
		return buf[:l]
	}
}

func benchWrite(b *testing.B, out io.Writer, pos int, opts *WriterOptions, gen func(*testing.B) []byte) {
	w := NewWriterAt(out, pos, opts)
	for i := 0; i < b.N; i++ {
		rec := gen(b)
		if err := w.Put(rec); err != nil {
			b.Fatal(err)
		}
		b.SetBytes(int64(len(rec)))
	}
	if err := w.Flush(); err != nil {
		b.Fatal(err)
	}
}

func BenchmarkWriteNullUncompressed(b *testing.B) { benchWrite(b, ioutil.Discard, 0, nil, genNulls) }
func BenchmarkWriteNullBrotli(b *testing.B) {
	benchWrite(b, ioutil.Discard, 0, &WriterOptions{Compression: BrotliCompression(-1)}, genNulls)
}

func BenchmarkWriteRandUncompressed(b *testing.B) { benchWrite(b, ioutil.Discard, 0, nil, genRand(0)) }
func BenchmarkWriteRandBrotli(b *testing.B) {
	benchWrite(b, ioutil.Discard, 0, &WriterOptions{Compression: BrotliCompression(-1)}, genRand(0))
}

func benchRead(b *testing.B, opts *WriterOptions, gen func(*testing.B) []byte) {
	buf := bytes.NewBuffer(nil)
	benchWrite(b, buf, 0, opts, gen)
	b.ResetTimer()

	r := NewReader(buf)
	for i := 0; i < b.N; i++ {
		rec, err := r.Next()
		if err != nil {
			b.Fatal(err)
		}
		b.SetBytes(int64(len(rec)))
	}
}

func BenchmarkReadNullUncompressed(b *testing.B) { benchRead(b, nil, genNulls) }
func BenchmarkReadNullBrotli(b *testing.B) {
	benchRead(b, &WriterOptions{Compression: BrotliCompression(-1)}, genNulls)
}

func BenchmarkReadRandUncompressed(b *testing.B) { benchRead(b, nil, genRand(0)) }
func BenchmarkReadRandBrotli(b *testing.B) {
	benchRead(b, &WriterOptions{Compression: BrotliCompression(-1)}, genRand(0))
}
