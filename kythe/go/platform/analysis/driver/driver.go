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
	"io"

	"kythe.io/kythe/go/platform/analysis"

	"golang.org/x/net/context"

	apb "kythe.io/kythe/proto/analysis_proto"
)

// CompilationFunc handles a single CompilationUnit.
type CompilationFunc func(context.Context, *apb.CompilationUnit) error

// Queue is a generic interface to a sequence of CompilationUnits.
type Queue interface {
	Next(context.Context, CompilationFunc) error
}

// Driver sends compilations sequentially from a queue to an analyzer.
type Driver struct {
	Analyzer analysis.CompilationAnalyzer
	Output   analysis.OutputFunc

	FileDataService string
	Compilations    Queue
}

// Run sends each compilation received from the driver's Queue to the driver's
// Analyzer.  All outputs are passed to Output in turn.
func (d *Driver) Run(ctx context.Context) error {
	for {
		if err := d.Compilations.Next(ctx, func(ctx context.Context, cu *apb.CompilationUnit) error {
			return d.Analyzer.Analyze(ctx, &apb.AnalysisRequest{
				Compilation:     cu,
				FileDataService: d.FileDataService,
			}, d.Output)
		}); err == io.EOF {
			return nil
		} else if err != nil {
			return err
		}
	}
}
