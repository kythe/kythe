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

import static com.google.common.collect.ImmutableList.toImmutableList;
import static java.nio.charset.StandardCharsets.UTF_8;

import com.google.common.base.Joiner;
import com.google.common.base.Predicates;
import com.google.common.base.Splitter;
import com.google.common.base.StandardSystemProperty;
import com.google.common.collect.ImmutableList;
import com.google.common.collect.Iterables;
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

/**
 * A utility class for dealing with javac command-line options.
 *
 * To make modifications to javac commandline arguments, use
 * {@code ModifiableOptions.of(args)}.
 */
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

  /**
   * Extract the encoding flag from the list of javac options. If the flag is specified more than
   * once, returns the last copy, which matches javac's behavior. If the flag is not specified,
   * returns null.
   */
  public static @Nullable Charset getEncodingOption(List<String> options) {
    int i = options.lastIndexOf("-encoding");
    return (i >= 0) ? Charset.forName(options.get(i + 1)) : null;
  }

  /** A useful container for modifying javac commandline arguements, in the style of a builder. */
  public static class ModifiableOptions {
    private List<String> internal = new ArrayList<>();

    private ModifiableOptions() {}

    public static ModifiableOptions of() {
      return of(ImmutableList.of());
    }

    public static ModifiableOptions of(Iterable<String> options) {
      ModifiableOptions o = new ModifiableOptions();
      Iterables.addAll(o.internal, options);
      return o;
    }

    public ImmutableList<String> build() {
      return ImmutableList.copyOf(internal);
    }

    public ModifiableOptions add(String option) {
      internal.add(option);
      return this;
    }

    /** Removes unsupported javac compiler options. */
    public ModifiableOptions removeUnsupportedOptions() {
      List<String> previous = internal;
      internal = new ArrayList<>();
      ImmutableList<OptionChecker> optionCheckers =
          ImmutableList.of(JavacTool.create(), new JavacFileManager(new Context(), false, UTF_8));
      PeekingIterator<String> it =
          Iterators.peekingIterator(
              Iterators.filter(
                  previous.iterator(),
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
            Iterators.addAll(internal, Iterators.limit(it, arity + 1));
            continue outer;
          }
        }
        it.next();
      }
      return this;
    }

    /** If there is no encoding set, make sure to set the default encoding. */
    public ModifiableOptions ensureEncodingSet(Charset defaultEncoding) {
      if (getEncodingOption(internal) == null) {
        internal.add("-encoding");
        internal.add(defaultEncoding.name());
      }
      return this;
    }

    /**
     * Update the command line arguments based on {@code compilationUnit}'s {@code JavaDetails} (if
     * present) and the JRE jars.
     *
     * @param compilationUnit the compilation unit in which to look for {@code JavaDetails}
     */
    public ModifiableOptions updateWithJavaOptions(CompilationUnit compilationUnit) {
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
        return updateFromJavaDetails(javaDetails);
      } else {
        return updateFromSystemProperties();
      }
    }

    private ModifiableOptions updateFromJavaDetails(JavaDetails javaDetails) {
      updatePathArguments(Option.CLASS_PATH, javaDetails.getClasspathList());
      updatePathArguments(Option.SOURCE_PATH, javaDetails.getSourcepathList());
      updatePathArguments(Option.BOOT_CLASS_PATH, javaDetails.getBootclasspathList());

      // just append the Java options to the arguments, separated by spaces
      // (assumes that this is a series of "option" followed by "value" entries)
      if (!javaDetails.getExtraJavacoptsList().isEmpty()) {
        internal.add(Joiner.on(' ').join(javaDetails.getExtraJavacoptsList()));
      }
      return this;
    }

    private ModifiableOptions updateFromSystemProperties() {
      ImmutableList.Builder<String> paths = ImmutableList.builder();
      ImmutableList<String> argPaths = this.removeArgumentPaths(Option.BOOT_CLASS_PATH);
      paths.addAll(argPaths);
      paths.addAll(JRE_PATHS);

      internal.add(Option.BOOT_CLASS_PATH.getPrimaryName());
      internal.add(PATH_JOINER.join(paths.build()));
      return this;
    }

    private ModifiableOptions updatePathArguments(Option option, List<String> pathList) {
      ImmutableList.Builder<String> pathEntries = ImmutableList.builder();

      // We need to call removeArgumentPaths() even if we don't use the return value because it
      // strips out any existing command-line-specified values from 'arguments'.
      List<String> argumentPaths = this.removeArgumentPaths(option);
      List<String> detailsPaths =
          pathList.stream()
              .flatMap(pL -> PATH_SPLITTER.splitToList(pL).stream())
              .collect(toImmutableList());

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
        internal.add(option.getPrimaryName());
        internal.add(PATH_JOINER.join(paths));
      }
      return this;
    }

    /**
     * Identify the paths associated with the specified option, remove them from the builder, and
     * return them.
     */
    private ImmutableList<String> removeArgumentPaths(Option option) {
      List<String> original = internal;
      internal = new ArrayList<>(original.size());
      ImmutableList.Builder<String> paths = ImmutableList.builder();
      for (int i = 0; i < original.size(); i++) {
        if (option.matches(original.get(i))) {
          if (i + 1 >= original.size()) {
            throw new IllegalArgumentException(
                String.format("Malformed %s argument: %s", option.getPrimaryName(), original));
          }
          i++;
          paths.addAll(PATH_SPLITTER.split(original.get(i)));
        } else {
          internal.add(original.get(i));
        }
      }
      return paths.build();
    }

    /** Removes the given {@link Option}s (and their arguments) from the builder. */
    public ModifiableOptions removeOptions(Set<Option> opts) {
      List<String> original = internal;
      internal = new ArrayList<>(original.size());
      for (int i = 0; i < original.size(); i++) {
        String opt = original.get(i);
        boolean matched = false;
        for (Option o : opts) {
          if (o.matches(opt)) {
            matched = true;
            if (o.hasArg()) {
              // Skip the argument too.
              i++;
            }
            break;
          }
        }
        if (!matched) {
          internal.add(opt);
        }
      }
      return this;
    }
  }
}
