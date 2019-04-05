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

import com.google.common.base.Joiner;
import com.google.common.base.Predicates;
import com.google.common.base.Splitter;
import com.google.common.base.StandardSystemProperty;
import com.google.common.collect.ImmutableList;
import com.google.common.collect.Iterators;
import com.google.common.collect.PeekingIterator;
import com.google.devtools.kythe.proto.Analysis.CompilationUnit;
import com.google.devtools.kythe.proto.Java.JavaDetails;
import com.google.protobuf.Any;
import com.google.protobuf.InvalidProtocolBufferException;
import com.sun.tools.javac.api.JavacTool;
import com.sun.tools.javac.file.JavacFileManager;
import com.sun.tools.javac.main.Option;
import com.sun.tools.javac.util.Context;
import java.nio.charset.Charset;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.List;
import java.util.Set;
import javax.annotation.Nullable;
import javax.tools.OptionChecker;

/** A utility class for dealing with javac command-line options. */
public class JavacOptionsUtils {

  private static final ImmutableList<String> JRE_JARS =
      ImmutableList.of(
          "lib/rt.jar", "lib/resources.jar", "lib/jsse.jar", "lib/jce.jar", "lib/charsets.jar");
  private static final ImmutableList<String> JRE_PATHS;

  static {
    ImmutableList.Builder<String> paths = ImmutableList.builder();
    Path javaHome = Paths.get(StandardSystemProperty.JAVA_HOME.value());
    for (String jreJar : JRE_JARS) {
      paths.add(javaHome.resolve(jreJar).toString());
    }
    JRE_PATHS = paths.build();
  }

  private static final Splitter PATH_SPLITTER = Splitter.on(':').trimResults().omitEmptyStrings();
  private static final Joiner PATH_JOINER = Joiner.on(':').skipNulls();
  /** Returns a new list of options with only javac compiler supported options. */
  public static List<String> removeUnsupportedOptions(Iterable<String> rawOptions) {
    List<String> options = new ArrayList<>();
    ImmutableList<OptionChecker> optionCheckers =
        ImmutableList.of(JavacTool.create(), new JavacFileManager(new Context(), false, UTF_8));
    PeekingIterator<String> it =
        Iterators.peekingIterator(
            Iterators.filter(
                rawOptions.iterator(),
                Predicates.not(o -> o.startsWith("-Xlint") || o.startsWith("-Werror"))));
    outer:
    while (it.hasNext()) {
      for (OptionChecker optionChecker : optionCheckers) {
        int arity = optionChecker.isSupportedOption(it.peek());
        if (arity > 0 && it.peek().indexOf(':') != -1) {
          // For "conjoined" flags (e.g., -flag:arg) we want to consume just command-line option.
          arity = 0;
        }
        if (arity != -1) {
          Iterators.addAll(options, Iterators.limit(it, arity + 1));
          continue outer;
        }
      }
      it.next();
    }
    return options;
  }

  /** Removes the given {@link Option}s (and their arguments) from {@code options}. */
  public static List<String> removeOptions(List<String> options, Set<Option> opts) {
    for (int i = 0; i < options.size(); ) {
      String opt = options.get(i);
      boolean matched = false;
      for (Option o : opts) {
        if (o.matches(opt)) {
          matched = true;
          options.subList(i, i + (o.hasArg() ? 2 : 1)).clear();
          break;
        }
      }
      if (!matched) {
        i++;
      }
    }
    return options;
  }

  /**
   * Extract the encoding flag from the list of javac options. If the flag is specified more than
   * once, returns the last copy, which matches javac's behavior. If the flag is not specified,
   * returns null.
   */
  public static @Nullable Charset getEncodingOption(List<String> options) {
    int i = options.lastIndexOf("-encoding");
    return (i >= 0) ? Charset.forName(options.get(i + 1)) : null;
  }

  /** If there is no encoding set, make sure to set the default encoding. */
  public static List<String> ensureEncodingSet(List<String> options, Charset defaultEncoding) {
    if (getEncodingOption(options) == null) {
      options.add("-encoding");
      options.add(defaultEncoding.name());
    }
    return options;
  }

  /**
   * Update the command line arguments based on {@code compilationUnit}'s {@code JavaDetails} (if
   * present) and the JRE jars.
   *
   * @param arguments the command line arguments to be updated
   * @param compilationUnit the compilation unit in which to look for {@code JavaDetails}
   */
  public static void updateArgumentsWithJavaOptions(
      List<String> arguments, CompilationUnit compilationUnit) {
    JavaDetails javaDetails = null;

    for (Any detail : compilationUnit.getDetailsList()) {
      if (detail.is(JavaDetails.class)) {
        try {
          javaDetails = detail.unpack(JavaDetails.class);
        } catch (InvalidProtocolBufferException e) {
          throw new IllegalArgumentException("Error in extracting JavaDetails", e);
        }
        break; // assume that <= 1 of these is a JavaDetails
      }
    }
    // Use JavaDetails if present, otherwise use the default (system) properties.
    if (javaDetails != null) {
      updateArgumentsFromJavaDetails(arguments, javaDetails);
    } else {
      updateArgumentsFromSystemProperties(arguments);
    }
  }

  private static void updateArgumentsFromJavaDetails(
      List<String> arguments, JavaDetails javaDetails) {
    updatePathArguments(arguments, Option.CLASS_PATH, javaDetails.getClasspathList());
    updatePathArguments(arguments, Option.SOURCE_PATH, javaDetails.getSourcepathList());
    updatePathArguments(arguments, Option.BOOT_CLASS_PATH, javaDetails.getBootclasspathList());

    // just append the Java options to the arguments, separated by spaces
    // (assumes that this is a series of "option" followed by "value" entries)
    if (!javaDetails.getExtraJavacoptsList().isEmpty()) {
      arguments.add(Joiner.on(' ').join(javaDetails.getExtraJavacoptsList()));
    }
  }

  private static void updateArgumentsFromSystemProperties(List<String> arguments) {
    ImmutableList.Builder<String> paths = ImmutableList.builder();
    paths.addAll(extractArgumentPaths(arguments, Option.BOOT_CLASS_PATH));
    paths.addAll(JRE_PATHS);

    arguments.add(Option.BOOT_CLASS_PATH.getPrimaryName());
    arguments.add(PATH_JOINER.join(paths.build()));
  }

  private static void updatePathArguments(
      List<String> arguments, Option option, List<String> pathList) {
    ImmutableList.Builder<String> pathEntries = ImmutableList.builder();

    // We need to call extractArgumentPaths() even if we don't use the return value because it
    // strips out any existing command-line-specified values from 'arguments'.
    List<String> argumentPaths = extractArgumentPaths(arguments, option);
    List<String> detailsPaths = extractDetailsPaths(pathList);

    // Use paths specified in the JavaDetails' pathList if present; otherwise, use those
    // specified on the command line.
    if (!detailsPaths.isEmpty()) {
      pathEntries.addAll(detailsPaths);
    } else {
      pathEntries.addAll(argumentPaths);
    }

    // If this is for the bootclasspath, append the paths to the system jars.
    if (Option.BOOT_CLASS_PATH.equals(option)) {
      pathEntries.addAll(JRE_PATHS);
    }

    ImmutableList<String> paths = pathEntries.build();

    if (!paths.isEmpty()) {
      arguments.add(option.getPrimaryName());
      arguments.add(PATH_JOINER.join(paths));
    }
  }

  /**
   * Identify the paths associated with the specified option, remove them from the arguments list,
   * and return them.
   */
  private static ImmutableList<String> extractArgumentPaths(List<String> arguments, Option option) {
    ImmutableList.Builder<String> paths = ImmutableList.builder();
    for (int i = 0; i < arguments.size(); i++) {
      if (option.matches(arguments.get(i))) {
        if (i + 1 >= arguments.size()) {
          throw new IllegalArgumentException(
              String.format("Malformed %s argument: %s", option.getPrimaryName(), arguments));
        }
        arguments.remove(i);
        paths.addAll(PATH_SPLITTER.split(arguments.remove(i)));
      }
    }
    return paths.build();
  }

  private static List<String> extractDetailsPaths(List<String> pathList) {
    ImmutableList.Builder<String> paths = ImmutableList.builder();
    for (String entry : pathList) {
      paths.addAll(PATH_SPLITTER.split(entry));
    }
    return paths.build();
  }
}
