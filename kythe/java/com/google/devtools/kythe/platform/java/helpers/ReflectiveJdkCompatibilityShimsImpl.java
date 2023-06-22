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
import com.sun.tools.javac.tree.JCTree;
import com.sun.tools.javac.tree.JCTree.JCCase;
import com.sun.tools.javac.tree.JCTree.JCEnhancedForLoop;
import com.sun.tools.javac.tree.JCTree.JCExpression;
import com.sun.tools.javac.tree.JCTree.JCImport;
import java.lang.reflect.InvocationTargetException;
import java.util.List;
import java.util.Optional;

/** Shims for providing source-level compatibility between JDK versions. */
@AutoService(JdkCompatibilityShims.class)
public final class ReflectiveJdkCompatibilityShimsImpl implements JdkCompatibilityShims {
  private static final Runtime.Version minVersion = Runtime.Version.parse("9");

  public ReflectiveJdkCompatibilityShimsImpl() {}

  @Override
  public CompatibilityRange getCompatibleRange() {
    return new CompatibilityRange(minVersion, Optional.empty(), CompatibilityLevel.FALLBACK);
  }

  /** Return the list of expressions from a JCCase object */
  @Override
  @SuppressWarnings("unchecked") // Safe by specification.
  public List<JCExpression> getCaseExpressions(JCCase tree) {
    try {
      try {
        return (List<JCExpression>) tree.getClass().getMethod("getExpressions").invoke(tree);
      } catch (NoSuchMethodException nsme) {
        return ImmutableList.of(
            (JCExpression) tree.getClass().getMethod("getExpression").invoke(tree));
      }
    } catch (InvocationTargetException
        | IllegalAccessException
        | NoSuchMethodException
        | ClassCastException ex) {
      throw new LinkageError(ex.getMessage(), ex);
    }
  }

  @Override
  public JCTree getForLoopVar(JCEnhancedForLoop tree) {
    try {
      try {
        return (JCTree) tree.getClass().getField("varOrRecordPattern").get(tree);
      } catch (NoSuchFieldException e) {
        return (JCTree) tree.getClass().getField("var").get(tree);
      }
    } catch (IllegalAccessException | NoSuchFieldException | ClassCastException ex) {
      throw new LinkageError(ex.getMessage(), ex);
    }
  }

  @Override
  public JCTree getQualifiedIdentifier(JCImport tree) {
    try {
      return (JCTree) tree.getClass().getMethod("getQualifiedIdentifier").invoke(tree);
    } catch (InvocationTargetException
        | IllegalAccessException
        | NoSuchMethodException
        | ClassCastException ex) {
      throw new LinkageError(ex.getMessage(), ex);
    }
  }
}
