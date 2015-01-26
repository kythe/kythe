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

// Package keyvalue implements a generic GraphStore for anything that implements
// the DB interface.
package keyvalue

import (
	"bytes"
	"fmt"
	"io"
	"strings"

	"kythe/go/storage"

	spb "kythe/proto/storage_proto"
)

// A Store implements the storage.GraphStore interface for a keyvalue DB
type Store struct {
	db DB
}

// NewGraphStore returns a GraphStore backed by the given keyvalue DB.
func NewGraphStore(db DB) *Store {
	return &Store{db}
}

// A DB is a sorted key-value store with read/write access. DBs must be Closed
// when no longer used to ensure resources are not leaked.
type DB interface {
	io.Closer

	// Reader returns an Iterator for all key-values starting with the given
	// key prefix.  Options may be nil to use the defaults.
	Reader([]byte, *Options) (Iterator, error)
	// Writer return a new write-access object
	Writer() (Writer, error)
}

// Options alters the behavior of an Iterator.
type Options struct {
	// LargeRead expresses the client's intent that the read will likely be
	// "large" and the implementation should usually avoid certain behaviors such
	// as caching the entire visited key-value range.  Defaults to false.
	LargeRead bool
}

// IsLargeRead returns the LargeRead option or the default of false when o==nil.
func (o *Options) IsLargeRead() bool {
	return o != nil && o.LargeRead
}

// Iterator provides sequential access to a DB. Iterators must be Closed when
// no longer used to ensure that resources are not leaked.
type Iterator interface {
	io.Closer

	// Next returns the currently positioned key-value entry and moves to the next
	// entry. If there is no key-value entry to return, an io.EOF error is
	// returned.
	Next() (key, val []byte, err error)
}

// Writer provides write access to a DB. Writes must be Closed when no longer
// used to ensure that resources are not leaked.
type Writer interface {
	io.Closer

	// Write writes a key-value entry to the DB. Writes may be batched until the
	// Writer is Closed.
	Write(key, val []byte) error
}

// Read implements part of the GraphStore interface.
func (s *Store) Read(req *spb.ReadRequest, stream chan<- *spb.Entry) error {
	keyPrefix, err := KeyPrefix(req.Source, req.GetEdgeKind())
	if err != nil {
		return fmt.Errorf("invalid ReadRequest: %v", err)
	}
	iter, err := s.db.Reader(keyPrefix, nil)
	if err != nil {
		return fmt.Errorf("db seek error: %v", err)
	}
	defer iter.Close()
	for {
		key, val, err := iter.Next()
		if err == io.EOF {
			break
		} else if err != nil {
			return fmt.Errorf("db iteration error: %v", err)
		}

		entry, err := Entry(key, val)
		if err != nil {
			return fmt.Errorf("encoding error: %v", err)
		}
		stream <- entry
	}
	return nil
}

// Write implements part of the GraphStore interface.
func (s *Store) Write(req *spb.WriteRequest) (err error) {
	wr, err := s.db.Writer()
	if err != nil {
		return fmt.Errorf("db writer error: %v", err)
	}
	defer func() {
		cErr := wr.Close()
		if err == nil && cErr != nil {
			err = fmt.Errorf("db writer close error: %v", cErr)
		}
	}()
	for _, update := range req.Update {
		if update.GetFactName() == "" {
			return fmt.Errorf("invalid WriteRequest: Update missing FactName")
		}
		updateKey, err := EncodeKey(req.Source, update.GetFactName(), update.GetEdgeKind(), update.Target)
		if err != nil {
			return fmt.Errorf("encoding error: %v", err)
		}
		if err := wr.Write(updateKey, update.FactValue); err != nil {
			return fmt.Errorf("db write error: %v", err)
		}
	}
	return nil
}

// Scan implements part of the GraphStore interface.
func (s *Store) Scan(req *spb.ScanRequest, stream chan<- *spb.Entry) error {
	iter, err := s.db.Reader(entryKeyPrefixBytes, &Options{LargeRead: true})
	if err != nil {
		return fmt.Errorf("db seek error: %v", err)
	}
	defer iter.Close()
	for {
		key, val, err := iter.Next()
		if err == io.EOF {
			break
		} else if err != nil {
			return fmt.Errorf("db iteration error: %v", err)
		}
		entry, err := Entry(key, val)
		if err != nil {
			return fmt.Errorf("invalid key/value entry: %v", err)
		}

		if storage.EntryMatchesScan(req, entry) {
			stream <- entry
		}
	}
	return nil
}

// Close implements part of the GraphStore interface.
func (s *Store) Close() error { return s.db.Close() }

// GraphStore Implementation Details:
//   These details are strictly for this particular implementation of a
//   GraphStore and are *not* specified in the GraphStore specification.  These
//   particular encodings, however, do satisfy the GraphStore requirements,
//   including the GraphStore entry ordering property.  Also, due to the
//   "entry:" key prefix, this implementation allows for additional embedded
//   GraphStore metadata/indices using other distinct key prefixes.
//
//   The encoding format for entries in a keyvalue GraphStore is:
//     "entry:<source>_<edgeKind>_<factName>_<target>" == "<factValue>"
//   where:
//     "entry:" == entryKeyPrefix
//     "_"      == entryKeySep
//     <source> and <target> are the Entry's encoded VNames:
//
//   The encoding format for VNames is:
//     <signature>-<corpus>-<root>-<path>-<language>
//   where:
//     "-"      == vNameFieldSep

const (
	entryKeyPrefix = "entry:"

	// entryKeySep is used to separate the source, factName, edgeKind, and target of an
	// encoded Entry key
	entryKeySep    = '\n'
	entryKeySepStr = string(entryKeySep)

	// vNameFieldSep is used to separate the fields of an encoded VName
	vNameFieldSep = "\000"
)

var (
	entryKeyPrefixBytes = []byte(entryKeyPrefix)
	entryKeySepBytes    = []byte{entryKeySep}
)

// EncodeKey returns a canonical encoding of an Entry (minus its value).
func EncodeKey(source *spb.VName, factName string, edgeKind string, target *spb.VName) ([]byte, error) {
	if source == nil {
		return nil, fmt.Errorf("invalid Entry: missing source VName for key encoding")
	} else if (edgeKind == "" || target == nil) && (edgeKind != "" || target != nil) {
		return nil, fmt.Errorf("invalid Entry: edgeKind and target Ticket must be both non-empty or empty")
	} else if strings.Index(edgeKind, entryKeySepStr) != -1 {
		return nil, fmt.Errorf("invalid Entry: edgeKind contains key separator")
	} else if strings.Index(factName, entryKeySepStr) != -1 {
		return nil, fmt.Errorf("invalid Entry: factName contains key separator")
	}

	keySuffix := []byte(entryKeySepStr + edgeKind + entryKeySepStr + factName + entryKeySepStr)

	srcEncoding, err := encodeVName(source)
	if err != nil {
		return nil, fmt.Errorf("error encoding source VName: %v", err)
	} else if bytes.Index(srcEncoding, entryKeySepBytes) != -1 {
		return nil, fmt.Errorf("invalid Entry: source VName contains key separator %v", source)
	}
	targetEncoding, err := encodeVName(target)
	if err != nil {
		return nil, fmt.Errorf("error encoding target VName: %v", err)
	} else if bytes.Index(targetEncoding, entryKeySepBytes) != -1 {
		return nil, fmt.Errorf("invalid Entry: target VName contains key separator")
	}

	return bytes.Join([][]byte{
		entryKeyPrefixBytes,
		srcEncoding,
		keySuffix,
		targetEncoding,
	}, nil), nil
}

// KeyPrefix returns a prefix to every encoded key for the given source VName and exact
// edgeKind. If edgeKind is "*", the prefix will match any edgeKind.
func KeyPrefix(source *spb.VName, edgeKind string) ([]byte, error) {
	if source == nil {
		return nil, fmt.Errorf("missing source VName")
	}
	srcEncoding, err := encodeVName(source)
	if err != nil {
		return nil, fmt.Errorf("error encoding source VName: %v", err)
	}

	prefix := bytes.Join([][]byte{entryKeyPrefixBytes, append(srcEncoding, entryKeySep)}, nil)
	if edgeKind == "*" {
		return prefix, nil
	}

	return bytes.Join([][]byte{prefix, append([]byte(edgeKind), entryKeySep)}, nil), nil
}

// Entry decodes the key (assuming it was encoded by EncodeKey) into an Entry
// and populates its value field.
func Entry(key []byte, val []byte) (*spb.Entry, error) {
	if !bytes.HasPrefix(key, entryKeyPrefixBytes) {
		return nil, fmt.Errorf("key is not prefixed with entry prefix %q", entryKeyPrefix)
	}
	keyStr := string(bytes.TrimPrefix(key, entryKeyPrefixBytes))
	keyParts := strings.SplitN(keyStr, entryKeySepStr, 4)
	if len(keyParts) != 4 {
		return nil, fmt.Errorf("invalid key[%d]: %q", len(keyParts), string(key))
	}

	srcVName, err := decodeVName(keyParts[0])
	if err != nil {
		return nil, fmt.Errorf("error decoding source VName: %v", err)
	}
	targetVName, err := decodeVName(keyParts[3])
	if err != nil {
		return nil, fmt.Errorf("error decoding target VName: %v", err)
	}

	return &spb.Entry{
		Source:    srcVName,
		FactName:  &keyParts[2],
		EdgeKind:  &keyParts[1],
		Target:    targetVName,
		FactValue: val,
	}, nil
}

// encodeVName returns a canonical byte array for the given VName. Returns nil if given nil.
func encodeVName(v *spb.VName) ([]byte, error) {
	if v == nil {
		return nil, nil
	} else if strings.Index(v.GetSignature(), vNameFieldSep) != -1 ||
		strings.Index(v.GetCorpus(), vNameFieldSep) != -1 ||
		strings.Index(v.GetRoot(), vNameFieldSep) != -1 ||
		strings.Index(v.GetPath(), vNameFieldSep) != -1 ||
		strings.Index(v.GetLanguage(), vNameFieldSep) != -1 {
		return nil, fmt.Errorf("VName contains invalid rune: %q", vNameFieldSep)
	}
	return []byte(strings.Join([]string{
		v.GetSignature(),
		v.GetCorpus(),
		v.GetRoot(),
		v.GetPath(),
		v.GetLanguage(),
	}, vNameFieldSep)), nil
}

// decodeVName returns the VName coded in the given string. Returns nil, if len(data) == 0.
func decodeVName(data string) (*spb.VName, error) {
	if len(data) == 0 {
		return nil, nil
	}
	parts := strings.SplitN(data, vNameFieldSep, 5)
	if len(parts) != 5 {
		return nil, fmt.Errorf("invalid VName encoding: %q", data)
	}
	return &spb.VName{
		Signature: &parts[0],
		Corpus:    &parts[1],
		Root:      &parts[2],
		Path:      &parts[3],
		Language:  &parts[4],
	}, nil
}
