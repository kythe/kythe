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

package com.google.devtools.kythe.platform.indexpack;

import static com.google.common.truth.Truth.assertThat;

import static com.google.common.truth.Truth.assertThat;

import com.google.common.hash.HashFunction;
import com.google.common.hash.Hashing;
import com.google.devtools.kythe.extractors.shared.CompilationDescription;
import com.google.devtools.kythe.proto.Analysis.CompilationUnit;
import com.google.devtools.kythe.proto.Analysis.FileData;
import com.google.devtools.kythe.proto.Analysis.FileInfo;
import com.google.devtools.kythe.proto.Storage.VName;
import com.google.devtools.kythe.util.DeleteRecursively;
import com.google.protobuf.ByteString;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.Arrays;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Set;
import junit.framework.TestCase;

/** This class tests {@link Archive}. */
public class ArchiveTest extends TestCase {

  private static final HashFunction DIGEST_FUNCTION = Hashing.sha256();
  private static final CompilationDescription[] TEST_COMPILATIONS =
      new CompilationDescription[] {
        compilation(
            vname("signature", "corpus", "language"),
            file("file1", "contents1"),
            file("file2", "contents2")),
        compilation(
            vname("another_compilation", "", "java"),
            file("file", "contents1"),
            file("some/other/file", "contents1"),
            file("some/empty/file", ""))
      };

  private Archive archive;

  @Override
  public void setUp() throws IOException {
    Path tempDir = Files.createTempDirectory("archive_test");
    archive = new Archive(tempDir);
  }

  @Override
  public void tearDown() throws IOException {
    DeleteRecursively.delete(archive.getRoot());
    archive = null;
  }

  public void testCreation() throws IOException {
    assertThat(archive.getRoot().toFile().isDirectory()).isTrue();
  }

  public void testReadUnits_empty() throws IOException {
    assertThat(archive.readUnits().hasNext()).isFalse();
  }

  public void testReadFile_missing() throws IOException {
    try {
      archive.readFile("missingkey");
      fail("Missing FileNotFoundException");
    } catch (java.io.FileNotFoundException fnfe) {
      // we're all good
    }
  }

  public void testWriteUnit() throws IOException {
    String key1 = archive.writeUnit("test", new Object());
    assertThat(key1).isNotNull();
    Map<String, String> map = new HashMap<>();
    map.put("something", "else");
    String key2 = archive.writeUnit("test", map);
    assertThat(key2).isNotNull();
    assertThat(key1.equals(key2)).isFalse();
  }

  public void testWriteFile_single() throws IOException {
    String dataStr = "this is some random test to\r\n put in a file\n.";
    byte[] data = dataStr.getBytes(Archive.DATA_CHARSET);

    String key = archive.writeFile(data);
    assertThat(key).isNotNull();
    assertThat(archive.writeFile(dataStr)).isEqualTo(key);

    assertArrayEquals(data, archive.readFile(key));
  }

  public void testWriteFile_multiple() throws IOException {
    final int NUM = 10;
    String[] keys = new String[NUM];
    for (int i = 0; i < NUM; i++) {
      keys[i] = archive.writeFile(createTestData(i));
      assertThat(keys[i]).isNotNull();
    }

    for (int i = 0; i < NUM; i++) {
      assertArrayEquals(createTestData(i), archive.readFile(keys[i]));
    }
  }

  public void testReadUnits_single() throws IOException {
    assertThat(archive.writeUnit("test", new Object())).isNotNull();
    Iterator<Object> it = archive.readUnits("test", Object.class);
    assertThat(it.hasNext()).isTrue();
    assertThat(it.next()).isNotNull();
    assertThat(it.hasNext()).isFalse();
  }

  public void testWriteUnit_compilationUnit() throws IOException {
    for (CompilationDescription desc : TEST_COMPILATIONS) {
      assertThat(archive.writeUnit(desc.getCompilationUnit())).isNotNull();
    }
  }

  public void testReadUnits_compilationUnit() throws IOException {
    Set<CompilationUnit> toRead = new HashSet<>();
    for (CompilationDescription desc : TEST_COMPILATIONS) {
      archive.writeUnit(desc.getCompilationUnit());
      toRead.add(desc.getCompilationUnit());
    }

    Iterator<CompilationUnit> it = archive.readUnits();
    while (it.hasNext()) {
      CompilationUnit unit = it.next();
      assertThat(toRead).contains(unit);
      toRead.remove(unit);
    }

    assertThat(toRead).isEmpty();
  }

  public void testWriteDescription() throws IOException {
    for (CompilationDescription desc : TEST_COMPILATIONS) {
      assertThat(archive.writeDescription(desc)).isNotNull();
    }
  }

  public void testReadDescriptions() throws IOException {
    Set<CompilationDescription> toRead = new HashSet<>();
    for (CompilationDescription desc : TEST_COMPILATIONS) {
      archive.writeDescription(desc);
      toRead.add(desc);
    }

    Iterator<CompilationDescription> it = archive.readDescriptions();
    while (it.hasNext()) {
      CompilationDescription desc = it.next();
      assertThat(toRead).contains(desc);
      toRead.remove(desc);
    }

    assertThat(toRead).isEmpty();
  }

  private static byte[] createTestData(int i) {
    return ("" + ("" + i).intern().hashCode()).getBytes(Archive.DATA_CHARSET);
  }

  private static FileData file(String path, String contents) {
    byte[] data = contents.getBytes(Archive.DATA_CHARSET);
    String digest = DIGEST_FUNCTION.hashBytes(data).toString();
    return FileData.newBuilder()
        .setInfo(FileInfo.newBuilder().setPath(path).setDigest(digest).build())
        .setContent(ByteString.copyFrom(data))
        .build();
  }

  private static CompilationDescription compilation(VName vname, FileData... files) {
    CompilationUnit.Builder unit = CompilationUnit.newBuilder().setVName(vname);
    List<FileData> fileList = Arrays.asList(files);
    for (FileData file : fileList) {
      String path = file.getInfo().getPath();
      unit.addRequiredInput(
          CompilationUnit.FileInput.newBuilder()
              .setVName(VName.newBuilder().setPath(path))
              .setInfo(file.getInfo())
              .build());
      unit.addSourceFile(path);
    }
    return new CompilationDescription(unit.build(), fileList);
  }

  private static VName vname(String signature, String corpus, String language) {
    return VName.newBuilder()
        .setSignature(signature)
        .setCorpus(corpus)
        .setLanguage(language)
        .build();
  }

  private void assertArrayEquals(byte[] exp, byte[] a) {
    assertThat(a).isEqualTo(exp);
  }
}
