/*
 * Copyright 2015 The Kythe Authors. All rights reserved.
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

import static com.google.common.base.StandardSystemProperty.USER_DIR;
import static com.google.common.truth.Truth.assertThat;

import com.google.common.collect.ImmutableList;
import com.google.common.collect.Lists;
import com.google.common.io.Files;
import com.google.devtools.kythe.proto.Analysis.CompilationUnit;
import com.google.devtools.kythe.proto.Analysis.FileData;
import com.google.devtools.kythe.proto.Analysis.FileInfo;
import com.google.devtools.kythe.proto.Storage.VName;
import com.google.protobuf.ByteString;
import java.io.File;
import java.nio.file.Paths;
import java.security.NoSuchAlgorithmException;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import junit.framework.TestCase;

/** Tests the {@link ExtractorUtils} code. */
public class ExtractorUtilsTest extends TestCase {

  private static final String TEST_DATA_DIR =
      "kythe/javatests/com/google/devtools/kythe/extractors/shared/testdata/";
  public static final String ABCD_HASH =
      "e12e115acf4552b2568b55e93cbd39394c4ef81c82447fafc997882a02d23677";
  public static final byte[] ABCD = ByteString.copyFromUtf8("ABCD").toByteArray();

  public void testCreateFileData() {
    FileData fd = ExtractorUtils.createFileData("a/b/c", ABCD);
    assertThat(fd).isNotNull();
    assertThat(fd.getInfo().getPath()).isEqualTo("a/b/c");
    assertThat(fd.getInfo().getDigest()).isEqualTo(ABCD_HASH);
    assertThat(fd.getContent().toByteArray()).isEqualTo(ABCD);
  }

  public void testConvertBytesToFileDatas() throws Exception {
    Map<String, byte[]> data = new HashMap<>();
    data.put("a/b/c", new byte[] {1, 2, 3});
    data.put("d/e/f", new byte[] {4, 5, 6});
    List<FileData> results = ExtractorUtils.convertBytesToFileDatas(data);
    assertThat(results).hasSize(2);
    for (FileData entry : results) {
      assertThat(data).containsKey(entry.getInfo().getPath());
      byte[] input = data.get(entry.getInfo().getPath());
      assertThat(entry.getContent().toByteArray()).isEqualTo(input);
      assertThat(entry.getInfo().getDigest()).isEqualTo(ExtractorUtils.getContentDigest(input));
    }
  }

  public void testProcessRequiredInputs() throws Exception {
    String path = Paths.get(TEST_DATA_DIR, "sample.txt").toString();
    List<FileData> fds = ExtractorUtils.processRequiredInputs(Lists.newArrayList(path));

    byte[] content = Files.toByteArray(new File(path));

    assertThat(fds).hasSize(1);
    FileData fd = fds.get(0);
    assertThat(fd.getInfo().getPath()).isEqualTo(path);
    assertThat(fd.getInfo().getDigest()).isEqualTo(ExtractorUtils.getContentDigest(content));
    assertThat(fd.getContent().toByteArray()).isEqualTo(content);
  }

  public void testToCompilationFileInputs() {
    final String digest = "DIGEST";
    final String content = "CONTENT";
    final String path = "a/b/c";
    ImmutableList<CompilationUnit.FileInput> cfis =
        ExtractorUtils.toFileInputs(
            Lists.newArrayList(
                FileData.newBuilder()
                    .setContent(ByteString.copyFromUtf8(content))
                    .setInfo(FileInfo.newBuilder().setDigest(digest).setPath(path).build())
                    .build()));
    assertThat(cfis).hasSize(1);
    assertEquals(digest, cfis.get(0).getInfo().getDigest());
    assertEquals(path, cfis.get(0).getInfo().getPath());
  }

  public void testTryMakeRelative() {
    String cwd = USER_DIR.value();
    assertThat(ExtractorUtils.tryMakeRelative("/someroot", "relative"))
        .isEqualTo(cwd + "/relative");
    assertThat(ExtractorUtils.tryMakeRelative("/someroot", "relative/sd"))
        .isEqualTo(cwd + "/relative/sd");
    assertThat(ExtractorUtils.tryMakeRelative("/someroot", "/someroot/rootrelative"))
        .isEqualTo("rootrelative");
    assertThat(ExtractorUtils.tryMakeRelative("/someroot/", "/someroot/rootrelative"))
        .isEqualTo("rootrelative");
    assertThat(ExtractorUtils.tryMakeRelative("/someroot/", "./cwd_sub"))
        .isEqualTo(cwd + "/cwd_sub");
    assertThat(ExtractorUtils.tryMakeRelative("/someroot/", "/someroot/../one_up"))
        .isEqualTo("/one_up");

    assertThat(ExtractorUtils.tryMakeRelative(cwd, "relative")).isEqualTo("relative");
    assertThat(ExtractorUtils.tryMakeRelative(cwd, "relative/sd")).isEqualTo("relative/sd");
    assertThat(ExtractorUtils.tryMakeRelative(cwd, "./cwd_sub")).isEqualTo("cwd_sub");
    assertThat(ExtractorUtils.tryMakeRelative(cwd, "relative/../one_up")).isEqualTo("one_up");
    assertThat(ExtractorUtils.tryMakeRelative(cwd, ".")).isEqualTo(".");
    assertThat(ExtractorUtils.tryMakeRelative(cwd, cwd)).isEqualTo(".");
  }

  public void testGetCurrentWorkingDirectory() {
    assertThat(ExtractorUtils.getCurrentWorkingDirectory())
        .isEqualTo(System.getProperty("user.dir"));
  }

  public void testGetContentDigest() throws NoSuchAlgorithmException {
    assertThat(ExtractorUtils.getContentDigest(ABCD)).isEqualTo(ABCD_HASH);
  }

  public void testNormalizeCompilationUnit() {
    CompilationUnit unit =
        CompilationUnit.newBuilder()
            .setVName(VName.newBuilder().setSignature("//target:target").build())
            .addArgument("arg1")
            .addArgument("arg2")
            .setOutputKey("output")
            .addSourceFile("b.java")
            .addSourceFile("c.java")
            .addRequiredInput(
                CompilationUnit.FileInput.newBuilder()
                    .setInfo(FileInfo.newBuilder().setDigest("DIGEST1").setPath("d/e/f").build()))
            .addRequiredInput(
                CompilationUnit.FileInput.newBuilder()
                    .setInfo(FileInfo.newBuilder().setDigest("DIGEST2").setPath("a/b/c").build()))
            .build();

    CompilationUnit output = ExtractorUtils.normalizeCompilationUnit(unit);
    assertThat(output.getVName().getSignature()).isEqualTo(unit.getVName().getSignature());
    assertThat(output.getArgumentList()).isSameInstanceAs(unit.getArgumentList());
    assertThat(output.getOutputKey()).isEqualTo(unit.getOutputKey());
    assertThat(output.getSourceFileList()).isSameInstanceAs(unit.getSourceFileList());
    assertThat(output.getRequiredInputCount()).isEqualTo(2);

    CompilationUnit.FileInput cfi0 = output.getRequiredInput(0);
    assertThat(cfi0.getInfo().getPath()).isEqualTo("a/b/c");
    assertThat(cfi0.getInfo().getDigest()).isEqualTo("DIGEST2");

    CompilationUnit.FileInput cfi1 = output.getRequiredInput(1);
    assertThat(cfi1.getInfo().getPath()).isEqualTo("d/e/f");
    assertThat(cfi1.getInfo().getDigest()).isEqualTo("DIGEST1");
  }
}
