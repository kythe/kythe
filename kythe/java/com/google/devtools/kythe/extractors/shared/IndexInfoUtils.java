/*
 * Copyright 2014 The Kythe Authors. All rights reserved.
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
import com.google.devtools.kythe.platform.kzip.KZip;
import com.google.devtools.kythe.platform.kzip.KZipException;
import com.google.devtools.kythe.platform.kzip.KZipReader;
import com.google.devtools.kythe.platform.kzip.KZipWriter;
import com.google.devtools.kythe.proto.Analysis;
import com.google.devtools.kythe.proto.Analysis.CompilationUnit;
import com.google.devtools.kythe.proto.Analysis.CompilationUnit.FileInput;
import com.google.devtools.kythe.proto.Analysis.FileData;
import com.google.devtools.kythe.proto.Analysis.FileInfo;
import com.google.devtools.kythe.proto.Analysis.IndexedCompilation;
import com.google.protobuf.ByteString;
import com.google.protobuf.CodedInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.Collection;
import java.util.HashSet;
import java.util.List;
import java.util.Set;
import java.util.zip.GZIPInputStream;
import java.util.zip.GZIPOutputStream;

/** Utilities to read and write compilation index information in .kindex files. */
public class IndexInfoUtils {

  public static final String KINDEX_FILE_EXT = ".kindex";
  public static final String KZIP_FILE_EXT = ".kzip";

  public static CompilationDescription readKindexInfoFromFile(String indexInfoFilename)
      throws IOException {
    checkArgument(!Strings.isNullOrEmpty(indexInfoFilename), "indexInfoFilename");

    try (InputStream indexInfoInputStream =
        new GZIPInputStream(new FileInputStream(indexInfoFilename))) {
      return readKindexInfoFromStream(indexInfoInputStream);
    }
  }

  public static CompilationDescription readKindexInfoFromStream(InputStream inputStream)
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

  public static Collection<CompilationDescription> readKZip(String path)
      throws IOException, KZipException {
    KZip.Reader reader = new KZipReader(new File(path));
    List<CompilationDescription> compilations = new ArrayList<>();
    for (IndexedCompilation indexedCompilation : reader.scan()) {
      compilations.add(indexedCompilationToCompilationDescription(indexedCompilation, reader));
    }
    return compilations;
  }

  public static CompilationDescription indexedCompilationToCompilationDescription(
      IndexedCompilation indexedCompilation, KZip.Reader reader) {
    CompilationUnit unit = indexedCompilation.getUnit();
    Set<FileData> fileContents = new HashSet<>();

    for (FileInput info : unit.getRequiredInputList()) {
      String digest = info.getInfo().getDigest();
      byte[] fileContent = reader.readFile(digest);
      FileData fileData =
          FileData.newBuilder()
              .setContent(ByteString.copyFrom(fileContent))
              .setInfo(
                  FileInfo.newBuilder().setDigest(digest).setPath(info.getInfo().getPath()).build())
              .build();
      fileContents.add(fileData);
    }

    return new CompilationDescription(unit, fileContents);
  }

  public static void writeKindexInfoToStream(
      CompilationDescription description, OutputStream stream) throws IOException {
    try (OutputStream indexOutputStream = new GZIPOutputStream(stream)) {
      description.getCompilationUnit().writeDelimitedTo(indexOutputStream);
      for (FileData fileContent : description.getFileContents()) {
        fileContent.writeDelimitedTo(indexOutputStream);
      }
    }
  }

  public static void writeKindexToFile(CompilationDescription description, String path)
      throws IOException {
    Paths.get(path).getParent().toFile().mkdirs();
    writeKindexInfoToStream(description, new FileOutputStream(path));
  }

  public static void writeKzipToFile(CompilationDescription description, String path)
      throws IOException {
    Paths.get(path).getParent().toFile().mkdirs();
    try (KZip.Writer writer = new KZipWriter(new File(path))) {
      Analysis.IndexedCompilation indexedCompilation =
          Analysis.IndexedCompilation.newBuilder()
              .setUnit(description.getCompilationUnit())
              .build();
      writer.writeUnit(indexedCompilation);
      for (FileData fileData : description.getFileContents()) {
        writer.writeFile(fileData.getContent().toByteArray());
      }
    }
  }

  public static Path getKzipPath(String rootDirectory, String basename) {
    return Paths.get(rootDirectory, basename + KZIP_FILE_EXT);
  }
}
