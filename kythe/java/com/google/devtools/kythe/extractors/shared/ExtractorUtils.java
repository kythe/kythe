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
import static com.google.common.base.StandardSystemProperty.USER_DIR;
import static com.google.common.collect.ImmutableList.toImmutableList;
import static com.google.common.hash.Hashing.sha256;
import static java.util.stream.Collectors.toCollection;

import com.google.common.annotations.VisibleForTesting;
import com.google.common.collect.Ordering;
import com.google.common.collect.Streams;
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
import java.util.ArrayList;
import java.util.Comparator;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ExecutionException;
import java.util.function.Function;

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
  public static final Comparator<FileInput> FILE_INPUT_COMPARATOR =
      Comparator.comparing(FileInput::getInfo, Comparator.comparing(FileInfo::getPath));

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

    return filePathToFileContents
        .keySet()
        .stream()
        .map(path -> createFileData(path, filePathToFileContents.get(path)))
        .collect(toCollection(ArrayList::new));
  }

  public static FileData createFileData(String path, byte[] content) {
    return createFileData(path, getContentDigest(content), content);
  }

  public static List<FileData> processRequiredInputs(Iterable<String> files)
      throws ExtractionException {
    final SettableFuture<ExtractionException> exception = SettableFuture.create();

    List<FileData> result =
        Streams.stream(files)
            .map(
                path -> {
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
                })
            .collect(toCollection(ArrayList::new));
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

  public static List<FileInput> toFileInputs(Iterable<FileData> fileDatas) {
    return toFileInputs(FileVNames.staticCorpus(""), p -> p, fileDatas);
  }

  public static Function<String, String> makeRelativizer(final Path rootDir) {
    return p -> tryMakeRelative(rootDir, Paths.get(p));
  }

  public static List<FileInput> toFileInputs(
      FileVNames fileVNames, Function<String, String> relativize, Iterable<FileData> fileDatas) {
    return Streams.stream(fileDatas)
        .map(
            fileData -> {
              VName vname = lookupVName(fileVNames, relativize, fileData.getInfo().getPath());
              return FileInput.newBuilder().setInfo(fileData.getInfo()).setVName(vname).build();
            })
        .sorted(FILE_INPUT_COMPARATOR)
        .collect(toImmutableList());
  }

  public static VName lookupVName(
      FileVNames fileVNames, Function<String, String> relativize, String path) {
    String relativePath = relativize.apply(path);
    VName vname = fileVNames.lookupBaseVName(relativePath);
    if (vname.getPath().isEmpty()) {
      vname = vname.toBuilder().setPath(relativePath).build();
    }
    return vname;
  }

  /** Tries to make a path relative to a root directory. Returns the fullpath otherwise. */
  public static String tryMakeRelative(String rootDir, String path) {
    return tryMakeRelative(Paths.get(rootDir), Paths.get(path));
  }

  /** Tries to make a path relative to a root directory. Returns the fullpath otherwise. */
  public static String tryMakeRelative(Path rootDir, Path path) {
    Path absRoot = rootDir.toAbsolutePath().normalize();
    Path absPath = path.toAbsolutePath().normalize();
    Path relPath = absRoot.relativize(absPath).normalize();
    if (relPath.toString().isEmpty()) {
      return ".";
    }
    return (relPath.startsWith("..") ? absPath : relPath).toString();
  }

  public static String getCurrentWorkingDirectory() {
    return USER_DIR.value();
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
    List<FileInput> oldRequiredInputs =
        Ordering.from(FILE_INPUT_COMPARATOR).sortedCopy(builder.getRequiredInputList());
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
