/*
 * Copyright 2016 The Kythe Authors. All rights reserved.
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

// Package testutil provides support functions for unit testing implementations
// of the kcd.ReadWriter interface.
package testutil // import "kythe.io/kythe/go/platform/kcd/testutil"

import (
	"context"
	"errors"
	"fmt"
	"os"
	"regexp"
	"strings"
	"testing"
	"time"

	"kythe.io/kythe/go/platform/kcd"
	"kythe.io/kythe/go/platform/kcd/kythe"

	"google.golang.org/protobuf/proto"

	apb "kythe.io/kythe/proto/analysis_go_proto"
	spb "kythe.io/kythe/proto/storage_go_proto"
)

// TestError is the concrete type of errors returned by the Run function.
type TestError struct {
	Desc   string // Description of the test that failed (human-readable)
	Method string // The name of the method that returned an error
	Err    error  // The underlying error returned by the method
}

func (t *TestError) Error() string {
	return fmt.Sprintf("R [%s]: %s: %v", t.Desc, t.Method, t.Err)
}

// These constants are the expected values used by the Run tests.
const (
	Revision  = "1234"
	Corpus    = "ratzafratza"
	FormatKey = "λ"
	Language  = "go"
)

// UnitType is the type of compilation message stored in the database by the
// tests in Run.
var UnitType *apb.CompilationUnit

func regexps(exprs ...string) (res []*regexp.Regexp) {
	for _, expr := range exprs {
		res = append(res, regexp.MustCompile(expr))
	}
	return
}

// Run applies a sequence of correctness tests to db, which must be initially
// empty, and returns any errors that occur.  If db passes all the tests, the
// return value is nil; otherwise each error is of concrete type *TestError.
func Run(t *testing.T, ctx context.Context, db kcd.ReadWriter) []error {
	var errs []error

	// Each check is passed a function to report errors.  The errors are packed
	// into *TestError wrappers and accumulated to return.
	type failer func(method string, err error)
	check := func(desc string, test func(failer)) {
		t.Helper()
		test(func(method string, err error) {
			t.Helper()
			err = &TestError{desc, method, err}
			t.Error(err)
			errs = append(errs, err)
		})
	}

	// The order of the tests below is significant; each modifies the state of
	// the database being tested in a way that can be used by subsequent tests
	// on success.

	check("initial revisions list is empty", func(fail failer) {
		if err := db.Revisions(ctx, nil, func(rev kcd.Revision) error {
			return fmt.Errorf("unexpected revision %v", rev) // any hit is an error
		}); err != nil {
			fail("Revisions", err)
		}
	})

	check("initial units list is empty", func(fail failer) {
		anyTarget := &kcd.FindFilter{Targets: regexps(".*")}
		if err := db.Find(ctx, anyTarget, func(digest string) error {
			return fmt.Errorf("unexpected compilation %q", digest)
		}); err != nil {
			fail("Find", err)
		}
	})

	check("written revisions round-trip", func(fail failer) {
		wantTime := time.Now().In(time.UTC).Round(time.Microsecond)
		wantRev := kcd.Revision{Revision, Corpus, wantTime}
		if err := db.WriteRevision(ctx, wantRev, true); err != nil {
			fail("WriteRevision", err)
			return
		}
		var gotRev kcd.Revision
		if err := db.Revisions(ctx, nil, func(rev kcd.Revision) error {
			gotRev = rev
			return nil
		}); err != nil {
			fail("Revisions", err)
		}
		if got, want := gotRev.Corpus, wantRev.Corpus; got != want {
			fail("corpus", fmt.Errorf("got %q, want %q", got, want))
		}
		if got, want := gotRev.Revision, wantRev.Revision; got != want {
			fail("marker", fmt.Errorf("got %q, want %q", got, want))
		}
		if got, want := gotRev.Timestamp, wantRev.Timestamp; !got.Equal(want) {
			fail("timestamp", fmt.Errorf("got %v, want %v", got, want))
		}
	})

	check("write revision error checks", func(fail failer) {
		if db.WriteRevision(ctx, kcd.Revision{"foo", "", time.Time{}}, false) == nil {
			fail("WriteRevision", errors.New("no error on empty corpus"))
		}
		if db.WriteRevision(ctx, kcd.Revision{"", "bar", time.Time{}}, false) == nil {
			fail("WriteRevision", errors.New("no error on empty revision"))
		}
	})

	check("missing units are not found", func(fail failer) {
		if err := db.Units(ctx, []string{"none such"}, func(digest, key string, data []byte) error {
			fail("Units", fmt.Errorf("unexpected digest %q and key %q", digest, key))
			return nil
		}); err != nil {
			fail("Units", err)
		}
	})

	check("missing files are not found", func(fail failer) {
		if err := db.Files(ctx, []string{"none such"}, func(digest string, data []byte) error {
			fail("Files", fmt.Errorf("unexpected digest %q and data %q", digest, string(data)))
			return nil
		}); err != nil {
			fail("Files", err)
		}

		if err := db.FilesExist(ctx, []string{"none such"}, func(digest string) error {
			fail("FilesExist", fmt.Errorf("unexpected digest %q", digest))
			return nil
		}); err != nil {
			fail("FilesExist", err)
		}
	})

	// SHA256 test vector from http://www.nsrl.nist.gov/testdata/
	const wantData = "abc"
	const wantDigest = "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad"

	check("written files round-trip", func(fail failer) {
		gotDigest, err := db.WriteFile(ctx, strings.NewReader(wantData))
		if err != nil {
			fail("WriteFile", err)
		}
		if gotDigest != wantDigest {
			fail("WriteFile", fmt.Errorf("got digest %q, want %q", gotDigest, wantDigest))
		}
		var gotData string
		if err := db.Files(ctx, []string{gotDigest}, func(_ string, data []byte) error {
			gotData = string(data)
			return nil
		}); err != nil {
			fail("Files", err)
		}
		if gotData != wantData {
			fail("Files", fmt.Errorf("got %q, want %q", gotData, wantData))
		}
	})

	var unitDigest string // set by the test below

	check("written units round-trip", func(fail failer) {
		const inputDigest = "b05ffa4eea8fb5609d576a68c1066be3f99e4dc53d365a0ac2a78259b2dd91f9"
		dummy := kythe.Unit{&apb.CompilationUnit{
			VName:      &spb.VName{Signature: "//foo/bar/baz:quux", Language: "go"},
			SourceFile: []string{"quux.cc"},
			RequiredInput: []*apb.CompilationUnit_FileInput{{
				VName: &spb.VName{Path: "foo/bar/baz/quux.cc"},
				Info: &apb.FileInfo{
					Path:   "quux.cc",
					Digest: inputDigest,
				},
			}},
			OutputKey: "quux.a",
		}}
		digest, err := db.WriteUnit(ctx, kcd.Revision{
			Revision: Revision,
			Corpus:   Corpus,
		}, FormatKey, dummy)
		if err != nil {
			fail("WriteUnit", err)
			return
		}

		unitDigest = digest
		if err := db.Units(ctx, []string{digest}, func(gotDigest, gotKey string, data []byte) error {
			if gotDigest != digest {
				fail("Units", fmt.Errorf("got digest %q, want %q", gotDigest, digest))
			}
			if gotKey != FormatKey {
				fail("Units", fmt.Errorf("got key %q, want %q", gotKey, FormatKey))
			}
			var gotUnit apb.CompilationUnit
			if err := proto.Unmarshal(data, &gotUnit); err != nil {
				fail("Units", fmt.Errorf("unmarshaling proto: %v", err))
			} else if !proto.Equal(&gotUnit, dummy.Proto) {
				fail("Units", fmt.Errorf("got %+v, want %+v", &gotUnit, dummy.Proto))
			}
			return nil
		}); err != nil {
			fail("Units", err)
		}

		// Check that required input digests do not exist until their contents are written.
		if err := db.FilesExist(ctx, []string{inputDigest}, func(digest string) error {
			fail("FilesExist", fmt.Errorf("unexpected digest %q", digest))
			return nil
		}); err != nil {
			fail("FilesExist", err)
		}
	})

	check("basic filters match index terms", func(fail failer) {
		tests := []*kcd.FindFilter{
			{Revisions: []string{Revision}},
			{BuildCorpus: []string{Corpus}},
			{Revisions: []string{Revision}, BuildCorpus: []string{Corpus}},
			{Languages: []string{Language}},
			{Targets: regexps(`//foo/bar/baz:\w+`)},
			{Sources: regexps("quux.*")},
			{Outputs: regexps(`quux\.a`)},
		}
		for _, test := range tests {
			var numSeen int
			if err := db.Find(ctx, test, func(got string) error {
				numSeen++
				if got != unitDigest {
					fail("result", fmt.Errorf("on filter %+v; got %q, want %q", test, got, unitDigest))
				}
				return nil
			}); err != nil {
				fail("Find", err)
			}
			if numSeen != 1 {
				fail("result count", fmt.Errorf("on filter %+v; got %d, want %d", test, numSeen, 1))
			}
		}
	})

	check("empty find filter returns nothing", func(fail failer) {
		if err := db.Find(ctx, nil, func(digest string) error {
			fail("Find", fmt.Errorf("unexpected digest %q", digest))
			return nil
		}); err != nil {
			fail("Find", err)
		}
	})

	check("add more revisions", func(fail failer) {
		revs := []kcd.Revision{
			{"5678", Corpus, time.Unix(1, 1)},
			{"9012", Corpus, time.Unix(2, 3)},
			{"3459", "alt", time.Unix(5, 8)},
		}
		for _, rev := range revs {
			if err := db.WriteRevision(ctx, rev, false); err != nil {
				fail("WriteRevision", err)
			}
		}
	})

	check("empty revision filter returns everything", func(fail failer) {
		wantRevs := map[string]*kcd.Revision{
			Revision: nil, "5678": nil, "9012": nil, "3459": nil,
		}
		var numSeen int
		if err := db.Revisions(ctx, nil, func(rev kcd.Revision) error {
			numSeen++
			wantRevs[rev.Revision] = &rev
			return nil
		}); err != nil {
			fail("Revisions", err)
		}
		if numSeen != len(wantRevs) {
			fail("result count", fmt.Errorf("got %d, want %d", numSeen, len(wantRevs)))
		}
		for marker, rev := range wantRevs {
			if rev == nil {
				fail("Revisions", fmt.Errorf("missing revision for %q", marker))
			}
		}
	})

	check("revision filter by corpus", func(fail failer) {
		gotRevs := make(map[string]*kcd.Revision)
		filter := &kcd.RevisionsFilter{Corpus: "alt"}
		if err := db.Revisions(ctx, filter, func(rev kcd.Revision) error {
			gotRevs[rev.Revision] = &rev
			return nil
		}); err != nil {
			fail("Revisions", err)
		}
		if len(gotRevs) != 1 {
			fail("result count", fmt.Errorf("got %d, want %d", len(gotRevs), 1))
		}
		if rev := gotRevs["3459"]; rev == nil {
			fail("Revisions", fmt.Errorf("missing data for 3459\nFilter: %+v", filter))
		} else if rev.Corpus != "alt" {
			fail("Revisions", fmt.Errorf("got corpus %q, want %q", rev.Corpus, "alt"))
		}
	})

	check("revision filter by timestamp", func(fail failer) {
		wantRevs := map[string]*kcd.Revision{"5678": nil, "9012": nil}
		filter := &kcd.RevisionsFilter{Until: time.Unix(2, 3)}
		if err := db.Revisions(ctx, filter, func(rev kcd.Revision) error {
			wantRevs[rev.Revision] = &rev
			return nil
		}); err != nil {
			fail("Revisions", err)
		}
		if len(wantRevs) > 2 {
			fail("result count", fmt.Errorf("got %d, want %d", len(wantRevs), 2))
		}
		for marker, rev := range wantRevs {
			if rev == nil {
				fail("Revisions", fmt.Errorf("missing revision for %q\nFilter: %+v", marker, filter))
			}
		}
	})

	check("revision corpus does not match regexp", func(fail failer) {
		filter := &kcd.RevisionsFilter{Corpus: "a.."} // matches "alt", but shouldn't hit
		if err := db.Revisions(ctx, filter, func(rev kcd.Revision) error {
			return fmt.Errorf("unexpected corpus regexp match\nFilter: %+v\nResult:  %+v", filter, rev)
		}); err != nil {
			fail("Revisions", err)
		}

		wantRevs := map[string]*kcd.Revision{"9012": nil, "3459": nil}
		filter = &kcd.RevisionsFilter{Revision: ".*9.*"}
		if err := db.Revisions(ctx, filter, func(rev kcd.Revision) error {
			wantRevs[rev.Revision] = &rev
			return nil
		}); err != nil {
			fail("Revisions", err)
		}
		for marker, rev := range wantRevs {
			if rev == nil {
				fail("Revisions", fmt.Errorf("missing revision for %q\nFilter: %+v", marker, filter))
			}
		}
	})

	// If db implements the Deleter interface, verify that it works.
	del, ok := db.(kcd.Deleter)
	if !ok {
		return errs
	}

	check("deleting an existing unit succeeds", func(fail failer) {
		if err := del.DeleteUnit(ctx, unitDigest); err != nil {
			fail("DeleteUnit", err)
		}
	})

	check("deleting a nonexistent unit reports an error", func(fail failer) {
		if err := del.DeleteUnit(ctx, "no-such-unit"); err == nil {
			fail("DeleteUnit", errors.New("no error returned for absent digest"))
		} else if !os.IsNotExist(err) {
			fail("DeleteUnit", fmt.Errorf("error %v does not satisfy os.IsNotExist", err))
		}
	})

	check("deleting an existing file succeeds", func(fail failer) {
		if err := del.DeleteFile(ctx, wantDigest); err != nil {
			fail("DeleteFile", err)
		}
	})

	check("deleting a nonexistent file reports an error", func(fail failer) {
		if err := del.DeleteFile(ctx, "no-such-file"); err == nil {
			fail("DeleteFile", errors.New("no error returned for absent file"))
		} else if !os.IsNotExist(err) {
			fail("DeleteFile", fmt.Errorf("error %v does not satisfy os.IsNotExist", err))
		}
	})

	check("deleting an existing revision succeeds", func(fail failer) {
		if err := del.DeleteRevision(ctx, Revision, Corpus); err != nil {
			fail("DeleteRevision", err)
		}
	})

	check("deleting a nonexistent revision reports an error", func(fail failer) {
		if err := del.DeleteRevision(ctx, "nsrev", "nscorp"); err == nil {
			fail("DeleteRevision", errors.New("no error returned for absent revision"))
		} else if !os.IsNotExist(err) {
			fail("DeleteRevision", fmt.Errorf("error %v does not satisfy os.IsNotExist", err))
		}
	})

	return errs
}
