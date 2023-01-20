// Binary markedsource provides a utility for printing the resolved MarkedSource
// for any ticket from a delimited stream of wire-encoded kythe.proto.Entry
// messages.
//
// Example usage:
//
//	markedsource kythe://kythe#myTicket < entries
package main

import (
	"bufio"
	"flag"
	"log"
	"os"

	"kythe.io/kythe/go/storage/stream"
	"kythe.io/kythe/go/util/markedsource"

	"google.golang.org/protobuf/encoding/protojson"

	spb "kythe.io/kythe/proto/storage_go_proto"
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
		log.Fatalf("Failed to read entrystream: %v", err)
	}

	r, err := markedsource.NewResolver(entries)
	if err != nil {
		log.Fatalf("Failed to construct MarkedSource Resolver: %v", err)
	}

	wr := bufio.NewWriter(os.Stdout)
	for _, ticket := range flag.Args() {
		rec, err := protojson.Marshal(r.ResolveTicket(ticket))
		if err != nil {
			log.Fatalf("Error encoding MarkedSource: %v", err)
		}
		if _, err := wr.Write(rec); err != nil {
			log.Fatalf("Error writing MarkedSource: %v", err)
		}
	}
	if err := wr.Flush(); err != nil {
		log.Fatalf("Error flushing stdout: %v", err)
	}
}
