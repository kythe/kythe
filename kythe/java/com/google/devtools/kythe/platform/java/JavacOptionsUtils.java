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
import com.google.common.collect.Lists;
import com.google.devtools.kythe.common.PathUtil;

import java.util.ArrayList;
import java.util.List;

import javax.annotation.Nullable;

/**
 * A utility class for dealing with javac command-line options.
 */
public class JavacOptionsUtils {

  private static final String[] JRE_JARS = new String[] {
    "lib/rt.jar", "lib/resources.jar", "lib/jsse.jar", "lib/jce.jar", "lib/charsets.jar"
  };

  /**
   * Extract the encoding flag from the list of javac options. If the flag is specified more than
   * once, returns the last copy, which matches javac's behavior.  If the flag is not specified,
   * returns null.
   */
  public static @Nullable String getEncodingOption(List<String> options) {
    int i = options.lastIndexOf("-encoding");
    return (i >= 0) ? options.get(i + 1) : null;
  }

  /** If there is no encoding set, make sure to set the default encoding.*/
  public static List<String> ensureEncodingSet(List<String> options, String defaultEncoding) {
    if (getEncodingOption(options) == null) {
      options.add("-encoding");
      options.add(defaultEncoding);
    }
    return options;
  }

  /** Remove the existing warning options, and do all instead.*/
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

  /** Append the classpath to the list of options.*/
  public static void appendJREJarsToClasspath(List<String> arguments) {
    List<String> paths = Lists.newArrayList();
    String javaHome = StandardSystemProperty.JAVA_HOME.value();
    for (String jreJar : JRE_JARS) {
      paths.add(PathUtil.join(javaHome, jreJar));
    }

    for (int i = 0; i < arguments.size(); i++) {
      if (arguments.get(i).equals("-cp")) {
        if (i+1 >= arguments.size()) {
          throw new IllegalArgumentException("Malformed -cp argument: " + arguments);
        }
        arguments.remove(i);
        for (String path : arguments.remove(i).split(":")) {
          paths.add(path);
        }
        break;
      }
    }

    arguments.add("-cp");
    arguments.add(Joiner.on(":").join(paths));
  }
}
