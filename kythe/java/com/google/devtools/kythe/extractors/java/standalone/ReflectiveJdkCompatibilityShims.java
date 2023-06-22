/*
 * Copyright 2022 The Kythe Authors. All rights reserved.
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

import com.google.auto.service.AutoService;
import com.sun.tools.javac.main.Arguments;
import com.sun.tools.javac.main.CommandLine;
import java.io.IOException;
import java.lang.reflect.InvocationTargetException;
import java.util.Arrays;
import java.util.List;
import java.util.Optional;

/** JdkCompatibilityShims implementation for JDK9-compatible releases. */
@AutoService(JdkCompatibilityShims.class)
public final class ReflectiveJdkCompatibilityShims implements JdkCompatibilityShims {
  private static final Runtime.Version minVersion = Runtime.Version.parse("9");

  public ReflectiveJdkCompatibilityShims() {}

  @Override
  public CompatibilityRange getCompatibleRange() {
    return new CompatibilityRange(minVersion, Optional.empty(), CompatibilityLevel.FALLBACK);
  }

  @Override
  @SuppressWarnings({"CheckedExceptionNotThrown", "unchecked"}) // Safe by specification.
  public List<String> parseCompilerArguments(String[] args) throws IOException {
    try {
      try {
        // JDK15+: the signature is
        //     List<String> parse(List<String> args)
        return (List<String>)
            CommandLine.class
                .getMethod("parse", List.class)
                .invoke(null, (Object) Arrays.asList(args));
      } catch (NoSuchMethodException nsme) {
        // 9 <= JDK < 15: the signature is
        //     String[] parse(String[] args)
        return Arrays.asList(
            (String[])
                CommandLine.class.getMethod("parse", String[].class).invoke(null, (Object) args));
      }
    } catch (ClassCastException
        | InvocationTargetException
        | IllegalAccessException
        | NoSuchMethodException ex) {
      throw new LinkageError(ex.getMessage(), ex);
    }
  }

  @Override
  public void initializeArguments(Arguments arguments, String[] args) {
    try {
      try {
        // JDK15+: the signature is
        //     init(String, Iterable<String> args);
        arguments
            .getClass()
            .getMethod("init", String.class, Iterable.class)
            .invoke(arguments, "kythe_javac", (Object) Arrays.asList(args));
      } catch (NoSuchMethodException nsme) {
        // pre-JDK15:
        //     init(String, String...);
        arguments
            .getClass()
            .getMethod("init", String.class, String[].class)
            .invoke(arguments, "kythe_javac", (Object) args);
      }
    } catch (ClassCastException
        | InvocationTargetException
        | IllegalAccessException
        | NoSuchMethodException ex) {
      throw new LinkageError(ex.getMessage(), ex);
    }
  }
}
