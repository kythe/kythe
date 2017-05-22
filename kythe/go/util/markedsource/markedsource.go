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
	"io"

	cpb "kythe.io/kythe/proto/common_proto"
)

// maxRenderDepth cuts the render algorithm if it recurses too deeply into a
// nested MarkedSource.  The resulting identifiers will thus be partial.  This
// value matches the kMaxRenderDepth in the C++ implementation.
const maxRenderDepth = 10

type state struct {
	w       io.Writer
	depth   int
	inIdent bool
}

func render(ms *cpb.MarkedSource, st state) {
	if st.depth > maxRenderDepth {
		return
	}
	io.WriteString(st.w, ms.PreText)
	st.depth++
	for n, child := range ms.Child {
		render(child, st)
		if ms.AddFinalListToken || n < len(ms.Child)-1 {
			io.WriteString(st.w, ms.PostChildText)
		}
	}
	io.WriteString(st.w, ms.PostText)
}

// Render flattens MarkedSource to a string using reasonable defaults.
func Render(ms *cpb.MarkedSource) string {
	var buf bytes.Buffer
	render(ms, state{w: &buf})
	return buf.String()
}

// RenderSimpleIdentifier extracts and renders the simple identifier from a
// MarkedSource.
func RenderSimpleIdentifier(ms *cpb.MarkedSource) string {
	var buf bytes.Buffer
	renderIdent(ms, state{w: &buf})
	return buf.String()
}

func renderIdent(ms *cpb.MarkedSource, st state) {
	if st.depth >= maxRenderDepth {
		return
	}
	st.depth++

	// Anything other than identifiers and boxes can be skipped.
	switch ms.Kind {
	case cpb.MarkedSource_IDENTIFIER:
		st.inIdent = true
	case cpb.MarkedSource_BOX:
		// good; we can continue
	default:
		return
	}

	if st.inIdent {
		io.WriteString(st.w, ms.PreText)
	}
	for i, child := range ms.Child {
		renderIdent(child, st)
		if st.inIdent {
			if ms.AddFinalListToken || i < len(ms.Child)-1 {
				io.WriteString(st.w, ms.PostChildText)
			}
		}
	}
	if st.inIdent {
		io.WriteString(st.w, ms.PostText)
	}
}

// RenderSimpleParams extracts and renders the simple identifiers from a
// MarkedSource's parameters.
func RenderSimpleParams(ms *cpb.MarkedSource) []string { return renderParams(ms, state{}, nil) }

func renderParams(ms *cpb.MarkedSource, st state, params []string) []string {
	if st.depth >= maxRenderDepth {
		return nil
	}
	st.depth++

	switch ms.Kind {
	case cpb.MarkedSource_PARAMETER:
		for _, child := range ms.Child {
			params = append(params, RenderSimpleIdentifier(child))
		}
	case cpb.MarkedSource_BOX:
		for _, child := range ms.Child {
			params = append(params, renderParams(child, st, params)...)
		}
	default:
		// do nothing
	}
	return params
}
