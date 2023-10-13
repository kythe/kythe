/*
 * Copyright 2015 The Kythe Authors. All rights reserved.
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

// Package xrefs provides a high-performance table-based implementation of the
// xrefs.Service.
//
// Table format:
//
//	decor:<ticket>         -> srvpb.FileDecorations
//	docs:<ticket>          -> srvpb.Document
//	xrefs:<ticket>         -> srvpb.PagedCrossReferences
//	xrefPages:<page_key>   -> srvpb.PagedCrossReferences_Page
package xrefs // import "kythe.io/kythe/go/serving/xrefs"

import (
	"context"
	"encoding/base64"
	"flag"
	"fmt"
	"path/filepath"
	"regexp"
	"strings"
	"sync"
	"time"

	"kythe.io/kythe/go/services/xrefs"
	"kythe.io/kythe/go/storage/table"
	"kythe.io/kythe/go/util/flagutil"
	"kythe.io/kythe/go/util/kytheuri"
	"kythe.io/kythe/go/util/log"
	"kythe.io/kythe/go/util/schema/edges"
	"kythe.io/kythe/go/util/schema/facts"
	"kythe.io/kythe/go/util/schema/tickets"
	"kythe.io/kythe/go/util/span"

	"bitbucket.org/creachadair/stringset"
	"github.com/golang/snappy"
	"golang.org/x/net/trace"
	"golang.org/x/sync/errgroup"
	"google.golang.org/grpc/codes"
	"google.golang.org/grpc/status"
	"google.golang.org/protobuf/proto"

	cpb "kythe.io/kythe/proto/common_go_proto"
	ipb "kythe.io/kythe/proto/internal_go_proto"
	srvpb "kythe.io/kythe/proto/serving_go_proto"
	xpb "kythe.io/kythe/proto/xref_go_proto"
)

var (
	mergeCrossReferences = flag.Bool("merge_cross_references", true, "Whether to merge nodes when responding to a CrossReferencesRequest")

	experimentalCrossReferenceIndirectionKinds flagutil.StringMultimap

	// TODO(schroederc): remove once relevant clients specify their required quality
	defaultTotalsQuality = flag.String("experimental_default_totals_quality", "APPROXIMATE_TOTALS", "Default TotalsQuality when unspecified in CrossReferencesRequest")

	pageReadAhead = flag.Uint("page_read_ahead", 0, "How many xref pages to read ahead concurrently (0 disables readahead)")

	responseLeewayTime = flag.Duration("xrefs_response_leeway_time", 50*time.Millisecond, "If possible, leave this much time at the end of a CrossReferencesRequest to return any results already read")
)

func init() {
	flag.Var(&experimentalCrossReferenceIndirectionKinds, "experimental_cross_reference_indirection_kinds",
		`Comma-separated set of key-value pairs (node_kind=edge_kind) to indirect through in CrossReferences.  For example, "talias=/kythe/edge/aliases" indicates that the targets of a 'talias' node's '/kythe/edge/aliases' related nodes will have their cross-references merged into the root 'talias' node's.  A "*=edge_kind" entry indicates to indirect through the specified edge kind for any node kind.`)
}

type staticLookupTables interface {
	fileDecorations(ctx context.Context, ticket string) (*srvpb.FileDecorations, error)
	crossReferences(ctx context.Context, ticket string) (*srvpb.PagedCrossReferences, error)
	crossReferencesPage(ctx context.Context, key string) (*srvpb.PagedCrossReferences_Page, error)
	documentation(ctx context.Context, ticket string) (*srvpb.Document, error)
}

// SplitTable implements the xrefs Service interface using separate static
// lookup tables for each API component.
type SplitTable struct {
	// Decorations is a table of srvpb.FileDecorations keyed by their source
	// location tickets.
	Decorations table.Proto

	// CrossReferences is a table of srvpb.PagedCrossReferences keyed by their
	// source node tickets.
	CrossReferences table.Proto

	// CrossReferencePages is a table of srvpb.PagedCrossReferences_Pages keyed by
	// their page keys.
	CrossReferencePages table.Proto

	// Documentation is a table of srvpb.Documents keyed by their node ticket.
	Documentation table.Proto

	// RewriteEdgeLabel is an optional callback to rewrite edge labels.
	// It will be called once per request; the function it returns will then be
	// called once per edge.
	RewriteEdgeLabel func(context.Context) func(string) string
}

func (s *SplitTable) rewriteFileDecorations(ctx context.Context, fd *srvpb.FileDecorations, err error) (*srvpb.FileDecorations, error) {
	if fd == nil || err != nil || s.RewriteEdgeLabel == nil || fd.Decoration == nil {
		return fd, err
	}
	f := s.RewriteEdgeLabel(ctx)
	if f == nil {
		return fd, err
	}
	for _, d := range fd.Decoration {
		d.Kind = f(d.Kind)
	}
	return fd, err
}

func rewriteCrossReferencesGroup(g *srvpb.PagedCrossReferences_Group, f func(string) string) {
	if f != nil && g != nil {
		g.Kind = f(g.Kind)
	}
}

func (s *SplitTable) rewriteCrossReferences(ctx context.Context, cr *srvpb.PagedCrossReferences, err error) (*srvpb.PagedCrossReferences, error) {
	if cr == nil || err != nil || s.RewriteEdgeLabel == nil || cr.Group == nil {
		return cr, err
	}
	f := s.RewriteEdgeLabel(ctx)
	for _, g := range cr.Group {
		rewriteCrossReferencesGroup(g, f)
	}
	return cr, err
}

func (s *SplitTable) rewriteCrossReferencesPage(ctx context.Context, cr *srvpb.PagedCrossReferences_Page, err error) (*srvpb.PagedCrossReferences_Page, error) {
	if cr == nil || err != nil || s.RewriteEdgeLabel == nil || cr.Group == nil {
		return cr, err
	}
	f := s.RewriteEdgeLabel(ctx)
	rewriteCrossReferencesGroup(cr.Group, f)
	return cr, err
}

func (s *SplitTable) fileDecorations(ctx context.Context, ticket string) (*srvpb.FileDecorations, error) {
	tracePrintf(ctx, "Reading FileDecorations: %s", ticket)
	var fd srvpb.FileDecorations
	return s.rewriteFileDecorations(ctx, &fd, s.Decorations.Lookup(ctx, []byte(ticket), &fd))
}
func (s *SplitTable) crossReferences(ctx context.Context, ticket string) (*srvpb.PagedCrossReferences, error) {
	tracePrintf(ctx, "Reading PagedCrossReferences: %s", ticket)
	var cr srvpb.PagedCrossReferences
	return s.rewriteCrossReferences(ctx, &cr, s.CrossReferences.Lookup(ctx, []byte(ticket), &cr))
}
func (s *SplitTable) crossReferencesPage(ctx context.Context, key string) (*srvpb.PagedCrossReferences_Page, error) {
	tracePrintf(ctx, "Reading PagedCrossReferences.Page: %s", key)
	var p srvpb.PagedCrossReferences_Page
	return s.rewriteCrossReferencesPage(ctx, &p, s.CrossReferencePages.Lookup(ctx, []byte(key), &p))
}
func (s *SplitTable) documentation(ctx context.Context, ticket string) (*srvpb.Document, error) {
	tracePrintf(ctx, "Reading Document: %s", ticket)
	var d srvpb.Document
	return &d, s.Documentation.Lookup(ctx, []byte(ticket), &d)
}

// Key prefixes for the combinedTable implementation.
const (
	crossRefTablePrefix      = "xrefs:"
	crossRefPageTablePrefix  = "xrefPages:"
	decorTablePrefix         = "decor:"
	documentationTablePrefix = "docs:"
)

type combinedTable struct{ table.Proto }

func (c *combinedTable) fileDecorations(ctx context.Context, ticket string) (*srvpb.FileDecorations, error) {
	var fd srvpb.FileDecorations
	return &fd, c.Lookup(ctx, DecorationsKey(ticket), &fd)
}
func (c *combinedTable) crossReferences(ctx context.Context, ticket string) (*srvpb.PagedCrossReferences, error) {
	var cr srvpb.PagedCrossReferences
	return &cr, c.Lookup(ctx, CrossReferencesKey(ticket), &cr)
}
func (c *combinedTable) crossReferencesPage(ctx context.Context, key string) (*srvpb.PagedCrossReferences_Page, error) {
	var p srvpb.PagedCrossReferences_Page
	return &p, c.Lookup(ctx, CrossReferencesPageKey(key), &p)
}
func (c *combinedTable) documentation(ctx context.Context, ticket string) (*srvpb.Document, error) {
	var d srvpb.Document
	return &d, c.Lookup(ctx, DocumentationKey(ticket), &d)
}

// NewSplitTable returns a table based on the given serving tables for each API
// component.
func NewSplitTable(c *SplitTable) *Table { return &Table{staticLookupTables: c} }

// NewCombinedTable returns a table for the given combined xrefs lookup table.
// The table's keys are expected to be constructed using only the *Key functions.
func NewCombinedTable(t table.Proto) *Table { return &Table{staticLookupTables: &combinedTable{t}} }

// DecorationsKey returns the decorations CombinedTable key for the given source
// location ticket.
func DecorationsKey(ticket string) []byte {
	return []byte(decorTablePrefix + ticket)
}

// CrossReferencesKey returns the cross-references CombinedTable key for the
// given node ticket.
func CrossReferencesKey(ticket string) []byte {
	return []byte(crossRefTablePrefix + ticket)
}

// CrossReferencesPageKey returns the cross-references page CombinedTable key
// for the given key.
func CrossReferencesPageKey(key string) []byte {
	return []byte(crossRefPageTablePrefix + key)
}

// DocumentationKey returns the documentation CombinedTable key for the given
// ticket.
func DocumentationKey(ticket string) []byte {
	return []byte(documentationTablePrefix + ticket)
}

// Table implements the xrefs Service interface using static lookup tables.
type Table struct {
	staticLookupTables

	// MakePatcher returns a patching client that targets a Workspace.
	MakePatcher func(context.Context, *xpb.Workspace) (MultiFilePatcher, error)

	// ResolvePath is used to resolve CorpusPaths for filtering.  If unset,
	// DefaultResolvePath will be used.
	ResolvePath PathResolver
}

// A PathResolver resolves a CorpusPath into a single filepath.
type PathResolver func(*cpb.CorpusPath) string

// DefaultResolvePath returns the default resolved path for the CorpusPath by
// joining its corpus, root, and path into a single filepath.
func DefaultResolvePath(cp *cpb.CorpusPath) string {
	return filepath.Join(cp.GetCorpus(), cp.GetRoot(), cp.GetPath())
}

// A MultiFilePatcher provides an interface to patch sets of xref anchors to an
// underlying baseline, usually a Workspace.
//
// After creation, the client is required to call AddFile for each possible file
// referenced by any anchors that will be patched.  After the files are added, a
// set of anchors may be passed to PatchAnchors.
type MultiFilePatcher interface {
	// AddFile adds a file to current set of files to patch against.
	AddFile(context.Context, *srvpb.FileInfo) error

	// PatchAnchors updates the set of anchors given to match their referenced
	// files' state as known by the MultiLinePatcher, usually based on a
	// Workspace.  If an anchor no longer exists, it will be ellided from the
	// returned set.  Otherwise, the ordering of the anchors will be retained.
	PatchAnchors(context.Context, []*xpb.Anchor) ([]*xpb.Anchor, error)

	// PatchRelatedAnchors updates the set of related anchors given to match their
	// referenced files' state as known by the MultiLinePatcher, usually based on
	// a Workspace.  If an anchor no longer exists, it will be ellided from the
	// returned set.  Otherwise, the ordering of the anchors will be retained.
	PatchRelatedAnchors(context.Context, []*xpb.CrossReferencesReply_RelatedAnchor) ([]*xpb.CrossReferencesReply_RelatedAnchor, error)

	// Close releases any resources used the patcher.  Further calls to the
	// patcher will become invalid.
	Close() error
}

const (
	defaultPageSize = 2048
	maxPageSize     = 10000
)

type nodeConverter struct {
	factPatterns []*regexp.Regexp
}

func (c *nodeConverter) ToInfo(n *srvpb.Node) *cpb.NodeInfo {
	ni := &cpb.NodeInfo{Facts: make(map[string][]byte, len(n.Fact))}
	for _, f := range n.Fact {
		if xrefs.MatchesAny(f.Name, c.factPatterns) {
			ni.Facts[f.Name] = f.Value
		}
	}
	if len(ni.Facts) == 0 {
		return nil
	}
	return ni
}

func corpusPathTicket(cp *cpb.CorpusPath) string { return kytheuri.FromCorpusPath(cp).String() }

// Decorations implements part of the xrefs Service interface.
func (t *Table) Decorations(ctx context.Context, req *xpb.DecorationsRequest) (*xpb.DecorationsReply, error) {
	if req.GetLocation() == nil || req.GetLocation().Ticket == "" {
		return nil, status.Error(codes.InvalidArgument, "missing location")
	}

	ticket, err := kytheuri.Fix(req.GetLocation().Ticket)
	if err != nil {
		return nil, status.Errorf(codes.InvalidArgument, "invalid ticket %q: %v", req.GetLocation().Ticket, err)
	}

	var multiPatcher MultiFilePatcher
	if t.MakePatcher != nil && req.GetWorkspace() != nil && req.GetPatchAgainstWorkspace() {
		multiPatcher, err = t.MakePatcher(ctx, req.GetWorkspace())
		if isNonContextError(err) {
			log.Errorf("creating patcher: %v", err)
		}

		if multiPatcher != nil {
			defer func() {
				if err := multiPatcher.Close(); isNonContextError(err) {
					// No need to fail the request; just log the error.
					log.Errorf("closing patcher: %v", err)
				}
			}()
		}
	}

	decor, err := t.fileDecorations(ctx, ticket)
	if err == table.ErrNoSuchKey {
		return nil, xrefs.ErrDecorationsNotFound
	} else if err != nil {
		return nil, canonicalError(err, "file decorations", ticket)
	}

	if decor.File == nil {
		if len(decor.Diagnostic) == 0 {
			log.Errorf("FileDecorations.file is missing without related diagnostics: %q", req.Location.Ticket)
			return nil, xrefs.ErrDecorationsNotFound
		}

		// FileDecorations may be saved without a File if the file does not exist in
		// the index but related diagnostics do exist.  If diagnostics were
		// requested, we may return them successfully, but otherwise, an error
		// indicating a missing file is returned.
		if req.Diagnostics {
			return &xpb.DecorationsReply{
				Location:   req.Location,
				Diagnostic: decor.Diagnostic,
			}, nil
		}
		return nil, xrefs.ErrDecorationsNotFound
	}

	text := decor.File.Text
	if len(req.DirtyBuffer) > 0 {
		text = req.DirtyBuffer
	}
	norm := span.NewNormalizer(text)

	loc, err := norm.Location(req.GetLocation())
	if err != nil {
		return nil, err
	}

	fileInfos := makeFileInfoMap(decor.FileInfo)

	reply := &xpb.DecorationsReply{
		Location: loc,
		Revision: fileInfos[loc.GetTicket()].GetRevision(),
	}

	for _, g := range decor.GeneratedBy {
		uri, err := kytheuri.Parse(g)
		if err != nil {
			return nil, fmt.Errorf("unable to parse generated_by ticket %q: %w", g, err)
		}
		reply.GeneratedByFile = append(reply.GeneratedByFile, &xpb.File{
			CorpusPath: &cpb.CorpusPath{
				Corpus: uri.Corpus,
				Root:   uri.Root,
				Path:   uri.Path,
			},
			Revision: fileInfos[g].GetRevision(),
		})
	}

	if req.SourceText && text != nil {
		reply.Encoding = decor.File.Encoding
		if loc.Kind == xpb.Location_FILE {
			reply.SourceText = text
		} else {
			reply.SourceText = text[loc.Span.Start.ByteOffset:loc.Span.End.ByteOffset]
		}
	}

	var patcher *span.Patcher
	if len(req.DirtyBuffer) > 0 {
		if multiPatcher != nil {
			return nil, status.Errorf(codes.Unimplemented, "cannot patch decorations against Workspace with a dirty_buffer")
		}
		patcher, err = span.NewPatcher(decor.File.Text, req.DirtyBuffer)
		if err != nil {
			return nil, status.Errorf(codes.Internal, "error patching decorations for %s: %v", req.Location.Ticket, err)
		}
	}

	// The span with which to constrain the set of returned anchor references.
	var startBoundary, endBoundary int32
	spanKind := req.SpanKind
	if loc.Kind == xpb.Location_FILE {
		startBoundary = 0
		endBoundary = int32(len(text))
		spanKind = xpb.DecorationsRequest_WITHIN_SPAN
	} else {
		startBoundary = loc.Span.Start.ByteOffset
		endBoundary = loc.Span.End.ByteOffset
	}

	if req.References {
		patterns := xrefs.ConvertFilters(req.Filter)
		buildConfigs := stringset.New(req.BuildConfig...)

		ac := &anchorConverter{fileInfos: fileInfos}
		nc := &nodeConverter{patterns}

		reply.Reference = make([]*xpb.DecorationsReply_Reference, 0, len(decor.Decoration))
		reply.Nodes = make(map[string]*cpb.NodeInfo, len(decor.Target))

		// Reference.TargetTicket -> NodeInfo (superset of reply.Nodes)
		nodes := make(map[string]*cpb.NodeInfo, len(decor.Target))
		if len(patterns) > 0 {
			for _, n := range decor.Target {
				if info := nc.ToInfo(n); info != nil {
					nodes[n.Ticket] = info
				}
			}
		}
		tracePrintf(ctx, "Potential target nodes: %d", len(nodes))

		// All known definition locations (Anchor.Ticket -> Anchor)
		defs := make(map[string]*xpb.Anchor, len(decor.TargetDefinitions))
		for _, def := range decor.TargetDefinitions {
			a := ac.Convert(def).Anchor
			defs[def.Ticket] = a
			if multiPatcher != nil {
				fileInfo := def.GetFileInfo()
				if fileInfo == nil {
					fileInfo = fileInfos[a.GetParent()]
				}
				if fileInfo != nil {
					if err := multiPatcher.AddFile(ctx, fileInfo); isNonContextError(err) {
						// Attempt to continue with the request, just log the error.
						log.Errorf("adding file: %v", err)
					}
				}
			}
		}
		if req.TargetDefinitions {
			reply.DefinitionLocations = make(map[string]*xpb.Anchor, len(decor.TargetDefinitions))
		}
		tracePrintf(ctx, "Potential target defs: %d", len(defs))

		bindings := stringset.New()

		for _, d := range decor.Decoration {
			// Filter decorations by requested build configs.
			if len(buildConfigs) != 0 && !buildConfigs.Contains(d.Anchor.BuildConfiguration) {
				continue
			}

			start, end, exists := patcher.Patch(d.Anchor.StartOffset, d.Anchor.EndOffset)
			// Filter non-existent anchor.  Anchors can no longer exist if we were
			// given a dirty buffer and the anchor was inside a changed region.
			if !exists || !span.InBounds(spanKind, start, end, startBoundary, endBoundary) {
				continue
			}

			d.Anchor.StartOffset = start
			d.Anchor.EndOffset = end

			r := decorationToReference(norm, d)
			if req.TargetDefinitions {
				if def, ok := defs[d.TargetDefinition]; ok {
					reply.DefinitionLocations[d.TargetDefinition] = def
				}
			} else {
				r.TargetDefinition = ""
			}

			if !req.SemanticScopes {
				r.SemanticScope = ""
			}

			// Populate any target revision, if known
			r.TargetRevision = fileInfos[r.TargetTicket].GetRevision()

			if req.ExtendsOverrides && (r.Kind == edges.Defines || r.Kind == edges.DefinesBinding) {
				bindings.Add(r.TargetTicket)
			}

			reply.Reference = append(reply.Reference, r)

			if n := nodes[r.TargetTicket]; n != nil {
				reply.Nodes[r.TargetTicket] = n
			}
		}
		tracePrintf(ctx, "References: %d", len(reply.Reference))

		if len(decor.TargetOverride) > 0 {
			// Read overrides from serving data
			reply.ExtendsOverrides = make(map[string]*xpb.DecorationsReply_Overrides, len(bindings))

			for _, o := range decor.TargetOverride {
				if bindings.Contains(o.Overriding) {
					def := defs[o.OverriddenDefinition]
					if def != nil && len(buildConfigs) != 0 && !buildConfigs.Contains(def.BuildConfig) {
						// Skip override with undesirable build configuration.
						continue
					}

					os, ok := reply.ExtendsOverrides[o.Overriding]
					if !ok {
						os = &xpb.DecorationsReply_Overrides{}
						reply.ExtendsOverrides[o.Overriding] = os
					}

					ov := &xpb.DecorationsReply_Override{
						Target:       o.Overridden,
						Kind:         xpb.DecorationsReply_Override_Kind(o.Kind),
						MarkedSource: o.MarkedSource,
					}
					os.Override = append(os.Override, ov)

					if n := nodes[o.Overridden]; n != nil {
						reply.Nodes[o.Overridden] = n
					}
					if req.TargetDefinitions && def != nil {
						ov.TargetDefinition = o.OverriddenDefinition
						reply.DefinitionLocations[o.OverriddenDefinition] = def
					}
				}
			}
			tracePrintf(ctx, "ExtendsOverrides: %d", len(reply.ExtendsOverrides))
		}
		tracePrintf(ctx, "DefinitionLocations: %d", len(reply.DefinitionLocations))
	}

	if req.Diagnostics {
		for _, diag := range decor.Diagnostic {
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
		}
		tracePrintf(ctx, "Diagnostics: %d", len(reply.Diagnostic))
	}

	if req.Snippets == xpb.SnippetsKind_NONE {
		for _, anchor := range reply.DefinitionLocations {
			clearSnippet(anchor)
		}
	}

	if multiPatcher != nil {
		defs, err := patchDefLocations(ctx, multiPatcher, reply.GetDefinitionLocations())
		if err != nil {
			log.Errorf("patching definition locations: %v", err)
		} else {
			reply.DefinitionLocations = defs
		}
	}

	return reply, nil
}

func patchDefLocations(ctx context.Context, patcher MultiFilePatcher, defLocs map[string]*xpb.Anchor) (map[string]*xpb.Anchor, error) {
	if len(defLocs) == 0 {
		return nil, nil
	}
	defs := make([]*xpb.Anchor, 0, len(defLocs))
	for _, def := range defLocs {
		defs = append(defs, def)
	}
	defs, err := patcher.PatchAnchors(ctx, defs)
	if err != nil {
		return defLocs, err
	}
	res := make(map[string]*xpb.Anchor, len(defs))
	for _, def := range defs {
		res[def.GetTicket()] = def
	}
	tracePrintf(ctx, "Patched DefinitionLocations: %d", len(defs))
	return res, nil
}

func makeFileInfoMap(infos []*srvpb.FileInfo) map[string]*srvpb.FileInfo {
	fileInfos := make(map[string]*srvpb.FileInfo, len(infos))
	for _, info := range infos {
		fileInfos[corpusPathTicket(info.CorpusPath)] = info
	}
	return fileInfos
}

func decorationToReference(norm *span.Normalizer, d *srvpb.FileDecorations_Decoration) *xpb.DecorationsReply_Reference {
	span := norm.SpanOffsets(d.Anchor.StartOffset, d.Anchor.EndOffset)
	return &xpb.DecorationsReply_Reference{
		TargetTicket:     d.Target,
		Kind:             d.Kind,
		Span:             span,
		TargetDefinition: d.TargetDefinition,
		BuildConfig:      d.Anchor.BuildConfiguration,
		SemanticScope:    d.SemanticScope,
	}
}

type xrefCategory int

const (
	xrefCategoryNone xrefCategory = iota
	xrefCategoryDef
	xrefCategoryDecl
	xrefCategoryRef
	xrefCategoryCall
	xrefCategoryRelated
	xrefCategoryIndirection
)

func (c xrefCategory) AddCount(reply *xpb.CrossReferencesReply, idx *srvpb.PagedCrossReferences_PageIndex, pageSet *pageSet) {
	switch c {
	case xrefCategoryDef:
		if pageSet.Contains(idx) {
			reply.Total.Definitions += int64(idx.Count)
		} else {
			reply.Filtered.Definitions += int64(idx.Count)
		}
	case xrefCategoryDecl:
		if pageSet.Contains(idx) {
			reply.Total.Declarations += int64(idx.Count)
		} else {
			reply.Filtered.Declarations += int64(idx.Count)
		}
	case xrefCategoryRef:
		if pageSet.Contains(idx) {
			reply.Total.RefEdgeToCount[strings.TrimPrefix(idx.Kind, "%")] += int64(idx.Count)
			reply.Total.References += int64(idx.Count)
		} else {
			reply.Filtered.RefEdgeToCount[strings.TrimPrefix(idx.Kind, "%")] += int64(idx.Count)
			reply.Filtered.References += int64(idx.Count)
		}
	case xrefCategoryRelated:
		if pageSet.Contains(idx) {
			reply.Total.RelatedNodesByRelation[idx.Kind] += int64(idx.Count)
		} else {
			reply.Filtered.RelatedNodesByRelation[idx.Kind] += int64(idx.Count)
		}
	case xrefCategoryCall:
		if pageSet.Contains(idx) {
			reply.Total.Callers += int64(idx.Count)
		} else {
			reply.Filtered.Callers += int64(idx.Count)
		}
	}
}

// CrossReferences implements part of the xrefs.Service interface.
func (t *Table) CrossReferences(ctx context.Context, req *xpb.CrossReferencesRequest) (*xpb.CrossReferencesReply, error) {
	tickets, err := xrefs.FixTickets(req.Ticket)
	if err != nil {
		return nil, err
	}

	var leewayTime time.Time
	if d, ok := ctx.Deadline(); ok && *responseLeewayTime > 0 {
		leewayTime = d.Add(-*responseLeewayTime)
		if leewayTime.Before(time.Now()) {
			// Clear leeway time; try to use entire leftover timeout.
			leewayTime = time.Time{}
		}
	}

	filter, err := compileCorpusPathFilters(req.GetCorpusPathFilters(), t.ResolvePath)
	if err != nil {
		return nil, status.Errorf(codes.InvalidArgument, "invalid corpus_path_filters %s: %v", strings.ReplaceAll(req.GetCorpusPathFilters().String(), "\n", " "), err)
	}

	pageReadGroupCtx, stopReadingPages := context.WithCancel(ctx)
	defer stopReadingPages()
	pageReadGroup, pageReadGroupCtx := errgroup.WithContext(pageReadGroupCtx)
	pageReadGroup.SetLimit(int(*pageReadAhead) + 1)
	single := new(syncCache[*srvpb.PagedCrossReferences_Page])

	getCachedPage := func(ctx context.Context, pageKey string) (*srvpb.PagedCrossReferences_Page, error) {
		return single.Get(pageKey, func() (*srvpb.PagedCrossReferences_Page, error) {
			return t.crossReferencesPage(ctx, pageKey)
		})
	}
	getFilteredPage := func(ctx context.Context, pageKey string) (*srvpb.PagedCrossReferences_Page, int, error) {
		p, err := getCachedPage(ctx, pageKey)
		if err != nil {
			return nil, 0, err
		}
		// Clear page from cache; it should only be used once.
		single.Delete(pageKey)
		return p, filter.FilterGroup(p.GetGroup()), nil
	}

	stats := refStats{
		max: int(req.PageSize),

		refOptions: refOptions{
			anchorText:    req.AnchorText,
			includeScopes: req.SemanticScopes,
		},
	}
	if stats.max < 0 {
		return nil, status.Errorf(codes.InvalidArgument, "invalid page_size: %d", req.PageSize)
	} else if stats.max == 0 {
		stats.max = defaultPageSize
	} else if stats.max > maxPageSize {
		stats.max = maxPageSize
	}

	var pageToken ipb.PageToken
	if req.PageToken != "" {
		rec, err := base64.StdEncoding.DecodeString(req.PageToken)
		if err != nil {
			return nil, status.Errorf(codes.InvalidArgument, "invalid page_token: %q", req.PageToken)
		}
		rec, err = snappy.Decode(nil, rec)
		if err != nil {
			return nil, status.Errorf(codes.InvalidArgument, "invalid page_token: %q", req.PageToken)
		}
		if err := proto.Unmarshal(rec, &pageToken); err != nil {
			return nil, status.Errorf(codes.InvalidArgument, "invalid page_token: %q", req.PageToken)
		}
		for _, index := range pageToken.Indices {
			if index < 0 {
				return nil, status.Errorf(codes.InvalidArgument, "invalid page_token: %q", req.PageToken)
			}
		}
	}
	initialSkip := int(pageToken.Indices["skip"])
	stats.skip = initialSkip

	reply := &xpb.CrossReferencesReply{
		CrossReferences: make(map[string]*xpb.CrossReferencesReply_CrossReferenceSet, len(req.Ticket)),
		Nodes:           make(map[string]*cpb.NodeInfo, len(req.Ticket)),

		Total: &xpb.CrossReferencesReply_Total{
			RefEdgeToCount: make(map[string]int64),
		},
		Filtered: &xpb.CrossReferencesReply_Total{
			RefEdgeToCount:         make(map[string]int64),
			RelatedNodesByRelation: make(map[string]int64),
		},
	}
	// Before we return reply, remove all RefEdgeToCount map entries that point to a 0 count.
	defer cleanupRefEdgeToCount(reply)

	if len(req.Filter) > 0 {
		reply.Total.RelatedNodesByRelation = make(map[string]int64)
	}
	if req.NodeDefinitions {
		reply.DefinitionLocations = make(map[string]*xpb.Anchor)
	}
	stats.reply = reply

	buildConfigs := stringset.New(req.BuildConfig...)
	patterns := xrefs.ConvertFilters(req.Filter)
	stats.nodeConverter = nodeConverter{patterns}

	nextPageToken := &ipb.PageToken{
		SubTokens: make(map[string]string),
		Indices:   make(map[string]int32),
	}

	mergeInto := make(map[string]string)
	for _, ticket := range tickets {
		mergeInto[ticket] = ticket
	}

	relatedKinds := stringset.New(req.RelatedNodeKind...)

	wantMoreCrossRefs := (req.DefinitionKind != xpb.CrossReferencesRequest_NO_DEFINITIONS ||
		req.DeclarationKind != xpb.CrossReferencesRequest_NO_DECLARATIONS ||
		req.ReferenceKind != xpb.CrossReferencesRequest_NO_REFERENCES ||
		req.CallerKind != xpb.CrossReferencesRequest_NO_CALLERS ||
		len(req.Filter) > 0)

	totalsQuality := req.TotalsQuality
	if totalsQuality == xpb.CrossReferencesRequest_UNSPECIFIED_TOTALS {
		totalsQuality = xpb.CrossReferencesRequest_TotalsQuality(xpb.CrossReferencesRequest_TotalsQuality_value[strings.ToUpper(*defaultTotalsQuality)])
	}

	var patcher MultiFilePatcher
	if t.MakePatcher != nil && req.GetWorkspace() != nil && req.GetPatchAgainstWorkspace() {
		patcher, err = t.MakePatcher(ctx, req.GetWorkspace())
		if isNonContextError(err) {
			log.Errorf("creating patcher: %v", err)
		}

		if patcher != nil {
			defer func() {
				if err := patcher.Close(); isNonContextError(err) {
					// No need to fail the request; just log the error.
					log.Errorf("closing patcher: %v", err)
				}
			}()

			stats.refOptions.patcherFunc = func(f *srvpb.FileInfo) {
				if err := patcher.AddFile(ctx, f); isNonContextError(err) {
					// Attempt to continue with the request, just log the error.
					log.Errorf("adding file: %v", err)
				}
			}
		}
	}

	// Set of xref page keys to read for further indirection nodes.
	var indirectionPages []string

	var foundCrossRefs bool
readLoop:
	for i := 0; i < len(tickets); i++ {
		if totalsQuality == xpb.CrossReferencesRequest_APPROXIMATE_TOTALS && stats.done() {
			break
		}

		if !leewayTime.IsZero() && time.Now().After(leewayTime) {
			log.Warning("hit soft deadline; trying to return already read xrefs")
			break
		}

		ticket := tickets[i]
		cr, err := t.crossReferences(ctx, ticket)
		if err == table.ErrNoSuchKey {
			continue
		} else if err != nil {
			return nil, canonicalError(err, "cross-references", ticket)
		}
		foundCrossRefs = true

		// If this node is to be merged into another, we will use that node's ticket
		// for all further book-keeping purposes.
		ticket = mergeInto[ticket]

		// We may have partially completed the xrefs set due merge nodes.
		crs := reply.CrossReferences[ticket]
		if crs == nil {
			crs = &xpb.CrossReferencesReply_CrossReferenceSet{
				Ticket: ticket,
			}
			reply.CrossReferences[ticket] = crs

			// If visiting a non-merge node and facts are requested, add them to the result.
			if ticket == cr.SourceTicket && len(patterns) > 0 && cr.SourceNode != nil {
				if _, ok := reply.Nodes[ticket]; !ok {
					if info := stats.ToInfo(cr.SourceNode); info != nil {
						reply.Nodes[ticket] = info
					}
				}
			}
		}
		if crs.MarkedSource == nil {
			crs.MarkedSource = cr.MarkedSource
		}

		if *mergeCrossReferences {
			// Add any additional merge nodes to the set of table lookups
			for _, mergeNode := range cr.MergeWith {
				tickets = addMergeNode(mergeInto, tickets, ticket, mergeNode)
			}
		}

		// Read the set of indirection edge kinds for the given node kind.
		nodeKind := nodeKind(cr.SourceNode)
		indirections := experimentalCrossReferenceIndirectionKinds[nodeKind].
			Union(experimentalCrossReferenceIndirectionKinds["*"])

		for _, grp := range cr.Group {
			// Filter anchor groups based on requested build configs
			if len(buildConfigs) != 0 && !buildConfigs.Contains(grp.BuildConfig) && !xrefs.IsRelatedNodeKind(relatedKinds, grp.Kind) {
				continue
			}

			switch {
			case xrefs.IsDefKind(req.DefinitionKind, grp.Kind, cr.Incomplete):
				filtered := filter.FilterGroup(grp)
				reply.Total.Definitions += int64(len(grp.Anchor))
				reply.Total.Definitions += int64(countRefs(grp.GetScopedReference()))
				reply.Filtered.Definitions += int64(filtered)
				if wantMoreCrossRefs {
					stats.addAnchors(&crs.Definition, grp)
				}
			case xrefs.IsDeclKind(req.DeclarationKind, grp.Kind, cr.Incomplete):
				filtered := filter.FilterGroup(grp)
				reply.Total.Declarations += int64(len(grp.Anchor))
				reply.Total.Declarations += int64(countRefs(grp.GetScopedReference()))
				reply.Filtered.Declarations += int64(filtered)
				if wantMoreCrossRefs {
					stats.addAnchors(&crs.Declaration, grp)
				}
			case xrefs.IsRefKind(req.ReferenceKind, grp.Kind):
				filtered := filter.FilterGroup(grp)
				reply.Total.RefEdgeToCount[strings.TrimPrefix(grp.Kind, "%")] += int64(len(grp.Anchor))
				reply.Total.References += int64(len(grp.Anchor))
				reply.Total.RefEdgeToCount[strings.TrimPrefix(grp.Kind, "%")] += int64(countRefs(grp.GetScopedReference()))
				reply.Total.References += int64(countRefs(grp.GetScopedReference()))
				reply.Filtered.RefEdgeToCount[strings.TrimPrefix(grp.Kind, "%")] += int64(filtered)
				reply.Filtered.References += int64(filtered)
				if wantMoreCrossRefs {
					stats.addAnchors(&crs.Reference, grp)
				}
			case len(grp.RelatedNode) > 0:
				// If requested, add related nodes to merge node set.
				if indirections.Contains(grp.Kind) {
					for _, rn := range grp.RelatedNode {
						tickets = addMergeNode(mergeInto, tickets, ticket, rn.Node.GetTicket())
					}
				}

				if len(req.Filter) > 0 && xrefs.IsRelatedNodeKind(relatedKinds, grp.Kind) {
					filtered := filter.FilterGroup(grp)
					reply.Total.RelatedNodesByRelation[grp.Kind] += int64(len(grp.RelatedNode))
					reply.Filtered.RelatedNodesByRelation[grp.Kind] += int64(filtered)
					if wantMoreCrossRefs {
						stats.addRelatedNodes(crs, grp)
					}
				}
			case xrefs.IsCallerKind(req.CallerKind, grp.Kind):
				filtered := filter.FilterGroup(grp)
				reply.Total.Callers += int64(len(grp.Caller))
				reply.Filtered.Callers += int64(filtered)
				if wantMoreCrossRefs {
					stats.addCallers(crs, grp)
				}
			}
		}

		pageSet := filter.PageSet(cr)

		pageCategory := func(idx *srvpb.PagedCrossReferences_PageIndex) xrefCategory {
			// Filter anchor pages based on requested build configs
			if len(buildConfigs) != 0 && !buildConfigs.Contains(idx.BuildConfig) && !xrefs.IsRelatedNodeKind(relatedKinds, idx.Kind) {
				return xrefCategoryNone
			}

			switch {
			case xrefs.IsDefKind(req.DefinitionKind, idx.Kind, cr.Incomplete):
				return xrefCategoryDef
			case xrefs.IsDeclKind(req.DeclarationKind, idx.Kind, cr.Incomplete):
				return xrefCategoryDecl
			case xrefs.IsRefKind(req.ReferenceKind, idx.Kind):
				return xrefCategoryRef
			case len(req.Filter) > 0 && xrefs.IsRelatedNodeKind(relatedKinds, idx.Kind):
				return xrefCategoryRelated
			case indirections.Contains(idx.Kind):
				return xrefCategoryIndirection
			case xrefs.IsCallerKind(req.CallerKind, idx.Kind):
				return xrefCategoryCall
			default:
				return xrefCategoryNone
			}
		}

		// Find the first unskipped page index so proper read ahead.
		firstUnskippedPage := len(cr.GetPageIndex())
		for i, idx := range cr.GetPageIndex() {
			c := pageCategory(idx)
			if c == xrefCategoryNone {
				continue
			}

			if !stats.skipPage(idx) {
				firstUnskippedPage = i
				break
			}
			c.AddCount(reply, idx, pageSet)
		}

		// If enabled, start reading pages concurrently starting from the first
		// unskipped page.
		if *pageReadAhead > 0 {
			pageReadGroup.Go(func() error {
				ctx := pageReadGroupCtx
				for _, idx := range cr.GetPageIndex()[firstUnskippedPage:] {
					if err := ctx.Err(); err != nil {
						return err
					}
					if pageCategory(idx) == xrefCategoryNone || !pageSet.Contains(idx) {
						continue
					}

					idx := idx
					pageReadGroup.Go(func() error {
						_, err := getCachedPage(ctx, idx.PageKey)
						return err
					})
				}
				return nil
			})
		}

		for _, idx := range cr.GetPageIndex()[firstUnskippedPage:] {
			if !leewayTime.IsZero() && time.Now().After(leewayTime) {
				log.Warningf("hit soft deadline; trying to return already read xrefs: %s", time.Now().Sub(leewayTime))
				break readLoop
			}

			c := pageCategory(idx)
			if c == xrefCategoryNone {
				continue
			}
			if wantMoreCrossRefs && !stats.skipPage(idx) {
				c.AddCount(reply, idx, pageSet)
			}
			if c != xrefCategoryIndirection && c != xrefCategoryRelated && !pageSet.Contains(idx) {
				continue
			}

			switch c {
			case xrefCategoryDef:
				if wantMoreCrossRefs && !stats.skipPage(idx) {
					p, filtered, err := getFilteredPage(ctx, idx.PageKey)
					if err != nil {
						return nil, fmt.Errorf("internal error: error retrieving cross-references page %v: %v", idx.PageKey, err)
					}
					reply.Total.Definitions -= int64(filtered) // update counts to reflect filtering
					reply.Filtered.Definitions += int64(filtered)
					stats.addAnchors(&crs.Definition, p.Group)
				}
			case xrefCategoryDecl:
				if wantMoreCrossRefs && !stats.skipPage(idx) {
					p, filtered, err := getFilteredPage(ctx, idx.PageKey)
					if err != nil {
						return nil, fmt.Errorf("internal error: error retrieving cross-references page %v: %v", idx.PageKey, err)
					}
					reply.Total.Declarations -= int64(filtered) // update counts to reflect filtering
					reply.Filtered.Declarations += int64(filtered)
					stats.addAnchors(&crs.Declaration, p.Group)
				}
			case xrefCategoryRef:
				if wantMoreCrossRefs && !stats.skipPage(idx) {
					p, filtered, err := getFilteredPage(ctx, idx.PageKey)
					if err != nil {
						return nil, fmt.Errorf("internal error: error retrieving cross-references page %v: %v", idx.PageKey, err)
					}
					reply.Total.RefEdgeToCount[strings.TrimPrefix(idx.Kind, "%")] -= int64(filtered) // update counts to reflect filtering
					reply.Total.References -= int64(filtered)                                        // update counts to reflect filtering
					reply.Filtered.RefEdgeToCount[strings.TrimPrefix(idx.Kind, "%")] += int64(filtered)
					reply.Filtered.References += int64(filtered)
					stats.addAnchors(&crs.Reference, p.Group)
				}
			case xrefCategoryRelated, xrefCategoryIndirection:
				var p *srvpb.PagedCrossReferences_Page

				if len(req.Filter) > 0 && xrefs.IsRelatedNodeKind(relatedKinds, idx.Kind) {
					if pageSet.Contains(idx) {
						if wantMoreCrossRefs && !stats.skipPage(idx) {
							var filtered int
							p, filtered, err = getFilteredPage(ctx, idx.PageKey)
							if err != nil {
								return nil, fmt.Errorf("internal error: error retrieving cross-references page: %v", idx.PageKey)
							}
							reply.Total.RelatedNodesByRelation[idx.Kind] -= int64(filtered) // update counts to reflect filtering
							reply.Filtered.RelatedNodesByRelation[idx.Kind] += int64(filtered)
							stats.addRelatedNodes(crs, p.Group)
						}
					}
				}

				// If requested, add related nodes to merge node set.
				if indirections.Contains(idx.Kind) {
					if p == nil {
						// We haven't needed to read the page yet; save it until we need
						// more tickets.
						indirectionPages = append(indirectionPages, idx.PageKey)
					} else {
						// We've already read the page, immediately populate the indirect
						// nodes.
						for _, rn := range p.Group.RelatedNode {
							tickets = addMergeNode(mergeInto, tickets, ticket, rn.Node.GetTicket())
						}
					}
				}
			case xrefCategoryCall:
				if wantMoreCrossRefs && !stats.skipPage(idx) {
					p, filtered, err := getFilteredPage(ctx, idx.PageKey)
					if err != nil {
						return nil, fmt.Errorf("internal error: error retrieving cross-references page: %v", idx.PageKey)
					}
					reply.Total.Callers -= int64(filtered) // update counts to reflect filtering
					reply.Filtered.Callers += int64(filtered)
					stats.addCallers(crs, p.Group)
				}
			}
		}

		for i == len(tickets)-1 && len(indirectionPages) > 0 {
			// We've hit the end of known tickets to pull for xrefs; read an
			// indirection page until we've found another ticket or we've exhausted
			// all indirection pages.
			pageKey := indirectionPages[len(indirectionPages)-1]
			indirectionPages = indirectionPages[:len(indirectionPages)-1]
			p, err := t.crossReferencesPage(ctx, pageKey)
			if err != nil {
				return nil, fmt.Errorf("internal error: error retrieving cross-references page: %v", pageKey)
			}
			for _, rn := range p.Group.RelatedNode {
				tickets = addMergeNode(mergeInto, tickets, ticket, rn.Node.GetTicket())
			}
		}

		tracePrintf(ctx, "CrossReferenceSet: %s", crs.Ticket)
	}

	stopReadingPages()
	go func() {
		if err := pageReadGroup.Wait(); isNonContextError(err) {
			log.Errorf("page read ahead error: %v", err)
		}
	}()

	if !foundCrossRefs {
		// Short-circuit return; skip any slow requests.
		return &xpb.CrossReferencesReply{}, nil
	}

	var emptySets []string
	for key, crs := range reply.CrossReferences {
		if len(crs.Declaration)+len(crs.Definition)+len(crs.Reference)+len(crs.Caller)+len(crs.RelatedNode) == 0 {
			emptySets = append(emptySets, key)
		}
	}
	for _, k := range emptySets {
		delete(reply.CrossReferences, k)
	}

	if initialSkip+stats.total != sumTotalCrossRefs(reply.Total) && stats.total != 0 {
		nextPageToken.Indices["skip"] = int32(initialSkip + stats.total)
	}

	if _, skip := nextPageToken.Indices["skip"]; skip {
		rec, err := proto.Marshal(nextPageToken)
		if err != nil {
			return nil, fmt.Errorf("internal error: error marshalling page token: %v", err)
		}
		reply.NextPageToken = base64.StdEncoding.EncodeToString(snappy.Encode(nil, rec))
	}

	if req.Snippets == xpb.SnippetsKind_NONE {
		for _, crs := range reply.CrossReferences {
			for _, def := range crs.Definition {
				clearRelatedSnippets(def)
			}
			for _, dec := range crs.Declaration {
				clearRelatedSnippets(dec)
			}
			for _, ref := range crs.Reference {
				clearRelatedSnippets(ref)
			}
			for _, ca := range crs.Caller {
				clearRelatedSnippets(ca)
			}
		}
		for _, def := range reply.DefinitionLocations {
			clearSnippet(def)
		}
	}

	if patcher != nil {
		tracePrintf(ctx, "Patching anchors")
		// Patch each set of anchors in parallel.  Files were added as they were
		// seen when populating the xref sets.
		g, gCtx := errgroup.WithContext(ctx)
		g.Go(func() error {
			defs, err := patchDefLocations(gCtx, patcher, reply.GetDefinitionLocations())
			if err != nil {
				return err
			}
			reply.DefinitionLocations = defs
			return nil
		})
		for _, set := range reply.GetCrossReferences() {
			g.Go(func() error {
				as, err := patcher.PatchRelatedAnchors(gCtx, set.GetDefinition())
				if err != nil {
					return err
				}
				set.Definition = as
				tracePrintf(ctx, "Patched Definitions: %d", len(as))
				return nil
			})

			g.Go(func() error {
				as, err := patcher.PatchRelatedAnchors(gCtx, set.GetDeclaration())
				if err != nil {
					return err
				}
				set.Declaration = as
				tracePrintf(ctx, "Patched Declarations: %d", len(as))
				return nil
			})

			g.Go(func() error {
				as, err := patcher.PatchRelatedAnchors(gCtx, set.GetReference())
				if err != nil {
					return err
				}
				set.Reference = as
				tracePrintf(ctx, "Patched References: %d", len(as))
				return nil
			})

			g.Go(func() error {
				as, err := patcher.PatchRelatedAnchors(gCtx, set.GetCaller())
				if err != nil {
					return err
				}
				set.Caller = as
				tracePrintf(ctx, "Patched Callers: %d", len(as))
				return nil
			})
		}
		if err := g.Wait(); err != nil {
			return nil, err
		}
	}

	return reply, nil
}

// cleanupRefEdgeToCount removes all the keys from r.Total.RefEdgeToCount and
// r.Filtered.RefEdgeToCount that have a value of 0.
func cleanupRefEdgeToCount(r *xpb.CrossReferencesReply) {
	for k, v := range r.Total.RefEdgeToCount {
		if v == 0 {
			delete(r.Total.RefEdgeToCount, k)
		}
	}
	for k, v := range r.Filtered.RefEdgeToCount {
		if v == 0 {
			delete(r.Filtered.RefEdgeToCount, k)
		}
	}

}

func addMergeNode(mergeMap map[string]string, allTickets []string, rootNode, mergeNode string) []string {
	if _, ok := mergeMap[mergeNode]; ok {
		return allTickets
	}
	allTickets = append(allTickets, mergeNode)
	mergeMap[mergeNode] = rootNode
	return allTickets
}

func countRefs(rs []*srvpb.PagedCrossReferences_ScopedReference) int {
	var n int
	for _, ref := range rs {
		n += len(ref.GetReference())
	}
	return n
}

func nodeKind(n *srvpb.Node) string {
	if n == nil {
		return ""
	}
	for _, f := range n.Fact {
		if f.Name == facts.NodeKind {
			return string(f.Value)
		}
	}
	return ""
}

func sumTotalCrossRefs(ts *xpb.CrossReferencesReply_Total) int {
	var refs int
	for _, cnt := range ts.RefEdgeToCount {
		refs += int(cnt)
	}
	var relatedNodes int
	for _, cnt := range ts.RelatedNodesByRelation {
		relatedNodes += int(cnt)
	}
	return int(ts.Callers) +
		int(ts.Definitions) +
		int(ts.Declarations) +
		refs +
		int(ts.Documentation) +
		relatedNodes
}

type refOptions struct {
	patcherFunc   patcherFunc
	anchorText    bool
	includeScopes bool
}

type refStats struct {
	// number of refs:
	//   to skip (returned on previous pages)
	//   max to return (the page size)
	//   total (count of refs so far read for current page)
	skip, total, max int

	reply *xpb.CrossReferencesReply
	refOptions
	nodeConverter
}

func (s *refStats) done() bool { return s.total == s.max }

func (s *refStats) skipPage(idx *srvpb.PagedCrossReferences_PageIndex) bool {
	if s.skip > int(idx.Count) {
		s.skip -= int(idx.Count)
		return true
	}
	return s.total >= s.max
}

func (s *refStats) addCallers(crs *xpb.CrossReferencesReply_CrossReferenceSet, grp *srvpb.PagedCrossReferences_Group) bool {
	cs := grp.Caller
	converter := &anchorConverter{
		fileInfos:   makeFileInfoMap(grp.FileInfo),
		patcherFunc: s.patcherFunc,
	}

	if s.done() {
		// We've already hit our cap; return true that we're done.
		return true
	} else if s.skip > len(cs) {
		// We can skip this entire group.
		s.skip -= len(cs)
		return false
	} else if s.skip > 0 {
		// Skip part of the group, and put the rest in the reply.
		cs = cs[s.skip:]
		s.skip = 0
	}

	if s.total+len(cs) > s.max {
		cs = cs[:(s.max - s.total)]
	}
	s.total += len(cs)
	for _, c := range cs {
		ra := &xpb.CrossReferencesReply_RelatedAnchor{
			Anchor: converter.Convert(c.Caller).Anchor,
			Ticket: c.SemanticCaller,
			Site:   make([]*xpb.Anchor, 0, len(c.Callsite)),
		}
		ra.MarkedSource = c.MarkedSource
		for _, site := range c.Callsite {
			ra.Site = append(ra.Site, converter.Convert(site).Anchor)
		}
		crs.Caller = append(crs.Caller, ra)
	}
	return s.done() // return whether we've hit our cap
}

func (s *refStats) addRelatedNodes(crs *xpb.CrossReferencesReply_CrossReferenceSet, grp *srvpb.PagedCrossReferences_Group) bool {
	ns := grp.RelatedNode
	nodes := s.reply.Nodes
	defs := s.reply.DefinitionLocations
	ac := &anchorConverter{
		fileInfos:   makeFileInfoMap(grp.FileInfo),
		patcherFunc: s.patcherFunc,
	}

	if s.total == s.max {
		// We've already hit our cap; return true that we're done.
		return true
	} else if s.skip > len(ns) {
		// We can skip this entire group.
		s.skip -= len(ns)
		return false
	} else if s.skip > 0 {
		// Skip part of the group, and put the rest in the reply.
		ns = ns[s.skip:]
		s.skip = 0
	}

	if s.total+len(ns) > s.max {
		ns = ns[:(s.max - s.total)]
	}
	s.total += len(ns)
	for _, rn := range ns {
		if _, ok := nodes[rn.Node.Ticket]; !ok {
			if info := s.ToInfo(rn.Node); info != nil {
				nodes[rn.Node.Ticket] = info
				if defs != nil && rn.Node.DefinitionLocation != nil {
					nodes[rn.Node.Ticket].Definition = rn.Node.DefinitionLocation.Ticket
					defs[rn.Node.DefinitionLocation.Ticket] = ac.Convert(rn.Node.DefinitionLocation).Anchor
				}
			}
		}
		crs.RelatedNode = append(crs.RelatedNode, &xpb.CrossReferencesReply_RelatedNode{
			RelationKind: grp.Kind,
			Ticket:       rn.Node.Ticket,
			Ordinal:      rn.Ordinal,
		})
	}
	return s.total == s.max // return whether we've hit our cap
}

func (s *refStats) addAnchors(to *[]*xpb.CrossReferencesReply_RelatedAnchor, grp *srvpb.PagedCrossReferences_Group) bool {
	if s.total >= s.max {
		return true
	}

	scopedRefs := grp.GetScopedReference()[:len(grp.GetScopedReference()):len(grp.GetScopedReference())]
	// Convert legacy unscoped references to simple ScopedReference containers
	for _, a := range grp.Anchor {
		scopedRefs = append(scopedRefs, &srvpb.PagedCrossReferences_ScopedReference{
			Reference: []*srvpb.ExpandedAnchor{a},
		})
	}
	totalRefs := countRefs(scopedRefs)

	if s.skip >= totalRefs {
		s.skip -= totalRefs
		return false
	}

	if s.skip > 0 {
		var firstNonEmpty int
		for i := 0; i < len(scopedRefs) && s.skip > 0; i++ {
			sr := scopedRefs[i]
			if len(sr.GetReference()) <= s.skip {
				s.skip -= len(sr.GetReference())
				firstNonEmpty++
				continue
			}
			sr.Reference = sr.GetReference()[s.skip:]
			s.skip = 0
		}
		scopedRefs = scopedRefs[firstNonEmpty:]
	}

	kind := edges.Canonical(grp.Kind)
	fileInfos := makeFileInfoMap(grp.FileInfo)
	c := &anchorConverter{fileInfos: fileInfos, anchorText: s.anchorText, patcherFunc: s.patcherFunc}
	for _, sr := range scopedRefs {
		if !s.includeScopes || sr.Scope == nil {
			for _, a := range sr.Reference {
				ra := c.Convert(a)
				ra.Anchor.Kind = kind
				*to = append(*to, ra)
				s.total++
				if s.total >= s.max {
					return true
				}
			}
			continue
		}
		scope := c.Convert(sr.Scope).Anchor
		scope.Kind = sr.Scope.Kind
		ra := &xpb.CrossReferencesReply_RelatedAnchor{
			Anchor: scope,
			Ticket: sr.SemanticScope,
			Site:   make([]*xpb.Anchor, 0, len(sr.Reference)),
		}
		ra.MarkedSource = sr.MarkedSource
		refs := sr.GetReference()
		if s.total+len(refs) > s.max {
			refs = refs[:(s.max - s.total)]
		}
		for _, site := range refs {
			a := c.Convert(site).Anchor
			a.Kind = kind
			ra.Site = append(ra.Site, a)
		}
		*to = append(*to, ra)
		s.total += len(ra.Site)
		if s.total >= s.max {
			return true
		}
	}
	return false
}

type patcherFunc func(f *srvpb.FileInfo)

type anchorConverter struct {
	fileInfos   map[string]*srvpb.FileInfo
	anchorText  bool
	patcherFunc patcherFunc
}

func (c *anchorConverter) Convert(a *srvpb.ExpandedAnchor) *xpb.CrossReferencesReply_RelatedAnchor {
	var text string
	if c.anchorText {
		text = a.Text
	}
	parent, err := tickets.AnchorFile(a.Ticket)
	if err != nil {
		log.Errorf("parsing anchor ticket: %v", err)
	}
	fileInfo := a.GetFileInfo()
	if fileInfo == nil {
		fileInfo = c.fileInfos[parent]
	}
	if c.patcherFunc != nil {
		c.patcherFunc(fileInfo)
	}
	return &xpb.CrossReferencesReply_RelatedAnchor{Anchor: &xpb.Anchor{
		Ticket:      a.Ticket,
		Kind:        edges.Canonical(a.Kind),
		Parent:      parent,
		Text:        text,
		Span:        a.Span,
		Snippet:     a.Snippet,
		SnippetSpan: a.SnippetSpan,
		BuildConfig: a.BuildConfiguration,
		Revision:    fileInfo.GetRevision(),
	}}
}

type documentConverter struct {
	anchorConverter
	nodeConverter

	nodes map[string]*cpb.NodeInfo
	defs  map[string]*xpb.Anchor
}

func (c *documentConverter) Convert(d *srvpb.Document) *xpb.DocumentationReply_Document {
	for _, node := range d.Node {
		if _, ok := c.nodes[node.Ticket]; ok {
			continue
		}

		n := c.ToInfo(node)
		if def := node.DefinitionLocation; def != nil {
			if n == nil {
				// Add an empty NodeInfo to attach definition location even if no facts
				// are requested.
				n = &cpb.NodeInfo{}
			}

			n.Definition = def.Ticket
			if _, ok := c.defs[def.Ticket]; !ok {
				c.defs[def.Ticket] = c.anchorConverter.Convert(def).Anchor
			}
		}

		if n != nil {
			c.nodes[node.Ticket] = n
		}
	}

	return &xpb.DocumentationReply_Document{
		Ticket: d.Ticket,
		Text: &xpb.Printable{
			RawText: d.RawText,
			Link:    d.Link,
		},
		MarkedSource: d.MarkedSource,
	}
}

func (t *Table) lookupDocument(ctx context.Context, ticket string) (*srvpb.Document, error) {
	d, err := t.documentation(ctx, ticket)
	if err != nil {
		return nil, err
	}
	tracePrintf(ctx, "Document: %s", ticket)

	// If DocumentedBy is provided, replace document with another lookup.
	if d.DocumentedBy != "" {
		doc, err := t.documentation(ctx, d.DocumentedBy)
		if err != nil {
			log.Errorf("looking up subsuming documentation for {%+v}: %v", d, err)
			return nil, err
		}

		// Ensure the subsuming documentation has the correct ticket and node.
		doc.Ticket = ticket
		for _, n := range d.Node {
			if n.Ticket == ticket {
				doc.Node = append(doc.Node, n)
				break
			}
		}

		tracePrintf(ctx, "DocumentedBy: %s", d.DocumentedBy)
		d = doc
	}
	return d, nil
}

// Documentation implements part of the xrefs Service interface.
func (t *Table) Documentation(ctx context.Context, req *xpb.DocumentationRequest) (*xpb.DocumentationReply, error) {
	tickets, err := xrefs.FixTickets(req.Ticket)
	if err != nil {
		return nil, err
	}

	reply := &xpb.DocumentationReply{
		Nodes:               make(map[string]*cpb.NodeInfo, len(tickets)),
		DefinitionLocations: make(map[string]*xpb.Anchor, len(tickets)),
	}
	patterns := xrefs.ConvertFilters(req.Filter)
	if len(patterns) == 0 {
		// Match all facts if given no filters
		patterns = xrefs.ConvertFilters([]string{"**"})
	}
	fileInfos := make(map[string]*srvpb.FileInfo)

	dc := &documentConverter{
		anchorConverter: anchorConverter{fileInfos: fileInfos},
		nodeConverter:   nodeConverter{patterns},
		nodes:           reply.Nodes,
		defs:            reply.DefinitionLocations,
	}

	var patcher MultiFilePatcher
	if t.MakePatcher != nil && req.GetWorkspace() != nil && req.GetPatchAgainstWorkspace() {
		patcher, err = t.MakePatcher(ctx, req.GetWorkspace())
		if isNonContextError(err) {
			log.Errorf("creating patcher: %v", err)
		}

		if patcher != nil {
			defer func() {
				if err := patcher.Close(); isNonContextError(err) {
					// No need to fail the request; just log the error.
					log.Errorf("closing patcher: %v", err)
				}
			}()

			dc.anchorConverter.patcherFunc = func(f *srvpb.FileInfo) {
				if err := patcher.AddFile(ctx, f); isNonContextError(err) {
					// Attempt to continue with the request, just log the error.
					log.Errorf("adding file: %v", err)
				}
			}
		}
	}

	for _, ticket := range tickets {
		d, err := t.lookupDocument(ctx, ticket)
		if err == table.ErrNoSuchKey {
			continue
		} else if err != nil {
			return nil, canonicalError(err, "documentation", ticket)
		}

		doc := dc.Convert(d)
		if req.IncludeChildren {
			for _, child := range d.ChildTicket {
				// TODO(schroederc): store children with root of documentation tree
				cd, err := t.lookupDocument(ctx, child)
				if err == table.ErrNoSuchKey {
					continue
				} else if err != nil {
					return nil, canonicalError(err, "documentation child", ticket)
				}

				doc.Children = append(doc.Children, dc.Convert(cd))
			}
			tracePrintf(ctx, "Children: %d", len(d.ChildTicket))
		}

		reply.Document = append(reply.Document, doc)
	}
	tracePrintf(ctx, "Documents: %d (nodes: %d) (defs: %d", len(reply.Document), len(reply.Nodes), len(reply.DefinitionLocations))

	if patcher != nil {
		defs, err := patchDefLocations(ctx, patcher, reply.GetDefinitionLocations())
		if err != nil {
			log.Errorf("patching definition locations: %v", err)
		} else {
			reply.DefinitionLocations = defs
		}
	}

	return reply, nil
}

func clearRelatedSnippets(ra *xpb.CrossReferencesReply_RelatedAnchor) {
	clearSnippet(ra.Anchor)
	for _, site := range ra.Site {
		clearSnippet(site)
	}
}

func clearSnippet(anchor *xpb.Anchor) {
	anchor.Snippet = ""
	anchor.SnippetSpan = nil
}

func tracePrintf(ctx context.Context, msg string, args ...any) {
	if t, ok := trace.FromContext(ctx); ok {
		t.LazyPrintf(msg, args...)
	}
}

// Wrap known error types with corresponding rpc status errors.
// If no sensible parsing can be found, just return fmt.Errorf with some hints
// about the calling code and ticket.
// Follows util::error::Code from
// http://google.github.io/google-api-cpp-client/latest/doxygen/namespacegoogleapis_1_1util_1_1error.html
func canonicalError(err error, caller string, ticket string) error {
	switch code := status.Code(err); code {
	case codes.OK:
		return nil
	case codes.Canceled:
		return xrefs.ErrCanceled
	case codes.DeadlineExceeded:
		return xrefs.ErrDeadlineExceeded
	default:
		st := err.Error()
		if strings.Contains(st, "RPC::CANCELLED") || strings.Contains(st, "context canceled") {
			return xrefs.ErrCanceled
		}
		if strings.Contains(st, "RPC::DEADLINE_EXCEEDED") || strings.Contains(st, "context deadline exceeded") {
			return xrefs.ErrDeadlineExceeded
		}
		return status.Error(code, st)
	}
}

func isNonContextError(err error) bool {
	err = canonicalError(err, "", "")
	return err != nil && err != xrefs.ErrCanceled && err != xrefs.ErrDeadlineExceeded
}

// call is an in-flight or completed Get call
type call[T any] struct {
	wg  sync.WaitGroup
	val T
	err error
}

type syncCache[T any] struct {
	mu sync.Mutex
	m  map[string]*call[T]
}

// Get executes and returns the results of the given function, making sure that
// there is only one execution for a given key (until Delete is called). If a
// duplicate comes in, the duplicate caller waits for the original to complete
// and receives the same results.
func (g *syncCache[T]) Get(key string, fn func() (T, error)) (T, error) {
	g.mu.Lock()
	if g.m == nil {
		g.m = make(map[string]*call[T])
	}
	if c, ok := g.m[key]; ok {
		g.mu.Unlock()
		c.wg.Wait()
		return c.val, c.err
	}
	c := new(call[T])
	c.wg.Add(1)
	g.m[key] = c
	g.mu.Unlock()

	c.val, c.err = fn()
	c.wg.Done()

	return c.val, c.err
}

// Delete removes the given key from the cache.
func (g *syncCache[T]) Delete(key string) {
	g.mu.Lock()
	delete(g.m, key)
	g.mu.Unlock()
}
