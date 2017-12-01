/*
 * Copyright 2017 Google Inc. All rights reserved.
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
import com.google.protobuf.TextFormat;
import java.util.Optional;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

/** Tests {@link QualifiedNameExtractor}. */
@RunWith(JUnit4.class)
public class QualifiedNameExtractorTest {
  @Test
  public void testExtractNameFromMarkedSourceReturnsProperly() throws Exception {
    MarkedSource.Builder builder = MarkedSource.newBuilder();
    TextFormat.merge(
        "child {\nkind: CONTEXT\nchild {\nkind: IDENTIFIER\npre_text: \"java\"\n} \nchild {\nkind: IDENTIFIER\npre_text: \"com\"\n} \nchild {\nkind: IDENTIFIER\npre_text: \"google\"\n} \nchild {\nkind: IDENTIFIER\npre_text: \"devtools\"\n} \nchild {\nkind: IDENTIFIER\npre_text: \"kythe\"\n} \nchild {\nkind: IDENTIFIER\npre_text: \"analyzers\"\n} \nchild {\nkind: IDENTIFIER\npre_text: \"java\"\n} \npost_child_text: \".\"\nadd_final_list_token: true\n} \nchild {\nkind: IDENTIFIER\npre_text: \"JavaEntrySets\"\n}",
        builder);
    MarkedSource testInput = builder.build();
    Optional<String> resolvedName = QualifiedNameExtractor.extractNameFromMarkedSource(testInput);
    assertThat(resolvedName.isPresent()).isTrue();
    assertThat(resolvedName.get())
        .isEqualTo("java.com.google.devtools.kythe.analyzers.java.JavaEntrySets");
  }

  @Test
  public void testExtractNameFromMarkedSourceReturnsProperlyForNoPackage() throws Exception {
    MarkedSource.Builder builder = MarkedSource.newBuilder();
    TextFormat.merge(
        "child {\nkind: CONTEXT \npost_child_text: \".\"\nadd_final_list_token: true\n} \nchild {\nkind: IDENTIFIER\npre_text: \"JavaEntrySets\"\n}",
        builder);
    MarkedSource testInput = builder.build();
    Optional<String> resolvedName = QualifiedNameExtractor.extractNameFromMarkedSource(testInput);
    assertThat(resolvedName.isPresent()).isTrue();
    assertThat(resolvedName.get()).isEqualTo("JavaEntrySets");
  }

  @Test
  public void testExtractNameFromMarkedSourceProperlyHandlesMissingContext() throws Exception {
    MarkedSource.Builder builder = MarkedSource.newBuilder();
    TextFormat.merge(
        "child {\nchild {\nkind: IDENTIFIER\npre_text: \"JavaEntrySets\"\n}\n}", builder);
    MarkedSource testInput = builder.build();
    Optional<String> resolvedName = QualifiedNameExtractor.extractNameFromMarkedSource(testInput);
    assertThat(resolvedName.isPresent()).isFalse();
  }

  @Test
  public void testExtractNameFromMarkedSourceProperlyHandlesMissingData() throws Exception {
    MarkedSource.Builder builder = MarkedSource.newBuilder();
    TextFormat.merge("child {}", builder);
    MarkedSource testInput = builder.build();
    Optional<String> resolvedName = QualifiedNameExtractor.extractNameFromMarkedSource(testInput);
    assertThat(resolvedName.isPresent()).isFalse();
  }

  @Test
  public void testNestedContext() throws Exception {
    MarkedSource.Builder builder = MarkedSource.newBuilder();
    TextFormat.merge(
        "child { pre_text: \"type \" } child { child { kind: CONTEXT child { kind: IDENTIFIER pre_text: \"kythe/go/platform/kindex\" } post_child_text: \".\" add_final_list_token: true } child { kind: IDENTIFIER pre_text: \"Settings\" } } child { kind: TYPE pre_text: \" \" } child { kind: TYPE pre_text: \"struct {...}\" }",
        builder);
    MarkedSource testInput = builder.build();
    Optional<String> resolvedName = QualifiedNameExtractor.extractNameFromMarkedSource(testInput);
    assertThat(resolvedName.isPresent()).isTrue();
    assertThat(resolvedName.get()).isEqualTo("kythe/go/platform/kindex.Settings");
  }

  @Test
  public void testUnmarkedBaseName() throws Exception {
    MarkedSource.Builder builder = MarkedSource.newBuilder();
    TextFormat.merge(
        "child { kind: CONTEXT child { kind: IDENTIFIER pre_text: \"//kythe/proto\" } } child { kind: IDENTIFIER pre_text: \":analysis_go_proto\" }",
        builder);
    MarkedSource testInput = builder.build();
    Optional<String> resolvedName = QualifiedNameExtractor.extractNameFromMarkedSource(testInput);
    assertThat(resolvedName.isPresent()).isTrue();
    assertThat(resolvedName.get()).isEqualTo("//kythe/proto:analysis_go_proto");
  }
}
