/*
 * Copyright 2015 The Kythe Authors. All rights reserved.
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

// Binary kwazthis (K, what's this?) determines what references are located at a
// particular offset (or line and column) within a file.  All results are
// printed as JSON.
//
// By default, kwazthis will search for a .kythe configuration file in a
// directory above the given --path (if it exists locally relative to the
// current working directory).  If found, --path will be made relative to this
// directory and --root before making any Kythe service requests.  If not found,
// --path will be passed unchanged.  --ignore_local_repo will turn off this
// behavior.
//
// Usage:
//
//	kwazthis --path kythe/cxx/tools/kindex_tool_main.cc --offset 2660
//	kwazthis --path kythe/cxx/common/CommandLineUtils.cc --line 81 --column 27
//	kwazthis --path kythe/java/com/google/devtools/kythe/analyzers/base/EntrySet.java --offset 2815
package main

import (
	"context"
	"encoding/json"
	"flag"
	"io/ioutil"
	"os"
	"path/filepath"
	"strconv"
	"strings"

	"kythe.io/kythe/go/platform/vfs"
	"kythe.io/kythe/go/services/graph"
	"kythe.io/kythe/go/services/xrefs"
	"kythe.io/kythe/go/serving/api"
	"kythe.io/kythe/go/util/flagutil"
	"kythe.io/kythe/go/util/kytheuri"
	"kythe.io/kythe/go/util/log"
	"kythe.io/kythe/go/util/schema"
	"kythe.io/kythe/go/util/schema/edges"
	"kythe.io/kythe/go/util/schema/facts"
	"kythe.io/kythe/go/util/schema/tickets"

	cpb "kythe.io/kythe/proto/common_go_proto"
	gpb "kythe.io/kythe/proto/graph_go_proto"
	spb "kythe.io/kythe/proto/storage_go_proto"
	xpb "kythe.io/kythe/proto/xref_go_proto"
)

func init() {
	flag.Usage = flagutil.SimpleUsage(`Determine what references are located at a particular offset (or line and column) within a file.

kwazthis normally searches for a .kythe configuration file in a directory above
the given --path (if it exists locally relative to the current working
directory).  If found, --path will be made relative to this directory and --root
before making any Kythe service requests.  If not found, --path will be passed
unchanged.

If the given --path file is found locally and --dirty_buffer is unset,
--dirty_buffer is automatically set to found local file and sent to the server .

--local_repo supplies kwazthis with the corpus root without searching the
filesystem for the .kythe file and --local_repo=NONE will turn off all local
filesystem behavior completely (including the automatic --dirty_buffer
feature).`,
		`(--offset int | --line int --column int) (--path p | --signature s)
[--corpus c] [--root r] [--language l]
[--api spec] [--local_repo root] [--dirty_buffer path] [--skip_defs]`)
}

var (
	ctx = context.Background()

	apiFlag = api.Flag("api", api.CommonDefault, api.CommonFlagUsage)

	localRepoRoot = flag.String("local_repo", "",
		`Path to local repository root ("" indicates to search for a .kythe configuration file in a directory about the given --path; "NONE" completely disables all local repository behavior)`)

	dirtyBuffer = flag.String("dirty_buffer", "", "Path to file with dirty buffer contents (optional)")

	path   = flag.String("path", "", "Path of file")
	corpus = flag.String("corpus", "", "Corpus of file VName")
	root   = flag.String("root", "", "Root of file VName")

	offset       = flag.Int("offset", -1, "Non-negative offset in file to list references (mutually exclusive with --line and --column)")
	lineNumber   = flag.Int("line", -1, "1-based line number in file to list references (must be given with --column)")
	columnOffset = flag.Int("column", -1, "Non-negative column offset in file to list references (must be given with --line)")

	skipDefinitions = flag.Bool("skip_defs", false, "Skip listing definitions for each node")
)

var (
	xs xrefs.Service
	gs graph.Service
)

type definition struct {
	File  *spb.VName `json:"file"`
	Start int        `json:"start"`
	End   int        `json:"end"`
}

type reference struct {
	Span struct {
		Start int    `json:"start"`
		End   int    `json:"end"`
		Text  string `json:"text,omitempty"`
	} `json:"span"`
	Kind string `json:"kind"`

	Node struct {
		Ticket  string   `json:"ticket"`
		Names   []string `json:"names,omitempty"`
		Kind    string   `json:"kind,omitempty"`
		Subkind string   `json:"subkind,omitempty"`
		Typed   string   `json:"typed,omitempty"`

		Definitions []*definition `json:"definitions,omitempty"`
	} `json:"node"`
}

var (
	definedAtEdge        = edges.Mirror(edges.Defines)
	definedBindingAtEdge = edges.Mirror(edges.DefinesBinding)
)

func main() {
	flag.Parse()
	if flag.NArg() > 0 {
		flagutil.UsageErrorf("unknown non-flag argument(s): %v", flag.Args())
	} else if *offset < 0 && (*lineNumber < 0 || *columnOffset < 0) {
		flagutil.UsageError("non-negative --offset (or --line and --column) required")
	} else if *path == "" {
		flagutil.UsageError("must provide --path")
	}

	defer (*apiFlag).Close(ctx)
	xs = *apiFlag
	gs = *apiFlag

	relPath := *path
	if *localRepoRoot != "NONE" {
		if _, err := os.Stat(relPath); err == nil {
			absPath, err := filepath.Abs(relPath)
			if err != nil {
				log.Fatal(err)
			}
			if *dirtyBuffer == "" {
				*dirtyBuffer = absPath
			}

			kytheRoot := *localRepoRoot
			if kytheRoot == "" {
				kytheRoot = findKytheRoot(filepath.Dir(absPath))
			}
			if kytheRoot != "" {
				relPath, err = filepath.Rel(filepath.Join(kytheRoot, *root), absPath)
				if err != nil {
					log.Fatal(err)
				}
			}
		}
	}

	fileTicket := (&kytheuri.URI{Corpus: *corpus, Root: *root, Path: relPath}).String()
	point := &cpb.Point{
		ByteOffset:   int32(*offset),
		LineNumber:   int32(*lineNumber),
		ColumnOffset: int32(*columnOffset),
	}
	dirtyBuffer := readDirtyBuffer(ctx)
	decor, err := xs.Decorations(ctx, &xpb.DecorationsRequest{
		Location: &xpb.Location{
			Ticket: fileTicket,
			Kind:   xpb.Location_SPAN,
			Span:   &cpb.Span{Start: point, End: point},
		},
		SpanKind:    xpb.DecorationsRequest_AROUND_SPAN,
		References:  true,
		SourceText:  true,
		DirtyBuffer: dirtyBuffer,
		Filter: []string{
			facts.NodeKind,
			facts.Subkind,
		},
	})
	if err != nil {
		log.Fatal(err)
	}
	nodes := graph.NodesMap(decor.Nodes)

	en := json.NewEncoder(os.Stdout)
	for _, ref := range decor.Reference {
		start, end := int(ref.Span.Start.ByteOffset), int(ref.Span.End.ByteOffset)

		var r reference
		r.Span.Start = start
		r.Span.End = end
		if len(dirtyBuffer) > 0 {
			r.Span.Text = string(dirtyBuffer[start:end])
		} // TODO(schroederc): add option to get anchor text from DecorationsReply
		r.Kind = strings.TrimPrefix(ref.Kind, edges.Prefix)
		r.Node.Ticket = ref.TargetTicket

		node := nodes[ref.TargetTicket]
		r.Node.Kind = string(node[facts.NodeKind])
		r.Node.Subkind = string(node[facts.Subkind])

		// TODO(schroederc): use CrossReferences method
		if eReply, err := graph.AllEdges(ctx, gs, &gpb.EdgesRequest{
			Ticket: []string{ref.TargetTicket},
			Kind:   []string{edges.Named, edges.Typed, definedAtEdge, definedBindingAtEdge},
		}); err != nil {
			log.Warningf("error getting edges for %q: %v", ref.TargetTicket, err)
		} else {
			matching := graph.EdgesMap(eReply.EdgeSets)[ref.TargetTicket]
			for name := range matching[edges.Named] {
				if uri, err := kytheuri.Parse(name); err != nil {
					log.Warningf("named node ticket (%q) could not be parsed: %v", name, err)
				} else {
					r.Node.Names = append(r.Node.Names, uri.Signature)
				}
			}

			for typed := range matching[edges.Typed] {
				r.Node.Typed = typed
				break
			}

			if !*skipDefinitions {
				defs := matching[definedAtEdge]
				if len(defs) == 0 {
					defs = matching[definedBindingAtEdge]
				}
				for defAnchor := range defs {
					def, err := completeDefinition(defAnchor)
					if err != nil {
						log.Warningf("failed to complete definition for %q: %v", defAnchor, err)
					} else {
						r.Node.Definitions = append(r.Node.Definitions, def)
					}
				}
			}
		}

		if err := en.Encode(r); err != nil {
			log.Fatal(err)
		}
	}
}

func completeDefinition(defAnchor string) (*definition, error) {
	parentFile, err := tickets.AnchorFile(defAnchor)
	if err != nil {
		return nil, err
	}
	parent, err := kytheuri.Parse(parentFile)
	if err != nil {
		return nil, err
	}
	locReply, err := gs.Nodes(ctx, &gpb.NodesRequest{
		Ticket: []string{defAnchor},
		Filter: []string{schema.AnchorLocFilter},
	})
	if err != nil {
		return nil, err
	}
	nodes := graph.NodesMap(locReply.Nodes)
	start, end := parseAnchorSpan(nodes[defAnchor])
	return &definition{
		File:  parent.VName(),
		Start: start,
		End:   end,
	}, nil
}

func parseAnchorSpan(anchor map[string][]byte) (start int, end int) {
	start, _ = strconv.Atoi(string(anchor[facts.AnchorStart]))
	end, _ = strconv.Atoi(string(anchor[facts.AnchorEnd]))
	return
}

func readDirtyBuffer(ctx context.Context) []byte {
	if *dirtyBuffer == "" {
		return nil
	}

	f, err := vfs.Open(ctx, *dirtyBuffer)
	if err != nil {
		log.Fatalf("ERROR: could not open dirty buffer at %q: %v", *dirtyBuffer, err)
	}
	defer f.Close()
	data, err := ioutil.ReadAll(f)
	if err != nil {
		log.Fatalf("ERROR: could read dirty buffer at %q: %v", *dirtyBuffer, err)
	}
	return data
}

func findKytheRoot(dir string) string {
	for {
		if fi, err := os.Stat(filepath.Join(dir, ".kythe")); err == nil && fi.Mode().IsRegular() {
			return dir
		}
		if dir == "/" {
			break
		}
		dir = filepath.Dir(dir)
	}
	return ""
}
