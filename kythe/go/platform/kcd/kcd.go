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

// Package kcd defines an interface and utility functions for the
// implementation of a Kythe compilation database.
//
// Design documentation: kythe/docs/kythe-compilation-database.txt
package kcd // import "kythe.io/kythe/go/platform/kcd"

import (
	"context"
	"crypto/sha256"
	"encoding"
	"encoding/hex"
	"encoding/json"
	"fmt"
	"io"
	"regexp"
	"strings"
	"time"

	spb "kythe.io/kythe/proto/storage_go_proto"
)

// Reader represents read-only access to an underlying storage layer used to
// implement a compilation database.
type Reader interface {
	// Revisions calls f with each known revision matching the given filter.
	// If filter == nil or is empty, all known revisions are reported.
	// If f returns an error, that error is returned from Revisions.
	// Timestamps are returned in UTC.
	Revisions(_ context.Context, filter *RevisionsFilter, f func(Revision) error) error

	// Find calls f with the digest of each known compilation matching filter.
	// If filter == nil or is empty, no compilations are reported.
	// If f returns an error, that error is returned from Find.
	Find(_ context.Context, filter *FindFilter, f func(string) error) error

	// Units calls f with the digest, format key, and content of each specified
	// compilation that exists in the store.
	// If f returns an error, that error is returned from Units.
	Units(_ context.Context, unitDigests []string, f func(digest, key string, data []byte) error) error

	// Files calls f with the digest and content of each specified file that
	// exists in the store.
	// If f returns an error, that error is returned from Files.
	Files(_ context.Context, fileDigests []string, f func(string, []byte) error) error

	// FilesExist calls f with the digest of each specified file that exists in
	// the store.
	// If f returns an error, that error is returned from FilesExist.
	FilesExist(_ context.Context, fileDigests []string, f func(string) error) error
}

// Writer represents write access to an underlying storage layer used to
// implement a compilation database.
type Writer interface {
	// WriteRevision records the specified revision into the store at the given
	// timestamp.  It is an error if rev.Revision == "" or rev.Corpus == "".
	// If replace is true, any previous version of this marker is discarded
	// before writing.
	WriteRevision(_ context.Context, rev Revision, replace bool) error

	// WriteUnit records unit in the store at the given revision, and returns
	// the digest of the stored unit.  It is an error if rev is invalid per
	// WriteRevision.
	WriteUnit(_ context.Context, rev Revision, formatKey string, unit Unit) (string, error)

	// WriteFile fully reads r and records its content as a file in the store.
	// Returns the digest of the stored file.
	WriteFile(_ context.Context, r io.Reader) (string, error)
}

// ReadWriter expresses the capacity to both read and write a compilation database.
type ReadWriter interface {
	Reader
	Writer
}

// Deleter expresses the capacity to delete data from a compilation database.
// Each of the methods of this interface should return an error that satisfies
// os.IsNotExist if its argument does not match any known entries.
// Not all writable databases must support this interface.
type Deleter interface {
	// DeleteUnit removes the specified unit from the database.
	DeleteUnit(_ context.Context, unitDigest string) error

	// DeleteFile removes the specified file from the database.
	DeleteFile(_ context.Context, fileDigest string) error

	// DeleteRevision removes the specified revision marker from the database.
	// It is an error if rev == "" or corpus == "".  All timestamps for the
	// matching revision are discarded.
	DeleteRevision(_ context.Context, revision, corpus string) error
}

// ReadWriteDeleter expresses the capacity to read, write, and delete data in a
// compilation database.
type ReadWriteDeleter interface {
	Reader
	Writer
	Deleter
}

// RevisionsFilter gives constraints on which revisions are matched by a call to
// the Revisions method of compdb.Reader.
type RevisionsFilter struct {
	Revision string    // If set return revision markers matching this RE2.
	Corpus   string    // If set, return only revisions for this corpus.
	Until    time.Time // If nonzero, return only revisions at or before this time.
	Since    time.Time // If nonzero, return only revisions at or after this time.
}

// Compile compiles the filter into a matching function that reports whether
// its argument matches the original filter.
func (rf *RevisionsFilter) Compile() (func(Revision) bool, error) {
	// A nil or empty filter matches all revisions.
	if rf == nil || (rf.Revision == "" && rf.Corpus == "" && rf.Until.IsZero() && rf.Since.IsZero()) {
		return func(Revision) bool { return true }, nil
	}
	var matchRevision, matchCorpus func(...string) bool
	var err error
	if matchRevision, err = singleMatcher(rf.Revision); err != nil {
		return nil, err
	}
	if matchCorpus, err = singleMatcher(regexp.QuoteMeta(rf.Corpus)); err != nil {
		return nil, err
	}
	return func(rev Revision) bool {
		return matchRevision(rev.Revision) &&
			matchCorpus(rev.Corpus) &&
			(rf.Until.IsZero() || !rf.Until.Before(rev.Timestamp.In(time.UTC))) &&
			(rf.Since.IsZero() || !rev.Timestamp.In(time.UTC).Before(rf.Since))
	}, nil
}

// A Revision represents a single revision stored in the database.
type Revision struct {
	Revision, Corpus string
	Timestamp        time.Time
}

func (r Revision) String() string {
	return fmt.Sprintf("#<rev %q corpus=%q at %v>", r.Revision, r.Corpus, r.Timestamp)
}

// IsValid returns an error if the revision or corpus fields are invalid.
func (r Revision) IsValid() error {
	if !IsRevisionValid(r.Revision) {
		return fmt.Errorf("invalid revision %q", r.Revision)
	} else if !IsCorpusValid(r.Corpus) {
		return fmt.Errorf("invalid corpus %q", r.Corpus)
	}
	return nil
}

// IsRevisionValid reports whether r is a valid revision marker.
// A marker is valid if it is nonempty and does not contain whitespace.
func IsRevisionValid(r string) bool { return r != "" && !containsWhitespace(r) }

// IsCorpusValid reports whether c is a valid corpus marker.
// A corpus is valid if it is nonempty and does not contain whitespace.
func IsCorpusValid(c string) bool { return c != "" && !containsWhitespace(c) }

// FindFilter gives constraints on which compilations are matched by a call to
// the Find method of compdb.Reader.
type FindFilter struct {
	UnitCorpus  []string // Include only these unit corpus labels (exact match)
	Revisions   []string // Include only these revisions (exact match).
	Languages   []string // Include only these languages (Kythe language names).
	BuildCorpus []string // Include only these build corpus labels (exact match).

	Targets []*regexp.Regexp // Include only compilations for these targets.
	Sources []*regexp.Regexp // Include only compilations for these sources.
	Outputs []*regexp.Regexp // include only compilations for these outputs.
}

// Compile returns a compiled filter that matches index terms based on ff.
// Returns nil if ff is an empty filter.
func (ff *FindFilter) Compile() (*CompiledFilter, error) {
	if ff.IsEmpty() {
		return nil, nil
	}
	var cf CompiledFilter
	var err error
	cf.RevisionMatches, err = stringMatcher(ff.Revisions...)
	if err != nil {
		return nil, err
	}
	cf.BuildCorpusMatches, err = stringMatcher(ff.BuildCorpus...)
	if err != nil {
		return nil, err
	}
	cf.UnitCorpusMatches, err = stringMatcher(ff.UnitCorpus...)
	if err != nil {
		return nil, err
	}
	cf.LanguageMatches, err = stringMatcher(ff.Languages...)
	if err != nil {
		return nil, err
	}
	cf.TargetMatches, err = combineRegexps(ff.Targets)
	if err != nil {
		return nil, err
	}
	cf.OutputMatches, err = combineRegexps(ff.Outputs)
	if err != nil {
		return nil, err
	}
	cf.SourcesMatch, err = combineRegexps(ff.Sources)
	if err != nil {
		return nil, err
	}
	return &cf, nil
}

// IsEmpty reports whether f is an empty filter, meaning it specifies no
// non-empty query terms.
func (ff *FindFilter) IsEmpty() bool {
	return ff == nil ||
		(len(ff.Revisions) == 0 && len(ff.Languages) == 0 && len(ff.BuildCorpus) == 0 &&
			len(ff.Targets) == 0 && len(ff.Sources) == 0 && len(ff.Outputs) == 0) && len(ff.UnitCorpus) == 0
}

// The Unit interface expresses the capabilities required to represent a
// compilation unit in a data store.
type Unit interface {
	encoding.BinaryMarshaler
	json.Marshaler

	// Index returns a the indexable terms of this unit.
	Index() Index

	// Canonicalize organizes the unit into a canonical form.  The meaning of
	// canonicalization is unit-dependent, and may safely be a no-op.
	Canonicalize()

	// Digest produces a unique string representation of a unit sufficient to
	// serve as a content-addressable digest.
	Digest() string

	// LookupVName looks up and returns the VName for the given file path
	// or nil if it could not be found or the operation is unsupported.
	LookupVName(path string) *spb.VName
}

// Index represents the indexable terms of a compilation.
type Index struct {
	Corpus   string   // The Kythe corpus name, e.g., "kythe"
	Language string   // The Kythe language name, e.g., "c++".
	Output   string   // The output name, e.g., "bazel-out/foo.o".
	Inputs   []string // The digests of all required inputs.
	Sources  []string // The paths of all source files.
	Target   string   // The target name, e.g., "//file/base/go:file".
}

// HexDigest computes a hex-encoded SHA256 digest of data.
func HexDigest(data []byte) string {
	sum := sha256.Sum256(data)
	return hex.EncodeToString(sum[:])
}

// IsValidDigest reports whether s is valid as a digest computed by the
// HexDigest function. It does not check whether s could have actually been
// generated by the hash function, only the structure of the value.
func IsValidDigest(s string) bool {
	if len(s) != 2*sha256.Size {
		return false
	}
	for i := 0; i < len(s); i++ {
		if !isLowerHex(s[i]) {
			return false
		}
	}
	return true
}

func isLowerHex(b byte) bool { return ('0' <= b && b <= '9') || ('a' <= b && b <= 'f') }

// A CompiledFilter is a collection of matchers compiled from a FindFilter.
type CompiledFilter struct {
	RevisionMatches    func(...string) bool
	BuildCorpusMatches func(...string) bool
	UnitCorpusMatches  func(...string) bool
	LanguageMatches    func(...string) bool
	TargetMatches      func(...string) bool
	OutputMatches      func(...string) bool
	SourcesMatch       func(...string) bool
}

// matcher returns a function that reports whether any of its string arguments
// is matched by at least one of the given regular expressions.  Matches are
// implicitly anchored at both ends.
//
// If quote != nil, it is applied to each expression before compiling it.
// If there are no expressions, the matcher returns true.
func matcher(exprs []string, quote func(string) string) (func(...string) bool, error) {
	if len(exprs) == 0 {
		return func(...string) bool { return true }, nil
	} else if quote == nil {
		quote = func(s string) string { return s }
	}
	prep := make([]string, len(exprs))
	for i, expr := range exprs {
		prep[i] = `(?:` + quote(expr) + `)`
	}
	re, err := regexp.Compile(`^(?:` + strings.Join(prep, "|") + `)$`)
	if err != nil {
		return nil, err
	}
	return func(values ...string) bool {
		for _, value := range values {
			if re.MatchString(value) {
				return true
			}
		}
		return false
	}, nil
}

// singleMatcher is a shortcut for a match with a single non-empty expression.
func singleMatcher(single string) (func(...string) bool, error) {
	if single == "" {
		return func(...string) bool { return true }, nil
	}
	return matcher([]string{single}, nil)
}

// stringMatcher is a shortcut for matcher(exprs, regexp.QuoteMeta).
func stringMatcher(exprs ...string) (func(...string) bool, error) {
	return matcher(exprs, regexp.QuoteMeta)
}

// combineRegexps constructs a matcher that accepts the disjunction of its
// input expressions.
func combineRegexps(res []*regexp.Regexp) (func(...string) bool, error) {
	exprs := make([]string, len(res))
	for i, re := range res {
		exprs[i] = re.String()
	}
	return matcher(exprs, nil)
}

// containsWhitespace reports whether s contains whitespace characters.
func containsWhitespace(s string) bool { return strings.ContainsAny(s, "\t\n\v\f\r ") }
