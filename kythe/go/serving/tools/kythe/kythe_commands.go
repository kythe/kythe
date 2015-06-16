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
	"path/filepath"
	"regexp"
	"strconv"
	"strings"

	"kythe.io/kythe/go/platform/vfs"
	"kythe.io/kythe/go/services/filetree"
	"kythe.io/kythe/go/services/search"
	"kythe.io/kythe/go/services/xrefs"
	"kythe.io/kythe/go/util/kytheuri"
	"kythe.io/kythe/go/util/schema"

	"golang.org/x/net/context"

	ftpb "kythe.io/kythe/proto/filetree_proto"
	spb "kythe.io/kythe/proto/storage_proto"
	xpb "kythe.io/kythe/proto/xref_proto"
)

var (
	ctx = context.Background()

	xs  xrefs.Service
	ft  filetree.Service
	idx search.Service

	// ls flags
	lsURIs    bool
	filesOnly bool
	dirsOnly  bool

	// node flags
	nodeFilters       string
	factSizeThreshold int

	// edges flags
	countOnly   bool
	targetsOnly bool
	edgeKinds   string
	pageToken   string
	pageSize    int

	// source/refs flags
	decorSpan string

	// refs flags
	dirtyFile string
	refFormat string

	// search flags
	suffixWildcard string
	corpus         string
	root           string
	path           string
	language       string
	signature      string

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
			path = filepath.Join("/", path)
			req := &ftpb.DirectoryRequest{
				Corpus: corpus,
				Root:   root,
				Path:   path,
			}
			logRequest(req)
			dir, err := ft.Directory(ctx, req)
			if err != nil {
				return err
			} else if dir == nil {
				return fmt.Errorf("no such directory: %q in corpus %q (root %q)", path, corpus, root)
			}

			if filesOnly {
				dir.Subdirectory = nil
			} else if dirsOnly {
				dir.File = nil
			}

			return displayDirectory(dir)
		})

	cmdEdges = newCommand("edges", "[--count_only] [--kinds edgeKind1,edgeKind2,...] [--page_token token] [--page_size num] <ticket>",
		"Retrieve outward edges from a node",
		func(flag *flag.FlagSet) {
			flag.BoolVar(&countOnly, "count_only", false, "Only print counts per edge kind")
			flag.BoolVar(&targetsOnly, "targets_only", false, "Only display edge targets")
			flag.StringVar(&edgeKinds, "kinds", "", "Comma-separated list of edge kinds to return (default returns all)")
			flag.StringVar(&pageToken, "page_token", "", "Edges page token")
			flag.IntVar(&pageSize, "page_size", 0, "Maximum number of edges returned (0 lets the service use a sensible default)")
		},
		func(flag *flag.FlagSet) error {
			req := &xpb.EdgesRequest{
				Ticket:    flag.Args(),
				PageToken: pageToken,
				PageSize:  int32(pageSize),
			}
			if edgeKinds != "" {
				req.Kind = strings.Split(edgeKinds, ",")
			}
			logRequest(req)
			reply, err := xs.Edges(ctx, req)
			if err != nil {
				return err
			}
			if countOnly {
				return displayEdgeCounts(reply)
			} else if targetsOnly {
				return displayTargets(reply.EdgeSet)
			}
			return displayEdges(reply)
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
			return displayNodes(reply.Node)
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

	cmdRefs = newCommand("refs", "[--format spec] [--dirty file] [--span span] <file-ticket>",
		"List a file's anchor references",
		func(flag *flag.FlagSet) {
			// TODO(schroederc): add option to look for dirty files based on file-ticket path and a directory root
			flag.StringVar(&dirtyFile, "dirty", "", "Send the given file as the dirty buffer for patching references")
			flag.StringVar(&refFormat, "format", "@edgeKind@\t@^line@:@^col@-@$line@:@$col@\t@nodeKind@\t@target@",
				`Format for each decoration result.
      Format Markers:
        @source@   -- ticket of anchor source node
        @target@   -- ticket of referenced target node
        @edgeKind@ -- edge kind from anchor node to its referenced target
        @nodeKind@ -- node kind of referenced target
        @subkind@  -- subkind of referenced target
        @^offset@  -- anchor source's starting byte-offset
        @^line@    -- anchor source's starting line
        @^col@     -- anchor source's starting column offset
        @$offset@  -- anchor source's ending byte-offset
        @$line@    -- anchor source's ending line
        @$col@     -- anchor source's ending column offset`)
			flag.StringVar(&decorSpan, "span", "", spanHelp)
		},
		func(flag *flag.FlagSet) error {
			req := &xpb.DecorationsRequest{
				Location: &xpb.Location{
					Ticket: flag.Arg(0),
				},
				References: true,
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
			} else {
				req.SourceText = true // TODO(schroederc): remove need for this
			}

			logRequest(req)
			reply, err := xs.Decorations(ctx, req)
			if err != nil {
				return err
			}

			if !req.SourceText {
				// We need to grab the full SourceText to normalize each anchor's
				// location, but when given a --span, we don't receive the full text and
				// we require a separate Nodes call.
				// TODO(schroederc): add Locations for each DecorationsReply_Reference

				nodesReq := &xpb.NodesRequest{
					Ticket: []string{req.Location.Ticket},
					Filter: []string{schema.TextFact, schema.TextEncodingFact},
				}
				logRequest(nodesReq)
				fileNodeReply, err := xs.Nodes(ctx, nodesReq)
				if err != nil {
					return err
				}
				for _, n := range fileNodeReply.Node {
					if n.Ticket != req.Location.Ticket {
						log.Printf("WARNING: received unrequested node: %q", n.Ticket)
						continue
					}
					for _, f := range n.Fact {
						switch f.Name {
						case schema.TextFact:
							reply.SourceText = f.Value
						case schema.TextEncodingFact:
							reply.Encoding = string(f.Value)
						}
					}
				}
			}

			return displayReferences(reply)
		})

	cmdSearch = newCommand("search", "[--corpus c] [--sig s] [--root r] [--lang l] [--path p] [factName factValue]...",
		"Search for nodes based on partial components and fact values.",
		func(flag *flag.FlagSet) {
			flag.StringVar(&suffixWildcard, "suffix_wildcard", "%", "Suffix wildcard for search values (optional)")
			flag.StringVar(&corpus, "corpus", "", "Limit results to nodes with the given corpus (optional)")
			flag.StringVar(&root, "root", "", "Limit results to nodes with the given root (optional)")
			flag.StringVar(&path, "path", "", "Limit results to nodes with the given path (optional)")
			flag.StringVar(&signature, "sig", "", "Limit results to nodes with the given signature (optional)")
			flag.StringVar(&language, "lang", "", "Limit results to nodes with the given language (optional)")
		},
		func(flag *flag.FlagSet) error {
			if len(flag.Args())%2 != 0 {
				return fmt.Errorf("given odd number of arguments (%d): %v", len(flag.Args()), flag.Args())
			}

			req := &spb.SearchRequest{
				Partial: &spb.VName{
					Corpus:    strings.TrimSuffix(corpus, suffixWildcard),
					Signature: strings.TrimSuffix(signature, suffixWildcard),
					Root:      strings.TrimSuffix(root, suffixWildcard),
					Path:      strings.TrimSuffix(path, suffixWildcard),
					Language:  strings.TrimSuffix(language, suffixWildcard),
				},
			}
			req.PartialPrefix = &spb.VNameMask{
				Corpus:    req.Partial.Corpus != corpus,
				Signature: req.Partial.Signature != signature,
				Root:      req.Partial.Root != root,
				Path:      req.Partial.Path != path,
				Language:  req.Partial.Language != language,
			}
			for i := 0; i < len(flag.Args()); i = i + 2 {
				if flag.Arg(i) == schema.TextFact {
					log.Printf("WARNING: Large facts such as %s are not likely to be indexed", schema.TextFact)
				}
				v := strings.TrimSuffix(flag.Arg(i+1), suffixWildcard)
				req.Fact = append(req.Fact, &spb.SearchRequest_Fact{
					Name:   flag.Arg(i),
					Value:  []byte(v),
					Prefix: v != flag.Arg(i+1),
				})
			}

			logRequest(req)
			reply, err := idx.Search(ctx, req)
			if err != nil {
				return err
			}
			return displaySearch(reply)
		})
)

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
