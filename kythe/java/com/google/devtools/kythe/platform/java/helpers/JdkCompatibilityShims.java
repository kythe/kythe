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

import com.google.devtools.kythe.util.OrderedCompatibilityService;
import com.sun.tools.javac.tree.JCTree;
import com.sun.tools.javac.tree.JCTree.JCCase;
import com.sun.tools.javac.tree.JCTree.JCEnhancedForLoop;
import com.sun.tools.javac.tree.JCTree.JCExpression;
import com.sun.tools.javac.tree.JCTree.JCImport;
import java.util.List;
import java.util.Optional;

/** Shims for providing source-level compatibility between JDK versions. */
public interface JdkCompatibilityShims extends OrderedCompatibilityService {
  /** Loads the best service provider for this interface, if any. */
  public static Optional<JdkCompatibilityShims> loadBest() {
    return OrderedCompatibilityService.loadBest(JdkCompatibilityShims.class);
  }

  /** Return the list of expressions from a JCCase object */
  List<JCExpression> getCaseExpressions(JCCase tree);

  /** Return var in a for loop */
  JCTree getForLoopVar(JCEnhancedForLoop tree);

  /** Return the qualified identifier for an import. */
  JCTree getQualifiedIdentifier(JCImport tree);
}
