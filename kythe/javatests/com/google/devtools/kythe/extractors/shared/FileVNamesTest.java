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

package com.google.devtools.kythe.extractors.shared;

import com.google.common.base.Joiner;
import com.google.devtools.kythe.proto.Storage.VName;
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
                "    \"pattern\": \"(grp1)/(\\\\d+)/(.*)\",",
                "    \"vname\": {",
                "      \"root\": \"@2@\",",
                "      \"corpus\": \"@1@/@3@\"",
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

  private FileVNames f;

  @Override
  public void setUp() {
    f = FileVNames.fromJson(TEST_CONFIG);
  }

  public void testParsing() {
    assertNotNull(f);
  }

  public void testLookup_default() {
    assertEquals(VName.getDefaultInstance(), f.lookupBaseVName(""));
  }

  public void testLookup_static() {
    assertEquals(
        VName.newBuilder().setRoot("root").setCorpus("static").build(),
        f.lookupBaseVName("static/path"));
  }

  public void testLookup_ordered() {
    assertEquals(VName.newBuilder().setCorpus("first").build(), f.lookupBaseVName("dup/path"));
    assertEquals(VName.newBuilder().setCorpus("second").build(), f.lookupBaseVName("dup/path2"));
  }

  public void testLookup_groups() {
    assertEquals(
        VName.newBuilder().setCorpus("corpus").build(), f.lookupBaseVName("corpus/some/path/here"));
    assertEquals(
        VName.newBuilder().setCorpus("grp1/endingGroup").setRoot("12345").build(),
        f.lookupBaseVName("grp1/12345/endingGroup"));

    assertEquals(
        VName.newBuilder().setCorpus("kythe").setRoot("java").build(),
        f.lookupBaseVName("bazel-bin/kythe/java/some/path/A.jar!/some/path/A.class"));
    assertEquals(
        VName.newBuilder().setCorpus("kythe").setRoot("java").build(),
        f.lookupBaseVName("kythe/java/com/google/devtools/kythe/util/KytheURI.java"));
    assertEquals(
        VName.newBuilder().setCorpus("otherCorpus").setRoot("java").build(),
        f.lookupBaseVName("otherCorpus/java/com/google/devtools/kythe/util/KytheURI.java"));
  }
}
