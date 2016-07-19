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

package com.google.devtools.kythe.extractors.shared;

import static com.google.common.base.Preconditions.checkNotNull;
import static com.google.common.hash.Hashing.sha256;

import com.google.common.annotations.VisibleForTesting;
import com.google.common.base.Function;
import com.google.common.collect.ImmutableList;
import com.google.common.collect.Iterables;
import com.google.common.collect.Lists;
import com.google.common.io.Files;
import com.google.common.util.concurrent.SettableFuture;
import com.google.devtools.kythe.proto.Analysis.CompilationUnit;
import com.google.devtools.kythe.proto.Analysis.CompilationUnit.FileInput;
import com.google.devtools.kythe.proto.Analysis.FileData;
import com.google.devtools.kythe.proto.Analysis.FileInfo;
import com.google.devtools.kythe.proto.Storage.VName;
import com.google.protobuf.ByteString;
import java.io.File;
import java.io.IOException;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.security.NoSuchAlgorithmException;
import java.util.Collections;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ExecutionException;

/**
 * A class containing common utilities used by language extractors.
 *
 * This class is a bit of a mishmash. It includes helpers for:
 *  - dealing with Google 3 specific file paths
 *  - loading files into FileData
 *  - converting from byte[] to FileData
 */
// TODO: Split this class by domain.
public class ExtractorUtils {

  /**
   * Creates fully populated FileInput protocol buffers based on a provided set of files.
   *
   * @param filePathToFileDatas map with file contents.
   * @return fully populated FileInput protos
   * @throws ExtractionException
   */
  public static List<FileData> convertBytesToFileDatas(
      final Map<String, byte[]> filePathToFileContents) throws ExtractionException {
    checkNotNull(filePathToFileContents);

    return Lists.newArrayList(
        Iterables.transform(
            filePathToFileContents.keySet(),
            new Function<String, FileData>() {
              @Override
              public FileData apply(String path) {
                return createFileData(path, filePathToFileContents.get(path));
              }
            }));
  }

  public static FileData createFileData(String path, byte[] content) {
    return createFileData(path, getContentDigest(content), content);
  }

  public static List<FileData> processRequiredInputs(Iterable<String> files)
      throws ExtractionException {
    final SettableFuture<ExtractionException> exception = SettableFuture.create();

    List<FileData> result =
        Lists.newArrayList(
            Iterables.transform(
                files,
                new Function<String, FileData>() {
                  @Override
                  public FileData apply(String path) {
                    byte[] content = new byte[0];
                    try {
                      content = Files.toByteArray(new File(path));
                    } catch (IOException e) {
                      exception.set(new ExtractionException(e, false));
                    }
                    if (content == null) {
                      exception.set(
                          new ExtractionException(
                              String.format("Unable to locate required input %s", path), false));
                      return null;
                    }
                    String digest = getContentDigest(content);
                    return createFileData(path, digest, content);
                  }
                }));
    if (exception.isDone()) {
      try {
        throw exception.get();
      } catch (InterruptedException e) {
        throw new ExtractionException(e, true);
      } catch (ExecutionException e) {
        throw new ExtractionException(e, false);
      }
    }
    return result;
  }

  private static final Function<FileData, FileInput> FILE_DATA_TO_COMPILATION_FILE_INPUT =
      new Function<FileData, FileInput>() {
        @Override
        public FileInput apply(FileData fileData) {
          return FileInput.newBuilder()
              .setInfo(fileData.getInfo())
              .setVName(
                  VName.newBuilder()
                      // TODO(schroederc): VName path should be corpus+root relative
                      .setPath(fileData.getInfo().getPath())
                      .build())
              .build();
        }
      };

  public static List<FileInput> toFileInputs(Iterable<FileData> fileDatas) {
    return ImmutableList.copyOf(
        Iterables.transform(fileDatas, FILE_DATA_TO_COMPILATION_FILE_INPUT));
  }

  /**
   * Tries to make a path relative based on the current working dir. Returns the fullpath otherwise.
   */
  public static String tryMakeRelative(String rootDir, String path) {
    Path absPath = Paths.get(path).toAbsolutePath().normalize();
    Path relPath = Paths.get(rootDir).toAbsolutePath().relativize(absPath).normalize();
    if (relPath.toString().isEmpty()) {
      return ".";
    }
    return (relPath.startsWith("..") ? absPath : relPath).toString();
  }

  public static String getCurrentWorkingDirectory() {
    return System.getProperty("user.dir");
  }

  public static String digestForPath(String path) throws NoSuchAlgorithmException, IOException {
    return digestForFile(new File(path));
  }

  public static String digestForFile(File file) throws NoSuchAlgorithmException, IOException {
    return getContentDigest(Files.toByteArray(file));
  }

  /** Computes a digest over the contents of a file. */
  @VisibleForTesting
  public static String getContentDigest(byte[] content) {
    return sha256().hashBytes(content).toString();
  }

  public static CompilationUnit normalizeCompilationUnit(CompilationUnit existingCompilationUnit) {
    CompilationUnit.Builder builder = CompilationUnit.newBuilder(existingCompilationUnit);
    List<FileInput> oldRequiredInputs = Lists.newArrayList(builder.getRequiredInputList());
    Collections.sort(oldRequiredInputs, CompilationFileInputComparator.getComparator());
    builder.clearRequiredInput();
    builder.addAllRequiredInput(oldRequiredInputs);
    existingCompilationUnit = builder.build();
    return existingCompilationUnit;
  }

  private static FileData createFileData(String path, String digest, byte[] content) {
    return FileData.newBuilder()
        .setContent(ByteString.copyFrom(content))
        .setInfo(FileInfo.newBuilder().setDigest(digest).setPath(path))
        .build();
  }
}
