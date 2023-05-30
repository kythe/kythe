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
import com.google.common.base.Splitter;
import com.google.common.base.StandardSystemProperty;
import com.google.common.collect.ImmutableList;
import com.google.common.collect.ImmutableSet;
import com.google.common.collect.Iterables;
import com.google.common.collect.Iterators;
import com.google.common.flogger.FluentLogger;
import com.google.devtools.kythe.proto.Analysis.CompilationUnit;
import com.google.devtools.kythe.proto.Java.JavaDetails;
import com.google.protobuf.Any;
import com.google.protobuf.InvalidProtocolBufferException;
import com.sun.tools.javac.api.JavacTool;
import com.sun.tools.javac.code.Source;
import com.sun.tools.javac.file.JavacFileManager;
import com.sun.tools.javac.main.Option;
import com.sun.tools.javac.util.Context;
import java.io.File;
import java.io.IOException;
import java.nio.charset.Charset;
import java.nio.file.DirectoryStream;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.Set;
import java.util.function.Consumer;
import javax.tools.OptionChecker;
import javax.tools.StandardJavaFileManager;
import javax.tools.StandardLocation;
import org.checkerframework.checker.nullness.qual.Nullable;

/**
 * A utility class for dealing with javac command-line options.
 *
 * <p>To make modifications to javac commandline arguments, use {@code ModifiableOptions.of(args)}.
 */
public class JavacOptionsUtils {
  private static final FluentLogger logger = FluentLogger.forEnclosingClass();

  private JavacOptionsUtils() {}

  private static final Path JAVA_HOME = Paths.get(StandardSystemProperty.JAVA_HOME.value());
  private static final ImmutableList<String> JRE_JARS =
      ImmutableList.of("rt.jar", "resources.jar", "jsse.jar", "jce.jar", "charsets.jar");
  private static final ImmutableList<String> JRE_PATHS;

  static {
    try {
      ImmutableList<String> paths = getBootClassPath8();
      if (paths.isEmpty()) {
        paths = getBootclassPath();
      }
      JRE_PATHS = paths;
    } catch (IOException ioe) {
      throw new IllegalStateException(ioe);
    }
  }

  /** Returns the JRE 8-style bootclasspath. */
  private static ImmutableList<String> getBootClassPath8() throws IOException {
    Path jreLib = JAVA_HOME.resolve("lib");
    ImmutableList.Builder<String> jars = ImmutableList.builder();
    Path extDir = jreLib.resolve("ext");
    if (Files.exists(extDir)) {
      try (DirectoryStream<Path> dirStream = Files.newDirectoryStream(extDir, "*.jar")) {
        for (Path extJar : dirStream) {
          jars.add(extJar.toString());
        }
      }
    }
    for (String jar : JRE_JARS) {
      Path path = jreLib.resolve(jar);
      if (Files.exists(path)) {
        jars.add(path.toString());
      }
    }
    return jars.build();
  }

  /** Returns the JRE 9-style bootclasspath. */
  private static ImmutableList<String> getBootclassPath() {
    StandardJavaFileManager fileManager =
        JavacTool.create().getStandardFileManager(null, null, null);

    ImmutableList.Builder<String> paths = ImmutableList.builder();
    for (File file : fileManager.getLocation(StandardLocation.PLATFORM_CLASS_PATH)) {
      paths.add(file.toString());
    }
    return paths.build();
  }

  private static final Splitter PATH_SPLITTER = Splitter.on(':').trimResults().omitEmptyStrings();
  private static final Joiner PATH_JOINER = Joiner.on(':').skipNulls();

  @FunctionalInterface
  private static interface OptionHandler {
    /** Returns an iterable with the consumed and accepted options from {options, remaining}. */
    Iterable<String> handleOption(String option, Iterator<String> remaining);
  }

  /** Utility for converting between {@link OptionChecker} and {@link OptionHandler}. */
  private static OptionHandler fromCheckers(final Iterable<OptionChecker> checkers) {
    return (arg, tail) -> {
      for (OptionChecker checker : checkers) {
        int supported = checker.isSupportedOption(arg);
        if (supported >= 0) {
          if (supported > 0 && arg.indexOf(':') == -1 && arg.indexOf('=') == -1) {
            return new ImmutableList.Builder<String>()
                .add(arg)
                .addAll(Iterators.limit(tail, supported))
                .build();
          }
          return ImmutableList.of(arg);
        }
      }
      return ImmutableList.of();
    };
  }

  /** Utility for matching directly specified {@link Option}s. */
  private static OptionHandler handleOpts(Iterable<Option> opts) {
    return (arg, tail) -> {
      for (Option opt : opts) {
        if (opt.matches(arg)) {
          if (opt.hasArg() && arg.indexOf(':') == -1 && arg.indexOf('=') == -1) {
            return new ImmutableList.Builder<String>()
                .add(arg)
                .addAll(Iterators.limit(tail, 1))
                .build();
          }
          return ImmutableList.of(arg);
        }
      }
      return ImmutableList.of();
    };
  }

  /** A useful container for modifying javac commandline arguments, in the style of a builder. */
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

    private static final ImmutableList<OptionChecker> SUPPORTED_OPTIONS =
        ImmutableList.of(JavacTool.create(), new JavacFileManager(new Context(), false, UTF_8));

    /** Removes unsupported javac compiler options. */
    public ModifiableOptions removeUnsupportedOptions() {
      removeOptions(ImmutableSet.of(Option.WERROR, Option.XLINT, Option.XLINT_CUSTOM));
      replaceOptions(fromCheckers(SUPPORTED_OPTIONS), true);
      return this;
    }

    /** Removes the given {@link Option}s (and their arguments) from the builder. */
    public ModifiableOptions removeOptions(final Set<Option> opts) {
      return replaceOptions(handleOpts(opts), false);
    }

    /** Keep only the matched options. */
    public ModifiableOptions keepOptions(final Set<Option> opts) {
      return replaceOptions(handleOpts(opts), true);
    }

    /** If present, replace the value with the specified one. */
    public ModifiableOptions replaceOptionValue(final Option opt, final String repl) {
      internal =
          handleOptions(
              (arg, tail) -> {
                if (opt.matches(arg)) {
                  tail.next();
                  return ImmutableList.of(arg, repl);
                }
                return ImmutableList.of(arg);
              });
      return this;
    }

    private ModifiableOptions replaceOptions(OptionHandler handler, boolean matched) {
      final List<String> replacements = new ArrayList<>(internal.size());
      Consumer<String> placer = (value) -> replacements.add(value);
      if (matched) {
        acceptOptions(handler, placer, x -> {});
      } else {
        acceptOptions(handler, x -> {}, placer);
      }
      internal = replacements;
      return this;
    }

    private ModifiableOptions acceptOptions(
        OptionHandler handler, final Consumer<String> matched, final Consumer<String> unmatched) {
      return acceptOptions(handler, matched, unmatched, x -> {});
    }

    private ModifiableOptions acceptOptions(
        OptionHandler handler,
        final Consumer<String> matched,
        final Consumer<String> unmatched,
        final Consumer<String> positionalMatched) {
      Iterator<String> args = internal.iterator();
      while (args.hasNext()) {
        String value = args.next();
        Iterable<String> match = handler.handleOption(value, args);
        if (Iterables.isEmpty(match)) {
          unmatched.accept(value);
        } else {
          Iterator<String> iter = match.iterator();
          matched.accept(iter.next());
          while (iter.hasNext()) {
            String positional = iter.next();
            matched.accept(positional);
            positionalMatched.accept(positional);
          }
        }
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
      ImmutableList<String> argumentPaths = this.removeArgumentPaths(option);
      ImmutableList<String> detailsPaths =
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
      ImmutableList.Builder<String> paths = ImmutableList.builder();
      List<String> replacements = new ArrayList<>(internal.size());
      Consumer<String> matched = (value) -> paths.addAll(PATH_SPLITTER.split(value));
      Consumer<String> unmatched = (value) -> replacements.add(value);
      acceptOptions(handleOpts(ImmutableList.of(option)), x -> {}, unmatched, matched);
      internal = replacements;
      return paths.build();
    }

    /**
     * Find any -source flags that specify a version of java that the JRE can't support and replace
     * them with the lowest version that the JRE does support.
     *
     * <p>There may be problems during analysis due to flags passed to javac or language features
     * that changed, but the alternative is to have the analysis fail immediately when we send the
     * flag to the underlying Java APIs to do the analysis.
     */
    public ModifiableOptions updateToMinimumSupportedSourceVersion() {
      ArrayList<String> unsupportedVersions = new ArrayList<>();
      ArrayList<String> supportedVersions = new ArrayList<>();
      List<String> replacements = new ArrayList<>(internal.size());
      Consumer<String> matched =
          (value) -> {
            Source v = Source.lookup(value);
            if (v == null) {
              logger.atWarning().log("Could not parse source version number: %s", value);
              // Don't mutate the flag if it can't be parsed.
              supportedVersions.add(value);
            } else if (v.compareTo(Source.MIN) < 0) {
              unsupportedVersions.add(value);
            } else {
              supportedVersions.add(value);
            }
          };
      Consumer<String> unmatched = replacements::add;
      acceptOptions(handleOpts(ImmutableList.of(Option.SOURCE)), x -> {}, unmatched, matched);
      internal = replacements;

      if (!supportedVersions.isEmpty() || !unsupportedVersions.isEmpty()) {
        if (supportedVersions.size() + unsupportedVersions.size() > 1) {
          logger.atWarning().log("More than one -source flag passed, only using the last value");
        }
        internal.add(Option.SOURCE.getPrimaryName());
        if (!supportedVersions.isEmpty()) {
          internal.add(Iterables.getLast(supportedVersions));
        } else if (!unsupportedVersions.isEmpty()) {
          internal.add(Source.MIN.name);
          // If we changed the source version, remove the target flag since the set of valid target
          // values depends on what source was set to. Since we are already changing the source
          // version, it shouldn't be any worse to change the explicit target version and instead
          // use the default.
          removeOptions(ImmutableSet.of(Option.TARGET));
          // TODO(salguarnieri) increment a counter here.
        }
      }

      return this;
    }

    /** Applies handler to the interal options and returns the result. */
    private List<String> handleOptions(OptionHandler handler) {
      List<String> result = new ArrayList<>(internal.size());
      Iterator<String> iter = internal.iterator();
      while (iter.hasNext()) {
        Iterables.addAll(result, handler.handleOption(iter.next(), iter));
      }
      return result;
    }
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
}
