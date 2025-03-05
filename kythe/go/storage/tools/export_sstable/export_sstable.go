package main

import (
	"context"
	"flag"
	"io"

	"github.com/cockroachdb/pebble/objstorage/objstorageprovider"
	"github.com/cockroachdb/pebble/sstable"
	"github.com/cockroachdb/pebble/vfs"
	"kythe.io/kythe/go/storage/kvutil"
	"kythe.io/kythe/go/util/flagutil"
	"kythe.io/kythe/go/util/log"
	"kythe.io/kythe/go/util/profile"

	_ "kythe.io/kythe/go/services/graphstore/proxy"
	_ "kythe.io/kythe/go/storage/leveldb"
	_ "kythe.io/kythe/go/storage/pebble"
)

var (
	input  = flag.String("input", "", "Path to input db")
	output = flag.String("output", "", "Path to output sstable")
)

func init() {
	flag.Usage = flagutil.SimpleUsage("Dump contents of a GraphStore db to a single SSTable file",
		"--input spec --output /path/to/sstable")
}

func main() {
	flag.Parse()
	if *input == "" {
		flagutil.UsageError("Missing --input")
	} else if *output == "" {
		flagutil.UsageError("Missing --output")
	}

	ctx := context.Background()

	db, err := kvutil.ParseDB(*input)
	if err != nil {
		log.Fatal(err)
	}
	defer kvutil.LogClose(ctx, db)
	kvutil.EnsureGracefulExit(db)

	if err := profile.Start(ctx); err != nil {
		log.Fatal(err)
	}
	defer profile.Stop()

	f, err := vfs.Default.Create(*output)
	if err != nil {
		log.Fatalf("error opening output path: %s", err)
	}
	w := sstable.NewWriter(objstorageprovider.NewFileWritable(f), sstable.WriterOptions{})

	iter, err := db.ScanPrefix(context.TODO(), nil, nil)
	if err != nil {
		log.Fatalf("error opening iter: %s", err)
	}
	defer iter.Close()

	for {
		k, v, err := iter.Next()
		if err == io.EOF {
			break
		}
		if err := w.Set(k, v); err != nil {
			log.Fatalf("Error writing to sstable: %s", err)
		}
	}
	if err := w.Close(); err != nil {
		log.Fatalf("Error closing sstable: %s", err)
	}
}
