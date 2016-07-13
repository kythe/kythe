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

package com.google.devtools.kythe.analyzers.base;

import com.google.common.base.Preconditions;
import com.google.common.base.Stopwatch;
import com.google.common.base.Strings;
import com.google.common.base.Throwables;
import com.google.devtools.kythe.common.FormattingLogger;
import com.google.devtools.kythe.platform.shared.AnalysisException;
import com.google.devtools.kythe.platform.shared.FileDataProvider;
import com.google.devtools.kythe.platform.shared.NullStatisticsCollector;
import com.google.devtools.kythe.platform.shared.StatisticsCollector;
import com.google.devtools.kythe.proto.Analysis.AnalysisOutput;
import com.google.devtools.kythe.proto.Analysis.AnalysisRequest;
import com.google.devtools.kythe.proto.Analysis.CompilationUnit;
import com.google.devtools.kythe.proto.Storage.Entry;
import com.google.devtools.kythe.proto.Storage.VName;
import com.google.protobuf.ByteString;

/** Abstract CompilationAnalyzer that handles common boilerplate code. */
public abstract class AbstractCompilationAnalyzer {
  private static final FormattingLogger logger =
      FormattingLogger.getLogger(AbstractCompilationAnalyzer.class);

  private final StatisticsCollector statistics;

  public AbstractCompilationAnalyzer() {
    this(NullStatisticsCollector.getInstance());
  }

  public AbstractCompilationAnalyzer(StatisticsCollector statistics) {
    Preconditions.checkNotNull(statistics, "StatisticsCollector must be non-null");
    this.statistics = statistics;
  }

  /**
   * Analyzes the given {@link AnalysisRequest}, emitting all facts with the given {@link
   * FactEmitter}.
   */
  public void analyzeRequest(AnalysisRequest req, FactEmitter emitter) throws AnalysisException {
    Preconditions.checkNotNull(req, "AnalysisRequest must be non-null");
    Stopwatch timer = Stopwatch.createStarted();
    try (FileDataProvider fileData = parseFileDataService(req.getFileDataService())) {
      analyzeCompilation(req.getCompilation(), fileData, emitter);
    } catch (Throwable t) {
      logger.warningfmt("Uncaught exception: %s", t);
      t.printStackTrace();
      Throwables.propagateIfInstanceOf(t, AnalysisException.class);
      throw new AnalysisException(t);
    } finally {
      logger.infofmt("Analysis completed in %s", timer.stop());
    }
  }

  /** Returns the {@link StatisticsCollector} to be used during analyses. */
  protected StatisticsCollector getStatisticsCollector() {
    return statistics;
  }

  /** Returns a {@link FileDataProvider} based on the given specification. */
  protected abstract FileDataProvider parseFileDataService(String fileDataService)
      throws AnalysisException;

  /**
   * Analyzes the given {@link CompilationUnit}. The given {@link FileDataProvider} and {@link
   * FactEmitter} should be used to get any necessary file data and emit any generated facts,
   * respectively, as a result of the compilation unit's processing. After returning, the given
   * {@link FileDataProvider} and {@link FactEmitter} should no longer be used.
   */
  protected abstract void analyzeCompilation(
      CompilationUnit compilationUnit, FileDataProvider fileDataProvider, FactEmitter emitter)
      throws AnalysisException;

  /**
   * {@link FactEmitter} that emits an {@link AnalysisOutput} with an embedded {@link Entry} for
   * each fact.
   */
  public abstract static class AnalysisOutputEmitter implements FactEmitter {
    @Override
    public void emit(
        VName source, String edgeKind, VName target, String factName, byte[] factValue) {
      Entry.Builder entry =
          Entry.newBuilder()
              .setSource(source)
              .setFactName(factName)
              .setFactValue(ByteString.copyFrom(factValue));
      if (!Strings.isNullOrEmpty(edgeKind)) {
        entry.setEdgeKind(edgeKind);
        entry.setTarget(target);
      }
      emitOutput(AnalysisOutput.newBuilder().setValue(entry.build().toByteString()).build());
    }

    /** Emits the given {@link AnalysisOutput}. */
    public abstract void emitOutput(AnalysisOutput output);
  }
}
