/*
 * Copyright 2021 The Kythe Authors. All rights reserved.
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

package com.google.devtools.kythe.extractors.shared;

import com.google.common.base.Strings;
import com.google.common.flogger.FluentLogger;
import java.util.Optional;

/**
 * A class containing common utilities for uniform access to system properties and environment
 * variables.
 */
public class EnvironmentUtils {
  private static final FluentLogger logger = FluentLogger.forEnclosingClass();

  private EnvironmentUtils() {}

  /** Returns the value of the specified variable; exits if not set. */
  public static String readEnvironmentVariable(String variableName) {
    return readEnvironmentVariable(variableName, null);
  }

  /**
   * Returns the value of the specified variable; uses default if not set, exits if default also
   * empty.
   */
  public static String readEnvironmentVariable(String variableName, String defaultValue) {
    return tryReadEnvironmentVariable(variableName).orElseGet(() -> {
      if (Strings.isNullOrEmpty(defaultValue)) {
        System.err.printf("Missing environment variable: %s%n", variableName);
        System.exit(1);
      }
      return defaultValue;
    });
  }

  /** Returns the value of the specified variable, if set, otherwise <code>empty()</code>. */
  public static Optional<String> tryReadEnvironmentVariable(String variableName) {
    // First see if we have a system property.
    String result = System.getProperty(variableName);
    if (Strings.isNullOrEmpty(result)) {
      // Fall back to the environment variable.
      result = System.getenv(variableName);
    }
    if (Strings.isNullOrEmpty(result)) {
      return Optional.empty();
    }
    return Optional.of(result);
  }

  /** Returns the value of the default corpus. */
  public static String defaultCorpus() {
    return tryReadEnvironmentVariable("KYTHE_CORPUS").orElseGet(() -> {
      logger.atWarning().log("KYTHE_CORPUS not set, using suboptimal default of 'kythe'");
      return DEFAULT_CORPUS;
    });
  }

  public static final String DEFAULT_CORPUS = "kythe";
}
