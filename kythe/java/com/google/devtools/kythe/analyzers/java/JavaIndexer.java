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

package com.google.devtools.kythe.analyzers.java;

import com.beust.jcommander.Parameter;
import com.google.common.base.Strings;
import com.google.devtools.kythe.analyzers.base.StreamFactEmitter;
import com.google.devtools.kythe.extractors.java.JavaCompilationUnitExtractor;
import com.google.devtools.kythe.extractors.shared.CompilationDescription;
import com.google.devtools.kythe.extractors.shared.IndexInfoUtils;
import com.google.devtools.kythe.platform.indexpack.Archive;
import com.google.devtools.kythe.platform.java.JavacAnalysisDriver;
import com.google.devtools.kythe.platform.kzip.KZipException;
import com.google.devtools.kythe.platform.shared.AnalysisException;
import com.google.devtools.kythe.platform.shared.FileDataCache;
import com.google.devtools.kythe.platform.shared.KytheMetadataLoader;
import com.google.devtools.kythe.platform.shared.MemoryStatisticsCollector;
import com.google.devtools.kythe.platform.shared.MetadataLoaders;
import com.google.devtools.kythe.platform.shared.NullStatisticsCollector;
import com.google.devtools.kythe.platform.shared.ProtobufMetadataLoader;
import com.google.devtools.kythe.util.JsonUtil;
import java.io.BufferedOutputStream;
import java.io.EOFException;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.net.MalformedURLException;
import java.net.URL;
import java.net.URLClassLoader;
import java.util.ArrayList;
import java.util.Collection;
import java.util.List;
import java.util.ServiceLoader;
import java.util.function.Supplier;

/**
 * Binary to run Kythe's Java index over a single .kindex file, emitting entries to a file or
 * STDOUT.
 */
public class JavaIndexer {
  public static void main(String[] args) throws AnalysisException, IOException {
    JsonUtil.usingTypeRegistry(JavaCompilationUnitExtractor.JSON_TYPE_REGISTRY);

    StandaloneConfig config = new StandaloneConfig();
    config.parseCommandLine(args);

    List<Supplier<Plugin>> plugins = new ArrayList<>();
    if (!Strings.isNullOrEmpty(config.getPlugin())) {
      URLClassLoader classLoader = new URLClassLoader(new URL[] {fileURL(config.getPlugin())});
      for (Plugin plugin : ServiceLoader.load(Plugin.class, classLoader)) {
        final Class<? extends Plugin> clazz = plugin.getClass();
        System.err.println("Registering plugin: " + clazz);
        plugins.add(
            () -> {
              try {
                return clazz.getConstructor().newInstance();
              } catch (Exception e) {
                throw new IllegalStateException("failed to construct Plugin " + clazz, e);
              }
            });
      }
    }
    if (config.getPrintVNames()) {
      plugins.add(Plugin.PrintKytheNodes::new);
    }

    MemoryStatisticsCollector statistics = null;
    if (config.getPrintStatistics()) {
      statistics = new MemoryStatisticsCollector();
    }

    List<String> compilation = config.getCompilation();
    if (compilation.size() > 1) {
      System.err.println("Java indexer received too many arguments; got " + compilation);
      config.showHelpAndExit();
    }

    CompilationDescription desc = null;
    String compilationPath = compilation.get(0);
    if (!Strings.isNullOrEmpty(config.getIndexPackRoot())) {
      // java_indexer --index_pack=archive-root unit-key
      desc = new Archive(config.getIndexPackRoot()).readDescription(compilationPath);
    } else {
      if (compilationPath.endsWith(IndexInfoUtils.KZIP_FILE_EXT)) {
        // java_indexer kzip-file
        try {
          Collection<CompilationDescription> compilationDescriptions =
              IndexInfoUtils.readKZip(compilationPath);
          if (compilationDescriptions.size() != 1) {
            throw new IllegalArgumentException(
                "The kzip did not contain exactly 1 CompilationDescription. It contained "
                    + compilationDescriptions.size());
          }
          desc = compilationDescriptions.iterator().next();
        } catch (KZipException e) {
          throw new IllegalArgumentException("Unable to read kzip", e);
        }
      } else {
        // java_indexer kindex-file
        try {
          desc = IndexInfoUtils.readKindexInfoFromFile(compilationPath);
        } catch (EOFException e) {
          if (config.getIgnoreEmptyKIndex()) {
            return;
          }
          throw new IllegalArgumentException(
              "given empty .kindex file \"" + compilationPath + "\"; try --ignore_empty_kindex", e);
        }
      }
    }

    if (desc == null) {
      throw new IllegalStateException("Unknown error reading CompilationDescription");
    }
    if (!desc.getFileContents().iterator().hasNext()) {
      return;
    }

    MetadataLoaders metadataLoaders = new MetadataLoaders();
    metadataLoaders.addLoader(
        new ProtobufMetadataLoader(desc.getCompilationUnit(), config.getDefaultMetadataCorpus()));
    metadataLoaders.addLoader(new KytheMetadataLoader());

    try (OutputStream stream =
        Strings.isNullOrEmpty(config.getOutputPath())
            ? System.out
            : new BufferedOutputStream(new FileOutputStream(config.getOutputPath()))) {
      KytheJavacAnalyzer analyzer =
          new KytheJavacAnalyzer(
              config,
              new StreamFactEmitter(stream),
              statistics == null ? NullStatisticsCollector.getInstance() : statistics,
              metadataLoaders);
      plugins.forEach(analyzer::registerPlugin);

      new JavacAnalysisDriver()
          .analyze(analyzer, desc.getCompilationUnit(), new FileDataCache(desc.getFileContents()));
    }

    if (statistics != null) {
      statistics.printStatistics(System.err);
    }
  }

  private static URL fileURL(String path) throws MalformedURLException {
    return new File(path).toURI().toURL();
  }

  private static class StandaloneConfig extends JavaIndexerConfig {
    @Parameter(description = "<compilation to analyze>", required = true)
    private List<String> compilation = new ArrayList<>();

    @Parameter(
        names = "--load_plugin",
        description = "Load and execute each Kythe Plugin in the given .jar")
    private String pluginJar;

    @Parameter(
        names = "--print_vnames",
        description = "Print Kythe node VNames associated to each JCTree to stderr")
    private boolean printVNames;

    @Parameter(
        names = "--print_statistics",
        description = "Print final analyzer statistics to stderr")
    private boolean printStatistics;

    @Parameter(
        names = {"--index_pack", "-index_pack"},
        description = "Retrieve the specified compilation from the given index pack")
    private String indexPackRoot;

    @Parameter(
        names = "--ignore_empty_kindex",
        description = "Ignore empty .kindex files; exit successfully with no output")
    private boolean ignoreEmptyKIndex;

    @Parameter(
        names = {"--out", "-out"},
        description = "Write the entries to this file (or stdout if unspecified)")
    private String outputPath;

    public StandaloneConfig() {
      super("java-indexer");
    }

    public final String getPlugin() {
      return pluginJar;
    }

    public final boolean getPrintVNames() {
      return printVNames;
    }

    public final boolean getPrintStatistics() {
      return printStatistics;
    }

    public final String getIndexPackRoot() {
      return indexPackRoot;
    }

    public final boolean getIgnoreEmptyKIndex() {
      return ignoreEmptyKIndex;
    }

    public final String getOutputPath() {
      return outputPath;
    }

    public final List<String> getCompilation() {
      return compilation;
    }
  }
}
