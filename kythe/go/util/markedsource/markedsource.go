/*
 * Copyright 2016 The Kythe Authors. All rights reserved.
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
package markedsource // import "kythe.io/kythe/go/util/markedsource"

import (
	"fmt"
	"html"
	"regexp"
	"strings"

	"kythe.io/kythe/go/util/md"

	"github.com/JohannesKaufmann/html-to-markdown/escape"

	cpb "kythe.io/kythe/proto/common_go_proto"
)

// maxRenderDepth cuts the render algorithm if it recurses too deeply into a
// nested MarkedSource.  The resulting identifiers will thus be partial.  This
// value matches the kMaxRenderDepth in the C++ implementation.
const maxRenderDepth = 10

const invalidLookupMarker = "???"

// ContentType is a type of rendering output.
type ContentType string

// Supported list of ContentTypes
var (
	PlaintextContent ContentType = "txt"
	MarkdownContent  ContentType = "md"
)

// content holds text and a possible destination to help us build markdown links.
type content struct {
	text   string
	target string
}

// build creates markdown for c. If there is no target, the plain text is returned, if there is a
// target, a markdown link is returned.
func (c content) build(format ContentType) string {
	if format == MarkdownContent {
		if c.target == "" {
			return c.text
		}
		return md.Link(c.text, c.target)
	}

	// Assume plain text if markdown isn't set.
	return c.text
}

// escapeMD escapes the signature so that 1) generics don't get parsed as HTML by
// html-to-markdown in the Javadoc parser and 2) markdown characters in code (e.g. *) don't change
// how the text is displayed.
func escapeMD(s string) string {
	return escape.MarkdownCharacters(html.EscapeString(s))
}

type renderer struct {
	buffer            *content
	prependBuffer     string
	bufferIsNonempty  bool
	bufferEndsInSpace bool
	format            ContentType
	prependSpace      bool

	linkify func(string) string
}

// Render flattens MarkedSource to a string using reasonable defaults.
func Render(ms *cpb.MarkedSource) string {
	enabled := map[cpb.MarkedSource_Kind]bool{}
	for _, k := range cpb.MarkedSource_Kind_value {
		enabled[cpb.MarkedSource_Kind(k)] = true
	}
	r := &renderer{
		buffer: &content{},
		format: PlaintextContent,
	}

	r.renderRoot(ms, enabled)
	return r.buffer.build(r.format)
}

// RenderSignature renders the full signature from node.
// If not nil, linkify will be used to
// generate link URIs from semantic node tickets. It may return an empty string if there is no
// available URI.
func RenderSignature(node *cpb.MarkedSource, format ContentType, linkify func(string) string) string {
	enabled := map[cpb.MarkedSource_Kind]bool{
		cpb.MarkedSource_IDENTIFIER: true,
		cpb.MarkedSource_TYPE:       true,
		cpb.MarkedSource_PARAMETER:  true,
		cpb.MarkedSource_MODIFIER:   true,
	}
	r := &renderer{
		buffer:  &content{},
		linkify: linkify,
		format:  format,
	}

	r.renderRoot(node, enabled)
	return r.buffer.build(r.format)
}

// RenderCallSiteSignature returns the text snippet for a callsite as plaintext.
func RenderCallSiteSignature(node *cpb.MarkedSource) string {
	// This is similar to RenderSignature but does not render types to keep the text a little more
	// compact.
	enabled := map[cpb.MarkedSource_Kind]bool{
		cpb.MarkedSource_IDENTIFIER: true,
		cpb.MarkedSource_PARAMETER:  true,
	}
	r := &renderer{
		buffer:  &content{},
		linkify: nil,
		format:  PlaintextContent,
	}

	r.renderRoot(node, enabled)
	return r.buffer.build(r.format)
}

// RenderInitializer extracts and renders a plaintext initializer from node. If not nil, linkify
// will be used to generate link URIs from semantic node tickets. It may return an empty string if
// there is no available URI.
func RenderInitializer(node *cpb.MarkedSource, format ContentType, linkify func(string) string) string {
	enabled := map[cpb.MarkedSource_Kind]bool{
		cpb.MarkedSource_INITIALIZER: true,
	}
	r := &renderer{
		buffer:  &content{},
		linkify: linkify,
		format:  format,
	}

	r.renderRoot(node, enabled)
	return r.buffer.build(r.format)
}

// RenderSimpleQualifiedName extracts and renders the simple qualified name from node. If
// includeIdentifier is true it includes the identifier on the qualified name. If not nil, linkify
// will be used to generate link URIs from semantic node tickets. It may return an empty string if
// there is no available URI.
func RenderSimpleQualifiedName(node *cpb.MarkedSource, includeIdentifier bool, format ContentType, linkify func(string) string) string {
	enabled := map[cpb.MarkedSource_Kind]bool{
		cpb.MarkedSource_CONTEXT: true,
	}
	if includeIdentifier {
		enabled[cpb.MarkedSource_IDENTIFIER] = true
	}
	r := &renderer{
		buffer:  &content{},
		linkify: linkify,
		format:  format,
	}

	r.renderRoot(node, enabled)
	return r.buffer.build(r.format)
}

// RenderSimpleIdentifier extracts and renders a the simple identifier from node. If not nil,
// linkify will be used to generate link URIs from semantic node tickets. It may return an empty
// string if there is no available URI.
func RenderSimpleIdentifier(node *cpb.MarkedSource, format ContentType, linkify func(string) string) string {
	enabled := map[cpb.MarkedSource_Kind]bool{
		cpb.MarkedSource_IDENTIFIER: true,
	}
	r := &renderer{
		buffer:  &content{},
		linkify: linkify,
		format:  format,
	}

	r.renderRoot(node, enabled)
	return r.buffer.build(r.format)
}

// RenderSimpleParams extracts and renders the simple identifiers for parameters in node. If not
// nil, linkify will be used to generate link URIs from semantic node tickets. It may return an
// empty string if there is no available URI.
func RenderSimpleParams(node *cpb.MarkedSource, format ContentType, linkify func(string) string) []string {
	return renderSimpleParams(node, format, linkify, 0)
}

func renderSimpleParams(node *cpb.MarkedSource, format ContentType, linkify func(string) string, level int) []string {
	if level >= maxRenderDepth {
		return nil
	}

	var out []string
	switch node.GetKind() {
	case cpb.MarkedSource_BOX:
		for _, child := range node.GetChild() {
			out = append(out, renderSimpleParams(child, format, linkify, level+1)...)
		}
	case cpb.MarkedSource_PARAMETER:
		for _, child := range node.GetChild() {
			enabled := map[cpb.MarkedSource_Kind]bool{
				cpb.MarkedSource_IDENTIFIER: true,
			}
			r := &renderer{
				buffer:  &content{},
				linkify: linkify,
				format:  format,
			}
			r.renderChild(child, enabled, make(map[cpb.MarkedSource_Kind]bool), level+1)
			out = append(out, r.buffer.build(r.format))
		}
	case
		cpb.MarkedSource_PARAMETER_LOOKUP_BY_PARAM,
		cpb.MarkedSource_PARAMETER_LOOKUP_BY_PARAM_WITH_DEFAULTS,
		cpb.MarkedSource_PARAMETER_LOOKUP_BY_TPARAM:
		out = append(out, renderInvalidLookup(node, format))
	}

	return out
}

func shouldRenderInvalidLookup(k cpb.MarkedSource_Kind, enabled map[cpb.MarkedSource_Kind]bool) bool {
	switch k {
	case
		cpb.MarkedSource_PARAMETER_LOOKUP_BY_PARAM,
		cpb.MarkedSource_PARAMETER_LOOKUP_BY_PARAM_WITH_DEFAULTS,
		cpb.MarkedSource_PARAMETER_LOOKUP_BY_TPARAM,
		cpb.MarkedSource_LOOKUP_BY_PARAM,
		cpb.MarkedSource_LOOKUP_BY_TPARAM:
		return enabled[cpb.MarkedSource_PARAMETER]
	case cpb.MarkedSource_LOOKUP_BY_TYPED:
		return enabled[cpb.MarkedSource_TYPE]
	default:
		return false
	}
}

func renderInvalidLookup(node *cpb.MarkedSource, format ContentType) string {
	return fmt.Sprintf("%s%s%s", nodePreText(node, format), invalidLookupMarker, nodePostText(node, format))
}

func willRender(node *cpb.MarkedSource, enabled, under map[cpb.MarkedSource_Kind]bool, level int) bool {
	kind := node.GetKind()
	return level < maxRenderDepth && (kind == cpb.MarkedSource_BOX || kind == cpb.MarkedSource_IDENTIFIER || shouldRenderInvalidLookup(kind, enabled) || enabled[kind])
}

func (r *renderer) renderRoot(node *cpb.MarkedSource, enabled map[cpb.MarkedSource_Kind]bool) {
	r.renderChild(node, enabled, map[cpb.MarkedSource_Kind]bool{}, 0)
}

// renderChild renders the children of node up to maxRenderDepth. It only renders nodes that are
// present in enabled.
func (r *renderer) renderChild(node *cpb.MarkedSource, enabled, under map[cpb.MarkedSource_Kind]bool, level int) {
	if level >= maxRenderDepth {
		return
	}
	kind := node.GetKind()
	if kind != cpb.MarkedSource_BOX || enabled[cpb.MarkedSource_BOX] {
		if shouldRenderInvalidLookup(kind, enabled) {
			r.add(renderInvalidLookup(node, r.format))
			return
		}
		if kind != cpb.MarkedSource_IDENTIFIER && !enabled[kind] {
			return
		}
		// Make a copy of under for our recursive tree and add in our new kind.
		newUnder := make(map[cpb.MarkedSource_Kind]bool)
		for k, v := range under {
			newUnder[k] = v
		}
		newUnder[kind] = true
		under = newUnder
	}
	var savedContent *content
	if shouldRender(enabled, under) {
		l := r.linkForSource(node)
		if l != "" && r.buffer != nil {
			savedContent = r.buffer
			r.buffer = &content{target: l}
		}
		r.add(nodePreText(node, r.format))
	}
	lastRenderedChild := -1
	for i, c := range node.GetChild() {
		if willRender(c, enabled, under, level+1) {
			lastRenderedChild = i
		}
	}
	for i, c := range node.GetChild() {
		if willRender(c, enabled, under, level+1) {
			r.renderChild(c, enabled, under, level+1)
			if lastRenderedChild > i {
				r.add(nodePostChildText(node, r.format))
			} else if node.GetAddFinalListToken() {
				r.addFinalListToken(nodePostChildText(node, r.format))
			}
		}
	}
	if shouldRender(enabled, under) {
		r.add(nodePostText(node, r.format))
		if savedContent != nil {
			r.buffer = &content{
				text: savedContent.build(r.format) + r.buffer.build(r.format),
			}
		}
		if node.GetKind() == cpb.MarkedSource_TYPE {
			r.prependSpace = true
		}
	}
}

func nodePreText(node *cpb.MarkedSource, format ContentType) string {
	if format == MarkdownContent {
		return escapeMD(node.GetPreText())
	}
	return node.GetPreText()
}

func nodePostText(node *cpb.MarkedSource, format ContentType) string {
	if format == MarkdownContent {
		return escapeMD(node.GetPostText())
	}
	return node.GetPostText()
}

func nodePostChildText(node *cpb.MarkedSource, format ContentType) string {
	if format == MarkdownContent {
		return escapeMD(node.GetPostChildText())
	}
	return node.GetPostChildText()
}

func shouldRender(enabled, under map[cpb.MarkedSource_Kind]bool) (v bool) {
	for kind := range enabled {
		if under[kind] {
			return true
		}
	}
	return false
}

// linkForSource uses linkify (if present) to turn the link in node into a like that can be used in
// the documentation text.
func (r renderer) linkForSource(node *cpb.MarkedSource) string {
	if r.linkify == nil {
		return ""
	}

	for _, link := range node.GetLink() {
		for _, def := range link.GetDefinition() {
			if l := r.linkify(def); l != "" {
				return l
			}
		}
	}
	return ""
}

var excludedSpaceCharacters = regexp.MustCompile(`^[),\]>\s].*`)

// add appends s to the current buffer text.
func (r *renderer) add(s string) {
	if r.prependBuffer != "" && s != "" {
		r.buffer.text += r.prependBuffer
		r.bufferEndsInSpace = strings.HasSuffix(r.prependBuffer, " ")
		r.prependBuffer = ""
		r.bufferIsNonempty = true
	}
	if r.prependSpace && s != "" && r.bufferIsNonempty && !r.bufferEndsInSpace && !excludedSpaceCharacters.MatchString(s) {
		r.buffer.text += " "
		r.bufferEndsInSpace = true
	}
	r.buffer.text += s
	if s != "" {
		r.bufferEndsInSpace = strings.HasSuffix(s, " ")
		r.bufferIsNonempty = true
		r.prependSpace = false
	}
}

func (r *renderer) addFinalListToken(s string) {
	r.prependBuffer += s
}

// RenderQualifiedName renders a language-appropriate qualified name from a
// MarkedSource message.
func RenderQualifiedName(ms *cpb.MarkedSource) *cpb.SymbolInfo {
	id := firstMatching(ms, func(ms *cpb.MarkedSource) bool {
		return ms.Kind == cpb.MarkedSource_IDENTIFIER && ms.PreText != ""
	})

	if id == nil {
		return new(cpb.SymbolInfo)
	}

	symbolInfo := &cpb.SymbolInfo{BaseName: id.PreText}

	ctx := firstMatching(ms, func(ms *cpb.MarkedSource) bool {
		return ms.Kind == cpb.MarkedSource_CONTEXT
	})

	if ctx != nil {
		delim := ctx.PostChildText
		if delim == "" {
			delim = "."
		}
		var quals []string
		for _, kid := range ctx.Child {
			if kid.Kind == cpb.MarkedSource_IDENTIFIER || kid.Kind == cpb.MarkedSource_BOX {
				if namespace := Render(kid); namespace != "" {
					quals = append(quals, namespace)
				}
			}
		}
		if pkg := strings.Join(quals, delim); pkg != "" {
			symbolInfo.QualifiedName = pkg + ctx.PostChildText + id.GetPreText()
		}
	}

	return symbolInfo
}

// firstMatching returns the first node in a breadth-first traversal of the
// children of ms, or ms itself, for which f reports true, or nil.
func firstMatching(ms *cpb.MarkedSource, f func(*cpb.MarkedSource) bool) *cpb.MarkedSource {
	if ms == nil {
		return nil
	}
	if f(ms) {
		return ms
	}
	for _, kid := range ms.Child {
		if f(kid) {
			return kid
		}
	}
	for _, kid := range ms.Child {
		if match := firstMatching(kid, f); match != nil {
			return match
		}
	}
	return nil
}
