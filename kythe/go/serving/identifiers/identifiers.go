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

// Package identifiers provides a high-performance table-based implementation of the
// identifiers.Service.
// The table is structured as:
//
//	qualifed_name -> IdentifierMatch
package identifiers // import "kythe.io/kythe/go/serving/identifiers"

import (
	"context"
	"net/http"
	"time"

	"kythe.io/kythe/go/services/web"
	"kythe.io/kythe/go/storage/table"
	"kythe.io/kythe/go/util/kytheuri"
	"kythe.io/kythe/go/util/log"

	"google.golang.org/protobuf/proto"

	ipb "kythe.io/kythe/proto/identifier_go_proto"
	srvpb "kythe.io/kythe/proto/serving_go_proto"
)

// Service describes the interface for the identifier service which provides
// lookups from fully qualified identifiers to any matching semantic nodes
type Service interface {
	// Find returns an index of nodes associated with a given identifier
	Find(context.Context, *ipb.FindRequest) (*ipb.FindReply, error)
}

// Table wraps around a table.Proto to provide the Service interface
type Table struct {
	table.Proto
}

// Find implements the Service interface for Table
func (it *Table) Find(ctx context.Context, req *ipb.FindRequest) (*ipb.FindReply, error) {
	var (
		qname     = req.GetIdentifier()
		corpora   = req.GetCorpus()
		languages = req.GetLanguages()
		reply     ipb.FindReply
	)

	return &reply, it.LookupValues(ctx, []byte(qname), (*srvpb.IdentifierMatch)(nil), func(msg proto.Message) error {
		match := msg.(*srvpb.IdentifierMatch)
		for _, node := range match.GetNode() {
			if !validCorpusAndLang(corpora, languages, node) {
				continue
			}

			matchNode := &ipb.FindReply_Match{
				Ticket:        node.GetTicket(),
				NodeKind:      node.GetNodeKind(),
				NodeSubkind:   node.GetNodeSubkind(),
				BaseName:      match.GetBaseName(),
				QualifiedName: match.GetQualifiedName(),
			}

			reply.Matches = append(reply.GetMatches(), matchNode)
		}
		return nil
	})
}

func validCorpusAndLang(corpora, langs []string, node *srvpb.IdentifierMatch_Node) bool {
	uri, err := kytheuri.Parse(node.GetTicket())
	if err != nil {
		return false
	}

	if len(langs) > 0 && !contains(langs, uri.Language) {
		return false
	}

	if len(corpora) > 0 && !contains(corpora, uri.Corpus) {
		return false
	}

	return true
}

func contains(haystack []string, needle string) bool {
	for _, s := range haystack {
		if s == needle {
			return true
		}
	}
	return false
}

// RegisterHTTPHandlers registers a JSON HTTP handler with mux using the given
// identifiers Service.  The following method with be exposed:
//
//	GET /find_identifier
//	  Request: JSON encoded identifier.FindRequest
//	  Response: JSON encoded identifier.FindReply
//
// Note: /find_identifier will return its response as a serialized protobuf if
// the "proto" query parameter is set.
func RegisterHTTPHandlers(ctx context.Context, id Service, mux *http.ServeMux) {
	mux.HandleFunc("/find_identifier", func(w http.ResponseWriter, r *http.Request) {
		start := time.Now()
		defer func() {
			log.Infof("identifiers.Find:\t%s", time.Since(start))
		}()
		var req ipb.FindRequest
		if err := web.ReadJSONBody(r, &req); err != nil {
			http.Error(w, err.Error(), http.StatusBadRequest)
			return
		}
		reply, err := id.Find(ctx, &req)
		if err != nil {
			http.Error(w, err.Error(), http.StatusInternalServerError)
			return
		}

		if err := web.WriteResponse(w, r, reply); err != nil {
			log.Info(err)
		}
	})
}

type webClient struct{ addr string }

// Find implements part of the Service interface.
func (w *webClient) Find(ctx context.Context, q *ipb.FindRequest) (*ipb.FindReply, error) {
	var reply ipb.FindReply
	return &reply, web.Call(w.addr, "find_identifier", q, &reply)
}

// WebClient returns an identifiers Service based on a remote web server.
func WebClient(addr string) Service {
	return &webClient{addr}
}
