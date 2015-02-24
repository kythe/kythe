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

// Package search provides an inverted index implemenation of the search.Service
// interface.
//
// Table format:
//   indexNodes:vname.signature=<src.signature>   -> <ticket>
//   indexNodes:vname.corpus=<src.corpus>         -> <ticket>
//   indexNodes:vname.root=<src.root>             -> <ticket>
//   indexNodes:vname.path=<src.path>             -> <ticket>
//   indexNodes:vname.language=<src.language>     -> <ticket>
//   indexNodes:fact.<factName>=<factValue>       -> <ticket>
package search

import (
	"bytes"

	"kythe/go/storage/table"

	spb "kythe/proto/storage_proto"
)

// Table implements a search.Service using an inverted index table.
type Table struct{ table.Inverted }

const indexTablePrefix = "indexNodes:"

// Search implements the search.Service interface.
func (t *Table) Search(q *spb.SearchRequest) (*spb.SearchReply, error) {

	// Ordering of vals matter for performance reasons.  Values should be ordered,
	// as best as possible, from least associated keys to most so that there are
	// fewer Contains checks and a small initial Lookup set.
	var vals [][]byte

	if q.Partial.Signature != "" {
		vals = append(vals, VNameVal("signature", q.Partial.Signature))
	}
	if q.Partial.Path != "" {
		vals = append(vals, VNameVal("path", q.Partial.Path))
	}
	for _, f := range q.Fact {
		vals = append(vals, FactVal(f.Name, f.Value))
	}
	if q.Partial.Language != "" {
		vals = append(vals, VNameVal("language", q.Partial.Language))
	}
	if q.Partial.Root != "" {
		vals = append(vals, VNameVal("root", q.Partial.Root))
	}
	if q.Partial.Corpus != "" {
		vals = append(vals, VNameVal("corpus", q.Partial.Corpus))
	}

	if len(vals) == 0 {
		return &spb.SearchReply{}, nil
	}

	keys, err := t.Lookup(vals[0])
	if err != nil {
		return nil, err
	}

	for _, val := range vals[1:] {
		if len(keys) == 0 {
			return &spb.SearchReply{}, nil
		}

		for i := len(keys) - 1; i >= 0; i-- {
			if exists, err := t.Contains(keys[i], val); err != nil {
				return nil, err
			} else if !exists {
				keys = append(keys[:i], keys[i+1:]...)
			}
		}
	}

	tickets := make([]string, len(keys), len(keys))
	for i, k := range keys {
		tickets[i] = string(k)
	}
	return &spb.SearchReply{Ticket: tickets}, nil
}

const (
	labelValSep = "\n"

	vNameComponentPrefix = "vname."
	factValuePrefix      = "fact."
)

// VNameVal returns an inverted index value for a vname component.
func VNameVal(field, val string) []byte {
	var buf bytes.Buffer
	buf.Grow(len(indexTablePrefix) + len(vNameComponentPrefix) + len(field) + len(labelValSep) + len(val))
	buf.WriteString(indexTablePrefix)
	buf.WriteString(vNameComponentPrefix)
	buf.WriteString(field)
	buf.WriteString(labelValSep)
	buf.WriteString(val)
	return buf.Bytes()
}

// FactVal returns an inverted index value for a node fact.
func FactVal(name string, val []byte) []byte {
	var buf bytes.Buffer
	buf.Grow(len(name) + 1 + len(val))
	buf.WriteString(name)
	buf.WriteString(labelValSep)
	buf.Write(val)
	return buf.Bytes()
}
