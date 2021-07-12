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
import com.google.common.collect.ImmutableList;
import com.google.common.flogger.FluentLogger;
import com.google.devtools.kythe.analyzers.base.FactEmitter;
import com.google.devtools.kythe.analyzers.base.StreamFactEmitter;
import com.google.devtools.kythe.extractors.shared.CompilationDescription;
import com.google.devtools.kythe.extractors.shared.IndexInfoUtils;
import com.google.devtools.kythe.platform.java.JavacAnalysisDriver;
import com.google.devtools.kythe.platform.kzip.KZip;
import com.google.devtools.kythe.platform.kzip.KZipException;
import com.google.devtools.kythe.platform.kzip.KZipReader;
import com.google.devtools.kythe.platform.shared.AnalysisException;
import com.google.devtools.kythe.platform.shared.FileDataCache;
import com.google.devtools.kythe.platform.shared.KytheMetadataLoader;
import com.google.devtools.kythe.platform.shared.MemoryStatisticsCollector;
import com.google.devtools.kythe.platform.shared.MetadataLoaders;
import com.google.devtools.kythe.platform.shared.NullStatisticsCollector;
import com.google.devtools.kythe.platform.shared.ProtobufMetadataLoader;
import com.google.devtools.kythe.platform.shared.StatisticsCollector;
import com.google.devtools.kythe.proto.Analysis.IndexedCompilation;
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
import java.nio.file.FileSystems;
import java.nio.file.Path;
import java.util.ArrayList;
import java.util.List;
import java.util.ServiceLoader;
import java.util.concurrent.TimeUnit;
import java.util.function.Supplier;

/**
 * Binary to run Kythe's Java indexer over one of more .kzip/.kindex files, emitting entries to a
 * file or STDOUT.
 */
public class JavaIndexer {
  private JavaIndexer() {}

  private static final FluentLogger logger = FluentLogger.forEnclosingClass();

  public static void main(String[] args) throws AnalysisException, IOException {
    JsonUtil.usingTypeRegistry(JsonUtil.JSON_TYPE_REGISTRY);

    StandaloneConfig config = new StandaloneConfig();
    config.parseCommandLine(args);

    List<Supplier<Plugin>> plugins = new ArrayList<>();
    if (!Strings.isNullOrEmpty(config.getPlugin())) {
      URLClassLoader classLoader = new URLClassLoader(new URL[] {fileURL(config.getPlugin())});
      for (Plugin plugin : ServiceLoader.load(Plugin.class, classLoader)) {
        final Class<? extends Plugin> clazz = plugin.getClass();
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

    try (OutputStream stream =
            Strings.isNullOrEmpty(config.getOutputPath())
                ? System.out
                : new BufferedOutputStream(new FileOutputStream(config.getOutputPath()));
        FactEmitter emitter = new StreamFactEmitter(stream)) {

      // java_indexer compilation-file+
      for (String compilationPath : config.getCompilation()) {
        if (compilationPath.endsWith(IndexInfoUtils.KZIP_FILE_EXT)) {
          // java_indexer kzip-file
          boolean foundCompilation = false;
          try {
            KZip.Reader reader = new KZipReader(new File(compilationPath));
            for (IndexedCompilation indexedCompilation : reader.scan()) {
              foundCompilation = true;
              CompilationDescription desc =
                  IndexInfoUtils.indexedCompilationToCompilationDescription(
                      indexedCompilation, reader);
              analyzeCompilation(config, plugins, statistics, desc, emitter);
            }
          } catch (KZipException e) {
            throw new IllegalArgumentException("Unable to read kzip", e);
          }
          if (!config.getIgnoreEmptyKZip() && !foundCompilation) {
            throw new IllegalArgumentException(
                "given empty .kzip file \"" + compilationPath + "\"; try --ignore_empty_kzip");
          }
        } else {
          // java_indexer kindex-file
          logger.atWarning().atMostEvery(1, TimeUnit.DAYS).log(
              ".kindex files are deprecated; "
                  + "use kzip files instead: https://kythe.io/docs/kythe-kzip.html");
          try {
            CompilationDescription desc = IndexInfoUtils.readKindexInfoFromFile(compilationPath);
            analyzeCompilation(config, plugins, statistics, desc, emitter);
          } catch (EOFException e) {
            if (config.getIgnoreEmptyKIndex()) {
              return;
            }
            throw new IllegalArgumentException(
                "given empty .kindex file \"" + compilationPath + "\"; try --ignore_empty_kindex",
                e);
          }
        }
      }
    }

    if (statistics != null) {
      statistics.printStatistics(System.err);
    }
  }

  private static void analyzeCompilation(
      StandaloneConfig config,
      List<Supplier<Plugin>> plugins,
      StatisticsCollector statistics,
      CompilationDescription desc,
      FactEmitter emitter)
      throws AnalysisException {
    if (!desc.getFileContents().iterator().hasNext()) {
      // Skip empty compilations.
      return;
    }

    MetadataLoaders metadataLoaders = new MetadataLoaders();
    metadataLoaders.addLoader(
        new ProtobufMetadataLoader(desc.getCompilationUnit(), config.getDefaultMetadataCorpus()));
    metadataLoaders.addLoader(new KytheMetadataLoader());

    KytheJavacAnalyzer analyzer =
        new KytheJavacAnalyzer(
            config,
            emitter,
            statistics == null ? NullStatisticsCollector.getInstance() : statistics,
            metadataLoaders);
    plugins.forEach(analyzer::registerPlugin);

    Path tempPath =
        Strings.isNullOrEmpty(config.getTemporaryDirectory())
            ? null
            : FileSystems.getDefault().getPath(config.getTemporaryDirectory());
    new JavacAnalysisDriver(ImmutableList.of(), tempPath)
        .analyze(analyzer, desc.getCompilationUnit(), new FileDataCache(desc.getFileContents()));
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
        names = "--ignore_empty_kindex",
        description = "Ignore empty .kindex files; exit successfully with no output")
    private boolean ignoreEmptyKIndex;

    @Parameter(
        names = "--ignore_empty_kzip",
        description = "Ignore empty .kzip files; exit successfully with no output")
    private boolean ignoreEmptyKZip;

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

    public final boolean getIgnoreEmptyKIndex() {
      return ignoreEmptyKIndex;
    }

    public final boolean getIgnoreEmptyKZip() {
      return ignoreEmptyKZip;
    }

    public final String getOutputPath() {
      return outputPath;
    }

    public final List<String> getCompilation() {
      return compilation;
    }
  }
}
