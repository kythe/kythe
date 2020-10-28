/*
 * Copyright 2019 The Kythe Authors. All rights reserved.
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

package com.google.devtools.kythe.platform.shared;

import static com.google.common.truth.Truth.assertThat;

import com.google.common.io.Files;
import com.google.devtools.kythe.proto.Analysis.CompilationUnit;
import com.google.devtools.kythe.proto.Analysis.CompilationUnit.Env;
import com.google.devtools.kythe.proto.Analysis.CompilationUnit.FileInput;
import com.google.devtools.kythe.proto.Analysis.FileInfo;
import com.google.protobuf.Any;
import com.google.protobuf.TextFormat;
import java.nio.charset.StandardCharsets;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

@RunWith(JUnit4.class)
public final class CompilationUnitsTest {

  private CompilationUnit readCompilationUnit(String basename) throws Exception {
    String text =
        Files.asCharSource(TestDataUtil.getTestFile(basename + ".pbtxt"), StandardCharsets.UTF_8)
            .read();
    return TextFormat.parse(text, CompilationUnit.class);
  }

  @Test
  public void testEmpty() throws Exception {
    final String expected = "56bf5044e1b5c4c1cc7c4b131ac2fb979d288460e63352b10eef80ca35bd0a7b";
    assertThat(CompilationUnits.digestFor(readCompilationUnit(expected))).isEqualTo(expected);
  }

  @Test
  public void testBasic() throws Exception {
    final String expected = "e9e170dcfca53c8126755bbc8b703994dedd3af32584291e01fba164ab5d3f32";
    assertThat(CompilationUnits.digestFor(readCompilationUnit(expected))).isEqualTo(expected);
  }

  @Test
  public void testFull() throws Exception {
    final String expected = "bb761979683e7c268e967eb5bcdedaa7fa5d1d472b0826b00b69acafbaad7ee6";
    assertThat(CompilationUnits.digestFor(readCompilationUnit(expected))).isEqualTo(expected);
  }

  @Test
  public void testCanonicalizationEmpty() {
    assertThat(CompilationUnits.canonicalize(CompilationUnit.getDefaultInstance()))
        .isEqualTo(CompilationUnit.getDefaultInstance());
  }

  @Test
  public void testCanonicalization() {
    CompilationUnit expected =
        CompilationUnit.newBuilder()
            .addRequiredInput(FileInput.newBuilder().setInfo(FileInfo.newBuilder().setDigest("A")))
            .addRequiredInput(FileInput.newBuilder().setInfo(FileInfo.newBuilder().setDigest("B")))
            .addRequiredInput(FileInput.newBuilder().setInfo(FileInfo.newBuilder().setDigest("C")))
            .addSourceFile("A")
            .addSourceFile("A")
            .addSourceFile("B")
            .addSourceFile("C")
            .addSourceFile("C")
            .addEnvironment(Env.newBuilder().setName("A"))
            .addEnvironment(Env.newBuilder().setName("B"))
            .addEnvironment(Env.newBuilder().setName("C"))
            .addDetails(Any.newBuilder().setTypeUrl("A"))
            .addDetails(Any.newBuilder().setTypeUrl("B"))
            .addDetails(Any.newBuilder().setTypeUrl("C"))
            .build();
    assertThat(
            CompilationUnits.canonicalize(
                CompilationUnit.newBuilder()
                    .addRequiredInput(
                        FileInput.newBuilder().setInfo(FileInfo.newBuilder().setDigest("B")))
                    .addRequiredInput(
                        FileInput.newBuilder().setInfo(FileInfo.newBuilder().setDigest("C")))
                    .addRequiredInput(
                        FileInput.newBuilder().setInfo(FileInfo.newBuilder().setDigest("A")))
                    .addRequiredInput(
                        FileInput.newBuilder().setInfo(FileInfo.newBuilder().setDigest("C")))
                    .addRequiredInput(
                        FileInput.newBuilder().setInfo(FileInfo.newBuilder().setDigest("A")))
                    .addSourceFile("B")
                    .addSourceFile("C")
                    .addSourceFile("A")
                    .addSourceFile("C")
                    .addSourceFile("A")
                    .addEnvironment(Env.newBuilder().setName("B"))
                    .addEnvironment(Env.newBuilder().setName("A"))
                    .addEnvironment(Env.newBuilder().setName("C"))
                    .addDetails(Any.newBuilder().setTypeUrl("C"))
                    .addDetails(Any.newBuilder().setTypeUrl("B"))
                    .addDetails(Any.newBuilder().setTypeUrl("A"))
                    .build()))
        .isEqualTo(expected);
  }
}
