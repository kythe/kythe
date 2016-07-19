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

import static java.nio.charset.StandardCharsets.UTF_8;

import com.google.common.base.Preconditions;
import com.google.common.base.Predicate;
import com.google.common.collect.ImmutableList;
import com.google.common.collect.Iterables;
import com.google.common.collect.Lists;
import com.google.devtools.kythe.common.FormattingLogger;
import com.google.devtools.kythe.platform.java.filemanager.CompilationUnitBasedJavaFileManager;
import com.google.devtools.kythe.platform.shared.FileDataProvider;
import com.google.devtools.kythe.proto.Analysis.CompilationUnit;
import com.sun.source.tree.CompilationUnitTree;
import com.sun.source.util.JavacTask;
import com.sun.tools.javac.api.JavacTaskImpl;
import java.io.Writer;
import java.nio.charset.Charset;
import java.util.List;
import javax.annotation.processing.Processor;
import javax.tools.Diagnostic;
import javax.tools.Diagnostic.Kind;
import javax.tools.DiagnosticCollector;
import javax.tools.JavaCompiler;
import javax.tools.JavaFileObject;
import javax.tools.StandardJavaFileManager;

/** Provides a {@link JavacAnalyzer} with access to compilation information. */
public class JavaCompilationDetails {
  private final JavacTask javac;
  private final DiagnosticCollector<JavaFileObject> diagnostics;
  private final Iterable<? extends CompilationUnitTree> asts;
  private final CompilationUnit compilationUnit;
  private final Throwable analysisCrash;
  private final Charset encoding;

  private static final FormattingLogger logger =
      FormattingLogger.getLogger(JavaCompilationDetails.class);

  private static final Charset DEFAULT_ENCODING = UTF_8;

  private static final Predicate<Diagnostic<?>> ERROR_DIAGNOSTIC =
      new Predicate<Diagnostic<?>>() {
        @Override
        public boolean apply(Diagnostic<?> diag) {
          return diag.getKind() == Kind.ERROR;
        }
      };

  public static JavaCompilationDetails createDetails(
      CompilationUnit compilationUnit, FileDataProvider fileDataProvider, boolean useStdErr) {
    return createDetails(
        compilationUnit, fileDataProvider, false, ImmutableList.<Processor>of(), useStdErr);
  }

  public static JavaCompilationDetails createDetails(
      CompilationUnit compilationUnit,
      FileDataProvider fileDataProvider,
      boolean isLocalAnalysis,
      List<Processor> processors) {
    return createDetails(compilationUnit, fileDataProvider, isLocalAnalysis, processors, false);
  }

  public static JavaCompilationDetails createDetails(
      CompilationUnit compilationUnit,
      FileDataProvider fileDataProvider,
      boolean isLocalAnalysis,
      List<Processor> processors,
      boolean useStdErr) {

    JavaCompiler compiler = JavacAnalysisDriver.getCompiler();
    DiagnosticCollector<JavaFileObject> diagnosticsCollector = new DiagnosticCollector<>();

    // Get the compilation options
    List<String> options = optionsFromCompilationUnit(compilationUnit, processors, isLocalAnalysis);
    Charset encoding = JavacOptionsUtils.getEncodingOption(options);

    // Create a StandardFileManager that uses the fileDataProvider and compilationUnit
    StandardJavaFileManager fileManager =
        new CompilationUnitBasedJavaFileManager(
            fileDataProvider,
            compilationUnit,
            compiler.getStandardFileManager(diagnosticsCollector, null, null),
            encoding);

    Iterable<? extends JavaFileObject> sources =
        fileManager.getJavaFileObjectsFromStrings(compilationUnit.getSourceFileList());

    // If we use no writer, output will go to stdErr. The NullWriter is /dev/null.
    Writer javacOut = useStdErr ? null : NullWriter.getInstance();

    // Get a task for compiling the current CompilationUnit.
    JavacTaskImpl javacTask =
        (JavacTaskImpl)
            compiler.getTask(javacOut, fileManager, diagnosticsCollector, options, null, sources);

    if (!processors.isEmpty()) {
      javacTask.setProcessors(processors);
    }

    Throwable analysisCrash = null;
    Iterable<? extends CompilationUnitTree> compilationUnits = null;
    try {
      compilationUnits = javacTask.parse();
      javacTask.analyze();
    } catch (Throwable e) {
      logger.severefmt(e, "Unexpected error in javac analysis of {%s}", compilationUnit.getVName());
      analysisCrash = e;
    }

    return new JavaCompilationDetails(
        javacTask,
        diagnosticsCollector,
        compilationUnits,
        compilationUnit,
        analysisCrash,
        encoding);
  }

  private JavaCompilationDetails(
      JavacTask javac,
      DiagnosticCollector<JavaFileObject> diagnostics,
      Iterable<? extends CompilationUnitTree> asts,
      CompilationUnit compilationUnit,
      Throwable analysisCrash,
      Charset encoding) {
    this.javac = javac;
    this.diagnostics = diagnostics;
    this.asts = asts;
    this.compilationUnit = compilationUnit;
    this.analysisCrash = analysisCrash;
    this.encoding = Preconditions.checkNotNull(encoding);
  }

  public boolean inBadCompilationState() {
    return analysisCrash != null || asts == null;
  }

  public boolean hasCompileErrors() {
    return Iterables.any(diagnostics.getDiagnostics(), ERROR_DIAGNOSTIC);
  }

  public Iterable<Diagnostic<? extends JavaFileObject>> getCompileErrors() {
    return Iterables.filter(diagnostics.getDiagnostics(), ERROR_DIAGNOSTIC);
  }

  /** Returns the Javac compiler instance initialized for current analysis target. */
  public JavacTask getJavac() {
    return javac;
  }

  /** Returns the Diagnostics reported while analyzing the code for the current analysis target. */
  public List<Diagnostic<? extends JavaFileObject>> getDiagnostics() {
    return diagnostics.getDiagnostics();
  }

  /** Returns the DiagnosticsCollector for the current analysis target. */
  DiagnosticCollector<JavaFileObject> getDiagnosticsCollector() {
    return diagnostics;
  }

  /** Returns the AST for the current analysis target. */
  public Iterable<? extends CompilationUnitTree> getAsts() {
    return asts;
  }

  /** Returns the protocol buffer describing the current analysis target. */
  public CompilationUnit getCompilationUnit() {
    return compilationUnit;
  }

  /** Returns any unexpected crash that might have occurred during javac analysis. */
  public Throwable getAnalysisCrash() {
    return analysisCrash;
  }

  /** Returns he encoding for the source files in this compilation */
  public Charset getEncoding() {
    return encoding;
  }

  /**
   * Modify options so the compiler can find the classpath and sourcepath. As well as disable any
   * annotation processor.
   *
   * @param isLocalAnalysis when true we do not add jre jars to the classpath. Adding jre jars to
   *     classpath for local analysis done by {@link
   *     com.google.devtools.kythe.platform.java.local.LocalJavacAnalysisDriver} will cause the
   *     analysis to fail.
   */
  private static List<String> optionsFromCompilationUnit(
      CompilationUnit compilationUnit, List<Processor> processors, boolean isLocalAnalysis) {
    // Start with the default options, and then add in source
    // Turn on all warnings as well.
    List<String> options = Lists.newArrayList(compilationUnit.getArgumentList());
    options = JavacOptionsUtils.useAllWarnings(options);
    options = JavacOptionsUtils.ensureEncodingSet(options, DEFAULT_ENCODING);
    options = JavacOptionsUtils.removeUnsupportedOptions(options);

    if (!isLocalAnalysis) {
      JavacOptionsUtils.appendJREJarsToClasspath(options);
    }

    if (processors.isEmpty()) {
      options.add("-proc:none");
    }
    return ImmutableList.copyOf(options);
  }

  /** Writes nothing, used to reduce noise from the javac analysis output. */
  private static class NullWriter extends Writer {
    private static NullWriter instance = null;

    private NullWriter() {}

    public static NullWriter getInstance() {
      if (instance == null) {
        instance = new NullWriter();
      }
      return instance;
    }

    @Override
    public void flush() {}

    @Override
    public void close() {}

    @Override
    public void write(char cbuf[], int off, int len) {}
  }
}
