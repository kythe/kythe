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

package xrefs

import (
	"bytes"
	"context"
	"fmt"
	"io"
	"log"

	"kythe.io/kythe/go/services/xrefs"
	"kythe.io/kythe/go/serving/xrefs/columnar"
	"kythe.io/kythe/go/storage/keyvalue"
	"kythe.io/kythe/go/storage/table"
	"kythe.io/kythe/go/util/keys"
	"kythe.io/kythe/go/util/kytheuri"
	"kythe.io/kythe/go/util/schema"
	"kythe.io/kythe/go/util/schema/facts"
	"kythe.io/kythe/go/util/span"

	"bitbucket.org/creachadair/stringset"
	"github.com/golang/protobuf/proto"
	"google.golang.org/grpc/codes"
	"google.golang.org/grpc/status"

	cpb "kythe.io/kythe/proto/common_go_proto"
	xpb "kythe.io/kythe/proto/xref_go_proto"
	xspb "kythe.io/kythe/proto/xref_serving_go_proto"
)

// ColumnarTableKeyMarker is stored within a Kythe columnar table to
// differentiate it from the legacy combined table format.
const ColumnarTableKeyMarker = "kythe:columnar"

// NewService returns an xrefs.Service backed by the given table.  The format of
// the table with be automatically detected.
func NewService(ctx context.Context, t keyvalue.DB) xrefs.Service {
	_, err := t.Get(ctx, []byte(ColumnarTableKeyMarker), nil)
	if err == nil {
		log.Println("WARNING: detected a experimental columnar table")
		return NewColumnarTable(t)
	}
	return NewCombinedTable(&table.KVProto{t})
}

// NewColumnarTable returns a table for the given columnar xrefs lookup table.
func NewColumnarTable(t keyvalue.DB) *ColumnarTable {
	return &ColumnarTable{t, NewCombinedTable(&table.KVProto{t})}
}

// ColumnarTable implements an xrefs.Service backed by a columnar serving table.
type ColumnarTable struct {
	keyvalue.DB

	// TODO(schroederc): implement xrefs columnar format reader
	*Table // fallback non-columnar xrefs/documentation
}

// Decorations implements part of the xrefs.Service interface.
func (c *ColumnarTable) Decorations(ctx context.Context, req *xpb.DecorationsRequest) (*xpb.DecorationsReply, error) {
	ticket, err := kytheuri.Fix(req.GetLocation().Ticket)
	if err != nil {
		return nil, status.Errorf(codes.InvalidArgument, "invalid ticket %q: %v", req.GetLocation().Ticket, err)
	} else if req.Location.Kind == xpb.Location_SPAN && req.Location.Span == nil {
		return nil, status.Errorf(codes.InvalidArgument, "missing requested Location span: %v", req.Location)
	}

	// TODO(schroederc): handle SPAN requests
	// TODO(schroederc): handle dirty buffers

	fileURI, err := kytheuri.Parse(ticket)
	if err != nil {
		return nil, err
	}
	file := fileURI.VName()
	prefix, err := keys.Append(columnar.DecorationsKeyPrefix, file)
	if err != nil {
		return nil, err
	}
	it, err := c.DB.ScanPrefix(ctx, prefix, &keyvalue.Options{LargeRead: true})
	if err != nil {
		return nil, err
	}

	k, val, err := it.Next()
	if err == io.EOF || !bytes.Equal(k, prefix) {
		return nil, xrefs.ErrDecorationsNotFound
	} else if err != nil {
		return nil, err
	}

	// Decode FileDecorations Index
	var idx xspb.FileDecorations_Index
	if err := proto.Unmarshal(val, &idx); err != nil {
		return nil, fmt.Errorf("error decoding index: %v", err)
	}

	// Setup reply state based on request
	reply := &xpb.DecorationsReply{Location: req.Location}
	if req.References && len(req.Filter) > 0 {
		reply.Nodes = make(map[string]*cpb.NodeInfo)
	}
	if req.TargetDefinitions {
		reply.DefinitionLocations = make(map[string]*xpb.Anchor)
	}

	// Setup scanning state for constructing reply
	var norm *span.Normalizer                                          // span normalizer for references
	refsByTarget := make(map[string][]*xpb.DecorationsReply_Reference) // target -> set<Reference>
	defs := stringset.New()                                            // set<needed definition tickets>
	patterns := xrefs.ConvertFilters(req.Filter)
	emitSnippets := req.Snippets != xpb.SnippetsKind_NONE

	// Main loop to scan over each columnar kv entry.
	for {
		k, val, err := it.Next()
		if err == io.EOF {
			break
		} else if err != nil {
			return nil, err
		}
		key := string(k[len(prefix):])

		// TODO(schroederc): only parse needed entries
		e, err := columnar.DecodeDecorationsEntry(file, key, val)
		if err != nil {
			return nil, err
		}

		switch e := e.Entry.(type) {
		case *xspb.FileDecorations_Text_:
			file := e.Text
			norm = span.NewNormalizer(file.Text)

			loc, err := norm.Location(req.GetLocation())
			if err != nil {
				return nil, status.Errorf(codes.InvalidArgument, "invalid Location: %v", err)
			}
			reply.Location = loc

			if req.SourceText {
				reply.Encoding = idx.TextEncoding
				if loc.Kind == xpb.Location_FILE {
					reply.SourceText = file.Text
				} else {
					reply.SourceText = file.Text[loc.Span.Start.ByteOffset:loc.Span.End.ByteOffset]
				}
			}
		case *xspb.FileDecorations_Target_:
			if !req.References {
				// TODO(schroederc): seek to next group
				continue
			}
			t := e.Target
			kind := t.GetGenericKind()
			if kind == "" {
				kind = schema.EdgeKindString(t.GetKytheKind())
			}
			ref := &xpb.DecorationsReply_Reference{
				TargetTicket: kytheuri.ToString(t.Target),
				Kind:         kind,
				Span:         norm.SpanOffsets(t.StartOffset, t.EndOffset),
			}
			refsByTarget[ref.TargetTicket] = append(refsByTarget[ref.TargetTicket], ref)
			reply.Reference = append(reply.Reference, ref)
		case *xspb.FileDecorations_TargetOverride_:
			// TODO(schroederc): handle
		case *xspb.FileDecorations_TargetNode_:
			if len(patterns) == 0 {
				// TODO(schroederc): seek to next group
				continue
			}
			n := e.TargetNode.Node

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
			if len(c.Facts) > 0 {
				reply.Nodes[kytheuri.ToString(n.Source)] = c
			}
		case *xspb.FileDecorations_TargetDefinition_:
			if !req.TargetDefinitions {
				continue
			}
			def := e.TargetDefinition
			// refsByTarget will be populated by now due to our chosen key ordering
			// See: kythe/proto/xref_serving.proto
			refs := refsByTarget[kytheuri.ToString(def.Target)]
			if len(refs) == 0 {
				continue
			}
			defTicket := kytheuri.ToString(def.Definition)
			defs.Add(defTicket)
			for _, ref := range refs {
				ref.TargetDefinition = defTicket
			}
		case *xspb.FileDecorations_DefinitionLocation_:
			if !req.TargetDefinitions {
				continue
			}
			def := e.DefinitionLocation
			if !defs.Contains(def.Location.Ticket) {
				continue
			}
			reply.DefinitionLocations[def.Location.Ticket] = a2a(def.Location, emitSnippets).Anchor
		case *xspb.FileDecorations_Override_:
			// TODO(schroederc): handle
		case *xspb.FileDecorations_Diagnostic_:
			// TODO(schroederc): handle
		default:
			return nil, fmt.Errorf("unknown FileDecorations entry: %T", e)
		}
	}
	if err := it.Close(); err != nil {
		return nil, err
	}

	return reply, nil
}
