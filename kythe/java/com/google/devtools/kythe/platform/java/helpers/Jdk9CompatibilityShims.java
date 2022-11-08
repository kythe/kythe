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

package com.google.devtools.kythe.platform.java.helpers;

import com.google.auto.service.AutoService;
import com.google.common.collect.ImmutableList;
import com.sun.tools.javac.tree.JCTree.JCCase;
import com.sun.tools.javac.tree.JCTree.JCExpression;

/** Shims for providing source-level compatibility between JDK versions. */
@AutoService(JdkCompatibilityShims.class)
public final class Jdk9CompatibilityShims implements JdkCompatibilityShims {
  private static final Runtime.Version minVersion = Runtime.Version.parse("9");
  private static final Runtime.Version maxVersion = Runtime.Version.parse("15");

  public Jdk9CompatibilityShims() {}

  @Override
  public CompatibilityClass getCompatibility() {
    Runtime.Version version = Runtime.version();
    if (version.compareToIgnoreOptional(minVersion) >= 0
        && version.compareToIgnoreOptional(maxVersion) < 0) {
      return CompatibilityClass.COMPATIBLE;
    }
    return CompatibilityClass.INCOMPATIBLE;
  }

  /** Return the list of expressions from a JCCase object */
  @Override
  public ImmutableList<JCExpression> getCaseExpressions(JCCase tree) {
    JCExpression expr = tree.getExpression();
    if (expr == null) {
      return ImmutableList.of();
    }
    return ImmutableList.of(tree.getExpression());
  }
}
