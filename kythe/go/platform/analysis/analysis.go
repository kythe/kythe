/*
 * Copyright 2014 The Kythe Authors. All rights reserved.
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

// Package analysis defines interfaces used to locate and analyze compilation
// units and their inputs.
//
// The CompilationAnalyzer interface represents a generic analysis process that
// processes kythe.proto.CompilationUnit messages and their associated inputs.
// The Fetcher interface expresses the ability to fetch required files from
// storage based on their corpus-relative paths and digests.
package analysis // import "kythe.io/kythe/go/platform/analysis"

import (
	"context"
	"fmt"

	"github.com/golang/protobuf/proto"

	apb "kythe.io/kythe/proto/analysis_go_proto"
	spb "kythe.io/kythe/proto/storage_go_proto"
)

// OutputFunc handles a single AnalysisOutput.
type OutputFunc func(context.Context, *apb.AnalysisOutput) error

// A CompilationAnalyzer processes compilation units and delivers output
// artifacts to a user-supplied callback function.
type CompilationAnalyzer interface {
	// Analyze calls f on each analysis output resulting from the analysis of the
	// given apb.CompilationUnit.  If f returns an error, f is no longer called
	// and Analyze returns with the same error.
	Analyze(ctx context.Context, req *apb.AnalysisRequest, f OutputFunc) error
}

// EntryOutput returns an OutputFunc that unmarshals each output's value as an
// Entry and calls f on it.
func EntryOutput(f func(context.Context, *spb.Entry) error) OutputFunc {
	return func(ctx context.Context, out *apb.AnalysisOutput) error {
		var entry spb.Entry
		if err := proto.Unmarshal(out.Value, &entry); err != nil {
			return fmt.Errorf("error unmarshaling Entry from AnalysisOutput: %v", err)
		}
		return f(ctx, &entry)
	}
}

// A Fetcher provides the ability to fetch file contents from storage.
type Fetcher interface {
	// Fetch retrieves the contents of a single file.  At least one of path and
	// digest must be provided; both are preferred.  The implementation may
	// return an error if both are not set.
	Fetch(path, digest string) ([]byte, error)
}
