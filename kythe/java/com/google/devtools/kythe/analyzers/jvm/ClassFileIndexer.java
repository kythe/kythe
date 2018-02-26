/*
 * Copyright 2018 Google Inc. All rights reserved.
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

import com.beust.jcommander.Parameter;
import com.google.common.base.Strings;
import com.google.common.util.concurrent.ListenableFuture;
import com.google.devtools.kythe.analyzers.base.FactEmitter;
import com.google.devtools.kythe.analyzers.base.IndexerConfig;
import com.google.devtools.kythe.analyzers.base.StreamFactEmitter;
import com.google.devtools.kythe.common.FormattingLogger;
import com.google.devtools.kythe.extractors.shared.CompilationDescription;
import com.google.devtools.kythe.extractors.shared.IndexInfoUtils;
import com.google.devtools.kythe.platform.shared.AnalysisException;
import com.google.devtools.kythe.platform.shared.FileDataCache;
import com.google.devtools.kythe.platform.shared.FileDataProvider;
import com.google.devtools.kythe.platform.shared.MemoryStatisticsCollector;
import com.google.devtools.kythe.platform.shared.NullStatisticsCollector;
import com.google.devtools.kythe.platform.shared.StatisticsCollector;
import com.google.devtools.kythe.proto.Analysis.CompilationUnit;
import com.google.devtools.kythe.proto.Analysis.FileInfo;
import com.google.devtools.kythe.proto.Java.JarDetails;
import com.google.devtools.kythe.proto.Storage.VName;
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
import java.util.Enumeration;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ExecutionException;
import java.util.jar.JarEntry;
import java.util.jar.JarFile;

/**
 * Kythe analyzer for JVM class files (possibly within a jar or kindex file).
 *
 * <p>Usage: class_file_indexer <class_file | jar_file | kindex_file> ...
 */
public class ClassFileIndexer {
  public static final String JAR_FILE_EXT = ".jar";
  public static final String CLASS_FILE_EXT = ".class";
  public static final String JAR_DETAILS_URL = "kythe.io/proto/kythe.proto.JarDetails";

  private static final FormattingLogger logger = FormattingLogger.getLogger(ClassFileIndexer.class);

  public static void main(String[] args) throws AnalysisException {
    StandaloneConfig config = new StandaloneConfig();
    config.parseCommandLine(args);

    try (OutputStream stream =
        Strings.isNullOrEmpty(config.getOutputPath())
            ? System.out
            : new BufferedOutputStream(new FileOutputStream(config.getOutputPath()))) {
      FactEmitter emitter = new StreamFactEmitter(stream);
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
        } else if (fileName.endsWith(IndexInfoUtils.INDEX_FILE_EXT)) {
          CompilationDescription desc = IndexInfoUtils.readIndexInfoFromFile(fileName);
          analyzeCompilation(
              desc.getCompilationUnit(), new FileDataCache(desc.getFileContents()), classVisitor);
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
      FactEmitter emitter)
      throws AnalysisException {
    KytheClassVisitor classVisitor = new KytheClassVisitor(statistics, emitter);
    analyzeCompilation(compilationUnit, fileDataProvider, classVisitor);
  }

  private static void analyzeCompilation(
      CompilationUnit compilationUnit,
      FileDataProvider fileDataProvider,
      KytheClassVisitor classVisitor)
      throws AnalysisException {
    Map<FileInfo, VName> enclosingJars = createJarMap(compilationUnit);
    for (CompilationUnit.FileInput file : compilationUnit.getRequiredInputList()) {
      FileInfo info = file.getInfo();
      if (info.getPath().endsWith(CLASS_FILE_EXT)) {
        classVisitor = classVisitor.withEnclosingJarFile(enclosingJars.get(info));
        try {
          ListenableFuture<byte[]> contents = fileDataProvider.startLookup(info);
          classVisitor.visitClassFile(contents.get());
        } catch (ExecutionException | InterruptedException e) {
          throw new AnalysisException("error retrieving file contents for " + info, e);
        }
      }
    }
  }

  private static Map<FileInfo, VName> createJarMap(CompilationUnit compilationUnit) {
    JarDetails jarDetails = null;
    for (Any details : compilationUnit.getDetailsList()) {
      if (details.getTypeUrl().equals(JAR_DETAILS_URL)) {
        try {
          jarDetails = JarDetails.parseFrom(details.getValue());
        } catch (InvalidProtocolBufferException ipbe) {
          logger.warning(ipbe, "Error unpacking JarDetails");
        }
      }
    }
    Map<FileInfo, VName> map = new HashMap<>();
    if (jarDetails == null) {
      return map;
    }
    for (JarDetails.Jar jar : jarDetails.getJarList()) {
      for (FileInfo entry : jar.getEntryList()) {
        map.put(entry, jar.getVName());
      }
    }
    return map;
  }

  private static void visitJarClassFiles(File jarFile, KytheClassVisitor visitor)
      throws AnalysisException {
    try (JarFile jar = new JarFile(jarFile)) {
      for (Enumeration<JarEntry> entries = jar.entries(); entries.hasMoreElements(); ) {
        JarEntry entry = entries.nextElement();
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
      description = "Print final analyzer statistics to stderr"
    )
    private boolean printStatistics;

    @Parameter(
      names = {"--out", "-out"},
      description = "Write the entries to this file (or stdout if unspecified)"
    )
    private String outputPath;

    public StandaloneConfig() {
      super("classfile-indexer");
    }

    public final boolean getPrintStatistics() {
      return printStatistics;
    }

    public final String getOutputPath() {
      return outputPath;
    }

    public final List<String> getFilesToIndex() {
      return filesToIndex;
    }
  }
}
