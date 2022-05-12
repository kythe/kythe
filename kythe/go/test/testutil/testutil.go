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

// Package testutil contains common utilities to test Kythe libraries.
package testutil // import "kythe.io/kythe/go/test/testutil"

import (
	"encoding/json"
	"fmt"
	"math/rand"
	"os"
	"path/filepath"
	"reflect"
	"regexp"
	"runtime"
	"strings"
	"testing"

	"kythe.io/kythe/go/util/compare"

	"github.com/google/go-cmp/cmp"
	"google.golang.org/protobuf/proto"
	"sigs.k8s.io/yaml"
)

// DeepEqual determines if expected is deeply equal to got, returning a
// detailed error if not. It is okay for expected and got to be protobuf
// message values.
func DeepEqual(expected, got interface{}) error {
	// Check for proto.Message types specifically; because protobuf generated
	// types have complex conditions for equality, only do the reflective step
	// if they are known not to be equivalent. This avoids spurious errors that
	// may arise from messages that are equivalent but have different internal
	// states (e.g., due to unrecognized fields, caching, etc.).
	if epb, ok := expected.(proto.Message); ok {
		if gpb, ok := got.(proto.Message); ok {
			if proto.Equal(epb, gpb) {
				return nil
			}
			return fmt.Errorf("(-expected; +found)\n%s", compare.ProtoDiff(expected, got))
		}
	}

	// At this point, we either have non-protobuf values, or we know that the
	// two are unequal.
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

		if ev.IsValid() != gv.IsValid() {
			return expectError(expected.Interface(), got.Interface(), "%#v key values not both valid", key.Interface())
		} else if err := DeepEqual(ev.Interface(), gv.Interface()); err != nil {
			return expectError(expected.Interface(), got.Interface(), "%#v key values differ: %s", key.Interface(), err)
		}
	}

	return nil
}

var multipleNewLines = regexp.MustCompile("\n{2,}")

// TrimmedEqual compares two strings after collapsing irrelevant whitespace at
// the beginning or end of lines. It returns both a boolean indicating equality,
// as well as any relevant diff.
func TrimmedEqual(got, want []byte) (bool, string) {
	// remove superfluous whitespace
	gotStr := strings.Trim(string(got[:]), " \n")
	wantStr := strings.Trim(string(want[:]), " \n")
	gotStr = multipleNewLines.ReplaceAllString(gotStr, "\n")
	wantStr = multipleNewLines.ReplaceAllString(wantStr, "\n")

	// diff want vs got
	diff := cmp.Diff(gotStr, wantStr)
	return diff == "", diff
}

// YAMLEqual compares two bytes assuming they are yaml, by converting to json
// and doing an ordering-agnostic comparison.  Note this carries some
// restrictions because yaml->json conversion fails for nil or binary map keys.
func YAMLEqual(expected, got []byte) error {
	e, err := yaml.YAMLToJSON(expected)
	if err != nil {
		return fmt.Errorf("yaml->json failure for expected: %v", err)
	}
	g, err := yaml.YAMLToJSON(got)
	if err != nil {
		return fmt.Errorf("yaml->json failure for got: %v", err)
	}
	return JSONEqual(e, g)
}

// JSONEqual compares two bytes assuming they are json, using encoding/json
// and DeepEqual.
func JSONEqual(expected, got []byte) error {
	var e, g interface{}
	if err := json.Unmarshal(expected, &e); err != nil {
		return fmt.Errorf("decoding expected json: %v", err)
	}
	if err := json.Unmarshal(got, &g); err != nil {
		return fmt.Errorf("decoding got json: %v", err)
	}
	return DeepEqual(e, g)
}

func caller(up int) (file string, line int) {
	_, file, line, ok := runtime.Caller(up + 2)
	if !ok {
		panic("could not get runtime.Caller")
	}
	return filepath.Base(file), line
}

// Errorf is equivalent to t.Errorf(msg, err, args...) if err != nil.
func Errorf(t testing.TB, msg string, err error, args ...interface{}) {
	if err != nil {
		t.Helper()
		t.Errorf(msg, append([]interface{}{err}, args...)...)
	}
}

// Fatalf is equivalent to t.Fatalf(msg, err, args...) if err != nil.
func Fatalf(t testing.TB, msg string, err error, args ...interface{}) {
	if err != nil {
		t.Helper()
		t.Fatalf(msg, append([]interface{}{err}, args...)...)
	}
}

// RandStr returns a random string of the given length
func RandStr(size int) string {
	const chars = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
	buf := make([]byte, size)
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

// TestFilePath takes a path and resolves it based on the testdir.  If it
// cannot successfully do so, it calls t.Fatal and abandons.
func TestFilePath(t *testing.T, path string) string {
	t.Helper()
	pwd, err := os.Getwd()
	if err != nil {
		t.Fatalf("Failed to resolve path %s: %v", path, err)
	}
	return filepath.Join(pwd, filepath.FromSlash(path))
}
