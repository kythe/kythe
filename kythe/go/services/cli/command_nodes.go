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
	"flag"
	"fmt"
	"strings"

	cpb "kythe.io/kythe/proto/common_go_proto"
	gpb "kythe.io/kythe/proto/graph_go_proto"
)

type nodesCommand struct {
	nodeFilters       string
	factSizeThreshold int
}

func (nodesCommand) Name() string     { return "nodes" }
func (nodesCommand) Synopsis() string { return "retrieve a node's facts" }
func (nodesCommand) Usage() string    { return "" }
func (c *nodesCommand) SetFlags(flag *flag.FlagSet) {
	flag.StringVar(&c.nodeFilters, "filters", "", "Comma-separated list of node fact filters (default returns all)")
	flag.IntVar(&c.factSizeThreshold, "max_fact_size", 64,
		"Maximum size of fact values to display.  Facts with byte lengths longer than this value will only have their fact names displayed.")
}
func (c nodesCommand) Run(ctx context.Context, flag *flag.FlagSet, api API) error {
	if c.factSizeThreshold < 0 {
		return fmt.Errorf("invalid --max_fact_size value (must be non-negative): %d", c.factSizeThreshold)
	}

	req := &gpb.NodesRequest{
		Ticket: flag.Args(),
	}
	if c.nodeFilters != "" {
		req.Filter = strings.Split(c.nodeFilters, ",")
	}
	LogRequest(req)
	reply, err := api.GraphService.Nodes(ctx, req)
	if err != nil {
		return err
	}
	return c.displayNodes(reply.Nodes)
}

func (c *nodesCommand) displayNodes(nodes map[string]*cpb.NodeInfo) error {
	if DisplayJSON {
		return PrintJSON(nodes)
	}

	for ticket, n := range nodes {
		if _, err := fmt.Fprintln(out, ticket); err != nil {
			return err
		}
		for name, value := range n.Facts {
			if len(value) <= c.factSizeThreshold {
				if _, err := fmt.Fprintf(out, "  %s\t%s\n", name, value); err != nil {
					return err
				}
			} else {
				if _, err := fmt.Fprintf(out, "  %s\t%s<truncated>\n", name, value[:c.factSizeThreshold]); err != nil {
					return err
				}
			}
		}
	}
	return nil
}
