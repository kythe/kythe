/*
 * Copyright 2015 Google Inc. All rights reserved.
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

// Binary write_tables creates a combined xrefs/filetree/search serving table
// based on a given GraphStore.
package main

import (
	"context"
	"errors"
	"flag"
	"log"

	"kythe.io/kythe/go/platform/vfs"
	"kythe.io/kythe/go/services/graphstore"
	"kythe.io/kythe/go/serving/pipeline"
	"kythe.io/kythe/go/serving/pipeline/beamio"
	"kythe.io/kythe/go/storage/gsutil"
	"kythe.io/kythe/go/storage/leveldb"
	"kythe.io/kythe/go/storage/stream"
	"kythe.io/kythe/go/util/flagutil"
	"kythe.io/kythe/go/util/profile"

	spb "kythe.io/kythe/proto/storage_go_proto"

	"github.com/apache/beam/sdks/go/pkg/beam"
	"github.com/apache/beam/sdks/go/pkg/beam/x/beamx"

	_ "kythe.io/kythe/go/services/graphstore/proxy"
	_ "kythe.io/third_party/beam/sdks/go/pkg/beam/runners/disksort"
)

var (
	gs          graphstore.Service
	entriesFile = flag.String("entries", "", "Path to GraphStore-ordered entries file (mutually exclusive with --graphstore)")

	tablePath = flag.String("out", "", "Directory path to output serving table")

	maxPageSize = flag.Int("max_page_size", 4000,
		"If positive, edge/cross-reference pages are restricted to under this number of edges/references")
	compressShards = flag.Bool("compress_shards", false,
		"Determines whether intermediate data written to disk should be compressed.")
	maxShardSize = flag.Int("max_shard_size", 32000,
		"Maximum number of elements (edges, decoration fragments, etc.) to keep in-memory before flushing an intermediary data shard to disk.")

	verbose = flag.Bool("verbose", false, "Whether to emit extra, and possibly excessive, log messages")

	experimentalBeamPipeline = flag.Bool("experimental_beam_pipeline", false, "Whether to use the Beam experimental pipeline implementation")
)

func init() {
	gsutil.Flag(&gs, "graphstore", "GraphStore to read (mutually exclusive with --entries)")
	flag.Usage = flagutil.SimpleUsage(
		"Creates a combined xrefs/filetree/search serving table based on a given GraphStore or stream of GraphStore-ordered entries",
		"(--graphstore spec | --entries path) --out path")
}

func main() {
	flag.Parse()
	beam.Init()
	ctx := context.Background()
	if *experimentalBeamPipeline {
		if err := runExperimentalBeamPipeline(ctx); err != nil {
			log.Fatalf("Pipeline error: %v", err)
		}
		return
	}

	if gs == nil && *entriesFile == "" {
		flagutil.UsageError("missing --graphstore or --entries")
	} else if gs != nil && *entriesFile != "" {
		flagutil.UsageError("--graphstore and --entries are mutually exclusive")
	} else if *tablePath == "" {
		flagutil.UsageError("missing required --out flag")
	}

	db, err := leveldb.Open(*tablePath, nil)
	if err != nil {
		log.Fatal(err)
	}
	defer db.Close()

	if err := profile.Start(ctx); err != nil {
		log.Fatal(err)
	}
	defer profile.Stop()

	var rd stream.EntryReader
	if gs != nil {
		rd = func(f func(e *spb.Entry) error) error {
			defer gs.Close(ctx)
			return gs.Scan(ctx, &spb.ScanRequest{}, f)
		}
	} else {
		f, err := vfs.Open(ctx, *entriesFile)
		if err != nil {
			log.Fatalf("Error opening %q: %v", *entriesFile, err)
		}
		defer f.Close()
		rd = stream.NewReader(f)
	}

	if err := pipeline.Run(ctx, rd, db, &pipeline.Options{
		Verbose:        *verbose,
		MaxPageSize:    *maxPageSize,
		CompressShards: *compressShards,
		MaxShardSize:   *maxShardSize,
	}); err != nil {
		log.Fatal("FATAL ERROR: ", err)
	}
}

func runExperimentalBeamPipeline(ctx context.Context) error {
	if runnerFlag := flag.Lookup("runner"); runnerFlag.Value.String() == "direct" {
		runnerFlag.Value.Set("disksort")
	}

	if gs != nil {
		return errors.New("--graphstore input not supported with --experimental_beam_pipeline")
	} else if *entriesFile == "" {
		return errors.New("--entries file path required")
	} else if *tablePath == "" {
		return errors.New("--out table path required")
	}

	p, s := beam.NewPipelineWithRoot()
	entries := beamio.ReadEntries(s, *entriesFile)
	k := pipeline.FromEntries(s, entries)
	shards := 8 // TODO(schroederc): better determine number of shards
	edgeSets, edgePages := k.Edges()
	xrefSets, xrefPages := k.CrossReferences()
	beamio.WriteLevelDB(s, *tablePath, shards,
		k.CorpusRoots(),
		k.Decorations(),
		k.Directories(),
		k.Documents(),
		xrefSets, xrefPages,
		edgeSets, edgePages,
	)
	return beamx.Run(ctx, p)
}
