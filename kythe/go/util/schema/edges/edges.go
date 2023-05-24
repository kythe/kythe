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

// Package edges defines constants for Kythe edges.
package edges // import "kythe.io/kythe/go/util/schema/edges"

import (
	"regexp"
	"strconv"
	"strings"

	"kythe.io/kythe/go/util/schema"
)

// Prefix defines the common prefix for all Kythe edge kinds.
const Prefix = schema.Prefix + "edge/"

// Edge kind labels
const (
	ChildOf                 = Prefix + "childof"
	Extends                 = Prefix + "extends"
	ExtendsPrivate          = Prefix + "extends/private"
	ExtendsPrivateVirtual   = Prefix + "extends/private/virtual"
	ExtendsProtected        = Prefix + "extends/protected"
	ExtendsProtectedVirtual = Prefix + "extends/protected/virtual"
	ExtendsPublic           = Prefix + "extends/public"
	ExtendsPublicVirtual    = Prefix + "extends/public/virtual"
	ExtendsVirtual          = Prefix + "extends/virtual"
	Generates               = Prefix + "generates"
	Named                   = Prefix + "named"
	Overrides               = Prefix + "overrides"
	OverridesTransitive     = Prefix + "overrides/transitive"
	Param                   = Prefix + "param"
	Satisfies               = Prefix + "satisfies"
	TParam                  = Prefix + "tparam"
	Typed                   = Prefix + "typed"
)

// Edge kinds associated with anchors
const (
	Defines         = Prefix + "defines"
	DefinesBinding  = Prefix + "defines/binding"
	Documents       = Prefix + "documents"
	Ref             = Prefix + "ref"
	RefCall         = Prefix + "ref/call"
	RefImplicit     = Prefix + "ref/implicit"
	RefCallImplicit = Prefix + "ref/call/implicit"
	RefImports      = Prefix + "ref/imports"
	RefInit         = Prefix + "ref/init"
	RefInitImplicit = Prefix + "ref/init/implicit"
	RefWrites       = Prefix + "ref/writes"
	Tagged          = Prefix + "tagged"
)

// ParamIndex returns an edge label of the form "param.i" for the i given.
func ParamIndex(i int) string { return Param + "." + strconv.Itoa(i) }

// TParamIndex returns an edge label of the form "tparam.i" for the i given.
func TParamIndex(i int) string { return TParam + "." + strconv.Itoa(i) }

// revPrefix is used to distinguish reverse kinds from forward ones.
const revPrefix = "%"

// Mirror returns the opposite-directional edge label for kind.
func Mirror(kind string) string {
	if rev := strings.TrimPrefix(kind, revPrefix); rev != kind {
		return rev
	}
	return revPrefix + kind
}

// Canonical returns the canonical forward version of an edge kind.
func Canonical(kind string) string { return strings.TrimPrefix(kind, revPrefix) }

// IsForward reports whether kind is a forward edge kind.
func IsForward(kind string) bool { return !IsReverse(kind) }

// IsReverse reports whether kind is a reverse edge kind.
func IsReverse(kind string) bool { return strings.HasPrefix(kind, revPrefix) }

// IsVariant reports whether x is equal to or a subkind of y.
// For example, each of the following returns true:
//
//	IsVariant("/kythe/edge/defines/binding", "/kythe/edge/defines")
//	IsVariant("/kythe/edge/defines", "/kythe/edge/defines")
//
// Moreover IsVariant(x, y) == IsVariant(Mirror(x), Mirror(y)) for all x, y.
func IsVariant(x, y string) bool { return x == y || strings.HasPrefix(x, y+"/") }

// IsAnchorEdge reports whether kind is one associated with anchors.
func IsAnchorEdge(kind string) bool {
	canon := Canonical(kind)
	return IsVariant(canon, Defines) || IsVariant(canon, DefinesBinding) ||
		IsVariant(canon, Documents) ||
		IsVariant(canon, Ref) || IsVariant(canon, RefCall)
}

var ordinalKind = regexp.MustCompile(`^(.+)\.(\d+)$`)

// ParseOrdinal reports whether kind has an ordinal suffix (.nnn), and if so,
// returns the value of the ordinal as an integer. If not, the original kind is
// returned as written and ordinal == 0.
func ParseOrdinal(kind string) (base string, ordinal int, hasOrdinal bool) {
	m := ordinalKind.FindStringSubmatch(kind)
	if m == nil {
		return kind, 0, false
	}
	ordinal, _ = strconv.Atoi(m[2])
	return m[1], ordinal, true
}

// OrdinalKind reports whether kind (which does not have an ordinal suffix)
// generally has an associated ordinal (e.g. /kythe/edge/param edges).
func OrdinalKind(kind string) bool {
	switch Canonical(kind) {
	case Param:
		return true
	default:
		return false
	}
}
