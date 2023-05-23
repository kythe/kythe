/*
 * Copyright 2014 The Kythe Authors. All rights reserved.
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
package stream // import "kythe.io/kythe/go/storage/stream"

import (
	"encoding/json"
	"fmt"
	"io"

	"kythe.io/kythe/go/platform/delimited"
	"kythe.io/kythe/go/util/log"
	"kythe.io/kythe/go/util/schema/facts"

	"google.golang.org/protobuf/encoding/protojson"
	"google.golang.org/protobuf/proto"

	cpb "kythe.io/kythe/proto/common_go_proto"
	spb "kythe.io/kythe/proto/storage_go_proto"
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
			if err := f((*spb.Entry)(&entry)); err != nil {
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

// StructuredEntry has custom marshaling behavior to handle structured FactValues
type StructuredEntry spb.Entry

// Reset calls the implementation for Entry
func (r *StructuredEntry) Reset() {
	(*spb.Entry)(r).Reset()
}

// String calls the implementation for Entry
func (r *StructuredEntry) String() string {
	return (*spb.Entry)(r).String()
}

// ProtoMessage calls the implementation for Entry
func (r *StructuredEntry) ProtoMessage() {
	(*spb.Entry)(r).ProtoMessage()
}

// NewStructuredJSONReader reads a JSON stream of StructuredEntry protobufs from r.
func NewStructuredJSONReader(r io.Reader) EntryReader {
	return func(f func(*spb.Entry) error) error {
		de := json.NewDecoder(r)
		for {
			var entry StructuredEntry
			if err := de.Decode(&entry); err == io.EOF {
				return nil
			} else if err != nil {
				return fmt.Errorf("error decoding JSON Entry: %v", err)
			}
			if err := f((*spb.Entry)(&entry)); err != nil {
				return err
			}
		}
	}
}

// NewJSONReader reads a JSON stream of Entry protobufs from r.
func NewJSONReader(r io.Reader) EntryReader {
	return func(f func(*spb.Entry) error) error {
		de := json.NewDecoder(r)
		for {
			var raw json.RawMessage
			if err := de.Decode(&raw); err == io.EOF {
				return nil
			} else if err != nil {
				return fmt.Errorf("error decoding JSON Entry: %w", err)
			}
			var entry spb.Entry
			if err := protojson.Unmarshal(raw, &entry); err != nil {
				return fmt.Errorf("error decoding JSON Entry: %w", err)
			}
			if err := f(&entry); err != nil {
				return err
			}
		}
	}
}

var marshaler = protojson.MarshalOptions{UseProtoNames: true}

// richJSONEntry delays the unmarshaling of the fact_value field
type richJSONEntry struct {
	Source    json.RawMessage `json:"source,omitempty"`
	Target    json.RawMessage `json:"target,omitempty"`
	EdgeKind  string          `json:"edge_kind,omitempty"`
	FactName  string          `json:"fact_name,omitempty"`
	FactValue json.RawMessage `json:"fact_value,omitempty"`
}

// StructuredFactValueJSON creates a json object from e.FactValue
func StructuredFactValueJSON(e *spb.Entry) (json.RawMessage, error) {
	if e.FactName == facts.Code {
		var ms cpb.MarkedSource
		if err := proto.Unmarshal(e.FactValue, &ms); err != nil {
			return nil, err
		}
		rec, err := marshaler.Marshal(&ms)
		if err != nil {
			return nil, err
		}
		return rec, nil
	}
	return json.Marshal(e.FactValue)
}

// Structured creates an entry that serializes factValue to a full value
func Structured(e *spb.Entry) *StructuredEntry {
	return (*StructuredEntry)(e)
}

// UnmarshalJSON unmarshals r including an object representation of FactValue when appropriate
func (r *StructuredEntry) UnmarshalJSON(data []byte) error {
	var jsonEntry richJSONEntry
	if err := json.Unmarshal(data, &jsonEntry); err != nil {
		return err
	}
	if jsonEntry.FactName == facts.Code {
		var ms cpb.MarkedSource
		if err := protojson.Unmarshal(jsonEntry.FactValue, &ms); err != nil {
			return err
		}
		pb, err := proto.Marshal(&ms)
		if err != nil {
			return err
		}
		r.FactValue = pb
	} else if err := json.Unmarshal(jsonEntry.FactValue, &r.FactValue); err != nil {
		return err
	}

	r.EdgeKind = jsonEntry.EdgeKind
	r.FactName = jsonEntry.FactName

	var err error
	if r.Source, err = unmarshalVName(jsonEntry.Source); err != nil {
		return err
	} else if r.Target, err = unmarshalVName(jsonEntry.Target); err != nil {
		return err
	}

	return nil
}

func marshalVName(v *spb.VName) (json.RawMessage, error) {
	if v == nil {
		return nil, nil
	}
	return protojson.Marshal(v)
}

func unmarshalVName(msg json.RawMessage) (*spb.VName, error) {
	if len(msg) == 0 {
		return nil, nil
	}
	var v spb.VName
	return &v, protojson.Unmarshal(msg, &v)
}

// MarshalJSON marshals r including an object representation of FactValue when appropriate
func (r *StructuredEntry) MarshalJSON() ([]byte, error) {
	jsonEntry := richJSONEntry{
		EdgeKind: r.EdgeKind,
		FactName: r.FactName,
	}
	var err error
	if jsonEntry.Source, err = marshalVName(r.Source); err != nil {
		return nil, err
	} else if jsonEntry.Target, err = marshalVName(r.Target); err != nil {
		return nil, err
	} else if jsonEntry.FactValue, err = StructuredFactValueJSON((*spb.Entry)(r)); err != nil {
		return nil, err
	}

	return json.Marshal(jsonEntry)
}
