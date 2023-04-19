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
import com.sun.tools.javac.tree.JCTree;
import com.sun.tools.javac.tree.JCTree.JCCase;
import com.sun.tools.javac.tree.JCTree.JCEnhancedForLoop;
import com.sun.tools.javac.tree.JCTree.JCExpression;
import java.util.List;

/** Shims for providing source-level compatibility between JDK versions. */
@AutoService(JdkCompatibilityShims.class)
public final class Jdk20CompatibilityShims implements JdkCompatibilityShims {
  private static final Runtime.Version minVersion = Runtime.Version.parse("20");

  public Jdk20CompatibilityShims() {}

  @Override
  public CompatibilityRange getCompatibileRange() {
    return new CompatibilityRange(minVersion);
  }

  /** Return the list of expressions from a JCCase object */
  @Override
  public List<JCExpression> getCaseExpressions(JCCase tree) {
    return tree.getExpressions();
  }

  @Override
  public JCTree getForLoopVar(JCEnhancedForLoop tree) {
    return tree.varOrRecordPattern;
  }
}
