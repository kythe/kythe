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

package driver

import (
	"bytes"
	"context"
	"errors"
	"fmt"
	"strings"
	"testing"
	"time"

	"kythe.io/kythe/go/platform/analysis"
	"kythe.io/kythe/go/test/testutil"
	"kythe.io/kythe/go/util/log"

	apb "kythe.io/kythe/proto/analysis_go_proto"
	spb "kythe.io/kythe/proto/storage_go_proto"
)

type mock struct {
	t *testing.T

	idx int

	Compilations   []Compilation
	Outputs        []*apb.AnalysisOutput
	AnalysisResult *apb.AnalysisResult
	AnalyzeError   error
	OutputError    error

	AnalysisDuration time.Duration

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

const buildID = "aabbcc"

// Analyze implements the analysis.CompilationAnalyzer interface.
func (m *mock) Analyze(ctx context.Context, req *apb.AnalysisRequest, out analysis.OutputFunc) (*apb.AnalysisResult, error) {
	if m.AnalysisDuration != 0 {
		log.Infof("Waiting %s for analysis request", m.AnalysisDuration)
		time.Sleep(m.AnalysisDuration)
	}
	m.OutputIndex = 0
	m.Requests = append(m.Requests, req)
	for _, o := range m.Outputs {
		if err := out(ctx, o); err != m.OutputError {
			m.t.Errorf("Expected OutputFunc error: %v; found: %v", m.OutputError, err)
			return nil, err
		}
	}
	if m.OutputIndex != len(m.Outputs) {
		m.t.Errorf("Expected OutputIndex: %d; found: %d", len(m.Outputs), m.OutputIndex)
	}
	if req.BuildId != buildID {
		m.t.Errorf("Missing build ID")
	}
	if err := ctx.Err(); err != nil {
		return m.AnalysisResult, err
	}
	return m.AnalysisResult, m.AnalyzeError
}

// Next implements the Queue interface.
func (m *mock) Next(ctx context.Context, f CompilationFunc) error {
	if m.idx >= len(m.Compilations) {
		return ErrEndOfQueue
	}
	err := f(ctx, m.Compilations[m.idx])
	m.idx++
	return err
}

// A testContext implements the Context interface through local functions.
// The default implementations are no-ops without error.
type testContext struct {
	setup         func(context.Context, Compilation) error
	teardown      func(context.Context, Compilation) error
	analysisError func(context.Context, Compilation, error) error
}

func (t testContext) Setup(ctx context.Context, unit Compilation) error {
	if t.setup != nil {
		return t.setup(ctx, unit)
	}
	return nil
}

func (t testContext) Teardown(ctx context.Context, unit Compilation) error {
	if t.teardown != nil {
		return t.teardown(ctx, unit)
	}
	return nil
}

func (t testContext) AnalysisError(ctx context.Context, unit Compilation, err error) error {
	if t.analysisError != nil {
		return t.analysisError(ctx, unit, err)
	}
	return err
}

func TestDriverInvalid(t *testing.T) {
	m := &mock{t: t}
	test := new(Driver)
	if err := test.Run(context.Background(), m); err == nil {
		t.Errorf("Expected error from %#v.Run but got none", test)
	}
}

func TestDriverEmpty(t *testing.T) {
	m := &mock{t: t}
	d := &Driver{
		Analyzer:    m,
		WriteOutput: m.out(),
	}
	testutil.Fatalf(t, "Driver error: %v", d.Run(context.Background(), m))
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
		Analyzer:    m,
		WriteOutput: m.out(),
		Context: testContext{
			setup: func(_ context.Context, cu Compilation) error {
				setupIdx++
				return nil
			},
			teardown: func(_ context.Context, cu Compilation) error {
				if setupIdx != teardownIdx+1 {
					t.Error("Teardown was not called directly after Setup/Analyze")
				}
				teardownIdx++
				return nil
			},
			analysisError: func(_ context.Context, _ Compilation, err error) error {
				// Compilations that do not report an error should not call this hook.
				t.Errorf("Unexpected call of AnalysisError hook with %v", err)
				return err
			},
		},
	}
	testutil.Fatalf(t, "Driver error: %v", d.Run(context.Background(), m))
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
		Analyzer:    m,
		WriteOutput: m.out(),
	}
	if err := d.Run(context.Background(), m); err != errFromAnalysis {
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
		Analyzer:    m,
		WriteOutput: m.out(),
		Context: testContext{
			analysisError: func(_ context.Context, cu Compilation, err error) error {
				analysisErr = err
				return nil // don't return err
			},
		},
	}
	testutil.Fatalf(t, "Driver error: %v", d.Run(context.Background(), m))
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
		Analyzer:    m,
		WriteOutput: m.out(),
		Context: testContext{
			setup: func(_ context.Context, cu Compilation) error {
				setupIdx++
				var missing []string
				if cu.UnitDigest == "" {
					missing = append(missing, "unit digest")
				}
				if cu.Revision == "" {
					missing = append(missing, "revision marker")
				}
				if cu.BuildID == "" {
					missing = append(missing, "build ID")
				}
				if len(missing) != 0 {
					return fmt.Errorf("missing %s: %v", strings.Join(missing, ", "), cu)
				}
				return nil
			},
		},
	}
	testutil.Fatalf(t, "Driver error: %v", d.Run(context.Background(), m))
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
		Analyzer:    m,
		WriteOutput: m.out(),
		Context: testContext{
			teardown: func(_ context.Context, cu Compilation) error {
				teardownIdx++
				return nil
			},
		},
	}
	testutil.Fatalf(t, "Driver error: %v", d.Run(context.Background(), m))
	if len(m.Requests) != len(m.Compilations) {
		t.Errorf("Expected %d AnalysisRequests; found %v", len(m.Compilations), m.Requests)
	}
	if teardownIdx != len(m.Compilations) {
		t.Errorf("Expected %d calls to Teardown; found %d", len(m.Compilations), teardownIdx)
	}
}

func TestDriverTimeout(t *testing.T) {
	timeout := 10 * time.Millisecond
	m := &mock{
		t:            t,
		Outputs:      outs("a", "b", "c"),
		Compilations: comps("target1", "target2"),

		AnalysisDuration: timeout * 3,
	}
	d := &Driver{
		Analyzer:        m,
		AnalysisOptions: AnalysisOptions{Timeout: timeout},
		WriteOutput:     m.out(),
		Context:         testContext{},
	}
	if err := d.Run(context.Background(), m); !errors.Is(err, context.DeadlineExceeded) {
		t.Errorf("Expected error %v; found %v", context.DeadlineExceeded, err)
	}
	if len(m.Requests) != 1 {
		t.Errorf("Expected 1 AnalysisRequest; found %v", m.Requests)
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
			Unit:       &apb.CompilationUnit{VName: &spb.VName{Signature: sig}},
			Revision:   "12345",
			UnitDigest: "digest:" + sig,
			BuildID:    buildID,
		})
	}
	return
}
