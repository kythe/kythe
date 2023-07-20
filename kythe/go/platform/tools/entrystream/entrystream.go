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

// Binary entrystream provides tools to manipulate a stream of delimited Entry
// messages. By default, entrystream does nothing to the entry stream.
//
// Examples:
//
//	$ ... | entrystream                      # Passes through proto entry stream unchanged
//	$ ... | entrystream --sort               # Sorts the entry stream into GraphStore order
//	$ ... | entrystream --write_format=json  # Prints entry stream as JSON
//	$ ... | entrystream --entrysets          # Prints combined entry sets as JSON
//	$ ... | entrystream --count              # Prints the number of entries in the incoming stream
//	$ ... | entrystream --read_format=json   # Reads entry stream as JSON and prints a proto stream
//
//	$ ... | entrystream --write_format=riegeli # Writes entry stream as a Riegeli file
//	$ ... | entrystream --read_format=riegeli  # Reads the entry stream from a Riegeli file
package main

import (
	"bufio"
	"encoding/json"
	"flag"
	"fmt"
	"io"
	"os"
	"strings"

	"kythe.io/kythe/go/platform/delimited"
	"kythe.io/kythe/go/storage/entryset"
	"kythe.io/kythe/go/storage/stream"
	"kythe.io/kythe/go/util/compare"
	"kythe.io/kythe/go/util/disksort"
	"kythe.io/kythe/go/util/flagutil"
	"kythe.io/kythe/go/util/log"
	"kythe.io/kythe/go/util/riegeli"

	"google.golang.org/protobuf/proto"

	spb "kythe.io/kythe/proto/storage_go_proto"
)

type entrySet struct { // TODO(schroederc): rename to avoid confusion with EntrySet proto
	Source   *spb.VName `json:"source"`
	Target   *spb.VName `json:"target,omitempty"`
	EdgeKind string     `json:"edge_kind,omitempty"`

	Properties map[string]json.RawMessage `json:"properties"`
}

// Accepted --{read,write}_format values
const (
	delimitedFormat = "delimited"
	jsonFormat      = "json"
	riegeliFormat   = "riegeli"
)

var (
	readJSON  = flag.Bool("read_json", false, "Assume stdin is a stream of JSON entries instead of protobufs (deprecated: use --read_format)")
	writeJSON = flag.Bool("write_json", false, "Print JSON stream as output (deprecated: use --write_format)")

	readFormat  = flag.String("read_format", delimitedFormat, "Format of the input stream (accepted formats: {delimited,json,riegeli})")
	writeFormat = flag.String("write_format", delimitedFormat, "Format of the output stream (accepted formats: {delimited,json,riegeli})")

	riegeliOptions = flag.String("riegeli_writer_options", "", "Riegeli writer options")

	sortStream  = flag.Bool("sort", false, "Sort entry stream into GraphStore order")
	uniqEntries = flag.Bool("unique", false, "Print only unique entries (implies --sort)")

	aggregateEntrySet = flag.Bool("aggregate_entryset", false, "Output a single aggregate EntrySet proto")
	entrySets         = flag.Bool("entrysets", false, "Print Entry protos as JSON EntrySets (implies --sort and --write_format=json)")
	countOnly         = flag.Bool("count", false, "Only print the count of protos streamed")

	structuredFacts = flag.Bool("structured_facts", false, "Encode and/or decode the fact_value for marked source facts")
)

func init() {
	flag.Usage = flagutil.SimpleUsage("Manipulate a stream of Entry messages",
		"[--read_format=<format>] [--unique] ([--write_format=<format>] [--sort] | [--entrysets] | [--count] | [--aggregate_entryset])")
}

func main() {
	flag.Parse()
	if len(flag.Args()) > 0 {
		flagutil.UsageErrorf("unknown arguments: %v", flag.Args())
	}

	// Normalize --{read,write}_format values
	*readFormat = strings.ToLower(*readFormat)
	*writeFormat = strings.ToLower(*writeFormat)

	if *readJSON {
		log.Warningf("--read_json is deprecated; use --read_format=json")
		*readFormat = jsonFormat
	}
	if *writeJSON {
		log.Warningf("--write_json is deprecated; use --write_format=json")
		*writeFormat = jsonFormat
	}

	in := bufio.NewReaderSize(os.Stdin, 2*4096)
	out := bufio.NewWriter(os.Stdout)

	var rd stream.EntryReader
	switch *readFormat {
	case jsonFormat:
		if *structuredFacts {
			rd = stream.NewStructuredJSONReader(in)
		} else {
			rd = stream.NewJSONReader(in)
		}
	case riegeliFormat:
		rd = func(emit func(*spb.Entry) error) error {
			r := riegeli.NewReader(in)
			for {
				rec, err := r.Next()
				if err == io.EOF {
					return nil
				} else if err != nil {
					return err
				}
				var e spb.Entry
				if err := proto.Unmarshal(rec, &e); err != nil {
					return err
				} else if err := emit(&e); err != nil {
					return err
				}
			}
		}
	case delimitedFormat:
		rd = stream.NewReader(in)
	default:
		log.Fatalf("Unsupported --read_format=%s", *readFormat)
	}

	if *sortStream || *entrySets || *uniqEntries {
		var err error
		rd, err = sortEntries(rd)
		failOnErr(err)
	}

	if *uniqEntries {
		rd = dedupEntries(rd)
	}

	switch {
	case *countOnly:
		var count int
		failOnErr(rd(func(_ *spb.Entry) error {
			count++
			return nil
		}))
		fmt.Println(count)
	case *aggregateEntrySet:
		es := entryset.New(nil)
		failOnErr(rd(es.Add))
		pb := es.Encode()
		switch *writeFormat {
		case jsonFormat:
			encoder := json.NewEncoder(out)
			failOnErr(encoder.Encode(pb))
		case riegeliFormat:
			opts, err := riegeli.ParseOptions(*riegeliOptions)
			failOnErr(err)
			wr := riegeli.NewWriter(out, opts)
			failOnErr(wr.PutProto(pb))
			failOnErr(wr.Flush())
		case delimitedFormat:
			wr := delimited.NewWriter(out)
			failOnErr(wr.PutProto(pb))
		default:
			log.Fatalf("Unsupported --write_format=%s", *writeFormat)
		}
	case *entrySets:
		encoder := json.NewEncoder(out)
		var set entrySet
		failOnErr(rd(func(entry *spb.Entry) error {
			if !compare.VNamesEqual(set.Source, entry.Source) || !compare.VNamesEqual(set.Target, entry.Target) || set.EdgeKind != entry.EdgeKind {
				if len(set.Properties) != 0 {
					if err := encoder.Encode(set); err != nil {
						return err
					}
				}
				set.Source = entry.Source
				set.EdgeKind = entry.EdgeKind
				set.Target = entry.Target
				set.Properties = make(map[string]json.RawMessage)
			}
			var err error
			if *structuredFacts {
				set.Properties[entry.FactName], err = stream.StructuredFactValueJSON(entry)
			} else {
				set.Properties[entry.FactName], err = json.Marshal(entry)
			}
			return err
		}))
		if len(set.Properties) != 0 {
			failOnErr(encoder.Encode(set))
		}
	default:
		switch *writeFormat {
		case jsonFormat:
			encoder := json.NewEncoder(out)
			failOnErr(rd(func(entry *spb.Entry) error {
				if *structuredFacts {
					return encoder.Encode(stream.Structured(entry))
				}
				return encoder.Encode(entry)
			}))
		case riegeliFormat:
			opts, err := riegeli.ParseOptions(*riegeliOptions)
			failOnErr(err)
			wr := riegeli.NewWriter(out, opts)
			failOnErr(rd(func(entry *spb.Entry) error {
				return wr.PutProto(entry)
			}))
			failOnErr(wr.Flush())
		case delimitedFormat:
			wr := delimited.NewWriter(out)
			failOnErr(rd(func(entry *spb.Entry) error {
				return wr.PutProto(entry)
			}))
		default:
			log.Fatalf("Unsupported --write_format=%s", *writeFormat)
		}
	}
	failOnErr(out.Flush())
}

func sortEntries(rd stream.EntryReader) (stream.EntryReader, error) {
	sorter, err := disksort.NewMergeSorter(disksort.MergeOptions{
		Lesser:    entryLesser{},
		Marshaler: entryMarshaler{},
	})
	if err != nil {
		return nil, fmt.Errorf("error creating entries sorter: %v", err)
	}

	if err := rd(func(e *spb.Entry) error {
		return sorter.Add(e)
	}); err != nil {
		return nil, fmt.Errorf("error sorting entries: %v", err)
	}

	return func(f func(*spb.Entry) error) error {
		return sorter.Read(func(i any) error {
			return f(i.(*spb.Entry))
		})
	}, nil
}

type entryLesser struct{}

func (entryLesser) Less(a, b any) bool {
	return compare.Entries(a.(*spb.Entry), b.(*spb.Entry)) == compare.LT
}

type entryMarshaler struct{}

func (entryMarshaler) Marshal(x any) ([]byte, error) { return proto.Marshal(x.(proto.Message)) }

func (entryMarshaler) Unmarshal(rec []byte) (any, error) {
	var e spb.Entry
	return &e, proto.Unmarshal(rec, &e)
}

func dedupEntries(rd stream.EntryReader) stream.EntryReader {
	return func(f func(*spb.Entry) error) error {
		var last *spb.Entry
		return rd(func(e *spb.Entry) error {
			if compare.Entries(last, e) != compare.EQ {
				last = e
				return f(e)
			}
			return nil
		})
	}
}

func failOnErr(err error) {
	if err != nil {
		log.Fatal(err)
	}
}
