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

import com.beust.jcommander.Parameter;
import com.google.common.base.Strings;
import com.google.common.io.ByteStreams;
import com.google.devtools.kythe.analyzers.jvm.JvmGraph;
import com.google.devtools.kythe.extractors.shared.CompilationDescription;
import com.google.devtools.kythe.extractors.shared.ExtractionException;
import com.google.devtools.kythe.extractors.shared.ExtractorUtils;
import com.google.devtools.kythe.extractors.shared.FileVNames;
import com.google.devtools.kythe.proto.Analysis.CompilationUnit;
import com.google.devtools.kythe.proto.Analysis.FileData;
import com.google.devtools.kythe.proto.Buildinfo.BuildDetails;
import com.google.devtools.kythe.proto.Java.JarDetails;
import com.google.devtools.kythe.proto.Storage.VName;
import com.google.protobuf.Any;
import java.io.IOException;
import java.io.InputStream;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.Enumeration;
import java.util.List;
import java.util.Optional;
import java.util.function.Function;
import java.util.jar.JarEntry;
import java.util.jar.JarFile;

/** Kythe extractor for Java .jar/.class files. */
public class JvmExtractor {
  public static final String JAR_FILE_EXTENSION = ".jar";
  public static final String CLASS_FILE_EXTENSION = ".class";
  public static final String BUILD_DETAILS_URL = "kythe.io/proto/kythe.proto.BuildDetails";
  public static final String JAR_DETAILS_URL = "kythe.io/proto/kythe.proto.JarDetails";

  /**
   * Returns a JVM {@link CompilationDescription} for the {@code .jar}/{@code .class} file paths
   * specified by the given {@link Options}.
   */
  public static CompilationDescription extract(Options options)
      throws IOException, ExtractionException {
    CompilationUnit.Builder compilation =
        CompilationUnit.newBuilder()
            .setVName(VName.newBuilder().setLanguage(JvmGraph.JVM_LANGUAGE));

    FileVNames fileVNames;
    if (options.vnamesConfigFile != null) {
      fileVNames = FileVNames.fromFile(options.vnamesConfigFile);
    } else {
      fileVNames = FileVNames.staticCorpus(options.defaultCorpus);
    }

    String buildTarget;
    if ((buildTarget = Strings.emptyToNull(options.buildTarget)) != null) {
      compilation.addDetails(
          Any.newBuilder()
              .setTypeUrl(BUILD_DETAILS_URL)
              .setValue(
                  BuildDetails.newBuilder().setBuildTarget(buildTarget).build().toByteString()));
    }

    Function<String, String> relativizer = ExtractorUtils.makeRelativizer(options.rootDirectory);

    List<FileData> fileContents = new ArrayList<>();
    List<String> classFiles = new ArrayList<>();
    JarDetails.Builder jarDetails = JarDetails.newBuilder();
    for (Path path : options.jarOrClassFiles) {
      compilation.addArgument(path.toString());
      compilation.addSourceFile(path.toString());
      if (path.toString().endsWith(JAR_FILE_EXTENSION)) {
        VName jarVName = ExtractorUtils.lookupVName(fileVNames, relativizer, path.toString());
        JarDetails.Jar.Builder jar = jarDetails.addJarBuilder().setVName(jarVName);
        for (FileData file : extractClassFiles(path)) {
          jar.addEntry(file.getInfo());
          fileContents.add(file);
        }
      } else {
        classFiles.add(path.toString());
      }
    }
    fileContents.addAll(ExtractorUtils.processRequiredInputs(classFiles));
    compilation.addAllRequiredInput(
        ExtractorUtils.toFileInputs(fileVNames, relativizer, fileContents));

    if (jarDetails.getJarCount() > 0) {
      compilation.addDetails(
          Any.newBuilder().setTypeUrl(JAR_DETAILS_URL).setValue(jarDetails.build().toByteString()));
    }

    if (fileContents.size() > options.maxRequiredInputs) {
      throw new ExtractionException(
          String.format(
              "number of required inputs (%d) exceeded maximum (%d)",
              fileContents.size(), options.maxRequiredInputs),
          false);
    }

    long totalFileSize = 0;
    for (FileData data : fileContents) {
      totalFileSize += data.getContent().size();
    }
    if (totalFileSize > options.maxTotalFileSize) {
      throw new ExtractionException(
          String.format(
              "total size of inputs (%d) exceeded maximum (%d)",
              totalFileSize, options.maxTotalFileSize),
          false);
    }

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

  /**
   * Extractor options controlling which files to extract and what limits to place upon the
   * resulting {@link CompilationUnit}.
   */
  public static class Options {
    @Parameter(
      names = {"--help", "-h"},
      description = "Help requested",
      help = true
    )
    public boolean help;

    @Parameter(
      names = "--max_required_inputs",
      description = "Maximum allowed number of required_inputs per CompilationUnit"
    )
    public int maxRequiredInputs = 1024 * 16;

    @Parameter(
      names = "--max_total_file_size",
      description = "Maximum allowed total size (bytes) of all input files per CompilationUnit"
    )
    public long maxTotalFileSize = 1024 * 1024 * 64;

    @Parameter(names = "--build_target", description = "Name of build target being extracted")
    public String buildTarget = System.getenv("KYTHE_ANALYSIS_TARGET");

    @Parameter(names = "--default_corpus", description = "Default file VName corpus")
    public String defaultCorpus = System.getenv("KYTHE_CORPUS");

    @Parameter(
      names = "--root_directory",
      description = "Root directory for compilation (defaults to $PWD)"
    )
    public Path rootDirectory =
        Paths.get(Optional.ofNullable(System.getenv("KYTHE_ROOT_DIRECTORY")).orElse(""));

    @Parameter(names = "--vnames_config", description = "Path to JSON VNames configuration file")
    public Path vnamesConfigFile =
        Optional.ofNullable(System.getenv("KYTHE_VNAMES")).map(Paths::get).orElse(null);

    @Parameter(description = "<.jar file | .class file>", required = true)
    public List<Path> jarOrClassFiles = new ArrayList<>();
  }
}
