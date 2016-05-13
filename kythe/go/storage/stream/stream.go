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

// Package stream provides utility functions to consume Entry streams.
package stream

import (
	"encoding/json"
	"fmt"
	"io"
	"log"

	"kythe.io/kythe/go/platform/delimited"

	spb "kythe.io/kythe/proto/storage_proto"
)

// EntryReader functions read a stream of entries, passing each to a handler
// function.
type EntryReader func(func(*spb.Entry) error) error

// ReadEntries reads a stream of Entry protobufs from r.
func ReadEntries(r io.Reader) <-chan *spb.Entry {
	ch := make(chan *spb.Entry)
	go func() {
		defer close(ch)
		if err := NewReader(r)(func(e *spb.Entry) error {
			ch <- e
			return nil
		}); err != nil {
			log.Fatal(err)
		}
	}()
	return ch
}

// NewReader reads a stream of Entry protobufs from r.
func NewReader(r io.Reader) EntryReader {
	return func(f func(*spb.Entry) error) error {
		rd := delimited.NewReader(r)
		for {
			var entry spb.Entry
			if err := rd.NextProto(&entry); err == io.EOF {
				return nil
			} else if err != nil {
				return fmt.Errorf("error decoding Entry: %v", err)
			}
			if err := f(&entry); err != nil {
				return err
			}
		}
	}
}

// ReadJSONEntries reads a JSON stream of Entry protobufs from r.
func ReadJSONEntries(r io.Reader) <-chan *spb.Entry {
	ch := make(chan *spb.Entry)
	go func() {
		defer close(ch)
		if err := NewJSONReader(r)(func(e *spb.Entry) error {
			ch <- e
			return nil
		}); err != nil {
			log.Fatal(err)
		}
	}()
	return ch
}

// NewJSONReader reads a JSON stream of Entry protobufs from r.
func NewJSONReader(r io.Reader) EntryReader {
	return func(f func(*spb.Entry) error) error {
		de := json.NewDecoder(r)
		for {
			var entry spb.Entry
			if err := de.Decode(&entry); err == io.EOF {
				return nil
			} else if err != nil {
				return fmt.Errorf("error decoding JSON Entry: %v", err)
			}
			if err := f(&entry); err != nil {
				return err
			}
		}
	}
}
