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

// Package table implements lookup table interfaces for protobufs.
package table // import "kythe.io/kythe/go/storage/table"

import (
	"context"
	"errors"
	"fmt"
	"io"

	"kythe.io/kythe/go/storage/keyvalue"

	"google.golang.org/protobuf/proto"
)

// Proto is a key-value direct lookup table with protobuf values.
type Proto interface {
	ProtoLookup

	// Put marshals msg and writes it as the value for the given key.
	Put(ctx context.Context, key []byte, msg proto.Message) error

	// Buffered returns a buffered write interface.
	Buffered() BufferedProto

	// Close release the underlying resources for the table.
	Close(context.Context) error
}

// ProtoLookup is a read-only key-value direct lookup table with protobuf values.
type ProtoLookup interface {
	// Lookup unmarshals the value for the given key into msg, returning any
	// error.  If the key was not found, ErrNoSuchKey is returned.
	Lookup(ctx context.Context, key []byte, msg proto.Message) error

	// Lookup unmarshals the values for the given key into new proto.Message using
	// m, returning any error.  If the key was not found, f is never called.
	LookupValues(ctx context.Context, key []byte, m proto.Message, f func(msg proto.Message) error) error
}

// BufferedProto buffers calls to Put to provide a high throughput write
// interface to a Proto table.
type BufferedProto interface {
	// Put marshals msg and writes it as the value for the given key.
	Put(ctx context.Context, key []byte, msg proto.Message) error

	// Flush sends all buffered writes to the underlying table.
	Flush(ctx context.Context) error
}

// KVProto implements a Proto table using a keyvalue.DB.
type KVProto struct{ keyvalue.DB }

// ErrNoSuchKey is returned when a value was not found for a particular key.
var ErrNoSuchKey = errors.New("no such key")

// ErrStopLookup should be returned from the function passed to LookupValues
// when the client wants no further values.
var ErrStopLookup = errors.New("stop lookup")

// Lookup implements part of the Proto interface.
func (t *KVProto) Lookup(ctx context.Context, key []byte, msg proto.Message) error {
	v, err := t.Get(ctx, key, nil)
	if err == io.EOF {
		return ErrNoSuchKey
	} else if err != nil {
		return err
	} else if err := proto.Unmarshal(v, msg); err != nil {
		return fmt.Errorf("proto unmarshal error: %v", err)
	}
	return nil
}

// LookupValues implements part of the ProtoLookup interface.
func (t *KVProto) LookupValues(ctx context.Context, key []byte, m proto.Message, f func(proto.Message) error) error {
	it, err := t.ScanRange(ctx, keyvalue.KeyRange(key), nil)
	if err != nil {
		return err
	}
	for {
		_, val, err := it.Next()
		if err == io.EOF {
			return nil
		} else if err != nil {
			return err
		}

		msg := m.ProtoReflect().New().Interface()
		if err := proto.Unmarshal(val, msg); err != nil {
			return err
		}

		if err := f(msg); err == ErrStopLookup {
			return nil
		} else if err != nil {
			return err
		}
	}
	return nil
}

// Put implements part of the Proto interface.
func (t *KVProto) Put(ctx context.Context, key []byte, msg proto.Message) error {
	b := t.Buffered()
	if err := b.Put(ctx, key, msg); err != nil {
		return err
	}
	return b.Flush(ctx)
}

type kvProtoBuffer struct{ pool *keyvalue.WritePool }

// Put implements part of the BufferedProto interface.
func (b *kvProtoBuffer) Put(ctx context.Context, key []byte, msg proto.Message) error {
	rec, err := proto.Marshal(msg)
	if err != nil {
		return err
	}
	return b.pool.Write(ctx, key, rec)
}

// Flush implements part of the BufferedProto interface.
func (b *kvProtoBuffer) Flush(_ context.Context) error { return b.pool.Flush() }

// Buffered implements part of the Proto interface.
func (t *KVProto) Buffered() BufferedProto { return &kvProtoBuffer{keyvalue.NewPool(t.DB, nil)} }

// Close implements part of the Proto interface.
func (t *KVProto) Close(ctx context.Context) error { return t.DB.Close(ctx) }
