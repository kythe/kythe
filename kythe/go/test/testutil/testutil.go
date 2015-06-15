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

// Package testutil contains common utilities to test Kythe libraries.
package testutil

import (
	"math/rand"
	"testing"
)

// FatalOnErr calls b.Fatalf(msg, err, args...) if err != nil
func FatalOnErr(b *testing.B, msg string, err error, args ...interface{}) {
	if err != nil {
		b.Fatalf(msg, append([]interface{}{err}, args...)...)
	}
}

// FatalOnErrT calls t.Fatalf(msg, err, args...) if err != nil
func FatalOnErrT(t *testing.T, msg string, err error, args ...interface{}) {
	if err != nil {
		t.Fatalf(msg, append([]interface{}{err}, args...)...)
	}
}

// RandStr returns a random string of the given length
func RandStr(size int) string {
	const chars = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
	buf := make([]byte, size, size)
	RandBytes(buf)
	for i, b := range buf {
		buf[i] = chars[b%byte(len(chars))]
	}
	return string(buf)
}

// RandBytes fills the given slice with random bytes
func RandBytes(bytes []byte) {
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
