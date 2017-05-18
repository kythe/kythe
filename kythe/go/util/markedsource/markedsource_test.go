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

package markedsource

import (
	"testing"

	cpb "kythe.io/kythe/proto/common_proto"
)

func TestRender(t *testing.T) {
	tests := []struct {
		in  *cpb.MarkedSource
		out string
	}{
		{&cpb.MarkedSource{}, ""},
		{&cpb.MarkedSource{PreText: "PRE", PostText: "POST"}, "PREPOST"},
		{&cpb.MarkedSource{PostChildText: ","}, ""},
		{&cpb.MarkedSource{PostChildText: ",", AddFinalListToken: true}, ""},
		{&cpb.MarkedSource{PreText: "PRE", PostText: "POST", PostChildText: ",",
			Child: []*cpb.MarkedSource{{PreText: "C1"}}}, "PREC1POST"},
		{&cpb.MarkedSource{PreText: "PRE", PostText: "POST", PostChildText: ",", AddFinalListToken: true,
			Child: []*cpb.MarkedSource{{PreText: "C1"}}}, "PREC1,POST"},
		{&cpb.MarkedSource{PreText: "PRE", PostText: "POST", PostChildText: ",",
			Child: []*cpb.MarkedSource{{PreText: "C1"}, {PreText: "C2"}}}, "PREC1,C2POST"},
		{&cpb.MarkedSource{PreText: "PRE", PostText: "POST", PostChildText: ",", AddFinalListToken: true,
			Child: []*cpb.MarkedSource{{PreText: "C1"}, {PreText: "C2"}}}, "PREC1,C2,POST"},
		{&cpb.MarkedSource{PreText: "PRE", PostChildText: ",", AddFinalListToken: true,
			Child: []*cpb.MarkedSource{{PreText: "C1"}, {PreText: "C2"}}}, "PREC1,C2,"},
	}
	for _, test := range tests {
		if got := Render(test.in); got != test.out {
			t.Errorf("from %v: got %q, expected %q", test.in, got, test.out)
		}
	}
}
