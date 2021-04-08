/*
 * Copyright 2014 The Kythe Authors. All rights reserved.
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

package com.google.devtools.kythe.extractors.shared;

import static com.google.common.truth.Truth.assertThat;

import com.google.common.base.Joiner;
import com.google.devtools.kythe.proto.Storage.VName;
import com.google.devtools.kythe.proto.Storage.VNameRewriteRule;
import com.google.devtools.kythe.proto.Storage.VNameRewriteRules;
import junit.framework.TestCase;

/** Tests for {@link FileVNames}. */
public class FileVNamesTest extends TestCase {

  // Mirror of test config in kythe/storage/go/filevnames/filevnames_test.go
  private static final String TEST_CONFIG =
      Joiner.on('\n')
          .join(
              new String[] {
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
                "    \"pattern\": \"(?P<corpus>grp1)/(\\\\d+)/(.*)\",",
                "    \"vname\": {",
                "      \"root\": \"@2@\",",
                "      \"corpus\": \"@1@/@corpus@/@3@\"",
                "    }",
                "  },",
                "  {",
                "    \"pattern\": \"bazel-bin/([^/]+)/java/.*[.]jar!/.*\",",
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
                "]"
              });

  // Equivalent proto of TEST_CONFIG
  private static final VNameRewriteRules TEST_PROTO =
      VNameRewriteRules.newBuilder()
          .addRule(rule("static/path", v("static", "root", "")))
          .addRule(rule("dup/path", v("first", "", "")))
          .addRule(rule("dup/path2", v("second", "", "")))
          .addRule(rule("(?P<corpus>grp1)/(\\d+)/(.*)", v("@1@/@corpus@/@3@", "@2@", "")))
          .addRule(rule("bazel-bin/([^/]+)/java/.*[.]jar!/.*", v("@1@", "java", "")))
          .addRule(rule("third_party/([^/]+)/.*[.]jar!/.*", v("third_party", "@1@", "")))
          .addRule(rule("([^/]+)/java/.*", v("@1@", "java", "")))
          .addRule(rule("([^/]+)/.*", v("@1@", "", "")))
          .build();

  private FileVNames f;

  @Override
  public void setUp() {
    f = FileVNames.fromJson(TEST_CONFIG);
  }

  public void testParsing() {
    assertThat(f).isNotNull();
  }

  public void testToProto() {
    assertThat(f.toProto()).isEqualTo(TEST_PROTO);
  }

  public void testLookup_default() {
    assertThat(f.lookupBaseVName("")).isEqualTo(VName.getDefaultInstance());
  }

  public void testLookup_static() {
    assertThat(f.lookupBaseVName("static/path"))
        .isEqualTo(VName.newBuilder().setRoot("root").setCorpus("static").build());
  }

  public void testLookup_ordered() {
    assertThat(f.lookupBaseVName("dup/path"))
        .isEqualTo(VName.newBuilder().setCorpus("first").build());
    assertThat(f.lookupBaseVName("dup/path2"))
        .isEqualTo(VName.newBuilder().setCorpus("second").build());
  }

  public void testLookup_groups() {
    assertThat(f.lookupBaseVName("corpus/some/path/here"))
        .isEqualTo(VName.newBuilder().setCorpus("corpus").build());
    assertThat(f.lookupBaseVName("grp1/12345/endingGroup"))
        .isEqualTo(VName.newBuilder().setCorpus("grp1/grp1/endingGroup").setRoot("12345").build());

    assertThat(f.lookupBaseVName("bazel-bin/kythe/java/some/path/A.jar!/some/path/A.class"))
        .isEqualTo(VName.newBuilder().setCorpus("kythe").setRoot("java").build());
    assertThat(f.lookupBaseVName("kythe/java/com/google/devtools/kythe/util/KytheURI.java"))
        .isEqualTo(VName.newBuilder().setCorpus("kythe").setRoot("java").build());
    assertThat(f.lookupBaseVName("otherCorpus/java/com/google/devtools/kythe/util/KytheURI.java"))
        .isEqualTo(VName.newBuilder().setCorpus("otherCorpus").setRoot("java").build());
  }

  private static VNameRewriteRule rule(String pattern, VName v) {
    return VNameRewriteRule.newBuilder().setPattern(pattern).setVName(v).build();
  }

  private static VName v(String corpus, String root, String path) {
    return VName.newBuilder().setCorpus(corpus).setRoot(root).setPath(path).build();
  }
}
