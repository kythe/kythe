/*
 * Copyright 2017 The Kythe Authors. All rights reserved.
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

package cli

import (
	"context"
	"errors"
	"flag"
	"fmt"
	"io/ioutil"
	"strconv"
	"strings"

	"kythe.io/kythe/go/platform/vfs"
	"kythe.io/kythe/go/services/graph"
	"kythe.io/kythe/go/util/flagutil"
	"kythe.io/kythe/go/util/kytheuri"
	"kythe.io/kythe/go/util/schema/facts"

	cpb "kythe.io/kythe/proto/common_go_proto"
	xpb "kythe.io/kythe/proto/xref_go_proto"
)

var (
	// DefaultFileCorpus is the --corpus flag used by source/decor/diagnostics
	// commands to construct a file ticket from a raw file path.
	DefaultFileCorpus string

	// DefaultFileRoot is the --root flag used by source/decor/diagnostics commands
	// to construct a file ticket from a raw file path.
	DefaultFileRoot string

	// DefaultFilePathPrefix is the --path_prefix flag used by
	// source/decor/diagnostics commands to construct a file ticket from a raw file
	// path.
	DefaultFilePathPrefix string
)

// baseDecorCommand is a shared base for the source/decor/diagnostics commands
type baseDecorCommand struct {
	baseKytheCommand
	decorSpan                string
	corpus, root, pathPrefix string
	buildConfigs             flagutil.StringSet

	workspaceURI string
}

func (c *baseDecorCommand) SetFlags(flag *flag.FlagSet) {
	flag.StringVar(&c.decorSpan, "span", "",
		`Limit results to this span (e.g. "10-30", "b1462-b1847", "3:5-3:10", "10")
      Formats:
        b\d+-b\d+             -- Byte-offsets
        \d+(:\d+)?-\d+(:\d+)? -- Line offsets with optional column offsets
        \d+(:\d+)?            -- Full line span (with an optional starting column offset)`)
	flag.StringVar(&c.workspaceURI, "workspace_uri", "", "Workspace URI to patch file decorations")
	flag.StringVar(&c.corpus, "corpus", DefaultFileCorpus, "File corpus to use if given a raw path")
	flag.StringVar(&c.root, "root", DefaultFileRoot, "File root to use if given a raw path")
	flag.StringVar(&c.pathPrefix, "path_prefix", DefaultFilePathPrefix, "File path prefix to use if given a raw path (this is prepended directly to the raw path without any joining slashes)")
	flag.Var(&c.buildConfigs, "build_config", "CSV set of build configs with which to filter file decorations")
}

func (c baseDecorCommand) fileTicketArg(flag *flag.FlagSet) (string, error) {
	if flag.NArg() < 0 {
		return "", errors.New("no file given")
	}
	file := flag.Arg(0)
	if strings.HasPrefix(file, kytheuri.Scheme) {
		return file, nil
	}
	return (&kytheuri.URI{
		Corpus: c.corpus,
		Root:   c.root,
		Path:   c.pathPrefix + file,
	}).String(), nil
}

func (c baseDecorCommand) baseRequest(flag *flag.FlagSet) (*xpb.DecorationsRequest, error) {
	ticket, err := c.fileTicketArg(flag)
	if err != nil {
		return nil, err
	}
	req := &xpb.DecorationsRequest{
		Location: &xpb.Location{Ticket: ticket},
	}
	span, err := c.spanArg()
	if err != nil {
		return nil, err
	} else if span != nil {
		req.Location.Kind = xpb.Location_SPAN
		req.Location.Span = span
	}
	req.BuildConfig = c.buildConfigs.Elements()
	if c.workspaceURI != "" {
		req.Workspace = &xpb.Workspace{Uri: c.workspaceURI}
		req.PatchAgainstWorkspace = true
	}
	return req, nil
}

func (c baseDecorCommand) spanArg() (*cpb.Span, error) {
	if c.decorSpan == "" {
		return nil, nil
	}
	span, err := parseSpan(c.decorSpan)
	if err != nil {
		return nil, fmt.Errorf("invalid --span %q: %v", c.decorSpan, err)
	}
	return span, nil
}

type decorCommand struct {
	baseDecorCommand
	targetDefs       bool
	dirtyFile        string
	refFormat        string
	extendsOverrides bool
	semanticScopes   bool
}

func (decorCommand) Name() string      { return "decor" }
func (decorCommand) Synopsis() string  { return "list a file's decorations" }
func (decorCommand) Aliases() []string { return []string{"decors"} }
func (c *decorCommand) SetFlags(flag *flag.FlagSet) {
	c.baseDecorCommand.SetFlags(flag)
	// TODO(schroederc): add option to look for dirty files based on file-ticket path and a directory root
	flag.StringVar(&c.dirtyFile, "dirty", "", "Send the given file as the dirty buffer for patching references")
	flag.StringVar(&c.refFormat, "format", "@edgeKind@\t@^line@:@^col@-@$line@:@$col@\t@targetKind@\t@target@\t@targetDef@",
		`Format for each decoration result.
      Format Markers:
        @target@     -- ticket of referenced target node
        @targetDef@  -- ticket of referenced target's definition
        @edgeKind@   -- edge kind from anchor node to its referenced target
        @targetKind@ -- node kind and subkind of referenced target
        @nodeKind@   -- node kind of referenced target
        @subkind@    -- subkind of referenced target
        @^offset@    -- anchor source's starting byte-offset
        @^line@      -- anchor source's starting line
        @^col@       -- anchor source's starting column offset
        @$offset@    -- anchor source's ending byte-offset
        @$line@      -- anchor source's ending line
        @$col@       -- anchor source's ending column offset`)
	flag.BoolVar(&c.targetDefs, "target_definitions", false, "Whether to request definitions (@targetDef@ format marker) for each reference's target")
	flag.BoolVar(&c.extendsOverrides, "extends_overrides", false, "Whether to request extends/overrides information")
	flag.BoolVar(&c.semanticScopes, "semantic_scopes", false, "Whether to request semantic scope information")
}
func (c decorCommand) Run(ctx context.Context, flag *flag.FlagSet, api API) error {
	req, err := c.baseRequest(flag)
	if err != nil {
		return err
	}
	req.References = true
	req.TargetDefinitions = c.targetDefs
	req.ExtendsOverrides = c.extendsOverrides
	req.SemanticScopes = c.semanticScopes
	req.Filter = []string{
		facts.NodeKind,
		facts.Subkind,
	}
	if c.dirtyFile != "" {
		f, err := vfs.Open(ctx, c.dirtyFile)
		if err != nil {
			return fmt.Errorf("error opening dirty buffer file at %q: %v", c.dirtyFile, err)
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

	LogRequest(req)
	reply, err := api.XRefService.Decorations(ctx, req)
	if err != nil {
		return err
	}

	return c.displayDecorations(reply)
}

func (c decorCommand) displayDecorations(decor *xpb.DecorationsReply) error {
	if DisplayJSON {
		return PrintJSONMessage(decor)
	}

	nodes := graph.NodesMap(decor.Nodes)

	for _, ref := range decor.Reference {
		nodeKind := factValue(nodes, ref.TargetTicket, facts.NodeKind, "UNKNOWN")
		subkind := factValue(nodes, ref.TargetTicket, facts.Subkind, "")

		tgtKind := nodeKind
		if subkind != "" {
			tgtKind += "/" + subkind
		}

		var targetDef string
		if ref.TargetDefinition != "" {
			targetDef = ref.TargetDefinition
			// TODO(schroederc): fields from decor.DefinitionLocations
			// TODO(zarko): fields from decor.ExtendsOverrides
		}

		r := strings.NewReplacer(
			"@target@", ref.TargetTicket,
			"@edgeKind@", ref.Kind,
			"@targetKind@", tgtKind,
			"@nodeKind@", nodeKind,
			"@subkind@", subkind,
			"@^offset@", itoa(ref.Span.Start.ByteOffset),
			"@^line@", itoa(ref.Span.Start.LineNumber),
			"@^col@", itoa(ref.Span.Start.ColumnOffset),
			"@$offset@", itoa(ref.Span.End.ByteOffset),
			"@$line@", itoa(ref.Span.End.LineNumber),
			"@$col@", itoa(ref.Span.End.ColumnOffset),
			"@targetDef@", targetDef,
		)
		if _, err := r.WriteString(out, c.refFormat+"\n"); err != nil {
			return err
		}
	}

	return nil
}

func itoa(n int32) string { return strconv.Itoa(int(n)) }

func factValue(m map[string]map[string][]byte, ticket, factName, def string) string {
	if n, ok := m[ticket]; ok {
		if val, ok := n[factName]; ok {
			return string(val)
		}
	}
	return def
}
