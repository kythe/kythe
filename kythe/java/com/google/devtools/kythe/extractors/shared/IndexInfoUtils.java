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

import static com.google.common.base.Preconditions.checkArgument;
import static com.google.common.base.Preconditions.checkNotNull;

import com.google.common.base.Strings;
import com.google.devtools.kythe.proto.Analysis.CompilationUnit;
import com.google.devtools.kythe.proto.Analysis.FileData;
import com.google.protobuf.CodedInputStream;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.List;
import java.util.zip.GZIPInputStream;
import java.util.zip.GZIPOutputStream;

/** Utilities to read and write compilation index information in .kindex files. */
public class IndexInfoUtils {
  public static final String INDEX_FILE_EXT = ".kindex";

  public static CompilationDescription readIndexInfoFromFile(String indexInfoFilename)
      throws IOException {
    checkArgument(!Strings.isNullOrEmpty(indexInfoFilename), "indexInfoFilename");

    try (InputStream indexInfoInputStream =
        new GZIPInputStream(new FileInputStream(indexInfoFilename))) {
      return readIndexInfoFromStream(indexInfoInputStream);
    }
  }

  public static CompilationDescription readIndexInfoFromStream(InputStream inputStream)
      throws IOException {
    checkNotNull(inputStream, "inputStream");

    // We do not use parseDelimitedFrom but use CodedInputStream manually,
    // as it allows us to control the size limit on each individual protobuf message read.
    CodedInputStream codedStream = CodedInputStream.newInstance(inputStream);
    codedStream.setSizeLimit(Integer.MAX_VALUE);

    CompilationUnit compilationUnit = CompilationUnit.parseFrom(codedStream.readBytes());
    List<FileData> fileContents = new ArrayList<>();
    while (!codedStream.isAtEnd()) {
      fileContents.add(FileData.parseFrom(codedStream.readBytes()));
    }
    return new CompilationDescription(compilationUnit, fileContents);
  }

  public static void writeIndexInfoToStream(CompilationDescription description, OutputStream stream)
      throws IOException {
    try (OutputStream indexOutputStream = new GZIPOutputStream(stream)) {
      description.getCompilationUnit().writeDelimitedTo(indexOutputStream);
      for (FileData fileContent : description.getFileContents()) {
        fileContent.writeDelimitedTo(indexOutputStream);
      }
    }
  }

  public static void writeIndexInfoToFile(CompilationDescription description, String path)
      throws IOException {
    Paths.get(path).getParent().toFile().mkdirs();
    writeIndexInfoToStream(description, new FileOutputStream(path));
  }

  public static Path getIndexPath(String rootDirectory, String basename) {
    return Paths.get(rootDirectory, basename + INDEX_FILE_EXT);
  }
}
