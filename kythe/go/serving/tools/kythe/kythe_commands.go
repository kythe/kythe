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

package main

import (
	"errors"
	"flag"
	"fmt"
	"io/ioutil"
	"log"
	"os"
	"regexp"
	"strconv"
	"strings"

	"kythe.io/kythe/go/platform/vfs"
	"kythe.io/kythe/go/services/filetree"
	"kythe.io/kythe/go/services/xrefs"
	"kythe.io/kythe/go/util/kytheuri"
	"kythe.io/kythe/go/util/schema"

	"golang.org/x/net/context"

	ftpb "kythe.io/kythe/proto/filetree_proto"
	xpb "kythe.io/kythe/proto/xref_proto"
)

var (
	ctx = context.Background()

	xs xrefs.Service
	ft filetree.Service

	// ls flags
	lsURIs    bool
	filesOnly bool
	dirsOnly  bool

	// node flags
	nodeFilters       string
	factSizeThreshold int

	// edges flags
	dotGraph    bool
	countOnly   bool
	targetsOnly bool
	edgeKinds   string
	pageToken   string
	pageSize    int

	// docs flags

	// source/decor flags
	decorSpan string

	// decor flags
	targetDefs bool
	dirtyFile  string
	refFormat  string

	// xrefs flags
	defKind, declKind, refKind, docKind, callerKind string
	relatedNodes                                    bool

	spanHelp = `Limit results to this span (e.g. "10-30", "b1462-b1847", "3:5-3:10")
      Formats:
        b\d+-b\d+             -- Byte-offsets
        \d+(:\d+)?-\d+(:\d+)? -- Line offsets with optional column offsets`

	cmdLS = newCommand("ls", "[--uris] [directory-uri]",
		"List a directory's contents",
		func(flag *flag.FlagSet) {
			flag.BoolVar(&lsURIs, "uris", false, "Display files/directories as Kythe URIs")
			flag.BoolVar(&filesOnly, "files", false, "Display only files")
			flag.BoolVar(&dirsOnly, "dirs", false, "Display only directories")
		},
		func(flag *flag.FlagSet) error {
			if filesOnly && dirsOnly {
				return errors.New("--files and --dirs are mutually exclusive")
			}

			if len(flag.Args()) == 0 {
				req := &ftpb.CorpusRootsRequest{}
				logRequest(req)
				cr, err := ft.CorpusRoots(ctx, req)
				if err != nil {
					return err
				}
				return displayCorpusRoots(cr)
			}
			var corpus, root, path string
			switch len(flag.Args()) {
			case 1:
				uri, err := kytheuri.Parse(flag.Arg(0))
				if err != nil {
					log.Fatalf("invalid uri %q: %v", flag.Arg(0), err)
				}
				corpus = uri.Corpus
				root = uri.Root
				path = uri.Path
			default:
				flag.Usage()
				os.Exit(1)
			}
			path = filetree.CleanDirPath(path)
			req := &ftpb.DirectoryRequest{
				Corpus: corpus,
				Root:   root,
				Path:   path,
			}
			logRequest(req)
			dir, err := ft.Directory(ctx, req)
			if err != nil {
				return err
			}

			if filesOnly {
				dir.Subdirectory = nil
			} else if dirsOnly {
				dir.File = nil
			}

			return displayDirectory(dir)
		})

	cmdEdges = newCommand("edges", "[--count_only | --targets_only | --graphvizviz] [--kinds edgeKind1,edgeKind2,...] [--page_token token] [--page_size num] <ticket>",
		"Retrieve outward edges from a node",
		func(flag *flag.FlagSet) {
			flag.BoolVar(&dotGraph, "graphviz", false, "Print resulting edges as a dot graph")
			flag.BoolVar(&countOnly, "count_only", false, "Only print counts per edge kind")
			flag.BoolVar(&targetsOnly, "targets_only", false, "Only display edge targets")
			flag.StringVar(&edgeKinds, "kinds", "", "Comma-separated list of edge kinds to return (default returns all)")
			flag.StringVar(&pageToken, "page_token", "", "Edges page token")
			flag.IntVar(&pageSize, "page_size", 0, "Maximum number of edges returned (0 lets the service use a sensible default)")
		},
		func(flag *flag.FlagSet) error {
			if countOnly && targetsOnly {
				return errors.New("--count_only and --targets_only are mutually exclusive")
			} else if countOnly && dotGraph {
				return errors.New("--count_only and --graphviz are mutually exclusive")
			} else if targetsOnly && dotGraph {
				return errors.New("--targets_only and --graphviz are mutually exclusive")
			}

			req := &xpb.EdgesRequest{
				Ticket:    flag.Args(),
				PageToken: pageToken,
				PageSize:  int32(pageSize),
			}
			if edgeKinds != "" {
				for _, kind := range strings.Split(edgeKinds, ",") {
					req.Kind = append(req.Kind, expandEdgeKind(kind))
				}
			}
			if dotGraph {
				req.Filter = []string{"**"}
			}
			logRequest(req)
			reply, err := xs.Edges(ctx, req)
			if err != nil {
				return err
			}
			if reply.NextPageToken != "" {
				defer log.Printf("Next page token: %s", reply.NextPageToken)
			}
			if countOnly {
				return displayEdgeCounts(reply)
			} else if targetsOnly {
				return displayTargets(reply.EdgeSets)
			} else if dotGraph {
				return displayEdgeGraph(reply)
			}
			return displayEdges(reply)
		})

	cmdDocs = newCommand("docs", "<ticket>",
		"Retrieve documentation for the given node",
		func(flag *flag.FlagSet) {},
		func(flag *flag.FlagSet) error {
			fmt.Fprintf(os.Stderr, "Warning: The Documentation API is experimental and may be slow.")
			req := &xpb.DocumentationRequest{
				Ticket: flag.Args(),
			}
			logRequest(req)
			reply, err := xs.Documentation(ctx, req)
			if err != nil {
				return err
			}
			return displayDocumentation(reply)
		})

	cmdXRefs = newCommand("xrefs", "[--definitions kind] [--references kind] [--documentation kind] [--related_nodes] [--page_token token] [--page_size num] <ticket>",
		"Retrieve the global cross-references of the given node",
		func(flag *flag.FlagSet) {
			flag.StringVar(&defKind, "definitions", "all", "Kind of definitions to return (kinds: all, binding, full, or none)")
			flag.StringVar(&declKind, "declarations", "all", "Kind of declarations to return (kinds: all or none)")
			flag.StringVar(&refKind, "references", "noncall", "Kind of references to return (kinds: all, noncall, call, or none)")
			flag.StringVar(&docKind, "documentation", "all", "Kind of documentation to return (kinds: all or none)")
			flag.StringVar(&callerKind, "callers", "none", "Kind of callers to return (kinds: direct, overrides, or none)")
			flag.BoolVar(&relatedNodes, "related_nodes", false, "Whether to request related nodes")

			flag.StringVar(&pageToken, "page_token", "", "CrossReferences page token")
			flag.IntVar(&pageSize, "page_size", 0, "Maximum number of cross-references returned (0 lets the service use a sensible default)")
		},
		func(flag *flag.FlagSet) error {
			req := &xpb.CrossReferencesRequest{
				Ticket:    flag.Args(),
				PageToken: pageToken,
				PageSize:  int32(pageSize),
			}
			if relatedNodes {
				req.Filter = []string{schema.NodeKindFact, schema.SubkindFact}
			}
			switch defKind {
			case "all":
				req.DefinitionKind = xpb.CrossReferencesRequest_ALL_DEFINITIONS
			case "none":
				req.DefinitionKind = xpb.CrossReferencesRequest_NO_DEFINITIONS
			case "binding":
				req.DefinitionKind = xpb.CrossReferencesRequest_BINDING_DEFINITIONS
			case "full":
				req.DefinitionKind = xpb.CrossReferencesRequest_FULL_DEFINITIONS
			default:
				return fmt.Errorf("unknown definition kind: %q", defKind)
			}
			switch declKind {
			case "all":
				req.DeclarationKind = xpb.CrossReferencesRequest_ALL_DECLARATIONS
			case "none":
				req.DeclarationKind = xpb.CrossReferencesRequest_NO_DECLARATIONS
			default:
				return fmt.Errorf("unknown declaration kind: %q", declKind)
			}
			switch refKind {
			case "all":
				req.ReferenceKind = xpb.CrossReferencesRequest_ALL_REFERENCES
			case "noncall":
				req.ReferenceKind = xpb.CrossReferencesRequest_NON_CALL_REFERENCES
			case "call":
				req.ReferenceKind = xpb.CrossReferencesRequest_CALL_REFERENCES
			case "none":
				req.ReferenceKind = xpb.CrossReferencesRequest_NO_REFERENCES
			default:
				return fmt.Errorf("unknown reference kind: %q", refKind)
			}
			switch docKind {
			case "all":
				req.DocumentationKind = xpb.CrossReferencesRequest_ALL_DOCUMENTATION
			case "none":
				req.DocumentationKind = xpb.CrossReferencesRequest_NO_DOCUMENTATION
			default:
				return fmt.Errorf("unknown documentation kind: %q", docKind)
			}
			switch callerKind {
			case "direct":
				req.CallerKind = xpb.CrossReferencesRequest_DIRECT_CALLERS
			case "overrides":
				req.CallerKind = xpb.CrossReferencesRequest_OVERRIDE_CALLERS
			case "none":
				req.CallerKind = xpb.CrossReferencesRequest_NO_CALLERS
			default:
				return fmt.Errorf("unknown caller kind: %q", docKind)
			}
			logRequest(req)
			reply, err := xs.CrossReferences(ctx, req)
			if err != nil {
				return err
			}
			if reply.NextPageToken != "" {
				defer log.Printf("Next page token: %s", reply.NextPageToken)
			}
			return displayXRefs(reply)
		})

	cmdNode = newCommand("node", "[--filters factFilter1,factFilter2,...] [--max_fact_size] <ticket>",
		"Retrieve a node's facts",
		func(flag *flag.FlagSet) {
			flag.StringVar(&nodeFilters, "filters", "", "Comma-separated list of node fact filters (default returns all)")
			flag.IntVar(&factSizeThreshold, "max_fact_size", 64,
				"Maximum size of fact values to display.  Facts with byte lengths longer than this value will only have their fact names displayed.")
		},
		func(flag *flag.FlagSet) error {
			if factSizeThreshold < 0 {
				return fmt.Errorf("invalid --max_fact_size value (must be non-negative): %d", factSizeThreshold)
			}

			req := &xpb.NodesRequest{
				Ticket: flag.Args(),
			}
			if nodeFilters != "" {
				req.Filter = strings.Split(nodeFilters, ",")
			}
			logRequest(req)
			reply, err := xs.Nodes(ctx, req)
			if err != nil {
				return err
			}
			return displayNodes(reply.Nodes)
		})

	cmdSource = newCommand("source", "[--span span] <file-ticket>",
		"Retrieve a file's source text",
		func(flag *flag.FlagSet) {
			flag.StringVar(&decorSpan, "span", "", spanHelp)
		},
		func(flag *flag.FlagSet) error {
			req := &xpb.DecorationsRequest{
				Location: &xpb.Location{
					Ticket: flag.Arg(0),
				},
				SourceText: true,
			}
			if decorSpan != "" {
				start, end, err := parseSpan(decorSpan)
				if err != nil {
					return fmt.Errorf("invalid --span %q: %v", decorSpan, err)
				}

				req.Location.Kind = xpb.Location_SPAN
				req.Location.Start = start
				req.Location.End = end
			}

			logRequest(req)
			reply, err := xs.Decorations(ctx, req)
			if err != nil {
				return err
			}
			return displaySource(reply)
		})

	cmdDecor = newCommand("decor", "[--format spec] [--dirty file] [--span span] <file-ticket>",
		"List a file's anchor decorations",
		func(flag *flag.FlagSet) {
			// TODO(schroederc): add option to look for dirty files based on file-ticket path and a directory root
			flag.StringVar(&dirtyFile, "dirty", "", "Send the given file as the dirty buffer for patching references")
			flag.StringVar(&refFormat, "format", "@edgeKind@\t@^line@:@^col@-@$line@:@$col@\t@nodeKind@\t@target@",
				`Format for each decoration result.
      Format Markers:
        @source@    -- ticket of anchor source node
        @target@    -- ticket of referenced target node
        @targetDef@ -- ticket of referenced target's definition
        @edgeKind@  -- edge kind from anchor node to its referenced target
        @nodeKind@  -- node kind of referenced target
        @subkind@   -- subkind of referenced target
        @^offset@   -- anchor source's starting byte-offset
        @^line@     -- anchor source's starting line
        @^col@      -- anchor source's starting column offset
        @$offset@   -- anchor source's ending byte-offset
        @$line@     -- anchor source's ending line
        @$col@      -- anchor source's ending column offset`)
			flag.StringVar(&decorSpan, "span", "", spanHelp)
			flag.BoolVar(&targetDefs, "target_definitions", false, "Whether to request definitions (@targetDef@ format marker) for each reference's target")
		},
		func(flag *flag.FlagSet) error {
			req := &xpb.DecorationsRequest{
				Location: &xpb.Location{
					Ticket: flag.Arg(0),
				},
				References:        true,
				TargetDefinitions: targetDefs,
				Filter: []string{
					schema.NodeKindFact,
					schema.SubkindFact,
				},
			}
			if dirtyFile != "" {
				f, err := vfs.Open(ctx, dirtyFile)
				if err != nil {
					return fmt.Errorf("error opening dirty buffer file at %q: %v", dirtyFile, err)
				}
				buf, err := ioutil.ReadAll(f)
				if err != nil {
					f.Close()
					return fmt.Errorf("error reading dirty buffer file: %v", err)
				} else if err := f.Close(); err != nil {
					return fmt.Errorf("error closing dirty buffer file: %v", err)
				}
				req.DirtyBuffer = buf
			}
			if decorSpan != "" {
				start, end, err := parseSpan(decorSpan)
				if err != nil {
					return fmt.Errorf("invalid --span %q: %v", decorSpan, err)
				}

				req.Location.Kind = xpb.Location_SPAN
				req.Location.Start = start
				req.Location.End = end
			}

			logRequest(req)
			reply, err := xs.Decorations(ctx, req)
			if err != nil {
				return err
			}

			return displayDecorations(reply)
		})
)

// expandEdgeKind prefixes unrooted (not starting with "/") edge kinds with the
// standard Kythe edge prefix ("/kythe/edge/").
func expandEdgeKind(kind string) string {
	ck := schema.Canonicalize(kind)
	if strings.HasPrefix(ck, "/") {
		return kind
	}

	expansion := schema.EdgePrefix + ck
	if schema.EdgeDirection(kind) == schema.Reverse {
		return schema.MirrorEdge(expansion)
	}
	return expansion
}

var (
	byteOffsetPointRE = regexp.MustCompile(`^b(\d+)$`)
	lineNumberPointRE = regexp.MustCompile(`^(\d+)(:(\d+))?$`)
)

func parsePoint(p string) (*xpb.Location_Point, error) {
	if m := byteOffsetPointRE.FindStringSubmatch(p); m != nil {
		offset, err := strconv.Atoi(m[1])
		if err != nil {
			return nil, fmt.Errorf("invalid byte-offset: %v", err)
		}
		return &xpb.Location_Point{ByteOffset: int32(offset)}, nil
	} else if m := lineNumberPointRE.FindStringSubmatch(p); m != nil {
		line, err := strconv.Atoi(m[1])
		if err != nil {
			return nil, fmt.Errorf("invalid line number: %v", err)
		}
		np := &xpb.Location_Point{LineNumber: int32(line)}
		if m[3] != "" {
			col, err := strconv.Atoi(m[3])
			if err != nil {
				return nil, fmt.Errorf("invalid column offset: %v", err)
			}
			np.ColumnOffset = int32(col)
		}
		return np, nil
	}
	return nil, fmt.Errorf("unknown format %q", p)
}

func parseSpan(span string) (start, end *xpb.Location_Point, err error) {
	parts := strings.Split(span, "-")
	if len(parts) != 2 {
		return nil, nil, errors.New("unknown format")
	}
	start, err = parsePoint(parts[0])
	if err != nil {
		return nil, nil, fmt.Errorf("invalid start: %v", err)
	}
	end, err = parsePoint(parts[1])
	if err != nil {
		return nil, nil, fmt.Errorf("invalid end: %v", err)
	}
	return
}

// command specifies a named sub-command for the kythe tool with its own flags.
type command struct {
	*flag.FlagSet
	f func(*flag.FlagSet) error
}

// run executes c with the leftover arguments after the sub-command name
// parsed by c's FlagSet.
func (c command) run() error {
	c.FlagSet.Parse(flag.Args()[1:])
	return c.f(c.FlagSet)
}

// newCommand constructs a named sub-command.  init will be called on the new
// command's FlagSet during construction, and if one was not set during init, a
// generic Usage function will be set to the FlagSet using the given argsSpec
// and command description strings.  f defines the behavior of the command and
// will be given the same FlagSet as init.
func newCommand(name, argsSpec, description string, init func(*flag.FlagSet), f func(*flag.FlagSet) error) command {
	fs := flag.NewFlagSet(name, flag.ExitOnError)
	init(fs)
	if fs.Usage == nil {
		fs.Usage = func() {
			fmt.Fprintln(os.Stderr, "#", description)
			fmt.Fprintln(os.Stderr, name, argsSpec)
			if !shortHelp {
				fs.PrintDefaults()
			}
		}
	}
	return command{fs, f}
}
