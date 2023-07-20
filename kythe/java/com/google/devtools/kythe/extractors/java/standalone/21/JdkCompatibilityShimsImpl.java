/*
 * Copyright 2023 The Kythe Authors. All rights reserved.
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
import java.io.IOException;
import java.util.Arrays;
import java.util.List;
import jdk.internal.opt.CommandLine;

/** JdkCompatibilityShims implementation for JDK21-compatible releases. */
@AutoService(JdkCompatibilityShims.class)
public final class JdkCompatibilityShimsImpl implements JdkCompatibilityShims {
  private static final Runtime.Version minVersion = Runtime.Version.parse("21");

  public JdkCompatibilityShimsImpl() {}

  @Override
  public CompatibilityRange getCompatibleRange() {
    return new CompatibilityRange(minVersion);
  }

  @Override
  public List<String> parseCompilerArguments(String[] args) throws IOException {
    return Lists.newArrayList(CommandLine.parse(Arrays.asList(args)));
  }

  @Override
  public void initializeArguments(Arguments arguments, String[] args) {
    arguments.init("kythe_javac", Arrays.asList(args));
  }
}
