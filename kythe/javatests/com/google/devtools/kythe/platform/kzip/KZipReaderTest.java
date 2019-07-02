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
import static junit.framework.TestCase.fail;

import com.google.common.collect.ImmutableSet;
import com.google.devtools.kythe.proto.Analysis;
import com.google.devtools.kythe.proto.Analysis.IndexedCompilation;
import com.google.devtools.kythe.proto.Go.GoDetails;
import com.google.devtools.kythe.util.JsonUtil;
import com.google.gson.JsonParseException;
import com.google.protobuf.util.JsonFormat;
import java.io.IOException;
import java.util.HashSet;
import java.util.Set;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

@RunWith(JUnit4.class)
public final class KZipReaderTest {

  private static final JsonFormat.TypeRegistry JSON_TYPE_REGISTRY =
      JsonFormat.TypeRegistry.newBuilder().add(GoDetails.getDescriptor()).build();

  @Before
  public void setup() {
    // This must be called so the json library knows how to read/write the go details message.
    JsonUtil.usingTypeRegistry(JSON_TYPE_REGISTRY);
  }

  @Test
  public void testOpenFailsForMissingFile() throws KZipException {
    try {
      new KZipReader(TestDataUtil.getTestFile("MISSING.kzip"));
      fail();
    } catch (IOException expected) {
    }
  }

  @Test
  public void testOpenFailsForMissingPbUnit() throws IOException {
    try {
      new KZipReader(TestDataUtil.getTestFile("missing-pbunit.kzip"));
      fail();
    } catch (KZipException expected) {
    }
  }

  @Test
  public void testOpenFailsForMissingJsonUnit() throws IOException {
    try {
      new KZipReader(TestDataUtil.getTestFile("missing-unit.kzip"));
      fail();
    } catch (KZipException expected) {
    }
  }

  @Test
  public void testOpenFailsForEmptyFile() throws KZipException {
    try {
      new KZipReader(TestDataUtil.getTestFile("empty.kzip"));
      fail();
    } catch (IOException expected) {
    }
  }

  @Test
  public void testOpenFailsForMissingRoot() throws IOException {
    try {
      new KZipReader(TestDataUtil.getTestFile("malformed.kzip"));
      fail();
    } catch (KZipException expected) {
    }
  }

  @Test
  public void testOpenFailsForGarbageUnit() throws IOException {
    try {
      KZipReader reader = new KZipReader(TestDataUtil.getTestFile("garbage_unit.kzip"));
      // Iterate over the units so we try to read in the garbage.
      for (IndexedCompilation compilation : reader.scan()) {}
      fail();
    } catch (JsonParseException expected) {
    }
  }

  @Test
  public void testKZipWithEmptyFileDoesNotCrash() throws IOException {
    KZipReader reader = new KZipReader(TestDataUtil.getTestFile("stringset_with_empty_file.kzip"));
    for (IndexedCompilation compilation : reader.scan()) {
      for (Analysis.CompilationUnit.FileInput fileInput :
          compilation.getUnit().getRequiredInputList()) {
        byte[] bytes = reader.readFile(fileInput.getInfo().getDigest());
        if (bytes.length == 0) {
          // we have properly read an empty file.
          return;
        }
      }
    }
    fail("Never read an empty file");
  }

  private static final Set<String> EXPECTED_STRINGSET_FILE_DIGESTS =
      ImmutableSet.of(
          "138b780e9b469f7b708ce7e3480e11a82bcf082a6530ea606d7f1236302048d2",
          "1990379205ae42e3fb7651b9e17441bd3a0f2feadf033fbaf2368cc1cd244f17",
          "3bdb25cf8fe309faf6bec11847d09ab5a3073671bc1994072a5f1bc1dfe39a29",
          "626dd9b3e7125cfa788c251db4849b35377bc464e27c8b1b48a910fb6d01ae9b",
          "b4cce2eabf0872848904909410f3cd461ea3781773bf3be2a9449a57ed1ed54b",
          "c85dc0a595232b78052c40d126f7c284c031131ea6d1d12404f6dd81f95ebf99",
          "dd4320b4dd84a248e0b7fcc22577e98941c4c48fca348c7ecd2e643f7a9f9707",
          "e8ae8761901127495388ac7b98110f88e5dee5a1b53b896df30303e1d87ad122",
          "f0297e8de7fc9ef71738bceaec525805ff09983f5d77a72c8f1cf29614dbca11",
          "f5dd2fdeb602e085677d2332e6a8ac6618ff18ed8c611043c6c363d5b7d975b6",
          "fc36b04dea7d47d3f1ae063dcea8029631915b02b98533a8a6f04449517cf7ca");

  @Test
  public void testOpenAndReadSimpleKZip() throws IOException, KZipException {
    int numFoundUnits = 0;
    Set<String> foundFiles = new HashSet<>();

    KZipReader reader = new KZipReader(TestDataUtil.getTestFile("stringset.kzip"));
    for (IndexedCompilation cu : reader.scan()) {
      numFoundUnits++;
      for (Analysis.CompilationUnit.FileInput file : cu.getUnit().getRequiredInputList()) {
        byte[] fileContents = reader.readFile(file.getInfo().getDigest());
        String hash = KZip.DATA_DIGEST.hashBytes(fileContents).toString();
        assertThat(EXPECTED_STRINGSET_FILE_DIGESTS).contains(hash);
        foundFiles.add(hash);
      }
    }

    // There should only be 1 unit in stringset.kzip
    assertThat(numFoundUnits).isEqualTo(1);
    assertThat(foundFiles).containsExactlyElementsIn(EXPECTED_STRINGSET_FILE_DIGESTS);
  }
}
