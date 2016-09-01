/*
 * Copyright 2016 Google Inc. All rights reserved.
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

package kythe

import (
	"reflect"
	"sort"
	"testing"

	apb "kythe.io/kythe/proto/analysis_proto"
	spb "kythe.io/kythe/proto/storage_proto"
)

func TestIndexing(t *testing.T) {
	unit := Unit{&apb.CompilationUnit{
		VName: &spb.VName{
			Signature: "T",
			Language:  "c++",
		},
		RequiredInput: []*apb.CompilationUnit_FileInput{{
			VName: &spb.VName{Path: "p1"},
			Info:  &apb.FileInfo{Path: "../p1", Digest: "d1"},
		}, {
			Info: &apb.FileInfo{Path: "p2", Digest: "d2"},
		}},
		SourceFile: []string{"S"},
		OutputKey:  "O",
	}}

	idx := unit.Index()
	sort.Strings(idx.Inputs)
	if got, want := idx.Language, "c++"; got != want {
		t.Errorf("%T Language: got %q, want %q", unit, got, want)
	}
	if got, want := idx.Output, "O"; got != want {
		t.Errorf("%T Output: got %q, want %q", unit, got, want)
	}
	if got, want := idx.Inputs, []string{"d1", "d2"}; !reflect.DeepEqual(got, want) {
		t.Errorf("%T Inputs: got %+q, want %+q", unit, got, want)
	}
	if got, want := idx.Sources, []string{"S"}; !reflect.DeepEqual(got, want) {
		t.Errorf("%T Sources: got %+q, want %+q", unit, got, want)
	}
	if got, want := idx.Target, "T"; got != want {
		t.Errorf("%T Target: got %q, want %q", unit, got, want)
	}
}
