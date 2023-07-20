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

// Package xrefs defines the xrefs Service interface and some useful utility
// functions.
package xrefs // import "kythe.io/kythe/go/services/xrefs"

import (
	"context"
	"net/http"
	"regexp"
	"strings"
	"time"

	"kythe.io/kythe/go/services/web"
	"kythe.io/kythe/go/util/kytheuri"
	"kythe.io/kythe/go/util/log"
	"kythe.io/kythe/go/util/schema/edges"

	"bitbucket.org/creachadair/stringset"
	"google.golang.org/grpc/codes"
	"google.golang.org/grpc/status"

	cpb "kythe.io/kythe/proto/common_go_proto"
	xpb "kythe.io/kythe/proto/xref_go_proto"
)

// Service defines the interface for file based cross-references.  Informally,
// the cross-references of an entity comprise the definitions of that entity,
// together with all the places where those definitions are referenced through
// constructs such as type declarations, variable references, function calls,
// and so on.
type Service interface {
	// Decorations returns an index of the nodes associated with a specified file.
	Decorations(context.Context, *xpb.DecorationsRequest) (*xpb.DecorationsReply, error)

	// CrossReferences returns the global cross-references for the given nodes.
	CrossReferences(context.Context, *xpb.CrossReferencesRequest) (*xpb.CrossReferencesReply, error)

	// Documentation takes a set of tickets and returns documentation for them.
	Documentation(context.Context, *xpb.DocumentationRequest) (*xpb.DocumentationReply, error)
}

var (
	// ErrPermissionDenied is returned by an implementation of a method when the
	// user is not allowed to view the content because of some restrictions.
	ErrPermissionDenied = status.Error(codes.PermissionDenied, "access denied")

	// ErrDecorationsNotFound is returned by an implementation of the Decorations
	// method when decorations for the given file cannot be found.
	ErrDecorationsNotFound = status.Error(codes.NotFound, "file decorations not found")

	// ErrCanceled is returned by services when the caller cancels the RPC.
	ErrCanceled = status.Error(codes.Canceled, "canceled")

	// ErrDeadlineExceeded is returned by services when something times out.
	ErrDeadlineExceeded = status.Error(codes.DeadlineExceeded, "deadline exceeded")
)

// FixTickets converts the specified tickets, which are expected to be Kythe
// URIs, into canonical form. It is an error if len(tickets) == 0.
func FixTickets(tickets []string) ([]string, error) {
	if len(tickets) == 0 {
		return nil, status.Error(codes.InvalidArgument, "no tickets specified")
	}

	canonical := make([]string, len(tickets))
	for i, ticket := range tickets {
		fixed, err := kytheuri.Fix(ticket)
		if err != nil {
			return nil, status.Errorf(codes.InvalidArgument, "invalid ticket %q: %v", ticket, err)
		}
		canonical[i] = fixed
	}
	return canonical, nil
}

// IsDefKind reports whether the given edgeKind matches the requested
// definition kind.
func IsDefKind(requestedKind xpb.CrossReferencesRequest_DefinitionKind, edgeKind string, incomplete bool) bool {
	edgeKind = edges.Canonical(edgeKind)
	if IsDeclKind(xpb.CrossReferencesRequest_ALL_DECLARATIONS, edgeKind, incomplete) {
		return false
	}
	switch requestedKind {
	case xpb.CrossReferencesRequest_NO_DEFINITIONS:
		return false
	case xpb.CrossReferencesRequest_FULL_DEFINITIONS:
		return edgeKind == edges.Defines
	case xpb.CrossReferencesRequest_BINDING_DEFINITIONS:
		return edgeKind == edges.DefinesBinding
	case xpb.CrossReferencesRequest_ALL_DEFINITIONS:
		return edges.IsVariant(edgeKind, edges.Defines)
	default:
		log.Errorf("unhandled CrossReferencesRequest_DefinitionKind: %v", requestedKind)
		return false
	}
}

// IsDeclKind reports whether the given edgeKind matches the requested
// declaration kind
func IsDeclKind(requestedKind xpb.CrossReferencesRequest_DeclarationKind, edgeKind string, incomplete bool) bool {
	edgeKind = edges.Canonical(edgeKind)
	switch requestedKind {
	case xpb.CrossReferencesRequest_NO_DECLARATIONS:
		return false
	case xpb.CrossReferencesRequest_ALL_DECLARATIONS:
		return (incomplete && edges.IsVariant(edgeKind, edges.Defines)) || edgeKind == internalDeclarationKind
	default:
		log.Errorf("unhandled CrossReferenceRequest_DeclarationKind: %v", requestedKind)
		return false
	}
}

// IsRefKind determines whether the given edgeKind matches the requested
// reference kind.
func IsRefKind(requestedKind xpb.CrossReferencesRequest_ReferenceKind, edgeKind string) bool {
	edgeKind = edges.Canonical(edgeKind)
	switch requestedKind {
	case xpb.CrossReferencesRequest_NO_REFERENCES:
		return false
	case xpb.CrossReferencesRequest_CALL_REFERENCES:
		return edges.IsVariant(edgeKind, edges.RefCall)
	case xpb.CrossReferencesRequest_NON_CALL_REFERENCES:
		return !edges.IsVariant(edgeKind, edges.RefCall) && edges.IsVariant(edgeKind, edges.Ref)
	case xpb.CrossReferencesRequest_ALL_REFERENCES:
		return edges.IsVariant(edgeKind, edges.Ref)
	default:
		log.Errorf("unhandled CrossReferencesRequest_ReferenceKind: %v", requestedKind)
		return false
	}
}

// Internal-only edge kinds for cross-references
const (
	internalKindPrefix         = "#internal/"
	internalCallerKindDirect   = internalKindPrefix + "ref/call/direct"
	internalCallerKindOverride = internalKindPrefix + "ref/call/override"
	internalDeclarationKind    = internalKindPrefix + "ref/declare"
)

// IsInternalKind determines whether the given edge kind is an internal variant.
func IsInternalKind(kind string) bool {
	return strings.HasPrefix(kind, internalKindPrefix)
}

// IsRelatedNodeKind determines whether the give edge kind matches the requested
// related node kinds.
func IsRelatedNodeKind(requestedKinds stringset.Set, kind string) bool {
	return !IsInternalKind(kind) && !edges.IsAnchorEdge(kind) && (len(requestedKinds) == 0 || requestedKinds.Contains(kind))
}

// IsCallerKind determines whether the given edgeKind matches the requested
// caller kind.
func IsCallerKind(requestedKind xpb.CrossReferencesRequest_CallerKind, edgeKind string) bool {
	edgeKind = edges.Canonical(edgeKind)
	switch requestedKind {
	case xpb.CrossReferencesRequest_NO_CALLERS:
		return false
	case xpb.CrossReferencesRequest_DIRECT_CALLERS:
		return edgeKind == internalCallerKindDirect
	case xpb.CrossReferencesRequest_OVERRIDE_CALLERS:
		return edgeKind == internalCallerKindDirect || edgeKind == internalCallerKindOverride
	default:
		log.Errorf("unhandled CrossReferencesRequest_CallerKind: %v", requestedKind)
		return false
	}
}

// ConvertFilters converts each filter glob into an equivalent regexp.
func ConvertFilters(filters []string) []*regexp.Regexp {
	var patterns []*regexp.Regexp
	for _, filter := range filters {
		re := filterToRegexp(filter)
		if re == matchesAll {
			return []*regexp.Regexp{re}
		}
		patterns = append(patterns, re)
	}
	return patterns
}

var (
	filterOpsRE = regexp.MustCompile("[*][*]|[*?]")
	matchesAll  = regexp.MustCompile(".*")
)

func filterToRegexp(pattern string) *regexp.Regexp {
	if pattern == "**" {
		return matchesAll
	}
	var re string
	for {
		loc := filterOpsRE.FindStringIndex(pattern)
		if loc == nil {
			break
		}
		re += regexp.QuoteMeta(pattern[:loc[0]])
		switch pattern[loc[0]:loc[1]] {
		case "**":
			re += ".*"
		case "*":
			re += "[^/]*"
		case "?":
			re += "[^/]"
		default:
			log.Fatal("Unknown filter operator: " + pattern[loc[0]:loc[1]])
		}
		pattern = pattern[loc[1]:]
	}
	return regexp.MustCompile(re + regexp.QuoteMeta(pattern))
}

// MatchesAny reports whether if str matches any of the patterns
func MatchesAny(str string, patterns []*regexp.Regexp) bool {
	for _, p := range patterns {
		if p == matchesAll || p.MatchString(str) {
			return true
		}
	}
	return false
}

// BoundedRequests guards against requests for more tickets than allowed per
// the MaxTickets configuration.
type BoundedRequests struct {
	MaxTickets int
	Service
}

// CrossReferences implements part of the Service interface.
func (b BoundedRequests) CrossReferences(ctx context.Context, req *xpb.CrossReferencesRequest) (*xpb.CrossReferencesReply, error) {
	if len(req.Ticket) > b.MaxTickets {
		return nil, status.Errorf(codes.InvalidArgument, "too many tickets requested: %d (max %d)", len(req.Ticket), b.MaxTickets)
	}
	return b.Service.CrossReferences(ctx, req)
}

// Documentation implements part of the Service interface.
func (b BoundedRequests) Documentation(ctx context.Context, req *xpb.DocumentationRequest) (*xpb.DocumentationReply, error) {
	if len(req.Ticket) > b.MaxTickets {
		return nil, status.Errorf(codes.InvalidArgument, "too many tickets requested: %d (max %d)", len(req.Ticket), b.MaxTickets)
	}
	return b.Service.Documentation(ctx, req)
}

type webClient struct{ addr string }

// Decorations implements part of the Service interface.
func (w *webClient) Decorations(ctx context.Context, q *xpb.DecorationsRequest) (*xpb.DecorationsReply, error) {
	var reply xpb.DecorationsReply
	return &reply, web.Call(w.addr, "decorations", q, &reply)
}

// CrossReferences implements part of the Service interface.
func (w *webClient) CrossReferences(ctx context.Context, q *xpb.CrossReferencesRequest) (*xpb.CrossReferencesReply, error) {
	var reply xpb.CrossReferencesReply
	return &reply, web.Call(w.addr, "xrefs", q, &reply)
}

// Documentation implements part of the Service interface.
func (w *webClient) Documentation(ctx context.Context, q *xpb.DocumentationRequest) (*xpb.DocumentationReply, error) {
	var reply xpb.DocumentationReply
	return &reply, web.Call(w.addr, "documentation", q, &reply)
}

// WebClient returns an xrefs Service based on a remote web server.
func WebClient(addr string) Service {
	return &webClient{addr}
}

// RegisterHTTPHandlers registers JSON HTTP handlers with mux using the given
// xrefs Service.  The following methods with be exposed:
//
//	GET /decorations
//	  Request: JSON encoded xrefs.DecorationsRequest
//	  Response: JSON encoded xrefs.DecorationsReply
//	GET /xrefs
//	  Request: JSON encoded xrefs.CrossReferencesRequest
//	  Response: JSON encoded xrefs.CrossReferencesReply
//	GET /documentation
//	  Request: JSON encoded xrefs.DocumentationRequest
//	  Response: JSON encoded xrefs.DocumentationReply
//
// Note: /nodes, /edges, /decorations, and /xrefs will return their responses as
// serialized protobufs if the "proto" query parameter is set.
func RegisterHTTPHandlers(ctx context.Context, xs Service, mux *http.ServeMux) {
	mux.HandleFunc("/xrefs", func(w http.ResponseWriter, r *http.Request) {
		start := time.Now()
		defer func() {
			log.Infof("xrefs.CrossReferences:\t%s", time.Since(start))
		}()
		var req xpb.CrossReferencesRequest
		if err := web.ReadJSONBody(r, &req); err != nil {
			http.Error(w, err.Error(), http.StatusBadRequest)
			return
		}
		reply, err := xs.CrossReferences(ctx, &req)
		if err != nil {
			http.Error(w, err.Error(), http.StatusInternalServerError)
			return
		}

		if err := web.WriteResponse(w, r, reply); err != nil {
			log.Errorf("CrossReferences error: %v", err)
		}
	})
	mux.HandleFunc("/decorations", func(w http.ResponseWriter, r *http.Request) {
		start := time.Now()
		defer func() {
			log.Infof("xrefs.Decorations:\t%s", time.Since(start))
		}()
		var req xpb.DecorationsRequest
		if err := web.ReadJSONBody(r, &req); err != nil {
			http.Error(w, err.Error(), http.StatusBadRequest)
			return
		}
		reply, err := xs.Decorations(ctx, &req)
		if err != nil {
			http.Error(w, err.Error(), http.StatusInternalServerError)
			return
		}

		if err := web.WriteResponse(w, r, reply); err != nil {
			log.Errorf("Decorations error: %v", err)
		}
	})
	mux.HandleFunc("/documentation", func(w http.ResponseWriter, r *http.Request) {
		start := time.Now()
		defer func() {
			log.Infof("xrefs.Documentation:\t%s", time.Since(start))
		}()
		var req xpb.DocumentationRequest
		if err := web.ReadJSONBody(r, &req); err != nil {
			http.Error(w, err.Error(), http.StatusBadRequest)
			return
		}
		reply, err := xs.Documentation(ctx, &req)
		if err != nil {
			http.Error(w, err.Error(), http.StatusInternalServerError)
			return
		}

		if err := web.WriteResponse(w, r, reply); err != nil {
			log.Errorf("Documentation error: %v", err)
		}
	})
}

// ByName orders a slice of facts by their fact names.
type ByName []*cpb.Fact

func (s ByName) Len() int           { return len(s) }
func (s ByName) Less(i, j int) bool { return s[i].Name < s[j].Name }
func (s ByName) Swap(i, j int)      { s[i], s[j] = s[j], s[i] }
