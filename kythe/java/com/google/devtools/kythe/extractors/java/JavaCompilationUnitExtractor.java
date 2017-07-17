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

package com.google.devtools.kythe.extractors.java;

import static java.nio.file.LinkOption.NOFOLLOW_LINKS;

import com.google.common.base.Charsets;
import com.google.common.base.Joiner;
import com.google.common.base.Optional;
import com.google.common.base.Preconditions;
import com.google.common.base.Strings;
import com.google.common.collect.HashMultimap;
import com.google.common.collect.ImmutableSet;
import com.google.common.collect.Iterables;
import com.google.common.collect.Lists;
import com.google.common.collect.Multimap;
import com.google.common.collect.Sets;
import com.google.common.io.ByteStreams;
import com.google.devtools.kythe.common.FormattingLogger;
import com.google.devtools.kythe.extractors.shared.CompilationDescription;
import com.google.devtools.kythe.extractors.shared.CompilationFileInputComparator;
import com.google.devtools.kythe.extractors.shared.ExtractionException;
import com.google.devtools.kythe.extractors.shared.ExtractorUtils;
import com.google.devtools.kythe.extractors.shared.FileVNames;
import com.google.devtools.kythe.platform.java.JavacOptionsUtils;
import com.google.devtools.kythe.proto.Analysis.CompilationUnit;
import com.google.devtools.kythe.proto.Analysis.CompilationUnit.FileInput;
import com.google.devtools.kythe.proto.Analysis.FileData;
import com.google.devtools.kythe.proto.Buildinfo.BuildDetails;
import com.google.devtools.kythe.proto.Java.JavaDetails;
import com.google.devtools.kythe.proto.Storage.VName;
import com.google.devtools.kythe.util.DeleteRecursively;
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
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.EnumSet;
import java.util.HashMap;
import java.util.LinkedHashMap;
import java.util.LinkedHashSet;
import java.util.LinkedList;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.ServiceLoader;
import java.util.Set;
import javax.annotation.processing.Processor;
import javax.tools.Diagnostic;
import javax.tools.DiagnosticCollector;
import javax.tools.JavaCompiler;
import javax.tools.JavaCompiler.CompilationTask;
import javax.tools.JavaFileManager.Location;
import javax.tools.JavaFileObject;
import javax.tools.JavaFileObject.Kind;
import javax.tools.StandardJavaFileManager;
import javax.tools.StandardLocation;
import javax.tools.ToolProvider;

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

  private static final FormattingLogger logger =
      FormattingLogger.getLogger(JavaCompilationUnitExtractor.class);

  private static final String SOURCE_JAR_ROOT = "!SOURCE_JAR!";

  private static String classJarRoot(Location location) {
    return String.format("!%s_JAR!", location);
  }

  private static final String JAR_SCHEME = "jar";
  private final String jdkJar;
  private final String rootDirectory;
  private final FileVNames fileVNames;

  /**
   * Creates an instance of the JavaExtractor to store java compilation information in an .kindex
   * file.
   */
  public JavaCompilationUnitExtractor(String corpus) throws ExtractionException {
    this(corpus, ExtractorUtils.getCurrentWorkingDirectory());
  }

  /**
   * Creates an instance of the JavaExtractor to store java compilation information in an .kindex
   * file.
   */
  public JavaCompilationUnitExtractor(String corpus, String rootDirectory)
      throws ExtractionException {
    this(FileVNames.staticCorpus(corpus), rootDirectory);
  }

  /**
   * Creates an instance of the JavaExtractor to store java compilation information in an .kindex
   * file.
   */
  public JavaCompilationUnitExtractor(FileVNames fileVNames) throws ExtractionException {
    this(fileVNames, ExtractorUtils.getCurrentWorkingDirectory());
  }

  public JavaCompilationUnitExtractor(FileVNames fileVNames, String rootDirectory)
      throws ExtractionException {
    this.fileVNames = fileVNames;

    Path javaHome = Paths.get(System.getProperty("java.home")).getParent();
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

  public String getRootDirectory() {
    return rootDirectory;
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
    unit.setVName(VName.newBuilder().setSignature(target).setLanguage("java"));
    unit.addAllArgument(options);
    unit.setHasCompileErrors(hasErrors);
    unit.addAllRequiredInput(requiredInputs);
    for (String sourceFile : sourceFiles) {
      unit.addSourceFile(sourceFile);
    }
    unit.setOutputKey(outputPath);
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
      Optional<Path> genSrcDir,
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
    Preconditions.checkNotNull(genSrcDir);
    Preconditions.checkNotNull(options);
    Preconditions.checkNotNull(outputPath);

    AnalysisResults results;
    if (sources.iterator().hasNext()) {
      results =
          runJavaAnalysisToExtractCompilationDetails(
              sources,
              classpath,
              bootclasspath,
              sourcepath,
              processorpath,
              processors,
              genSrcDir,
              options);
    } else {
      results = new AnalysisResults();
    }

    List<FileData> fileContents = ExtractorUtils.convertBytesToFileDatas(results.fileContents);
    List<FileInput> compilationFileInputs = new LinkedList<>();
    for (FileData data : fileContents) {
      String relativePath = results.relativePaths.get(data.getInfo().getPath());
      VName vname = fileVNames.lookupBaseVName(relativePath);
      if (vname.getPath().isEmpty()) {
        vname = vname.toBuilder().setPath(data.getInfo().getPath()).build();
      }
      String sourceBasename = results.sourceFileNames.get(data.getInfo().getPath());
      if (sourceBasename != null
          && vname.getPath().endsWith(".java")
          && !vname.getPath().endsWith(sourceBasename)) {
        Path fixedPath = Paths.get(vname.getPath()).getParent().resolve(sourceBasename);
        vname = vname.toBuilder().setPath(fixedPath.toString()).build();
      }
      compilationFileInputs.add(
          FileInput.newBuilder().setInfo(data.getInfo()).setVName(vname).build());
    }
    Collections.sort(compilationFileInputs, CompilationFileInputComparator.getComparator());

    CompilationUnit compilationUnit =
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

  /** Returns a new list with the same options except any destination directory options. */
  private static List<String> removeDestDirOptions(Iterable<String> options) {
    return JavacOptionsUtils.removeOptions(
        Lists.newArrayList(options), EnumSet.of(Option.D, Option.S, Option.H));
  }

  /**
   * If the code has wildcard imports (e.g. import foo.bar.*) but doesn't actually use any of the
   * imports, errors will happen. We don't get callbacks for file open of these files (since they
   * aren't used) but when java runs it will report errors if it can't find any files to match the
   * wildcard. So we add one matching file here.
   */
  private void findOnDemandImportedFiles(
      Iterable<? extends CompilationUnitTree> compilationUnits,
      UsageAsInputReportingFileManager fileManager)
      throws ExtractionException {
    // Maps package names to source files that wildcard import them.
    Multimap<String, String> pkgs = HashMultimap.create();

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
                    StandardLocation.CLASS_PATH, pkg.getKey(), Sets.newHashSet(Kind.CLASS), false),
                null);
        if (firstClass == null) {
          firstClass =
              Iterables.getFirst(
                  fileManager.list(
                      StandardLocation.PLATFORM_CLASS_PATH,
                      pkg.getKey(),
                      Sets.newHashSet(Kind.CLASS),
                      false),
                  null);
        }
        if (firstClass != null) {
          firstClass.getCharContent(true);
        }
        JavaFileObject firstSource =
            Iterables.getFirst(
                fileManager.list(
                    StandardLocation.SOURCE_PATH,
                    pkg.getKey(),
                    Sets.newHashSet(Kind.SOURCE),
                    false),
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
  private void getAdditionalSourcePaths(
      Iterable<? extends CompilationUnitTree> compilationUnits, AnalysisResults results) {

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
          String path = compilationUnit.getSourceFile().toUri().getPath();
          // This needs to be lastIndexOf as there are source jars that
          // contain the same package name in the files contained in them
          // as the path the source jars live in. As we extract the source
          // jars, we end up with a double named path.
          int index = path.lastIndexOf(packageSubDir);
          if (index >= 0) {
            String root = ExtractorUtils.tryMakeRelative(rootDirectory, path.substring(0, index));
            results.newSourcePath.add(root);
          }
        }
      }
    }
  }

  private void findRequiredFiles(
      UsageAsInputReportingFileManager fileManager,
      Map<URI, String> sourceFiles,
      Optional<Path> genSrcDir,
      AnalysisResults results)
      throws ExtractionException {
    for (InputUsageRecord input : fileManager.getUsages()) {
      processRequiredInput(
          input.fileObject(), input.location(), fileManager, sourceFiles, genSrcDir, results);
    }
  }

  private void processRequiredInput(
      JavaFileObject requiredInput,
      Location location,
      UsageAsInputReportingFileManager fileManager,
      Map<URI, String> sourceFiles,
      Optional<Path> genSrcDir,
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
        jarPath = jarConn.getJarFileURL().getFile().toString();
        // jar entries don't have a leading '/', and we expect
        // paths like "!CLASS_PATH_JAR!/com/foo/Bar.class"
        entryPath = "/" + jarConn.getEntryName();
      } else {
        entryPath = url.getFile().toString();
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

    // If the file was part of the JDK we do not store it as the JDK is tied
    // to the analyzer we'll run on this information later on.
    if (isJarPath && jarPath.startsWith(jdkJar)) {
      return;
    }

    // Make the path relative to the indexer (e.g. a subdir of corpus/).
    // If not possible, we store the fullpath.
    String relativePath =
        ExtractorUtils.tryMakeRelative(rootDirectory, uri.getRawSchemeSpecificPart());

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
          // TODO(T227): we shouldn't need to throw an exception here because the above switch
          // statement means that we never hit this default, but the static analysis tools can't
          // figure that out.  Try to refactor this code to remove this issue.
          throw new IllegalStateException(
              String.format(
                  "Unsupported java file kind: '%s' for '%s'",
                  requiredInput.getKind().name(), uri));
      }
    }
    if (!isJarPath && requiredInput.getKind() == Kind.CLASS) {
      // If the class file was on disk, we need to infer the correct classpath to add.
      String binaryName = getBinaryNameForClass(fileManager, requiredInput);
      // Java package names map to folders on disk, so if we want to find the right directory
      // we replace each . with a /.
      String csubdir = binaryName.replace('.', '/');
      int cindex = strippedPath.indexOf(csubdir);
      if (cindex <= 0) {
        throw new ExtractionException(
            String.format(
                "unable to infer classpath for %s from %s, %s", strippedPath, csubdir, binaryName),
            false);
      } else {
        (location == StandardLocation.PLATFORM_CLASS_PATH
                ? results.newBootClassPath
                : results.newClassPath)
            .add(strippedPath.substring(0, cindex));
      }
    }

    // Identify generated sources by checking if the source file is under the genSrcDir.
    if (genSrcDir.isPresent()
        && !isJarPath
        && requiredInput.getKind() == Kind.SOURCE
        && Paths.get(relativePath).startsWith(genSrcDir.get())) {
      results.explicitSources.add(strippedPath);
      results.newSourcePath.add(genSrcDir.get().toString());
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
          logger.warningfmt("Empty java source file: %s", strippedPath);
        }
        results.fileContents.put(strippedPath, data);
        results.relativePaths.put(strippedPath, relativePath);
        if (sourceFiles.containsKey(requiredInput.toUri())) {
          results.sourceFileNames.put(strippedPath, sourceFiles.get(requiredInput.toUri()));
        }
      } catch (IOException e) {
        throw new ExtractionException(
            String.format("Unable to read file content of %s", strippedPath), false);
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
  private static String getBinaryNameForClass(
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
  private void setupFSInfo(StandardJavaFileManager fileManager) {
    Context context = new Context();
    NonResolvingCacheFSInfo.preRegister(context);
    ((JavacFileManager) fileManager).setContext(context);
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
      Iterable<String> classpath,
      Iterable<String> bootclasspath,
      Iterable<String> sourcepath,
      Iterable<String> processorpath,
      Iterable<String> processors,
      Optional<Path> genSrcDir,
      Iterable<String> options)
      throws ExtractionException {

    AnalysisResults results = new AnalysisResults();

    // We will initialize and run the Javac compiler to detect which dependencies
    // the current compilation has.
    JavaCompiler compiler = ToolProvider.getSystemJavaCompiler();
    if (compiler == null) {
      // TODO(schroederc): provide link to further context
      throw new IllegalStateException(
          "Could not get system Java compiler; are you missing the JDK?");
    }

    DiagnosticCollector<JavaFileObject> diagnosticsCollector =
        new DiagnosticCollector<JavaFileObject>();

    StandardJavaFileManager standardFileManager =
        compiler.getStandardFileManager(diagnosticsCollector, null, null);
    setupFSInfo(standardFileManager);

    // We insert a filemanager that wraps the standard filemanager and records the compiler's
    // usage of .java & .class files.
    final UsageAsInputReportingFileManager fileManager =
        new UsageAsInputReportingFileManager(standardFileManager);

    Iterable<? extends JavaFileObject> sourceFiles = fileManager.getJavaFileForSources(sources);

    // Generate class files in a temporary directory
    Path tempDir;
    try {
      tempDir = Files.createTempDirectory("javac_extractor");
    } catch (IOException ioe) {
      throw new ExtractionException(
          "Unable to create temporary .class output directory", ioe, true);
    }
    List<String> completeOptions =
        completeCompilerOptions(
            standardFileManager, options, classpath, bootclasspath, sourcepath, tempDir);

    final List<CompilationUnitTree> compilationUnits = new ArrayList<>();
    Symtab syms;
    try {
      // Launch the java compiler with our modified settings and the filemanager wrapper
      CompilationTask task =
          compiler.getTask(
              null, fileManager, diagnosticsCollector, completeOptions, null, sourceFiles);

      List<Processor> procs = Lists.<Processor>newArrayList(new ProcessAnnotation(fileManager));
      ClassLoader loader = processingClassloader(classpath, processorpath);

      // Add any processors passed as flags.
      for (String processor : processors) {
        try {
          procs.add(
              loader
                  .loadClass(processor)
                  .asSubclass(Processor.class)
                  .getConstructor()
                  .newInstance());
        } catch (ReflectiveOperationException e) {
          throw new ExtractionException("Bad processor entry", e, false);
        }
      }

      if (procs.isEmpty()) {
        // If no --processors were passed, add any processors registered in the META-INF/services
        // configuration.
        for (Processor proc : ServiceLoader.load(Processor.class, loader)) {
          procs.add(proc);
        }
      }

      JavacTask javacTask = (JavacTask) task;
      javacTask.setProcessors(procs);
      syms = Symtab.instance(((JavacTaskImpl) javacTask).getContext());

      javacTask.addTaskListener(
          new TaskListener() {
            @Override
            public void finished(TaskEvent e) {
              if (e.getKind() == TaskEvent.Kind.PARSE) {
                compilationUnits.add(e.getCompilationUnit());
              }
            }

            @Override
            public void started(TaskEvent e) {}
          });

      try {
        // In order for the compiler to load all required .java & .class files we need to have it go
        // through parsing, analysis & generate phases.  Unfortunately the latter is needed to get a
        // complete list, this was found as we were breaking on analyzing certain files.
        // JavacTask#call() subsumes parse() and generate(), but calling those methods directly may
        // silently ignore fatal errors.
        results.hasErrors = !javacTask.call();
      } catch (com.sun.tools.javac.util.Abort e) {
        // Type resolution issues, the diagnostics will give hints on what's going wrong.
        for (Diagnostic<? extends JavaFileObject> diagnostic :
            diagnosticsCollector.getDiagnostics()) {
          if (diagnostic.getKind() == Diagnostic.Kind.ERROR) {
            logger.severefmt("Fatal error in compiler: %s", diagnostic.getMessage(Locale.ENGLISH));
          }
        }
        throw new ExtractionException("Fatal error while running javac compiler.", e, false);
      }
    } finally {
      try {
        DeleteRecursively.delete(tempDir);
      } catch (IOException ioe) {
        logger.severefmt(ioe, "Failed to delete temporary directory " + tempDir);
      }
    }
    // If we encountered any compilation errors, we report them even though we
    // still store the compilation information for this set of sources.
    for (Diagnostic<? extends JavaFileObject> diag : diagnosticsCollector.getDiagnostics()) {
      if (diag.getKind() == Diagnostic.Kind.ERROR) {
        results.hasErrors = true;
        if (diag.getSource() != null) {
          logger.severefmt(
              "compiler error: %s(%d): %s",
              diag.getSource().getName(), diag.getLineNumber(), diag.getMessage(Locale.ENGLISH));
        } else {
          logger.severefmt("compiler error: %s", diag.getMessage(Locale.ENGLISH));
        }
      }
    }

    // Ensure generated source directory is relative to root.
    genSrcDir =
        genSrcDir.transform(
            p -> Paths.get(ExtractorUtils.tryMakeRelative(rootDirectory, p.toString())));

    for (String source : sources) {
      results.explicitSources.add(ExtractorUtils.tryMakeRelative(rootDirectory, source));
    }

    getAdditionalSourcePaths(compilationUnits, results);

    // Find files potentially used for resolving .* imports.
    findOnDemandImportedFiles(compilationUnits, fileManager);
    // We accumulate all file contents from the java compiler so we can store it in the bigtable.
    findRequiredFiles(fileManager, mapClassesToSources(syms), genSrcDir, results);
    return results;
  }

  /** Sets the given location using command-line flags and the FileManager API. */
  private static void setLocation(
      List<String> options,
      StandardJavaFileManager fileManager,
      Iterable<String> searchpath,
      String flag,
      StandardLocation location)
      throws ExtractionException {
    String joined = Joiner.on(":").join(searchpath);
    if (!joined.isEmpty()) {
      options.add(flag);
      options.add(joined);
      try {
        List<File> files = Lists.newArrayList();
        for (String elt : searchpath) {
          files.add(new File(elt));
        }
        fileManager.setLocation(location, files);
      } catch (IOException e) {
        throw new ExtractionException(String.format("Couldn't set %s", location), e, false);
      }
    }
  }

  /** Create the ClassLoader to use for annotation processors. */
  private static ClassLoader processingClassloader(
      Iterable<String> classpath, Iterable<String> processorpath) throws ExtractionException {
    // If javac is run with -processor set and -processorpath *unset*, it will fall back to
    // searching the regular classpath for annotation processors.
    if (Iterables.isEmpty(processorpath)) {
      processorpath = classpath;
    }

    List<URL> urls = new ArrayList<>();
    for (String path : processorpath) {
      try {
        urls.add(new File(path).toURI().toURL());
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
      super(null);
    }

    @Override
    protected Class<?> findClass(String name) throws ClassNotFoundException {
      if (name.startsWith("com.sun.source.") || name.startsWith("com.sun.tools.")) {
        return JavaCompilationUnitExtractor.class.getClassLoader().loadClass(name);
      }
      throw new ClassNotFoundException(name);
    }
  }

  /**
   * Completes the given raw compiler options with the given classpath, sourcepath, and temporary
   * destination directory. Only options supported by the Java compiler will be within the returned
   * {@link List}.
   */
  private static List<String> completeCompilerOptions(
      StandardJavaFileManager standardFileManager,
      Iterable<String> rawOptions,
      Iterable<String> classpath,
      Iterable<String> bootclasspath,
      Iterable<String> sourcepath,
      Path tempDestinationDir)
      throws ExtractionException {

    List<String> completeOptions =
        JavacOptionsUtils.ensureEncodingSet(
            JavacOptionsUtils.removeUnsupportedOptions(Lists.newArrayList(rawOptions)),
            Charsets.UTF_8);

    setLocation(
        completeOptions, standardFileManager, classpath, "-cp", StandardLocation.CLASS_PATH);
    setLocation(
        completeOptions,
        standardFileManager,
        sourcepath,
        "-sourcepath",
        StandardLocation.SOURCE_PATH);
    setLocation(
        completeOptions,
        standardFileManager,
        bootclasspath,
        "-bootclasspath",
        StandardLocation.PLATFORM_CLASS_PATH);

    JavacOptionsUtils.removeOptions(completeOptions, EnumSet.of(Option.D));
    completeOptions.add("-d");
    completeOptions.add(tempDestinationDir.toString());

    return completeOptions;
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
            logger.warning("Invalid sourcefile name: '" + basename + "'");
          }
          sourceBaseNames.put(sym.classfile.toUri(), basename);
        }
      }
    }
    return sourceBaseNames;
  }
}
