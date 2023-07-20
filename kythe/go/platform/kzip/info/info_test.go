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
	"bytes"
	"testing"

	"kythe.io/kythe/go/platform/kzip"
	"kythe.io/kythe/go/util/compare"

	apb "kythe.io/kythe/proto/analysis_go_proto"
	spb "kythe.io/kythe/proto/storage_go_proto"
)

func TestMergeKzipInfo(t *testing.T) {
	infos := []*apb.KzipInfo{
		{
			Corpora: map[string]*apb.KzipInfo_CorpusInfo{
				"corpus1": {
					LanguageRequiredInputs: map[string]*apb.KzipInfo_CorpusInfo_Inputs{
						"python": {
							Count: 2,
						},
					},
					LanguageSources: map[string]*apb.KzipInfo_CorpusInfo_Inputs{
						"python": {
							Count: 1,
						},
					},
					LanguageCuInfo: map[string]*apb.KzipInfo_CorpusInfo_CUInfo{
						"python": {
							Count: 1,
						},
						"java": {
							Count: 20,
							JavaVersionCount: map[int32]int32{
								7: 10,
							},
						},
					},
				},
			},
			CriticalKzipErrors: []string{"foo.py doesn't have a required_input"},
			Size:               10,
		},
		{
			Corpora: map[string]*apb.KzipInfo_CorpusInfo{
				"corpus1": {
					LanguageRequiredInputs: map[string]*apb.KzipInfo_CorpusInfo_Inputs{
						"python": {
							Count: 1,
						},
					},
					LanguageCuInfo: map[string]*apb.KzipInfo_CorpusInfo_CUInfo{
						"python": {
							Count: 1,
						},
						"java": {
							Count: 2,
							JavaVersionCount: map[int32]int32{
								6: 1,
								7: 7,
							},
						},
					},
				},
				"corpus2": {
					LanguageSources: map[string]*apb.KzipInfo_CorpusInfo_Inputs{
						"python": {
							Count: 5,
						},
						"java": {
							Count: 3,
						},
					},
					LanguageCuInfo: map[string]*apb.KzipInfo_CorpusInfo_CUInfo{
						"java": {
							Count: 3,
							JavaVersionCount: map[int32]int32{
								4: 1,
							},
						},
					},
				},
				"unnamed_required_input_corpus": {
					LanguageRequiredInputs: map[string]*apb.KzipInfo_CorpusInfo_Inputs{
						"python": {
							Count: 9,
						},
						"java": {
							Count: 20,
						},
						"go": {
							Count: 2,
						},
					},
				},
			},
			CriticalKzipErrors: []string{"foo2.py doesn't have a required_input"},
			Size:               30,
		},
	}

	want := &apb.KzipInfo{
		Corpora: map[string]*apb.KzipInfo_CorpusInfo{
			"corpus1": {
				LanguageRequiredInputs: map[string]*apb.KzipInfo_CorpusInfo_Inputs{
					"python": {
						Count: 3,
					},
				},
				LanguageSources: map[string]*apb.KzipInfo_CorpusInfo_Inputs{
					"python": {
						Count: 1,
					},
				},
				LanguageCuInfo: map[string]*apb.KzipInfo_CorpusInfo_CUInfo{
					"python": {
						Count: 2,
					},
					"java": {
						Count: 22,
						JavaVersionCount: map[int32]int32{
							6: 1,
							7: 17,
						},
					},
				},
			},
			"corpus2": {
				LanguageSources: map[string]*apb.KzipInfo_CorpusInfo_Inputs{
					"python": {
						Count: 5,
					},
					"java": {
						Count: 3,
					},
				},
				LanguageCuInfo: map[string]*apb.KzipInfo_CorpusInfo_CUInfo{
					"java": {
						Count: 3,
						JavaVersionCount: map[int32]int32{
							4: 1,
						},
					},
				},
			},
			"unnamed_required_input_corpus": {
				LanguageRequiredInputs: map[string]*apb.KzipInfo_CorpusInfo_Inputs{
					"python": {
						Count: 9,
					},
					"java": {
						Count: 20,
					},
					"go": {
						Count: 2,
					},
				},
			},
		},
		CriticalKzipErrors: []string{"foo.py doesn't have a required_input", "foo2.py doesn't have a required_input"},
		Size:               40,
	}
	wantTotal := &apb.KzipInfo_CorpusInfo{
		LanguageRequiredInputs: map[string]*apb.KzipInfo_CorpusInfo_Inputs{
			"python": {
				Count: 12,
			},
			"java": {
				Count: 20,
			},
			"go": {
				Count: 2,
			},
		},
		LanguageSources: map[string]*apb.KzipInfo_CorpusInfo_Inputs{
			"python": {
				Count: 6,
			},
			"java": {
				Count: 3,
			},
		},
		LanguageCuInfo: map[string]*apb.KzipInfo_CorpusInfo_CUInfo{
			"python": {
				Count: 2,
			},
			"java": {
				Count: 25,
				JavaVersionCount: map[int32]int32{
					4: 1,
					6: 1,
					7: 17,
				},
			},
		},
	}

	got := MergeKzipInfo(infos)
	gotTotal := KzipInfoTotalCount(infos)
	if diff := compare.ProtoDiff(got, want); diff != "" {
		t.Errorf("Merged kzips don't match: (-: found, +: expected)\n%s", diff)
	}
	if diff := compare.ProtoDiff(gotTotal, wantTotal); diff != "" {
		t.Errorf("got %v, want %v", gotTotal, wantTotal)
		t.Errorf("Merged kzips don't match: (-: found, +: expected)\n%s", diff)
	}
}

func makeKzip(t *testing.T, u *apb.CompilationUnit) *bytes.Reader {
	b := bytes.NewBuffer(nil)
	w, err := kzip.NewWriter(b)
	if err != nil {
		t.Fatalf("opening writer: %v", err)
	}
	if _, err = w.AddUnit(u, nil); err != nil {
		t.Fatalf("AddUnit: unexpected error: %v", err)
	}
	if err := w.Close(); err != nil {
		t.Fatalf("Writer.Close: unexpected error: %v", err)
	}
	return bytes.NewReader(b.Bytes())
}

var infoTests = []struct {
	name string
	in   *apb.CompilationUnit
	out  *apb.KzipInfo
}{
	{
		"required counts",
		&apb.CompilationUnit{
			VName: &spb.VName{Language: "python"},
			RequiredInput: []*apb.CompilationUnit_FileInput{
				{
					VName: &spb.VName{Corpus: "mycorpus", Language: "python", Path: "file1.py"},
					Info:  &apb.FileInfo{Path: "file1.py"},
				},
			},
		},
		&apb.KzipInfo{
			Corpora: map[string]*apb.KzipInfo_CorpusInfo{
				"mycorpus": {
					LanguageRequiredInputs: map[string]*apb.KzipInfo_CorpusInfo_Inputs{
						"python": {
							Count: 1,
						},
					},
				},
				"": {
					LanguageCuInfo: map[string]*apb.KzipInfo_CorpusInfo_CUInfo{
						"python": {
							Count: 1,
						},
					},
				},
			},
		},
	},

	{
		"No corpus for required input",
		&apb.CompilationUnit{
			VName: &spb.VName{Language: "python"},
			RequiredInput: []*apb.CompilationUnit_FileInput{
				{
					VName: &spb.VName{Language: "python", Path: "file1.py"},
					Info:  &apb.FileInfo{Path: "file1.py"},
				},
			},
		},
		&apb.KzipInfo{
			Corpora: map[string]*apb.KzipInfo_CorpusInfo{
				"": {
					LanguageCuInfo: map[string]*apb.KzipInfo_CorpusInfo_CUInfo{
						"python": {
							Count: 1,
						},
					},
				},
			},
			CriticalKzipErrors: []string{`unable to determine corpus for required_input "file1.py" in CU(language:"python")`},
		},
	},

	// kzip info checks that each source file has a corresponding required_input
	// entry. This test case checks that the required_input path is normalized
	// before doing the check.
	{
		"normalize paths",
		&apb.CompilationUnit{
			VName: &spb.VName{Language: "c++", Corpus: "kythe"},
			RequiredInput: []*apb.CompilationUnit_FileInput{
				{
					VName: &spb.VName{
						Language: "c++",
						Corpus:   "kythe",
						Path:     "kythe/cxx/verifier/parser.yy.cc",
						Root:     "bazel-out/bin",
					},
					Info: &apb.FileInfo{
						Path: "bazel-out/k8-fastbuild/bin/kythe/cxx/verifier/../../../../../../bazel-out/k8-fastbuild/bin/kythe/cxx/verifier/parser.yy.cc",
					},
				},
			},
			SourceFile: []string{"bazel-out/k8-fastbuild/bin/kythe/cxx/verifier/parser.yy.cc"},
		},
		&apb.KzipInfo{
			Corpora: map[string]*apb.KzipInfo_CorpusInfo{
				"kythe": {
					LanguageRequiredInputs: map[string]*apb.KzipInfo_CorpusInfo_Inputs{
						"c++": {
							Count: 1,
						},
					},
					LanguageSources: map[string]*apb.KzipInfo_CorpusInfo_Inputs{
						"c++": {
							Count: 1,
						},
					},
					LanguageCuInfo: map[string]*apb.KzipInfo_CorpusInfo_CUInfo{
						"c++": {
							Count: 1,
						},
					},
				},
			},
		},
	},
}

func TestInfo(t *testing.T) {
	for _, testcase := range infoTests {
		kz := makeKzip(t, testcase.in)
		got, err := KzipInfo(kz, 0)
		if err != nil {
			t.Error(err)
			continue
		}

		if diff := compare.ProtoDiff(testcase.out, got); diff != "" {
			t.Errorf("KzipInfo() output differs from expected for %s (-: found, +: expected)\n%s", testcase.name, diff)
		}
	}
}
