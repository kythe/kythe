/*
 * Copyright 2016 Google Inc. All rights reserved.
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

package com.google.devtools.kythe.analyzers.bazel;

import static com.google.common.base.Preconditions.checkArgument;

import com.google.common.base.Strings;
import com.google.devtools.build.lib.events.Location;
import com.google.devtools.build.lib.vfs.FileSystem;
import com.google.devtools.build.lib.vfs.Path;
import com.google.devtools.kythe.util.KytheURI;
import java.io.File;
import java.util.ArrayList;

/**
 * A set of methods for generating tickets in the BUILD analyzer. Allows handling preprocessed vs.
 * real BUILD files, by customizing tickets with configurable prefixes.
 */
public class Tickets {
  private static final String RUNFILES = "/runfiles/";

  // A constant string defining the language for build files.
  static final String BUILD_FILE_LANGUAGE = "bazel";

  // A constant int defining the maximum path length.
  static final int MAX_PATH_LENGTH = 1000;

  // E.g., "kythe://<corpus>?path=javatests/foo/BUILD?lang=bazel".
  private final String buildFileUri;

  // E.g., "javatests/foo".
  private final String packageName;

  // Package name, derived from {@link #physicalRelativeBuildFilePath}.
  // Prefixes (locationUriPrefix, packageNamePrefix, ...) are not taken into account.
  // E.g., "javatests/foo".
  private final String basePackageName;

  // Path to physical BUILD file, relative to the corpus.
  // E.g., "javatests/foo/BUILD".
  private final String physicalRelativeBuildFilePath;

  // A bazel Path to the BUILD file.
  private final Path buildFilePath;

  // A bazel path to the root of the corpus, e.g., /src/cloud/user/client.
  private final Path corpusRoot;

  // The label for the corpus which tickets will be generated for.
  private final String corpusLabel;

  // The file system to be utilized by this class.
  private FileSystem fileSystem;

  /**
   * @param fileSystem
   * @param physicalRelativeBuildFilePath path to the real, un-preprocessed BUILD file. E.g.,
   *     "javatests/foo/BUILD".
   * @param locationUriPrefix Tickets will be e.g.,
   *     kythe://<corpus>/`locationUriPrefix`?path=/foo/Foo.java
   * @param packageNamePrefix build: tickets will be e.g., build:'packageNamePrefix'/foo:Foo
   */
  public Tickets(
      FileSystem fileSystem,
      String corpusRoot,
      String physicalRelativeBuildFilePath,
      String locationUriPrefix,
      String packageNamePrefix,
      String corpusLabel) {
    this.fileSystem = fileSystem;
    this.corpusLabel = corpusLabel;
    this.physicalRelativeBuildFilePath = physicalRelativeBuildFilePath;
    this.corpusRoot = corpusRoot != null ? this.fileSystem.getPath(corpusRoot) : null;

    // Attempt to parse the build file path.
    if (this.corpusRoot != null) {
      this.buildFilePath = this.corpusRoot.getRelative(this.physicalRelativeBuildFilePath);
    } else {
      this.buildFilePath = this.fileSystem.getPath(this.physicalRelativeBuildFilePath);
    }

    this.basePackageName = this.buildFilePath.getParentDirectory().getBaseName();

    if (!Strings.isNullOrEmpty(packageNamePrefix)) {
      this.packageName = joinTicketPaths(packageNamePrefix, basePackageName);
    } else {
      this.packageName = basePackageName;
    }

    this.buildFileUri =
        this.fileUri(
                joinTicketPaths(
                    locationUriPrefix, this.basePackageName, this.buildFilePath.getBaseName()))
            .toString();
  }

  /** @return The corpus root. */
  public Path getCorpusRoot() {
    return this.corpusRoot;
  }

  /** @return The corpus label for ticket generator. */
  public String getCorpusLabel() {
    return this.corpusLabel;
  }

  /**
   * @return package name, as derived from the BUILD file. Takes `packageNamePrefix` into account.
   */
  public String getPackageName() {
    return packageName;
  }

  /** {@see #basePackageName} */
  public String getBasePackageName() {
    return basePackageName;
  }

  public Path getBuildFilePath() {
    return buildFilePath;
  }

  /**
   * Returns "build:" ticket (rule, target, etc.) for an entity defined in the currently processed
   * BUILD file. `packageNamePrefix` is used in the result.
   *
   * @param ruleName e.g., "Bar".
   * @return e.g., "build:foo/Foo:bar".
   */
  public String localRule(String ruleName) {
    checkArgument(ruleName.indexOf(":") <= 0, "ruleName must be a 'local' target (%s)", ruleName);
    return String.format("build:%s", localRuleDisplayName(ruleName));
  }

  /**
   * Returns a ticket for the named rule.
   *
   * @param packageName e.g., "devtools/kythe/lang/foo"
   * @param ruleName e.g., "Bar"
   * @return if `packageName` is the same as the package derived from
   *     `physicalRelativeBuildFilePath`, returns rule(ruleName). Note: this takes
   *     `packageNamePrefix` into account. Otherwise, returns build:packageName:ruleName.
   */
  public String rule(String packageName, String ruleName) {
    return basePackageName.equals(packageName)
        ? localRule(ruleName)
        : String.format("build:%s:%s", packageName, ruleName);
  }

  /**
   * Returns a ticket for an exported variable. Exported variables are variables that are visible
   * from other files (i.e. a top-level symbol in a .bzl file).
   */
  public String exportedVariable(String bzlFile, String name) {
    return String.format("build:%s:%s", bzlFile, name);
  }

  /**
   * Attempts to parse a string as a BUILD target. Returns null if it can't. Returns a ticket to a
   * BUILD artifact. For non-global targets, uses `packageNamePrefix` in the result. `value` must
   * either be a global target (//foo) or have a colon (:). Examples: 1. "//foo:bar" ->
   * "build:foo:bar" 2. ":bar" -> "build:packageNamePrefix/current_BUILD_path:bar" 3.
   * "//example/d/foo:bar/baz.bing" -> "build:example/d/foo:bar/baz.bing" 4. "foobar" -> NULL
   *
   * @param value The BUILD string to parse for embedded references
   */
  public String tryParseRule(String value) {
    String result = tryParseRuleDisplayName(value);
    return result == null ? null : absoluteBuildTicket(result);
  }

  /**
   * Similar to {@link #tryParseRule(String)}, but returns a display name, instead of a ticket.
   * Prefixes are used.
   */
  public String tryParseRuleDisplayName(String value) {
    if (value.startsWith("//") || (value.indexOf(':') >= 0 && value.indexOf(' ') < 0)) {
      return LabelUtil.canonicalize(value, packageName);
    }
    return null;
  }

  /**
   * If `value` represents an existing file, returns a path to it. Otherwise, null.
   *
   * <p>`value` is taken relative to the BUILD file.
   */
  public Path tryParseFilename(String value) {
    System.out.println("tryParseFilename(): value: " + value);
    // Do not try to check for file existence on strings that obviously are not paths.
    if (value.length() > MAX_PATH_LENGTH) {
      return null;
    }

    Path f = buildFilePath.getParentDirectory().getRelative(value);
    System.out.println("tryParseFilename(): f.getPathString(): " + f.getPathString());
    return f.isFile() ? f : null;
  }

  public String localRuleDisplayName(String name) {
    return String.format("%s:%s", packageName, name);
  }

  /**
   * @return true iff `filePath` points to the processed BUILD file.
   * @deprecated Use {@link #isBuildFile(Path)}.
   */
  @Deprecated
  public boolean isBuildFile(String filePath) {
    return this.buildFileUri != null
        && (filePath.equals(physicalRelativeBuildFilePath)
            || filePath.equals(this.buildFilePath.getPathString()));
  }

  /** @return true iff `filePath` points to the processed BUILD file. */
  public boolean isBuildFile(Path file) {
    return file.equals(buildFilePath);
  }

  /**
   * @param filePath
   * @return Kythe-URI path to `filePath`, e.g., kythe://<corpus>?path=foo/bar.h
   * @deprecated Use {@link #fileUri(Path)}.
   */
  @Deprecated
  public String fileUri(String filePath) {

    if (isBuildFile(filePath)) {
      return buildFileUri;
    }
    // Special case hack: In tests, everything is nested inside a runfiles
    // directory:  Get rid of that.  It doesn't occur in production, though.
    int runfiles = filePath.indexOf(RUNFILES);
    if (runfiles >= 0) {
      filePath = filePath.substring(runfiles);
    }

    // Attempt to generate a signature hash for the file
    Path path = this.corpusRoot != null ? this.corpusRoot.getRelative(filePath) : null;
    if (path == null) {
      path = this.fileSystem.getPath(filePath);
    }

    return new KytheURI(
            null,
            this.getCorpusLabel(),
            this.getCorpusRoot().getPathString(),
            path.getPathString(),
            BUILD_FILE_LANGUAGE)
        .toString();
  }

  /** @return KytheURI path to `filePath`, e.g., kythe://<corpus>?path=/foo/bar.h */
  public String fileUri(Path file) {
    if (isBuildFile(file)) {
      return this.buildFileUri;
    }

    return new KytheURI(
            null,
            this.getCorpusLabel(),
            (this.getCorpusRoot() != null ? this.getCorpusRoot().getPathString() : null),
            file.getPathString(),
            BUILD_FILE_LANGUAGE)
        .toString();
  }

  /** Transforms Path(/home/user/client/javatests/foo/Bar.java) to "javatests/foo/Bar.java". */
  public String relativeToCorpusRoot(Path file) {
    return file.relativeTo(this.getCorpusRoot()).getPathString();
  }

  /** @return The build file's Kythe URI. */
  public String buildFileUri() {
    return this.buildFileUri.toString();
  }

  /**
   * Creates a BUILD ticket for an entity based on its position in the processed BUILD file.
   *
   * @param displayName display-name of the referenced entity, e.g., "foo:Bar".
   */
  public String positional(String displayName, Location location) {
    return String.format(
        "build:%s@%s[%d:%d]",
        displayName, buildFileUri, location.getStartOffset(), location.getEndOffset());
  }

  /**
   * @param name e.g., "java_library".
   * @return e.g., "build:java_library".
   */
  public static String external(String name) {
    return absoluteBuildTicket(name);
  }

  /**
   * Returns a BUILD ticket for s, not appending any prefixes.
   *
   * @return build:s
   */
  private static String absoluteBuildTicket(String s) {
    return "build:" + s;
  }

  /**
   * Returns a ticket for accessing a BUILD-language function and its attribute.
   *
   * @param functionName e.g., "java_library"
   * @param attribute e.g., "srcs".
   * @return e.g., "build:java_library:srcs".
   */
  public String attribute(String functionName, String attribute) {
    return external(attributeDisplayName(functionName, attribute));
  }

  public String attributeDisplayName(String functionName, String attribute) {
    return functionName + ":" + attribute;
  }

  /**
   * Joins the specified paths.
   *
   * @param paths The paths to be joined.
   * @return A join string composed of the specified paths.
   */
  private static String joinTicketPaths(String... paths) {
    ArrayList<String> pathsWithValue = new ArrayList<>();
    for (String path : paths) {
      if (!Strings.isNullOrEmpty(path)) {
        pathsWithValue.add(path);
      }
    }

    return String.join(File.separator, pathsWithValue);
  }
}
