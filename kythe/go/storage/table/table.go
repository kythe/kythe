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

// Package table implements lookup table interfaces for protobufs.
package table

import (
	"bytes"
	"errors"
	"fmt"
	"io"

	"kythe.io/kythe/go/storage/keyvalue"

	"github.com/golang/protobuf/proto"
)

// Proto is a key-value direct lookup table with protobuf values.
type Proto interface {
	// Lookup unmarshals the value for the given key into msg, returning any
	// error.  If the key was not found, ErrNoSuchKey is returned.
	Lookup(key []byte, msg proto.Message) error

	// Put marshals msg and writes it as the value for the given key.
	Put(key []byte, msg proto.Message) error
}

// Inverted is an inverted index lookup table for []byte values with associated
// []byte keys.  Keys and values should not contain \000 bytes.
type Inverted interface {
	// Lookup returns a slice of []byte keys associated with the given value.
	Lookup(val []byte) ([][]byte, error)

	// Contains determines whether there is an association between key and val
	Contains(key, val []byte) (bool, error)

	// Put writes an entry associating val with key.
	Put(key, val []byte) error
}

// KVProto implements a Proto table using a keyvalue.DB.
type KVProto struct{ keyvalue.DB }

// ErrNoSuchKey is returned when a value was not found for a particular key.
var ErrNoSuchKey = errors.New("no such key")

// Lookup implements part of the Proto interface.
func (t *KVProto) Lookup(key []byte, msg proto.Message) error {
	iter, err := t.ScanPrefix(key, nil)
	if err != nil {
		return fmt.Errorf("table iterator error: %v", err)
	}
	defer iter.Close()
	k, v, err := iter.Next()
	if err == io.EOF {
		return ErrNoSuchKey
	} else if err != nil {
		return err
	} else if !bytes.Equal(key, k) {
		return ErrNoSuchKey
	}
	if err := proto.Unmarshal(v, msg); err != nil {
		return fmt.Errorf("proto unmarshal error: %v", err)
	}
	return nil
}

// Put implements part of the Proto interface.
func (t *KVProto) Put(key []byte, msg proto.Message) error {
	rec, err := proto.Marshal(msg)
	if err != nil {
		return err
	}
	wr, err := t.Writer()
	if err != nil {
		return err
	}
	defer wr.Close()
	return wr.Write(key, rec)
}

// KVInverted implements an Inverted table in a keyvalue.DB.
type KVInverted struct{ keyvalue.DB }

// Lookup implements part of the Inverted interface.
func (i *KVInverted) Lookup(val []byte) ([][]byte, error) {
	prefix := invertedPrefix(val)
	iter, err := i.ScanPrefix(prefix, nil)
	if err != nil {
		return nil, fmt.Errorf("table iterator error: %v", err)
	}
	defer iter.Close()

	var results [][]byte
	for {
		k, _, err := iter.Next()
		if err == io.EOF {
			return results, nil
		} else if err != nil {
			return nil, err
		}

		if !bytes.HasPrefix(k, prefix) {
			return results, nil
		}

		i := bytes.IndexByte(k, invertedKeySep)
		if i == -1 {
			return nil, fmt.Errorf("invalid index key: %q", string(k))
		}
		results = append(results, k[i+1:])
	}
}

// Contains implements part of the Inverted interface.
func (i *KVInverted) Contains(key, val []byte) (bool, error) {
	iKey := invertedKey(key, val)
	iter, err := i.ScanPrefix(iKey, nil)
	if err != nil {
		return false, fmt.Errorf("table iterator error: %v", err)
	}
	defer iter.Close()

	k, _, err := iter.Next()
	if err == io.EOF {
		return false, nil
	} else if err != nil {
		return false, err
	}

	return bytes.Equal(k, iKey), nil
}

var emptyValue = []byte{}

// Put implements part of the Inverted interface.
func (i *KVInverted) Put(key, val []byte) error {
	wr, err := i.Writer()
	if err != nil {
		return err
	}
	defer wr.Close()
	return wr.Write(invertedKey(key, val), emptyValue)
}

const invertedKeySep = '\000'

func invertedKey(key, val []byte) []byte {
	var buf bytes.Buffer
	buf.Grow(len(val) + 1 + len(key))
	buf.Write(val)
	buf.WriteByte(invertedKeySep)
	buf.Write(key)
	return buf.Bytes()
}

func invertedPrefix(val []byte) []byte {
	var buf bytes.Buffer
	buf.Grow(len(val) + 1)
	buf.Write(val)
	buf.WriteByte(invertedKeySep)
	return buf.Bytes()
}
