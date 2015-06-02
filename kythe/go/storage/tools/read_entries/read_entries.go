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

	"kythe.io/kythe/go/platform/delimited"
	"kythe.io/kythe/go/services/graphstore"
	"kythe.io/kythe/go/storage/gsutil"
	"kythe.io/kythe/go/util/kytheuri"

	spb "kythe.io/kythe/proto/storage_proto"
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
	flag.Usage = func() {
		fmt.Fprintf(os.Stderr, `%s - scans/reads the entries from a GraphStore, emitting a delimited entry stream to stdout
usage: %[1]s --graphstore spec [ticket...]
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
	} else if *shards > 0 && len(flag.Args()) > 0 {
		log.Fatal("--shards and giving tickets for reads are mutually exclusive")
	}

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
			if err := readEntries(gs, entryFunc, *edgeKind, flag.Args()); err != nil {
				log.Fatal(err)
			}
		} else {
			if err := scanEntries(gs, entryFunc, *edgeKind, *targetTicket, *factPrefix); err != nil {
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
		cnt, err := sgs.Count(&spb.CountRequest{Index: *shardIndex, Shards: *shards})
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
				if err := sgs.Shard(&spb.ShardRequest{
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

	if err := sgs.Shard(&spb.ShardRequest{
		Index:  *shardIndex,
		Shards: *shards,
	}, func(entry *spb.Entry) error {
		return wr.PutProto(entry)
	}); err != nil {
		log.Fatalf("GraphStore shard scan error: %v", err)
	}
}

func readEntries(gs graphstore.Service, entryFunc graphstore.EntryFunc, edgeKind string, tickets []string) error {
	for _, ticket := range tickets {
		src, err := kytheuri.ToVName(ticket)
		if err != nil {
			return fmt.Errorf("error parsing ticket %q: %v", ticket, err)
		}
		if err := gs.Read(&spb.ReadRequest{
			Source:   src,
			EdgeKind: edgeKind,
		}, entryFunc); err != nil {
			return fmt.Errorf("GraphStore Read error for ticket %q: %v", ticket, err)
		}
	}
	return nil
}

func scanEntries(gs graphstore.Service, entryFunc graphstore.EntryFunc, edgeKind, targetTicket, factPrefix string) error {
	var target *spb.VName
	var err error
	if targetTicket != "" {
		target, err = kytheuri.ToVName(targetTicket)
		if err != nil {
			return fmt.Errorf("error parsing --target %q: %v", targetTicket, err)
		}
	}
	if err := gs.Scan(&spb.ScanRequest{
		EdgeKind:   edgeKind,
		FactPrefix: factPrefix,
		Target:     target,
	}, entryFunc); err != nil {
		return fmt.Errorf("GraphStore Scan error: %v", err)
	}
	return nil
}
