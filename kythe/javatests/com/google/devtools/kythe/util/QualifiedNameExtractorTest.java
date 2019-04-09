/*
 * Copyright 2017 The Kythe Authors. All rights reserved.
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

package com.google.devtools.kythe.util;

import static com.google.common.truth.Truth.assertThat;

import com.google.devtools.kythe.proto.MarkedSource;
import com.google.devtools.kythe.proto.SymbolInfo;
import com.google.protobuf.TextFormat;
import java.util.Optional;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

/** Tests {@link QualifiedNameExtractor}. */
@RunWith(JUnit4.class)
public class QualifiedNameExtractorTest {
  @Test
  public void testCStyleMarkedSource() throws Exception {
    MarkedSource.Builder builder = MarkedSource.newBuilder();
    TextFormat.merge(
        "child {\n"
            + "    kind: TYPE\n"
            + "    pre_text: \"Output*\"\n"
            + "}\n"
            + "child {\n"
            + "    pre_text: \" \"\n"
            + "}\n"
            + "child {\n"
            + "    kind: IDENTIFIER\n"
            + "    pre_text: \"parse\"\n"
            + "}\n"
            + "child {\n"
            + "    kind: PARAMETER\n"
            + "    pre_text: \"(\"\n"
            + "    child: {\n"
            + "        pre_text: \"const \"\n"
            + "        child: {\n"
            + "            kind: TYPE\n"
            + "            pre_text: \"char*\"\n"
            + "        }\n"
            + "        child {\n"
            + "            pre_text: \" \"\n"
            + "        }\n"
            + "        child {\n"
            + "            child: {\n"
            + "                kind: CONTEXT\n"
            + "                child: [\n"
            + "                  {\n"
            + "                    kind: IDENTIFIER\n"
            + "                    pre_text: \"parse\"\n"
            + "                  }\n"
            + "                ]\n"
            + "                post_child_text: \"::\"\n"
            + "                add_final_list_token: true\n"
            + "            }\n"
            + "            child: {\n"
            + "                kind: IDENTIFIER\n"
            + "                pre_text: \"buffer\"\n"
            + "            }\n"
            + "        }\n"
            + "    }\n"
            + "    post_child_text: \", \"\n"
            + "    post_text: \")\"\n"
            + "}\n",
        builder);
    MarkedSource testInput = builder.build();
    Optional<SymbolInfo> resolvedName =
        QualifiedNameExtractor.extractNameFromMarkedSource(testInput);
    assertThat(resolvedName.isPresent()).isTrue();
    assertThat(resolvedName.get())
        .isEqualTo(SymbolInfo.newBuilder().setBaseName("parse").setQualifiedName("parse").build());
  }

  @Test
  public void testExtractNameFromMarkedSourceReturnsProperly() throws Exception {
    MarkedSource.Builder builder = MarkedSource.newBuilder();
    TextFormat.merge(
        "child {\n"
            + "kind: CONTEXT\n"
            + "child {\n"
            + "kind: IDENTIFIER\n"
            + "pre_text: \"java\"\n"
            + "} \n"
            + "child {\n"
            + "kind: IDENTIFIER\n"
            + "pre_text: \"com\"\n"
            + "} \n"
            + "child {\n"
            + "kind: IDENTIFIER\n"
            + "pre_text: \"google\"\n"
            + "} \n"
            + "child {\n"
            + "kind: IDENTIFIER\n"
            + "pre_text: \"devtools\"\n"
            + "} \n"
            + "child {\n"
            + "kind: IDENTIFIER\n"
            + "pre_text: \"kythe\"\n"
            + "} \n"
            + "child {\n"
            + "kind: IDENTIFIER\n"
            + "pre_text: \"analyzers\"\n"
            + "} \n"
            + "child {\n"
            + "kind: IDENTIFIER\n"
            + "pre_text: \"java\"\n"
            + "} \n"
            + "post_child_text: \".\"\n"
            + "add_final_list_token: true\n"
            + "} \n"
            + "child {\n"
            + "kind: IDENTIFIER\n"
            + "pre_text: \"JavaEntrySets\"\n"
            + "}",
        builder);
    MarkedSource testInput = builder.build();
    Optional<SymbolInfo> resolvedName =
        QualifiedNameExtractor.extractNameFromMarkedSource(testInput);
    assertThat(resolvedName.isPresent()).isTrue();
    assertThat(resolvedName.get())
        .isEqualTo(
            SymbolInfo.newBuilder()
                .setBaseName("JavaEntrySets")
                .setQualifiedName("java.com.google.devtools.kythe.analyzers.java.JavaEntrySets")
                .build());
  }

  @Test
  public void testExtractNameFromMarkedSourceReturnsProperlyForNoPackage() throws Exception {
    MarkedSource.Builder builder = MarkedSource.newBuilder();
    TextFormat.merge(
        "child {\n"
            + "kind: CONTEXT \n"
            + "post_child_text: \".\"\n"
            + "add_final_list_token: true\n"
            + "} \n"
            + "child {\n"
            + "kind: IDENTIFIER\n"
            + "pre_text: \"JavaEntrySets\"\n"
            + "}",
        builder);
    MarkedSource testInput = builder.build();
    Optional<SymbolInfo> resolvedName =
        QualifiedNameExtractor.extractNameFromMarkedSource(testInput);
    assertThat(resolvedName.isPresent()).isTrue();
    assertThat(resolvedName.get())
        .isEqualTo(
            SymbolInfo.newBuilder()
                .setBaseName("JavaEntrySets")
                .setQualifiedName("JavaEntrySets")
                .build());
  }

  @Test
  public void testExtractNameFromMarkedSourceProperlyHandlesMissingContext() throws Exception {
    MarkedSource.Builder builder = MarkedSource.newBuilder();
    TextFormat.merge(
        "child {\nchild {\nkind: IDENTIFIER\npre_text: \"JavaEntrySets\"\n}\n}", builder);
    MarkedSource testInput = builder.build();
    Optional<SymbolInfo> resolvedName =
        QualifiedNameExtractor.extractNameFromMarkedSource(testInput);
    assertThat(resolvedName.isPresent()).isTrue();
    assertThat(resolvedName.get())
        .isEqualTo(
            SymbolInfo.newBuilder()
                .setBaseName("JavaEntrySets")
                .setQualifiedName("JavaEntrySets")
                .build());
  }

  @Test
  public void testExtractNameFromMarkedSourceProperlyHandlesMissingData() throws Exception {
    MarkedSource.Builder builder = MarkedSource.newBuilder();
    TextFormat.merge("child {}", builder);
    MarkedSource testInput = builder.build();
    Optional<SymbolInfo> resolvedName =
        QualifiedNameExtractor.extractNameFromMarkedSource(testInput);
    assertThat(resolvedName.isPresent()).isFalse();
  }

  @Test
  public void testNestedContext() throws Exception {
    MarkedSource.Builder builder = MarkedSource.newBuilder();
    TextFormat.merge(
        "child { pre_text: \"type \" } child { child { kind: CONTEXT child { kind: IDENTIFIER"
            + " pre_text: \"kythe/go/platform/kindex\" } post_child_text: \".\""
            + " add_final_list_token: true } child { kind: IDENTIFIER pre_text: \"Settings\" } }"
            + " child { kind: TYPE pre_text: \" \" } child { kind: TYPE pre_text: \"struct {...}\""
            + " }",
        builder);
    MarkedSource testInput = builder.build();
    Optional<SymbolInfo> resolvedName =
        QualifiedNameExtractor.extractNameFromMarkedSource(testInput);
    assertThat(resolvedName.isPresent()).isTrue();
    assertThat(resolvedName.get())
        .isEqualTo(
            SymbolInfo.newBuilder()
                .setBaseName("Settings")
                .setQualifiedName("kythe/go/platform/kindex.Settings")
                .build());
  }

  @Test
  public void testUnmarkedBaseName() throws Exception {
    MarkedSource.Builder builder = MarkedSource.newBuilder();
    TextFormat.merge(
        "child { kind: CONTEXT child { kind: IDENTIFIER pre_text: \"//kythe/proto\" } } child {"
            + " kind: IDENTIFIER pre_text: \":analysis_go_proto\" }",
        builder);
    MarkedSource testInput = builder.build();
    Optional<SymbolInfo> resolvedName =
        QualifiedNameExtractor.extractNameFromMarkedSource(testInput);
    assertThat(resolvedName.isPresent()).isTrue();
    assertThat(resolvedName.get())
        .isEqualTo(
            SymbolInfo.newBuilder()
                .setBaseName(":analysis_go_proto")
                .setQualifiedName("//kythe/proto:analysis_go_proto")
                .build());
  }

  @Test
  public void testIdentifierAtRoot() throws Exception {
    MarkedSource.Builder builder = MarkedSource.newBuilder();
    TextFormat.merge("kind: IDENTIFIER pre_text: \"fizzlepug\"", builder);
    MarkedSource testInput = builder.build();
    Optional<SymbolInfo> resolvedName =
        QualifiedNameExtractor.extractNameFromMarkedSource(testInput);
    assertThat(resolvedName.isPresent()).isTrue();
    assertThat(resolvedName.get())
        .isEqualTo(
            SymbolInfo.newBuilder().setBaseName("fizzlepug").setQualifiedName("fizzlepug").build());
  }
}
