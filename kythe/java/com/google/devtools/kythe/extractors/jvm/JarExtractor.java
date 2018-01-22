/*
 * Copyright 2018 Google Inc. All rights reserved.
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

package com.google.devtools.kythe.extractors.jvm;

import com.google.common.base.Joiner;
import com.google.common.base.Strings;
import com.google.common.hash.Hashing;
import com.google.common.io.ByteStreams;
import com.google.devtools.kythe.analyzers.jvm.JvmGraph;
import com.google.devtools.kythe.extractors.shared.CompilationDescription;
import com.google.devtools.kythe.extractors.shared.ExtractionException;
import com.google.devtools.kythe.extractors.shared.ExtractorUtils;
import com.google.devtools.kythe.extractors.shared.IndexInfoUtils;
import com.google.devtools.kythe.platform.indexpack.Archive;
import com.google.devtools.kythe.proto.Analysis.CompilationUnit;
import com.google.devtools.kythe.proto.Analysis.FileData;
import com.google.devtools.kythe.proto.Storage.VName;
import java.io.IOException;
import java.io.InputStream;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.Enumeration;
import java.util.List;
import java.util.jar.JarEntry;
import java.util.jar.JarFile;

/**
 * Kythe extractor for Java .jar files.
 *
 * <p>Usage: jar_extractor <.jar file | .class file>*
 */
public class JarExtractor {
  public static void main(String[] args) throws IOException, ExtractionException {
    CompilationUnit.Builder compilation =
        CompilationUnit.newBuilder()
            .setVName(VName.newBuilder().setLanguage(JvmGraph.JVM_LANGUAGE));
    List<FileData> fileContents = new ArrayList<>();
    List<String> classFiles = new ArrayList<>();
    for (String arg : args) {
      compilation.addArgument(arg);
      if (arg.endsWith(".jar")) {
        fileContents.addAll(extractClassFiles(Paths.get(arg)));
      } else {
        classFiles.add(arg);
      }
    }
    fileContents.addAll(ExtractorUtils.processRequiredInputs(classFiles));
    compilation.addAllRequiredInput(ExtractorUtils.toFileInputs(fileContents));

    CompilationDescription indexInfo =
        new CompilationDescription(compilation.build(), fileContents);

    String outputFile = System.getenv("KYTHE_OUTPUT_FILE");
    if (!Strings.isNullOrEmpty(outputFile)) {
      IndexInfoUtils.writeIndexInfoToFile(indexInfo, outputFile);
    } else {
      String outputDir = System.getenv("KYTHE_OUTPUT_DIRECTORY");
      if (Strings.isNullOrEmpty(outputDir)) {
        throw new IllegalArgumentException(
            "required KYTHE_OUTPUT_DIRECTORY environment variable is unset");
      }
      if (Strings.isNullOrEmpty(System.getenv("KYTHE_INDEX_PACK"))) {
        String name = Hashing.sha256().hashUnencodedChars(Joiner.on(" ").join(args)).toString();
        String path = IndexInfoUtils.getIndexPath(outputDir, name).toString();
        IndexInfoUtils.writeIndexInfoToFile(indexInfo, path);
      } else {
        new Archive(outputDir).writeDescription(indexInfo);
      }
    }
  }

  private static List<FileData> extractClassFiles(Path jarPath) throws IOException {
    List<FileData> files = new ArrayList<>();
    try (JarFile jar = new JarFile(jarPath.toFile())) {
      for (Enumeration<JarEntry> entries = jar.entries(); entries.hasMoreElements(); ) {
        JarEntry entry = entries.nextElement();
        if (!entry.getName().endsWith(".class")) {
          continue;
        }
        try (InputStream input = jar.getInputStream(entry)) {
          String path = jarPath.resolve(entry.getName()).toString();
          byte[] contents = ByteStreams.toByteArray(input);
          files.add(ExtractorUtils.createFileData(path, contents));
        }
      }
    }
    return files;
  }
}
