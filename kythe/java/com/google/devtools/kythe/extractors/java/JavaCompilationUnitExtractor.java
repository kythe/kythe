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

package com.google.devtools.kythe.extractors.java;

import static com.google.common.base.StandardSystemProperty.JAVA_HOME;
import static com.google.common.collect.ImmutableList.toImmutableList;
import static java.nio.file.LinkOption.NOFOLLOW_LINKS;

import com.google.common.base.Joiner;
import com.google.common.base.Preconditions;
import com.google.common.base.Strings;
import com.google.common.collect.HashMultimap;
import com.google.common.collect.ImmutableList;
import com.google.common.collect.ImmutableMap;
import com.google.common.collect.ImmutableSet;
import com.google.common.collect.Iterables;
import com.google.common.collect.SetMultimap;
import com.google.common.collect.Streams;
import com.google.common.flogger.FluentLogger;
import com.google.common.io.ByteStreams;
import com.google.devtools.kythe.extractors.shared.CompilationDescription;
import com.google.devtools.kythe.extractors.shared.ExtractionException;
import com.google.devtools.kythe.extractors.shared.ExtractorUtils;
import com.google.devtools.kythe.extractors.shared.FileVNames;
import com.google.devtools.kythe.platform.java.JavacOptionsUtils.ModifiableOptions;
import com.google.devtools.kythe.proto.Analysis.CompilationUnit;
import com.google.devtools.kythe.proto.Analysis.CompilationUnit.FileInput;
import com.google.devtools.kythe.proto.Analysis.FileData;
import com.google.devtools.kythe.proto.Buildinfo.BuildDetails;
import com.google.devtools.kythe.proto.Java.JavaDetails;
import com.google.devtools.kythe.proto.Storage.VName;
import com.google.devtools.kythe.util.DeleteRecursively;
import com.google.errorprone.annotations.CanIgnoreReturnValue;
import com.google.protobuf.Any;
import com.sun.source.tree.CompilationUnitTree;
import com.sun.source.tree.ExpressionTree;
import com.sun.source.tree.ImportTree;
import com.sun.source.util.JavacTask;
import com.sun.source.util.TaskEvent;
import com.sun.source.util.TaskListener;
import com.sun.tools.javac.api.JavacTaskImpl;
import com.sun.tools.javac.code.Symbol.ClassSymbol;
import com.sun.tools.javac.code.Symtab;
import com.sun.tools.javac.file.CacheFSInfo;
import com.sun.tools.javac.file.FSInfo;
import com.sun.tools.javac.file.JavacFileManager;
import com.sun.tools.javac.main.Option;
import com.sun.tools.javac.util.Context;
import java.io.File;
import java.io.IOError;
import java.io.IOException;
import java.io.InputStream;
import java.net.JarURLConnection;
import java.net.MalformedURLException;
import java.net.URI;
import java.net.URL;
import java.net.URLClassLoader;
import java.net.URLConnection;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.EnumSet;
import java.util.HashMap;
import java.util.HashSet;
import java.util.LinkedHashMap;
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.Optional;
import java.util.ServiceLoader;
import java.util.Set;
import javax.annotation.processing.Processor;
import javax.tools.Diagnostic;
import javax.tools.DiagnosticCollector;
import javax.tools.JavaCompiler;
import javax.tools.JavaFileManager.Location;
import javax.tools.JavaFileObject;
import javax.tools.JavaFileObject.Kind;
import javax.tools.StandardJavaFileManager;
import javax.tools.StandardLocation;
import javax.tools.ToolProvider;
import org.checkerframework.checker.nullness.qual.Nullable;

/**
 * Extracts all required information (set of source files, class paths, and compiler options) from a
 * java compilation command and stores the information to replay the compilation.
 *
 * <p>The extractor runs the Javac compiler to get a exact description of all files required for the
 * compilation as a whole. It then creates CompilationUnit entries for each source file. We do not
 * do this on a per file basis as the Java Compiler takes too long to do this.
 */
public class JavaCompilationUnitExtractor {
  public static final String JAVA_DETAILS_URL = "kythe.io/proto/kythe.proto.JavaDetails";
  public static final String BUILD_DETAILS_URL = "kythe.io/proto/kythe.proto.BuildDetails";

  private static final FluentLogger logger = FluentLogger.forEnclosingClass();

  private static final String MODULE_INFO_NAME = "module-info";
  private static final String SOURCE_JAR_ROOT = "!SOURCE_JAR!";

  private static String classJarRoot(Location location) {
    return String.format("!%s_JAR!", location);
  }

  private static final ClassLoader moduleClassLoader = findModuleClassLoader();

  private static final String JAR_SCHEME = "jar";
  private final String jdkJar;
  private final String rootDirectory;
  private final FileVNames fileVNames;
  private String systemDir;
  private boolean allowServiceProcessors = true;

  /** Set whether to allow service processors to run during extraction. Defaults to {@code true}. */
  @CanIgnoreReturnValue
  public JavaCompilationUnitExtractor setAllowServiceProcessors(boolean allowServiceProcessors) {
    this.allowServiceProcessors = allowServiceProcessors;
    return this;
  }

  /**
   * ExtractionTask represents a single invocation of the java compiler in order to determine the
   * required inputs.
   */
  public static class ExtractionTask implements AutoCloseable {
    private final TemporaryDirectory tempDir;

    // Can only be intiailized once the task is created.
    private Symtab symbolTable;

    private final JavaCompiler compiler = getJavaCompiler();
    private final DiagnosticCollector<JavaFileObject> diagnosticCollector =
        new DiagnosticCollector<>();
    private final CompilationUnitCollector compilationCollector = new CompilationUnitCollector();
    private final UsageAsInputReportingFileManager fileManager =
        JavaCompilationUnitExtractor.getFileManager(compiler, diagnosticCollector);
    private final boolean allowServiceProcessors;

    ExtractionTask(boolean allowServiceProcessors) throws ExtractionException {
      this.tempDir = new TemporaryDirectory();
      this.allowServiceProcessors = allowServiceProcessors;
    }

    public UsageAsInputReportingFileManager getFileManager() {
      return fileManager;
    }

    public Collection<Diagnostic<? extends JavaFileObject>> getDiagnostics() {
      return diagnosticCollector.getDiagnostics();
    }

    public Collection<CompilationUnitTree> getCompilations() {
      return compilationCollector.getCompilations();
    }

    public Symtab getSymbolTable() {
      return symbolTable;
    }

    public boolean compileResolved(
        Iterable<String> options,
        Iterable<? extends JavaFileObject> sourceFiles,
        Iterable<Processor> processors)
        throws ExtractionException {
      Preconditions.checkNotNull(options);
      Preconditions.checkNotNull(sourceFiles);
      Preconditions.checkNotNull(processors);
      // Launch the java compiler with our modified settings and the filemanager wrapper
      JavacTask javacTask =
          (JavacTask)
              compiler.getTask(
                  null,
                  fileManager,
                  diagnosticCollector,
                  completeCompilerOptions(options, tempDir.getPath()),
                  null,
                  sourceFiles);
      javacTask.addTaskListener(compilationCollector);
      javacTask.setProcessors(
          Iterables.concat(processors, ImmutableList.of(new ProcessAnnotation(fileManager))));

      symbolTable = JavaCompilationUnitExtractor.getSymbolTable(javacTask);
      boolean result;
      try {
        // In order for the compiler to load all required .java & .class files we need to have it go
        // through parsing, analysis & generate phases.  Unfortunately the latter is needed to get a
        // complete list, this was found as we were breaking on analyzing certain files.
        // JavacTask#call() subsumes parse() and generate(), but calling those methods directly may
        // silently ignore fatal errors.
        result = javacTask.call();
      } catch (com.sun.tools.javac.util.Abort e) {
        logFatalErrors(getDiagnostics());
        throw new ExtractionException("Fatal error while running javac compiler.", e, false);
      }
      return !logErrors(getDiagnostics()) && result;
    }

    public boolean compile(
        Iterable<String> options, Iterable<String> sourcePaths, Iterable<String> processors)
        throws ExtractionException {
      return compileResolved(
          options,
          fileManager.getJavaFileObjectsFromStrings(sourcePaths),
          loadProcessors(processingClassLoader(fileManager), processors, allowServiceProcessors));
    }

    @Override
    public void close() {
      tempDir.close();
      try {
        fileManager.close();
      } catch (IOException err) {
        logger.atWarning().withCause(err).log("Unable to close file manager");
      }
    }
  }

  /**
   * Creates an instance of the JavaExtractor to store java compilation information in an .kzip
   * file.
   */
  public JavaCompilationUnitExtractor(String corpus) throws ExtractionException {
    this(corpus, ExtractorUtils.getCurrentWorkingDirectory());
  }

  /**
   * Creates an instance of the JavaExtractor to store java compilation information in an .kzip
   * file.
   */
  public JavaCompilationUnitExtractor(String corpus, String rootDirectory)
      throws ExtractionException {
    this(FileVNames.staticCorpus(corpus), rootDirectory);
  }

  /**
   * Creates an instance of the JavaExtractor to store java compilation information in an .kzip
   * file.
   */
  public JavaCompilationUnitExtractor(FileVNames fileVNames) throws ExtractionException {
    this(fileVNames, ExtractorUtils.getCurrentWorkingDirectory());
  }

  public JavaCompilationUnitExtractor(FileVNames fileVNames, String rootDirectory)
      throws ExtractionException {
    this.fileVNames = fileVNames;

    Path javaHome = Paths.get(JAVA_HOME.value()).getParent();
    try {
      // Remove trailing dots.  Interesting trivia: in some build systems,
      // the java.home variable is terminated with "/bin/..".
      // However, this is not the case for the class files
      // that we are trying to filter.
      this.jdkJar = javaHome.toRealPath(NOFOLLOW_LINKS).toString();
    } catch (IOException e) {
      throw new ExtractionException("JDK path not found: " + javaHome, e, false);
    }

    try {
      this.rootDirectory = Paths.get(rootDirectory).toRealPath().toString();
    } catch (IOException ioe) {
      throw new ExtractionException("Root directory does not exist", ioe, false);
    }
  }

  /**
   * Use this value for the -system javac option. {@code systemDir} should either be a path to the
   * system module directory or "none" which is a special value to javac.
   */
  public void useSystemDirectory(String systemDir) {
    this.systemDir = systemDir;
  }

  private CompilationUnit buildCompilationUnit(
      String target,
      Iterable<String> options,
      Iterable<FileInput> requiredInputs,
      boolean hasErrors,
      Set<String> newSourcePath,
      Set<String> newClassPath,
      Iterable<String> newBootClassPath,
      List<String> sourceFiles,
      String outputPath) {
    CompilationUnit.Builder unit = CompilationUnit.newBuilder();
    unit.getVNameBuilder().setSignature(target).setLanguage("java");
    unit.addAllArgument(options);
    unit.setHasCompileErrors(hasErrors);
    unit.addAllRequiredInput(requiredInputs);
    ImmutableMap<String, String> inputCorpus =
        Streams.stream(requiredInputs)
            .collect(
                ImmutableMap.toImmutableMap(
                    f -> f.getInfo().getPath(), f -> f.getVName().getCorpus()));
    Set<String> sourceFileCorpora = new HashSet<>();
    for (String sourceFile : sourceFiles) {
      unit.addSourceFile(sourceFile);
      sourceFileCorpora.add(inputCorpus.getOrDefault(sourceFile, ""));
    }
    // Attribute the source files' corpus to the CompilationUnit if it is unambiguous. Otherwise use
    // the default corpus.
    String cuCorpus =
        sourceFileCorpora.size() == 1
            ? Iterables.getOnlyElement(sourceFileCorpora)
            : fileVNames.getDefaultCorpus();
    unit.getVNameBuilder().setCorpus(cuCorpus);
    unit.setOutputKey(outputPath);
    unit.setWorkingDirectory(stableRoot(rootDirectory, options, requiredInputs));
    unit.addDetails(
        Any.newBuilder()
            .setTypeUrl(JAVA_DETAILS_URL)
            .setValue(
                JavaDetails.newBuilder()
                    .addAllClasspath(newClassPath)
                    .addAllBootclasspath(newBootClassPath)
                    .addAllSourcepath(newSourcePath)
                    .build()
                    .toByteString()));
    unit.addDetails(
        Any.newBuilder()
            .setTypeUrl(BUILD_DETAILS_URL)
            .setValue(BuildDetails.newBuilder().setBuildTarget(target).build().toByteString()));
    return unit.build();
  }

  /**
   * Indexes a compilation unit to the bigtable. The extraction process will try to build a minimum
   * set of what is needed to replay the compilation. To do this it runs the java compiler, and
   * tracks all .class & .java files that are needed. It then builds up a new classpath & sourcepath
   * that only contains the minimum set of paths required to replay the compilation.
   *
   * <p>New classpath: because we extract classes from the jars into a temp path that needs to be
   * set. Also we only use the classpaths that are actually used, not the ones that are provided.
   * New sourcepath: as we're doing a partial compilation, we need to set up the source path to
   * correctly load any files that are not the main compilation but are needed to perform
   * compilation. This is not required when doing a full compilation as all sources are
   * automatically loaded.
   *
   * <p>Next we store all required files in the bigtable and writes the CompilationUnit to the
   * bigtable.
   *
   * @throws ExtractionException if anything blocks the indexing to be completed.
   */
  public CompilationDescription extract(
      String target,
      Iterable<String> sources,
      Iterable<String> classpath,
      Iterable<String> bootclasspath,
      Iterable<String> sourcepath,
      Iterable<String> processorpath,
      Iterable<String> processors,
      Iterable<String> options,
      String outputPath)
      throws ExtractionException {
    Preconditions.checkNotNull(target);
    Preconditions.checkNotNull(sources);
    Preconditions.checkNotNull(classpath);
    Preconditions.checkNotNull(bootclasspath);
    Preconditions.checkNotNull(sourcepath);
    Preconditions.checkNotNull(processorpath);
    Preconditions.checkNotNull(processors);
    Preconditions.checkNotNull(options);
    Preconditions.checkNotNull(outputPath);
    return extract(
        target,
        sources,
        ImmutableMap.of(
            StandardLocation.CLASS_PATH, classpath,
            StandardLocation.PLATFORM_CLASS_PATH, bootclasspath,
            StandardLocation.SOURCE_PATH, sourcepath,
            StandardLocation.ANNOTATION_PROCESSOR_PATH, processorpath),
        processors,
        options,
        outputPath);
  }

  /** Extracts required compilation files and arguments into a CompilationDescription. */
  public CompilationDescription extract(
      String target,
      Iterable<String> sources,
      Map<Location, Iterable<String>> searchPaths,
      Iterable<String> processors,
      Iterable<String> options,
      String outputPath)
      throws ExtractionException {
    Preconditions.checkNotNull(target);
    Preconditions.checkNotNull(sources);
    Preconditions.checkNotNull(searchPaths);
    Preconditions.checkNotNull(processors);
    Preconditions.checkNotNull(options);
    Preconditions.checkNotNull(outputPath);

    final AnalysisResults results =
        Iterables.isEmpty(sources)
            ? new AnalysisResults()
            : runJavaAnalysisToExtractCompilationDetails(sources, searchPaths, processors, options);

    List<FileData> fileContents = ExtractorUtils.convertBytesToFileDatas(results.fileContents);
    ImmutableList<FileInput> compilationFileInputs =
        ExtractorUtils.toFileInputs(fileVNames, results.relativePaths::get, fileContents).stream()
            .map(
                input -> {
                  String sourceBasename = results.sourceFileNames.get(input.getInfo().getPath());
                  VName vname = input.getVName();
                  if (sourceBasename != null
                      && vname.getPath().endsWith(".java")
                      && !vname.getPath().endsWith(sourceBasename)) {
                    Path fixedPath = Paths.get(vname.getPath()).resolveSibling(sourceBasename);
                    vname = vname.toBuilder().setPath(fixedPath.toString()).build();
                    return input.toBuilder().setVName(vname).build();
                  }
                  return input;
                })
            .collect(toImmutableList());

    final CompilationUnit compilationUnit =
        buildCompilationUnit(
            target,
            removeDestDirOptions(options),
            compilationFileInputs,
            results.hasErrors,
            results.newSourcePath,
            results.newClassPath,
            results.newBootClassPath,
            results.explicitSources,
            ExtractorUtils.tryMakeRelative(rootDirectory, outputPath));
    return new CompilationDescription(compilationUnit, fileContents);
  }

  /**
   * Returns a new list with the same options except header/source destination directory options.
   */
  private static ImmutableList<String> removeDestDirOptions(Iterable<String> options) {
    return ModifiableOptions.of(options)
        .removeOptions(EnumSet.of(Option.D, Option.S, Option.H))
        .build();
  }

  /**
   * If the code has wildcard imports (e.g. import foo.bar.*) but doesn't actually use any of the
   * imports, errors will happen. We don't get callbacks for file open of these files (since they
   * aren't used) but when java runs it will report errors if it can't find any files to match the
   * wildcard. So we add one matching file here.
   */
  private void findOnDemandImportedFiles(
      UsageAsInputReportingFileManager fileManager,
      Iterable<? extends CompilationUnitTree> compilationUnits)
      throws ExtractionException {
    // Maps package names to source files that wildcard import them.
    SetMultimap<String, String> pkgs = HashMultimap.create();

    // Javac synthesizes an "import java.lang.*" for every compilation unit.
    pkgs.put("java.lang", "*.java");

    for (CompilationUnitTree unit : compilationUnits) {
      for (ImportTree importTree : unit.getImports()) {
        if (importTree.isStatic()) {
          continue;
        }
        String qualifiedIdentifier = importTree.getQualifiedIdentifier().toString();
        if (!qualifiedIdentifier.endsWith(".*")) {
          continue;
        }
        pkgs.put(
            qualifiedIdentifier.substring(0, qualifiedIdentifier.length() - 2),
            unit.getSourceFile().getName());
      }
    }

    for (Map.Entry<String, Collection<String>> pkg : pkgs.asMap().entrySet()) {
      try {
        JavaFileObject firstClass =
            Iterables.getFirst(
                fileManager.list(
                    StandardLocation.CLASS_PATH, pkg.getKey(), EnumSet.of(Kind.CLASS), false),
                null);
        if (firstClass == null) {
          firstClass =
              Iterables.getFirst(
                  fileManager.list(
                      StandardLocation.PLATFORM_CLASS_PATH,
                      pkg.getKey(),
                      EnumSet.of(Kind.CLASS),
                      false),
                  null);
        }
        if (firstClass != null) {
          firstClass.getCharContent(true);
        }
        JavaFileObject firstSource =
            Iterables.getFirst(
                fileManager.list(
                    StandardLocation.SOURCE_PATH, pkg.getKey(), EnumSet.of(Kind.SOURCE), false),
                null);
        if (firstSource != null) {
          firstSource.getCharContent(true);
        }
      } catch (IOException e) {
        throw new ExtractionException(
            String.format(
                "Unable to extract files used for on demand imports in {%s}",
                Joiner.on(", ").join(pkg.getValue())),
            e,
            false);
      }
    }
  }

  /**
   * Determines -sourcepath arguments to add to the compilation unit based on the package name. This
   * is needed as the sharded analysis will need to resolve dependent source files. Also locates
   * sources that do not follow the package == path convention and list them as explicit sources.
   */
  private ImmutableList<String> getAdditionalSourcePaths(
      Iterable<? extends CompilationUnitTree> compilationUnits) {
    ImmutableList.Builder<String> results = new ImmutableList.Builder<>();
    for (CompilationUnitTree compilationUnit : compilationUnits) {
      ExpressionTree packageExpression = compilationUnit.getPackageName();
      if (packageExpression != null) {
        String packageName = packageExpression.toString();
        if (!Strings.isNullOrEmpty(packageName)) {
          // If the source file specifies a package, we try to find that
          // package name in the path to the source file and assume
          // the correct sourcepath to add is the directory containing that
          // package.
          String packageSubDir = packageName.replace('.', '/');
          URI uri = compilationUnit.getSourceFile().toUri();
          // If the user included source files in their jars, we don't record anything special here
          // because we pick up this source jar and the classpath/sourcepath usage with our other
          // compiler hooks.
          if ("jar".equals(uri.getScheme())) {
            logger.atWarning().log(
                "Detected a source in a jar file: %s - %s", uri, compilationUnit);
            continue;
          }
          String path = uri.getPath();
          // This needs to be lastIndexOf as there are source jars that
          // contain the same package name in the files contained in them
          // as the path the source jars live in. As we extract the source
          // jars, we end up with a double named path.
          int index = path.lastIndexOf(packageSubDir);
          if (index >= 0) {
            results.add(ExtractorUtils.tryMakeRelative(rootDirectory, path.substring(0, index)));
          }
        }
      }
    }
    return results.build();
  }

  // Checks for ".pb.meta" files for each compilation unit and marks it as required if present.
  private static void findMetadataFiles(
      UsageAsInputReportingFileManager fileManager,
      Iterable<CompilationUnitTree> compilationUnits) {
    for (CompilationUnitTree compilationUnit : compilationUnits) {
      try {
        String annotationPath = compilationUnit.getSourceFile().toUri().getPath() + ".pb.meta";
        if (Files.exists(Paths.get(annotationPath))) {
          for (JavaFileObject file : fileManager.getJavaFileObjects(annotationPath)) {
            ((UsageAsInputReportingJavaFileObject) file).markUsed();
          }
        }
      } catch (IllegalArgumentException ex) {
        // Invalid path.
      }
    }
  }

  private void findRequiredFiles(
      UsageAsInputReportingFileManager fileManager,
      Map<URI, String> sourceFiles,
      AnalysisResults results)
      throws ExtractionException {
    Set<String> genSrcDirs =
        fileManager.hasLocation(StandardLocation.SOURCE_OUTPUT)
            ? ImmutableSet.copyOf(
                Iterables.transform(
                    fileManager.getLocation(StandardLocation.SOURCE_OUTPUT),
                    f -> ExtractorUtils.tryMakeRelative(rootDirectory, f.toString())))
            : ImmutableSet.of();
    for (InputUsageRecord input : fileManager.getUsages()) {
      processRequiredInput(
          input.fileObject(),
          input.location().orElse(null),
          fileManager,
          sourceFiles,
          genSrcDirs,
          results);
    }
  }

  private void processRequiredInput(
      JavaFileObject requiredInput,
      Location location,
      UsageAsInputReportingFileManager fileManager,
      Map<URI, String> sourceFiles,
      Set<String> genSrcDirs,
      AnalysisResults results)
      throws ExtractionException {
    URI uri = requiredInput.toUri();
    String entryPath;
    String jarPath = null;
    boolean isJarPath = false;

    {
      URLConnection conn;
      URL url;
      try {
        url = uri.toURL();
        conn = url.openConnection();
      } catch (IOException e) {
        throw new IOError(e);
      }
      if (conn instanceof JarURLConnection) {
        isJarPath = true;
        JarURLConnection jarConn = ((JarURLConnection) conn);
        jarPath = jarConn.getJarFileURL().getFile();
        // jar entries don't have a leading '/', and we expect
        // paths like "!CLASS_PATH_JAR!/com/foo/Bar.class"
        entryPath = "/" + jarConn.getEntryName();
      } else {
        entryPath = url.getFile();
      }
    }

    if (uri.getScheme().equals(JAR_SCHEME)) {
      isJarPath = true;
      uri = URI.create(uri.getRawSchemeSpecificPart());
    }

    switch (requiredInput.getKind()) {
      case CLASS:
      case SOURCE:
        break;
      case OTHER:
        if (uri.getPath().endsWith(".meta")) {
          break;
        }
        throw new IllegalStateException(String.format("Unsupported OTHER file kind: '%s'", uri));
      default:
        throw new IllegalStateException(
            String.format(
                "Unsupported java file kind: '%s' for '%s'", requiredInput.getKind().name(), uri));
    }
    String path = uri.getRawSchemeSpecificPart();

    // If the file was part of the JDK we do not store it as the JDK is tied
    // to the analyzer we'll run on this information later on.
    if ((isJarPath && jarPath.startsWith(jdkJar))
        || (location != null && location.getName().startsWith("SYSTEM_MODULES"))) {
      return;
    }

    // Make the path relative to the indexer (e.g. a subdir of corpus/).
    // If not possible, we store the fullpath.
    String relativePath = ExtractorUtils.tryMakeRelative(rootDirectory, path);

    String strippedPath = relativePath;
    if (isJarPath) {
      // If the file came from a jar file, we strip that out as we don't care where it came from, it
      // was not as a source file in source control.  We turn it in to a fake path.  This is done so
      // we do not need to download the entire jar file for each file's compilation (e.g. instead of
      // downloading 200MB we only download 60K for analyzing the Kythe java indexer).
      switch (requiredInput.getKind()) {
        case CLASS:
          String root = classJarRoot(location);
          strippedPath = root + entryPath;
          (location == StandardLocation.PLATFORM_CLASS_PATH
                  ? results.newBootClassPath
                  : results.newClassPath)
              .add(root);
          break;
        case SOURCE:
          results.newSourcePath.add(SOURCE_JAR_ROOT);
          strippedPath = SOURCE_JAR_ROOT + entryPath;
          break;
        default:
          // TODO(#1845): we shouldn't need to throw an exception here because the above switch
          // statement means that we never hit this default, but the static analysis tools can't
          // figure that out.  Try to refactor this code to remove this issue.
          throw new IllegalStateException(
              String.format(
                  "Unsupported java file kind: '%s' for '%s'",
                  requiredInput.getKind().name(), uri));
      }
    } else {
      // If the class file was on disk, we need to infer the correct classpath to add.
      String binaryName = getBinaryNameForClass(fileManager, requiredInput);
      if (binaryName != null) {
        // Java package names map to folders on disk, so if we want to find the right directory
        // we replace each . with a /.
        String csubdir = binaryName.replace('.', '/');
        int cindex = strippedPath.indexOf(csubdir);
        if (cindex <= 0) {
          throw new ExtractionException(
              String.format(
                  "unable to infer classpath for %s from %s, %s",
                  strippedPath, csubdir, binaryName),
              false);
        } else {
          (location == StandardLocation.PLATFORM_CLASS_PATH
                  ? results.newBootClassPath
                  : results.newClassPath)
              .add(strippedPath.substring(0, cindex));
        }
      }
    }

    // Identify generated sources by checking if the source file is under the genSrcDirs.
    try {
      if (!genSrcDirs.isEmpty()
          && !isJarPath
          && requiredInput.getKind() == Kind.SOURCE
          && fileManager.contains(StandardLocation.SOURCE_OUTPUT, requiredInput)) {
        results.explicitSources.add(strippedPath);
        results.newSourcePath.addAll(genSrcDirs);
      }
    } catch (IOException err) {
      // Ignore IOExceptions potentially thrown by fileManager.contains(...);
    }

    if (!results.fileContents.containsKey(strippedPath)) {
      try {
        // Retrieve the contents of the file.
        InputStream stream = requiredInput.openInputStream();
        if (stream.markSupported()) {
          // The stream has already been read by the compiler, we need to reset it
          // so we can read it as well.
          stream.reset();
        }
        byte[] data = ByteStreams.toByteArray(stream);
        if (data.length == 0) {
          logger.atWarning().log("Empty java source file: %s", strippedPath);
        }
        results.fileContents.put(strippedPath, data);
        results.relativePaths.put(strippedPath, relativePath);
        if (sourceFiles.containsKey(requiredInput.toUri())) {
          results.sourceFileNames.put(strippedPath, sourceFiles.get(requiredInput.toUri()));
        }
      } catch (IOException e) {
        throw new ExtractionException(
            String.format("Unable to read file content of %s", strippedPath), e, false);
      }
    }
  }

  /** {@link Location}s that may contain class files. */
  private static final ImmutableSet<Location> CLASS_LOCATIONS =
      ImmutableSet.<Location>of(
          StandardLocation.CLASS_OUTPUT,
          StandardLocation.CLASS_PATH,
          StandardLocation.PLATFORM_CLASS_PATH);

  /**
   * Returns the location and binary name of a class file, or {@code null} if the file object is not
   * a class.
   */
  private static @Nullable String getBinaryNameForClass(
      UsageAsInputReportingFileManager fileManager, JavaFileObject fileObject)
      throws ExtractionException {
    if (fileObject.getKind() != Kind.CLASS) {
      return null;
    }
    String binaryName;
    for (Location location : CLASS_LOCATIONS) {
      if ((binaryName = fileManager.inferBinaryName(location, fileObject)) != null) {
        return binaryName;
      }
    }
    if (fileObject.isNameCompatible(MODULE_INFO_NAME, Kind.CLASS)) {
      // Ignore automatic module-info.class files
      return null;
    }
    throw new ExtractionException(
        String.format("unable to infer classpath for %s", fileObject.getName()), false);
  }

  private static class AnalysisResults {
    // Map from strippedPath to an input's relative path to the corpus root.
    final Map<String, String> relativePaths = new LinkedHashMap<>();
    // Map from strippedPath to an input's contents.
    final Map<String, byte[]> fileContents = new LinkedHashMap<>();
    // Map from strippedPath to an input's true source basename. This is usually only needed for
    // non-public top-level classes where their filename does not match the path derived from their
    // fully-qualified name.
    final Map<String, String> sourceFileNames = new HashMap<>();

    // We build a new sourcepath & classpath that contain the minimum set of paths
    // as well as the modified set of paths that are needed to analyze the single compilation unit.
    // This is done to speed up analysis.
    final Set<String> newSourcePath = new LinkedHashSet<>();
    final Set<String> newClassPath = new LinkedHashSet<>();
    final Set<String> newBootClassPath = new LinkedHashSet<>();
    final List<String> explicitSources = new ArrayList<>();
    boolean hasErrors = false;
  }

  // Install NonResolvingCacheFSInfo into Context to avoid resolving symlinks.
  private static void setupFSInfo(StandardJavaFileManager fileManager) {
    Context context = new Context();
    NonResolvingCacheFSInfo.preRegister(context);
    ((JavacFileManager) fileManager).setContext(context);
  }

  private static UsageAsInputReportingFileManager getFileManager(
      JavaCompiler compiler, DiagnosticCollector<JavaFileObject> diagnosticCollector) {
    // We insert a filemanager that wraps the standard filemanager and records the compiler's
    // usage of .java & .class files.
    StandardJavaFileManager fileManager =
        compiler.getStandardFileManager(diagnosticCollector, null, null);
    setupFSInfo(fileManager);
    return new UsageAsInputReportingFileManager(fileManager);
  }

  // FSInfo class which does not resolve symlinks when canonicalizing paths.
  private static class NonResolvingCacheFSInfo extends CacheFSInfo {
    public static void preRegister(Context context) {
      context.put(
          FSInfo.class,
          new Context.Factory<FSInfo>() {
            @Override
            public FSInfo make(Context c) {
              FSInfo instance = new NonResolvingCacheFSInfo();
              c.put(FSInfo.class, instance);
              return instance;
            }
          });
    }

    @Override
    public Path getCanonicalFile(Path file) {
      return file.toAbsolutePath();
    }
  }

  private AnalysisResults runJavaAnalysisToExtractCompilationDetails(
      Iterable<String> sources,
      Map<Location, Iterable<String>> searchPaths,
      Iterable<String> processors,
      Iterable<String> options)
      throws ExtractionException {

    AnalysisResults results = new AnalysisResults();

    // Generate class files in a temporary directory
    try (ExtractionTask task = new ExtractionTask(allowServiceProcessors)) {
      for (Map.Entry<Location, Iterable<String>> entry : searchPaths.entrySet()) {
        setLocation(task.getFileManager(), entry.getKey(), entry.getValue());
      }

      results.hasErrors = !task.compile(options, sources, processors);

      for (String source : sources) {
        results.explicitSources.add(ExtractorUtils.tryMakeRelative(rootDirectory, source));
      }

      results.newSourcePath.addAll(getAdditionalSourcePaths(task.getCompilations()));

      // Include any .pb.meta files which may reside next to compilations.
      findMetadataFiles(task.getFileManager(), task.getCompilations());
      // Find files potentially used for resolving .* imports.
      findOnDemandImportedFiles(task.getFileManager(), task.getCompilations());
      // Add modular system files directly, if specified explicitly.
      addSystemFiles(results);

      // We accumulate all file contents from the java compiler.
      findRequiredFiles(task.getFileManager(), mapClassesToSources(task.getSymbolTable()), results);
    }

    return results;
  }

  /** Sets the given location using the FileManager API. */
  private static void setLocation(
      UsageAsInputReportingFileManager fileManager, Location location, Iterable<String> searchpath)
      throws ExtractionException {
    if (!Iterables.isEmpty(searchpath)) {
      try {
        fileManager.setLocation(location, Iterables.transform(searchpath, File::new));
      } catch (IOException e) {
        throw new ExtractionException(String.format("Couldn't set %s", location), e, false);
      }
    }
  }

  /**
   * Adds the files from "system" directory. When --system=DIR option is present, the compiler looks
   * for system classes (e.g., java.lang.*) in DIR, which contains two files: DIR/lib/jrt-fs.jar and
   * DIR/lib/modules. The former is a regular jar containing the implementation of a "module" file
   * system, and the latter contains such file system. The analyzer will be also invoked with
   * --system and thus will need them.
   */
  private void addSystemFiles(AnalysisResults results) throws ExtractionException {
    if (Strings.isNullOrEmpty(systemDir)) {
      return;
    }
    for (String fname : ImmutableList.of("jrt-fs.jar", "modules")) {
      Path modPath = Paths.get(systemDir, "lib", fname);
      String relativePath = ExtractorUtils.tryMakeRelative(rootDirectory, modPath.toString());
      if (results.fileContents.containsKey(relativePath)) {
        continue;
      }
      try {
        results.fileContents.put(relativePath, Files.readAllBytes(modPath));
        results.relativePaths.put(relativePath, relativePath);
      } catch (IOException e) {
        throw new ExtractionException(
            String.format("Bad system directory, cannot read %s", modPath), e, false);
      }
    }
  }

  /** Create the ClassLoader to use for annotation processors. */
  private static ClassLoader processingClassLoader(StandardJavaFileManager fileManager)
      throws ExtractionException {
    // If javac is run with -processor set and -processorpath *unset*, it will fall back to
    // searching the regular classpath for annotation processors.
    Iterable<? extends File> files =
        fileManager.hasLocation(StandardLocation.ANNOTATION_PROCESSOR_PATH)
            ? fileManager.getLocation(StandardLocation.ANNOTATION_PROCESSOR_PATH)
            : fileManager.getLocation(StandardLocation.CLASS_PATH);

    List<URL> urls = new ArrayList<>();
    for (File file : files) {
      try {
        urls.add(file.toURI().toURL());
      } catch (MalformedURLException e) {
        throw new ExtractionException("Bad processorpath entry", e, false);
      }
    }
    ClassLoader parent = new MaskedClassLoader();
    return new URLClassLoader(Iterables.toArray(urls, URL.class), parent);
  }

  /** Isolated classloader for annotation processors, to avoid skew with the ambient classpath. */
  private static class MaskedClassLoader extends ClassLoader {
    private MaskedClassLoader() {
      // delegate only to the bootclasspath
      super(ClassLoader.getPlatformClassLoader());
    }

    @Override
    protected Class<?> findClass(String name) throws ClassNotFoundException {
      if (name.startsWith("com.sun.source.") || name.startsWith("com.sun.tools.")) {
        return JavaCompilationUnitExtractor.class.getClassLoader().loadClass(name);
      }
      throw new ClassNotFoundException(name);
    }
  }

  private static class CompilationUnitCollector implements TaskListener {
    private final List<CompilationUnitTree> compilationUnits = new ArrayList<>();

    public Collection<CompilationUnitTree> getCompilations() {
      return Collections.unmodifiableCollection(compilationUnits);
    }

    @Override
    public void finished(TaskEvent e) {
      if (e.getKind() == TaskEvent.Kind.PARSE) {
        compilationUnits.add(e.getCompilationUnit());
      }
    }

    @Override
    public void started(TaskEvent e) {}
  }

  private static class TemporaryDirectory implements AutoCloseable {
    private final Path path;

    TemporaryDirectory() throws ExtractionException {
      try {
        this.path = Files.createTempDirectory("javac_extractor");
      } catch (IOException ioe) {
        throw new ExtractionException(
            "Unable to create temporary .class output directory", ioe, true);
      }
    }

    public Path getPath() {
      return path;
    }

    @Override
    public void close() {
      try {
        DeleteRecursively.delete(path);
      } catch (IOException ioe) {
        logger.atSevere().withCause(ioe).log("Failed to delete temporary directory %s", path);
      }
    }
  }

  /**
   * Completes the given raw compiler options with the given classpath, sourcepath, and temporary
   * destination directory. Only options supported by the Java compiler will be within the returned
   * {@link List}.
   */
  private static ImmutableList<String> completeCompilerOptions(
      Iterable<String> rawOptions, Path tempDestinationDir) {
    return ModifiableOptions.of(rawOptions)
        .removeUnsupportedOptions()
        .ensureEncodingSet(StandardCharsets.UTF_8)
        .replaceOptionValue(Option.D, tempDestinationDir.toString())
        .build();
  }

  private static Optional<JavaCompiler> findJavaCompiler() {
    JavaCompiler compiler = ToolProvider.getSystemJavaCompiler();
    if (compiler == null && moduleClassLoader != null) {
      // This is all a bit of a hack to be able to extract OpenJDK itself, which
      // uses a bootstrap compiler and a lot of JDK options to compile itself.
      // Notably, when using modules the system compiler is inhibited and the actual compiler
      // resides in jdk.compiler.iterim.  Rather than hard-code this, just fall back to the first
      // JavaCompiler we can find.
      logger.atWarning().log("Unable to find system compiler, using first available.");
      for (JavaCompiler found : ServiceLoader.load(JavaCompiler.class, moduleClassLoader)) {
        return Optional.ofNullable(found);
      }
    }
    return Optional.ofNullable(compiler);
  }

  private static JavaCompiler getJavaCompiler() {
    return findJavaCompiler()
        .orElseThrow(
            () ->
                // TODO(schroederc): provide link to further context
                new IllegalStateException(
                    "Could not get system Java compiler; are you missing the JDK?"));
  }

  private static ClassLoader findModuleClassLoader() {
    return JavaCompilationUnitExtractor.class
        .getModule()
        .addUses(JavaCompiler.class)
        .getClassLoader();
  }

  /** Returns a map from a classfile's {@link URI} to its sourcefile path's basename. */
  private static Map<URI, String> mapClassesToSources(Symtab syms) {
    Map<URI, String> sourceBaseNames = new HashMap<>();
    for (ClassSymbol sym : syms.getAllClasses()) {
      if (sym.sourcefile != null && sym.classfile != null) {
        String path = sym.sourcefile.toUri().getPath();
        if (path != null) {
          String basename = Paths.get(path).getFileName().toString();
          if (!basename.endsWith(".java")) {
            logger.atWarning().log("Invalid sourcefile name: '%s'", basename);
          }
          sourceBaseNames.put(sym.classfile.toUri(), basename);
        }
      }
    }
    return sourceBaseNames;
  }

  private static Iterable<Processor> loadProcessors(
      ClassLoader loader, Iterable<String> names, boolean allowServiceProcessors)
      throws ExtractionException {
    return Iterables.isEmpty(names) && allowServiceProcessors
        // If no --processors were passed, add any processors registered in the META-INF/services
        // configuration.
        ? loadServiceProcessors(loader)
        // Add any processors passed as flags.
        : loadNamedProcessors(loader, names);
  }

  private static List<Processor> loadNamedProcessors(ClassLoader loader, Iterable<String> names)
      throws ExtractionException {
    List<Processor> procs = new ArrayList<>();
    for (String processor : names) {
      try {
        procs.add(
            loader.loadClass(processor).asSubclass(Processor.class).getConstructor().newInstance());
      } catch (Throwable e) {
        throw new ExtractionException("Bad processor entry: " + processor, e, false);
      }
    }
    return procs;
  }

  private static Iterable<Processor> loadServiceProcessors(ClassLoader loader) {
    return ServiceLoader.load(Processor.class, loader);
  }

  private static boolean logErrors(Collection<Diagnostic<? extends JavaFileObject>> diagnostics) {
    // If we encountered any compilation errors, we report them even though we
    // still store the compilation information for this set of sources.
    return diagnostics.stream()
            .filter(d -> d.getKind() == Diagnostic.Kind.ERROR)
            .peek(
                diag -> {
                  if (diag.getSource() != null) {
                    logger.atSevere().log(
                        "compiler error: %s(%d): %s",
                        diag.getSource().getName(),
                        diag.getLineNumber(),
                        diag.getMessage(Locale.ENGLISH));
                  } else {
                    logger.atSevere().log("compiler error: %s", diag.getMessage(Locale.ENGLISH));
                  }
                })
            .count()
        > 0;
  }

  private static void logFatalErrors(Collection<Diagnostic<? extends JavaFileObject>> diagnostics) {
    // Type resolution issues, the diagnostics will give hints on what's going wrong.
    diagnostics.stream()
        .filter(d -> d.getKind() == Diagnostic.Kind.ERROR)
        .forEach(
            d ->
                logger.atSevere().log("Fatal error in compiler: %s", d.getMessage(Locale.ENGLISH)));
  }

  private static Symtab getSymbolTable(JavacTask javacTask) {
    return getSymbolTable(((JavacTaskImpl) javacTask).getContext());
  }

  private static Symtab getSymbolTable(Context context) {
    return Symtab.instance(context);
  }

  // Returns a string to use for the compilation unit working directory.
  // Attempts to use a stable path for the root, but falls back to the specified rootDirectory
  // if the stable root is present in an absolutely-qualified requiredInput or the rootDirectory
  // is mentioned in options.
  private static String stableRoot(
      String rootDirectory, Iterable<String> options, Iterable<FileInput> requiredInput) {
    if (Iterables.any(options, o -> o.contains(rootDirectory))) {
      logger.atInfo().log(
          "Using real working directory (%s) due to its inclusion in %s", rootDirectory, options);
      return rootDirectory;
    }
    ImmutableSet<String> requiredRoots =
        Streams.stream(requiredInput)
            .map(
                fi -> {
                  Path path = Paths.get(fi.getInfo().getPath());
                  return path.isAbsolute() ? path.subpath(0, 1).toString() : null;
                })
            .filter(p -> p != null)
            .collect(ImmutableSet.toImmutableSet());
    for (String root : ImmutableList.of("root", "build", "kythe_java_extractor_root")) {
      if (!requiredRoots.contains(root)) {
        return "/" + root;
      }
    }
    return rootDirectory;
  }
}
