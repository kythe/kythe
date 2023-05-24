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

package schema

// This is a generated file -- do not edit it by hand.
// Input file: kythe/proto/schema.proto

import scpb "kythe.io/kythe/proto/schema_go_proto"

var (
	nodeKinds = map[string]scpb.NodeKind{
		"anchor":       3,
		"constant":     4,
		"diagnostic":   5,
		"doc":          6,
		"file":         7,
		"function":     9,
		"google/gflag": 27,
		"interface":    8,
		"lookup":       10,
		"macro":        11,
		"meta":         12,
		"name":         13,
		"package":      14,
		"process":      15,
		"record":       16,
		"sum":          17,
		"symbol":       18,
		"talias":       19,
		"tapp":         20,
		"tbuiltin":     21,
		"tnominal":     22,
		"tsigma":       23,
		"tvar":         26,
		"variable":     24,
		"vcs":          25,
	}

	subkinds = map[string]scpb.Subkind{
		"category":        1,
		"class":           2,
		"constructor":     3,
		"destructor":      4,
		"enum":            5,
		"enumClass":       6,
		"field":           7,
		"implicit":        8,
		"import":          9,
		"initializer":     10,
		"local":           11,
		"local/parameter": 12,
		"method":          13,
		"namespace":       14,
		"struct":          15,
		"type":            16,
		"union":           17,
	}

	factNames = map[string]scpb.FactName{
		"/kythe/build/config":   19,
		"/kythe/code":           1,
		"/kythe/complete":       2,
		"/kythe/context/url":    3,
		"/kythe/details":        4,
		"/kythe/doc/uri":        5,
		"/kythe/label":          6,
		"/kythe/loc/end":        7,
		"/kythe/loc/start":      8,
		"/kythe/message":        9,
		"/kythe/node/kind":      10,
		"/kythe/param/default":  11,
		"/kythe/ruleclass":      12,
		"/kythe/snippet/end":    13,
		"/kythe/snippet/start":  14,
		"/kythe/subkind":        15,
		"/kythe/tag/deprecated": 20,
		"/kythe/text":           16,
		"/kythe/text/encoding":  17,
		"/kythe/visibility":     18,
	}

	edgeKinds = map[string]scpb.EdgeKind{
		"/kythe/edge/aliases":                  1,
		"/kythe/edge/aliases/root":             2,
		"/kythe/edge/annotatedby":              3,
		"/kythe/edge/bounded/lower":            4,
		"/kythe/edge/bounded/upper":            5,
		"/kythe/edge/childof":                  6,
		"/kythe/edge/childof/context":          7,
		"/kythe/edge/completedby":              49,
		"/kythe/edge/defines":                  10,
		"/kythe/edge/defines/binding":          11,
		"/kythe/edge/defines/implicit":         50,
		"/kythe/edge/depends":                  12,
		"/kythe/edge/documents":                13,
		"/kythe/edge/exports":                  14,
		"/kythe/edge/extends":                  15,
		"/kythe/edge/generates":                16,
		"/kythe/edge/imputes":                  17,
		"/kythe/edge/instantiates":             18,
		"/kythe/edge/instantiates/speculative": 19,
		"/kythe/edge/named":                    20,
		"/kythe/edge/overrides":                21,
		"/kythe/edge/overrides/root":           22,
		"/kythe/edge/overrides/transitive":     23,
		"/kythe/edge/param":                    24,
		"/kythe/edge/property/reads":           44,
		"/kythe/edge/property/writes":          45,
		"/kythe/edge/ref":                      25,
		"/kythe/edge/ref/call":                 26,
		"/kythe/edge/ref/call/implicit":        27,
		"/kythe/edge/ref/doc":                  28,
		"/kythe/edge/ref/expands":              29,
		"/kythe/edge/ref/expands/transitive":   30,
		"/kythe/edge/ref/file":                 31,
		"/kythe/edge/ref/id":                   46,
		"/kythe/edge/ref/implicit":             32,
		"/kythe/edge/ref/imports":              33,
		"/kythe/edge/ref/includes":             34,
		"/kythe/edge/ref/init":                 35,
		"/kythe/edge/ref/init/implicit":        36,
		"/kythe/edge/ref/queries":              37,
		"/kythe/edge/ref/writes":               47,
		"/kythe/edge/satisfies":                38,
		"/kythe/edge/specializes":              39,
		"/kythe/edge/specializes/speculative":  40,
		"/kythe/edge/tagged":                   41,
		"/kythe/edge/tparam":                   48,
		"/kythe/edge/typed":                    42,
		"/kythe/edge/undefines":                43,
	}

	nodeKindsRev = map[scpb.NodeKind]string{
		3:  "anchor",
		4:  "constant",
		5:  "diagnostic",
		6:  "doc",
		7:  "file",
		8:  "interface",
		9:  "function",
		10: "lookup",
		11: "macro",
		12: "meta",
		13: "name",
		14: "package",
		15: "process",
		16: "record",
		17: "sum",
		18: "symbol",
		19: "talias",
		20: "tapp",
		21: "tbuiltin",
		22: "tnominal",
		23: "tsigma",
		24: "variable",
		25: "vcs",
		26: "tvar",
		27: "google/gflag",
	}

	subkindsRev = map[scpb.Subkind]string{
		1:  "category",
		2:  "class",
		3:  "constructor",
		4:  "destructor",
		5:  "enum",
		6:  "enumClass",
		7:  "field",
		8:  "implicit",
		9:  "import",
		10: "initializer",
		11: "local",
		12: "local/parameter",
		13: "method",
		14: "namespace",
		15: "struct",
		16: "type",
		17: "union",
	}

	factNamesRev = map[scpb.FactName]string{
		1:  "/kythe/code",
		2:  "/kythe/complete",
		3:  "/kythe/context/url",
		4:  "/kythe/details",
		5:  "/kythe/doc/uri",
		6:  "/kythe/label",
		7:  "/kythe/loc/end",
		8:  "/kythe/loc/start",
		9:  "/kythe/message",
		10: "/kythe/node/kind",
		11: "/kythe/param/default",
		12: "/kythe/ruleclass",
		13: "/kythe/snippet/end",
		14: "/kythe/snippet/start",
		15: "/kythe/subkind",
		16: "/kythe/text",
		17: "/kythe/text/encoding",
		18: "/kythe/visibility",
		19: "/kythe/build/config",
		20: "/kythe/tag/deprecated",
	}

	edgeKindsRev = map[scpb.EdgeKind]string{
		1:  "/kythe/edge/aliases",
		2:  "/kythe/edge/aliases/root",
		3:  "/kythe/edge/annotatedby",
		4:  "/kythe/edge/bounded/lower",
		5:  "/kythe/edge/bounded/upper",
		6:  "/kythe/edge/childof",
		7:  "/kythe/edge/childof/context",
		10: "/kythe/edge/defines",
		11: "/kythe/edge/defines/binding",
		12: "/kythe/edge/depends",
		13: "/kythe/edge/documents",
		14: "/kythe/edge/exports",
		15: "/kythe/edge/extends",
		16: "/kythe/edge/generates",
		17: "/kythe/edge/imputes",
		18: "/kythe/edge/instantiates",
		19: "/kythe/edge/instantiates/speculative",
		20: "/kythe/edge/named",
		21: "/kythe/edge/overrides",
		22: "/kythe/edge/overrides/root",
		23: "/kythe/edge/overrides/transitive",
		24: "/kythe/edge/param",
		25: "/kythe/edge/ref",
		26: "/kythe/edge/ref/call",
		27: "/kythe/edge/ref/call/implicit",
		28: "/kythe/edge/ref/doc",
		29: "/kythe/edge/ref/expands",
		30: "/kythe/edge/ref/expands/transitive",
		31: "/kythe/edge/ref/file",
		32: "/kythe/edge/ref/implicit",
		33: "/kythe/edge/ref/imports",
		34: "/kythe/edge/ref/includes",
		35: "/kythe/edge/ref/init",
		36: "/kythe/edge/ref/init/implicit",
		37: "/kythe/edge/ref/queries",
		38: "/kythe/edge/satisfies",
		39: "/kythe/edge/specializes",
		40: "/kythe/edge/specializes/speculative",
		41: "/kythe/edge/tagged",
		42: "/kythe/edge/typed",
		43: "/kythe/edge/undefines",
		44: "/kythe/edge/property/reads",
		45: "/kythe/edge/property/writes",
		46: "/kythe/edge/ref/id",
		47: "/kythe/edge/ref/writes",
		48: "/kythe/edge/tparam",
		49: "/kythe/edge/completedby",
		50: "/kythe/edge/defines/implicit",
	}
)

// NodeKind returns the schema enum for the given node kind.
func NodeKind(k string) scpb.NodeKind { return nodeKinds[k] }

// EdgeKind returns the schema enum for the given edge kind.
func EdgeKind(k string) scpb.EdgeKind { return edgeKinds[k] }

// FactName returns the schema enum for the given fact name.
func FactName(f string) scpb.FactName { return factNames[f] }

// Subkind returns the schema enum for the given subkind.
func Subkind(k string) scpb.Subkind { return subkinds[k] }

// NodeKindString returns the string representation of the given node kind.
func NodeKindString(k scpb.NodeKind) string { return nodeKindsRev[k] }

// EdgeKindString returns the string representation of the given edge kind.
func EdgeKindString(k scpb.EdgeKind) string { return edgeKindsRev[k] }

// FactNameString returns the string representation of the given fact name.
func FactNameString(f scpb.FactName) string { return factNamesRev[f] }

// SubkindString returns the string representation of the given subkind.
func SubkindString(k scpb.Subkind) string { return subkindsRev[k] }
