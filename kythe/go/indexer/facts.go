/*
 * Copyright 2015 Google Inc. All rights reserved.
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
	"golang.org/x/net/context"

	spb "kythe.io/kythe/proto/storage_proto"
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
