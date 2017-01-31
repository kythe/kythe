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

import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * Utility methods for manipulation and understanding of bazel build labels and files. See
 * LabelUtilTest for examples.
 */
public class LabelUtil {

  /** Fully qualified bazel build labels start with double slashes. */
  private static final String LABEL_PREFIX = "//";

  /**
   * In labels bazel build packages and rules/files are separated with a colon. Unqualified names
   * may also start with a colon.
   */
  private static final String RULE_SEPARATOR = ":";

  /** bazel uses forward slashes for path part separators. */
  private static final String SLASH = "/";

  /**
   * Build labels can be just about any vaguely path-like format with a colon thrown in somewhere.
   * This pattern handles every working visible BUILD label in bazel, as of its check-in.
   *
   * <p>Example build labels:
   *
   * <ul>
   *   <li><code>//base</code>
   *   <li><code>//base:base</code>
   *   <li><code>:base</code>
   *   <li><code>base</code>
   *   <li><code>//gws/base:release</code>
   *   <li><code>//util/gtl:lazy_static_ptr</code>
   *   <li><code>//example/d/foo:bar/baz.bing+</code>
   * </ul>
   */
  private static final Pattern LABEL_PATTERN =
      Pattern.compile(
          "^((?://)(([a-zA-Z_.][a-zA-Z0-9_.\\-]*/)*([a-zA-Z_.][a-zA-Z0-9_.\\-\\+]*)))?"
              + "(?::)?([a-zA-Z0-9_.\\-/\\+]+)?$");

  // The important regex capture groups for LABEL_PATTERN
  private static final int PACKAGE_GROUP = 2;
  private static final int PACKAGE_NAME_GROUP = 4;
  private static final int RULE_GROUP = 5;

  /**
   * Every rule name is stored in the full {@code package:rule} format, even for eponymous rules
   * (e.g. {@code foo/package:package}.
   *
   * @param label the label to canonicalize
   * @param packageName the name of the current package context
   * @return the canonical label or null if the label format isn't recognized
   */
  public static String canonicalize(String label, String packageName) {
    // Canonicalize the "/..." target to ":__subpackages__", since they are synonymous.
    label = label.replace("/...", ":__subpackages__");

    /**
     * TODO(craigbarber): This logic is duplicated in the bazel {@code Label} class. Once this class
     * and other bazel libs become available, this code should be re-factored to utilize the bazel
     * lib.
     */
    Matcher matcher = LABEL_PATTERN.matcher(label.trim());

    if (!matcher.matches()) {
      return null;
    }

    StringBuffer canonicalLabel = new StringBuffer();
    String packageGroup = matcher.group(PACKAGE_GROUP);
    if (packageGroup != null) {
      canonicalLabel.append(packageGroup);
    } else {
      canonicalLabel.append(packageName);
    }
    canonicalLabel.append(RULE_SEPARATOR);
    String rule = matcher.group(RULE_GROUP);
    if (rule != null) {
      canonicalLabel.append(rule);
    } else {
      canonicalLabel.append(matcher.group(PACKAGE_NAME_GROUP));
    }
    return canonicalLabel.toString();
  }

  /** Strips any label prefix and replaces the rule separating colon with a slash. */
  public static String labelToFilePath(String label) {
    return label.replaceFirst(LABEL_PREFIX, "").replace(RULE_SEPARATOR, SLASH);
  }
}
