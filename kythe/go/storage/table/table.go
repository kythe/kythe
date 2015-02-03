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

	"kythe/go/storage/keyvalue"

	"code.google.com/p/goprotobuf/proto"
)

// Proto is a key-value direct lookup table with protobuf values.
type Proto interface {
	// Lookup unmarshals the value for the given key into msg, returning any
	// error.  If the key was not found, ErrNoSuchKey is returned.
	Lookup(key []byte, msg proto.Message) error

	// Put marshals msg and writes it as the value for the given key.
	Put(key []byte, msg proto.Message) error
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
