/*
 * Copyright 2019 The Kythe Authors. All rights reserved.
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

package info

import (
	"testing"

	"kythe.io/kythe/go/util/compare"

	apb "kythe.io/kythe/proto/analysis_go_proto"
)

func TestMergeKzipInfo(t *testing.T) {
	infos := []*apb.KzipInfo{
		{
			Corpora: map[string]*apb.KzipInfo_CorpusInfo{
				"corpus1": {
					LanguageCompilationUnits: map[string]*apb.KzipInfo_CorpusInfo_CompilationUnits{
						"python": {
							Count:          1,
							Sources:        1,
							RequiredInputs: 2,
						},
					},
				},
			},
		},
		{
			Corpora: map[string]*apb.KzipInfo_CorpusInfo{
				"corpus1": {
					LanguageCompilationUnits: map[string]*apb.KzipInfo_CorpusInfo_CompilationUnits{
						"python": {
							Count:          1,
							Sources:        0,
							RequiredInputs: 0,
						},
						"go": {
							Count:          1,
							Sources:        0,
							RequiredInputs: 2,
						},
					},
				},
				"corpus2": {
					LanguageCompilationUnits: map[string]*apb.KzipInfo_CorpusInfo_CompilationUnits{
						"python": {
							Count:          4,
							Sources:        5,
							RequiredInputs: 9,
						},
						"java": {
							Count:          3,
							Sources:        3,
							RequiredInputs: 20,
						},
					},
				},
			},
		},
	}

	want := &apb.KzipInfo{
		Corpora: map[string]*apb.KzipInfo_CorpusInfo{
			"corpus1": {
				LanguageCompilationUnits: map[string]*apb.KzipInfo_CorpusInfo_CompilationUnits{
					"python": {
						Count:          2,
						Sources:        1,
						RequiredInputs: 2,
					},
					"go": {
						Count:          1,
						Sources:        0,
						RequiredInputs: 2,
					},
				},
			},
			"corpus2": {
				LanguageCompilationUnits: map[string]*apb.KzipInfo_CorpusInfo_CompilationUnits{
					"python": {
						Count:          4,
						Sources:        5,
						RequiredInputs: 9,
					},
					"java": {
						Count:          3,
						Sources:        3,
						RequiredInputs: 20,
					},
				},
			},
		},
	}
	wantTotal := apb.KzipInfo_CorpusInfo{
		LanguageCompilationUnits: map[string]*apb.KzipInfo_CorpusInfo_CompilationUnits{
			"python": {
				Count:          6,
				Sources:        6,
				RequiredInputs: 11,
			},
			"go": {
				Count:          1,
				Sources:        0,
				RequiredInputs: 2,
			},
			"java": {
				Count:          3,
				Sources:        3,
				RequiredInputs: 20,
			},
		},
	}

	got := MergeKzipInfo(infos)
	gotTotal := KzipInfoTotalCount(infos)
	if diff := compare.ProtoDiff(got, want); diff != "" {
		t.Errorf("Merged kzips don't match: (-: found, +: expected)\n%s", diff)
	}
	if diff := compare.ProtoDiff(&gotTotal, &wantTotal); diff != "" {
		t.Errorf("got %v, want %v", gotTotal, wantTotal)
		t.Errorf("Merged kzips don't match: (-: found, +: expected)\n%s", diff)
	}
}
