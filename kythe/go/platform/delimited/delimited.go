/*
 * Copyright 2014 Google Inc. All rights reserved.
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

// Package delimited implements a reader and writer for simple streams of
// length-delimited byte records.  Each record is written as a varint-encoded
// length in bytes, followed immediately by the record itself.
//
// A stream consists of a sequence of such records packed consecutively without
// additional padding.  There are no checksums or compression.
package delimited

import (
	"bufio"
	"crypto/sha512"
	"encoding/binary"
	"fmt"
	"io"

	"github.com/golang/protobuf/proto"
)

// Reader consumes length-delimited records from a byte source.
//
// Usage:
//   rd := delimited.NewReader(r)
//   for {
//     rec, err := rd.Next()
//     if err == io.EOF {
//       break
//     } else if err != nil {
//       log.Fatal(err)
//     }
//     doStuffWith(rec)
//   }
//
type Reader interface {
	// Next returns the next length-delimited record from the input, or io.EOF if
	// there are no more records available.  Returns io.ErrUnexpectedEOF if a
	// short record is found, with a length of n but fewer than n bytes of data.
	// Because there is no resynchronization mechanism, it is generally not
	// possible to recover from a short record in this format.
	//
	// The slice returned is valid only until a subsequent call to Next.
	Next() ([]byte, error)

	// NextProto reads a record using Next and decodes it into the given
	// proto.Message.
	NextProto(pb proto.Message) error
}

type reader struct {
	buf  *bufio.Reader
	data []byte
}

// Next implements part of the Reader interface.
func (r *reader) Next() ([]byte, error) {
	size, err := binary.ReadUvarint(r.buf)
	if err != nil {
		return nil, err
	}
	if cap(r.data) < int(size) {
		r.data = make([]byte, size)
	} else {
		r.data = r.data[:size]
	}

	if _, err := io.ReadFull(r.buf, r.data); err != nil {
		return nil, err
	}
	return r.data, nil
}

// NextProto implements part of the Reader interface.
func (r *reader) NextProto(pb proto.Message) error {
	rec, err := r.Next()
	if err != nil {
		return err
	}
	return proto.Unmarshal(rec, pb)
}

// NewReader constructs a new delimited Reader for the records in r.
func NewReader(r io.Reader) Reader { return &reader{buf: bufio.NewReader(r)} }

// UniqReader implements the Reader interface.  Duplicate records are removed by
// hashing each and checking against a set of known record hashes.  This is a
// quick-and-dirty method of removing duplicates; it will not be perfect.
type UniqReader struct {
	r Reader

	pri, sec map[[uniqHashSize]byte]struct{}
	maxSize  int // maximum number of entries in each half of the hash cache

	skipped uint64
}

const uniqHashSize = sha512.Size384

// NewUniqReader returns a UniqReader over the given Reader.  maxSize is the
// maximum byte size of the cache of known record hashes.
func NewUniqReader(r Reader, maxSize int) (*UniqReader, error) {
	max := maxSize / uniqHashSize / 2
	if max <= 0 {
		return nil, fmt.Errorf("invalid cache size: %d (must be at least %d)", maxSize, uniqHashSize)
	}
	return &UniqReader{
		r:       r,
		pri:     make(map[[uniqHashSize]byte]struct{}),
		sec:     make(map[[uniqHashSize]byte]struct{}),
		maxSize: max,
	}, nil
}

// Next implements part of the Reader interface.
func (u *UniqReader) Next() ([]byte, error) {
	for {
		rec, err := u.r.Next()
		if err != nil {
			return nil, err
		}

		hash := sha512.Sum384(rec)
		if u.seen(hash) {
			u.skipped++
			continue
		}

		if len(u.sec) == u.maxSize {
			u.pri = u.sec
			u.sec = make(map[[uniqHashSize]byte]struct{})
		}
		if len(u.pri) == u.maxSize {
			u.sec[hash] = struct{}{}
		} else {
			u.pri[hash] = struct{}{}
		}

		return rec, err
	}
}

func (u *UniqReader) seen(hash [uniqHashSize]byte) bool {
	if _, ok := u.pri[hash]; ok {
		return true
	}
	_, ok := u.sec[hash]
	return ok
}

// NextProto implements part of the Reader interface.
func (u *UniqReader) NextProto(pb proto.Message) error {
	rec, err := u.Next()
	if err != nil {
		return err
	}
	return proto.Unmarshal(rec, pb)
}

// Skipped returns the number of skipped records.
func (u *UniqReader) Skipped() uint64 { return u.skipped }

// Copy writes each record read from rd to wr until rd returns io.EOF or an
// error occurs.
func Copy(wr Writer, rd Reader) error {
	for {
		rec, err := rd.Next()
		if err == io.EOF {
			return nil
		} else if err != nil {
			return fmt.Errorf("read error: %v", err)
		}

		if err := wr.Put(rec); err != nil {
			return fmt.Errorf("write error: %v", err)
		}
	}
}

// A Writer outputs delimited records to an io.Writer.
//
// Basic usage:
//   wr := delimited.NewWriter(w)
//   for record := range records {
//      if err := wr.Put(record); err != nil {
//        log.Fatal(err)
//      }
//   }
//
type Writer interface {
	io.Writer

	// Put writes the specified record to the writer.  It equivalent to Write, but
	// discards the number of bytes written.
	Put(record []byte) error

	// PutProto encodes and writes the specified proto.Message to the writer.
	PutProto(msg proto.Message) error
}

type writer struct{ w io.Writer }

// PutProto implements part of the Writer interface.
func (w *writer) PutProto(msg proto.Message) error {
	rec, err := proto.Marshal(msg)
	if err != nil {
		return fmt.Errorf("error encoding proto: %v", err)
	}
	return w.Put(rec)
}

// Put implements part of the Writer interface.
func (w *writer) Put(record []byte) error {
	_, err := w.Write(record)
	return err
}

// Write writes the specified record to the underlying writer, returning the
// total number of bytes written including the length tag.  This method also
// satisfies io.Writer.
func (w *writer) Write(record []byte) (int, error) {
	var buf [binary.MaxVarintLen64]byte
	v := binary.PutUvarint(buf[:], uint64(len(record)))

	nw, err := w.w.Write(buf[:v])
	if err != nil {
		return 0, err
	}
	dw, err := w.w.Write(record)
	if err != nil {
		return nw, err
	}
	return nw + dw, nil
}

// NewWriter constructs a new delimited Writer that writes records to w.
func NewWriter(w io.Writer) Writer { return &writer{w: w} }
