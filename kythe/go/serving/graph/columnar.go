/*
 * Copyright 2018 The Kythe Authors. All rights reserved.
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

package graph

import (
	"bytes"
	"context"
	"fmt"
	"io"
	"regexp"

	"kythe.io/kythe/go/services/graph"
	"kythe.io/kythe/go/services/xrefs"
	"kythe.io/kythe/go/serving/graph/columnar"
	"kythe.io/kythe/go/storage/keyvalue"
	"kythe.io/kythe/go/storage/table"
	"kythe.io/kythe/go/util/keys"
	"kythe.io/kythe/go/util/kytheuri"
	"kythe.io/kythe/go/util/log"
	"kythe.io/kythe/go/util/schema"
	"kythe.io/kythe/go/util/schema/facts"

	"bitbucket.org/creachadair/stringset"
	"google.golang.org/protobuf/proto"

	cpb "kythe.io/kythe/proto/common_go_proto"
	gpb "kythe.io/kythe/proto/graph_go_proto"
	gspb "kythe.io/kythe/proto/graph_serving_go_proto"
	scpb "kythe.io/kythe/proto/schema_go_proto"
)

// ColumnarTableKeyMarker is stored within a Kythe columnar table to
// differentiate it from the legacy combined table format.
const ColumnarTableKeyMarker = "kythe:columnar"

// NewService returns an graph.Service backed by the given table.  The format of
// the table with be automatically detected.
func NewService(ctx context.Context, t keyvalue.DB) graph.Service {
	_, err := t.Get(ctx, []byte(ColumnarTableKeyMarker), nil)
	if err == nil {
		log.Info("WARNING: detected a experimental columnar graph table")
		return NewColumnarTable(t)
	}
	return NewCombinedTable(&table.KVProto{t})
}

// NewColumnarTable returns a table for the given columnar graph lookup table.
func NewColumnarTable(t keyvalue.DB) *ColumnarTable { return &ColumnarTable{t} }

// ColumnarTable implements an graph.Service backed by a columnar serving table.
type ColumnarTable struct{ keyvalue.DB }

// Nodes implements part of the graph.Service interface.
func (c *ColumnarTable) Nodes(ctx context.Context, req *gpb.NodesRequest) (*gpb.NodesReply, error) {
	reply := &gpb.NodesReply{Nodes: make(map[string]*cpb.NodeInfo, len(req.Ticket))}

	filters := req.Filter
	if len(filters) == 0 {
		filters = append(filters, "**")
	}
	patterns := xrefs.ConvertFilters(filters)

	for _, ticket := range req.Ticket {
		srcURI, err := kytheuri.Parse(ticket)
		if err != nil {
			return nil, err
		}

		src := srcURI.VName()
		key, err := keys.Append(columnar.EdgesKeyPrefix, src)
		if err != nil {
			return nil, err
		}

		val, err := c.DB.Get(ctx, key, &keyvalue.Options{LargeRead: true})
		if err == io.EOF {
			continue
		} else if err != nil {
			return nil, err
		}

		var idx gspb.Edges_Index
		if err := proto.Unmarshal(val, &idx); err != nil {
			return nil, fmt.Errorf("error decoding index: %v", err)
		}

		if info := filterNode(patterns, idx.Node); len(info.Facts) > 0 {
			reply.Nodes[ticket] = info
		}
	}

	if len(reply.Nodes) == 0 {
		reply.Nodes = nil
	}

	return reply, nil
}

// processTicket loads values associated with the search ticket and adds them to the reply.
func (c *ColumnarTable) processTicket(ctx context.Context, ticket string, patterns []*regexp.Regexp, allowedKinds stringset.Set, reply *gpb.EdgesReply) error {
	srcURI, err := kytheuri.Parse(ticket)
	if err != nil {
		return err
	}

	src := srcURI.VName()
	prefix, err := keys.Append(columnar.EdgesKeyPrefix, src)
	if err != nil {
		return err
	}

	it, err := c.DB.ScanPrefix(ctx, prefix, &keyvalue.Options{LargeRead: true})
	if err != nil {
		return err
	}
	defer it.Close()

	k, val, err := it.Next()
	if err == io.EOF || !bytes.Equal(k, prefix) {
		return nil
	} else if err != nil {
		return err
	}

	// Decode Edges Index
	var idx gspb.Edges_Index
	if err := proto.Unmarshal(val, &idx); err != nil {
		return fmt.Errorf("error decoding index: %v", err)
	}
	if len(patterns) > 0 {
		if info := filterNode(patterns, idx.Node); len(info.Facts) > 0 {
			reply.Nodes[ticket] = info
		}
	}

	edges := &gpb.EdgeSet{Groups: make(map[string]*gpb.EdgeSet_Group)}
	reply.EdgeSets[ticket] = edges
	targets := stringset.New()

	// Main loop to scan over each columnar kv entry.
	for {
		k, val, err := it.Next()
		if err == io.EOF {
			break
		} else if err != nil {
			return err
		}
		key := string(k[len(prefix):])

		// TODO(schroederc): only parse needed entries
		e, err := columnar.DecodeEdgesEntry(src, key, val)
		if err != nil {
			return err
		}

		switch e := e.Entry.(type) {
		case *gspb.Edges_Edge_:
			edge := e.Edge

			kind := edge.GetGenericKind()
			if kind == "" {
				kind = schema.EdgeKindString(edge.GetKytheKind())
			}
			if edge.Reverse {
				kind = "%" + kind
			}

			if len(allowedKinds) != 0 && !allowedKinds.Contains(kind) {
				continue
			}

			target := kytheuri.ToString(edge.Target)
			targets.Add(target)

			g := edges.Groups[kind]
			if g == nil {
				g = &gpb.EdgeSet_Group{}
				edges.Groups[kind] = g
			}
			g.Edge = append(g.Edge, &gpb.EdgeSet_Group_Edge{
				TargetTicket: target,
				Ordinal:      edge.Ordinal,
			})
		case *gspb.Edges_Target_:
			if len(patterns) == 0 || len(targets) == 0 {
				break
			}

			target := e.Target
			ticket := kytheuri.ToString(target.Node.Source)
			if targets.Contains(ticket) {
				if info := filterNode(patterns, target.Node); len(info.Facts) > 0 {
					reply.Nodes[ticket] = info
				}
			}
		default:
			return fmt.Errorf("unknown Edges entry: %T", e)
		}
	}

	if len(edges.Groups) == 0 {
		delete(reply.EdgeSets, ticket)
	}
	return nil
}

// Edges implements part of the graph.Service interface.
func (c *ColumnarTable) Edges(ctx context.Context, req *gpb.EdgesRequest) (*gpb.EdgesReply, error) {
	// TODO(schroederc): implement edge paging
	reply := &gpb.EdgesReply{
		EdgeSets: make(map[string]*gpb.EdgeSet, len(req.Ticket)),
		Nodes:    make(map[string]*cpb.NodeInfo),

		// TODO(schroederc): TotalEdgesByKind: make(map[string]int64),
	}
	patterns := xrefs.ConvertFilters(req.Filter)
	allowedKinds := stringset.New(req.Kind...)

	for _, ticket := range req.Ticket {
		err := c.processTicket(ctx, ticket, patterns, allowedKinds, reply)
		if err != nil {
			return nil, err
		}
	}

	if len(reply.EdgeSets) == 0 {
		reply.EdgeSets = nil
	}
	if len(reply.Nodes) == 0 {
		reply.Nodes = nil
	}

	return reply, nil
}

func filterNode(patterns []*regexp.Regexp, n *scpb.Node) *cpb.NodeInfo {
	c := &cpb.NodeInfo{Facts: make(map[string][]byte, len(n.Fact))}
	for _, f := range n.Fact {
		name := schema.GetFactName(f)
		if xrefs.MatchesAny(name, patterns) {
			c.Facts[name] = f.Value
		}
	}
	if kind := schema.GetNodeKind(n); kind != "" && xrefs.MatchesAny(facts.NodeKind, patterns) {
		c.Facts[facts.NodeKind] = []byte(kind)
	}
	if subkind := schema.GetSubkind(n); subkind != "" && xrefs.MatchesAny(facts.Subkind, patterns) {
		c.Facts[facts.Subkind] = []byte(subkind)
	}
	return c
}
