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
import com.google.common.base.Strings;
import com.google.devtools.kythe.platform.shared.AnalysisException;
import com.google.devtools.kythe.platform.shared.StatisticsCollector;
import com.google.devtools.kythe.proto.Analysis.CompilationUnit;
import com.sun.source.tree.CompilationUnitTree;
import java.io.Serializable;
import java.net.URI;

/**
 * Recreates a javac compiler instance from a CompilationUnit and allows derived classes to analyze
 * the instance and report results.
 */
public abstract class JavacAnalyzer implements Serializable {
  private static final long serialVersionUID = 8805455287690923133L;

  private final StatisticsCollector collector;

  public JavacAnalyzer(StatisticsCollector collector) {
    Preconditions.checkNotNull(collector);
    this.collector = collector;
  }

  protected StatisticsCollector getStatisticsCollector() {
    return collector;
  }

  /**
   * Overridden in derived classes to perform analysis on a java CompilationUnit.
   *
   * @param compilationDetails contains all information needed to perform java analysis.
   * @throws AnalysisException if analysis has a catastrophic failure.
   */
  public void analyzeCompilationUnit(JavaCompilationDetails compilationDetails)
      throws AnalysisException {
    try {
      for (CompilationUnitTree file : compilationDetails.getAsts()) {
        URI uri = file.getSourceFile().toUri();
        String fullPath = file.getSourceFile().toUri().getRawPath();
        if (!uri.getScheme().equals("file")) {
          fullPath = fullPath.substring(1);
        }
        String compilationUnitPath = fullPath;

        for (String sourceFile : compilationDetails.getCompilationUnit().getSourceFileList()) {
          if (fullPath.endsWith(sourceFile)) {
            compilationUnitPath = sourceFile;
            break;
          }
        }

        if (Strings.isNullOrEmpty(compilationUnitPath)) {
          continue;
        }

        analyzeFile(compilationDetails, file);
        getStatisticsCollector().incrementCounter("files-analyzed");
      }
      getStatisticsCollector().incrementCounter("compilations-analyzed");
    } catch (Throwable t) {
      getStatisticsCollector().incrementCounter("analyzer-exceptions");
      throw t;
    }
  }

  /**
   * Overridden in derived classes to perform analysis on a specific file of the CompilationUnit.
   *
   * @param compilationDetails contains all information needed to perform java analysis.
   * @throws AnalysisException if analysis has a catastrophic failure.
   */
  public void analyzeFile(JavaCompilationDetails compilationDetails, CompilationUnitTree file)
      throws AnalysisException {}
}
