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

// Binary read_entries scans the entries from a specified GraphStore and emits
// them to stdout as a delimited stream.
package main

import (
	"context"
	"flag"
	"fmt"
	"os"
	"sync"

	"kythe.io/kythe/go/platform/delimited"
	"kythe.io/kythe/go/platform/vfs"
	"kythe.io/kythe/go/services/graphstore"
	"kythe.io/kythe/go/storage/gsutil"
	"kythe.io/kythe/go/util/flagutil"
	"kythe.io/kythe/go/util/kytheuri"
	"kythe.io/kythe/go/util/log"

	spb "kythe.io/kythe/proto/storage_go_proto"

	_ "kythe.io/kythe/go/services/graphstore/proxy"
	_ "kythe.io/kythe/go/storage/leveldb"
)

var (
	gs graphstore.Service

	count = flag.Bool("count", false, "Only print the number of entries scanned")

	shardsToFiles = flag.String("sharded_file", "", "If given, scan the entire GraphStore, storing each shard in a separate file instead of stdout (requires --shards)")
	shardIndex    = flag.Int64("shard_index", 0, "Index of a single shard to emit (requires --shards)")
	shards        = flag.Int64("shards", 0, "Number of shards to split the GraphStore")

	edgeKind     = flag.String("edge_kind", "", "Edge kind by which to filter a read/scan")
	targetTicket = flag.String("target", "", "Ticket of target by which to filter a scan")
	factPrefix   = flag.String("fact_prefix", "", "Fact prefix by which to filter a scan")
)

func init() {
	gsutil.Flag(&gs, "graphstore", "GraphStore to read")
	flag.Usage = flagutil.SimpleUsage("Scans/reads the entries from a GraphStore, emitting a delimited entry stream to stdout",
		"--graphstore spec [--count] [--shards N [--shard_index I] --sharded_file path] [--edge_kind] ([--fact_prefix str] [--target ticket] | [ticket...])")
}

func main() {
	flag.Parse()
	if gs == nil {
		flagutil.UsageError("missing --graphstore")
	} else if *shardsToFiles != "" && *shards <= 0 {
		flagutil.UsageError("--sharded_file and --shards must be given together")
	} else if *shards > 0 && len(flag.Args()) > 0 {
		flagutil.UsageError("--shards and giving tickets for reads are mutually exclusive")
	}

	ctx := context.Background()

	wr := delimited.NewWriter(os.Stdout)
	var total int64
	if *shards <= 0 {
		entryFunc := func(entry *spb.Entry) error {
			if *count {
				total++
				return nil
			}
			return wr.PutProto(entry)
		}
		if len(flag.Args()) > 0 {
			if *targetTicket != "" || *factPrefix != "" {
				log.Fatal("--target and --fact_prefix are unsupported when given tickets")
			}
			if err := readEntries(ctx, gs, entryFunc, *edgeKind, flag.Args()); err != nil {
				log.Fatal(err)
			}
		} else {
			if err := scanEntries(ctx, gs, entryFunc, *edgeKind, *targetTicket, *factPrefix); err != nil {
				log.Fatal(err)
			}
		}
		if *count {
			fmt.Println(total)
		}
		return
	}

	sgs, ok := gs.(graphstore.Sharded)
	if !ok {
		log.Fatalf("Sharding unsupported for given GraphStore type: %T", gs)
	} else if *shardIndex >= *shards {
		log.Fatalf("Invalid shard index for %d shards: %d", *shards, *shardIndex)
	}

	if *count {
		cnt, err := sgs.Count(ctx, &spb.CountRequest{Index: *shardIndex, Shards: *shards})
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
				f, err := vfs.Create(ctx, path)
				if err != nil {
					log.Fatalf("Failed to create file %q: %v", path, err)
				}
				defer f.Close()
				wr := delimited.NewWriter(f)
				if err := sgs.Shard(ctx, &spb.ShardRequest{
					Index:  i,
					Shards: *shards,
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

	if err := sgs.Shard(ctx, &spb.ShardRequest{
		Index:  *shardIndex,
		Shards: *shards,
	}, func(entry *spb.Entry) error {
		return wr.PutProto(entry)
	}); err != nil {
		log.Fatalf("GraphStore shard scan error: %v", err)
	}
}

func readEntries(ctx context.Context, gs graphstore.Service, entryFunc graphstore.EntryFunc, edgeKind string, tickets []string) error {
	for _, ticket := range tickets {
		src, err := kytheuri.ToVName(ticket)
		if err != nil {
			return fmt.Errorf("error parsing ticket %q: %v", ticket, err)
		}
		if err := gs.Read(ctx, &spb.ReadRequest{
			Source:   src,
			EdgeKind: edgeKind,
		}, entryFunc); err != nil {
			return fmt.Errorf("GraphStore Read error for ticket %q: %v", ticket, err)
		}
	}
	return nil
}

func scanEntries(ctx context.Context, gs graphstore.Service, entryFunc graphstore.EntryFunc, edgeKind, targetTicket, factPrefix string) error {
	var target *spb.VName
	var err error
	if targetTicket != "" {
		target, err = kytheuri.ToVName(targetTicket)
		if err != nil {
			return fmt.Errorf("error parsing --target %q: %v", targetTicket, err)
		}
	}
	if err := gs.Scan(ctx, &spb.ScanRequest{
		EdgeKind:   edgeKind,
		FactPrefix: factPrefix,
		Target:     target,
	}, entryFunc); err != nil {
		return fmt.Errorf("GraphStore Scan error: %v", err)
	}
	return nil
}
