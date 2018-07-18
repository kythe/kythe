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

package vnameutil

import (
	"strings"

	spb "kythe.io/kythe/proto/storage_go_proto"
)

// Compare reports the relative order of two vnames, returning -1 if a < b, 0
// if a == b, and 1 if a > b.
func Compare(a, b *spb.VName) int {
	for _, v := range [...]struct{ ap, bp *string }{
		{&a.Corpus, &b.Corpus},
		{&a.Language, &b.Language},
		{&a.Path, &b.Path},
		{&a.Root, &b.Root},
		{&a.Signature, &b.Signature},
	} {
		if n := strings.Compare(*v.ap, *v.bp); n != 0 {
			return n
		}
	}
	return 0
}
