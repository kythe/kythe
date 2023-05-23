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

// Binary write_entries reads a stream of protobuf entries on os.Stdin and
// writes each to a graphstore server.
//
// Usage:
//
//	entry_emitter ... | write_entries --graphstore addr
//
// Example:
//
//	java_indexer_server --port 8181 &
//	graphstore --port 9999 &
//	analysis_driver --analyzer localhost:8181 /tmp/compilation.kzip | \
//	  write_entries --workers 10 --graphstore localhost:9999
//
// Example:
//
//	zcat entries.gz | write_entries --graphstore gs/leveldb
package main

import (
	"context"
	"flag"
	"os"
	"sync"
	"sync/atomic"

	"kythe.io/kythe/go/services/graphstore"
	"kythe.io/kythe/go/storage/gsutil"
	"kythe.io/kythe/go/storage/stream"
	"kythe.io/kythe/go/util/flagutil"
	"kythe.io/kythe/go/util/log"
	"kythe.io/kythe/go/util/profile"

	spb "kythe.io/kythe/proto/storage_go_proto"

	_ "kythe.io/kythe/go/services/graphstore/proxy"
	_ "kythe.io/kythe/go/storage/leveldb"
)

var (
	batchSize  = flag.Int("batch_size", 1024, "Maximum entries per write for consecutive entries with the same source")
	numWorkers = flag.Int("workers", 1, "Number of concurrent workers writing to the GraphStore")

	gs graphstore.Service
)

func init() {
	flag.Usage = flagutil.SimpleUsage("Write a delimited stream of entries from stdin to a GraphStore",
		"[--batch_size entries] [--workers n] --graphstore spec")
	gsutil.Flag(&gs, "graphstore", "GraphStore to which to write the entry stream")
}

func main() {
	flag.Parse()
	if *numWorkers < 1 {
		flagutil.UsageErrorf("Invalid number of --workers %d (must be ≥ 1)", *numWorkers)
	} else if *batchSize < 1 {
		flagutil.UsageErrorf("Invalid --batch_size %d (must be ≥ 1)", *batchSize)
	} else if gs == nil {
		flagutil.UsageError("Missing --graphstore")
	}

	ctx := context.Background()

	defer gsutil.LogClose(ctx, gs)
	gsutil.EnsureGracefulExit(gs)

	if err := profile.Start(ctx); err != nil {
		log.Fatal(err)
	}
	defer profile.Stop()

	writes := graphstore.BatchWrites(stream.ReadEntries(os.Stdin), *batchSize)

	var (
		wg         sync.WaitGroup
		numEntries uint64
	)
	wg.Add(*numWorkers)
	for i := 0; i < *numWorkers; i++ {
		go func() {
			defer wg.Done()
			num, err := writeEntries(ctx, gs, writes)
			if err != nil {
				log.Fatal(err)
			}

			atomic.AddUint64(&numEntries, num)
		}()
	}
	wg.Wait()

	log.Infof("Wrote %d entries", numEntries)
}

func writeEntries(ctx context.Context, s graphstore.Service, reqs <-chan *spb.WriteRequest) (uint64, error) {
	var num uint64

	for req := range reqs {
		num += uint64(len(req.Update))
		if err := s.Write(ctx, req); err != nil {
			return 0, err
		}
	}

	return num, nil
}
