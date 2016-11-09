/*
 * Copyright 2016 Google Inc. All rights reserved.
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

package com.google.devtools.kythe.analyzers.bazel;

import com.beust.jcommander.Parameter;
import com.google.common.base.Strings;
import com.google.devtools.kythe.analyzers.base.IndexerConfig;
import com.google.devtools.kythe.extractors.shared.CompilationDescription;
import com.google.devtools.kythe.extractors.shared.IndexInfoUtils;
import com.google.devtools.kythe.platform.indexpack.Archive;
import com.google.devtools.kythe.platform.shared.AnalysisException;
import com.google.devtools.kythe.platform.shared.MemoryStatisticsCollector;
import com.google.devtools.kythe.proto.Analysis.FileData;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

/**
 * Binary to run Kythe's Bazel index over a single .kindex file, emitting entries as Kythe nodes
 * and edges to STDOUT.
 */
public class BazelIndexer {
  public static void main(String[] args) throws AnalysisException, IOException {
    StandaloneConfig config = new StandaloneConfig();
    config.parseCommandLine(args);

    MemoryStatisticsCollector statistics = null;
    if (config.getPrintStatistics()) {
      statistics = new MemoryStatisticsCollector();
    }

    List<String> compilation = config.getCompilation();
    if (compilation.size() > 1) {
      System.err.println("Java indexer received too many arguments; got " + compilation);
      usage(1);
    }

    CompilationDescription desc = null;
    if (!Strings.isNullOrEmpty(config.getIndexPackRoot())) {
      // bazel_indexer --index_pack=archive-root unit-key
      desc = new Archive(config.getIndexPackRoot()).readDescription(compilation.get(0));
    } else {
      // bazel_indexer kindex-file
      desc = IndexInfoUtils.readIndexInfoFromFile(compilation.get(0));
    }

    if (desc == null) {
      throw new IllegalStateException("Unknown error reading CompilationDescription");
    }
    if (!desc.getFileContents().iterator().hasNext()) {
      return;
    }

    // print out the CU info and the file data list
    System.out.println("CU: " + desc.getCompilationUnit().getVName());
    for (FileData inputFile : desc.getFileContents()) {
      System.out.println("input file path: " + inputFile.getInfo().getPath());
    }

    // TODO: add analysis logic

    if (statistics != null) {
      statistics.printStatistics(System.err);
    }
  }

  /**
   * Prints the usage string for this program and exits with the specified code.
   *
   * @param exitCode The exit code to be used when exitting the program.
   */
  private static void usage(int exitCode) {
    System.err.println(
        "usage: bazel_indexer [--print_statistics] kindex-file\n"
            + "       java_indexer [--print_statistics] --index_pack=archive-root unit-key");
    System.exit(exitCode);
  }

  private static class StandaloneConfig extends IndexerConfig {
    @Parameter(description = "<compilation to analyze>", required = true)
    private List<String> compilation = new ArrayList<>();

    @Parameter(
      names = "--print_statistics",
      description = "Print final analyzer statistics to stderr"
    )
    private boolean printStatistics;

    @Parameter(
      names = {"--index_pack", "-index_pack"},
      description = "Retrieve the specified compilation from the given index pack"
    )
    private String indexPackRoot;

    @Parameter(
      names = {"--out", "-out"},
      description = "Write the entries to this file (or stdout if unspecified)"
    )
    private String outputPath;

    public StandaloneConfig() {
      super("bazel-indexer");
    }

    public final boolean getPrintStatistics() {
      return printStatistics;
    }

    public final String getIndexPackRoot() {
      return indexPackRoot;
    }

    public final String getOutputPath() {
      return outputPath;
    }

    public final List<String> getCompilation() {
      return compilation;
    }
  }
}
