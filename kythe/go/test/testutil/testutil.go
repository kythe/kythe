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
	"fmt"
	"math/rand"
	"path/filepath"
	"reflect"
	"runtime"
	"testing"
)

func caller(up int) (file string, line int) {
	_, file, line, ok := runtime.Caller(up + 2)
	if !ok {
		panic("could not get runtime.Caller")
	}
	return filepath.Base(file), line
}

// DeepEqual determines if expected is deeply equal to got, returning a detailed
// error if not.
func DeepEqual(expected, got interface{}) error {
	et, gt := reflect.TypeOf(expected), reflect.TypeOf(got)
	ev, gv := reflect.ValueOf(expected), reflect.ValueOf(got)

	for {
		if (et == nil) != (gt == nil) {
			// Only one was a nil interface value
			return fmt.Errorf("expected: %v; found %v", expected, got)
		} else if et == nil {
			return nil // both were nil interface values
		} else if et.Kind() != reflect.Ptr || gt.Kind() != reflect.Ptr {
			break
		}

		if ev.Elem().IsValid() != gv.Elem().IsValid() {
			return fmt.Errorf("expected: %v; found %v", expected, got)
		}
		if !ev.Elem().IsValid() {
			return nil
		}
		ev, gv = ev.Elem(), gv.Elem()
		et, gt = ev.Type(), gv.Type()
	}

	if et != gt {
		return expectError(expected, got, "types differ: expected %T; found %T", expected, got)
	}

	if et.Kind() == reflect.Slice && gt.Kind() == reflect.Slice {
		return expectSliceEqual(ev, gv)
	} else if et.Kind() == reflect.Struct && gt.Kind() == reflect.Struct {
		return expectStructEqual(ev, gv)
	} else if et.Kind() == reflect.Map && gt.Kind() == reflect.Map {
		return expectMapEqual(ev, gv)
	}

	if !reflect.DeepEqual(expected, got) {
		return expectError(expected, got, "")
	}
	return nil
}

func expectError(expected, got interface{}, msg string, args ...interface{}) error {
	if msg != "" {
		msg = fmt.Sprintf(": "+msg, args...)
	}

	return fmt.Errorf("expected %v; found %v%s", expected, got, msg)
}

func expectSliceEqual(expected reflect.Value, got reflect.Value) error {
	el, gl := expected.Len(), got.Len()
	if el != gl {
		return expectError(expected.Interface(), got.Interface(), "expected length: %d; found length: %d", el, gl)
	}

	for i := 0; i < el; i++ {
		ev := expected.Index(i).Interface()
		gv := got.Index(i).Interface()
		if err := DeepEqual(ev, gv); err != nil {
			return expectError(expected.Interface(), got.Interface(), "values at index %d differ: %s", i, err)
		}
	}
	return nil
}

func expectStructEqual(expected, got reflect.Value) error {
	for i := 0; i < expected.NumField(); i++ {
		ef, gf := expected.Field(i), got.Field(i)
		if err := DeepEqual(ef.Interface(), gf.Interface()); err != nil {
			field := expected.Type().Field(i)
			return expectError(expected.Interface(), got.Interface(), "%s fields differ: %s", field.Name, err)
		}
	}

	return nil
}

func expectMapEqual(expected, got reflect.Value) error {
	el, gl := expected.Len(), got.Len()
	if el != gl {
		return expectError(expected.Interface(), got.Interface(), "expected length: %d; found length: %d", el, gl)
	}

	for _, key := range expected.MapKeys() {
		ev, gv := expected.MapIndex(key), got.MapIndex(key)

		if err := DeepEqual(ev.Interface(), gv.Interface()); err != nil {
			return expectError(expected.Interface(), got.Interface(), "%#v key values differ: %s", key.Interface(), err)
		}
	}

	return nil
}

// FatalOnErr calls b.Fatalf(msg, err, args...) if err != nil
func FatalOnErr(b *testing.B, msg string, err error, args ...interface{}) {
	if err != nil {
		file, line := caller(0)
		b.Fatalf("%s:%d: "+msg, append([]interface{}{file, line, err}, args...)...)
	}
}

// FatalOnErrT calls t.Fatalf(msg, err, args...) if err != nil
func FatalOnErrT(t *testing.T, msg string, err error, args ...interface{}) {
	if err != nil {
		file, line := caller(0)
		t.Fatalf("%s:%d: "+msg, append([]interface{}{file, line, err}, args...)...)
	}
}

// Errorf calls t.Errorf(msg, err, args...) if err != nil
func Errorf(t *testing.T, msg string, err error, args ...interface{}) {
	if err != nil {
		file, line := caller(0)
		t.Errorf("%s:%d: "+msg, append([]interface{}{file, line, err}, args...)...)
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
