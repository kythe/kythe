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

import com.google.common.base.Throwables;
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

import java.io.IOException;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.util.Arrays;
import java.util.LinkedList;
import java.util.List;

/** Binary to run Kythe's Java index over a single .kindex file, emitting entries to STDOUT. */
public class JavaIndexer {
  public static void main(String[] rawArgs) throws AnalysisException, IOException {
    if (rawArgs.length > 0 && ("--help".equals(rawArgs[0]) || "-h".equals(rawArgs[0]))) {
      usage(0);
    } else if (rawArgs.length == 0) {
      usage(1);
    }

    List<String> args = new LinkedList(Arrays.asList(rawArgs));

    MemoryStatisticsCollector statistics = null;
    if ("--print_statistics".equals(args.get(0))) {
      args.remove(0);
      statistics = new MemoryStatisticsCollector();
    }

    CompilationDescription desc = null;
    switch (args.size()) {
      case 1:
        // java_indexer kindex-file
        desc = IndexInfoUtils.readIndexInfoFromFile(args.get(0));
        break;
      case 2: {
        // java_indexer --index_pack=archive-root unit-key
        if (!args.get(0).startsWith("--index_pack=") && !args.get(0).startsWith("-index_pack=")) {
          System.err.println("Expecting --index_pack as first argument; got " + args.get(0));
          usage(1);
        }
        String archiveRoot = args.get(0).substring(args.get(0).indexOf('=') + 1);
        desc = new Archive(archiveRoot).readDescription(args.get(1));
        break;
      }
      case 3:
        // java_indexer --index_pack archive-root unit-key
        if (!args.get(0).equals("--index_pack") && !args.get(0).equals("-index_pack")) {
          System.err.println("Expecting --index_pack as first argument; got " + args.get(0));
          usage(1);
        }
        desc = new Archive(args.get(1)).readDescription(args.get(2));
        break;
      default:
        System.err.println("Java indexer received too many arguments; got " + args);
        usage(1);
        break;
    }

    if (desc == null) {
      throw new IllegalStateException("Unknown error reading CompilationDescription");
    }

    try (OutputStream stream = System.out;
        OutputStreamWriter writer = new OutputStreamWriter(stream)) {
      new JavacAnalysisDriver()
          .analyze(new KytheJavacAnalyzer(new StreamFactEmitter(writer),
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
    System.err.println("usage: java_indexer [--print_statistics] kindex-file\n"
        + "       java_indexer [--print_statistics] --index_pack=archive-root unit-key");
    System.exit(exitCode);
  }

  /** {@link FactEmitter} directly streaming to an {@link OutputValueStream}. */
  private static class StreamFactEmitter implements FactEmitter {
    private final OutputStreamWriter writer;

    public StreamFactEmitter(OutputStreamWriter writer) {
      this.writer = writer;
    }

    @Override
    public void emit(VName source, String edgeKind, VName target,
        String factName, byte[] factValue) {
      Entry.Builder entry = Entry.newBuilder()
          .setSource(source)
          .setFactName(factName)
          .setFactValue(ByteString.copyFrom(factValue));
      if (edgeKind != null) {
        entry.setEdgeKind(edgeKind).setTarget(target);
      }

      try {
        entry.build().writeDelimitedTo(System.out);
      } catch (IOException ioe) {
        Throwables.propagate(ioe);
      }
    }
  }
}
