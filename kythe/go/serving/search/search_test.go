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

package search

import (
	"io/ioutil"
	"log"
	"os"
	"reflect"
	"runtime"
	"sort"
	"strings"
	"testing"

	"kythe.io/kythe/go/storage/leveldb"
	"kythe.io/kythe/go/storage/table"
	"kythe.io/kythe/go/util/kytheuri"

	"golang.org/x/net/context"

	srvpb "kythe.io/kythe/proto/serving_proto"
	spb "kythe.io/kythe/proto/storage_proto"
)

const suffixWildcard = "%*"

var ctx = context.Background()

var (
	testNodes = []*srvpb.Node{
		// each has a unique partial vname (see TestSearchExact)
		node(sig("1"), "/kythe/node/kind", "something"),
		node(&spb.VName{
			Signature: "signature",
			Corpus:    "corpus",
			Root:      "root",
			Path:      "path",
			Language:  "language",
		},
			"/kythe/node/kind", "something",
			"/kythe/subkind", "else",
			"/empty/fact", ""),
		node(&spb.VName{Path: "/some/path"}, "/kythe/node/kind", "file"),

		// other nodes
		node(sig("signature"), "/kythe/subkind", "else"),
		node(&spb.VName{Path: "path"}),
		node(sig("someSig"), "/kythe/node/kind", "some"),
		node(sig("3"), "/kythe/node/kind", "somekind"),
	}

	testVNames []*spb.VName
)

func init() {
	testVNames = make([]*spb.VName, len(testNodes))
	for i, n := range testNodes {
		uri, _ := kytheuri.Parse(n.Ticket)
		testVNames[i] = uri.VName()
	}
}

func TestIndexNode(t *testing.T) {
	// isolated test of withTempTable setup/indexing
	withTempTable(t, func(tbl *Table) {})
}

func TestSearchExact(t *testing.T) {
	withTempTable(t, func(tbl *Table) {
		for i := range testNodes[0:3] {
			testSearch(t, tbl, tickets(i), testVNames[i])
		}
	})
}

func TestSearchPartial(t *testing.T) {
	withTempTable(t, func(tbl *Table) {
		testSearch(t, tbl, tickets(0), sig("1"))
		testSearch(t, tbl, tickets(2), &spb.VName{Path: "/some/path"})
		testSearch(t, tbl, tickets(1, 4), &spb.VName{Path: "path"})
		testSearch(t, tbl, tickets(1, 3), sig("signature"))
	})
}

func TestSearchEmpty(t *testing.T) {
	withTempTable(t, func(tbl *Table) {
		testSearch(t, tbl, nil, nil)
		testSearch(t, tbl, nil, sig("no such sig"))
		testSearch(t, tbl, nil, testVNames[0], "/no/such/fact", "")
		testSearch(t, tbl, nil, nil, "/no/such/fact", "")
		testSearch(t, tbl, nil, nil, "/empty/fact", "no such value")
		testSearch(t, tbl, nil, nil, "/kythe/node/kind", "something", "but", "not")
	})
}

func TestSearchFact(t *testing.T) {
	withTempTable(t, func(tbl *Table) {
		testSearch(t, tbl, tickets(1), nil, "/empty/fact", "")
		testSearch(t, tbl, tickets(5), nil, "/kythe/node/kind", "some")
		testSearch(t, tbl, tickets(0, 1), nil, "/kythe/node/kind", "something")
		testSearch(t, tbl, tickets(1, 3), nil, "/kythe/subkind", "else")
	})
}

func TestSearchPrefix(t *testing.T) {
	withTempTable(t, func(tbl *Table) {
		testSearch(t, tbl, tickets(0, 1, 5, 6), nil, "/kythe/node/kind", "some"+suffixWildcard)
		testSearch(t, tbl, tickets(0, 1), nil, "/kythe/node/kind", "somet"+suffixWildcard)

		testSearch(t, tbl, tickets(0), sig("1"), "/kythe/node/kind", "some"+suffixWildcard)
		testSearch(t, tbl, tickets(1, 5), sig("s"+suffixWildcard), "/kythe/node/kind", "some"+suffixWildcard)
	})
}

func testSearch(t *testing.T, tbl *Table, expected []string, partial *spb.VName, facts ...string) []string {
	_, _, line, ok := runtime.Caller(1)
	if !ok {
		t.Fatal("Failed to determine caller")
	}
	if partial == nil {
		partial = emptyPartial
	}

	req := &spb.SearchRequest{
		Partial: &spb.VName{
			Corpus:    strings.TrimSuffix(partial.Corpus, suffixWildcard),
			Root:      strings.TrimSuffix(partial.Root, suffixWildcard),
			Path:      strings.TrimSuffix(partial.Path, suffixWildcard),
			Signature: strings.TrimSuffix(partial.Signature, suffixWildcard),
			Language:  strings.TrimSuffix(partial.Language, suffixWildcard),
		}}
	req.PartialPrefix = &spb.VNameMask{
		Corpus:    partial.Corpus != req.Partial.Corpus,
		Root:      partial.Root != req.Partial.Root,
		Path:      partial.Path != req.Partial.Path,
		Signature: partial.Signature != req.Partial.Signature,
		Language:  partial.Language != req.Partial.Language,
	}

	for i := 0; i < len(facts); i += 2 {
		v := strings.TrimSuffix(facts[i+1], suffixWildcard)
		req.Fact = append(req.Fact, &spb.SearchRequest_Fact{
			Name:   facts[i],
			Value:  []byte(v),
			Prefix: v != facts[i+1],
		})
	}
	reply, err := tbl.Search(ctx, req)
	if err != nil {
		t.Errorf("line %d: search error for {%+v}: %v", line, req, err)
	}
	sort.Strings(expected)
	sort.Strings(reply.Ticket)
	if !reflect.DeepEqual(expected, reply.Ticket) && (len(expected) != 0 || len(reply.Ticket) != 0) {
		t.Errorf("line %d: search results expected: %v; found: %v", line, expected, reply.Ticket)
	}
	return expected
}

func tickets(i ...int) (ts []string) {
	for _, idx := range i {
		ts = append(ts, testNodes[idx].Ticket)
	}
	return
}

func withTempTable(t *testing.T, f func(*Table)) {
	dir, err := ioutil.TempDir("", "table.test")
	if err != nil {
		t.Errorf("error creating temporary directory: %v", err)
	}
	defer func() {
		if err := os.RemoveAll(dir); err != nil {
			t.Errorf("error removing temporary directory: %v", err)
		}
	}()

	db, err := leveldb.Open(dir, nil)
	if err != nil {
		if err := os.RemoveAll(dir); err != nil {
			log.Printf("error removing temporary directory: %v", err)
		}
		t.Errorf("error opening temporary table: %v", err)
	}
	defer func() {
		if err := db.Close(); err != nil {
			t.Errorf("error closing temporary table: %v", err)
		}
	}()

	tbl := &Table{&table.KVInverted{db}}
	for _, n := range testNodes {
		if err := IndexNode(tbl, n); err != nil {
			t.Error(err)
		}
	}

	f(tbl)
}

func sig(signature string) *spb.VName {
	return &spb.VName{Signature: signature}
}

func node(v *spb.VName, facts ...string) *srvpb.Node {
	if len(facts)%2 != 0 {
		panic("odd number of facts")
	}
	n := &srvpb.Node{Ticket: kytheuri.ToString(v)}
	for i := 0; i < len(facts); i += 2 {
		n.Fact = append(n.Fact, &srvpb.Node_Fact{
			Name:  facts[i],
			Value: []byte(facts[i+1]),
		})
	}
	return n
}
