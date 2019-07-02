/*
 * Copyright 2018 The Kythe Authors. All rights reserved.
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
package com.google.devtools.kythe.platform.kzip;

import static com.google.common.truth.Truth.assertThat;

import com.google.devtools.kythe.proto.Analysis;
import com.google.devtools.kythe.proto.Go.GoDetails;
import com.google.devtools.kythe.util.JsonUtil;
import com.google.protobuf.util.JsonFormat;
import java.io.File;
import java.io.IOException;
import java.util.Arrays;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

@RunWith(JUnit4.class)
public final class KZipWriterTest {

  private static final JsonFormat.TypeRegistry JSON_TYPE_REGISTRY =
      JsonFormat.TypeRegistry.newBuilder().add(GoDetails.getDescriptor()).build();

  @Before
  public void setup() {
    // This must be called so the json library knows how to read/write the go details message.
    JsonUtil.usingTypeRegistry(JSON_TYPE_REGISTRY);
  }

  private static File getTestTempDir() {
    String testTempDir = System.getenv("TEST_TMPDIR");
    if (testTempDir == null) {
      throw new RuntimeException("TEST_TEMPDIR env. var. must exist for test runs.");
    }
    return new File(testTempDir);
  }

  /**
   * Read in a kzip and store the results in memory (A). Write out what we stored in memory. Read in
   * what we just wrote out (B). Then test that A is the same as B to make sure we wrote the data
   * correctly. {@link KZipReaderTest} will ensure that reading is done correctly.
   */
  @Test
  public void testWriteJson() throws IOException, KZipException {
    testWrite(KZip.Encoding.JSON);
  }

  @Test
  public void testWriteProto() throws IOException, KZipException {
    testWrite(KZip.Encoding.PROTO);
  }

  @Test
  public void testWriteAll() throws IOException, KZipException {
    testWrite(KZip.Encoding.ALL);
  }

  private void testWrite(KZip.Encoding encoding) throws IOException, KZipException {
    // Read in the provided kzip.
    InMemoryKZip originalKZip = readKZip(TestDataUtil.getTestFile("stringset.kzip"));

    // Delete old tmp kzip (if present).
    File tmpKZipFile = new File(getTestTempDir(), "write_test.kzip");
    tmpKZipFile.delete();

    // Write out our kzip.
    KZipWriter writer = new KZipWriter(tmpKZipFile, encoding);

    for (Analysis.IndexedCompilation compilation : originalKZip.compilations) {
      writer.writeUnit(compilation);
      for (Analysis.CompilationUnit.FileInput input :
          compilation.getUnit().getRequiredInputList()) {
        writer.writeFile(originalKZip.fileContents.get(input.getInfo().getDigest()));
      }
    }
    writer.close();

    // Read in the kzip we just wrote out.
    InMemoryKZip writtenKZip = readKZip(new File(getTestTempDir(), "write_test.kzip"));

    // Compare the two kzips.
    assertThat(writtenKZip.compilations).containsExactlyElementsIn(originalKZip.compilations);
    assertThat(writtenKZip.fileContents.keySet())
        .containsExactlyElementsIn(originalKZip.fileContents.keySet());
    for (String key : originalKZip.fileContents.keySet()) {
      assertThat(
              Arrays.equals(writtenKZip.fileContents.get(key), originalKZip.fileContents.get(key)))
          .isTrue();
    }
  }

  /** Simple class to store kzip data in memory. */
  private static final class InMemoryKZip {

    private final Set<Analysis.IndexedCompilation> compilations = new HashSet<>();
    private final Map<String, byte[]> fileContents = new HashMap<>();
  }

  private static InMemoryKZip readKZip(File file) throws IOException, KZipException {
    InMemoryKZip inMemoryKZip = new InMemoryKZip();
    KZipReader reader = new KZipReader(file);
    for (Analysis.IndexedCompilation compilation : reader.scan()) {
      Analysis.CompilationUnit unit = compilation.getUnit();
      inMemoryKZip.compilations.add(compilation);
      for (Analysis.CompilationUnit.FileInput input : unit.getRequiredInputList()) {
        byte[] fileContents = reader.readFile(input.getInfo().getDigest());
        inMemoryKZip.fileContents.put(input.getInfo().getDigest(), fileContents);
      }
    }
    return inMemoryKZip;
  }
}
