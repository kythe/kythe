/*
 * Copyright 2018 The Kythe Authors. All rights reserved.
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

func genNulls(size int) [][]byte {
	recs := make([][]byte, size)
	for i := 0; i < size; i++ {
		recs[i] = nullRecord
	}
	return recs
}

const maxRandRecordSize = 1024 * 1024 * 8

func genRand(seed int64) func(int) [][]byte {
	return func(size int) [][]byte {
		rand := rand.New(rand.NewSource(seed))
		recs := make([][]byte, 0, size)
		buf := make([]byte, size*maxRandRecordSize+8)
		for n := 0; n < size; n++ {
			l := rand.Int() % maxRandRecordSize
			for i := 0; i < l; i += 8 {
				binary.LittleEndian.PutUint64(buf[i:], rand.Uint64())
			}
			recs = append(recs, buf[:l])
			buf = buf[l:]
		}
		return recs
	}
}

func benchWrite(b *testing.B, out io.Writer, pos int, opts *WriterOptions, gen func(int) [][]byte) {
	recs := gen(b.N)
	b.ResetTimer()
	w := NewWriterAt(out, pos, opts)
	for _, rec := range recs {
		if err := w.Put(rec); err != nil {
			b.Fatal(err)
		}
		b.SetBytes(int64(len(rec)))
	}
	if err := w.Flush(); err != nil {
		b.Fatal(err)
	}
}

var benchOptions = []string{
	"default",

	"uncompressed",
	"brotli",
	"snappy",
	"zstd",

	"uncompressed,transpose",
	"brotli,transpose",
	"snappy,transpose",
	"zstd,transpose",
}

func BenchmarkWriteNull(b *testing.B) {
	for _, test := range benchOptions {
		opts, err := ParseOptions(test)
		if err != nil {
			b.Fatal(err)
		}
		b.Run(test, func(b *testing.B) { benchWrite(b, ioutil.Discard, 0, opts, genNulls) })
	}
}
func BenchmarkWriteRand(b *testing.B) {
	for _, test := range benchOptions {
		opts, err := ParseOptions(test)
		if err != nil {
			b.Fatal(err)
		}
		b.Run(test, func(b *testing.B) { benchWrite(b, ioutil.Discard, 0, opts, genRand(0)) })
	}
}

func benchRead(b *testing.B, opts *WriterOptions, gen func(int) [][]byte) {
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

func BenchmarkReadNull(b *testing.B) {
	for _, test := range benchOptions {
		opts, err := ParseOptions(test)
		if err != nil {
			b.Fatal(err)
		}
		b.Run(test, func(b *testing.B) { benchRead(b, opts, genNulls) })
	}
}
func BenchmarkReadRand(b *testing.B) {
	for _, test := range benchOptions {
		opts, err := ParseOptions(test)
		if err != nil {
			b.Fatal(err)
		}
		b.Run(test, func(b *testing.B) { benchRead(b, opts, genRand(0)) })
	}
}
