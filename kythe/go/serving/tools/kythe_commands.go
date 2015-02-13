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
	"flag"
	"fmt"
	"log"
	"os"
	"path/filepath"
	"strconv"
	"strings"

	"kythe/go/services/filetree"
	"kythe/go/services/search"
	"kythe/go/services/xrefs"
	"kythe/go/util/kytheuri"
	"kythe/go/util/schema"

	spb "kythe/proto/storage_proto"
	xpb "kythe/proto/xref_proto"

	"github.com/golang/protobuf/proto"
)

var (
	xs  xrefs.Service
	ft  filetree.Service
	idx search.Service

	// ls flags
	lsURIs bool

	// node flags
	nodeFilters string

	// node/edges shared flags
	showFileText bool

	// edges flags
	countOnly bool
	edgeKinds string
	pageToken string
	pageSize  int

	// file flags
	decorSpan string

	// search flags
	corpus    string
	root      string
	path      string
	language  string
	signature string

	cmdLS = newCommand("ls", "[--uris] [directory-uri]",
		"List a directory's contents",
		func(flag *flag.FlagSet) {
			flag.BoolVar(&lsURIs, "uris", false, "Display files/directories as Kythe URIs")
		},
		func(flag *flag.FlagSet) error {
			if len(flag.Args()) == 0 {
				cr, err := ft.CorporaRoots()
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
			dir, err := ft.Dir(corpus, root, path)
			if err != nil {
				return err
			} else if dir == nil {
				return fmt.Errorf("no such directory: %q in corpus %q (root %q)", path, corpus, root)
			}
			return displayDirectory(dir)
		})

	cmdEdges = newCommand("edges", "[--count_only] [--kinds edgeKind1,edgeKind2,...] [--page_token token] [--page_size num] <ticket>",
		"Retrieve outward edges from a node",
		func(flag *flag.FlagSet) {
			flag.BoolVar(&countOnly, "count_only", false, "Only print counts per edge kind")
			flag.StringVar(&edgeKinds, "kinds", "", "Comma-separated list of edge kinds to return (default returns all)")
			flag.StringVar(&pageToken, "page_token", "", "Edges page token")
			flag.IntVar(&pageSize, "page_size", 0, "Maximum number of edges returned (0 lets the service use a sensible default)")
		},
		func(flag *flag.FlagSet) error {
			req := &xpb.EdgesRequest{
				Ticket:    flag.Args(),
				PageToken: &pageToken,
				PageSize:  proto.Int(pageSize),
			}
			if edgeKinds != "" {
				req.Kind = strings.Split(edgeKinds, ",")
			}
			reply, err := xs.Edges(req)
			if err != nil {
				return err
			}
			if countOnly {
				return displayEdgeCounts(reply)
			}
			return displayEdges(reply)
		})

	cmdNode = newCommand("node", "[--filters factFilter1,factFilter2,...] [--show_text] <ticket>",
		"Retrieve a node's facts",
		func(flag *flag.FlagSet) {
			flag.StringVar(&nodeFilters, "filters", "", "Comma-separated list of node fact filters (default returns all)")
			flag.BoolVar(&showFileText, "show_text", false, "Show the /kythe/text fact when available")
		},
		func(flag *flag.FlagSet) error {
			req := &xpb.NodesRequest{
				Ticket: flag.Args(),
			}
			if nodeFilters != "" {
				req.Filter = strings.Split(nodeFilters, ",")
			}
			reply, err := xs.Nodes(req)
			if err != nil {
				return err
			}
			return displayNodes(reply.Node)
		})

	cmdSource = newCommand("source", "[--span b1-b2] <file-ticket>",
		"Retrieve a file's source text",
		func(flag *flag.FlagSet) {
			// TODO handle line/column spans
			flag.StringVar(&decorSpan, "span", "", "Limit to this byte-offset span (b1-b2)")
		},
		func(flag *flag.FlagSet) error {
			req := &xpb.DecorationsRequest{
				Location: &xpb.Location{
					Ticket: proto.String(flag.Arg(0)),
				},
				SourceText: proto.Bool(true),
			}
			if decorSpan != "" {
				parts := strings.Split(decorSpan, "-")
				if len(parts) != 2 {
					return fmt.Errorf("invalid --span %q", decorSpan)
				}
				start, err := strconv.Atoi(parts[0])
				if err != nil {
					return fmt.Errorf("invalid --span start %q: %v", decorSpan, err)
				}
				end, err := strconv.Atoi(parts[1])
				if err != nil {
					return fmt.Errorf("invalid --span end %q: %v", decorSpan, err)
				}

				req.Location.Kind = xpb.Location_SPAN.Enum()
				req.Location.Start = &xpb.Location_Point{ByteOffset: proto.Int(start)}
				req.Location.End = &xpb.Location_Point{ByteOffset: proto.Int(end)}
			}

			reply, err := xs.Decorations(req)
			if err != nil {
				return err
			}
			return displaySource(reply)
		})

	cmdRefs = newCommand("refs", "[--span b1-b2] <file-ticket>",
		"List a file's anchor references",
		func(flag *flag.FlagSet) {
			// TODO handle line/column spans
			flag.StringVar(&decorSpan, "span", "", "Limit to this byte-offset span (b1-b2)")
		},
		func(flag *flag.FlagSet) error {
			req := &xpb.DecorationsRequest{
				Location: &xpb.Location{
					Ticket: proto.String(flag.Arg(0)),
				},
				References: proto.Bool(true),
			}
			if decorSpan != "" {
				parts := strings.Split(decorSpan, "-")
				if len(parts) != 2 {
					return fmt.Errorf("invalid --span %q", decorSpan)
				}
				start, err := strconv.Atoi(parts[0])
				if err != nil {
					return fmt.Errorf("invalid --span start %q: %v", decorSpan, err)
				}
				end, err := strconv.Atoi(parts[1])
				if err != nil {
					return fmt.Errorf("invalid --span end %q: %v", decorSpan, err)
				}

				req.Location.Kind = xpb.Location_SPAN.Enum()
				req.Location.Start = &xpb.Location_Point{ByteOffset: proto.Int(start)}
				req.Location.End = &xpb.Location_Point{ByteOffset: proto.Int(end)}
			}

			reply, err := xs.Decorations(req)
			if err != nil {
				return err
			}
			return displayReferences(reply)
		})

	cmdSearch = newCommand("search", "[--corpus c] [--sig s] [--root r] [--lang l] [--path p] [factName factValue]...",
		"Search for nodes based on partial components and fact values.",
		func(flag *flag.FlagSet) {
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

			req := &spb.SearchRequest{Partial: &spb.VName{}}
			if corpus != "" {
				req.Partial.Corpus = &corpus
			}
			if signature != "" {
				req.Partial.Signature = &signature
			}
			if root != "" {
				req.Partial.Root = &root
			}
			if path != "" {
				req.Partial.Path = &path
			}
			if language != "" {
				req.Partial.Language = &language
			}

			for i := 0; i < len(flag.Args()); i = i + 2 {
				if flag.Arg(i) == schema.FileTextFact {
					log.Printf("WARNING: Large facts such as %s are not likely to be indexed", schema.FileTextFact)
				}
				req.Fact = append(req.Fact, &spb.SearchRequest_Fact{
					Name:  proto.String(flag.Arg(i)),
					Value: []byte(flag.Arg(i + 1)),
				})
			}

			reply, err := idx.Search(req)
			if err != nil {
				return err
			}
			return displaySearch(reply)
		})
)

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
			fs.PrintDefaults()
		}
	}
	return command{fs, f}
}
