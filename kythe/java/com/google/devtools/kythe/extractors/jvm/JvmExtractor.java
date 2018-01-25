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

import com.google.common.io.ByteStreams;
import com.google.devtools.kythe.analyzers.jvm.JvmGraph;
import com.google.devtools.kythe.extractors.shared.CompilationDescription;
import com.google.devtools.kythe.extractors.shared.ExtractionException;
import com.google.devtools.kythe.extractors.shared.ExtractorUtils;
import com.google.devtools.kythe.proto.Analysis.CompilationUnit;
import com.google.devtools.kythe.proto.Analysis.FileData;
import com.google.devtools.kythe.proto.Storage.VName;
import java.io.IOException;
import java.io.InputStream;
import java.nio.file.Path;
import java.util.ArrayList;
import java.util.Enumeration;
import java.util.List;
import java.util.jar.JarEntry;
import java.util.jar.JarFile;

/** Kythe extractor for Java .jar/.class files. */
public class JvmExtractor {
  public static final String JAR_FILE_EXTENSION = ".jar";
  public static final String CLASS_FILE_EXTENSION = ".class";

  /**
   * Returns a JVM {@link CompilationDescription} for the given list of {@code .jar} and {@code
   * .class} file paths.
   */
  public static CompilationDescription extract(List<Path> jarOrClassFiles)
      throws IOException, ExtractionException {
    CompilationUnit.Builder compilation =
        CompilationUnit.newBuilder()
            .setVName(VName.newBuilder().setLanguage(JvmGraph.JVM_LANGUAGE));
    List<FileData> fileContents = new ArrayList<>();
    List<String> classFiles = new ArrayList<>();
    for (Path path : jarOrClassFiles) {
      compilation.addArgument(path.toString());
      compilation.addSourceFile(path.toString());
      if (path.toString().endsWith(JAR_FILE_EXTENSION)) {
        fileContents.addAll(extractClassFiles(path));
      } else {
        classFiles.add(path.toString());
      }
    }
    fileContents.addAll(ExtractorUtils.processRequiredInputs(classFiles));
    compilation.addAllRequiredInput(ExtractorUtils.toFileInputs(fileContents));

    return new CompilationDescription(compilation.build(), fileContents);
  }

  private static List<FileData> extractClassFiles(Path jarPath) throws IOException {
    List<FileData> files = new ArrayList<>();
    try (JarFile jar = new JarFile(jarPath.toFile())) {
      for (Enumeration<JarEntry> entries = jar.entries(); entries.hasMoreElements(); ) {
        JarEntry entry = entries.nextElement();
        if (!entry.getName().endsWith(CLASS_FILE_EXTENSION)) {
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
