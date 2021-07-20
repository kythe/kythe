// Binary empty_corpus_checker reads an entrystream (in delimited binary proto
// form) from stdin and checks for any node vnames that have an empty corpus.
// Nodes with an empty corpus are logged to stderr and cause the binary to
// return a non-zero exit code.
package main

import (
	"bufio"
	"flag"
	"log"
	"os"

	"kythe.io/kythe/go/storage/entryset"
	"kythe.io/kythe/go/storage/stream"
	"kythe.io/kythe/go/util/flagutil"
	"kythe.io/kythe/go/util/kytheuri"

	intpb "kythe.io/kythe/proto/internal_go_proto"
	spb "kythe.io/kythe/proto/storage_go_proto"
)

func init() {
	flag.Usage = flagutil.SimpleUsage("Checks a stream of Entry protos via stdin for empty vname.corpus")
}

func main() {
	flag.Parse()

	in := bufio.NewReaderSize(os.Stdin, 2*4096)
	rd := stream.NewReader(in)

	set := entryset.New(nil)
	err := rd(func(entry *spb.Entry) error {
		return set.Add(entry)
	})
	if err != nil {
		log.Fatalf("Failed to read entrystream: %v", err)
	}
	set.Canonicalize()

	emptyCorpusCount := 0
	set.Sources(func(src *intpb.Source) bool {
		r, err := kytheuri.ParseRaw(src.GetTicket())
		if err != nil {
			log.Fatalf("Error parsing ticket: %q, %v", src.GetTicket(), err)
		}

		if r.URI.Corpus == "" {
			log.Printf("Found source with empty corpus: %v", src)
			emptyCorpusCount += 1
		}

		return true
	})
	if emptyCorpusCount != 0 {
		log.Fatalf("FAILURE: found %d sources with empty corpus", emptyCorpusCount)
	}
}
