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

	renderSignature         = flag.Bool("render_signatures", false, "Whether to emit /kythe/code/rendered/signature facts (requires --rewrite)")
	renderCallsiteSignature = flag.Bool("render_callsite_signatures", false, "Whether to emit /kythe/code/rendered/callsite_signature facts (requires --rewrite)")
	renderQualifiedName     = flag.Bool("render_qualified_names", false, "Whether to emit /kythe/code/rendered/qualified_name facts (requires --rewrite)")
)

const renderFactPrefix = facts.Code + "/rendered/"

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
	exitOnErrf("Failed to construct MarkedSource Resolver: %v", err)

	if *rewrite {
		var rewritten int
		out := bufio.NewWriter(os.Stdout)
		wr := delimited.NewWriter(out)
		for _, e := range entries {
			if e.GetFactName() == facts.Code {
				resolved := r.Resolve(e.GetSource())
				e = proto.Clone(e).(*spb.Entry)
				rec, err := proto.Marshal(resolved)
				exitOnErrf("Error marshalling resolved MarkedSource: %v", err)
				e.FactValue = rec
				rewritten++

				if *renderQualifiedName {
					val := markedsource.RenderSimpleQualifiedName(resolved, true, markedsource.PlaintextContent, nil)
					exitOnErr(wr.PutProto(&spb.Entry{
						Source:    e.Source,
						FactName:  renderFactPrefix + "qualified_name",
						FactValue: []byte(val),
					}))
				}
				if *renderCallsiteSignature {
					val := markedsource.RenderCallSiteSignature(resolved)
					exitOnErr(wr.PutProto(&spb.Entry{
						Source:    e.Source,
						FactName:  renderFactPrefix + "callsite_signature",
						FactValue: []byte(val),
					}))
				}
				if *renderSignature {
					val := markedsource.RenderSignature(resolved, markedsource.PlaintextContent, nil)
					exitOnErr(wr.PutProto(&spb.Entry{
						Source:    e.Source,
						FactName:  renderFactPrefix + "signature",
						FactValue: []byte(val),
					}))
				}
			}
			exitOnErr(wr.PutProto(e))
		}
		exitOnErr(out.Flush())
		log.Infof("Rewrote %d code facts", rewritten)
		return
	}

	wr := bufio.NewWriter(os.Stdout)
	for _, ticket := range flag.Args() {
		rec, err := protojson.Marshal(r.ResolveTicket(ticket))
		exitOnErrf("Error encoding MarkedSource: %v", err)
		if _, err := wr.Write(rec); err != nil {
			log.Exitf("Error writing MarkedSource: %v", err)
		}
	}
	exitOnErrf("Error flushing stdout: %v", wr.Flush())
}

func exitOnErrf(msg string, err error) {
	if err != nil {
		log.Exitf(msg, err)
	}
}

func exitOnErr(err error) {
	if err != nil {
		log.Exit(err)
	}
}
