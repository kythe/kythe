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
import com.google.common.collect.Lists;
import com.sun.tools.javac.main.Arguments;
import com.sun.tools.javac.main.CommandLine;
import java.io.IOException;
import java.util.List;

/** JdkCompatibilityShims implementation for JDK9-compatible releases. */
@AutoService(JdkCompatibilityShims.class)
public final class JdkCompatibilityShimsImpl implements JdkCompatibilityShims {
  private static final Runtime.Version minVersion = Runtime.Version.parse("9");
  private static final Runtime.Version maxVersion = Runtime.Version.parse("15");

  public JdkCompatibilityShimsImpl() {}

  @Override
  public CompatibilityRange getCompatibleRange() {
    return new CompatibilityRange(minVersion, maxVersion);
  }

  @Override
  public List<String> parseCompilerArguments(String[] args) throws IOException {
    return Lists.newArrayList(CommandLine.parse(args));
  }

  @Override
  public void initializeArguments(Arguments arguments, String[] args) {
    arguments.init("kythe_javac", args);
  }
}
