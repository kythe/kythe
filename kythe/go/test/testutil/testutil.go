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
	"regexp"
	"runtime"
	"strings"
	"testing"

	"kythe.io/kythe/go/util/compare"

	"github.com/google/go-cmp/cmp"
	"sigs.k8s.io/yaml"
)

// DeepEqual determines if expected is deeply equal to got, returning a
// detailed error if not. It is okay for expected and got to be protobuf
// message values.
func DeepEqual[T any](expected, got T, opts ...cmp.Option) error {
	if diff := compare.ProtoDiff(expected, got, opts...); diff != "" {
		return fmt.Errorf("(-expected; +found)\n%s", diff)
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
	var e, g any
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
func Errorf(t testing.TB, msg string, err error, args ...any) {
	if err != nil {
		t.Helper()
		t.Errorf(msg, append([]any{err}, args...)...)
	}
}

// Fatalf is equivalent to t.Fatalf(msg, err, args...) if err != nil.
func Fatalf(t testing.TB, msg string, err error, args ...any) {
	if err != nil {
		t.Helper()
		t.Fatalf(msg, append([]any{err}, args...)...)
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
