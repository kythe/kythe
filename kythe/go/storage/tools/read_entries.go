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

// Binary read_entries scans the entries from a specified GraphStore and emits
// them to stdout as a delimited stream.
package main

import (
	"flag"
	"fmt"
	"log"
	"os"
	"path/filepath"
	"sync"

	"kythe/go/platform/delimited"
	"kythe/go/storage"
	"kythe/go/storage/gsutil"

	spb "kythe/proto/storage_proto"
)

var (
	gs storage.GraphStore

	count = flag.Bool("count", false, "Only print the number of entries scanned")

	shardsToFiles = flag.String("sharded_file", "", "If given, scan the entire GraphStore, storing each shard in a separate file instead of stdout (requires --shards)")
	shardIndex    = flag.Int64("shard_index", 0, "Index of a single shard to emit (requires --shards)")
	shards        = flag.Int64("shards", 0, "Number of shards to split the GraphStore")
)

func init() {
	gsutil.Flag(&gs, "graphstore", "GraphStore to read")
	flag.Usage = func() {
		fmt.Fprintf(os.Stderr, `%s - scans the entries from a GraphStore, emitting a delimited entry stream to stdout
usage: %[1]s --graphstore spec
`, filepath.Base(os.Args[0]))
		flag.PrintDefaults()
		os.Exit(1)
	}
}

func main() {
	flag.Parse()
	if gs == nil {
		log.Fatal("Missing --graphstore")
	} else if *shardsToFiles != "" && *shards <= 0 {
		log.Fatal("--sharded_file and --shards must be given together")
	}

	wr := delimited.NewWriter(os.Stdout)
	var total int64
	if *shards <= 0 {
		if err := storage.EachScanEntry(gs, nil, func(entry *spb.Entry) error {
			if *count {
				total++
				return nil
			}
			return wr.PutProto(entry)
		}); err != nil {
			log.Fatalf("GraphStore Scan error: %v", err)
		}
		if *count {
			fmt.Println(total)
		}
		return
	}

	sgs, ok := gs.(storage.ShardedGraphStore)
	if !ok {
		log.Fatalf("Sharding unsupported for given GraphStore type: %T", gs)
	} else if *shardIndex >= *shards {
		log.Fatalf("Invalid shard index for %d shards: %d", *shards, *shardIndex)
	}

	if *count {
		cnt, err := sgs.Count(&spb.CountRequest{Index: shardIndex, Shards: shards})
		if err != nil {
			log.Fatalf("ERROR: %v", err)
		}
		fmt.Println(cnt)
		return
	} else if *shardsToFiles != "" {
		var wg sync.WaitGroup
		wg.Add(int(*shards))
		for i := int64(0); i < *shards; i++ {
			go func(i int64) {
				defer wg.Done()
				path := fmt.Sprintf("%s-%.5d-of-%.5d", *shardsToFiles, i, *shards)
				f, err := os.Create(path)
				if err != nil {
					log.Fatalf("Failed to create file %q: %v", path, err)
				}
				defer f.Close()
				wr := delimited.NewWriter(f)
				if err := storage.EachShardEntry(sgs, &spb.ShardRequest{
					Index:  &i,
					Shards: shards,
				}, func(entry *spb.Entry) error {
					return wr.PutProto(entry)
				}); err != nil {
					log.Fatalf("GraphStore shard scan error: %v", err)
				}
			}(i)
		}
		wg.Wait()
		return
	}

	if err := storage.EachShardEntry(sgs, &spb.ShardRequest{
		Index:  shardIndex,
		Shards: shards,
	}, func(entry *spb.Entry) error {
		return wr.PutProto(entry)
	}); err != nil {
		log.Fatalf("GraphStore shard scan error: %v", err)
	}
}
