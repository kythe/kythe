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

package indexer

import (
	"context"
	"crypto/sha256"
	"encoding/base64"
	"fmt"
	"strconv"

	"kythe.io/kythe/go/extractors/govname"
	"kythe.io/kythe/go/util/schema/edges"
	"kythe.io/kythe/go/util/schema/facts"
	"kythe.io/kythe/go/util/schema/nodes"

	spb "kythe.io/kythe/proto/storage_go_proto"
)

// A Sink is a callback invoked by the indexer to deliver entries.
type Sink func(context.Context, *spb.Entry) error

// writeFact writes a single fact with the given name and value for src to s.
func (s Sink) writeFact(ctx context.Context, src *spb.VName, name, value string) error {
	return s(ctx, &spb.Entry{
		Source:    src,
		FactName:  name,
		FactValue: []byte(value),
	})
}

// writeEdge writes an edge with the specified kind between src and tgt to s.
func (s Sink) writeEdge(ctx context.Context, src, tgt *spb.VName, kind string) error {
	return s(ctx, &spb.Entry{
		Source:   src,
		Target:   tgt,
		EdgeKind: kind,
		FactName: "/",
	})
}

// writeAnchor emits an anchor with the given offsets to s.
func (s Sink) writeAnchor(ctx context.Context, src *spb.VName, start, end int) error {
	if err := s.writeFact(ctx, src, facts.NodeKind, nodes.Anchor); err != nil {
		return err
	}
	if err := s.writeFact(ctx, src, facts.AnchorStart, strconv.Itoa(start)); err != nil {
		return err
	}
	return s.writeFact(ctx, src, facts.AnchorEnd, strconv.Itoa(end))
}

// A diagnostic represents a diagnostic message attached to some node in the
// graph by the indexer.
type diagnostic struct {
	Message string // One-line human-readable summary
	Details string // Detailed description or content
	URL     string // Optional context URL
}

func (d diagnostic) vname(src *spb.VName) *spb.VName {
	return &spb.VName{
		Language:  govname.Language,
		Corpus:    src.Corpus,
		Path:      src.Path,
		Root:      src.Root,
		Signature: "diag " + hashSignature(d.Message, d.Details, d.URL),
	}
}

// writeDiagnostic attaches d as a diagnostic on src.
func (s Sink) writeDiagnostic(ctx context.Context, src *spb.VName, d diagnostic) error {
	dname := d.vname(src)
	facts := [...]struct{ name, value string }{
		{facts.NodeKind, nodes.Diagnostic},
		{facts.Message, d.Message},
		{facts.Details, d.Details},
		{facts.ContextURL, d.URL},
	}
	for _, fact := range facts {
		if fact.value == "" {
			continue
		} else if err := s.writeFact(ctx, dname, fact.name, fact.value); err != nil {
			return err
		}
	}
	return s.writeEdge(ctx, src, dname, edges.Tagged)
}

func hashSignature(s ...any) string {
	hash := sha256.New()
	fmt.Fprintln(hash, s...)
	return base64.URLEncoding.EncodeToString(hash.Sum(nil)[:])
}
