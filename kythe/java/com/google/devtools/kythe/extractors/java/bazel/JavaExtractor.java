/*
 * Copyright 2015 Google Inc. All rights reserved.
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

package com.google.devtools.kythe.extractors.java.bazel;

import com.google.common.base.Predicate;
import com.google.common.collect.Iterables;
import com.google.common.collect.Lists;
import com.google.devtools.build.lib.actions.extra.ExtraActionInfo;
import com.google.devtools.build.lib.actions.extra.ExtraActionsBase;
import com.google.devtools.build.lib.actions.extra.JavaCompileInfo;
import com.google.devtools.kythe.extractors.java.JavaCompilationUnitExtractor;
import com.google.devtools.kythe.extractors.shared.CompilationDescription;
import com.google.devtools.kythe.extractors.shared.ExtractionException;
import com.google.devtools.kythe.extractors.shared.FileVNames;
import com.google.devtools.kythe.extractors.shared.IndexInfoUtils;
import com.google.protobuf.CodedInputStream;
import com.google.protobuf.ExtensionRegistry;
import java.io.IOException;
import java.io.InputStream;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.List;

/** Java CompilationUnit extractor using Bazel's extra_action feature. */
public class JavaExtractor {
  public static void main(String[] args) throws IOException, ExtractionException {
    if (args.length != 3) {
      System.err.println("Usage: java_extractor extra-action-file output-file vname-config");
      System.exit(1);
    }

    String extraActionPath = args[0];
    String outputPath = args[1];
    String vNamesConfigPath = args[2];

    ExtensionRegistry registry = ExtensionRegistry.newInstance();
    ExtraActionsBase.registerAllExtensions(registry);

    ExtraActionInfo info;
    try (InputStream stream = Files.newInputStream(Paths.get(extraActionPath))) {
      CodedInputStream coded = CodedInputStream.newInstance(stream);
      info = ExtraActionInfo.parseFrom(coded, registry);
    }

    if (!info.hasExtension(JavaCompileInfo.javaCompileInfo)) {
      throw new IllegalArgumentException("Given ExtraActionInfo without JavaCompileInfo");
    }

    JavaCompileInfo jInfo = info.getExtension(JavaCompileInfo.javaCompileInfo);

    List<String> javacOpts =
        Lists.newArrayList(Iterables.filter(jInfo.getJavacOptList(), JAVAC_OPT_FILTER));

    // Set up a fresh output directory
    javacOpts.add("-d");
    Path output = Files.createTempDirectory("output");
    javacOpts.add(output.toString());

    // Add the generated sources directory if any processors could be invoked.
    if (!jInfo.getProcessorList().isEmpty()) {
      String gensrcdir = readGeneratedSourceDirParam(jInfo);
      if (gensrcdir != null) {
        javacOpts.add("-s");
        javacOpts.add(gensrcdir);
        // javac expects the directory to already exist.
        Files.createDirectories(Paths.get(gensrcdir));
      }
    }

    CompilationDescription description =
        new JavaCompilationUnitExtractor(
                FileVNames.fromFile(vNamesConfigPath), System.getProperty("user.dir"))
            .extract(
                info.getOwner(),
                jInfo.getSourceFileList(),
                jInfo.getClasspathList(),
                jInfo.getBootclasspathList(),
                jInfo.getSourcepathList(),
                jInfo.getProcessorpathList(),
                jInfo.getProcessorList(),
                javacOpts,
                jInfo.getOutputjar());

    IndexInfoUtils.writeIndexInfoToFile(description, outputPath);
  }

  /**
   * Reads Bazel's compilation params file and returns the value of the --sourcegendir flag. Returns
   * {@code null} if the value does not exist.
   */
  private static String readGeneratedSourceDirParam(JavaCompileInfo jInfo) throws IOException {
    try (java.io.BufferedReader params =
        Files.newBufferedReader(
            Paths.get(jInfo.getOutputjar() + "-2.params"),
            java.nio.charset.StandardCharsets.UTF_8)) {
      String line;
      while ((line = params.readLine()) != null) {
        if ("--sourcegendir".equals(line)) {
          return params.readLine();
        }
      }
      return null;
    }
  }

  // Predicate that filters out Bazel-specific flags.  Bazel adds its own flags (such as error-prone
  // flags) to the javac_opt list that cannot be handled by the standard javac compiler, or in turn,
  // by this extractor.
  private static final Predicate<String> JAVAC_OPT_FILTER =
      new Predicate<String>() {
        @Override
        public boolean apply(String opt) {
          return !(opt.startsWith("-Werror:")
              || opt.startsWith("-extra_checks")
              || opt.startsWith("-Xep"));
        }
      };
}
