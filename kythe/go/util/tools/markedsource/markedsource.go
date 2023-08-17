/*
 * Copyright 2023 The Kythe Authors. All rights reserved.
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

// Binary markedsource provides a utility for printing the resolved MarkedSource
// for any ticket from a delimited stream of wire-encoded kythe.proto.Entry
// messages.
//
// Example usages:
//
//	markedsource kythe://kythe#myTicket < entries
//	markedsource --rewrite < entries > rewritten.entries
package main

import (
	"bufio"
	"flag"
	"os"

	"kythe.io/kythe/go/platform/delimited"
	"kythe.io/kythe/go/storage/stream"
	"kythe.io/kythe/go/util/log"
	"kythe.io/kythe/go/util/markedsource"
	"kythe.io/kythe/go/util/schema/facts"

	"google.golang.org/protobuf/encoding/protojson"
	"google.golang.org/protobuf/proto"

	spb "kythe.io/kythe/proto/storage_go_proto"
)

var (
	rewrite = flag.Bool("rewrite", false, "Rewrite all code facts to be fully resolved")
)

func main() {
	flag.Parse()

	in := bufio.NewReaderSize(os.Stdin, 2*4096)
	rd := stream.NewReader(in)

	var entries []*spb.Entry
	if err := rd(func(e *spb.Entry) error {
		entries = append(entries, e)
		return nil
	}); err != nil {
		log.Exitf("Failed to read entrystream: %v", err)
	}

	r, err := markedsource.NewResolver(entries)
	if err != nil {
		log.Exitf("Failed to construct MarkedSource Resolver: %v", err)
	}

	if *rewrite {
		var rewritten int
		out := bufio.NewWriter(os.Stdout)
		wr := delimited.NewWriter(out)
		for _, e := range entries {
			if e.GetFactName() == facts.Code {
				resolved := r.Resolve(e.GetSource())
				e = proto.Clone(e).(*spb.Entry)
				rec, err := proto.Marshal(resolved)
				if err != nil {
					log.Exitf("Error marshalling resolved MarkedSource: %v", err)
				}
				e.FactValue = rec
				rewritten++
			}
			if err := wr.PutProto(e); err != nil {
				log.Exit(err)
			}
		}
		if err := out.Flush(); err != nil {
			log.Exit(err)
		}
		log.Infof("Rewrote %d code facts", rewritten)
		return
	}

	wr := bufio.NewWriter(os.Stdout)
	for _, ticket := range flag.Args() {
		rec, err := protojson.Marshal(r.ResolveTicket(ticket))
		if err != nil {
			log.Exitf("Error encoding MarkedSource: %v", err)
		}
		if _, err := wr.Write(rec); err != nil {
			log.Exitf("Error writing MarkedSource: %v", err)
		}
	}
	if err := wr.Flush(); err != nil {
		log.Exitf("Error flushing stdout: %v", err)
	}
}
