/*
 * Copyright 2017 Google Inc. All rights reserved.
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
// 		qualifed_name -> IdentifierMatch
package identifiers

import (
	"context"

	"kythe.io/kythe/go/storage/table"
	"kythe.io/kythe/go/util/kytheuri"

	ipb "kythe.io/kythe/proto/identifier_proto"
	srvpb "kythe.io/kythe/proto/serving_proto"
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
		match     srvpb.IdentifierMatch
		reply     ipb.FindReply
	)

	if err := it.Lookup(ctx, []byte(qname), &match); err != nil {
		return &reply, nil
	}

	for _, node := range match.GetNode() {
		if !validCorpusAndLang(corpora, languages, node) {
			continue
		}

		matchNode := ipb.FindReply_Match{
			Ticket:        node.GetTicket(),
			NodeKind:      node.GetNodeKind(),
			NodeSubkind:   node.GetNodeSubkind(),
			BaseName:      match.GetBaseName(),
			QualifiedName: match.GetQualifiedName(),
		}

		reply.Matches = append(reply.GetMatches(), &matchNode)
	}

	return &reply, nil
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
