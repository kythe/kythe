/*
 * Copyright 2014 Google Inc. All rights reserved.
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

// Package schema defines constants used in the Kythe schema.
package schema

import (
	"regexp"
	"strconv"
	"strings"
)

// Kythe node fact labels
const (
	NodeKindFact = "/kythe/node/kind"
	SubkindFact  = "/kythe/subkind"

	AnchorStartFact = "/kythe/loc/start"
	AnchorEndFact   = "/kythe/loc/end"

	SnippetStartFact = "/kythe/snippet/start"
	SnippetEndFact   = "/kythe/snippet/end"

	TextFact         = "/kythe/text"
	TextEncodingFact = "/kythe/text/encoding"

	CompleteFact = "/kythe/complete"

	FormatFact = "/kythe/format"
)

// DefaultTextEncoding is the assumed value of the TextEncodingFact if it is
// empty or missing from a node with a TextFact.
const DefaultTextEncoding = "UTF-8"

// Kythe node kinds
const (
	AnchorKind = "anchor"
	FileKind   = "file"
	NameKind   = "name"

	DocKind      = "doc"
	EnumKind     = "enum"
	FunctionKind = "function"
	PackageKind  = "package"
	RecordKind   = "record"
	TAppKind     = "tapp"
	VariableKind = "variable"
)

// Kythe node subkinds
const (
	ClassSubkind     = "class"
	EnumClassSubkind = "enumClass"
	ImplicitSubkind  = "implicit"
)

// EdgePrefix is the standard Kythe prefix for all edge kinds.
const EdgePrefix = "/kythe/edge/"

// Kythe edge kinds
const (
	ChildOfEdge   = EdgePrefix + "childof"
	NamedEdge     = EdgePrefix + "named"
	OverridesEdge = EdgePrefix + "overrides"
	ParamEdge     = EdgePrefix + "param"
	TypedEdge     = EdgePrefix + "typed"
)

// Kythe edge kinds associated with anchors
const (
	CompletesEdge         = EdgePrefix + "completes"
	CompletesUniquelyEdge = EdgePrefix + "completes/uniquely"
	DefinesEdge           = EdgePrefix + "defines"
	DefinesBindingEdge    = EdgePrefix + "defines/binding"
	DocumentsEdge         = EdgePrefix + "documents"
	RefEdge               = EdgePrefix + "ref"
	RefCallEdge           = EdgePrefix + "ref/call"
)

const (
	// AnchorLocFilter is a fact filter for anchor locations
	AnchorLocFilter = "/kythe/loc/*"

	// SnippetLocFilter is a fact filter for snippet locations
	SnippetLocFilter = "/kythe/snippet/*"
)

// reverseEdgePrefix is the Kythe edgeKind prefix for reverse edges.  Edge kinds
// must be prefixed at most once with this string.
const reverseEdgePrefix = "%"

// EdgeDir represents the inherent direction of an edge kind.
type EdgeDir bool

// Forward edges are generally dependency edges and ensure that each node has a
// small out-degree in the Kythe graph.  Reverse edges are the opposite.
const (
	Forward EdgeDir = true
	Reverse EdgeDir = false
)

// IsEdgeVariant returns if k1 is the same edge kind as k2 or a more specific
// variant
// (i.e. IsEdgeVariant("/kythe/edge/defines/binding", "/kythe/edge/defines") == true).
// Note that
// IsEdgeVariant(k1, k2) == IsEdgeVariant(MirrorEdge(k1), MirrorEdge(k2)).
func IsEdgeVariant(k1, k2 string) bool { return k1 == k2 || strings.HasPrefix(k1, k2+"/") }

// IsAnchorEdge returns if the given edge kind is associated with anchors.
func IsAnchorEdge(kind string) bool {
	kind = Canonicalize(kind)
	return IsEdgeVariant(kind, DefinesEdge) || IsEdgeVariant(kind, DocumentsEdge) || IsEdgeVariant(kind, RefEdge) || IsEdgeVariant(kind, CompletesEdge)
}

// EdgeDirection returns the edge direction of the given edge kind
func EdgeDirection(kind string) EdgeDir {
	if strings.HasPrefix(kind, reverseEdgePrefix) {
		return Reverse
	}
	return Forward
}

// MirrorEdge returns the reverse edge kind for a given forward edge kind and
// returns the forward edge kind for a given reverse edge kind.
func MirrorEdge(kind string) string {
	if rev := strings.TrimPrefix(kind, reverseEdgePrefix); rev != kind {
		return rev
	}
	return reverseEdgePrefix + kind
}

// Canonicalize will return the canonical, forward version of an edge kind.
func Canonicalize(kind string) string {
	if EdgeDirection(kind) == Reverse {
		return MirrorEdge(kind)
	}
	return kind
}

var ordinalRE = regexp.MustCompile(`^(.+)\.(\d+)$`)

// ParseOrdinal removes an edge kind's `\.[0-9]+` ordinal suffix.
func ParseOrdinal(edgeKind string) (kind string, ordinal int, hasOrdinal bool) {
	match := ordinalRE.FindStringSubmatch(edgeKind)
	if match == nil {
		return edgeKind, 0, false
	}
	ordinal, _ = strconv.Atoi(match[2])
	return match[1], ordinal, true
}
