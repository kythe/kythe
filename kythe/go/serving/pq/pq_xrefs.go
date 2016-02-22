/*
 * Copyright 2016 Google Inc. All rights reserved.
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

package pq

import (
	"database/sql"
	"encoding/base64"
	"fmt"
	"reflect"
	"sort"
	"strconv"
	"strings"

	"kythe.io/kythe/go/services/xrefs"
	"kythe.io/kythe/go/util/kytheuri"
	"kythe.io/kythe/go/util/schema"
	"kythe.io/kythe/go/util/stringset"

	"github.com/golang/protobuf/proto"
	"golang.org/x/net/context"

	cpb "kythe.io/kythe/proto/common_proto"
	srvpb "kythe.io/kythe/proto/serving_proto"
	xpb "kythe.io/kythe/proto/xref_proto"
)

// TODO(schroederc): share base logic with LevelDB implementation

// Nodes implements part of the xrefs.Interface.
func (d *DB) Nodes(ctx context.Context, req *xpb.NodesRequest) (*xpb.NodesReply, error) {
	tickets, err := xrefs.FixTickets(req.Ticket)
	if err != nil {
		return nil, err
	}

	setQ, args := sqlSetQuery(1, tickets)
	rs, err := d.Query(fmt.Sprintf("SELECT * FROM Nodes WHERE ticket IN %s;", setQ), args...)
	if err != nil {
		return nil, fmt.Errorf("error querying for nodes: %v", err)
	}

	var reply xpb.NodesReply
	for rs.Next() {
		var ticket, nodeKind string
		var subkind, textEncoding sql.NullString
		var text, otherFacts []byte
		var startOffset, endOffset, snippetStart, snippetEnd sql.NullInt64
		var otherFactsNum int
		if err := rs.Scan(&ticket, &nodeKind, &subkind, &text, &textEncoding, &startOffset, &endOffset, &snippetStart, &snippetEnd, &otherFactsNum, &otherFacts); err != nil {
			return nil, fmt.Errorf("error scanning nodes: %v", err)
		}

		n := new(xpb.NodeInfo)
		if otherFactsNum > 0 {
			if err := proto.Unmarshal(otherFacts, n); err != nil {
				return nil, fmt.Errorf("unexpected node internal format: %v", err)
			}
		}
		n.Ticket = ticket

		if nodeKind != "" {
			n.Fact = append(n.Fact, &cpb.Fact{
				Name:  schema.NodeKindFact,
				Value: []byte(nodeKind),
			})
		}
		if subkind.Valid {
			n.Fact = append(n.Fact, &cpb.Fact{
				Name:  schema.SubkindFact,
				Value: []byte(subkind.String),
			})
		}
		if text != nil { // TODO(schroederc): NULL text
			n.Fact = append(n.Fact, &cpb.Fact{
				Name:  schema.TextFact,
				Value: text,
			})
		}
		if textEncoding.Valid {
			n.Fact = append(n.Fact, &cpb.Fact{
				Name:  schema.TextEncodingFact,
				Value: []byte(textEncoding.String),
			})
		}
		if startOffset.Valid {
			n.Fact = append(n.Fact, &cpb.Fact{
				Name:  schema.AnchorStartFact,
				Value: []byte(strconv.FormatInt(startOffset.Int64, 10)),
			})
		}
		if endOffset.Valid {
			n.Fact = append(n.Fact, &cpb.Fact{
				Name:  schema.AnchorEndFact,
				Value: []byte(strconv.FormatInt(endOffset.Int64, 10)),
			})
		}
		if snippetStart.Valid {
			n.Fact = append(n.Fact, &cpb.Fact{
				Name:  schema.SnippetStartFact,
				Value: []byte(strconv.FormatInt(snippetStart.Int64, 10)),
			})
		}
		if snippetEnd.Valid {
			n.Fact = append(n.Fact, &cpb.Fact{
				Name:  schema.SnippetEndFact,
				Value: []byte(strconv.FormatInt(snippetEnd.Int64, 10)),
			})
		}

		if len(req.Filter) > 0 {
			patterns := xrefs.ConvertFilters(req.Filter)
			matched := make([]*cpb.Fact, 0, len(n.Fact))
			for _, f := range n.Fact {
				if xrefs.MatchesAny(f.Name, patterns) {
					matched = append(matched, f)
				}
			}
			n.Fact = matched
		}

		if len(n.Fact) > 0 {
			sort.Sort(xrefs.ByName(n.Fact))
			reply.Node = append(reply.Node, n)
		}
	}

	return &reply, nil
}

// Edges implements part of the xrefs.Interface.
func (d *DB) Edges(ctx context.Context, req *xpb.EdgesRequest) (*xpb.EdgesReply, error) {
	tickets, err := xrefs.FixTickets(req.Ticket)
	if err != nil {
		return nil, err
	}

	// TODO(schroederc): filter by Kind

	pageSize := int(req.PageSize)
	if pageSize <= 0 {
		pageSize = defaultPageSize
	} else if pageSize > maxPageSize {
		pageSize = maxPageSize
	}

	var pageOffset int
	if req.PageToken != "" {
		rec, err := base64.StdEncoding.DecodeString(req.PageToken)
		if err != nil {
			return nil, fmt.Errorf("invalid page_token: %q", req.PageToken)
		}
		var t srvpb.PageToken
		if err := proto.Unmarshal(rec, &t); err != nil || t.Index < 0 {
			return nil, fmt.Errorf("invalid page_token: %q", req.PageToken)
		}
		pageOffset = int(t.Index)
	}

	setQ, args := sqlSetQuery(1, tickets)
	args = append(args, pageSize+1, pageOffset)
	rs, err := d.Query(fmt.Sprintf(`
SELECT * FROM AllEdges
WHERE source IN %s
LIMIT $%d
OFFSET $%d;`, setQ, len(tickets)+1, len(tickets)+2), args...)
	if err != nil {
		return nil, fmt.Errorf("error querying for edges: %v", err)
	}

	edges := make(map[string]map[string][]string, len(tickets))
	for i := 0; i < pageSize && rs.Next(); i++ {
		var source, kind, target string
		if err := rs.Scan(&source, &kind, &target); err != nil {
			return nil, fmt.Errorf("edges scan error: %v", err)
		}

		groups, ok := edges[source]
		if !ok {
			groups = make(map[string][]string)
			edges[source] = groups
		}
		groups[kind] = append(groups[kind], target)
	}

	reply := &xpb.EdgesReply{EdgeSet: make([]*xpb.EdgeSet, 0, len(edges))}
	nodeTickets := stringset.New()
	for src, groups := range edges {
		gs := make([]*xpb.EdgeSet_Group, 0, len(groups))
		nodeTickets.Add(src)
		for kind, targets := range groups {
			gs = append(gs, &xpb.EdgeSet_Group{
				Kind:         kind,
				TargetTicket: targets,
			})
			nodeTickets.Add(targets...)
		}
		reply.EdgeSet = append(reply.EdgeSet, &xpb.EdgeSet{
			SourceTicket: src,
			Group:        gs,
		})
	}

	if rs.Next() {
		rec, err := proto.Marshal(&srvpb.PageToken{Index: int32(pageOffset + pageSize)})
		if err != nil {
			return nil, fmt.Errorf("internal error: error marshalling page token: %v", err)
		}
		reply.NextPageToken = base64.StdEncoding.EncodeToString(rec)
	}

	// TODO(schroederc): faster node lookups
	if len(req.Filter) > 0 && len(nodeTickets) > 0 {
		nodes, err := d.Nodes(ctx, &xpb.NodesRequest{
			Ticket: nodeTickets.Slice(),
			Filter: req.Filter,
		})
		if err != nil {
			return nil, fmt.Errorf("error filtering nodes:%v", err)
		}
		reply.Node = nodes.Node
	}

	return reply, nil
}

// Callers implements part of the xrefs.Interface.
func (d *DB) Callers(ctx context.Context, req *xpb.CallersRequest) (*xpb.CallersReply, error) {
	return xrefs.SlowCallers(ctx, d, req)
}

// Decorations implements part of the xrefs.Interface.
func (d *DB) Decorations(ctx context.Context, req *xpb.DecorationsRequest) (*xpb.DecorationsReply, error) {
	if d.selectText == nil {
		var err error
		d.selectText, err = d.Prepare("SELECT text, text_encoding FROM Nodes WHERE ticket = $1;")
		if err != nil {
			return nil, fmt.Errorf("error preparing selectText statement: %v", err)
		}
	}

	// TODO(schroederc): dirty buffers
	// TODO(schroederc): span locations

	fileTicket, err := kytheuri.Fix(req.Location.Ticket)
	if err != nil {
		return nil, fmt.Errorf("invalid location ticket: %v", err)
	}
	req.Location.Ticket = fileTicket

	decor := &xpb.DecorationsReply{Location: req.Location}

	r := d.selectText.QueryRow(fileTicket)
	var text []byte
	var textEncoding sql.NullString
	if err := r.Scan(&text, &textEncoding); err != nil {
		return nil, err
	}
	norm := xrefs.NewNormalizer(text)

	if req.SourceText {
		decor.SourceText = text
		decor.Encoding = textEncoding.String
	}

	if req.References {
		var err error
		decor.Reference, err = d.scanReferences(fileTicket, norm)
		if err != nil {
			return nil, err
		}

		if len(req.Filter) > 0 && len(decor.Reference) > 0 {
			nodeTickets := stringset.New()
			for _, r := range decor.Reference {
				nodeTickets.Add(r.TargetTicket)
			}

			nodes, err := d.Nodes(ctx, &xpb.NodesRequest{
				Ticket: nodeTickets.Slice(),
				Filter: req.Filter,
			})
			if err != nil {
				return nil, fmt.Errorf("error filtering nodes:%v", err)
			}
			decor.Node = nodes.Node
		}
	}

	return decor, nil
}

const (
	defaultPageSize = 2048
	maxPageSize     = 10000
)

// CrossReferences implements part of the xrefs.Interface.
func (d *DB) CrossReferences(ctx context.Context, req *xpb.CrossReferencesRequest) (*xpb.CrossReferencesReply, error) {
	tickets, err := xrefs.FixTickets(req.Ticket)
	if err != nil {
		return nil, err
	}

	pageSize := int(req.PageSize)
	if pageSize <= 0 {
		pageSize = defaultPageSize
	} else if pageSize > maxPageSize {
		pageSize = maxPageSize
	}

	var pageOffset, edgesOffset int
	if req.PageToken != "" {
		rec, err := base64.StdEncoding.DecodeString(req.PageToken)
		if err != nil {
			return nil, fmt.Errorf("invalid page_token: %q", req.PageToken)
		}
		var t srvpb.PageToken
		if err := proto.Unmarshal(rec, &t); err != nil || t.Index < 0 {
			return nil, fmt.Errorf("invalid page_token: %q", req.PageToken)
		}
		pageOffset = int(t.Index)
		edgesOffset = int(t.SecondaryIndex)
	}

	reply := &xpb.CrossReferencesReply{
		CrossReferences: make(map[string]*xpb.CrossReferencesReply_CrossReferenceSet),
		Nodes:           make(map[string]*xpb.NodeInfo),
	}

	setQ, ticketArgs := sqlSetQuery(1, tickets)

	var count int
	if edgesOffset == 0 {
		args := append(ticketArgs, pageSize+1, pageOffset) // +1 to check for next page

		rs, err := d.Query(fmt.Sprintf("SELECT ticket, kind, proto FROM CrossReferences WHERE ticket IN %s ORDER BY ticket LIMIT $%d OFFSET $%d;", setQ, len(tickets)+1, len(tickets)+2), args...)
		if err != nil {
			return nil, err
		}

		var xrs *xpb.CrossReferencesReply_CrossReferenceSet
		for rs.Next() {
			count++
			if count > pageSize {
				continue
			}

			var ticket, kind string
			var rec []byte
			if err := rs.Scan(&ticket, &kind, &rec); err != nil {
				return nil, err
			}
			if xrs != nil && xrs.Ticket != ticket {
				if len(xrs.Definition) > 0 || len(xrs.Documentation) > 0 || len(xrs.Reference) > 0 || len(xrs.RelatedNode) > 0 {
					reply.CrossReferences[xrs.Ticket] = xrs
				}
				xrs = nil
			}
			if xrs == nil {
				xrs = &xpb.CrossReferencesReply_CrossReferenceSet{Ticket: ticket}
			}
			switch {
			case xrefs.IsDefKind(req.DefinitionKind, kind):
				xrs.Definition, err = addAnchor(xrs.Definition, rec, req.AnchorText)
				if err != nil {
					return nil, err
				}
			case xrefs.IsDocKind(req.DocumentationKind, kind):
				xrs.Documentation, err = addAnchor(xrs.Documentation, rec, req.AnchorText)
				if err != nil {
					return nil, err
				}
			case xrefs.IsRefKind(req.ReferenceKind, kind):
				xrs.Reference, err = addAnchor(xrs.Reference, rec, req.AnchorText)
				if err != nil {
					return nil, err
				}
			}
		}
		if xrs != nil && (len(xrs.Definition) > 0 || len(xrs.Documentation) > 0 || len(xrs.Reference) > 0 || len(xrs.RelatedNode) > 0) {
			reply.CrossReferences[xrs.Ticket] = xrs
		}

		if count > pageSize {
			rec, err := proto.Marshal(&srvpb.PageToken{Index: int32(pageOffset + pageSize)})
			if err != nil {
				return nil, fmt.Errorf("internal error: error marshalling page token: %v", err)
			}
			reply.NextPageToken = base64.StdEncoding.EncodeToString(rec)
		}
	}

	if count <= pageSize {
		requestedEdges := pageSize - count
		args := append(ticketArgs, requestedEdges+1, edgesOffset) // +1 to check for next page

		// TODO(schroederc): reuse db.Edges (get reverses); filter anchor edges
		rs, err := d.Query(fmt.Sprintf("SELECT source, kind, target FROM Edges WHERE source IN %s ORDER BY source LIMIT $%d OFFSET $%d", setQ, len(tickets)+1, len(tickets)+2), args...)
		if err != nil {
			return nil, fmt.Errorf("error getting related nodes: %v", err)
		}

		nodeTickets := stringset.New()
		for i := 0; i < requestedEdges && rs.Next(); i++ {
			var source string
			var node xpb.CrossReferencesReply_RelatedNode
			if err := rs.Scan(&source, &node.RelationKind, &node.Ticket); err != nil {
				return nil, err
			}
			xrs, ok := reply.CrossReferences[source]
			if !ok {
				xrs = &xpb.CrossReferencesReply_CrossReferenceSet{Ticket: source}
				reply.CrossReferences[xrs.Ticket] = xrs
			}

			xrs.RelatedNode = append(xrs.RelatedNode, &node)
			nodeTickets.Add(node.Ticket)
		}

		if len(req.Filter) > 0 && len(nodeTickets) > 0 {
			nodes, err := d.Nodes(ctx, &xpb.NodesRequest{
				Ticket: nodeTickets.Slice(),
				Filter: req.Filter,
			})
			if err != nil {
				return nil, fmt.Errorf("error getting related node facts: %v", err)
			}
			for _, n := range nodes.Node {
				reply.Nodes[n.Ticket] = n
			}
		}

		if rs.Next() {
			rec, err := proto.Marshal(&srvpb.PageToken{SecondaryIndex: int32(edgesOffset + requestedEdges)})
			if err != nil {
				return nil, fmt.Errorf("internal error: error marshalling page token: %v", err)
			}
			reply.NextPageToken = base64.StdEncoding.EncodeToString(rec)
		}
	}

	return reply, nil
}

func addAnchor(anchors []*xpb.Anchor, rec []byte, anchorText bool) ([]*xpb.Anchor, error) {
	a := new(xpb.Anchor)
	if err := proto.Unmarshal(rec, a); err != nil {
		return anchors, err
	}
	if !anchorText {
		a.Text = ""
	}
	return append(anchors, a), nil
}

func (d *DB) scanReferences(fileTicket string, norm *xrefs.Normalizer) ([]*xpb.DecorationsReply_Reference, error) {
	rs, err := d.Query("SELECT anchor_ticket, kind, target_ticket, start_offset, end_offset FROM Decorations WHERE file_ticket = $1 ORDER BY start_offset, end_offset;", fileTicket)
	if err != nil {
		return nil, fmt.Errorf("error retrieving decorations: %v", err)
	}
	defer rs.Close()

	var references []*xpb.DecorationsReply_Reference
	for rs.Next() {
		r := &xpb.DecorationsReply_Reference{
			AnchorStart: &xpb.Location_Point{},
			AnchorEnd:   &xpb.Location_Point{},
		}
		if err := rs.Scan(&r.SourceTicket, &r.Kind, &r.TargetTicket, &r.AnchorStart.ByteOffset, &r.AnchorEnd.ByteOffset); err != nil {
			return nil, fmt.Errorf("sql scan error: %v", err)
		}
		r.AnchorStart = norm.Point(r.AnchorStart)
		r.AnchorEnd = norm.Point(r.AnchorEnd)
		references = append(references, r)
	}

	return references, nil
}

func sqlSetQuery(n int, items interface{}) (query string, args []interface{}) {
	v := reflect.ValueOf(items)
	l := v.Len()
	qs := make([]string, 0, l)
	for i := 0; i < l; i++ {
		qs = append(qs, fmt.Sprintf("$%d", n))
		args = append(args, v.Index(i).Interface())
		n++
	}
	return fmt.Sprintf("(%s)", strings.Join(qs, ",")), args
}
