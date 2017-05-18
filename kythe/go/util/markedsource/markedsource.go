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

// Package markedsource defines functions for rendering MarkedSource.
package markedsource

import (
	"bytes"

	cpb "kythe.io/kythe/proto/common_proto"
)

type context struct {
	buffer bytes.Buffer
}

// render flattens ms to the given context.
func (cxt *context) render(ms *cpb.MarkedSource) {
	cxt.buffer.WriteString(ms.PreText)
	if len(ms.Child) > 0 {
		for n, c := range ms.Child {
			cxt.render(c)
			if ms.AddFinalListToken || n < len(ms.Child)-1 {
				cxt.buffer.WriteString(ms.PostChildText)
			}
		}
	}
	cxt.buffer.WriteString(ms.PostText)
}

// Render flattens MarkedSource to a string using reasonable defaults.
func Render(ms *cpb.MarkedSource) string {
	cxt := &context{}
	cxt.render(ms)
	return cxt.buffer.String()
}
