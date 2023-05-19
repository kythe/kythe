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

// Package driver contains a Driver implementation that sends analyses to a
// CompilationAnalyzer based on a Queue of compilations.
package driver // import "kythe.io/kythe/go/platform/analysis/driver"

import (
	"context"
	goerrors "errors"
	"time"

	"kythe.io/kythe/go/platform/analysis"
	"kythe.io/kythe/go/util/log"

	"github.com/pkg/errors"

	apb "kythe.io/kythe/proto/analysis_go_proto"
)

// A Compilation represents a compilation and other metadata needed to analyze it.
type Compilation struct {
	Unit       *apb.CompilationUnit // the compilation to analyze
	Revision   string               // revision marker to attribute to the compilation
	UnitDigest string               // unit digest identifying the compilation in a KCD
	BuildID    string               // id of the build executing the compilation
}

// CompilationFunc handles a single CompilationUnit.
type CompilationFunc func(context.Context, Compilation) error

// A Queue represents an ordered sequence of compilation units.
type Queue interface {
	// Next invokes f with the next available compilation in the queue.  If no
	// further values are available, Next must return ErrEndOfQueue; otherwise,
	// the return value from f is propagated to the caller of Next.
	Next(_ context.Context, f CompilationFunc) error
}

// A Context packages callbacks invoked during analysis.
type Context interface {
	// Setup is invoked after a compilation has been fetched from a Queue but
	// before it is sent to the analyzer.  If Setup reports an error, analysis
	// is aborted.
	Setup(context.Context, Compilation) error

	// Teardown is invoked after a analysis has completed for the compilation.
	// If Teardown reports an error after analysis succeeds, it is logged but
	// does not cause the analysis to fail.
	Teardown(context.Context, Compilation) error

	// AnalysisError is invoked for each non-nil error reported by the analyzer
	// prior to calling Teardown. The error returned from AnalysisError replaces
	// the error returned by the analyzer itself.
	//
	// If AnalysisError returns the special value ErrRetry, the analysis is
	// retried immediately.
	AnalysisError(context.Context, Compilation, error) error
}

var (
	// ErrRetry can be returned from a Driver's AnalysisError function to signal
	// that the driver should retry the analysis immediately.
	ErrRetry = goerrors.New("retry analysis")

	// ErrEndOfQueue can be returned from a Queue to signal there are no
	// compilations left to analyze.
	ErrEndOfQueue = goerrors.New("end of queue")
)

// AnalysisOptions contains extra configuration for analysis requests.
type AnalysisOptions struct {
	// Timeout, if nonzero, sets the given timeout for each analysis request.
	Timeout time.Duration
}

// Driver sends compilations sequentially from a queue to an analyzer.
type Driver struct {
	Analyzer        analysis.CompilationAnalyzer
	AnalysisOptions AnalysisOptions

	FileDataService string
	Context         Context             // if nil, callbacks are no-ops
	WriteOutput     analysis.OutputFunc // if nil, output is discarded
}

func (d *Driver) writeOutput(ctx context.Context, out *apb.AnalysisOutput) error {
	if write := d.WriteOutput; write != nil {
		return write(ctx, out)
	}
	return nil
}

func (d *Driver) setup(ctx context.Context, unit Compilation) error {
	if c := d.Context; c != nil {
		return c.Setup(ctx, unit)
	}
	return nil
}

func (d *Driver) teardown(ctx context.Context, unit Compilation) error {
	if c := d.Context; c != nil {
		return c.Teardown(ctx, unit)
	}
	return nil
}

func (d *Driver) analysisError(ctx context.Context, unit Compilation, err error) error {
	if c := d.Context; c != nil && err != nil {
		return c.AnalysisError(ctx, unit, err)
	}
	return err
}

// Run sends each compilation received from the driver's Queue to the driver's
// Analyzer.  All outputs are passed to Output in turn.  An error is immediately
// returned if the Analyzer, Output, or Compilations fields are unset.
func (d *Driver) Run(ctx context.Context, queue Queue) error {
	if d.Analyzer == nil {
		return errors.New("no analyzer has been specified")
	}

	for {
		if err := queue.Next(ctx, func(ctx context.Context, cu Compilation) error {
			if err := d.setup(ctx, cu); err != nil {
				return errors.WithMessage(err, "driver: analysis setup")
			}
			err := ErrRetry
			for err == ErrRetry {
				err = d.analysisError(ctx, cu, d.runAnalysis(ctx, cu))
			}
			if terr := d.teardown(ctx, cu); terr != nil {
				if err == nil {
					return errors.WithMessage(terr, "driver: analysis teardown")
				}
				log.Warningf("analysis teardown failed: %v (analysis error: %v)", terr, err)
			}
			return err
		}); err == ErrEndOfQueue {
			return nil
		} else if err != nil {
			return err
		}
	}
}

func (d *Driver) runAnalysis(ctx context.Context, cu Compilation) error {
	if d.AnalysisOptions.Timeout != 0 {
		var cancel func()
		ctx, cancel = context.WithTimeout(ctx, d.AnalysisOptions.Timeout)
		defer cancel()
	}
	_, err := d.Analyzer.Analyze(ctx, &apb.AnalysisRequest{
		Compilation:     cu.Unit,
		FileDataService: d.FileDataService,
		Revision:        cu.Revision,
		BuildId:         cu.BuildID,
	}, d.writeOutput)
	return err
}
