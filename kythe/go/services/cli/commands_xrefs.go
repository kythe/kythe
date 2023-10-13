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

	"kythe.io/kythe/go/util/flagutil"
	"kythe.io/kythe/go/util/kytheuri"
	"kythe.io/kythe/go/util/log"
	"kythe.io/kythe/go/util/markedsource"
	"kythe.io/kythe/go/util/schema/edges"
	"kythe.io/kythe/go/util/schema/facts"

	cpb "kythe.io/kythe/proto/common_go_proto"
	xpb "kythe.io/kythe/proto/xref_go_proto"
)

type xrefsCommand struct {
	baseKytheCommand
	nodeFilters  flagutil.StringList
	buildConfigs flagutil.StringSet

	workspaceURI string

	pageToken string
	pageSize  int

	defKind    string
	declKind   string
	refKind    string
	callerKind string

	semanticScopes  bool
	relatedNodes    bool
	nodeDefinitions bool
	anchorText      bool

	resolvedPathFilters flagutil.StringList

	excludeGenerated bool

	totalsOnly bool
}

func (xrefsCommand) Name() string     { return "xrefs" }
func (xrefsCommand) Synopsis() string { return "retrieve cross-references for the given node" }
func (xrefsCommand) Usage() string    { return "" }
func (c *xrefsCommand) SetFlags(flag *flag.FlagSet) {
	flag.StringVar(&c.defKind, "definitions", "binding", "Kind of definitions to return (kinds: all, binding, full, or none)")
	flag.StringVar(&c.declKind, "declarations", "all", "Kind of declarations to return (kinds: all or none)")
	flag.StringVar(&c.refKind, "references", "noncall", "Kind of references to return (kinds: all, noncall, call, or none)")
	flag.StringVar(&c.callerKind, "callers", "direct", "Kind of callers to return (kinds: direct, overrides, or none)")
	flag.StringVar(&c.workspaceURI, "workspace_uri", "", "Workspace URI to patch cross-references")
	flag.BoolVar(&c.relatedNodes, "related_nodes", true, "Whether to request related nodes")
	flag.Var(&c.nodeFilters, "filters", "CSV list of additional fact filters to use when requesting related nodes")
	flag.Var(&c.resolvedPathFilters, "resolved_path_filters", "CSV list of additional resolved path filters to use")
	flag.Var(&c.buildConfigs, "build_config", "CSV set of build configs with which to filter file decorations")
	flag.BoolVar(&c.nodeDefinitions, "node_definitions", false, "Whether to request definition locations for related nodes")
	flag.BoolVar(&c.anchorText, "anchor_text", false, "Whether to request text for anchors")
	flag.BoolVar(&c.semanticScopes, "semantic_scopes", false, "Whether to include semantic scopes")
	flag.BoolVar(&c.excludeGenerated, "exclude_generated", false, "Whether to exclude anchors with non-empty roots")

	flag.BoolVar(&c.totalsOnly, "totals_only", false, "Only output total count of xrefs")

	flag.StringVar(&c.pageToken, "page_token", "", "CrossReferences page token")
	flag.IntVar(&c.pageSize, "page_size", 0, "Maximum number of cross-references returned (0 lets the service use a sensible default)")
}
func (c xrefsCommand) Run(ctx context.Context, flag *flag.FlagSet, api API) error {
	req := &xpb.CrossReferencesRequest{
		Ticket:        flag.Args(),
		PageToken:     c.pageToken,
		PageSize:      int32(c.pageSize),
		Snippets:      xpb.SnippetsKind_DEFAULT,
		TotalsQuality: xpb.CrossReferencesRequest_APPROXIMATE_TOTALS,

		SemanticScopes:  c.semanticScopes,
		AnchorText:      c.anchorText,
		NodeDefinitions: c.nodeDefinitions,

		CorpusPathFilters: &xpb.CorpusPathFilters{},
	}
	if c.workspaceURI != "" {
		req.Workspace = &xpb.Workspace{Uri: c.workspaceURI}
		req.PatchAgainstWorkspace = true
	}
	if c.excludeGenerated {
		req.CorpusPathFilters.Filter = append(req.CorpusPathFilters.Filter, &xpb.CorpusPathFilter{
			Type: xpb.CorpusPathFilter_EXCLUDE,
			Root: ".+",
		})
	}
	for _, f := range c.resolvedPathFilters {
		req.CorpusPathFilters.Filter = append(req.CorpusPathFilters.Filter, &xpb.CorpusPathFilter{
			Type:         xpb.CorpusPathFilter_INCLUDE_ONLY,
			ResolvedPath: f,
		})
	}
	if c.relatedNodes {
		req.Filter = []string{facts.NodeKind, facts.Subkind}
		if len(c.nodeFilters) > 0 && (len(c.nodeFilters) != 1 || c.nodeFilters[0] != "") {
			req.Filter = append(req.Filter, c.nodeFilters...)
		}
	}
	req.BuildConfig = c.buildConfigs.Elements()
	switch c.defKind {
	case "all":
		req.DefinitionKind = xpb.CrossReferencesRequest_ALL_DEFINITIONS
	case "none":
		req.DefinitionKind = xpb.CrossReferencesRequest_NO_DEFINITIONS
	case "binding":
		req.DefinitionKind = xpb.CrossReferencesRequest_BINDING_DEFINITIONS
	case "full":
		req.DefinitionKind = xpb.CrossReferencesRequest_FULL_DEFINITIONS
	default:
		return fmt.Errorf("unknown definition kind: %q", c.defKind)
	}
	switch c.declKind {
	case "all":
		req.DeclarationKind = xpb.CrossReferencesRequest_ALL_DECLARATIONS
	case "none":
		req.DeclarationKind = xpb.CrossReferencesRequest_NO_DECLARATIONS
	default:
		return fmt.Errorf("unknown declaration kind: %q", c.declKind)
	}
	switch c.refKind {
	case "all":
		req.ReferenceKind = xpb.CrossReferencesRequest_ALL_REFERENCES
	case "noncall":
		req.ReferenceKind = xpb.CrossReferencesRequest_NON_CALL_REFERENCES
	case "call":
		req.ReferenceKind = xpb.CrossReferencesRequest_CALL_REFERENCES
	case "none":
		req.ReferenceKind = xpb.CrossReferencesRequest_NO_REFERENCES
	default:
		return fmt.Errorf("unknown reference kind: %q", c.refKind)
	}
	switch c.callerKind {
	case "direct":
		req.CallerKind = xpb.CrossReferencesRequest_DIRECT_CALLERS
	case "overrides":
		req.CallerKind = xpb.CrossReferencesRequest_OVERRIDE_CALLERS
	case "none":
		req.CallerKind = xpb.CrossReferencesRequest_NO_CALLERS
	default:
		return fmt.Errorf("unknown caller kind: %q", c.callerKind)
	}
	LogRequest(req)
	reply, err := api.XRefService.CrossReferences(ctx, req)
	if err != nil {
		return err
	}
	if reply.NextPageToken != "" {
		defer log.Infof("Next page token: %s", reply.NextPageToken)
	}
	return c.displayXRefs(reply)
}

func (c xrefsCommand) displayXRefs(reply *xpb.CrossReferencesReply) error {
	if DisplayJSON {
		return PrintJSONMessage(reply)
	}

	fmt.Fprintf(out, "Totals:\n%s\n\n", reply.GetTotal())

	if c.totalsOnly {
		return nil
	}

	for _, xr := range reply.CrossReferences {
		var sig string
		if xr.MarkedSource != nil {
			sig = showSignature(xr.MarkedSource) + " "
		}
		if _, err := fmt.Fprintf(out, "Cross-References for %s%s\n", sig, xr.Ticket); err != nil {
			return err
		}
		if err := displayRelatedAnchors("Definitions", xr.Definition); err != nil {
			return err
		}
		if err := displayRelatedAnchors("Declarations", xr.Declaration); err != nil {
			return err
		}
		if err := displayRelatedAnchors("References", xr.Reference); err != nil {
			return err
		}
		if err := displayRelatedAnchors("Callers", xr.Caller); err != nil {
			return err
		}
		if len(xr.RelatedNode) > 0 {
			if _, err := fmt.Fprintln(out, "  Related Nodes:"); err != nil {
				return err
			}
			for _, n := range xr.RelatedNode {
				var nodeKind, subkind string
				if node, ok := reply.Nodes[n.Ticket]; ok {
					for name, value := range node.Facts {
						switch name {
						case facts.NodeKind:
							nodeKind = string(value)
						case facts.Subkind:
							subkind = string(value)
						}
					}
				}
				if nodeKind == "" {
					nodeKind = "UNKNOWN"
				} else if subkind != "" {
					nodeKind += "/" + subkind
				}
				var ordinal string
				if edges.OrdinalKind(n.RelationKind) || n.Ordinal != 0 {
					ordinal = fmt.Sprintf(".%d", n.Ordinal)
				}
				if _, err := fmt.Fprintf(out, "    %s %s%s [%s]\n", n.Ticket, n.RelationKind, ordinal, nodeKind); err != nil {
					return err
				}
			}
		}
	}

	return nil
}

func displayRelatedAnchors(kind string, anchors []*xpb.CrossReferencesReply_RelatedAnchor) error {
	if len(anchors) == 0 {
		return nil
	}

	if _, err := fmt.Fprintf(out, "  %s:\n", kind); err != nil {
		return err
	}

	for _, a := range anchors {
		pURI, err := kytheuri.Parse(a.Anchor.Parent)
		if err != nil {
			return err
		}
		if _, err := fmt.Fprintf(out, "    %s\t", pURI.Path); err != nil {
			return err
		}
		if a.MarkedSource != nil {
			if _, err := fmt.Fprintf(out, "%s\t", showSignature(a.MarkedSource)); err != nil {
				return err
			}
		}
		if _, err := fmt.Fprintf(out, "    [%d:%d-%d:%d %s)\n      %q\n",
			a.GetAnchor().GetSpan().GetStart().GetLineNumber(), a.GetAnchor().GetSpan().GetStart().GetColumnOffset(),
			a.GetAnchor().GetSpan().GetEnd().GetLineNumber(), a.GetAnchor().GetSpan().GetEnd().GetColumnOffset(),
			a.GetAnchor().GetKind(), string(a.GetAnchor().GetSnippet())); err != nil {
			return err
		}
		for _, site := range a.Site {
			if _, err := fmt.Fprintf(out, "      [%d:%d-%d-%d %s)\n        %q\n",
				site.GetSpan().GetStart().GetLineNumber(), site.GetSpan().GetStart().GetColumnOffset(),
				site.GetSpan().GetEnd().GetLineNumber(), site.GetSpan().GetEnd().GetColumnOffset(),
				site.GetKind(), string(site.GetSnippet())); err != nil {
				return err
			}
		}
	}

	return nil
}

func showSignature(signature *cpb.MarkedSource) string {
	if signature == nil {
		return "(nil)"
	}
	ident := markedsource.RenderSimpleIdentifier(signature, markedsource.PlaintextContent, nil)
	params := markedsource.RenderSimpleParams(signature, markedsource.PlaintextContent, nil)
	if len(params) == 0 {
		return ident
	}
	return fmt.Sprintf("%s(%s)", ident, strings.Join(params, ","))
}
