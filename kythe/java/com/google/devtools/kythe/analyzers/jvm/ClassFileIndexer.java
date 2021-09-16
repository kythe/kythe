/*
 * Copyright 2018 The Kythe Authors. All rights reserved.
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

package com.google.devtools.kythe.analyzers.jvm;

import static com.google.devtools.kythe.extractors.jvm.JvmExtractor.CLASS_FILE_EXT;
import static com.google.devtools.kythe.extractors.jvm.JvmExtractor.JAR_DETAILS_URL;
import static com.google.devtools.kythe.extractors.jvm.JvmExtractor.JAR_ENTRY_DETAILS_URL;
import static com.google.devtools.kythe.extractors.jvm.JvmExtractor.JAR_FILE_EXT;

import com.beust.jcommander.Parameter;
import com.google.common.base.Strings;
import com.google.common.collect.ImmutableList;
import com.google.common.flogger.FluentLogger;
import com.google.common.util.concurrent.ListenableFuture;
import com.google.devtools.kythe.analyzers.base.FactEmitter;
import com.google.devtools.kythe.analyzers.base.IndexerConfig;
import com.google.devtools.kythe.analyzers.base.StreamFactEmitter;
import com.google.devtools.kythe.extractors.shared.CompilationDescription;
import com.google.devtools.kythe.extractors.shared.IndexInfoUtils;
import com.google.devtools.kythe.platform.kzip.KZip;
import com.google.devtools.kythe.platform.kzip.KZipException;
import com.google.devtools.kythe.platform.kzip.KZipReader;
import com.google.devtools.kythe.platform.shared.AnalysisException;
import com.google.devtools.kythe.platform.shared.FileDataCache;
import com.google.devtools.kythe.platform.shared.FileDataProvider;
import com.google.devtools.kythe.platform.shared.MemoryStatisticsCollector;
import com.google.devtools.kythe.platform.shared.NullStatisticsCollector;
import com.google.devtools.kythe.platform.shared.StatisticsCollector;
import com.google.devtools.kythe.proto.Analysis.CompilationUnit;
import com.google.devtools.kythe.proto.Analysis.FileInfo;
import com.google.devtools.kythe.proto.Analysis.IndexedCompilation;
import com.google.devtools.kythe.proto.Java.JarDetails;
import com.google.devtools.kythe.proto.Java.JarEntryDetails;
import com.google.devtools.kythe.proto.Storage.VName;
import com.google.devtools.kythe.util.JsonUtil;
import com.google.protobuf.Any;
import com.google.protobuf.InvalidProtocolBufferException;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.ExecutionException;
import java.util.jar.JarEntry;
import java.util.jar.JarFile;

/**
 * Kythe analyzer for JVM class files (possibly within a jar or kindex file).
 *
 * <p>Usage: class_file_indexer <class_file | jar_file | kindex_file> ...
 */
public class ClassFileIndexer {
  private static final FluentLogger logger = FluentLogger.forEnclosingClass();

  private ClassFileIndexer() {}

  public static void main(String[] args) throws AnalysisException {
    // Necessary to allow the kzip library to read the any fields in the proto.
    JsonUtil.usingTypeRegistry(JsonUtil.JSON_TYPE_REGISTRY);

    StandaloneConfig config = new StandaloneConfig();
    config.parseCommandLine(args);

    try (OutputStream stream =
            Strings.isNullOrEmpty(config.getOutputPath())
                ? System.out
                : new BufferedOutputStream(new FileOutputStream(config.getOutputPath()));
        FactEmitter emitter = new StreamFactEmitter(stream)) {
      MemoryStatisticsCollector statistics =
          config.getPrintStatistics() ? new MemoryStatisticsCollector() : null;
      KytheClassVisitor classVisitor =
          new KytheClassVisitor(
              statistics == null ? NullStatisticsCollector.getInstance() : statistics, emitter);
      for (String fileName : config.getFilesToIndex()) {
        File file = new File(fileName);
        if (fileName.endsWith(JAR_FILE_EXT)) {
          visitJarClassFiles(file, classVisitor);
        } else if (fileName.endsWith(CLASS_FILE_EXT)) {
          visitClassFile(file, classVisitor);
        } else if (fileName.endsWith(IndexInfoUtils.KINDEX_FILE_EXT)) {
          CompilationDescription desc = IndexInfoUtils.readKindexInfoFromFile(fileName);
          analyzeCompilation(
              desc.getCompilationUnit(), new FileDataCache(desc.getFileContents()), classVisitor);
        } else if (fileName.endsWith(IndexInfoUtils.KZIP_FILE_EXT)) {
          boolean foundCompilation = false;
          try {
            KZip.Reader reader = new KZipReader(new File(fileName));
            for (IndexedCompilation indexedCompilation : reader.scan()) {
              foundCompilation = true;
              CompilationDescription desc =
                  IndexInfoUtils.indexedCompilationToCompilationDescription(
                      indexedCompilation, reader);
              analyzeCompilation(
                  desc.getCompilationUnit(),
                  new FileDataCache(desc.getFileContents()),
                  classVisitor);
            }
          } catch (KZipException e) {
            throw new AnalysisException("Couldn't open kzip file", e);
          }
          if (!config.getIgnoreEmptyKZip() && !foundCompilation) {
            throw new IllegalArgumentException(
                "given empty .kzip file \"" + fileName + "\"; try --ignore_empty_kzip");
          }
        } else {
          throw new IllegalArgumentException("unknown file path extension: " + fileName);
        }
      }
      if (statistics != null) {
        statistics.printStatistics(System.err);
      }
    } catch (IOException ioe) {
      throw new AnalysisException("error writing output", ioe);
    }
  }

  /** Analyze each class file contained within the {@link CompilationUnit}. */
  public static void analyzeCompilation(
      StatisticsCollector statistics,
      CompilationUnit compilationUnit,
      FileDataProvider fileDataProvider,
      FactEmitter emitter,
      boolean useCompilationCorpusAsDefault)
      throws AnalysisException {
    KytheClassVisitor classVisitor = new KytheClassVisitor(statistics, emitter);
    classVisitor.getEntrySets().setUseCompilationCorpusAsDefault(useCompilationCorpusAsDefault);
    analyzeCompilation(compilationUnit, fileDataProvider, classVisitor);
  }

  private static void analyzeCompilation(
      CompilationUnit compilationUnit,
      FileDataProvider fileDataProvider,
      KytheClassVisitor classVisitor)
      throws AnalysisException {
    ImmutableList<VName> enclosingJars = createJarIndex(compilationUnit);
    for (CompilationUnit.FileInput file : compilationUnit.getRequiredInputList()) {
      FileInfo info = file.getInfo();
      if (info.getPath().endsWith(CLASS_FILE_EXT)) {
        classVisitor = classVisitor.withEnclosingJarFile(getEnclosingJar(enclosingJars, file));
        try {
          ListenableFuture<byte[]> contents = fileDataProvider.startLookup(info);
          classVisitor.visitClassFile(contents.get());
        } catch (ExecutionException | InterruptedException e) {
          throw new AnalysisException("error retrieving file contents for " + info, e);
        }
      }
    }
  }

  private static ImmutableList<VName> createJarIndex(CompilationUnit compilationUnit) {
    JarDetails jarDetails = null;
    for (Any details : compilationUnit.getDetailsList()) {
      if (details.getTypeUrl().equals(JAR_DETAILS_URL)) {
        try {
          jarDetails = JarDetails.parseFrom(details.getValue());
        } catch (InvalidProtocolBufferException ipbe) {
          logger.atWarning().withCause(ipbe).log("Error unpacking JarDetails");
        }
      }
    }
    return jarDetails.getJarList().stream()
        .map(JarDetails.Jar::getVName)
        .collect(ImmutableList.toImmutableList());
  }

  private static VName getEnclosingJar(
      ImmutableList<VName> enclosingJars, CompilationUnit.FileInput file) {
    JarEntryDetails jarEntryDetails = null;
    for (Any details : file.getDetailsList()) {
      if (details.getTypeUrl().equals(JAR_ENTRY_DETAILS_URL)) {
        try {
          jarEntryDetails = JarEntryDetails.parseFrom(details.getValue());
        } catch (InvalidProtocolBufferException ipbe) {
          logger.atWarning().withCause(ipbe).log("Error unpacking JarEntryDetails");
        }
      }
    }
    if (jarEntryDetails == null) {
      return null;
    }
    int idx = jarEntryDetails.getJarContainer();
    if (idx < 0 || idx >= enclosingJars.size()) {
      logger.atWarning().log(
          "JarEntryDetails index out of range: %s (jars: %s)", jarEntryDetails, enclosingJars);
      return null;
    }
    return enclosingJars.get(idx);
  }

  @SuppressWarnings("JdkObsolete")
  private static Iterable<JarEntry> entries(JarFile jar) {
    return () -> jar.entries().asIterator();
  }

  private static void visitJarClassFiles(File jarFile, KytheClassVisitor visitor)
      throws AnalysisException {
    try (JarFile jar = new JarFile(jarFile)) {
      for (JarEntry entry : entries(jar)) {
        if (!entry.getName().endsWith(CLASS_FILE_EXT)) {
          continue;
        }
        try (InputStream input = jar.getInputStream(entry)) {
          visitor.visitClassFile(input);
        } catch (IOException ioe) {
          throw new AnalysisException("error reading class file: " + entry.getName(), ioe);
        }
      }
    } catch (IOException ioe) {
      throw new AnalysisException("error reading jar file: " + jarFile, ioe);
    }
  }

  private static void visitClassFile(File classFile, KytheClassVisitor visitor)
      throws AnalysisException {
    try (InputStream input = new FileInputStream(classFile)) {
      visitor.visitClassFile(input);
    } catch (IOException ioe) {
      throw new AnalysisException("error reading class file: " + classFile, ioe);
    }
  }

  private static class StandaloneConfig extends IndexerConfig {
    @Parameter(description = "<jar|class|kindex files to analyze>", required = true)
    private List<String> filesToIndex = new ArrayList<>();

    @Parameter(
        names = "--print_statistics",
        description = "Print final analyzer statistics to stderr")
    private boolean printStatistics;

    @Parameter(
        names = {"--out", "-out"},
        description = "Write the entries to this file (or stdout if unspecified)")
    private String outputPath;

    @Parameter(
        names = "--ignore_empty_kzip",
        description = "Ignore empty .kzip files; exit successfully with no output")
    private boolean ignoreEmptyKZip;

    public StandaloneConfig() {
      super("classfile-indexer");
    }

    public final boolean getPrintStatistics() {
      return printStatistics;
    }

    public final String getOutputPath() {
      return outputPath;
    }

    public final boolean getIgnoreEmptyKZip() {
      return ignoreEmptyKZip;
    }

    public final List<String> getFilesToIndex() {
      return filesToIndex;
    }
  }
}
