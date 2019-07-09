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

package com.google.devtools.kythe.platform.shared.filesystem;

import static com.google.common.truth.Truth.assertThat;

import com.google.common.collect.ImmutableMap;
import com.google.devtools.kythe.extractors.shared.ExtractorUtils;
import com.google.devtools.kythe.platform.shared.FileDataCache;
import com.google.devtools.kythe.platform.shared.FileDataProvider;
import com.google.devtools.kythe.proto.Analysis.CompilationUnit;
import com.google.devtools.kythe.proto.Analysis.FileData;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.List;
import java.util.stream.Collectors;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

@RunWith(JUnit4.class)
public final class CompilationUnitFileSystemTest {
  private List<FileData> fileData =
      ExtractorUtils.convertBytesToFileDatas(
          new ImmutableMap.Builder<String, byte[]>()
              .put("relative/nested/path/with/empty/file", "relativeContents".getBytes())
              .put("/absolute/nested/path/with/empty/file", "absoluteContents".getBytes())
              .build());

  private CompilationUnit compilationUnit =
      CompilationUnit.newBuilder()
          .addAllRequiredInput(ExtractorUtils.toFileInputs(fileData))
          .build();

  private FileDataProvider fileDataProvider = new FileDataCache(fileData);

  private CompilationUnitFileSystem fileSystem =
      CompilationUnitFileSystem.create(compilationUnit, fileDataProvider);

  @Before
  public void setUp() {}

  @Test
  public void filesWalk_includesAllFiles() {
    try {
      List<String> found =
          Files.walk(fileSystem.getPath("/")).map(Path::toString).collect(Collectors.toList());
      assertThat(found)
          .containsExactly(
              "/",
              "/absolute",
              "/absolute/nested",
              "/absolute/nested/path",
              "/absolute/nested/path/with",
              "/absolute/nested/path/with/empty",
              "/absolute/nested/path/with/empty/file",
              "/relative",
              "/relative/nested",
              "/relative/nested/path",
              "/relative/nested/path/with",
              "/relative/nested/path/with/empty",
              "/relative/nested/path/with/empty/file");
    } catch (IOException exc) {
      throw new RuntimeException(exc);
    }
  }

  @Test
  public void filesWalk_readsAllFiles() {
    try {
      List<String> contents =
          Files.walk(fileSystem.getPath("/"))
              .filter(p -> Files.isRegularFile(p))
              .map(
                  p -> {
                    try {
                      return Files.readString(p);
                    } catch (IOException exc) {
                      throw new RuntimeException(exc);
                    }
                  })
              .collect(Collectors.toList());
      assertThat(contents).containsExactly("relativeContents", "absoluteContents");
    } catch (IOException exc) {
      throw new RuntimeException(exc);
    }
  }
}
