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
	"html"
	"sort"
	"strings"

	"kythe.io/kythe/go/services/graph"
	"kythe.io/kythe/go/util/log"
	"kythe.io/kythe/go/util/schema/edges"
	"kythe.io/kythe/go/util/schema/facts"

	"bitbucket.org/creachadair/stringset"

	gpb "kythe.io/kythe/proto/graph_go_proto"
)

type edgesCommand struct {
	baseKytheCommand
	dotGraph    bool
	countOnly   bool
	targetsOnly bool
	edgeKinds   string
	pageToken   string
	pageSize    int
}

func (edgesCommand) Name() string     { return "edges" }
func (edgesCommand) Synopsis() string { return "retrieve outward edges from a node" }
func (c *edgesCommand) SetFlags(flag *flag.FlagSet) {
	flag.BoolVar(&c.dotGraph, "graphviz", false, "Print resulting edges as a dot graph")
	flag.BoolVar(&c.countOnly, "count_only", false, "Only print counts per edge kind")
	flag.BoolVar(&c.targetsOnly, "targets_only", false, "Only display edge targets")
	flag.StringVar(&c.edgeKinds, "kinds", "", "Comma-separated list of edge kinds to return (default returns all)")
	flag.StringVar(&c.pageToken, "page_token", "", "Edges page token")
	flag.IntVar(&c.pageSize, "page_size", 0, "Maximum number of edges returned (0 lets the service use a sensible default)")
}
func (c edgesCommand) Run(ctx context.Context, flag *flag.FlagSet, api API) error {
	if c.countOnly && c.targetsOnly {
		return errors.New("--count_only and --targets_only are mutually exclusive")
	} else if c.countOnly && c.dotGraph {
		return errors.New("--count_only and --graphviz are mutually exclusive")
	} else if c.targetsOnly && c.dotGraph {
		return errors.New("--targets_only and --graphviz are mutually exclusive")
	}

	req := &gpb.EdgesRequest{
		Ticket:    flag.Args(),
		PageToken: c.pageToken,
		PageSize:  int32(c.pageSize),
	}
	if c.edgeKinds != "" {
		for _, kind := range strings.Split(c.edgeKinds, ",") {
			req.Kind = append(req.Kind, c.expandEdgeKind(kind))
		}
	}
	if c.dotGraph {
		req.Filter = []string{"**"}
	}
	LogRequest(req)
	reply, err := api.GraphService.Edges(ctx, req)
	if err != nil {
		return err
	}
	if reply.NextPageToken != "" {
		defer log.Infof("Next page token: %s", reply.NextPageToken)
	}
	if c.countOnly {
		return c.displayEdgeCounts(reply)
	} else if c.targetsOnly {
		return c.displayTargets(reply.EdgeSets)
	} else if c.dotGraph {
		return c.displayEdgeGraph(reply)
	}
	return c.displayEdges(reply)
}

func (c edgesCommand) displayEdges(reply *gpb.EdgesReply) error {
	if DisplayJSON {
		return PrintJSONMessage(reply)
	}

	for source, es := range reply.EdgeSets {
		if _, err := fmt.Fprintln(out, "source:", source); err != nil {
			return err
		}
		for kind, g := range es.Groups {
			hasOrdinal := edges.OrdinalKind(kind)
			for _, edge := range g.Edge {
				var ordinal string
				if hasOrdinal || edge.Ordinal != 0 {
					ordinal = fmt.Sprintf(".%d", edge.Ordinal)
				}
				if _, err := fmt.Fprintf(out, "%s%s\t%s\n", kind, ordinal, edge.TargetTicket); err != nil {
					return err
				}
			}
		}
	}
	return nil
}

func (c edgesCommand) displayTargets(edges map[string]*gpb.EdgeSet) error {
	var targets stringset.Set
	for _, es := range edges {
		for _, g := range es.Groups {
			for _, e := range g.Edge {
				targets.Add(e.TargetTicket)
			}
		}
	}

	if DisplayJSON {
		return PrintJSON(targets.Elements())
	}

	for target := range targets {
		if _, err := fmt.Fprintln(out, target); err != nil {
			return err
		}
	}
	return nil
}

func (c edgesCommand) displayEdgeGraph(reply *gpb.EdgesReply) error {
	nodes := graph.NodesMap(reply.Nodes)
	esets := make(map[string]map[string]stringset.Set)

	for source, es := range reply.EdgeSets {
		for gKind, g := range es.Groups {
			hasOrdinal := edges.OrdinalKind(gKind)
			for _, edge := range g.Edge {
				tgt := edge.TargetTicket
				src, kind := source, gKind
				if edges.IsReverse(kind) {
					src, kind, tgt = tgt, edges.Mirror(kind), src
				}
				if hasOrdinal || edge.Ordinal != 0 {
					kind = fmt.Sprintf("%s.%d", kind, edge.Ordinal)
				}
				groups, ok := esets[src]
				if !ok {
					groups = make(map[string]stringset.Set)
					esets[src] = groups
				}
				targets, ok := groups[kind]
				if ok {
					targets.Add(tgt)
				} else {
					groups[kind] = stringset.New(tgt)
				}

			}
		}
	}
	if _, err := fmt.Println("digraph kythe {"); err != nil {
		return err
	}
	for ticket, node := range nodes {
		if _, err := fmt.Printf(`	%q [label=<<table><tr><td colspan="2">%s</td></tr>`, ticket, html.EscapeString(ticket)); err != nil {
			return err
		}
		var factNames []string
		for fact := range node {
			if fact == facts.Code {
				continue
			}
			factNames = append(factNames, fact)
		}
		sort.Strings(factNames)
		for _, fact := range factNames {
			if _, err := fmt.Printf("<tr><td>%s</td><td>%s</td></tr>", html.EscapeString(fact), html.EscapeString(string(node[fact]))); err != nil {
				return err
			}
		}
		if _, err := fmt.Println("</table>> shape=plaintext];"); err != nil {
			return err
		}
	}
	if _, err := fmt.Println(); err != nil {
		return err
	}

	for src, groups := range esets {
		for kind, targets := range groups {
			for tgt := range targets {
				if _, err := fmt.Printf("\t%q -> %q [label=%q];\n", src, tgt, kind); err != nil {
					return err
				}
			}
		}
	}
	if _, err := fmt.Println("}"); err != nil {
		return err
	}
	return nil
}

func (c edgesCommand) displayEdgeCounts(edges *gpb.EdgesReply) error {
	counts := make(map[string]int)
	for _, es := range edges.EdgeSets {
		for kind, g := range es.Groups {
			counts[kind] += len(g.Edge)
		}
	}

	if DisplayJSON {
		return PrintJSON(counts)
	}

	for kind, cnt := range counts {
		if _, err := fmt.Fprintf(out, "%s\t%d\n", kind, cnt); err != nil {
			return err
		}
	}
	return nil
}

// expandEdgeKind prefixes unrooted (not starting with "/") edge kinds with the
// standard Kythe edge prefix ("/kythe/edge/").
func (c edgesCommand) expandEdgeKind(kind string) string {
	ck := edges.Canonical(kind)
	if strings.HasPrefix(ck, "/") {
		return kind
	}

	expansion := edges.Prefix + ck
	if edges.IsReverse(kind) {
		return edges.Mirror(expansion)
	}
	return expansion
}
