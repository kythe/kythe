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
	"regexp"

	"kythe.io/kythe/go/services/xrefs"
	"kythe.io/kythe/go/serving/xrefs/columnar"
	"kythe.io/kythe/go/storage/keyvalue"
	"kythe.io/kythe/go/storage/table"
	"kythe.io/kythe/go/util/keys"
	"kythe.io/kythe/go/util/kytheuri"
	"kythe.io/kythe/go/util/log"
	"kythe.io/kythe/go/util/schema"
	"kythe.io/kythe/go/util/schema/facts"
	"kythe.io/kythe/go/util/span"

	"bitbucket.org/creachadair/stringset"
	"google.golang.org/grpc/codes"
	"google.golang.org/grpc/status"
	"google.golang.org/protobuf/proto"

	cpb "kythe.io/kythe/proto/common_go_proto"
	scpb "kythe.io/kythe/proto/schema_go_proto"
	spb "kythe.io/kythe/proto/serving_go_proto"
	srvpb "kythe.io/kythe/proto/serving_go_proto"
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
		log.Warning("detected a experimental columnar xrefs table")
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

	*Table // fallback non-columnar documentation
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
	// TODO(schroederc): file infos

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
	var patcher *span.Patcher
	var norm *span.Normalizer                                          // span normalizer for references
	refsByTarget := make(map[string][]*xpb.DecorationsReply_Reference) // target -> set<Reference>
	defs := stringset.New()                                            // set<needed definition tickets>
	reply.ExtendsOverrides = make(map[string]*xpb.DecorationsReply_Overrides)
	buildConfigs := stringset.New(req.BuildConfig...)
	patterns := xrefs.ConvertFilters(req.Filter)
	emitSnippets := req.Snippets != xpb.SnippetsKind_NONE

	// The span with which to constrain the set of returned anchor references.
	var startBoundary, endBoundary int32
	spanKind := req.SpanKind

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
			// TODO(danielnorberg): Move the handling of this entry type up out of the loop to
			//                      ensure that variables used in other cases have been assigned
			text := e.Text.Text
			if len(req.DirtyBuffer) > 0 {
				patcher, err = span.NewPatcher(text, req.DirtyBuffer)
				if err != nil {
					return nil, status.Errorf(codes.Internal, "error patching decorations for %s: %v", req.Location.Ticket, err)
				}
				text = req.DirtyBuffer
			}
			norm = span.NewNormalizer(text)

			loc, err := norm.Location(req.GetLocation())
			if err != nil {
				return nil, status.Errorf(codes.InvalidArgument, "invalid Location: %v", err)
			}
			reply.Location = loc

			if loc.Kind == xpb.Location_FILE {
				startBoundary = 0
				endBoundary = int32(len(text))
				spanKind = xpb.DecorationsRequest_WITHIN_SPAN
			} else {
				startBoundary = loc.Span.Start.ByteOffset
				endBoundary = loc.Span.End.ByteOffset
			}

			if req.SourceText {
				reply.Encoding = idx.TextEncoding
				if loc.Kind == xpb.Location_FILE {
					reply.SourceText = text
				} else {
					reply.SourceText = text[loc.Span.Start.ByteOffset:loc.Span.End.ByteOffset]
				}
			}
		case *xspb.FileDecorations_Target_:
			if !req.References {
				// TODO(schroederc): seek to next group
				continue
			}
			t := e.Target
			// Filter decorations by requested build configs.
			if len(buildConfigs) != 0 && !buildConfigs.Contains(t.BuildConfig) {
				continue
			}
			kind := t.GetGenericKind()
			if kind == "" {
				kind = schema.EdgeKindString(t.GetKytheKind())
			}
			start, end, exists := patcher.Patch(t.StartOffset, t.EndOffset)
			// Filter non-existent anchor.  Anchors can no longer exist if we were
			// given a dirty buffer and the anchor was inside a changed region.
			if !exists || !span.InBounds(spanKind, start, end, startBoundary, endBoundary) {
				continue
			}
			ref := &xpb.DecorationsReply_Reference{
				TargetTicket: kytheuri.ToString(t.Target),
				BuildConfig:  t.BuildConfig,
				Kind:         kind,
				Span:         norm.SpanOffsets(start, end),
			}
			refsByTarget[ref.TargetTicket] = append(refsByTarget[ref.TargetTicket], ref)
			reply.Reference = append(reply.Reference, ref)
		case *xspb.FileDecorations_TargetOverride_:
			overridingTicket := kytheuri.ToString(e.TargetOverride.Overriding)
			t, ok := reply.ExtendsOverrides[overridingTicket]
			if !ok {
				t = &xpb.DecorationsReply_Overrides{}
				reply.ExtendsOverrides[overridingTicket] = t
			}
			var kind xpb.DecorationsReply_Override_Kind
			switch e.TargetOverride.Kind {
			case spb.FileDecorations_Override_OVERRIDES:
				kind = xpb.DecorationsReply_Override_OVERRIDES
			case spb.FileDecorations_Override_EXTENDS:
				kind = xpb.DecorationsReply_Override_EXTENDS
			}
			t.Override = append(t.Override, &xpb.DecorationsReply_Override{
				Target:           kytheuri.ToString(e.TargetOverride.Overridden),
				Kind:             kind,
				TargetDefinition: e.TargetOverride.OverridingDefinition.Ticket,
			})
			if req.TargetDefinitions {
				reply.DefinitionLocations[e.TargetOverride.OverridingDefinition.Ticket] = a2a(e.TargetOverride.OverridingDefinition, nil, false).Anchor
			}
		case *xspb.FileDecorations_TargetNode_:
			if len(patterns) == 0 {
				// TODO(schroederc): seek to next group
				continue
			}
			n := e.TargetNode.Node
			c := filterNode(patterns, n)
			if c != nil && len(c.Facts) > 0 {
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
			reply.DefinitionLocations[def.Location.Ticket] = a2a(def.Location, nil, emitSnippets).Anchor
		case *xspb.FileDecorations_Override_:
			// TODO(schroederc): handle
		case *xspb.FileDecorations_Diagnostic_:
			if !req.Diagnostics {
				continue
			}
			diag := e.Diagnostic.Diagnostic
			if diag.Span == nil {
				reply.Diagnostic = append(reply.Diagnostic, diag)
			} else {
				start, end, exists := patcher.Patch(span.ByteOffsets(diag.Span))
				// Filter non-existent (or out-of-bounds) diagnostic.  Diagnostics can
				// no longer exist if we were given a dirty buffer and the diagnostic
				// was inside a changed region.
				if !exists || !span.InBounds(spanKind, start, end, startBoundary, endBoundary) {
					continue
				}

				diag.Span = norm.SpanOffsets(start, end)
				reply.Diagnostic = append(reply.Diagnostic, diag)
			}
		default:
			return nil, fmt.Errorf("unknown FileDecorations entry: %T", e)
		}
	}
	if err := it.Close(); err != nil {
		return nil, err
	}

	return reply, nil
}

func addXRefNode(reply *xpb.CrossReferencesReply, patterns []*regexp.Regexp, n *scpb.Node) {
	if len(patterns) == 0 {
		return
	}
	ticket := kytheuri.ToString(n.Source)
	if _, ok := reply.Nodes[ticket]; !ok {
		c := filterNode(patterns, n)
		if c != nil && len(c.Facts) > 0 {
			reply.Nodes[ticket] = c
		}
	}
}

// CrossReferences implements part of the xrefs.Service interface.
func (c *ColumnarTable) CrossReferences(ctx context.Context, req *xpb.CrossReferencesRequest) (*xpb.CrossReferencesReply, error) {
	reply := &xpb.CrossReferencesReply{
		CrossReferences: make(map[string]*xpb.CrossReferencesReply_CrossReferenceSet),
	}

	relatedNodes := stringset.New()
	patterns := xrefs.ConvertFilters(req.Filter)
	relatedKinds := stringset.New(req.RelatedNodeKind...)
	if len(patterns) > 0 {
		reply.Nodes = make(map[string]*cpb.NodeInfo)
	}
	if req.NodeDefinitions {
		reply.DefinitionLocations = make(map[string]*xpb.Anchor)
	}
	emitSnippets := req.Snippets != xpb.SnippetsKind_NONE

	// TODO(schroederc): file infos
	// TODO(schroederc): implement paging xrefs in large CrossReferencesReply messages

	for _, ticket := range req.Ticket {
		uri, err := kytheuri.Parse(ticket)
		if err != nil {
			return nil, err
		}
		prefix, err := keys.Append(columnar.CrossReferencesKeyPrefix, uri.VName())
		if err != nil {
			return nil, err
		}
		it, err := c.DB.ScanPrefix(ctx, prefix, &keyvalue.Options{LargeRead: true})
		if err != nil {
			return nil, err
		}
		defer it.Close()

		k, val, err := it.Next()
		if err == io.EOF || !bytes.Equal(k, prefix) {
			continue
		} else if err != nil {
			return nil, err
		}

		// Decode CrossReferences Index
		var idx xspb.CrossReferences_Index
		if err := proto.Unmarshal(val, &idx); err != nil {
			return nil, fmt.Errorf("error decoding index: %v", err)
		}
		idx.Node.Source = uri.VName()
		addXRefNode(reply, patterns, idx.Node)

		// TODO(schroederc): handle merge_with

		set := &xpb.CrossReferencesReply_CrossReferenceSet{
			Ticket:       ticket,
			MarkedSource: idx.MarkedSource,
		}
		reply.CrossReferences[ticket] = set

		// TODO remove callers without callsites
		callers := make(map[string]*xpb.CrossReferencesReply_RelatedAnchor)

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
			e, err := columnar.DecodeCrossReferencesEntry(uri.VName(), key, val)
			if err != nil {
				return nil, err
			}

			switch e := e.Entry.(type) {
			case *xspb.CrossReferences_Reference_:
				ref := e.Reference
				kind := getRefKind(ref)
				var anchors *[]*xpb.CrossReferencesReply_RelatedAnchor
				switch {
				case xrefs.IsDefKind(req.DefinitionKind, kind, false):
					anchors = &set.Definition
				case xrefs.IsDeclKind(req.DeclarationKind, kind, false):
					anchors = &set.Declaration
				case xrefs.IsRefKind(req.ReferenceKind, kind):
					anchors = &set.Reference
				}
				if anchors != nil {
					a := a2a(ref.Location, nil, emitSnippets).Anchor
					a.Ticket = ""
					ra := &xpb.CrossReferencesReply_RelatedAnchor{Anchor: a}
					*anchors = append(*anchors, ra)
				}
			case *xspb.CrossReferences_Relation_:
				if len(patterns) == 0 {
					continue
				}
				rel := e.Relation
				kind := rel.GetGenericKind()
				if kind == "" {
					kind = schema.EdgeKindString(rel.GetKytheKind())
				}
				if rel.Reverse {
					kind = "%" + kind
				}
				if xrefs.IsRelatedNodeKind(relatedKinds, kind) {
					relatedNode := kytheuri.ToString(rel.Node)
					relatedNodes.Add(relatedNode)
					set.RelatedNode = append(set.RelatedNode, &xpb.CrossReferencesReply_RelatedNode{
						Ticket:       relatedNode,
						RelationKind: kind,
						Ordinal:      rel.Ordinal,
					})
				}
			case *xspb.CrossReferences_RelatedNode_:
				relatedNode := kytheuri.ToString(e.RelatedNode.Node.Source)
				if relatedNodes.Contains(relatedNode) {
					addXRefNode(reply, patterns, e.RelatedNode.Node)
				}
			case *xspb.CrossReferences_NodeDefinition_:
				if !req.NodeDefinitions || len(reply.Nodes) == 0 {
					continue
				}

				relatedNode := kytheuri.ToString(e.NodeDefinition.Node)
				if node := reply.Nodes[relatedNode]; node != nil {
					loc := e.NodeDefinition.Location
					node.Definition = loc.Ticket
					a := a2a(loc, nil, emitSnippets).Anchor
					reply.DefinitionLocations[loc.Ticket] = a
				}
			case *xspb.CrossReferences_Caller_:
				if req.CallerKind == xpb.CrossReferencesRequest_NO_CALLERS {
					continue
				}
				c := e.Caller
				a := a2a(c.Location, nil, emitSnippets).Anchor
				a.Ticket = ""
				callerTicket := kytheuri.ToString(c.Caller)
				caller := &xpb.CrossReferencesReply_RelatedAnchor{
					Anchor:       a,
					MarkedSource: c.MarkedSource,
					Ticket:       callerTicket,
				}
				callers[callerTicket] = caller
				set.Caller = append(set.Caller, caller)
			case *xspb.CrossReferences_Callsite_:
				c := e.Callsite
				if req.CallerKind == xpb.CrossReferencesRequest_NO_CALLERS ||
					(req.CallerKind == xpb.CrossReferencesRequest_DIRECT_CALLERS && c.Kind == xspb.CrossReferences_Callsite_OVERRIDE) {
					continue
				}
				caller := callers[kytheuri.ToString(c.Caller)]
				if caller == nil {
					log.Warningf("missing Caller for callsite: %+v", c)
					continue
				}
				a := a2a(c.Location, nil, emitSnippets).Anchor
				a.Ticket = ""
				// TODO(schroederc): set anchor kind to differentiate kinds?
				caller.Site = append(caller.Site, a)
			default:
				return nil, fmt.Errorf("unhandled internal serving type: %T", e)
			}
		}
	}

	return reply, nil
}

func getRefKind(ref *xspb.CrossReferences_Reference) string {
	if k := ref.GetGenericKind(); k != "" {
		return k
	}
	return schema.EdgeKindString(ref.GetKytheKind())
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

func a2a(a *srvpb.ExpandedAnchor, fileInfos map[string]*srvpb.FileInfo, anchorText bool) *xpb.CrossReferencesReply_RelatedAnchor {
	c := &anchorConverter{fileInfos: fileInfos, anchorText: anchorText}
	return c.Convert(a)
}
