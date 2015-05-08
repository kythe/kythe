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

import "strings"

// Kythe node fact labels
const (
	NodeKindFact = "/kythe/node/kind"
	SubkindFact  = "/kythe/subkind"

	AnchorStartFact = "/kythe/loc/start"
	AnchorEndFact   = "/kythe/loc/end"

	TextFact         = "/kythe/text"
	TextEncodingFact = "/kythe/text/encoding"
)

// Kythe node kinds
const (
	AnchorKind = "anchor"
	FileKind   = "file"
	NameKind   = "name"

	EnumKind     = "enum"
	FunctionKind = "function"
	PackageKind  = "package"
	RecordKind   = "record"
	VariableKind = "variable"
)

// Kythe node subkinds
const (
	ClassSubkind     = "class"
	EnumClassSubkind = "enumClass"
)

// EdgePrefix is the standard Kythe prefix for all edge kinds.
const EdgePrefix = "/kythe/edge/"

// Kythe edge kinds
const (
	ChildOfEdge = EdgePrefix + "childof"
	DefinesEdge = EdgePrefix + "defines"
	NamedEdge   = EdgePrefix + "named"
	ParamEdge   = EdgePrefix + "param"
	RefEdge     = EdgePrefix + "ref"
)

// Fact filter for anchor locations
const AnchorLocFilter = "/kythe/loc/*"

// reverseEdgePrefix is the Kythe edgeKind prefix for reverse edges.  Edge kinds
// must be prefixed at most once with this string.
const reverseEdgePrefix = "%"

// EdgeDir represents the inherent direction of an edge kind.
type EdgeDir bool

// Forward edges are generally depedency edges and ensure that each node has a
// small out-degree in the Kythe graph.  Reverse edges are the opposite.
const (
	Forward EdgeDir = true
	Reverse EdgeDir = false
)

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
