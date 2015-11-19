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

// Package remote contains kythe/go/platform/analysis implementations backed by
// remote RPC servers.
package remote

import (
	"io"

	"kythe.io/kythe/go/platform/analysis"

	"golang.org/x/net/context"

	apb "kythe.io/kythe/proto/analysis_proto"
)

// Analyzer implements the analysis.CompilationAnalyzer interface by using a
// remote GRPC CompilationAnalyzer server.
type Analyzer struct{ Client apb.CompilationAnalyzerClient }

// Analyze implements the analysis.CompilationAnalyzer interface.
func (a *Analyzer) Analyze(ctx context.Context, req *apb.AnalysisRequest, f analysis.OutputFunc) error {
	c, err := a.Client.Analyze(ctx, req)
	if err != nil {
		return err
	}

	for {
		out, err := c.Recv()
		if err == io.EOF {
			return nil
		} else if err != nil {
			return err
		}

		if err := f(ctx, out); err != nil {
			return err
		}
	}
}
