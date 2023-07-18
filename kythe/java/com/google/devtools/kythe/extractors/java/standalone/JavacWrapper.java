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

package com.google.devtools.kythe.extractors.java.standalone;

import static com.google.common.collect.ImmutableList.toImmutableList;

import com.google.common.collect.ImmutableList;
import com.google.common.collect.Lists;
import com.google.devtools.kythe.extractors.java.JavaCompilationUnitExtractor;
import com.google.devtools.kythe.extractors.shared.CompilationDescription;
import com.google.devtools.kythe.extractors.shared.EnvironmentUtils;
import com.sun.tools.javac.file.JavacFileManager;
import com.sun.tools.javac.main.Arguments;
import com.sun.tools.javac.main.Option;
import com.sun.tools.javac.util.Context;
import com.sun.tools.javac.util.Options;
import java.io.File;
import java.util.ArrayList;
import java.util.Collection;
import java.util.EnumSet;
import java.util.List;
import javax.tools.JavaFileObject;
import javax.tools.StandardLocation;

/**
 * A class that wraps javac to extract compilation information and write it to an index file.
 *
 * <p>This class works for Java 9 and later.
 */
public class JavacWrapper extends AbstractJavacWrapper {
  @Override
  protected Collection<CompilationDescription> processCompilation(
      String[] arguments, JavaCompilationUnitExtractor javaCompilationUnitExtractor)
      throws Exception {
    // Use javac's argument parser to get the list of source files
    Context context = new Context();

    JavacFileManager fileManager = new JavacFileManager(context, true, null);
    Arguments args = Arguments.instance(context);
    shims.initializeArguments(args, arguments);
    fileManager.handleOptions(args.getDeferredFileManagerOptions());
    Options options = Options.instance(context);

    ImmutableList<String> sources =
        args.getFileObjects().stream().map(JavaFileObject::getName).collect(toImmutableList());

    // Retrieve the list of class paths provided by the -classpath argument.
    List<String> classPaths = splitPaths(options.get(Option.CLASS_PATH));

    // Retrieve the list of source paths provided by the -sourcepath argument.
    List<String> sourcePaths = splitPaths(options.get(Option.SOURCE_PATH));

    // Retrieve the list of processor paths provided by the -processorpath argument.
    List<String> processorPaths = splitPaths(options.get(Option.PROCESSOR_PATH));

    // Retrieve the list of processors provided by the -processor argument.
    List<String> processors = splitCSV(options.get(Option.PROCESSOR));

    // Retrieve system path (the value of --system=, unless it equals 'none'
    String sysd = options.get(Option.SYSTEM);
    if (sysd != null && !"none".equals(sysd)) {
      javaCompilationUnitExtractor.useSystemDirectory(sysd);
    }

    EnumSet<Option> claimed =
        EnumSet.of(Option.CLASS_PATH, Option.SOURCE_PATH, Option.PROCESSOR_PATH, Option.PROCESSOR);

    List<String> completeOptions = new ArrayList<>();
    if (options.isSet(Option.RELEASE)) {
      // If --release is set, claim it and the associated -source/-target flags.
      completeOptions.add(Option.RELEASE.getPrimaryName());
      completeOptions.add(options.get(Option.RELEASE));
      claimed.add(Option.RELEASE);
      claimed.add(Option.SOURCE);
      claimed.add(Option.TARGET);
    }

    // Retrieve all other javac options.
    for (Option opt : Option.values()) {
      if (!claimed.contains(opt)) {
        String value = options.get(opt);
        if (value != null) {
          if (opt.getPrimaryName().endsWith(":")) {
            completeOptions.add(opt.getPrimaryName() + value);
          } else {
            completeOptions.add(opt.getPrimaryName());
            if (!value.equals(opt.getPrimaryName())) {
              completeOptions.add(value);
            }
          }
        }
      }
    }

    List<String> bootclasspath = new ArrayList<>();
    for (File bcp : fileManager.getLocation(StandardLocation.PLATFORM_CLASS_PATH)) {
      bootclasspath.add(bcp.toString());
    }

    // Get the output directory for this target.
    String outputDirectory = options.get(Option.D);
    if (outputDirectory == null) {
      outputDirectory = ".";
    }

    int batchSize = readSourcesBatchSize().orElse(sources.size());
    List<CompilationDescription> results = new ArrayList<>();
    for (List<String> sourceBatch : Lists.partition(sources, batchSize)) {
      String analysisTarget =
          EnvironmentUtils.readEnvironmentVariable(
              "KYTHE_ANALYSIS_TARGET", createTargetFromSourceFiles(sourceBatch));

      results.add(
          javaCompilationUnitExtractor.extract(
              analysisTarget,
              sourceBatch,
              classPaths,
              bootclasspath,
              sourcePaths,
              processorPaths,
              processors,
              completeOptions,
              outputDirectory));
    }
    return results;
  }

  @Override
  protected void passThrough(String[] args) throws Exception {
    com.sun.tools.javac.Main.main(args);
  }

  public static void main(String[] args) {
    new JavacWrapper().process(args);
  }
}
