/*
 * Copyright 2015 Google Inc. All rights reserved.
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

import static org.junit.Assert.assertArrayEquals;

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
    assertNotNull(fd);
    assertEquals("a/b/c", fd.getInfo().getPath());
    assertEquals(ABCD_HASH, fd.getInfo().getDigest());
    assertArrayEquals(ABCD, fd.getContent().toByteArray());
  }

  public void testConvertBytesToFileDatas() throws Exception {
    Map<String, byte[]> data = new HashMap<>();
    data.put("a/b/c", new byte[] {1, 2, 3});
    data.put("d/e/f", new byte[] {4, 5, 6});
    List<FileData> results = ExtractorUtils.convertBytesToFileDatas(data);
    assertEquals(2, results.size());
    for (FileData entry : results) {
      assertTrue(data.containsKey(entry.getInfo().getPath()));
      byte[] input = data.get(entry.getInfo().getPath());
      assertArrayEquals(input, entry.getContent().toByteArray());
      assertEquals(ExtractorUtils.getContentDigest(input), entry.getInfo().getDigest());
    }
  }

  public void testProcessRequiredInputs() throws Exception {
    String path = Paths.get(TEST_DATA_DIR, "sample.txt").toString();
    List<FileData> fds = ExtractorUtils.processRequiredInputs(Lists.newArrayList(path));

    byte[] content = Files.toByteArray(new File(path));

    assertEquals(1, fds.size());
    FileData fd = fds.get(0);
    assertEquals(path, fd.getInfo().getPath());
    assertEquals(ExtractorUtils.getContentDigest(content), fd.getInfo().getDigest());
    assertArrayEquals(content, fd.getContent().toByteArray());
  }

  public void testToCompilationFileInputs() {
    final String digest = "DIGEST";
    final String content = "CONTENT";
    final String path = "a/b/c";
    List<CompilationUnit.FileInput> cfis =
        ExtractorUtils.toFileInputs(
            Lists.newArrayList(
                FileData.newBuilder()
                    .setContent(ByteString.copyFromUtf8(content))
                    .setInfo(FileInfo.newBuilder().setDigest(digest).setPath(path).build())
                    .build()));
    assertEquals(1, cfis.size());
    assertEquals(digest, cfis.get(0).getInfo().getDigest());
    assertEquals(path, cfis.get(0).getInfo().getPath());
  }

  public void testTryMakeRelative() {
    String cwd = System.getProperty("user.dir");
    assertEquals(cwd + "/relative", ExtractorUtils.tryMakeRelative("/someroot", "relative"));
    assertEquals(cwd + "/relative/sd", ExtractorUtils.tryMakeRelative("/someroot", "relative/sd"));
    assertEquals(
        "rootrelative", ExtractorUtils.tryMakeRelative("/someroot", "/someroot/rootrelative"));
    assertEquals(
        "rootrelative", ExtractorUtils.tryMakeRelative("/someroot/", "/someroot/rootrelative"));
    assertEquals(cwd + "/cwd_sub", ExtractorUtils.tryMakeRelative("/someroot/", "./cwd_sub"));
    assertEquals("/one_up", ExtractorUtils.tryMakeRelative("/someroot/", "/someroot/../one_up"));

    assertEquals("relative", ExtractorUtils.tryMakeRelative(cwd, "relative"));
    assertEquals("relative/sd", ExtractorUtils.tryMakeRelative(cwd, "relative/sd"));
    assertEquals("cwd_sub", ExtractorUtils.tryMakeRelative(cwd, "./cwd_sub"));
    assertEquals("one_up", ExtractorUtils.tryMakeRelative(cwd, "relative/../one_up"));
    assertEquals(".", ExtractorUtils.tryMakeRelative(cwd, "."));
    assertEquals(".", ExtractorUtils.tryMakeRelative(cwd, cwd));
  }

  public void testGetCurrentWorkingDirectory() {
    assertEquals(System.getProperty("user.dir"), ExtractorUtils.getCurrentWorkingDirectory());
  }

  public void testGetContentDigest() throws NoSuchAlgorithmException {
    assertEquals(ABCD_HASH, ExtractorUtils.getContentDigest(ABCD));
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
    assertEquals(unit.getVName().getSignature(), output.getVName().getSignature());
    assertSame(unit.getArgumentList(), output.getArgumentList());
    assertEquals(unit.getOutputKey(), output.getOutputKey());
    assertSame(unit.getSourceFileList(), output.getSourceFileList());
    assertEquals(2, output.getRequiredInputCount());

    CompilationUnit.FileInput cfi0 = output.getRequiredInput(0);
    assertEquals("a/b/c", cfi0.getInfo().getPath());
    assertEquals("DIGEST2", cfi0.getInfo().getDigest());

    CompilationUnit.FileInput cfi1 = output.getRequiredInput(1);
    assertEquals("d/e/f", cfi1.getInfo().getPath());
    assertEquals("DIGEST1", cfi1.getInfo().getDigest());
  }
}
