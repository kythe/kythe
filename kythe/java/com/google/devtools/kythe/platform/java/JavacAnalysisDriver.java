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

package com.google.devtools.kythe.platform.java;

import com.google.common.base.Preconditions;
import com.google.common.collect.ImmutableList;
import com.google.devtools.kythe.common.FormattingLogger;
import com.google.devtools.kythe.platform.shared.AnalysisException;
import com.google.devtools.kythe.platform.shared.FileDataProvider;
import com.google.devtools.kythe.proto.Analysis.CompilationUnit;
import com.sun.tools.javac.api.JavacTool;
import java.io.File;
import java.util.List;
import javax.annotation.processing.Processor;
import javax.tools.JavaCompiler;

/**
 * Base implementation for the various analysis drivers. Allows running {@link JavacAnalyzer} over
 * compilations that are retrieved from various locations.
 */
public class JavacAnalysisDriver {
  private static final FormattingLogger logger =
      FormattingLogger.getLogger(JavacAnalysisDriver.class);
  private final List<Processor> processors;

  public JavacAnalysisDriver() {
    this(ImmutableList.<Processor>of());
  }

  public JavacAnalysisDriver(List<Processor> processors) {
    this.processors = processors;
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
      JavacAnalyzer analyzer,
      CompilationUnit compilationUnit,
      FileDataProvider fileDataProvider,
      boolean isLocalAnalysis)
      throws AnalysisException {
    checkEnvironment(compilationUnit);

    analyzer.analyzeCompilationUnit(
        JavaCompilationDetails.createDetails(
            compilationUnit, fileDataProvider, isLocalAnalysis, processors));
  }

  private static void checkEnvironment(CompilationUnit compilation) throws AnalysisException {
    Preconditions.checkArgument(
        !compilation.getSourceFileList().isEmpty(), "CompilationUnit has no source files");
    String sourcePath = compilation.getSourceFileList().get(0);
    if (new File(sourcePath).canRead()) {
      throw new AnalysisException(
          String.format(
              "can read source file: '%s'; "
                  + "indexer must be run outside of the compilation's source root "
                  + "(see https://kythe.io/phabricator/T70 for more details)",
              sourcePath));
    }
  }
}
