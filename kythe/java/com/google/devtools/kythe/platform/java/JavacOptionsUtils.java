/*
 * Copyright 2014 Google Inc. All rights reserved.
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

import com.google.common.base.Joiner;
import com.google.common.base.StandardSystemProperty;
import com.sun.tools.javac.main.Option;
import java.nio.charset.Charset;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.EnumSet;
import java.util.List;
import java.util.Set;
import javax.annotation.Nullable;

/** A utility class for dealing with javac command-line options. */
public class JavacOptionsUtils {

  private static final String[] JRE_JARS =
      new String[] {
        "lib/rt.jar", "lib/resources.jar", "lib/jsse.jar", "lib/jce.jar", "lib/charsets.jar"
      };

  /** Returns a new list of options with only javac compiler supported options. */
  public static List<String> removeUnsupportedOptions(List<String> rawOptions) {
    Set<Option> supportedOptions = EnumSet.allOf(Option.class);
    List<String> options = new ArrayList<>();
    for (int i = 0; i < rawOptions.size(); i++) {
      String opt = rawOptions.get(i);
      for (Option o : supportedOptions) {
        if (o.matches(opt)) {
          options.add(opt);
          if (o.hasArg()) {
            options.add(rawOptions.get(++i));
          }
          break;
        }
      }
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
    int i = options.lastIndexOf(Option.ENCODING.getText());
    return (i >= 0) ? Charset.forName(options.get(i + 1)) : null;
  }

  /** If there is no encoding set, make sure to set the default encoding. */
  public static List<String> ensureEncodingSet(List<String> options, Charset defaultEncoding) {
    if (getEncodingOption(options) == null) {
      options.add(Option.ENCODING.getText());
      options.add(defaultEncoding.name());
    }
    return options;
  }

  /** Remove the existing warning options, and do all instead. */
  public static List<String> useAllWarnings(List<String> options) {
    List<String> result = new ArrayList<>();
    for (String option : options) {
      if (!option.startsWith("-Xlint")) {
        result.add(option);
      }
    }
    result.add("-Xlint:all");
    return result;
  }

  /** Append the classpath to the list of options. */
  public static void appendJREJarsToClasspath(List<String> arguments) {
    List<String> paths = new ArrayList<>();
    Path javaHome = Paths.get(StandardSystemProperty.JAVA_HOME.value());
    for (String jreJar : JRE_JARS) {
      paths.add(javaHome.resolve(jreJar).toString());
    }

    for (int i = 0; i < arguments.size(); i++) {
      if (Option.CP.matches(arguments.get(i))) {
        if (i + 1 >= arguments.size()) {
          throw new IllegalArgumentException("Malformed -cp argument: " + arguments);
        }
        arguments.remove(i);
        for (String path : arguments.remove(i).split(":")) {
          paths.add(path);
        }
        break;
      }
    }

    arguments.add(Option.CP.getText());
    arguments.add(Joiner.on(":").join(paths));
  }
}
