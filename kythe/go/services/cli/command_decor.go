/*
 * Copyright 2017 Google Inc. All rights reserved.
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
	"flag"
	"fmt"
	"io/ioutil"
	"strconv"
	"strings"

	"kythe.io/kythe/go/platform/vfs"
	"kythe.io/kythe/go/services/xrefs"
	"kythe.io/kythe/go/util/schema/facts"

	xpb "kythe.io/kythe/proto/xref_proto"
)

type decorCommand struct {
	decorSpan        string
	targetDefs       bool
	dirtyFile        string
	refFormat        string
	extendsOverrides bool
}

func (decorCommand) Name() string     { return "decor" }
func (decorCommand) Synopsis() string { return "list a file's decorations" }
func (decorCommand) Usage() string    { return "" }
func (c *decorCommand) SetFlags(flag *flag.FlagSet) {
	// TODO(schroederc): add option to look for dirty files based on file-ticket path and a directory root
	flag.StringVar(&c.dirtyFile, "dirty", "", "Send the given file as the dirty buffer for patching references")
	flag.StringVar(&c.refFormat, "format", "@edgeKind@\t@^line@:@^col@-@$line@:@$col@\t@nodeKind@\t@target@",
		`Format for each decoration result.
      Format Markers:
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
	flag.StringVar(&c.decorSpan, "span", "", spanHelp)
	flag.BoolVar(&c.targetDefs, "target_definitions", false, "Whether to request definitions (@targetDef@ format marker) for each reference's target")
	flag.BoolVar(&c.extendsOverrides, "extends_overrides", false, "Whether to request extends/overrides information")
}
func (c *decorCommand) Run(ctx context.Context, flag *flag.FlagSet, api API) error {
	// TODO(schroederc): construct ticket using default corpus if just given path
	req := &xpb.DecorationsRequest{
		Location: &xpb.Location{
			Ticket: flag.Arg(0),
		},
		References:        true,
		TargetDefinitions: c.targetDefs,
		ExtendsOverrides:  c.extendsOverrides,
		Filter: []string{
			facts.NodeKind,
			facts.Subkind,
		},
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
	if c.decorSpan != "" {
		span, err := parseSpan(c.decorSpan)
		if err != nil {
			return fmt.Errorf("invalid --span %q: %v", c.decorSpan, err)
		}

		req.Location.Kind = xpb.Location_SPAN
		req.Location.Span = span
	}

	logRequest(req)
	reply, err := api.XRefService.Decorations(ctx, req)
	if err != nil {
		return err
	}

	return c.displayDecorations(reply)
}

func (c *decorCommand) displayDecorations(decor *xpb.DecorationsReply) error {
	if *displayJSON {
		return jsonMarshaler.Marshal(out, decor)
	}

	nodes := xrefs.NodesMap(decor.Nodes)

	for _, ref := range decor.Reference {
		nodeKind := factValue(nodes, ref.TargetTicket, facts.NodeKind, "UNKNOWN")
		subkind := factValue(nodes, ref.TargetTicket, facts.Subkind, "")

		var targetDef string
		if ref.TargetDefinition != "" {
			targetDef = ref.TargetDefinition
			// TODO(schroederc): fields from decor.DefinitionLocations
			// TODO(zarko): fields from decor.ExtendsOverrides
		}

		r := strings.NewReplacer(
			"@target@", ref.TargetTicket,
			"@edgeKind@", ref.Kind,
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
