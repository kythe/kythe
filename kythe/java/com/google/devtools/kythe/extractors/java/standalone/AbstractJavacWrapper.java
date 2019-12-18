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

package com.google.devtools.kythe.extractors.java.standalone;

import com.google.common.base.Joiner;
import com.google.common.base.Splitter;
import com.google.common.base.Strings;
import com.google.common.base.Throwables;
import com.google.common.collect.Lists;
import com.google.common.collect.Ordering;
import com.google.common.hash.Hashing;
import com.google.devtools.kythe.extractors.java.JavaCompilationUnitExtractor;
import com.google.devtools.kythe.extractors.shared.CompilationDescription;
import com.google.devtools.kythe.extractors.shared.FileVNames;
import com.google.devtools.kythe.extractors.shared.IndexInfoUtils;
import com.google.devtools.kythe.proto.Analysis.CompilationUnit;
import com.google.devtools.kythe.proto.Analysis.FileData;
import com.google.devtools.kythe.util.JsonUtil;
import com.sun.tools.javac.main.CommandLine;
import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.List;

/**
 * General logic for a javac-based {@link CompilationUnit} extractor.
 *
 * <p>Environment Variables Used (note that these can also be set as JVM system properties):
 *
 * <p>KYTHE_VNAMES: optional path to a JSON configuration file for {@link FileVNames} to populate
 * the {@link CompilationUnit}'s required input {@link VName}s
 *
 * <p>KYTHE_CORPUS: if KYTHE_VNAMES is not given, all {@link VName}s will be populated with this
 * corpus (default {@link DEFAULT_CORPUS})
 *
 * <p>KYTHE_ROOT_DIRECTORY: required root path for file inputs; the {@link FileData} paths stored in
 * the {@link CompilationUnit} will be made to be relative to this directory
 *
 * <p>KYTHE_OUTPUT_FILE: if set to a non-empty value, write the resulting .kzip file to this path
 * instead of using KYTHE_OUTPUT_DIRECTORY
 *
 * <p>KYTHE_OUTPUT_DIRECTORY: directory path to store the resulting .kzip file, if KYTHE_OUTPUT_FILE
 * is not set
 */
public abstract class AbstractJavacWrapper {
  public static final String DEFAULT_CORPUS = "kythe";

  protected abstract CompilationDescription processCompilation(
      String[] arguments, JavaCompilationUnitExtractor javaCompilationUnitExtractor)
      throws Exception;

  protected abstract void passThrough(String[] args) throws Exception;

  /**
   * Given the command-line arguments to javac, construct a {@link CompilationUnit} and write it to
   * a .kindex file. Parameters to the extraction logic are passed by environment variables (see
   * class comment).
   */
  public void process(String[] args) {
    JsonUtil.usingTypeRegistry(JsonUtil.JSON_TYPE_REGISTRY);
    try {
      if (!passThroughIfAnalysisOnly(args)) {
        String vnamesConfig = System.getenv("KYTHE_VNAMES");
        JavaCompilationUnitExtractor extractor;
        if (Strings.isNullOrEmpty(vnamesConfig)) {
          String corpus = readEnvironmentVariable("KYTHE_CORPUS", DEFAULT_CORPUS);
          extractor =
              new JavaCompilationUnitExtractor(
                  corpus, readEnvironmentVariable("KYTHE_ROOT_DIRECTORY"));
        } else {
          extractor =
              new JavaCompilationUnitExtractor(
                  FileVNames.fromFile(vnamesConfig),
                  readEnvironmentVariable("KYTHE_ROOT_DIRECTORY"));
        }

        CompilationDescription indexInfo =
            processCompilation(getCleanedUpArguments(args), extractor);
        outputIndexInfo(indexInfo);

        CompilationUnit compilationUnit = indexInfo.getCompilationUnit();
        if (compilationUnit.getHasCompileErrors()) {
          System.err.println("Errors encountered during compilation");
          System.exit(1);
        }
      }
    } catch (IOException e) {
      System.err.printf(
          "Unexpected IO error (probably while writing to index file): %s%n", e.toString());
      System.err.println(Throwables.getStackTraceAsString(e));
      System.exit(2);
    } catch (Exception e) {
      System.err.printf(
          "Unexpected error compiling and indexing java compilation: %s%n", e.toString());
      System.err.println(Throwables.getStackTraceAsString(e));
      System.exit(2);
    }
  }

  private static void outputIndexInfo(CompilationDescription indexInfo) throws IOException {
    String outputFile = System.getenv("KYTHE_OUTPUT_FILE");
    if (!Strings.isNullOrEmpty(outputFile)) {
      if (outputFile.endsWith(IndexInfoUtils.KZIP_FILE_EXT)) {
        IndexInfoUtils.writeKzipToFile(indexInfo, outputFile);
      } else {
        System.err.printf("Unsupported output file: %s%n", outputFile);
        System.exit(2);
      }
      return;
    }

    String outputDir = readEnvironmentVariable("KYTHE_OUTPUT_DIRECTORY");
    // Just rely on the underlying compilation unit's signature to get the filename, if we're not
    // writing to a single kzip file.
    String name =
        indexInfo
            .getCompilationUnit()
            .getVName()
            .getSignature()
            .trim()
            .replaceAll("^/+|/+$", "")
            .replace('/', '_');
    String path = IndexInfoUtils.getKzipPath(outputDir, name).toString();
    IndexInfoUtils.writeKzipToFile(indexInfo, path);
  }

  private static String[] getCleanedUpArguments(String[] args) throws IOException {
    // Expand all @file arguments
    List<String> expandedArgs = Lists.newArrayList(CommandLine.parse(args));

    // We skip some arguments that would normally be passed to javac:
    // -J, these are flags to the java environment running javac.
    // -XD, these are internal flags used for special debug information
    //          when compiling javac itself.
    // -Werror, we do not want to treat any warnings as errors.
    // -target, we do not care about the compiler outputs
    boolean skipArg = false;
    List<String> cleanedUpArgs = new ArrayList<>();
    for (String arg : expandedArgs) {
      if (arg.equals("-target")) {
        skipArg = true;
        continue;
      } else if (!(skipArg
          || arg.startsWith("-J")
          || arg.startsWith("-XD")
          || arg.startsWith("-Werror")
          // The errorprone plugin complicates the build due to certain other
          // flags it requires (such as -XDcompilePolicy=byfile) and is not
          // necessary for extraction.
          || arg.startsWith("-Xplugin:ErrorProne"))) {
        cleanedUpArgs.add(arg);
      }
      skipArg = false;
    }
    String[] cleanedUpArgsArray = new String[cleanedUpArgs.size()];
    return cleanedUpArgs.toArray(cleanedUpArgsArray);
  }

  private boolean passThroughIfAnalysisOnly(String[] args) throws Exception {
    // If '-proc:only' is passed as an argument, this is not a source file compilation, but an
    // analysis. We will let the real java compiler do its work.
    boolean hasProcOnly = false;
    for (String arg : args) {
      if (arg.equals("-proc:only")) {
        hasProcOnly = true;
        break;
      }
    }
    if (hasProcOnly) {
      passThrough(args);
      return true;
    }
    return false;
  }

  protected static String createTargetFromSourceFiles(List<String> sourceFiles) {
    List<String> sortedSourceFiles = Ordering.natural().sortedCopy(sourceFiles);
    String joinedSourceFiles = Joiner.on(":").join(sortedSourceFiles);
    return "#" + Hashing.sha256().hashUnencodedChars(joinedSourceFiles);
  }

  protected static List<String> splitPaths(String path) {
    return path == null ? Collections.<String>emptyList() : Splitter.on(':').splitToList(path);
  }

  protected static List<String> splitCSV(String lst) {
    return lst == null ? Collections.<String>emptyList() : Splitter.on(',').splitToList(lst);
  }

  static String readEnvironmentVariable(String variableName) {
    return readEnvironmentVariable(variableName, null);
  }

  static String readEnvironmentVariable(String variableName, String defaultValue) {
    // First see if we have a system property.
    String result = System.getProperty(variableName);
    if (Strings.isNullOrEmpty(result)) {
      // Fall back to the environment variable.
      result = System.getenv(variableName);
    }
    if (Strings.isNullOrEmpty(result)) {
      if (Strings.isNullOrEmpty(defaultValue)) {
        System.err.printf("Missing environment variable: %s%n", variableName);
        System.exit(1);
      }
      result = defaultValue;
    }
    return result;
  }

  protected static List<String> getSourceList(Collection<File> files) {
    List<String> sources = new ArrayList<>();
    for (File file : files) {
      sources.add(file.getPath());
    }
    return sources;
  }
}
