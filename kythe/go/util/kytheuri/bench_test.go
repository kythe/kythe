/*
 * Copyright 2017 The Kythe Authors. All rights reserved.
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

package kytheuri

import "testing"

const benchURI = `kythe://some.long.nasty/corpus/label/from.hell%21%21?lang=winkerbean%2b%2b?path=miscdata/experiments/parthenon/studies/gaming/weeble_native_live_catastrophe.wb#IDENTIFIER%3AWeebleNativeLiveCatastrophe.Experiment.Gaming.weeble_native_live_catastrophe`

func BenchmarkParse(b *testing.B) {
	for i := 0; i < b.N; i++ {
		_, err := Parse(benchURI)
		if err != nil {
			panic(err)
		}
	}
}

func BenchmarkString(b *testing.B) {
	p, err := Parse(benchURI)
	if err != nil {
		panic(err)
	}
	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		_ = p.Encode().String()
	}
}
