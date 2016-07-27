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

// Package driver contains a Driver implementation that sends analyses to a
// CompilationAnalyzer based on a Queue of compilations.
package driver

import (
	"errors"
	"fmt"
	"io"
	"log"

	"kythe.io/kythe/go/platform/analysis"

	"golang.org/x/net/context"

	apb "kythe.io/kythe/proto/analysis_proto"
)

// CompilationFunc handles a single CompilationUnit.
type CompilationFunc func(context.Context, *apb.CompilationUnit) error

// A Queue represents an ordered sequence of compilation units.
type Queue interface {
	// Next invokes f with the next available compilation in the queue.  If no
	// further values are available, Next must return io.EOF; otherwise, the
	// return value from f is propagated to the caller of Next.
	Next(_ context.Context, f CompilationFunc) error
}

// Driver sends compilations sequentially from a queue to an analyzer.
type Driver struct {
	Analyzer        analysis.CompilationAnalyzer
	FileDataService string

	// AnalysisError is called for each non-nil err returned from the Analyzer
	// (before Teardown is called).  The error returned from AnalysisError
	// replaces the analysis error that would normally be returned from Run.
	AnalysisError func(context.Context, *apb.CompilationUnit, error) error

	// Compilations is a queue of compilations to be sent for analysis.
	Compilations Queue

	// Setup is called after a compilation has been pulled from the Queue and
	// before it is sent to the Analyzer (or Output is called).
	Setup CompilationFunc
	// Output is called for each analysis output returned from the Analyzer
	Output analysis.OutputFunc
	// Teardown is called after a compilation has been analyzed and there will be no further calls to Output.
	Teardown CompilationFunc
}

func (d *Driver) validate() error {
	if d.Analyzer == nil {
		return errors.New("missing Analyzer")
	} else if d.Compilations == nil {
		return errors.New("missing Compilations Queue")
	} else if d.Output == nil {
		return errors.New("missing Output function")
	}
	return nil
}

// Run sends each compilation received from the driver's Queue to the driver's
// Analyzer.  All outputs are passed to Output in turn.  An error is immediately
// returned if the Analyzer, Output, or Compilations fields are unset.
func (d *Driver) Run(ctx context.Context) error {
	if err := d.validate(); err != nil {
		return err
	}
	for {
		if err := d.Compilations.Next(ctx, func(ctx context.Context, cu *apb.CompilationUnit) error {
			if d.Setup != nil {
				if err := d.Setup(ctx, cu); err != nil {
					return fmt.Errorf("analysis setup error: %v", err)
				}
			}
			err := d.Analyzer.Analyze(ctx, &apb.AnalysisRequest{
				Compilation:     cu,
				FileDataService: d.FileDataService,
			}, d.Output)
			if d.AnalysisError != nil && err != nil {
				err = d.AnalysisError(ctx, cu, err)
			}
			if d.Teardown != nil {
				if tErr := d.Teardown(ctx, cu); tErr != nil {
					if err == nil {
						return fmt.Errorf("analysis teardown error: %v", err)
					}
					log.Printf("WARNING: analysis teardown error after analysis error: %v (analysis error: %v)", tErr, err)
				}
			}
			return err
		}); err == io.EOF {
			return nil
		} else if err != nil {
			return err
		}
	}
}
