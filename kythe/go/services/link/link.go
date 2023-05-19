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

// Package link implements the link resolver service.
package link // import "kythe.io/kythe/go/services/link"

import (
	"context"
	"fmt"
	"regexp"
	"sort"
	"time"

	"kythe.io/kythe/go/util/kytheuri"
	"kythe.io/kythe/go/util/log"
	"kythe.io/kythe/go/util/schema/edges"
	"kythe.io/kythe/go/util/schema/facts"

	"bitbucket.org/creachadair/stringset"
	"golang.org/x/sync/errgroup"
	"golang.org/x/sync/semaphore"
	"google.golang.org/grpc/codes"
	"google.golang.org/grpc/status"
	"google.golang.org/protobuf/encoding/prototext"
	"google.golang.org/protobuf/proto"

	ipb "kythe.io/kythe/proto/identifier_go_proto"
	linkpb "kythe.io/kythe/proto/link_go_proto"
	xpb "kythe.io/kythe/proto/xref_go_proto"
)

// A Resolver implements the link service resolver by dispatching to a Kythe
// XRefService and IdentifierService to resolve qualified names.
type Resolver struct {
	Client interface {
		CrossReferences(context.Context, *xpb.CrossReferencesRequest) (*xpb.CrossReferencesReply, error)
		Find(context.Context, *ipb.FindRequest) (*ipb.FindReply, error)
	}
}

// Resolve implements the internals of the resolve method.
func (s *Resolver) Resolve(ctx context.Context, req *linkpb.LinkRequest) (*linkpb.LinkReply, error) {
	if req.Identifier == "" {
		return nil, status.Error(codes.InvalidArgument, "missing link identifier")
	}
	include, err := compileLocation(req.Include, true)
	if err != nil {
		return nil, status.Errorf(codes.InvalidArgument, "include: %v", err)
	}
	exclude, err := compileLocation(req.Exclude, false)
	if err != nil {
		return nil, status.Errorf(codes.InvalidArgument, "exclude: %v", err)
	}

	// Stage 1: Resolve identifiers.
	ireq := &ipb.FindRequest{
		Identifier: req.Identifier,
		Corpus:     req.Corpus,
		Languages:  req.Language,
	}
	ids, err := s.Client.Find(ctx, ireq)
	if err != nil {
		return nil, err
	}

	idMatches := make(map[string]*ipb.FindReply_Match)
	for _, m := range ids.Matches {
		if !kindMatches(m, req.NodeKind) {
			continue
		}
		idMatches[m.Ticket] = m
	}

	const maxMatches = 1000
	if len(idMatches) == 0 {
		return nil, status.Error(codes.NotFound, "no matches")
	} else if len(idMatches) > maxMatches {
		return nil, status.Errorf(codes.OutOfRange, "too many identifier matches (%d > %d)",
			len(idMatches), maxMatches/2) // a comforting deceit
	}
	log.Infof("Found %d of %d matches for identifier %q",
		len(idMatches), len(ids.Matches), req.Identifier)

	// Stage 2: Find definitions of the matching nodes.
	xreq := &xpb.CrossReferencesRequest{
		Ticket:          stringset.FromKeys(idMatches).Unordered(),
		DefinitionKind:  xpb.CrossReferencesRequest_BINDING_DEFINITIONS,
		Snippets:        xpb.SnippetsKind_NONE,
		Filter:          []string{facts.NodeKind, facts.Complete},
		NodeDefinitions: true,
	}
	switch req.DefinitionKind {
	case linkpb.LinkRequest_FULL:
		xreq.DefinitionKind = xpb.CrossReferencesRequest_FULL_DEFINITIONS
	case linkpb.LinkRequest_ANY:
		xreq.DefinitionKind = xpb.CrossReferencesRequest_ALL_DEFINITIONS
	case linkpb.LinkRequest_BINDING:
	default:
		log.Warningf("Unknown definition kind %v (ignored)", req.DefinitionKind)
	}
	log.Info("Cross-references request:\n", prototext.Format(xreq))
	defs, err := s.crossRefs(ctx, xreq)
	if err != nil {
		return nil, err
	}
	log.Infof("Found %d result sets", len(defs.CrossReferences))

	// Gather all the anchors matching each definition. Also check whether any
	// of the tickets is a complete definition, since that is preferred.
	var numAnchors int
	anchors := make(map[string][]*xpb.Anchor)
	complete := stringset.New()
	pref := false
	for ticket, xrefs := range defs.CrossReferences {
		log.Infof("Checking node %q...", ticket)

		if req.Params != nil {
			// Count parameters, and filter based on that.
			nparams := 0
			for _, rel := range xrefs.RelatedNode {
				n := int(rel.Ordinal) + 1
				if rel.RelationKind == edges.Param {
					if n > nparams {
						nparams = n
					}
				}
			}
			log.Infof("+ Node has %d parameters", nparams)
			if n := int(req.Params.GetCount()); n != nparams {
				log.Infof("- Wrong number of parameters (have %d, want %d)", nparams, n)
				continue
			}
		}

		// Check for complete definitions.
		if comp, ok := defs.Nodes[ticket].GetFacts()[facts.Complete]; ok {
			switch s := string(comp); s {
			case "definition":
				if !pref {
					complete = stringset.New()
					pref = true
				}
				complete.Add(ticket)
				log.Info("+ Node is a preferred complete definition")
			case "complete":
				if !pref {
					complete.Add(ticket)
					log.Info("+ Node is a complete definition")
				}
			}
		}
		anchors[ticket] = findAnchors(ticket, defs, include, exclude)
		numAnchors += len(anchors[ticket])
	}

	// If we do have any complete definitions, throw out all the others.
	if len(complete) != 0 {
		for ticket := range anchors {
			if !complete.Contains(ticket) {
				log.Infof("- Discarding incomplete definition %q", ticket)
				delete(anchors, ticket)
			}
		}
	}

	// Stage 3: Filter the definitions by location. We have to do a little
	// dance here to correctly deal with nodes whose definitions we share from
	// code generation.
	type result struct {
		nodes stringset.Set // semantic node tickets defined here
		link  *linkpb.Link  // the link message for the reply
	}
	type fileKey struct {
		file string
		line int32
	}
	seen := make(map[fileKey]*result)
	for ticket, anchors := range anchors {
		for _, anchor := range anchors {
			// Record one link for each distinct location. We prefer location to
			// anchor because there may be multiple anchors spanning the same
			// location. We keep track of the semantic node tickets along the way,
			// since the request may desire them.
			key := fileKey{
				file: anchor.Parent,
				line: anchor.Span.Start.GetLineNumber(),
			}
			if res, ok := seen[key]; ok {
				res.nodes.Add(ticket)
			} else {
				link := &linkpb.Link{
					FileTicket: anchor.Parent,
					Span:       anchor.Span,
				}
				seen[key] = &result{
					nodes: stringset.New(ticket),
					link:  link,
				}
			}
		}
	}

	log.Infof("After filtering %d anchor locations there are %d unique results",
		numAnchors, len(seen))
	if len(seen) == 0 {
		return nil, status.Error(codes.NotFound, "no matching definitions")
	}

	// Populate the links in the result, and order them for stability.
	rsp := new(linkpb.LinkReply)
	for _, res := range seen {
		if req.IncludeNodes {
			for _, ticket := range res.nodes.Elements() {
				m := idMatches[ticket]
				res.link.Nodes = append(res.link.Nodes, &linkpb.Link_Node{
					Ticket:     ticket,
					BaseName:   m.GetBaseName(),
					Identifier: m.GetQualifiedName(),
				})
			}
		}
		rsp.Links = append(rsp.Links, res.link)
		log.Infof("Result: %+v", res.link)
	}
	sort.Slice(rsp.Links, func(i, j int) bool {
		return rsp.Links[i].FileTicket < rsp.Links[j].FileTicket
	})

	return rsp, nil
}

// kindMatches reports whether the kind and subkind of m match any of the
// entries in kinds, which have the form "kind" or "kind/subkind".
func kindMatches(m *ipb.FindReply_Match, kinds []string) bool {
	if len(kinds) == 0 {
		return m.NodeKind != "lookup"
	}
	key := m.NodeKind
	if sk := m.NodeSubkind; sk != "" {
		key += "/" + sk
	}
	return stringset.Index(m.NodeKind, kinds) >= 0 || stringset.Index(key, kinds) >= 0
}

// crossRefs calls XRefService.CrossReferences with the given request, issuing
// one request per ticket in parallel. The API allows multiple tickets per
// request, but server-side merging can get confused if we pass in related
// tickets. The results are merged locally, which is safe.
func (s *Resolver) crossRefs(ctx context.Context, req *xpb.CrossReferencesRequest) (_ *xpb.CrossReferencesReply, err error) {
	start := time.Now()
	defer func() { log.Infof("CrossReferences complete err=%v [%v elapsed]", err, time.Since(start)) }()

	reqs := make([]*xpb.CrossReferencesRequest, 0, len(req.Ticket))
	for _, ticket := range req.Ticket {
		next := proto.Clone(req).(*xpb.CrossReferencesRequest)
		next.Ticket = []string{ticket}
		reqs = append(reqs, next)
	}
	rsps := make([]*xpb.CrossReferencesReply, len(req.Ticket))
	sem := semaphore.NewWeighted(32)
	g, gctx := errgroup.WithContext(ctx)
	for i, req := range reqs {
		i, req := i, req
		if sem.Acquire(gctx, 1) != nil {
			break
		}
		g.Go(func() error {
			defer sem.Release(1)
			var err error
			rsps[i], err = s.Client.CrossReferences(gctx, req)
			return err
		})
	}
	if err := g.Wait(); err != nil {
		return nil, err
	}
	for _, rsp := range rsps[1:] {
		proto.Merge(rsps[0], rsp)
	}
	return rsps[0], nil
}

type matcher func(*kytheuri.URI) bool

// compileLocation returns a matching function from the specified location.
// The function returns keep for all inputs if if loc == nil.
func compileLocation(locs []*linkpb.LinkRequest_Location, keep bool) (matcher, error) {
	if len(locs) == 0 {
		return func(*kytheuri.URI) bool { return keep }, nil
	}
	var matchPath, matchRoot []func(string) bool
	corpora := stringset.New()
	for _, loc := range locs {
		if r, err := regexp.Compile(loc.Path); err == nil {
			matchPath = append(matchPath, r.MatchString)
		} else {
			return nil, fmt.Errorf("path regexp: %v", err)
		}
		if r, err := regexp.Compile(loc.Root); err == nil {
			matchRoot = append(matchRoot, r.MatchString)
		} else {
			return nil, fmt.Errorf("root regexp: %v", err)
		}
		if loc.Corpus != "" {
			corpora.Add(loc.Corpus)
		}
	}
	matchCorpus := func(c string) bool { return corpora.Len() == 0 || corpora.Contains(c) }
	return func(u *kytheuri.URI) bool {
		for i := 0; i < len(matchPath); i++ {
			if matchPath[i](u.Path) && matchRoot[i](u.Root) && matchCorpus(u.Corpus) {
				return true
			}
		}
		return false
	}, nil
}

// findAnchors locates valid anchors for the given ticket in rsp, according to
// the include and exclude rules, as well as ticket language and corpus.
func findAnchors(ticket string, rsp *xpb.CrossReferencesReply, include, exclude matcher) []*xpb.Anchor {
	t, err := kytheuri.Parse(ticket)
	if err != nil {
		log.Errorf("Invalid ticket %q: %v", ticket, err)
		return nil
	}
	check := func(ticket string) bool {
		if a, err := kytheuri.Parse(ticket); err != nil {
			log.Infof("- Invalid ticket %s: %v", ticket, err)
		} else if t.Language != "" && a.Language != t.Language {
			log.Infof("- Language mismatch (%q ≠ %q)", a.Language, t.Language)
		} else if !include(a) || exclude(a) {
			log.Infof("- Filter mismatch %s", ticket)
		} else {
			return true
		}
		return false
	}

	// If there is a single direct definition, it supersedes all other options.
	if node, ok := rsp.Nodes[ticket]; ok && node.Definition != "" {
		if anchor, ok := rsp.DefinitionLocations[node.Definition]; ok {
			if check(anchor.Ticket) {
				log.Infof("+ Found matching definition anchor: %s", anchor.Ticket)
				return []*xpb.Anchor{anchor}
			}
		}
	}

	// Look for a definition among the possibly-clustered hoi polloi.
	var result []*xpb.Anchor
	for _, def := range rsp.CrossReferences[ticket].GetDefinition() {
		anchor := def.Anchor
		if !edges.IsVariant(anchor.Kind, edges.Defines) {
			continue // not a definition
		}
		if check(anchor.Ticket) {
			log.Infof("+ Found matching anchor: %s", anchor.Ticket)
			result = append(result, anchor)
		}
	}

	return result
}
