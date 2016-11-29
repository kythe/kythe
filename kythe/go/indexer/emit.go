/*
 * Copyright 2016 Google Inc. All rights reserved.
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
	"log"

	"kythe.io/kythe/go/util/schema/facts"
	"kythe.io/kythe/go/util/schema/nodes"
)

// Emit generates Kythe facts and edges to represent pi, and writes them to
// sink. In case of errors, processing continues as far as possible before the
// first error encountered is reported.
func (pi *PackageInfo) Emit(ctx context.Context, sink Sink) error {
	var firstErr error
	check := func(err error) {
		if err != nil && firstErr == nil {
			firstErr = err
			log.Printf("ERROR indexing %q: %v", pi.ImportPath, err)
		}
	}

	// Emit facts for all the source files claimed by this package.
	for path, text := range pi.SourceText {
		vname := pi.FileVName(path)
		check(sink.writeFact(ctx, vname, facts.NodeKind, nodes.File))
		check(sink.writeFact(ctx, vname, facts.Text, text))
		// All Go source files are encoded as UTF-8, which is the default.
	}

	// TODO(fromberger): Add diagnostics for type-checker errors.
	for _, err := range pi.Errors {
		log.Printf("WARNING: Type resolution error: %v", err)
	}
	return firstErr
}
