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

package com.google.devtools.kythe.platform.java;

import com.google.common.collect.ImmutableList;
import com.google.common.flogger.FluentLogger;
import com.google.devtools.kythe.platform.shared.AnalysisException;
import com.google.devtools.kythe.platform.shared.FileDataProvider;
import com.google.devtools.kythe.proto.Analysis.CompilationUnit;
import com.sun.tools.javac.api.JavacTool;
import java.nio.file.Path;
import java.util.List;
import javax.annotation.processing.Processor;
import javax.tools.JavaCompiler;
import org.checkerframework.checker.nullness.qual.Nullable;

/**
 * Base implementation for the various analysis drivers. Allows running {@link JavacAnalyzer} over
 * compilations that are retrieved from various locations.
 */
public class JavacAnalysisDriver {
  private static final FluentLogger logger = FluentLogger.forEnclosingClass();
  private final List<Processor> processors;
  private final boolean useExperimentalPathFileManager;
  @Nullable private final Path temporaryDirectory;

  public JavacAnalysisDriver() {
    this(ImmutableList.of(), false, null);
  }

  public JavacAnalysisDriver(
      List<Processor> processors,
      boolean useExperimentalPathFileManager,
      @Nullable Path temporaryDirectory) {
    this.processors = processors;
    this.useExperimentalPathFileManager = useExperimentalPathFileManager;
    this.temporaryDirectory = temporaryDirectory;
  }

  /**
   * Provides a {@link JavaCompiler} that this code is compiled with instead of the default behavior
   * of loading the {@link JavaCompiler} available at runtime. This is needed to run the Java 6
   * compiler on the Java 7 runtime.
   */
  public static JavaCompiler getCompiler() {
    return JavacTool.create();
  }

  public void analyze(
      JavacAnalyzer analyzer, CompilationUnit compilationUnit, FileDataProvider fileDataProvider)
      throws AnalysisException {

    // If there are no source files, then there is nothing for us to do.
    if (compilationUnit.getSourceFileList().isEmpty()) {
      logger.atWarning().log("CompilationUnit has no source files");
      return;
    }

    try (JavaCompilationDetails details =
        JavaCompilationDetails.createDetails(
            compilationUnit,
            fileDataProvider,
            processors,
            useExperimentalPathFileManager,
            temporaryDirectory)) {
      analyzer.analyzeCompilationUnit(details);
    }
  }
}
