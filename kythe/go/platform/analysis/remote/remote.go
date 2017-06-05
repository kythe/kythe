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
	"context"
	"fmt"
	"io"

	"kythe.io/kythe/go/platform/analysis"

	apb "kythe.io/kythe/proto/analysis_proto"
	aspb "kythe.io/kythe/proto/analysis_service_proto"

	"github.com/pkg/errors"
)

// Analyzer implements the analysis.CompilationAnalyzer interface by using a
// remote GRPC CompilationAnalyzer server.
type Analyzer struct {
	Client aspb.CompilationAnalyzerClient
}

// An AnalysisError implements the error interface carrying an analysis
// response message received from the remote analyzer.
type AnalysisError struct {
	Result *apb.AnalysisResult
}

func (a AnalysisError) Error() string {
	return fmt.Sprintf("analysis result: %s: %s", a.Result.Status, a.Result.Summary)
}

// Analyze implements the analysis.CompilationAnalyzer interface. If the remote
// analyzer reports a final result carrying a non-success error, the returned
// error will have concrete type AnalysisError carrying the result message.
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

		// Check for a final result. If there is one, we don't report that to
		// the output handler, but shortcut it explicitly here.
		if res := out.FinalResult; res != nil {
			// Constraints: There may be no further outputs; there may not be
			// an output value with the final result.
			if out.Value != nil {
				return errors.New("remote: output value returned with final result")
			} else if _, err := c.Recv(); err != io.EOF {
				return errors.WithMessage(err, "remote: extra output after final result")
			}

			// Don't count a complete analysis as an error.
			if res.Status == apb.AnalysisResult_COMPLETE {
				return nil
			}
			return AnalysisError{Result: res}
		}

		// Reaching here, we have an output to deliver to the callback.
		if err := f(ctx, out); err != nil {
			return err
		}
	}
}
