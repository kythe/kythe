/*
 * Copyright 2018 The Kythe Authors. All rights reserved.
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

package keys

import (
	"testing"

	"kythe.io/kythe/go/util/compare"

	spb "kythe.io/kythe/proto/storage_go_proto"
)

func TestVName(t *testing.T) {
	v := &spb.VName{
		Corpus:    "c",
		Root:      "r",
		Path:      "p",
		Language:  "l",
		Signature: "s",
	}

	key, err := Append(nil, v)
	if err != nil {
		t.Errorf("Error encoding VName %+v: %v", v, err)
	}

	var found spb.VName
	if remaining, err := Parse(string(key), &found); err != nil {
		t.Fatalf("Error parsing VName: %v", err)
	} else if remaining != "" {
		t.Errorf("Unexpected remaining key: %q", remaining)
	}

	if diff := compare.ProtoDiff(v, &found); diff != "" {
		t.Fatalf("VName differences: (- expected; found +)\n%s", diff)
	}
}
