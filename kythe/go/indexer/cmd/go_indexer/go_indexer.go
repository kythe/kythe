/*
 * Copyright 2016 The Kythe Authors. All rights reserved.
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

// Program go_indexer implements a Kythe indexer for the Go language.  Input is
// read from one or more .kzip paths.
package main

import (
	"bytes"
	"context"
	"encoding/json"
	"flag"
	"fmt"
	"net/url"
	"os"
	"path/filepath"
	"strings"

	"kythe.io/kythe/go/indexer"
	"kythe.io/kythe/go/platform/delimited"
	"kythe.io/kythe/go/platform/kzip"
	"kythe.io/kythe/go/util/log"
	"kythe.io/kythe/go/util/metadata"

	"github.com/golang/protobuf/proto"

	protopb "github.com/golang/protobuf/protoc-gen-go/descriptor"
	apb "kythe.io/kythe/proto/analysis_go_proto"
	spb "kythe.io/kythe/proto/storage_go_proto"
)

var (
	doJSON                         = flag.Bool("json", false, "Write output as JSON")
	doLibNodes                     = flag.Bool("libnodes", false, "Emit nodes for standard library packages")
	doCodeFacts                    = flag.Bool("code", false, "Emit code facts containing MarkedSource markup")
	doAnchorScopes                 = flag.Bool("anchor_scopes", false, "Emit childof edges to an anchor's semantic scope")
	metaSuffix                     = flag.String("meta", "", "If set, treat files with this suffix as JSON linkage metadata")
	docBase                        = flag.String("docbase", "http://godoc.org", "If set, use as the base URL for godoc links")
	onlyEmitDocURIsForStandardLibs = flag.Bool("only_emit_doc_uris_for_standard_libs", false, "If true, the doc/uri fact is only emitted for go std library packages")
	emitRefCallOverIdentifier      = flag.Bool("emit_ref_call_over_identifier", false, "If true, emit ref/call anchor spans over the function identifier")
	verbose                        = flag.Bool("verbose", false, "Emit verbose log information")
	contOnErr                      = flag.Bool("continue", false, "Log errors encountered during analysis but do not exit unsuccessfully")
	useCompilationCorpusForAll     = flag.Bool("use_compilation_corpus_for_all", false, "If enabled, all Entry VNames are given the corpus of the compilation unit being indexed. This includes items in the go std library and builtin types.")
	useFileAsTopLevelScope         = flag.Bool("use_file_as_top_level_scope", false, "If enabled, use the file node for top-level callsite scopes")
	overrideStdlibCorpus           = flag.String("override_stdlib_corpus", "", "If set, all stdlib nodes are assigned this corpus. Note that this takes precedence over --use_compilation_corpus_for_all")

	writeEntry func(context.Context, *spb.Entry) error
	docURL     *url.URL
)

func init() {
	flag.Usage = func() {
		fmt.Fprintf(os.Stderr, `Usage: %s [options] <path>...

Generate Kythe graph data for the compilations stored in .kzip format
named by the path arguments. Output is written to stdout.

By default, the output is a delimited stream of wire-format Kythe Entry
protobuf messages. With the --json flag, output is instead a stream of
undelimited JSON messages.

Options:
`, filepath.Base(os.Args[0]))

		flag.PrintDefaults()
	}
}

func main() {
	flag.Parse()

	if flag.NArg() == 0 {
		log.Fatal("No input paths were specified to index")
	}
	if *doJSON {
		enc := json.NewEncoder(os.Stdout)
		writeEntry = func(_ context.Context, entry *spb.Entry) error {
			return enc.Encode(entry)
		}
	} else {
		rw := delimited.NewWriter(os.Stdout)
		writeEntry = func(_ context.Context, entry *spb.Entry) error {
			return rw.PutProto(entry)
		}
	}
	if *docBase != "" {
		u, err := url.Parse(*docBase)
		if err != nil {
			log.Fatalf("Invalid doc base URL: %v", err)
		}
		docURL = u
	}

	ctx := context.Background()
	for _, path := range flag.Args() {
		if err := visitPath(ctx, path, func(ctx context.Context, unit *apb.CompilationUnit, f indexer.Fetcher) error {
			err := indexGo(ctx, unit, f)
			if err != nil && *contOnErr {
				log.Errorf("Continuing after error: %v", err)
				return nil
			}
			return err
		}); err != nil {
			log.Fatalf("Error indexing %q: %v", path, err)
		}
	}
}

// checkMetadata checks whether ri denotes a metadata file according to the
// setting of the -meta flag, and if so loads the corresponding ruleset.
func checkMetadata(ri *apb.CompilationUnit_FileInput, f indexer.Fetcher) (*indexer.Ruleset, error) {
	if *metaSuffix == "" || !strings.HasSuffix(ri.Info.GetPath(), *metaSuffix) {
		return nil, nil // nothing to do
	}
	bits, err := f.Fetch(ri.Info.GetPath(), ri.Info.GetDigest())
	if err != nil {
		return nil, fmt.Errorf("reading metadata file: %w", err)
	}
	rules, err := metadata.Parse(bytes.NewReader(bits))
	if err != nil {
		// Check if file is actually a GeneratedCodeInfo proto.
		var gci protopb.GeneratedCodeInfo
		if err := proto.UnmarshalText(string(bits), &gci); err != nil {
			return nil, fmt.Errorf("cannot parse .meta file as JSON or textproto: %w", err)
		}
		rules = metadata.FromGeneratedCodeInfo(&gci, ri.VName)
	}
	return &indexer.Ruleset{
		Path:  strings.TrimSuffix(ri.Info.GetPath(), *metaSuffix),
		Rules: rules,
	}, nil
}

// indexGo is a visitFunc that invokes the Kythe Go indexer on unit.
func indexGo(ctx context.Context, unit *apb.CompilationUnit, f indexer.Fetcher) error {
	pi, err := indexer.Resolve(unit, f, &indexer.ResolveOptions{
		Info:       indexer.XRefTypeInfo(),
		CheckRules: checkMetadata,
	})
	if err != nil {
		return err
	}
	if *verbose {
		log.Infof("Finished resolving compilation: %s", pi.String())
	}
	return pi.Emit(ctx, writeEntry, &indexer.EmitOptions{
		EmitStandardLibs:               *doLibNodes,
		EmitMarkedSource:               *doCodeFacts,
		EmitAnchorScopes:               *doAnchorScopes,
		EmitLinkages:                   *metaSuffix != "",
		DocBase:                        docURL,
		OnlyEmitDocURIsForStandardLibs: *onlyEmitDocURIsForStandardLibs,
		UseCompilationCorpusForAll:     *useCompilationCorpusForAll,
		UseFileAsTopLevelScope:         *useFileAsTopLevelScope,
		OverrideStdlibCorpus:           *overrideStdlibCorpus,
		EmitRefCallOverIdentifier:      *emitRefCallOverIdentifier,
		Verbose:                        *verbose,
	})
}

type visitFunc func(context.Context, *apb.CompilationUnit, indexer.Fetcher) error

// visitPath invokes visit for each compilation denoted by path, which is
// must be a .kzip file (with a single compilation).
func visitPath(ctx context.Context, path string, visit visitFunc) error {
	f, err := os.Open(path)
	if err != nil {
		return err
	}
	defer f.Close()
	switch ext := filepath.Ext(path); ext {
	case ".kzip":
		return kzip.Scan(f, func(r *kzip.Reader, unit *kzip.Unit) error {
			return visit(ctx, unit.Proto, kzipFetcher{r})
		})

	default:
		return fmt.Errorf("unknown file extension %q", ext)
	}
}

type kzipFetcher struct{ r *kzip.Reader }

// Fetch implements the analysis.Fetcher interface. Only the digest is used in
// this implementation, the path is ignored.
func (k kzipFetcher) Fetch(_, digest string) ([]byte, error) {
	return k.r.ReadAll(digest)
}
