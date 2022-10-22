/*
 * Copyright 2016 The Kythe Authors. All rights reserved.
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

package com.google.devtools.kythe.analyzers.java;

import com.sun.source.tree.ClassTree;
import com.sun.source.tree.CompilationUnitTree;
import com.sun.source.tree.IdentifierTree;
import com.sun.source.tree.ImportTree;
import com.sun.source.tree.LabeledStatementTree;
import com.sun.source.tree.MemberReferenceTree;
import com.sun.source.tree.MethodTree;
import com.sun.source.tree.PackageTree;
import com.sun.source.tree.Tree;
import com.sun.source.tree.TreeVisitor;
import com.sun.source.tree.TypeParameterTree;
import com.sun.source.tree.VariableTree;
import com.sun.source.util.SimpleTreeVisitor;
import org.checkerframework.checker.nullness.qual.Nullable;

/** Java {@link TreeVisitor} to return a tree's simple name (class/method/field name). */
public class NameVisitor extends SimpleTreeVisitor<String, Void> {

  public static String getName(Tree tree) {
    return tree.accept(new NameVisitor(), null);
  }

  @Override
  public final @Nullable String visitCompilationUnit(CompilationUnitTree tree, Void v) {
    if (tree.getSourceFile() != null) {
      return "" + tree.getSourceFile().getName();
    }
    return null;
  }

  @Override
  public final String visitImport(ImportTree tree, Void v) {
    return "" + tree.getQualifiedIdentifier();
  }

  @Override
  public final String visitClass(ClassTree tree, Void v) {
    return "" + tree.getSimpleName();
  }

  @Override
  public final String visitMethod(MethodTree tree, Void v) {
    return "" + tree.getName();
  }

  @Override
  public final String visitVariable(VariableTree tree, Void v) {
    return "" + tree.getName();
  }

  @Override
  public final String visitTypeParameter(TypeParameterTree tree, Void v) {
    return "" + tree.getName();
  }

  @Override
  public final String visitIdentifier(IdentifierTree tree, Void v) {
    return "" + tree.getName();
  }

  @Override
  public final String visitMemberReference(MemberReferenceTree tree, Void v) {
    return "" + tree.getName();
  }

  @Override
  public final String visitLabeledStatement(LabeledStatementTree tree, Void v) {
    return "" + tree.getLabel();
  }

  @Override
  public final String visitPackage(PackageTree tree, Void v) {
    return "" + tree.getPackageName();
  }
}
