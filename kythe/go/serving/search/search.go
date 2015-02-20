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
	"kythe/go/util/stringset"

	spb "kythe/proto/storage_proto"
)

// Table implements a search.Service using an inverted index table.
type Table struct{ table.Inverted }

const indexTablePrefix = "indexNodes:"

// Search implements the search.Service interface.
func (t *Table) Search(q *spb.SearchRequest) (*spb.SearchReply, error) {
	tickets := stringset.New()
	var err error

	if q.Partial.Signature != "" {
		tickets, err = t.intersect(tickets, VNameVal("signature", q.Partial.Signature))
		if err != nil {
			return nil, err
		} else if len(tickets) == 0 {
			return constructReply(tickets), nil
		}
	}
	if q.Partial.Path != "" {
		tickets, err = t.intersect(tickets, VNameVal("path", q.Partial.Path))
		if err != nil {
			return nil, err
		} else if len(tickets) == 0 {
			return constructReply(tickets), nil
		}
	}
	if q.Partial.Language != "" {
		tickets, err = t.intersect(tickets, VNameVal("language", q.Partial.Language))
		if err != nil {
			return nil, err
		} else if len(tickets) == 0 {
			return constructReply(tickets), nil
		}
	}
	if q.Partial.Root != "" {
		tickets, err = t.intersect(tickets, VNameVal("root", q.Partial.Root))
		if err != nil {
			return nil, err
		} else if len(tickets) == 0 {
			return constructReply(tickets), nil
		}
	}
	if q.Partial.Corpus != "" {
		tickets, err = t.intersect(tickets, VNameVal("corpus", q.Partial.Corpus))
		if err != nil {
			return nil, err
		} else if len(tickets) == 0 {
			return constructReply(tickets), nil
		}
	}

	for _, f := range q.Fact {
		tickets, err = t.intersect(tickets, FactVal(f.Name, f.Value))
		if err != nil {
			return nil, err
		} else if len(tickets) == 0 {
			return constructReply(tickets), nil
		}
	}

	return constructReply(tickets), nil
}

func constructReply(results stringset.Set) *spb.SearchReply {
	return &spb.SearchReply{Ticket: results.Slice()}
}

func (t *Table) intersect(results stringset.Set, val []byte) (stringset.Set, error) {
	keys, err := t.Lookup(val)
	if err != nil {
		return nil, err
	} else if len(keys) == 0 {
		return nil, nil
	}

	res := stringset.New()
	for _, k := range keys {
		s := string(k)
		if results.Contains(s) || len(results) == 0 {
			res.Add(s)
		}
	}

	return res, nil
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
