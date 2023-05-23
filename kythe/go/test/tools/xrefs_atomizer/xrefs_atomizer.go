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

// Binary xrefs_atomizer shatters the file decorations and cross-references
// responses from a xrefs.Service into GraphStore entries.

// Each non-flag argument is a file ticket that will be used in a
// DecorationsRequest to the --api service.  Each file decoration target will
// then be used in a CrossReferencesRequest.  Both the DecorationsReply and
// CrossReferencesReply messages will be shattered into GraphStore entries as a
// delimited stream on standard-output.
//
// See kythe.io/kythe/go/test/xrefs package for details.
//
// Example:
//
//	xrefs_atomizer --api http://localhost:8080 kythe://kythe?path=kythe/go/util/kytheuri/uri.go
package main

import (
	"context"
	"flag"
	"os"

	"kythe.io/kythe/go/platform/delimited"
	"kythe.io/kythe/go/services/xrefs"
	"kythe.io/kythe/go/serving/api"
	txrefs "kythe.io/kythe/go/test/services/xrefs"
	"kythe.io/kythe/go/util/log"
	"kythe.io/kythe/go/util/schema/facts"
	spb "kythe.io/kythe/proto/storage_go_proto"
	xpb "kythe.io/kythe/proto/xref_go_proto"
)

func main() {
	ctx := context.Background()
	xs := api.Flag("api", api.CommonDefault, api.CommonFlagUsage)
	flag.Parse()
	defer (*xs).Close(ctx)

	wr := delimited.NewWriter(os.Stdout)
	var count int
	atomizer := txrefs.Atomizer(func(_ context.Context, e *spb.Entry) error {
		count++
		return wr.PutProto(e)
	})

	for _, ticket := range flag.Args() {
		log.Infof("Atomizing: %q", ticket)
		if err := atomizeFileDecorations(ctx, *xs, ticket, atomizer); err != nil {
			log.Fatalf("Error atomizing file %q: %v", ticket, err)
		}
	}
	log.Infof("Emitted %d entries for %d files", count, flag.NArg())
}

var defaultFilters = []string{facts.NodeKind, facts.Subkind}

func atomizeFileDecorations(ctx context.Context, xs xrefs.Service, ticket string, a txrefs.Atomizer) error {
	reply, err := xs.Decorations(ctx, &xpb.DecorationsRequest{
		Location:   &xpb.Location{Ticket: ticket},
		SourceText: true,
		References: true,
		Filter:     defaultFilters,
	})
	if err != nil {
		return err
	}
	if err := a.Decorations(ctx, reply); err != nil {
		return err
	}
	for _, ref := range reply.Reference {
		if err := atomizeCrossReferences(ctx, xs, ref.TargetTicket, a); err != nil {
			return err
		}
	}
	return nil
}

func atomizeCrossReferences(ctx context.Context, xs xrefs.Service, ticket string, a txrefs.Atomizer) error {
	reply, err := xs.CrossReferences(ctx, &xpb.CrossReferencesRequest{
		Ticket: []string{ticket},

		DefinitionKind:  xpb.CrossReferencesRequest_BINDING_DEFINITIONS,
		DeclarationKind: xpb.CrossReferencesRequest_ALL_DECLARATIONS,
		ReferenceKind:   xpb.CrossReferencesRequest_ALL_REFERENCES,

		Filter: defaultFilters,
	})
	if err != nil {
		return err
	}
	return a.CrossReferences(ctx, reply)
}
