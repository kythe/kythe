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

// Package dedup implements a duplication-reducing reader for streams of
// length-delimited byte records.  Each record is read as a varint-encoded
// length in bytes, followed immediately by the record itself.
//
// A stream consists of a sequence of such records packed consecutively without
// additional padding.  There are no checksums or compression.
// See also: kythe.io/kythe/go/platform/delimited.
package dedup // import "kythe.io/kythe/go/platform/delimited/dedup"

import (
	"io"

	"kythe.io/kythe/go/platform/delimited"
	"kythe.io/kythe/go/util/dedup"

	"github.com/golang/protobuf/proto"
)

// Reader implements the Reader interface.  Duplicate records are removed by
// hashing each and checking against a set of known record hashes.  This is a
// quick-and-dirty method of removing duplicates; it will not be perfect.
type Reader struct {
	r *delimited.Reader
	d *dedup.Deduper
}

// NewReader returns a reader that consumes records from r, using a cache of up
// to maxSize bytes for known record hashes.
func NewReader(r io.Reader, maxSize int) (*Reader, error) {
	d, err := dedup.New(maxSize)
	if err != nil {
		return nil, err
	}
	return &Reader{delimited.NewReader(r), d}, nil
}

// Next returns the next length-delimited record from the input, or io.EOF if
// there are no more records available.  Returns io.ErrUnexpectedEOF if a short
// record is found, with a length of n but fewer than n bytes of data.  Because
// there is no resynchronization mechanism, it is generally not possible to
// recover from a short record in this format.
//
// The slice returned is valid only until a subsequent call to Next.
func (u *Reader) Next() ([]byte, error) {
	for {
		rec, err := u.r.Next()
		if err != nil {
			return nil, err
		} else if u.d.IsUnique(rec) {
			return rec, nil
		}
	}
}

// NextProto consumes the next available record by calling r.Next, and decodes
// it into pb with proto.Unmarshal.
func (u *Reader) NextProto(pb proto.Message) error {
	rec, err := u.Next()
	if err != nil {
		return err
	}
	return proto.Unmarshal(rec, pb)
}

// Skipped returns the number of records that have been skipped so far by the
// deduplication process.
func (u *Reader) Skipped() uint64 { return u.d.Duplicates() }
