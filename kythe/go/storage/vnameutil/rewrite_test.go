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

package vnameutil

import (
	"regexp"
	"strings"
	"testing"

	spb "kythe.io/kythe/proto/storage_proto"

	"github.com/golang/protobuf/proto"
)

// Copied exactly from kythe/javatests/com/google/devtools/kythe/extractors/shared/FileVNamesTest.java
// This could be prettier in Go but a copy ensures better compatibility between
// the libraries.
var testConfig = strings.Join([]string{
	"[",
	"  {",
	"    \"pattern\": \"static/path\",",
	"    \"vname\": {",
	"      \"root\": \"root\",",
	"      \"corpus\": \"static\"",
	"    }",
	"  },",
	"  {",
	"    \"pattern\": \"dup/path\",",
	"    \"vname\": {",
	"      \"corpus\": \"first\"",
	"    }",
	"  },",
	"  {",
	"    \"pattern\": \"dup/path2\",",
	"    \"vname\": {",
	"      \"corpus\": \"second\"",
	"    }",
	"  },",
	"  {",
	"    \"pattern\": \"(grp1)/(\\\\d+)/(.*)\",",
	"    \"vname\": {",
	"      \"root\": \"@2@\",",
	"      \"corpus\": \"@1@/@3@\"",
	"    }",
	"  },",
	"  {",
	"    \"pattern\": \"campfire-out/[^/]+/([^/]+)/java/.*[.]jar!/.*\",",
	"    \"vname\": {",
	"      \"root\": \"java\",",
	"      \"corpus\": \"@1@\"",
	"    }",
	"  },",
	"  {",
	"    \"pattern\": \"third_party/([^/]+)/.*[.]jar!/.*\",",
	"    \"vname\": {",
	"      \"root\": \"@1@\",",
	"      \"corpus\": \"third_party\"",
	"    }",
	"  },",
	"  {",
	"    \"pattern\": \"([^/]+)/java/.*\",",
	"    \"vname\": {",
	"      \"root\": \"java\",",
	"      \"corpus\": \"@1@\"",
	"    }",
	"  },",
	"  {",
	"    \"pattern\": \"([^/]+)/.*\",",
	"    \"vname\": {",
	"      \"corpus\": \"@1@\"",
	"    }",
	"  }",
	"]"}, "\n")

// Verify that parsing in the Go implementation is consistent with the Java
// implementation.
func TestParseConsistency(t *testing.T) {
	r, err := ParseRules([]byte(testConfig))
	if err != nil {
		t.Error(err)
	} else if len(r) == 0 {
		t.Error("empty rules")
	}
}

func TestCornerCases(t *testing.T) {
	testRule1 := Rule{regexp.MustCompile(`(?P<first>\w+)(?:/(?P<second>\w+))?`), V{Corpus: "${first}", Path: "${second}"}.pb()}
	testRule2 := Rule{regexp.MustCompile(`x/(?P<sig>\w+)/y/(?P<tail>.+)$`), V{Path: "${tail}", Sig: "|${sig}|"}.pb()}
	tests := []struct {
		rule  Rule
		input string
		want  *spb.VName
	}{
		// Optional portions of the pattern should be handled correctly.
		{testRule1, "alpha/bravo", V{Corpus: "alpha", Path: "bravo"}.pb()},
		{testRule1, "alpha", V{Corpus: "alpha"}.pb()},

		// Substitution of signature fields should work.
		{testRule2, "x/kanga/y/roo.txt", V{Path: "roo.txt", Sig: "|kanga|"}.pb()},
	}
	for _, test := range tests {
		got, ok := test.rule.Apply(test.input)
		if !ok {
			t.Errorf("Apply %v failed", test.rule)
		} else if !proto.Equal(got, test.want) {
			t.Errorf("Apply %v: got {%+v}, want {%+v}", test.rule, got, test.want)
		} else {
			t.Logf("Apply %v properly returned {%+v}", test.rule, got)
		}
	}
}

func TestFileRewrites(t *testing.T) {
	tests := []struct {
		path string
		want *spb.VName
	}{
		// static
		{"static/path", V{Corpus: "static", Root: "root"}.pb()},

		// ordered
		{"dup/path", V{Corpus: "first"}.pb()},
		{"dup/path2", V{Corpus: "second"}.pb()},

		// groups
		{"corpus/some/path/here", V{Corpus: "corpus"}.pb()},
		{"grp1/12345/endingGroup", V{Corpus: "grp1/endingGroup", Root: "12345"}.pb()},
		{"campfire-out/bin/kythe/java/some/path/A.jar!/some/path/A.class", V{Corpus: "kythe", Root: "java"}.pb()},
		{"kythe/java/com/google/devtools/kythe/util/KytheURI.java", V{Corpus: "kythe", Root: "java"}.pb()},
		{"otherCorpus/java/com/google/devtools/kythe/util/KytheURI.java", V{Corpus: "otherCorpus", Root: "java"}.pb()},
	}

	r, err := ParseRules([]byte(testConfig))
	if err != nil {
		t.Fatalf("Broken test rules: %v", err)
	}

	for _, test := range tests {
		got, ok := r.Apply(test.path)
		if !ok {
			t.Errorf("Apply(%q): no match", test.path)
		} else if !proto.Equal(got, test.want) {
			t.Errorf("Apply(%q): got {%+v}, want {%+v}", test.path, got, test.want)
		}
	}
}

type V struct {
	Corpus, Root, Path, Sig, Lang string
}

func (v V) pb() *spb.VName {
	return &spb.VName{
		Corpus:    v.Corpus,
		Root:      v.Root,
		Path:      v.Path,
		Signature: v.Sig,
		Language:  v.Lang,
	}
}
