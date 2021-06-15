/*
 * Copyright 2018 The Kythe Authors. All rights reserved.
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

package beamio

import (
	"io/ioutil"
	"math/rand"
	"os"
	"sort"
	"testing"
	"unicode"

	"github.com/apache/beam/sdks/go/pkg/beam"
	"github.com/apache/beam/sdks/go/pkg/beam/testing/ptest"
	"github.com/apache/beam/sdks/go/pkg/beam/transforms/stats"
	"github.com/jmhodges/levigo"

	_ "github.com/apache/beam/sdks/go/pkg/beam/io/filesystem/local"
)

func TestLevelDBSink(t *testing.T) {
	var strs []string
	for i := 0; i < 28; i++ {
		buf := ""
		for c := 'A'; c <= 'z'; c++ {
			if !unicode.IsLetter(c) {
				continue
			}
			buf += string(c)
			strs = append(strs, buf)
		}
		for len(buf) > 1 {
			buf = buf[:len(buf)-1]
			strs = append(strs, buf)
		}
	}

	p, s, coll := ptest.CreateList(strs)
	tbl := beam.ParDo(s, extendToKey, coll)

	path, err := ioutil.TempDir("", "leveldb")
	if err != nil {
		t.Fatalf("Error creating temporary directory: %v", err)
	}
	defer os.RemoveAll(path)
	WriteLevelDB(s, path, stats.Opts{NumQuantiles: 4, K: 100}, tbl)

	if err := ptest.Run(p); err != nil {
		t.Fatal(err)
	}

	opts := levigo.NewOptions()
	defer opts.Close()
	db, err := levigo.Open(path, opts)
	if err != nil {
		t.Fatal(err)
	}
	defer db.Close()

	ropts := levigo.NewReadOptions()
	defer ropts.Close()
	iter := db.NewIterator(ropts)
	defer iter.Close()

	// "Randomly" seek for the known key-value entries
	for seed := int64(0); seed < 10; seed++ {
		rand.Seed(seed)
		for i := 0; i < len(strs)*10; i++ {
			s := strs[rand.Int()%len(strs)]
			iter.Seek([]byte(s))
			if !iter.Valid() {
				t.Fatalf("Invalid Iterator: %v", iter.GetError())
			} else if found := string(iter.Key()); found != s {
				t.Errorf("Expected key %q; found %q", s, found)
			} else if found := string(iter.Value()); found != s {
				t.Errorf("Expected value %q; found %q", s, found)
			}
		}
	}

	// Seek for the known key-value entries in order.
	sort.Strings(strs)
	for _, s := range strs {
		iter.Seek([]byte(s))
		if !iter.Valid() {
			t.Fatal("Invalid Iterator")
		} else if found := string(iter.Key()); found != s {
			t.Errorf("Expected key %q; found %q", s, found)
		} else if found := string(iter.Value()); found != s {
			t.Errorf("Expected value %q; found %q", s, found)
		}
	}
}

func extendToKey(v beam.T) (beam.T, beam.T) { return v, v }

func TestSchemaPreservingPathJoin(t *testing.T) {
	exp := "gs://foo/bar/baz"
	res := schemePreservingPathJoin("gs://foo", "bar/baz")
	if exp != res {
		t.Fatalf("Expected [%s], got [%s]", exp, res)
	}

	exp = "foo/bar/baz"
	res = schemePreservingPathJoin("foo//bar", "baz")
	if exp != res {
		t.Fatalf("Expected [%s], got [%s]", exp, res)
	}

	exp = "/foo/bar"
	res = schemePreservingPathJoin("/foo/", "bar")
	if exp != res {
		t.Fatalf("Expected [%s], got [%s]", exp, res)
	}
}
