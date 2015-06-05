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

	"kythe.io/kythe/go/storage/table"
	"kythe.io/kythe/go/util/kytheuri"

	"golang.org/x/net/context"

	srvpb "kythe.io/kythe/proto/serving_proto"
	spb "kythe.io/kythe/proto/storage_proto"
)

// Table implements a Service using an inverted index table.
type Table struct{ table.Inverted }

const indexTablePrefix = "indexNodes:"

var (
	emptyPartial     = &spb.VName{}
	emptyPartialMask = &spb.VNameMask{}
)

type valQuery struct {
	value  []byte
	prefix bool
}

// Search implements the Service interface.
func (t *Table) Search(ctx context.Context, q *spb.SearchRequest) (*spb.SearchReply, error) {
	if q.Partial == nil {
		q.Partial = emptyPartial
	}
	if q.PartialPrefix == nil {
		q.PartialPrefix = emptyPartialMask
	}

	// Ordering of vals matter for performance reasons.  Values should be ordered,
	// as best as possible, from least associated keys to most so that there are
	// fewer Contains checks and a small initial Lookup set.
	var vals []valQuery

	if q.Partial.Signature != "" {
		vals = append(vals, valQuery{vNameVal("signature", q.Partial.Signature), q.PartialPrefix.Signature})
	}
	if q.Partial.Path != "" {
		vals = append(vals, valQuery{vNameVal("path", q.Partial.Path), q.PartialPrefix.Path})
	}
	for _, f := range q.Fact {
		vals = append(vals, valQuery{factVal(f.Name, f.Value), f.Prefix})
	}
	if q.Partial.Language != "" {
		vals = append(vals, valQuery{vNameVal("language", q.Partial.Language), q.PartialPrefix.Language})
	}
	if q.Partial.Root != "" {
		vals = append(vals, valQuery{vNameVal("root", q.Partial.Root), q.PartialPrefix.Root})
	}
	if q.Partial.Corpus != "" {
		vals = append(vals, valQuery{vNameVal("corpus", q.Partial.Corpus), q.PartialPrefix.Corpus})
	}

	if len(vals) == 0 {
		return &spb.SearchReply{}, nil
	}

	keys, err := t.Lookup(vals[0].value, vals[0].prefix)
	if err != nil {
		return nil, err
	}

	for _, v := range vals[1:] {
		if len(keys) == 0 {
			return &spb.SearchReply{}, nil
		}

		for i := len(keys) - 1; i >= 0; i-- {
			if exists, err := t.Contains(keys[i], v.value, v.prefix); err != nil {
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

// MaxIndexedFactValueSize is the maximum length in bytes of fact values to
// write to the inverted index search table.
var MaxIndexedFactValueSize = 512

// IndexNode writes each of n's VName components and facts to t as search index
// entries.  MaxIndexedFactValueSize limits fact values written to the index.
func IndexNode(t table.Inverted, n *srvpb.Node) error {
	uri, err := kytheuri.Parse(n.Ticket)
	if err != nil {
		return err
	}
	key := []byte(n.Ticket)

	if uri.Signature != "" {
		if err := t.Put(key, vNameVal("signature", uri.Signature)); err != nil {
			return err
		}
	}
	if uri.Corpus != "" {
		if err := t.Put(key, vNameVal("corpus", uri.Corpus)); err != nil {
			return err
		}
	}
	if uri.Root != "" {
		if err := t.Put(key, vNameVal("root", uri.Root)); err != nil {
			return err
		}
	}
	if uri.Path != "" {
		if err := t.Put(key, vNameVal("path", uri.Path)); err != nil {
			return err
		}
	}
	if uri.Language != "" {
		if err := t.Put(key, vNameVal("language", uri.Language)); err != nil {
			return err
		}
	}

	for _, f := range n.Fact {
		if len(f.Value) <= MaxIndexedFactValueSize {
			if err := t.Put(key, factVal(f.Name, f.Value)); err != nil {
				return err
			}
		}
	}

	return nil
}

const (
	labelValSep = "\n"

	vNameComponentPrefix = "vname."
	factValuePrefix      = "fact."
)

// vNameVal returns an inverted index value for a vname component.
func vNameVal(field, val string) []byte {
	var buf bytes.Buffer
	buf.Grow(len(indexTablePrefix) + len(vNameComponentPrefix) + len(field) + len(labelValSep) + len(val))
	buf.WriteString(indexTablePrefix)
	buf.WriteString(vNameComponentPrefix)
	buf.WriteString(field)
	buf.WriteString(labelValSep)
	buf.WriteString(val)
	return buf.Bytes()
}

// factVal returns an inverted index value for a node fact.
func factVal(name string, val []byte) []byte {
	var buf bytes.Buffer
	buf.Grow(len(indexTablePrefix) + len(factValuePrefix) + len(name) + len(labelValSep) + len(val))
	buf.WriteString(indexTablePrefix)
	buf.WriteString(factValuePrefix)
	buf.WriteString(name)
	buf.WriteString(labelValSep)
	buf.Write(val)
	return buf.Bytes()
}
