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

import com.google.devtools.kythe.proto.Analysis.CompilationUnit;
import com.google.devtools.kythe.proto.Analysis.CompilationUnit.Env;
import com.google.devtools.kythe.proto.Analysis.CompilationUnit.FileInput;
import com.google.devtools.kythe.proto.Analysis.FileInfo;
import com.google.devtools.kythe.proto.Storage.VName;
import com.google.protobuf.Any;
import com.google.protobuf.ByteString;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

@RunWith(JUnit4.class)
public final class CompilationUnitsTest {

  @Test
  public void testEmpty() {
    final String expected = "56bf5044e1b5c4c1cc7c4b131ac2fb979d288460e63352b10eef80ca35bd0a7b";
    assertThat(CompilationUnits.digestFor(CompilationUnit.getDefaultInstance()))
        .isEqualTo(expected);
  }

  @Test
  public void testBasic() {
    final String expected = "e9e170dcfca53c8126755bbc8b703994dedd3af32584291e01fba164ab5d3f32";
    assertThat(
            CompilationUnits.digestFor(
                CompilationUnit.newBuilder()
                    .addArgument("a1")
                    .addArgument("a2")
                    .setVName(
                        VName.newBuilder()
                            .setSignature("S")
                            .setCorpus("C")
                            .setPath("P")
                            .setLanguage("L"))
                    .build()))
        .isEqualTo(expected);
  }

  @Test
  public void testFull() {
    final String expected = "bb761979683e7c268e967eb5bcdedaa7fa5d1d472b0826b00b69acafbaad7ee6";
    assertThat(
            CompilationUnits.digestFor(
                CompilationUnit.newBuilder()
                    .addRequiredInput(
                        FileInput.newBuilder()
                            .setVName(VName.newBuilder().setSignature("RIS"))
                            .setInfo(FileInfo.newBuilder().setPath("path").setDigest("digest")))
                    .setOutputKey("blah")
                    .addEnvironment(Env.newBuilder().setName("feefie").setValue("fofum"))
                    .addDetails(
                        Any.newBuilder()
                            .setTypeUrl("type")
                            .setValue(ByteString.copyFromUtf8("nasaldemons")))
                    .build()))
        .isEqualTo(expected);
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
