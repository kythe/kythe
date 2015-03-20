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
	"io"
	"log"

	"kythe.io/kythe/go/platform/delimited"

	spb "kythe.io/kythe/proto/storage_proto"
)

// ReadEntries reads a stream of Entry protobufs from r.
func ReadEntries(r io.Reader) <-chan *spb.Entry {
	ch := make(chan *spb.Entry)
	go func() {
		defer close(ch)
		rd := delimited.NewReader(r)
		for {
			var entry spb.Entry
			if err := rd.NextProto(&entry); err == io.EOF {
				break
			} else if err != nil {
				log.Fatalf("Error decoding Entry: %v", err)
			}
			ch <- &entry
		}
	}()
	return ch
}

// ReadJSONEntries reads a JSON stream of Entry protobufs from r.
func ReadJSONEntries(r io.Reader) <-chan *spb.Entry {
	ch := make(chan *spb.Entry)
	go func() {
		defer close(ch)
		de := json.NewDecoder(r)
		for {
			var entry spb.Entry
			if err := de.Decode(&entry); err == io.EOF {
				break
			} else if err != nil {
				log.Fatalf("Error decoding Entry: %v", err)
			}
			ch <- &entry
		}
	}()
	return ch
}
