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

package com.google.devtools.kythe.analyzers.java;

import com.beust.jcommander.Parameter;
import com.google.common.base.Strings;
import com.google.devtools.kythe.analyzers.base.FactEmitter;
import com.google.devtools.kythe.extractors.shared.CompilationDescription;
import com.google.devtools.kythe.extractors.shared.IndexInfoUtils;
import com.google.devtools.kythe.platform.indexpack.Archive;
import com.google.devtools.kythe.platform.java.JavacAnalysisDriver;
import com.google.devtools.kythe.platform.shared.AnalysisException;
import com.google.devtools.kythe.platform.shared.FileDataCache;
import com.google.devtools.kythe.platform.shared.MemoryStatisticsCollector;
import com.google.devtools.kythe.platform.shared.NullStatisticsCollector;
import com.google.devtools.kythe.proto.Storage.Entry;
import com.google.devtools.kythe.proto.Storage.VName;
import com.google.protobuf.ByteString;
import java.io.BufferedOutputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.List;

/** Binary to run Kythe's Java index over a single .kindex file, emitting entries to STDOUT. */
public class JavaIndexer {
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
      // java_indexer --index_pack=archive-root unit-key
      desc = new Archive(config.getIndexPackRoot()).readDescription(compilation.get(0));
    } else {
      // java_indexer kindex-file
      desc = IndexInfoUtils.readIndexInfoFromFile(compilation.get(0));
    }

    if (desc == null) {
      throw new IllegalStateException("Unknown error reading CompilationDescription");
    }
    if (!desc.getFileContents().iterator().hasNext()) {
      return;
    }

    try (OutputStream stream =
        Strings.isNullOrEmpty(config.getOutputPath())
            ? System.out
            : new BufferedOutputStream(new FileOutputStream(config.getOutputPath()))) {
      new JavacAnalysisDriver()
          .analyze(
              new KytheJavacAnalyzer(
                  config,
                  new StreamFactEmitter(stream),
                  statistics == null ? NullStatisticsCollector.getInstance() : statistics),
              desc.getCompilationUnit(),
              new FileDataCache(desc.getFileContents()),
              false);
    }

    if (statistics != null) {
      statistics.printStatistics(System.err);
    }
  }

  private static void usage(int exitCode) {
    System.err.println(
        "usage: java_indexer [--print_statistics] kindex-file\n"
            + "       java_indexer [--print_statistics] --index_pack=archive-root unit-key");
    System.exit(exitCode);
  }

  /** {@link FactEmitter} directly streaming to an {@link OutputValueStream}. */
  private static class StreamFactEmitter implements FactEmitter {
    private final OutputStream stream;

    public StreamFactEmitter(OutputStream stream) {
      this.stream = stream;
    }

    @Override
    public void emit(
        VName source, String edgeKind, VName target, String factName, byte[] factValue) {
      Entry.Builder entry =
          Entry.newBuilder()
              .setSource(source)
              .setFactName(factName)
              .setFactValue(ByteString.copyFrom(factValue));
      if (edgeKind != null) {
        entry.setEdgeKind(edgeKind).setTarget(target);
      }

      try {
        entry.build().writeDelimitedTo(stream);
      } catch (IOException ioe) {
        throw new RuntimeException(ioe);
      }
    }
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
      super("java-indexer");
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
