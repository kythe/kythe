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

// Binary entrystream provides tools to manipulate a stream of delimited Entry
// messages. By default, entrystream does nothing to the entry stream.
//
// Examples:
//   $ ... | entrystream                      # Passes through proto entry stream unchanged
//   $ ... | entrystream --sort               # Sorts the entry stream into GraphStore order
//   $ ... | entrystream --write_json         # Prints entry stream as JSON
//   $ ... | entrystream --write_json --sort  # Sorts the JSON entry stream into GraphStore order
//   $ ... | entrystream --entrysets          # Prints combined entry sets as JSON
//   $ ... | entrystream --count              # Prints the number of entries in the incoming stream
//   $ ... | entrystream --read_json          # Reads entry stream as JSON and prints a proto stream
package main

import (
	"encoding/json"
	"flag"
	"fmt"
	"log"
	"os"

	"kythe.io/kythe/go/platform/delimited"
	"kythe.io/kythe/go/services/graphstore/compare"
	"kythe.io/kythe/go/storage/stream"
	"kythe.io/kythe/go/util/disksort"
	"kythe.io/kythe/go/util/flagutil"

	spb "kythe.io/kythe/proto/storage_proto"

	"github.com/golang/protobuf/proto"
)

type entrySet struct {
	Source   *spb.VName `json:"source"`
	Target   *spb.VName `json:"target,omitempty"`
	EdgeKind string     `json:"edge_kind,omitempty"`

	Properties map[string]string `json:"properties"`
}

var (
	readJSON    = flag.Bool("read_json", false, "Assume stdin is a stream of JSON entries instead of protobufs")
	writeJSON   = flag.Bool("write_json", false, "Print JSON stream as output")
	sortStream  = flag.Bool("sort", false, "Sort entry stream into GraphStore order")
	uniqEntries = flag.Bool("unique", false, "Print only unique entries (implies --sort)")
	entrySets   = flag.Bool("entrysets", false, "Print Entry protos as JSON EntrySets (implies --sort and --write_json)")
	countOnly   = flag.Bool("count", false, "Only print the count of protos streamed")
)

func init() {
	flag.Usage = flagutil.SimpleUsage("Manipulate a stream of delimited Entry messages",
		"[--read_json] [--unique] ([--write_json] [--sort] | [--entrysets] | [--count])")
}

func main() {
	flag.Parse()
	if len(flag.Args()) > 0 {
		flagutil.UsageErrorf("unknown arguments: %v", flag.Args())
	}

	in := os.Stdin
	var entries <-chan *spb.Entry
	if *readJSON {
		entries = stream.ReadJSONEntries(in)
	} else {
		entries = stream.ReadEntries(in)
	}
	if *sortStream || *entrySets || *uniqEntries {
		entries = sortEntries(entries)
	}

	if *uniqEntries {
		entries = dedupEntries(entries)
	}

	encoder := json.NewEncoder(os.Stdout)
	wr := delimited.NewWriter(os.Stdout)

	var set entrySet
	entryCount := 0
	for entry := range entries {
		if *countOnly {
			entryCount++
		} else if *entrySets {
			if !compare.VNamesEqual(set.Source, entry.Source) || !compare.VNamesEqual(set.Target, entry.Target) || set.EdgeKind != entry.EdgeKind {
				if len(set.Properties) != 0 {
					failOnErr(encoder.Encode(set))
				}
				set.Source = entry.Source
				set.EdgeKind = entry.EdgeKind
				set.Target = entry.Target
				set.Properties = make(map[string]string)
			}
			set.Properties[entry.FactName] = string(entry.FactValue)
		} else if *writeJSON {
			failOnErr(encoder.Encode(entry))
		} else {
			rec, err := proto.Marshal(entry)
			failOnErr(err)
			failOnErr(wr.Put(rec))
		}
	}
	if len(set.Properties) != 0 {
		failOnErr(encoder.Encode(set))
	}
	if *countOnly {
		fmt.Println(entryCount)
	}
}

func sortEntries(entries <-chan *spb.Entry) <-chan *spb.Entry {
	sorter, err := disksort.NewMergeSorter(disksort.MergeOptions{
		Lesser:    entryLesser{},
		Marshaler: entryMarshaler{},
	})
	if err != nil {
		log.Fatalf("Error creating entries sorter: %v", err)
	}

	for e := range entries {
		if err := sorter.Add(e); err != nil {
			log.Fatalf("Error sorting entries: %v", err)
		}
	}

	ch := make(chan *spb.Entry)
	go func() {
		defer close(ch)
		if err := sorter.Read(func(i interface{}) error {
			ch <- i.(*spb.Entry)
			return nil
		}); err != nil {
			log.Fatalf("error sorting entries: %v", err)
		}
	}()
	return ch
}

type entryLesser struct{}

func (entryLesser) Less(a, b interface{}) bool {
	return compare.Entries(a.(*spb.Entry), b.(*spb.Entry)) == compare.LT
}

type entryMarshaler struct{}

func (entryMarshaler) Marshal(x interface{}) ([]byte, error) { return proto.Marshal(x.(proto.Message)) }

func (entryMarshaler) Unmarshal(rec []byte) (interface{}, error) {
	var e spb.Entry
	return &e, proto.Unmarshal(rec, &e)
}

func dedupEntries(entries <-chan *spb.Entry) <-chan *spb.Entry {
	ch := make(chan *spb.Entry)
	go func() {
		defer close(ch)
		var last *spb.Entry
		var duplicates int
		for e := range entries {
			if compare.Entries(last, e) == compare.EQ {
				duplicates++
			} else {
				last = e
				ch <- e
			}
		}
		log.Printf("entrystream: removed %d duplicate entries", duplicates)
	}()
	return ch
}

func failOnErr(err error) {
	if err != nil {
		log.Fatal(err)
	}
}
