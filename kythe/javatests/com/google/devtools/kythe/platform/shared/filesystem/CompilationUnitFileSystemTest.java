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
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.List;
import java.util.stream.Collectors;
import java.util.stream.Stream;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

@RunWith(JUnit4.class)
public final class CompilationUnitFileSystemTest {
  private static class FileSystemBuilder {
    private final ImmutableMap.Builder<String, byte[]> inputFiles = new ImmutableMap.Builder<>();
    private String workingDirectory = "";

    FileSystemBuilder addFile(String path, String contents) {
      inputFiles.put(path, contents.getBytes(StandardCharsets.UTF_8));
      return this;
    }

    FileSystemBuilder setWorkingDirectory(String path) {
      workingDirectory = path;
      return this;
    }

    CompilationUnitFileSystem build() {
      List<FileData> fileData = ExtractorUtils.convertBytesToFileDatas(inputFiles.build());
      CompilationUnit compilationUnit =
          CompilationUnit.newBuilder()
              .addAllRequiredInput(ExtractorUtils.toFileInputs(fileData))
              .setWorkingDirectory(workingDirectory)
              .build();
      FileDataProvider fileDataProvider = new FileDataCache(fileData);
      return CompilationUnitFileSystem.create(compilationUnit, fileDataProvider);
    }
  }

  static FileSystemBuilder builder() {
    return new FileSystemBuilder();
  }

  @Before
  public void setUp() {}

  @Test
  public void filesWalk_includesAllFiles() {
    CompilationUnitFileSystem fileSystem =
        builder()
            .addFile("relative/nested/path/with/empty/file", "")
            .addFile("/absolute/nested/path/with/empty/file", "")
            .build();
    try {
      List<String> found;
      try (Stream<Path> stream = Files.walk(fileSystem.getPath("/"))) {
        found = stream.map(Path::toString).collect(Collectors.toList());
      }
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
  public void filesWalk_includesAllFilesFromWorkingDirectoy() {
    CompilationUnitFileSystem fileSystem =
        builder()
            .addFile("relative/nested/path/with/empty/file", "")
            .addFile("/absolute/nested/path/with/empty/file", "")
            .setWorkingDirectory("/a/different/path")
            .build();
    try {
      List<String> found;
      try (Stream<Path> stream = Files.walk(fileSystem.getPath("/"))) {
        found = stream.map(Path::toString).collect(Collectors.toList());
      }
      assertThat(found)
          .containsExactly(
              "/",
              "/absolute",
              "/absolute/nested",
              "/absolute/nested/path",
              "/absolute/nested/path/with",
              "/absolute/nested/path/with/empty",
              "/absolute/nested/path/with/empty/file",
              "/a/different/path/relative",
              "/a/different/path/relative/nested",
              "/a/different/path/relative/nested/path",
              "/a/different/path/relative/nested/path/with",
              "/a/different/path/relative/nested/path/with/empty",
              "/a/different/path/relative/nested/path/with/empty/file",
              // Entries added due to the working directory.
              "/a",
              "/a/different",
              "/a/different/path");
    } catch (IOException exc) {
      throw new RuntimeException(exc);
    }
  }

  @Test
  public void filesWalk_readsAllFiles() {
    CompilationUnitFileSystem fileSystem =
        builder()
            .addFile("relative/nested/path/with/empty/file", "relativeContents")
            .addFile("/absolute/nested/path/with/empty/file", "absoluteContents")
            .build();
    try {
      List<String> contents;
      try (Stream<Path> stream = Files.walk(fileSystem.getPath("/"))) {
        contents =
            stream
                .filter(p -> Files.isRegularFile(p))
                .map(
                    p -> {
                      try {
                        return new String(Files.readAllBytes(p), StandardCharsets.UTF_8);
                      } catch (IOException exc) {
                        throw new RuntimeException(exc);
                      }
                    })
                .collect(Collectors.toList());
      }
      assertThat(contents).containsExactly("relativeContents", "absoluteContents");
    } catch (IOException exc) {
      throw new RuntimeException(exc);
    }
  }
}
