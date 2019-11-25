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

import static java.nio.charset.StandardCharsets.UTF_8;

import com.google.common.base.Preconditions;
import com.google.common.base.Predicate;
import com.google.common.collect.ImmutableList;
import com.google.common.collect.Iterables;
import com.google.common.flogger.FluentLogger;
import com.google.devtools.kythe.platform.java.JavacOptionsUtils.ModifiableOptions;
import com.google.devtools.kythe.platform.java.filemanager.CompilationUnitBasedJavaFileManager;
import com.google.devtools.kythe.platform.java.filemanager.CompilationUnitPathFileManager;
import com.google.devtools.kythe.platform.shared.FileDataProvider;
import com.google.devtools.kythe.proto.Analysis.CompilationUnit;
import com.sun.source.tree.CompilationUnitTree;
import com.sun.source.util.JavacTask;
import com.sun.tools.javac.api.JavacTaskImpl;
import java.io.IOException;
import java.io.Writer;
import java.lang.reflect.Method;
import java.nio.charset.Charset;
import java.nio.file.Path;
import java.util.List;
import java.util.Optional;
import javax.annotation.processing.Processor;
import javax.tools.Diagnostic;
import javax.tools.Diagnostic.Kind;
import javax.tools.DiagnosticCollector;
import javax.tools.JavaCompiler;
import javax.tools.JavaFileObject;
import javax.tools.StandardJavaFileManager;
import org.checkerframework.checker.nullness.qual.Nullable;

/** Provides a {@link JavacAnalyzer} with access to compilation information. */
public class JavaCompilationDetails implements AutoCloseable {
  private final JavacTask javac;
  private final DiagnosticCollector<JavaFileObject> diagnostics;
  @Nullable private final Iterable<? extends CompilationUnitTree> asts;
  private final CompilationUnit compilationUnit;
  @Nullable private final Throwable analysisCrash;
  private final Charset encoding;
  private final StandardJavaFileManager fileManager;

  private static final FluentLogger logger = FluentLogger.forEnclosingClass();

  private static final Charset DEFAULT_ENCODING = UTF_8;

  private static final Predicate<Diagnostic<?>> ERROR_DIAGNOSTIC =
      diag -> diag.getKind() == Kind.ERROR;

  /**
   * Create a compilation details object. {@code temporaryDirectory} specifies a directory that can
   * be used to store files that java must read from a local file system (ex: system module files).
   */
  public static JavaCompilationDetails createDetails(
      CompilationUnit compilationUnit,
      FileDataProvider fileDataProvider,
      List<Processor> processors,
      boolean useExperimentalPathFileManager,
      @Nullable Path temporaryDirectory) {

    JavaCompiler compiler = JavacAnalysisDriver.getCompiler();
    DiagnosticCollector<JavaFileObject> diagnosticsCollector = new DiagnosticCollector<>();

    // Get the compilation options
    List<String> options = optionsFromCompilationUnit(compilationUnit, processors);
    Charset encoding = JavacOptionsUtils.getEncodingOption(options);

    // Create a CompilationUnitBasedJavaFileManager that uses the fileDataProvider and
    // compilationUnit
    StandardJavaFileManager fileManager =
        // The Path-based JavaFileManager is only compatible with JDK9+ and for now,
        // we have to remain compatible with JDK8.
        useExperimentalPathFileManager && isJdk9OrNewer()
            ? new CompilationUnitPathFileManager(
                compilationUnit,
                fileDataProvider,
                compiler.getStandardFileManager(diagnosticsCollector, null, null),
                temporaryDirectory)
            : new CompilationUnitBasedJavaFileManager(
                fileDataProvider,
                compilationUnit,
                compiler.getStandardFileManager(diagnosticsCollector, null, null),
                encoding);

    Iterable<? extends JavaFileObject> sources =
        fileManager.getJavaFileObjectsFromStrings(compilationUnit.getSourceFileList());

    // Causes output to go to stdErr
    Writer javacOut = null;

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
      logger.atSevere().withCause(e).log(
          "Unexpected error in javac analysis of {%s}", compilationUnit.getVName());
      analysisCrash = e;
    }

    return new JavaCompilationDetails(
        javacTask,
        diagnosticsCollector,
        compilationUnits,
        compilationUnit,
        analysisCrash,
        encoding,
        fileManager);
  }

  private JavaCompilationDetails(
      JavacTask javac,
      DiagnosticCollector<JavaFileObject> diagnostics,
      @Nullable Iterable<? extends CompilationUnitTree> asts,
      CompilationUnit compilationUnit,
      @Nullable Throwable analysisCrash,
      Charset encoding,
      StandardJavaFileManager fileManager) {
    this.javac = javac;
    this.diagnostics = diagnostics;
    this.asts = asts;
    this.compilationUnit = compilationUnit;
    this.analysisCrash = analysisCrash;
    this.encoding = Preconditions.checkNotNull(encoding);
    this.fileManager = fileManager;
  }

  public boolean inBadCompilationState() {
    return analysisCrash != null || asts == null;
  }

  public boolean hasCompileErrors() {
    return diagnostics.getDiagnostics().stream().anyMatch(ERROR_DIAGNOSTIC);
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
    return asts == null ? ImmutableList.of() : asts;
  }

  /** Returns the protocol buffer describing the current analysis target. */
  public CompilationUnit getCompilationUnit() {
    return compilationUnit;
  }

  /** Returns any unexpected crash that might have occurred during javac analysis. */
  public Optional<Throwable> getAnalysisCrash() {
    return Optional.ofNullable(analysisCrash);
  }

  /** Returns he encoding for the source files in this compilation */
  public Charset getEncoding() {
    return encoding;
  }

  /** @return The file manager for the source files in this compilation */
  public StandardJavaFileManager getFileManager() {
    return fileManager;
  }

  @Override
  public void close() {
    try {
      fileManager.close();
    } catch (IOException e) {
      logger.atWarning().withCause(e).log("Unable to clean up file manager resources");
    }
  }

  /** Generate options (such as classpath and sourcepath) from the compilation unit. */
  private static ImmutableList<String> optionsFromCompilationUnit(
      CompilationUnit compilationUnit, List<Processor> processors) {
    // Start with the default options, and then add in source
    ModifiableOptions arguments =
        ModifiableOptions.of(compilationUnit.getArgumentList())
            .ensureEncodingSet(DEFAULT_ENCODING)
            .updateWithJavaOptions(compilationUnit);

    if (processors.isEmpty()) {
      arguments.add("-proc:none");
    }

    return arguments.removeUnsupportedOptions().build();
  }

  /** Returns true if the runtime version is >= JRE 9 */
  private static boolean isJdk9OrNewer() {
    try {
      Method versionMethod = Runtime.class.getMethod("version");
      Object version = versionMethod.invoke(null);
      return ((int) version.getClass().getMethod("major").invoke(version) >= 9);
    } catch (ReflectiveOperationException e) {
      logger.atInfo().log("Falling back to legacy FileManager on JDK8 or older");
      return false;
    }
  }
}
