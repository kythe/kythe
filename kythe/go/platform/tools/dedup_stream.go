// Binary dedup_stream reads a delimited stream from stdin and writes a delimited stream to stdout.
// Each record in the stream will be hashed, and if that hash value has already been seen, the
// record will not be emitted.
package main

import (
	"crypto/sha512"
	"io"
	"log"
	"os"

	"kythe/go/platform/delimited"
)

func main() {
	written := make(map[[sha512.Size384]byte]struct{})

	var skipped uint64
	rd := delimited.NewReader(os.Stdin)
	wr := delimited.NewWriter(os.Stdout)
	for {
		rec, err := rd.Next()
		if err == io.EOF {
			break
		} else if err != nil {
			log.Fatal(err)
		}

		hash := sha512.Sum384(rec)
		if _, ok := written[hash]; ok {
			skipped++
			continue
		}
		if err := wr.Put(rec); err != nil {
			log.Fatal(err)
		}
		written[hash] = struct{}{}
	}
	log.Printf("dedup_stream: skipped %d records", skipped)
}
