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

// Package kzip_test is an external unit test for package kzip.
package kzip_test

import (
	"archive/zip"
	"bytes"
	"errors"
	"fmt"
	"io/ioutil"
	"path/filepath"
	"strings"
	"testing"

	"kythe.io/kythe/go/test/testutil"

	"github.com/golang/protobuf/proto"
	"kythe.io/kythe/go/platform/kzip"

	apb "kythe.io/kythe/proto/analysis_go_proto"
	spb "kythe.io/kythe/proto/storage_go_proto"
)

func TestRoundTrip_Proto(t *testing.T) {
	testRoundTrip(kzip.EncodingProto, t)
}

func TestRoundTrip_All(t *testing.T) {
	testRoundTrip(kzip.EncodingAll, t)
}

func TestRoundTrip_JSON(t *testing.T) {
	testRoundTrip(kzip.EncodingJSON, t)
}

func testRoundTrip(encoding kzip.Encoding, t *testing.T) {
	buf := bytes.NewBuffer(nil)

	// Create a kzip with some interesting data.
	w, err := kzip.NewWriter(buf, kzip.WithEncoding(encoding))
	if err != nil {
		t.Fatalf("NewWriter: unexpected error: %v", err)
	}

	// Write a compilation record with some index data.
	unitIn := &apb.CompilationUnit{
		VName:      &spb.VName{Corpus: "foo", Language: "bar"},
		SourceFile: []string{"blodgit"},
	}
	indexIn := &apb.IndexedCompilation_Index{
		Revisions: []string{"a", "b", "c"},
	}
	udigest, err := w.AddUnit(unitIn, indexIn)
	if err != nil {
		t.Errorf("AddUnit: unexpected error: %v", err)
	}
	t.Logf("Unit digest: %q", udigest)

	// Check that rewriting the same record reports the correct error, and
	// doesn't clobber the previous one, but returns the same digest.
	if cmp, err := w.AddUnit(unitIn, nil); err != kzip.ErrUnitExists {
		t.Errorf("AddUnit (again): got error %v, want %v", err, kzip.ErrUnitExists)
	} else if cmp != udigest {
		t.Errorf("AddUnit (again): got digest %q, want %q", cmp, udigest)
	}

	// Write a file record.
	const fileIn = "aaa\n"
	fdigest, err := w.AddFile(strings.NewReader(fileIn))
	if err != nil {
		t.Errorf("AddFile %q: unexpected error: %v", fileIn, err)
	}
	t.Logf("File digest: %q", fdigest)

	// Check that the file got hashed correctly.
	//   echo "aaa" | sha256sum
	const wantFileDigest = "17e682f060b5f8e47ea04c5c4855908b0a5ad612022260fe50e11ecb0cc0ab76"
	if fdigest != wantFileDigest {
		t.Errorf("AddFile reported the wrong digest: got %q, want %q", fdigest, wantFileDigest)
	}

	if err := w.Close(); err != nil {
		t.Errorf("Writer.Close: unexpected error: %v", err)
	}

	// Open a reader on the data produced by the writer.
	r, err := kzip.NewReader(bytes.NewReader(buf.Bytes()), int64(buf.Len()))
	if err != nil {
		t.Errorf("NewReader: unexpected error: %v", err)
	}

	// Verify that we can read the original unit back out.
	if u, err := r.Lookup(udigest); err != nil {
		t.Errorf("Lookup %q: unexpected error: %v", udigest, err)
	} else {
		if !proto.Equal(u.Proto, unitIn) {
			t.Errorf("Lookup unit: got %+v, want %+v", u.Proto, unitIn)
		}
		if !proto.Equal(u.Index, indexIn) {
			t.Errorf("Lookup index: got %+v, want %+v", u.Index, indexIn)
		}
	}

	// Verify that a non-existing unit digest reports ErrDigestNotFound.
	if u, err := r.Lookup("does not exist"); err != kzip.ErrDigestNotFound {
		t.Errorf("Lookup (non-existing unit): got %+v and error %v, want %v",
			u, err, kzip.ErrDigestNotFound)
	}

	// Verify that we can read the original file back out.
	if bits, err := r.ReadAll(fdigest); err != nil {
		t.Errorf("ReadAll %q: unexpected error: %v", fdigest, err)
	} else if got := string(bits); got != fileIn {
		t.Errorf("ReadAll %q: got %q, want %q", fdigest, got, fileIn)
	}

	// Verify that a non-existing file digest reports ErrDigestNotFound.
	if f, err := r.Open("does not exist"); err != kzip.ErrDigestNotFound {
		t.Errorf("Open (non-existing file): got error %v, want %v", err, kzip.ErrDigestNotFound)
		if err == nil {
			f.Close()
		}
	}

	// Verify that scanning works.
	ok := false
	if err := r.Scan(func(u *kzip.Unit) error {
		t.Logf("Scan found %#v", u)
		if ok {
			return errors.New("multiple units found during scan (wanted 1)")
		}
		ok = u.Digest == udigest && proto.Equal(u.Proto, unitIn) && proto.Equal(u.Index, indexIn)
		return nil
	}); err != nil {
		t.Errorf("Scan failed: %v", err)
	}
	if !ok {
		t.Errorf("Scan did not locate the stored compilation %+v", unitIn)
	}
}

// bufferStub implements io.WriteCloser and records when it has been closed to
// verify that closes are propagated correctly.
type bufferStub struct {
	writeErr error
	closeErr error
	closed   bool
}

func (b *bufferStub) Write(data []byte) (int, error) {
	if b.writeErr != nil {
		return 0, b.writeErr
	}
	return len(data), nil
}

func (b *bufferStub) Close() error {
	b.closed = true
	return b.closeErr
}

func TestWriteCloser(t *testing.T) {
	wbad := errors.New("the bad thing happened while writing")
	cbad := errors.New("the bad thing happened while closing")
	tests := []struct {
		write, close, want error
	}{
		{nil, nil, nil},    // no harm, no foul
		{wbad, nil, wbad},  // write errors propagate even if close is OK
		{nil, cbad, cbad},  // a close error doesn't get swallowed
		{wbad, cbad, wbad}, // write errors are preferred (primacy)
	}
	for _, test := range tests {
		stub := &bufferStub{writeErr: test.write, closeErr: test.close}
		w, err := kzip.NewWriteCloser(stub)
		if err != nil {
			t.Errorf("NewWriteCloser(%+v) failed: %v", test, err)
			continue
		}
		if got := w.Close(); got != test.want {
			t.Errorf("w.Close(): got %v, want %v", got, test.want)
		}
		if !stub.closed {
			t.Error("Stub was not correctly closed")
		}
	}
}

func TestErrors(t *testing.T) {
	// A structurally invalid zip stream should complain suitably.
	if r, err := kzip.NewReader(bytes.NewReader(nil), 0); err == nil {
		t.Errorf("NewReader (invalid): got %+v, want error", r)
	} else {
		t.Logf("NewReader (invalid): error OK: %v", err)
	}

	{ // An empty archive should fail at open.
		buf := bytes.NewBuffer(nil)
		if err := zip.NewWriter(buf).Close(); err != nil {
			t.Fatalf("Creating empty ZIP file failed: %v", err)
		}
		if r, err := kzip.NewReader(bytes.NewReader(buf.Bytes()), int64(buf.Len())); err == nil {
			t.Errorf("NewReader (empty): got %+v, want error", r)
		} else {
			t.Logf("NewReader (empty): error OK: %v", err)
		}
	}

	{ // An archive should have a leading root directory
		buf := bytes.NewBuffer(nil)
		w := zip.NewWriter(buf)
		f, err := w.Create("not-a-directory")
		if err != nil {
			t.Fatalf("Creating dummy file: %v", err)
		}
		fmt.Fprintln(f, "this is not a pipe")
		if err := w.Close(); err != nil {
			t.Fatalf("Closing dummy file: %v", err)
		}
		if r, err := kzip.NewReader(bytes.NewReader(buf.Bytes()), int64(buf.Len())); err == nil {
			t.Errorf("NewReader (file): got %+v, want error", r)
		} else {
			t.Logf("NewReader (file): error OK: %v", err)
		}
	}
}

func TestScanHelper(t *testing.T) {
	buf := bytes.NewBuffer(nil)
	w, err := kzip.NewWriter(buf)
	if err != nil {
		t.Fatalf("Creating kzip writer: %v", err)
	}

	const fileData = "fweep"
	fdigest, err := w.AddFile(strings.NewReader(fileData))
	if err != nil {
		t.Fatalf("AddFile failed: %v", err)
	}

	udigest, err := w.AddUnit(&apb.CompilationUnit{
		OutputKey: fdigest,
	}, &apb.IndexedCompilation_Index{
		Revisions: []string{"alphawozzle"},
	})
	if err != nil {
		t.Fatalf("AddUnit failed: %v", err)
	}

	if err := w.Close(); err != nil {
		t.Fatalf("Closing kzip: %v", err)
	}

	var numUnits int
	if err := kzip.Scan(bytes.NewReader(buf.Bytes()), func(r *kzip.Reader, unit *kzip.Unit) error {
		numUnits++
		if unit.Digest != udigest {
			t.Errorf("Unit digest: got %q, want %q", unit.Digest, udigest)
		}
		if bits, err := r.ReadAll(unit.Proto.GetOutputKey()); err != nil {
			t.Errorf("ReadAll %q failed: %v", unit.Proto.GetOutputKey(), err)
		} else if got := string(bits); got != fileData {
			t.Errorf("ReadAll %q: got %q, want %q", unit.Proto.GetOutputKey(), got, fileData)
		}
		return nil
	}); err != nil {
		t.Errorf("Scan failed: %v", err)
	}
	if numUnits != 1 {
		t.Errorf("Scan found %d units, want 1", numUnits)
	}
}

func TestScanError(t *testing.T) {
	buf := bytes.NewBuffer(nil)
	w, err := kzip.NewWriter(buf)
	if err != nil {
		t.Fatalf("Creating kzip writer: %v", err)
	}

	const fileData = "fweep"
	fdigest, err := w.AddFile(strings.NewReader(fileData))
	if err != nil {
		t.Fatalf("AddFile failed: %v", err)
	}

	if _, err := w.AddUnit(&apb.CompilationUnit{
		OutputKey: fdigest,
	}, &apb.IndexedCompilation_Index{
		Revisions: []string{"alphawozzle"},
	}); err != nil {
		t.Fatalf("AddUnit failed: %v", err)
	}

	if _, err := w.AddUnit(&apb.CompilationUnit{
		OutputKey: fdigest + "2",
	}, &apb.IndexedCompilation_Index{
		Revisions: []string{"alphawozzle"},
	}); err != nil {
		t.Fatalf("AddUnit failed: %v", err)
	}

	if err := w.Close(); err != nil {
		t.Fatalf("Closing kzip: %v", err)
	}

	expected := errors.New("expected error")
	var unitCount int
	if err := kzip.Scan(bytes.NewReader(buf.Bytes()), func(r *kzip.Reader, unit *kzip.Unit) error {
		unitCount++
		return expected
	}); err == nil {
		t.Errorf("Scan succeeded unexpectedly")
	} else if err != expected {
		t.Errorf("Scan failed unexpectedly: %v", err)
	} else if unitCount != 1 {
		t.Errorf("Scanned %d units; expected: 1", unitCount)
	}
}

func TestScanConcurrency(t *testing.T) {
	buf := bytes.NewBuffer(nil)
	w, err := kzip.NewWriter(buf)
	if err != nil {
		t.Fatalf("Creating kzip writer: %v", err)
	}

	const fileData = "fweep"
	fdigest, err := w.AddFile(strings.NewReader(fileData))
	if err != nil {
		t.Fatalf("AddFile failed: %v", err)
	}

	const N = 128
	for i := 0; i < N; i++ {
		if _, err := w.AddUnit(&apb.CompilationUnit{
			OutputKey: fmt.Sprintf("%s%d", fdigest, i),
		}, &apb.IndexedCompilation_Index{
			Revisions: []string{"alphawozzle"},
		}); err != nil {
			t.Fatalf("AddUnit %d failed: %v", i, err)
		}
	}

	if err := w.Close(); err != nil {
		t.Fatalf("Closing kzip: %v", err)
	}

	var numUnits int
	if err := kzip.Scan(bytes.NewReader(buf.Bytes()), func(r *kzip.Reader, unit *kzip.Unit) error {
		numUnits++
		return nil
	}, kzip.ReadConcurrency(16)); err != nil {
		t.Errorf("Scan failed: %v", err)
	}
	if numUnits != N {
		t.Errorf("Scan found %d units, want %d", numUnits, N)
	}
}

const testDataDir = "../../../cxx/common/testdata"

func TestMissingJSONUnitFails(t *testing.T) {
	b, err := ioutil.ReadFile(testutil.TestFilePath(t, filepath.Join(testDataDir, "missing-unit.kzip")))
	if err != nil {
		t.Fatalf("Unable to read test file missing-unit.kzip: %s", err)
	}
	_, err = kzip.NewReader(bytes.NewReader(b), int64(len(b)))
	if err == nil || err.Error() != "both proto and JSON units found but are not identical" {
		t.Errorf("Unexpected error: %s", err)
	}
}

func TestMissingProtoUnitFails(t *testing.T) {
	b, err := ioutil.ReadFile(testutil.TestFilePath(t, filepath.Join(testDataDir, "missing-pbunit.kzip")))
	if err != nil {
		t.Fatalf("Unable to read test file missing-pbunit.kzip: %s", err)
	}
	_, err = kzip.NewReader(bytes.NewReader(b), int64(len(b)))
	if err == nil || err.Error() != "both proto and JSON units found but are not identical" {
		t.Errorf("Unexpected error: %s", err)
	}
}
