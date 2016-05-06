/*
 * Copyright 2016 Google Inc. All rights reserved.
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

import com.sun.source.tree.AnnotatedTypeTree;
import com.sun.source.tree.AnnotationTree;
import com.sun.source.tree.ArrayAccessTree;
import com.sun.source.tree.ArrayTypeTree;
import com.sun.source.tree.AssertTree;
import com.sun.source.tree.AssignmentTree;
import com.sun.source.tree.BinaryTree;
import com.sun.source.tree.BlockTree;
import com.sun.source.tree.BreakTree;
import com.sun.source.tree.CaseTree;
import com.sun.source.tree.CatchTree;
import com.sun.source.tree.ClassTree;
import com.sun.source.tree.CompilationUnitTree;
import com.sun.source.tree.CompoundAssignmentTree;
import com.sun.source.tree.ConditionalExpressionTree;
import com.sun.source.tree.ContinueTree;
import com.sun.source.tree.DoWhileLoopTree;
import com.sun.source.tree.EmptyStatementTree;
import com.sun.source.tree.EnhancedForLoopTree;
import com.sun.source.tree.ErroneousTree;
import com.sun.source.tree.ExpressionStatementTree;
import com.sun.source.tree.ForLoopTree;
import com.sun.source.tree.IdentifierTree;
import com.sun.source.tree.IfTree;
import com.sun.source.tree.ImportTree;
import com.sun.source.tree.InstanceOfTree;
import com.sun.source.tree.IntersectionTypeTree;
import com.sun.source.tree.LabeledStatementTree;
import com.sun.source.tree.LambdaExpressionTree;
import com.sun.source.tree.LiteralTree;
import com.sun.source.tree.MemberReferenceTree;
import com.sun.source.tree.MemberSelectTree;
import com.sun.source.tree.MethodInvocationTree;
import com.sun.source.tree.MethodTree;
import com.sun.source.tree.ModifiersTree;
import com.sun.source.tree.NewArrayTree;
import com.sun.source.tree.NewClassTree;
import com.sun.source.tree.PackageTree;
import com.sun.source.tree.ParameterizedTypeTree;
import com.sun.source.tree.ParenthesizedTree;
import com.sun.source.tree.PrimitiveTypeTree;
import com.sun.source.tree.ReturnTree;
import com.sun.source.tree.SwitchTree;
import com.sun.source.tree.SynchronizedTree;
import com.sun.source.tree.ThrowTree;
import com.sun.source.tree.Tree;
import com.sun.source.tree.TreeVisitor;
import com.sun.source.tree.TryTree;
import com.sun.source.tree.TypeCastTree;
import com.sun.source.tree.TypeParameterTree;
import com.sun.source.tree.UnaryTree;
import com.sun.source.tree.UnionTypeTree;
import com.sun.source.tree.VariableTree;
import com.sun.source.tree.WhileLoopTree;
import com.sun.source.tree.WildcardTree;

/** Java {@link TreeVisitor} to return a tree's simple name (class/method/field name). */
public class NameVisitor implements TreeVisitor<String, Void> {

  public static String getName(Tree tree) {
    return tree.accept(new NameVisitor(), null);
  }

  @Override
  public final String visitCompilationUnit(CompilationUnitTree tree, Void v) {
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

  @Override
  public final String visitEmptyStatement(EmptyStatementTree tree, Void v) {
    return null;
  }

  @Override
  public final String visitBlock(BlockTree tree, Void v) {
    return null;
  }

  @Override
  public final String visitDoWhileLoop(DoWhileLoopTree tree, Void v) {
    return null;
  }

  @Override
  public final String visitWhileLoop(WhileLoopTree tree, Void v) {
    return null;
  }

  @Override
  public final String visitForLoop(ForLoopTree tree, Void v) {
    return null;
  }

  @Override
  public final String visitEnhancedForLoop(EnhancedForLoopTree tree, Void v) {
    return null;
  }

  @Override
  public final String visitSwitch(SwitchTree tree, Void v) {
    return null;
  }

  @Override
  public final String visitCase(CaseTree tree, Void v) {
    return null;
  }

  @Override
  public final String visitSynchronized(SynchronizedTree tree, Void v) {
    return null;
  }

  @Override
  public final String visitTry(TryTree tree, Void v) {
    return null;
  }

  @Override
  public final String visitCatch(CatchTree tree, Void v) {
    return null;
  }

  @Override
  public final String visitConditionalExpression(ConditionalExpressionTree tree, Void v) {
    return null;
  }

  @Override
  public final String visitIf(IfTree tree, Void v) {
    return null;
  }

  @Override
  public final String visitExpressionStatement(ExpressionStatementTree tree, Void v) {
    return null;
  }

  @Override
  public final String visitBreak(BreakTree tree, Void v) {
    return null;
  }

  @Override
  public final String visitContinue(ContinueTree tree, Void v) {
    return null;
  }

  @Override
  public final String visitReturn(ReturnTree tree, Void v) {
    return null;
  }

  @Override
  public final String visitThrow(ThrowTree tree, Void v) {
    return null;
  }

  @Override
  public final String visitAssert(AssertTree tree, Void v) {
    return null;
  }

  @Override
  public final String visitMethodInvocation(MethodInvocationTree tree, Void v) {
    return null;
  }

  @Override
  public final String visitNewClass(NewClassTree tree, Void v) {
    return null;
  }

  @Override
  public final String visitNewArray(NewArrayTree tree, Void v) {
    return null;
  }

  @Override
  public final String visitParenthesized(ParenthesizedTree tree, Void v) {
    return null;
  }

  @Override
  public final String visitAssignment(AssignmentTree tree, Void v) {
    return null;
  }

  @Override
  public final String visitCompoundAssignment(CompoundAssignmentTree tree, Void v) {
    return null;
  }

  @Override
  public final String visitUnary(UnaryTree tree, Void v) {
    return null;
  }

  @Override
  public final String visitBinary(BinaryTree tree, Void v) {
    return null;
  }

  @Override
  public final String visitTypeCast(TypeCastTree tree, Void v) {
    return null;
  }

  @Override
  public final String visitInstanceOf(InstanceOfTree tree, Void v) {
    return null;
  }

  @Override
  public final String visitArrayAccess(ArrayAccessTree tree, Void v) {
    return null;
  }

  @Override
  public final String visitMemberSelect(MemberSelectTree tree, Void v) {
    return null;
  }

  @Override
  public final String visitLiteral(LiteralTree tree, Void v) {
    return null;
  }

  @Override
  public final String visitPrimitiveType(PrimitiveTypeTree tree, Void v) {
    return null;
  }

  @Override
  public final String visitArrayType(ArrayTypeTree tree, Void v) {
    return null;
  }

  @Override
  public final String visitParameterizedType(ParameterizedTypeTree tree, Void v) {
    return null;
  }

  @Override
  public final String visitUnionType(UnionTypeTree tree, Void v) {
    return null;
  }

  @Override
  public final String visitWildcard(WildcardTree node, Void v) {
    return null;
  }

  @Override
  public final String visitModifiers(ModifiersTree tree, Void v) {
    return null;
  }

  @Override
  public final String visitAnnotation(AnnotationTree tree, Void v) {
    return null;
  }

  @Override
  public final String visitOther(Tree tree, Void v) {
    return null;
  }

  @Override
  public final String visitIntersectionType(IntersectionTypeTree tree, Void v) {
    return null;
  }

  @Override
  public final String visitErroneous(ErroneousTree tree, Void v) {
    return null;
  }

  @Override
  public final String visitLambdaExpression(LambdaExpressionTree tree, Void v) {
    return null;
  }

  @Override
  public final String visitAnnotatedType(AnnotatedTypeTree tree, Void v) {
    return null;
  }
}
