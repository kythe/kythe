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

package driver

import (
	"bytes"
	"context"
	"errors"
	"io"
	"testing"

	"kythe.io/kythe/go/platform/analysis"
	"kythe.io/kythe/go/test/testutil"

	apb "kythe.io/kythe/proto/analysis_proto"
	spb "kythe.io/kythe/proto/storage_proto"
)

type mock struct {
	t *testing.T

	idx          int
	Compilations []Compilation

	Outputs      []*apb.AnalysisOutput
	AnalyzeError error
	OutputError  error

	OutputIndex int
	Requests    []*apb.AnalysisRequest
}

func (m *mock) out() analysis.OutputFunc {
	return func(_ context.Context, out *apb.AnalysisOutput) error {
		if m.OutputIndex >= len(m.Outputs) {
			m.t.Fatal("OutputFunc called more times than expected")
		}
		expected := m.Outputs[m.OutputIndex]
		if !bytes.Equal(out.Value, expected.Value) {
			m.t.Errorf("Expected output %d: %q; found: %q", m.OutputIndex, string(expected.Value), string(out.Value))
		}
		m.OutputIndex++
		return m.OutputError
	}
}

// Analyze implements the analysis.CompilationAnalyzer interface.
func (m *mock) Analyze(ctx context.Context, req *apb.AnalysisRequest, out analysis.OutputFunc) error {
	m.OutputIndex = 0
	m.Requests = append(m.Requests, req)
	for _, o := range m.Outputs {
		if err := out(ctx, o); err != m.OutputError {
			m.t.Errorf("Expected OutputFunc error: %v; found: %v", m.OutputError, err)
			return err
		}
	}
	if m.OutputIndex != len(m.Outputs) {
		m.t.Errorf("Expected OutputIndex: %d; found: %d", len(m.Outputs), m.OutputIndex)
	}
	return m.AnalyzeError
}

// Next implements the Queue interface.
func (m *mock) Next(ctx context.Context, f CompilationFunc) error {
	if m.idx >= len(m.Compilations) {
		return io.EOF
	}
	err := f(ctx, m.Compilations[m.idx])
	m.idx++
	return err
}

const fdsAddr = "TEST FDS ADDR"

func TestDriverInvalid(t *testing.T) {
	m := &mock{t: t}
	tests := []*Driver{
		{},
		{Analyzer: m},
		{Compilations: m},
		{Output: m.out()},
		{Analyzer: m, Compilations: m},
		{Analyzer: m, Output: m.out()},
		{Compilations: m, Output: m.out()},
	}

	for _, d := range tests {
		if err := d.Run(context.Background()); err == nil {
			t.Fatalf("Did not receive expected error from %#v.Run(ctx)", d)
		}
	}
}

func TestDriverEmpty(t *testing.T) {
	m := &mock{t: t}
	d := &Driver{
		Analyzer:     m,
		Compilations: m,
		Output:       m.out(),
	}
	testutil.FatalOnErrT(t, "Driver error: %v", d.Run(context.Background()))
	if len(m.Requests) != 0 {
		t.Fatalf("Unexpected AnalysisRequests: %v", m.Requests)
	}
}

func TestDriver(t *testing.T) {
	m := &mock{
		t:            t,
		Outputs:      outs("a", "b", "c"),
		Compilations: comps("target1", "target2"),
	}
	var setupIdx, teardownIdx int
	d := &Driver{
		Analyzer:     m,
		Compilations: m,
		Output:       m.out(),
		Setup: func(_ context.Context, cu Compilation) error {
			setupIdx++
			return nil
		},
		Teardown: func(_ context.Context, cu Compilation) error {
			if setupIdx != teardownIdx+1 {
				t.Error("Teardown was not called directly after Setup/Analyze")
			}
			teardownIdx++
			return nil
		},
	}
	testutil.FatalOnErrT(t, "Driver error: %v", d.Run(context.Background()))
	if len(m.Requests) != len(m.Compilations) {
		t.Errorf("Expected %d AnalysisRequests; found %v", len(m.Compilations), m.Requests)
	}
	if setupIdx != len(m.Compilations) {
		t.Errorf("Expected %d calls to Setup; found %d", len(m.Compilations), setupIdx)
	}
	if teardownIdx != len(m.Compilations) {
		t.Errorf("Expected %d calls to Teardown; found %d", len(m.Compilations), teardownIdx)
	}
}

var errFromAnalysis = errors.New("some random analysis error")

func TestDriverAnalyzeError(t *testing.T) {
	m := &mock{
		t:            t,
		Outputs:      outs("a", "b", "c"),
		Compilations: comps("target1", "target2"),
		AnalyzeError: errFromAnalysis,
	}
	d := &Driver{
		Analyzer:     m,
		Compilations: m,
		Output:       m.out(),
	}
	if err := d.Run(context.Background()); err != errFromAnalysis {
		t.Errorf("Expected AnalysisError: %v; found: %v", errFromAnalysis, err)
	}
	if len(m.Requests) != 1 { // we didn't analyze the second
		t.Errorf("Expected %d AnalysisRequests; found %v", 1, m.Requests)
	}
}

func TestDriverErrorHandler(t *testing.T) {
	m := &mock{
		t:            t,
		Outputs:      outs("a", "b", "c"),
		Compilations: comps("target1", "target2"),
		AnalyzeError: errFromAnalysis,
	}
	var analysisErr error
	d := &Driver{
		Analyzer:     m,
		Compilations: m,
		Output:       m.out(),
		AnalysisError: func(_ context.Context, cu Compilation, err error) error {
			analysisErr = err
			return nil // don't return err
		},
	}
	testutil.FatalOnErrT(t, "Driver error: %v", d.Run(context.Background()))
	if len(m.Requests) != len(m.Compilations) {
		t.Errorf("Expected %d AnalysisRequests; found %v", len(m.Compilations), m.Requests)
	}
	if analysisErr != errFromAnalysis {
		t.Errorf("Expected AnalysisError: %v; found: %v", errFromAnalysis, analysisErr)
	}
}

func TestDriverSetup(t *testing.T) {
	m := &mock{
		t:            t,
		Outputs:      outs("a", "b", "c"),
		Compilations: comps("target1", "target2"),
	}
	var setupIdx int
	d := &Driver{
		Analyzer:     m,
		Compilations: m,
		Output:       m.out(),
		Setup: func(_ context.Context, cu Compilation) error {
			setupIdx++
			return nil
		},
	}
	testutil.FatalOnErrT(t, "Driver error: %v", d.Run(context.Background()))
	if len(m.Requests) != len(m.Compilations) {
		t.Errorf("Expected %d AnalysisRequests; found %v", len(m.Compilations), m.Requests)
	}
	if setupIdx != len(m.Compilations) {
		t.Errorf("Expected %d calls to Setup; found %d", len(m.Compilations), setupIdx)
	}
}

func TestDriverTeardown(t *testing.T) {
	m := &mock{
		t:            t,
		Outputs:      outs("a", "b", "c"),
		Compilations: comps("target1", "target2"),
	}
	var teardownIdx int
	d := &Driver{
		Analyzer:     m,
		Compilations: m,
		Output:       m.out(),
		Teardown: func(_ context.Context, cu Compilation) error {
			teardownIdx++
			return nil
		},
	}
	testutil.FatalOnErrT(t, "Driver error: %v", d.Run(context.Background()))
	if len(m.Requests) != len(m.Compilations) {
		t.Errorf("Expected %d AnalysisRequests; found %v", len(m.Compilations), m.Requests)
	}
	if teardownIdx != len(m.Compilations) {
		t.Errorf("Expected %d calls to Teardown; found %d", len(m.Compilations), teardownIdx)
	}
}

func outs(vals ...string) (as []*apb.AnalysisOutput) {
	for _, val := range vals {
		as = append(as, &apb.AnalysisOutput{Value: []byte(val)})
	}
	return
}

func comps(sigs ...string) (cs []Compilation) {
	for _, sig := range sigs {
		cs = append(cs, Compilation{
			Unit: &apb.CompilationUnit{VName: &spb.VName{Signature: sig}},
		})
	}
	return
}
