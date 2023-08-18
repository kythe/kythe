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

package markedsource

import (
	"bufio"
	"bytes"
	"os"
	"os/exec"
	"path/filepath"
	"strconv"
	"strings"
	"testing"

	"google.golang.org/protobuf/encoding/prototext"
	"google.golang.org/protobuf/proto"

	cpb "kythe.io/kythe/proto/common_go_proto"
)

var docPath = filepath.Join(os.Getenv("RUNFILES_DIR"), "io_kythe/kythe/cxx/doc/doc")

type oracleResults struct {
	SimpleIdentifier string
	SimpleParams     []string
}

// runOracle executes the C++ doc utility with ms as its input.  The utility's
// output is then parsed and returned.
//
// Example utility output:
//
//	      RenderSimpleIdentifier: "hello world"
//	      RenderSimpleParams: "param"
//	RenderSimpleQualifiedName-ID: ""
//	RenderSimpleQualifiedName+ID: "hello world"
func runOracle(t *testing.T, ms *cpb.MarkedSource) *oracleResults {
	cmd := exec.Command(docPath, "--common_signatures")
	// The doc utility expects its stdin to be a single text-format MarkedSource
	// proto message.
	opts := prototext.MarshalOptions{Multiline: false}
	rec, err := opts.Marshal(ms)
	if err != nil {
		t.Fatal(err)
	}
	// The utility's output is a series of lines, one per rendering
	out := &bytes.Buffer{}
	cmd.Stdin = bytes.NewReader(rec)
	cmd.Stdout = out
	cmd.Stderr = os.Stderr
	if err := cmd.Run(); err != nil {
		t.Fatal(err)
	}
	var res oracleResults
	s := bufio.NewScanner(out)
	for s.Scan() {
		line := strings.TrimSpace(s.Text())
		if ident := strings.TrimPrefix(line, "RenderSimpleIdentifier: "); ident != line {
			res.SimpleIdentifier = strings.Trim(ident, `"`)
		} else if param := strings.TrimPrefix(line, "RenderSimpleParams: "); param != line {
			res.SimpleParams = append(res.SimpleParams, strings.Trim(param, `"`))
		} else {
			t.Logf("Skipping doc line: %q", line)
		}
	}
	if err := s.Err(); err != nil {
		t.Fatal(err)
	}
	return &res
}

// TestInteropt checks the Go MarkedSource renderer against the canonical C++
// renderer.  Each test parses the C++ doc utility output and compares the
// results with the native Go implementations.
func TestInteropt(t *testing.T) {
	if os.Getenv("TEST_WORKSPACE") != "io_kythe" {
		// Skip test since it requires the C++ oracle program to be put inside this
		// test's Bazel RUNFILES_DIR.
		t.Skip("Skipping test outside of Bazel build")
	}
	tests := []*cpb.MarkedSource{{
		Kind:     cpb.MarkedSource_IDENTIFIER,
		PreText:  "hello",
		PostText: " world",
	}, {
		Kind: cpb.MarkedSource_BOX,
		Child: []*cpb.MarkedSource{{
			Kind:    cpb.MarkedSource_IDENTIFIER,
			PreText: "ident",
		}},
	}, {
		Kind: cpb.MarkedSource_PARAMETER,
		Child: []*cpb.MarkedSource{{
			Kind:    cpb.MarkedSource_IDENTIFIER,
			PreText: "parameter",
		}},
	}, {
		Kind: cpb.MarkedSource_BOX,
		Child: []*cpb.MarkedSource{{
			Kind:     cpb.MarkedSource_CONTEXT,
			PreText:  "context",
			PostText: ".",
			Child: []*cpb.MarkedSource{{
				Kind:    cpb.MarkedSource_IDENTIFIER,
				PreText: "funcName",
			}},
		}, {
			Kind: cpb.MarkedSource_PARAMETER,
			Child: []*cpb.MarkedSource{{
				Kind:    cpb.MarkedSource_IDENTIFIER,
				PreText: "paramA",
			}, {
				Kind:    cpb.MarkedSource_IDENTIFIER,
				PreText: "paramB",
			}},
		}},
	}, {
		Kind: cpb.MarkedSource_BOX,
		Child: []*cpb.MarkedSource{{
			Kind: cpb.MarkedSource_TYPE,
			Child: []*cpb.MarkedSource{{
				Kind:    cpb.MarkedSource_IDENTIFIER,
				PreText: "int ",
			}},
		}, {
			Kind:              cpb.MarkedSource_CONTEXT,
			PostChildText:     ".",
			AddFinalListToken: true,
			Child: []*cpb.MarkedSource{{
				Kind:    cpb.MarkedSource_IDENTIFIER,
				PreText: "pkg",
			}, {
				Kind:    cpb.MarkedSource_IDENTIFIER,
				PreText: "Files",
			}},
		}, {
			Kind:    cpb.MarkedSource_IDENTIFIER,
			PreText: "CONSTANT",
		}},
	}, {
		Kind: cpb.MarkedSource_PARAMETER,
		Child: []*cpb.MarkedSource{{
			Kind:    cpb.MarkedSource_TYPE,
			PreText: "*pkg.receiver",
		}, {
			Kind:          cpb.MarkedSource_BOX,
			PostChildText: " ",
			Child: []*cpb.MarkedSource{{
				Kind: cpb.MarkedSource_BOX,
				Child: []*cpb.MarkedSource{{
					Kind:    cpb.MarkedSource_CONTEXT,
					PreText: "pkg",
				}, {
					Kind:    cpb.MarkedSource_IDENTIFIER,
					PreText: "param",
				}},
			}, {
				Kind:    cpb.MarkedSource_TYPE,
				PreText: "string",
			}},
		}},
	}}

	for i, test := range tests {
		t.Run(strconv.Itoa(i), func(t *testing.T) {
			oracle := runOracle(t, test)
			if ident := RenderSimpleIdentifier(test, PlaintextContent, nil); oracle.SimpleIdentifier != ident {
				t.Errorf("RenderSimpleIdentifier({%+v}): expected: %q; found %q", test, oracle.SimpleIdentifier, ident)
			}
			params := RenderSimpleParams(test, PlaintextContent, nil)
			if len(params) != len(oracle.SimpleParams) {
				t.Errorf("RenderSimpleParams({%+v}); expected: %#v; found: %#v", test, oracle.SimpleParams, params)
			} else {
				for i, expected := range oracle.SimpleParams {
					if expected != params[i] {
						t.Errorf("RenderSimpleParams({%+v})[%d]; expected: %#v; found: %#v", test, i, expected, params[i])
					}
				}
			}
		})
	}
}

func TestRender(t *testing.T) {
	tests := []struct {
		in  *cpb.MarkedSource
		out string
	}{
		{&cpb.MarkedSource{}, ""},
		{&cpb.MarkedSource{PreText: "PRE", PostText: "POST"}, "PREPOST"},
		{&cpb.MarkedSource{PostChildText: ","}, ""},
		{&cpb.MarkedSource{PostChildText: ",", AddFinalListToken: true}, ""},
		{&cpb.MarkedSource{PreText: "PRE", PostText: "POST", PostChildText: ",",
			Child: []*cpb.MarkedSource{{PreText: "C1"}}}, "PREC1POST"},
		{&cpb.MarkedSource{PreText: "PRE", PostText: "POST", PostChildText: ",", AddFinalListToken: true,
			Child: []*cpb.MarkedSource{{PreText: "C1"}}}, "PREC1,POST"},
		{&cpb.MarkedSource{PreText: "PRE", PostText: "POST", PostChildText: ",",
			Child: []*cpb.MarkedSource{{PreText: "C1"}, {PreText: "C2"}}}, "PREC1,C2POST"},
		{&cpb.MarkedSource{PreText: "PRE", PostText: "POST", PostChildText: ",", AddFinalListToken: true,
			Child: []*cpb.MarkedSource{{PreText: "C1"}, {PreText: "C2"}}}, "PREC1,C2,POST"},
	}
	for i, test := range tests {
		t.Run(strconv.Itoa(i), func(t *testing.T) {
			if got := Render(test.in); got != test.out {
				t.Errorf("from %v: got %q, expected %q", test.in, got, test.out)
			}
		})
	}
}

func TestQName(t *testing.T) {
	// Tests transliterated from QualifiedNameExtractorTest.java.
	tests := []struct {
		input string // text-format proto
		want  *cpb.SymbolInfo
	}{
		{input: "child {\nkind: CONTEXT\nchild {\nkind: IDENTIFIER\npre_text: \"java\"\n} \nchild {\nkind: IDENTIFIER\npre_text: \"com\"\n} \nchild {\nkind: IDENTIFIER\npre_text: \"google\"\n} \nchild {\nkind: IDENTIFIER\npre_text: \"devtools\"\n} \nchild {\nkind: IDENTIFIER\npre_text: \"kythe\"\n} \nchild {\nkind: IDENTIFIER\npre_text: \"analyzers\"\n} \nchild {\nkind: IDENTIFIER\npre_text: \"java\"\n} \npost_child_text: \".\"\nadd_final_list_token: true\n} \nchild {\nkind: IDENTIFIER\npre_text: \"JavaEntrySets\"\n}",

			want: &cpb.SymbolInfo{BaseName: "JavaEntrySets", QualifiedName: "java.com.google.devtools.kythe.analyzers.java.JavaEntrySets"}},

		{input: "child {\nkind: CONTEXT \npost_child_text: \".\"\nadd_final_list_token: true\n} \nchild {\nkind: IDENTIFIER\npre_text: \"JavaEntrySets\"\n}",
			want: &cpb.SymbolInfo{BaseName: "JavaEntrySets"}},

		{input: "child {\nchild {\nkind: IDENTIFIER\npre_text: \"JavaEntrySets\"\n}\n}",
			want: &cpb.SymbolInfo{BaseName: "JavaEntrySets"}},

		{input: "child {}", want: new(cpb.SymbolInfo)},

		{input: "child { pre_text: \"type \" } child { child { kind: CONTEXT child { kind: IDENTIFIER pre_text: \"kythe/go/platform/kindex\" } post_child_text: \".\" add_final_list_token: true } child { kind: IDENTIFIER pre_text: \"Settings\" } } child { kind: TYPE pre_text: \" \" } child { kind: TYPE pre_text: \"struct {...}\" }",
			want: &cpb.SymbolInfo{BaseName: "Settings", QualifiedName: "kythe/go/platform/kindex.Settings"}},

		{input: "child: {\n  pre_text: \"func \"\n}\nchild: {\n  kind: PARAMETER\n  pre_text: \"(\"\n  child: {\n    kind: TYPE\n    pre_text: \"*w\"\n  }\n  post_text: \") \"\n}\nchild: {\n  child: {\n    kind: CONTEXT\n    child: {\n      kind: IDENTIFIER\n      pre_text: \"methdecl\"\n    }\n    child: {\n      kind: IDENTIFIER\n      pre_text: \"w\"\n    }\n    post_child_text: \".\"\n    add_final_list_token: true\n  }\n  child: {\n    kind: IDENTIFIER\n    pre_text: \"LessThan\"\n  }\n}\nchild: {\n  kind: PARAMETER_LOOKUP_BY_PARAM\n  pre_text: \"(\"\n  post_child_text: \", \"\n  post_text: \")\"\n  lookup_index: 1\n}\nchild: {\n  pre_text: \" \"\n  child: {\n    pre_text: \"bool\"\n  }\n}",
			want: &cpb.SymbolInfo{BaseName: "LessThan", QualifiedName: "methdecl.w.LessThan"}},

		// Verify that a default separator does not get injected at the end.
		{input: `child { kind: CONTEXT child { kind: IDENTIFIER pre_text: "//kythe/proto" } } child { kind: IDENTIFIER pre_text: ":analysis_go_proto" }`,
			want: &cpb.SymbolInfo{BaseName: ":analysis_go_proto", QualifiedName: "//kythe/proto:analysis_go_proto"}},

		// Verify that the default separator is correctly used.
		{input: `child { kind: CONTEXT child { kind: IDENTIFIER pre_text: "a" } child { kind: IDENTIFIER pre_text: "b" } } child { kind: IDENTIFIER pre_text: "-tail" }`,
			want: &cpb.SymbolInfo{BaseName: "-tail", QualifiedName: "a.b-tail"}},

		// Verify that template names in context can be correctly rendered.
		{input: `child { kind: CONTEXT child { kind: IDENTIFIER pre_text: "a" } child { child { kind: IDENTIFIER pre_text: "b" } child: { pre_text: "<" child: { pre_text: "c" } post_text: ">"} } post_child_text: "::" } child { kind: IDENTIFIER pre_text: "d" }`,
			want: &cpb.SymbolInfo{BaseName: "d", QualifiedName: "a::b<c>::d"}},

		// Verify that a standalone identifier can work.
		{input: `kind: IDENTIFIER pre_text: "hello"`,
			want: &cpb.SymbolInfo{BaseName: "hello", QualifiedName: ""}},
	}

	for i, test := range tests {
		t.Run(strconv.Itoa(i), func(t *testing.T) {
			var ms cpb.MarkedSource
			if err := prototext.Unmarshal([]byte(test.input), &ms); err != nil {
				t.Fatalf("Invalid test input: %v\nInput was %#q", err, test.input)
			}

			if got := RenderQualifiedName(&ms); !proto.Equal(got, test.want) {
				t.Errorf("Invalid result: got %q, want %q\nInput was %#q", got, test.want, test.input)
			}
		})
	}
}
