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
	"log"
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

	ipb "kythe.io/kythe/proto/internal_proto"
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
	defer closeRows(rs)

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

		if nodeKind != "" {
			n.Facts[ticket] = []byte(nodeKind)
		}
		if subkind.Valid {
			n.Facts[ticket] = []byte(subkind.String)
		}
		if text != nil { // TODO(schroederc): NULL text
			n.Facts[ticket] = text
		}
		if textEncoding.Valid {
			n.Facts[ticket] = []byte(textEncoding.String)
		}
		if startOffset.Valid {
			n.Facts[ticket] = []byte(strconv.FormatInt(startOffset.Int64, 10))
		}
		if endOffset.Valid {
			n.Facts[ticket] = []byte(strconv.FormatInt(endOffset.Int64, 10))
		}
		if snippetStart.Valid {
			n.Facts[ticket] = []byte(strconv.FormatInt(snippetStart.Int64, 10))
		}
		if snippetEnd.Valid {
			n.Facts[ticket] = []byte(strconv.FormatInt(snippetEnd.Int64, 10))
		}

		if len(req.Filter) > 0 {
			patterns := xrefs.ConvertFilters(req.Filter)
			matched := make(map[string][]byte, len(n.Facts))
			for name, value := range n.Facts {
				if xrefs.MatchesAny(name, patterns) {
					matched[name] = value
				}
			}
			n.Facts = matched
		}

		if len(n.Facts) > 0 {
			reply.Nodes[ticket] = n
		}
	}

	return &reply, nil
}

// Edges implements part of the xrefs.Interface.
func (d *DB) Edges(ctx context.Context, req *xpb.EdgesRequest) (*xpb.EdgesReply, error) {
	return d.edges(ctx, req, nil)
}

func (d *DB) edges(ctx context.Context, req *xpb.EdgesRequest, edgeFilter func(kind string) bool) (*xpb.EdgesReply, error) {
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

	var pageOffset int
	if req.PageToken != "" {
		rec, err := base64.StdEncoding.DecodeString(req.PageToken)
		if err != nil {
			return nil, fmt.Errorf("invalid page_token: %q", req.PageToken)
		}
		var t ipb.PageToken
		if err := proto.Unmarshal(rec, &t); err != nil || t.Index < 0 {
			return nil, fmt.Errorf("invalid page_token: %q", req.PageToken)
		}
		pageOffset = int(t.Index)
	}

	// Select only the edges from the given source tickets
	setQ, args := sqlSetQuery(1, tickets)
	query := fmt.Sprintf(`
SELECT * FROM AllEdges
WHERE source IN %s`, setQ)

	// Filter by edges kinds, if given
	if len(req.Kind) > 0 {
		kSetQ, kArgs := sqlSetQuery(1+len(args), req.Kind)
		query += fmt.Sprintf(`
AND kind IN %s`, kSetQ)
		args = append(args, kArgs...)
	}

	// Scan edge sets/groups in order; necessary for CrossReferences
	query += " ORDER BY source, kind, ordinal"

	// Seek to the requested page offset (req.PageToken.Index).  We don't use
	// LIMIT here because we don't yet know how many edges will be filtered by
	// edgeFilter.
	query += fmt.Sprintf(" OFFSET $%d", len(args)+1)
	args = append(args, pageOffset)

	rs, err := d.Query(query, args...)
	if err != nil {
		return nil, fmt.Errorf("error querying for edges: %v", err)
	}
	defer closeRows(rs)

	var scanned int
	// edges := map { source -> kind -> target -> ordinal set }
	edges := make(map[string]map[string]map[string]map[int32]struct{}, len(tickets))
	for count := 0; count < pageSize && rs.Next(); scanned++ {
		var source, kind, target string
		var ordinal int
		if err := rs.Scan(&source, &kind, &target, &ordinal); err != nil {
			return nil, fmt.Errorf("edges scan error: %v", err)
		}
		if edgeFilter != nil && !edgeFilter(kind) {
			continue
		}
		count++

		groups, ok := edges[source]
		if !ok {
			groups = make(map[string]map[string]map[int32]struct{})
			edges[source] = groups
		}
		targets, ok := groups[kind]
		if !ok {
			targets = make(map[string]map[int32]struct{})
			groups[kind] = targets
		}
		ordinals, ok := targets[target]
		if !ok {
			ordinals = make(map[int32]struct{})
			targets[target] = ordinals
		}
		ordinals[int32(ordinal)] = struct{}{}
	}

	reply := &xpb.EdgesReply{EdgeSets: make(map[string]*xpb.EdgeSet, len(edges))}
	nodeTickets := stringset.New()
	for src, groups := range edges {
		gs := make(map[string]*xpb.EdgeSet_Group, len(groups))
		nodeTickets.Add(src)
		for kind, targets := range groups {
			edges := make([]*xpb.EdgeSet_Group_Edge, 0, len(targets))
			for ticket, ordinals := range targets {
				for ordinal := range ordinals {
					edges = append(edges, &xpb.EdgeSet_Group_Edge{
						TargetTicket: ticket,
						Ordinal:      ordinal,
					})
				}
				nodeTickets.Add(ticket)
			}
			sort.Sort(xrefs.ByOrdinal(edges))
			gs[kind] = &xpb.EdgeSet_Group{
				Edge: edges,
			}
		}
		reply.EdgeSets[src] = &xpb.EdgeSet{
			Groups: gs,
		}
	}

	// If there is another row, there is a NextPageToken.
	if rs.Next() {
		rec, err := proto.Marshal(&ipb.PageToken{Index: int32(pageOffset + scanned)})
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
		reply.Nodes = nodes.Nodes
	}

	return reply, nil
}

// Callers implements part of the xrefs.Interface.
func (d *DB) Callers(ctx context.Context, req *xpb.CallersRequest) (*xpb.CallersReply, error) {
	return xrefs.SlowCallers(ctx, d, req)
}

// Documentation implements part of the xrefs.Interface.
func (d *DB) Documentation(ctx context.Context, req *xpb.DocumentationRequest) (*xpb.DocumentationReply, error) {
	return xrefs.SlowDocumentation(ctx, d, req)
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
			decor.Nodes = nodes.Nodes
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
	// TODO(zarko): Callgraph integration.
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

	var pageOffset int
	var edgesToken string
	if req.PageToken != "" {
		rec, err := base64.StdEncoding.DecodeString(req.PageToken)
		if err != nil {
			return nil, fmt.Errorf("invalid page_token: %q", req.PageToken)
		}
		var t ipb.PageToken
		if err := proto.Unmarshal(rec, &t); err != nil || t.Index < 0 {
			return nil, fmt.Errorf("invalid page_token: %q", req.PageToken)
		}
		pageOffset = int(t.Index)
		edgesToken = t.SecondaryToken
	}

	reply := &xpb.CrossReferencesReply{
		CrossReferences: make(map[string]*xpb.CrossReferencesReply_CrossReferenceSet),
		Nodes:           make(map[string]*xpb.NodeInfo),
	}

	setQ, ticketArgs := sqlSetQuery(1, tickets)

	var count int
	if edgesToken == "" {
		args := append(ticketArgs, pageSize+1, pageOffset) // +1 to check for next page

		rs, err := d.Query(fmt.Sprintf("SELECT ticket, kind, proto FROM CrossReferences WHERE ticket IN %s ORDER BY ticket LIMIT $%d OFFSET $%d;", setQ, len(tickets)+1, len(tickets)+2), args...)
		if err != nil {
			return nil, err
		}
		defer closeRows(rs)

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
			// TODO(schroederc): handle declarations
			case xrefs.IsDefKind(req.DefinitionKind, kind, false):
				xrs.Definition, err = addRelatedAnchor(xrs.Definition, rec, req.AnchorText)
				if err != nil {
					return nil, err
				}
			case xrefs.IsDocKind(req.DocumentationKind, kind):
				xrs.Documentation, err = addRelatedAnchor(xrs.Documentation, rec, req.AnchorText)
				if err != nil {
					return nil, err
				}
			case xrefs.IsRefKind(req.ReferenceKind, kind):
				xrs.Reference, err = addRelatedAnchor(xrs.Reference, rec, req.AnchorText)
				if err != nil {
					return nil, err
				}
			}
		}
		if xrs != nil && (len(xrs.Definition) > 0 || len(xrs.Documentation) > 0 || len(xrs.Reference) > 0 || len(xrs.RelatedNode) > 0) {
			reply.CrossReferences[xrs.Ticket] = xrs
		}

		if count > pageSize {
			rec, err := proto.Marshal(&ipb.PageToken{Index: int32(pageOffset + pageSize)})
			if err != nil {
				return nil, fmt.Errorf("internal error: error marshalling page token: %v", err)
			}
			reply.NextPageToken = base64.StdEncoding.EncodeToString(rec)
		}
	}

	if len(req.Filter) > 0 && count <= pageSize {
		// TODO(schroederc): consolidate w/ LevelDB implementation
		er, err := d.edges(ctx, &xpb.EdgesRequest{
			Ticket:    tickets,
			Filter:    req.Filter,
			PageSize:  int32(pageSize - count),
			PageToken: edgesToken,
		}, func(kind string) bool {
			return !schema.IsAnchorEdge(kind)
		})
		if err != nil {
			return nil, fmt.Errorf("error getting related nodes: %v", err)
		}

		for ticket, es := range er.EdgeSets {
			nodes := stringset.New()
			crs, ok := reply.CrossReferences[ticket]
			if !ok {
				crs = &xpb.CrossReferencesReply_CrossReferenceSet{
					Ticket: ticket,
				}
			}
			for kind, g := range es.Groups {
				if !schema.IsAnchorEdge(kind) {
					for _, edge := range g.Edge {
						nodes.Add(edge.TargetTicket)
						crs.RelatedNode = append(crs.RelatedNode, &xpb.CrossReferencesReply_RelatedNode{
							RelationKind: kind,
							Ticket:       edge.TargetTicket,
							Ordinal:      edge.Ordinal,
						})
					}
				}
			}
			if len(nodes) > 0 {
				for ticket, n := range er.Nodes {
					if nodes.Contains(ticket) {
						reply.Nodes[ticket] = n
					}
				}
			}

			if !ok && len(crs.RelatedNode) > 0 {
				reply.CrossReferences[ticket] = crs
			}
		}

		if er.NextPageToken != "" {
			rec, err := proto.Marshal(&ipb.PageToken{SecondaryToken: er.NextPageToken})
			if err != nil {
				return nil, fmt.Errorf("internal error: error marshalling page token: %v", err)
			}
			reply.NextPageToken = base64.StdEncoding.EncodeToString(rec)
		}
	}

	return reply, nil
}

func addRelatedAnchor(anchors []*xpb.CrossReferencesReply_RelatedAnchor, rec []byte, anchorText bool) ([]*xpb.CrossReferencesReply_RelatedAnchor, error) {
	a := new(xpb.Anchor)
	if err := proto.Unmarshal(rec, a); err != nil {
		return anchors, err
	}
	if !anchorText {
		a.Text = ""
	}
	return append(anchors, &xpb.CrossReferencesReply_RelatedAnchor{Anchor: a}), nil
}

func (d *DB) scanReferences(fileTicket string, norm *xrefs.Normalizer) ([]*xpb.DecorationsReply_Reference, error) {
	rs, err := d.Query("SELECT anchor_ticket, kind, target_ticket, start_offset, end_offset FROM Decorations WHERE file_ticket = $1 ORDER BY start_offset, end_offset;", fileTicket)
	if err != nil {
		return nil, fmt.Errorf("error retrieving decorations: %v", err)
	}
	defer closeRows(rs)

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

func closeRows(rs *sql.Rows) {
	if err := rs.Close(); err != nil {
		log.Printf("WARNING: error closing SQL scanner: %v", err)
	}
}
