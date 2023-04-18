/*
 * Copyright 2014 The Kythe Authors. All rights reserved.
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

import com.google.common.collect.Lists;
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
import com.sun.source.tree.ExportsTree;
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
import com.sun.source.tree.ModuleTree;
import com.sun.source.tree.NewArrayTree;
import com.sun.source.tree.NewClassTree;
import com.sun.source.tree.OpensTree;
import com.sun.source.tree.PackageTree;
import com.sun.source.tree.ParameterizedTypeTree;
import com.sun.source.tree.ParenthesizedTree;
import com.sun.source.tree.PrimitiveTypeTree;
import com.sun.source.tree.ProvidesTree;
import com.sun.source.tree.RequiresTree;
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
import com.sun.source.tree.UsesTree;
import com.sun.source.tree.VariableTree;
import com.sun.source.tree.WhileLoopTree;
import com.sun.source.tree.WildcardTree;
import com.sun.source.util.SimpleTreeVisitor;
import com.sun.source.util.TreePath;
import com.sun.tools.javac.tree.JCTree;
import com.sun.tools.javac.tree.JCTree.JCAnnotatedType;
import com.sun.tools.javac.tree.JCTree.JCAnnotation;
import com.sun.tools.javac.tree.JCTree.JCArrayAccess;
import com.sun.tools.javac.tree.JCTree.JCArrayTypeTree;
import com.sun.tools.javac.tree.JCTree.JCAssert;
import com.sun.tools.javac.tree.JCTree.JCAssign;
import com.sun.tools.javac.tree.JCTree.JCAssignOp;
import com.sun.tools.javac.tree.JCTree.JCBinary;
import com.sun.tools.javac.tree.JCTree.JCBlock;
import com.sun.tools.javac.tree.JCTree.JCBreak;
import com.sun.tools.javac.tree.JCTree.JCCase;
import com.sun.tools.javac.tree.JCTree.JCCatch;
import com.sun.tools.javac.tree.JCTree.JCClassDecl;
import com.sun.tools.javac.tree.JCTree.JCCompilationUnit;
import com.sun.tools.javac.tree.JCTree.JCConditional;
import com.sun.tools.javac.tree.JCTree.JCContinue;
import com.sun.tools.javac.tree.JCTree.JCDoWhileLoop;
import com.sun.tools.javac.tree.JCTree.JCEnhancedForLoop;
import com.sun.tools.javac.tree.JCTree.JCErroneous;
import com.sun.tools.javac.tree.JCTree.JCExports;
import com.sun.tools.javac.tree.JCTree.JCExpressionStatement;
import com.sun.tools.javac.tree.JCTree.JCFieldAccess;
import com.sun.tools.javac.tree.JCTree.JCForLoop;
import com.sun.tools.javac.tree.JCTree.JCIdent;
import com.sun.tools.javac.tree.JCTree.JCIf;
import com.sun.tools.javac.tree.JCTree.JCImport;
import com.sun.tools.javac.tree.JCTree.JCInstanceOf;
import com.sun.tools.javac.tree.JCTree.JCLabeledStatement;
import com.sun.tools.javac.tree.JCTree.JCLambda;
import com.sun.tools.javac.tree.JCTree.JCLiteral;
import com.sun.tools.javac.tree.JCTree.JCMemberReference;
import com.sun.tools.javac.tree.JCTree.JCMethodDecl;
import com.sun.tools.javac.tree.JCTree.JCMethodInvocation;
import com.sun.tools.javac.tree.JCTree.JCModifiers;
import com.sun.tools.javac.tree.JCTree.JCModuleDecl;
import com.sun.tools.javac.tree.JCTree.JCNewArray;
import com.sun.tools.javac.tree.JCTree.JCNewClass;
import com.sun.tools.javac.tree.JCTree.JCOpens;
import com.sun.tools.javac.tree.JCTree.JCPackageDecl;
import com.sun.tools.javac.tree.JCTree.JCParens;
import com.sun.tools.javac.tree.JCTree.JCPrimitiveTypeTree;
import com.sun.tools.javac.tree.JCTree.JCProvides;
import com.sun.tools.javac.tree.JCTree.JCRequires;
import com.sun.tools.javac.tree.JCTree.JCReturn;
import com.sun.tools.javac.tree.JCTree.JCSkip;
import com.sun.tools.javac.tree.JCTree.JCSwitch;
import com.sun.tools.javac.tree.JCTree.JCSynchronized;
import com.sun.tools.javac.tree.JCTree.JCThrow;
import com.sun.tools.javac.tree.JCTree.JCTry;
import com.sun.tools.javac.tree.JCTree.JCTypeApply;
import com.sun.tools.javac.tree.JCTree.JCTypeCast;
import com.sun.tools.javac.tree.JCTree.JCTypeIntersection;
import com.sun.tools.javac.tree.JCTree.JCTypeParameter;
import com.sun.tools.javac.tree.JCTree.JCTypeUnion;
import com.sun.tools.javac.tree.JCTree.JCUnary;
import com.sun.tools.javac.tree.JCTree.JCUses;
import com.sun.tools.javac.tree.JCTree.JCVariableDecl;
import com.sun.tools.javac.tree.JCTree.JCWhileLoop;
import com.sun.tools.javac.tree.JCTree.JCWildcard;
import com.sun.tools.javac.tree.JCTree.LetExpr;
import com.sun.tools.javac.tree.JCTree.TypeBoundKind;

/** A {@link TreeScanner} with the scan/reduce semantics of a {@link TreeVisitor}. */
public class JCTreeScanner<R, P> extends SimpleTreeVisitor<R, P> {
  protected static final JdkCompatibilityShims shims = JdkCompatibilityShims.loadBest().get();

  protected TreePath treePath;

  public R scan(JCCompilationUnit unit, P p) {
    return scan(new TreePath(unit), p);
  }

  public R scan(TreePath path, P p) {
    treePath = path;
    return scan((JCTree) path.getLeaf(), p);
  }

  public R scan(JCTree tree, P p) {
    if (tree instanceof LetExpr || tree instanceof TypeBoundKind) {
      // Skip non-public APIs
      return null;
    }
    if (tree == null) {
      return null;
    }
    TreePath prev = treePath;
    try {
      if (treePath != null) {
        treePath = new TreePath(treePath, tree);
      }
      return tree.accept(this, p);
    } finally {
      treePath = prev;
    }
  }

  public R scan(Iterable<? extends JCTree> trees, P p) {
    R r = null;
    if (trees != null) {
      boolean first = true;
      for (JCTree node : trees) {
        r = (first ? scan(node, p) : scanAndReduce(node, p, r));
        first = false;
      }
    }
    return r;
  }

  public R scanAll(P p, JCTree tree, JCTree... others) {
    return scan(Lists.asList(tree, others), p);
  }

  public R reduce(R r1, R r2) {
    return r1;
  }

  private R scanAndReduce(JCTree tree, P p, R r) {
    return reduce(scan(tree, p), r);
  }

  private R scanAndReduce(Iterable<? extends JCTree> trees, P p, R r) {
    return reduce(scan(trees, p), r);
  }

  @Override
  public final R visitCompilationUnit(CompilationUnitTree tree, P p) {
    return visitTopLevel((JCCompilationUnit) tree, p);
  }

  public R visitTopLevel(JCCompilationUnit tree, P p) {
    R r = scan(tree.getPackage(), p);
    r = scanAndReduce(tree.getImports(), p, r);
    r = scanAndReduce(tree.getTypeDecls(), p, r);
    return r;
  }

  @Override
  public final R visitImport(ImportTree tree, P p) {
    return visitImport((JCImport) tree, p);
  }

  public R visitImport(JCImport tree, P p) {
    return scan(tree.qualid, p);
  }

  @Override
  public final R visitClass(ClassTree tree, P p) {
    return visitClassDef((JCClassDecl) tree, p);
  }

  public R visitClassDef(JCClassDecl tree, P p) {
    R r = scan(tree.mods, p);
    r = scanAndReduce(tree.typarams, p, r);
    r = scanAndReduce(tree.extending, p, r);
    r = scanAndReduce(tree.implementing, p, r);
    return scanAndReduce(tree.defs, p, r);
  }

  @Override
  public final R visitMethod(MethodTree tree, P p) {
    return visitMethodDef((JCMethodDecl) tree, p);
  }

  public R visitMethodDef(JCMethodDecl tree, P p) {
    R r = scan(tree.mods, p);
    r = scanAndReduce(tree.restype, p, r);
    r = scanAndReduce(tree.typarams, p, r);
    r = scanAndReduce(tree.params, p, r);
    r = scanAndReduce(tree.thrown, p, r);
    r = scanAndReduce(tree.defaultValue, p, r);
    return scanAndReduce(tree.body, p, r);
  }

  @Override
  public final R visitVariable(VariableTree tree, P p) {
    return visitVarDef((JCVariableDecl) tree, p);
  }

  public R visitVarDef(JCVariableDecl tree, P p) {
    R r = scan(tree.mods, p);
    r = scanAndReduce(tree.vartype, p, r);
    return scanAndReduce(tree.init, p, r);
  }

  @Override
  public final R visitEmptyStatement(EmptyStatementTree tree, P p) {
    return visitSkip((JCSkip) tree, p);
  }

  public R visitSkip(JCSkip tree, P p) {
    return null;
  }

  @Override
  public final R visitBlock(BlockTree tree, P p) {
    return visitBlock((JCBlock) tree, p);
  }

  public R visitBlock(JCBlock tree, P p) {
    return scan(tree.stats, p);
  }

  @Override
  public final R visitDoWhileLoop(DoWhileLoopTree tree, P p) {
    return visitDoLoop((JCDoWhileLoop) tree, p);
  }

  public R visitDoLoop(JCDoWhileLoop tree, P p) {
    R r = scan(tree.body, p);
    return scanAndReduce(tree.cond, p, r);
  }

  @Override
  public final R visitWhileLoop(WhileLoopTree tree, P p) {
    return visitWhileLoop((JCWhileLoop) tree, p);
  }

  public R visitWhileLoop(JCWhileLoop tree, P p) {
    R r = scan(tree.cond, p);
    return scanAndReduce(tree.body, p, r);
  }

  @Override
  public final R visitForLoop(ForLoopTree tree, P p) {
    return visitForLoop((JCForLoop) tree, p);
  }

  public R visitForLoop(JCForLoop tree, P p) {
    R r = scan(tree.init, p);
    r = scanAndReduce(tree.cond, p, r);
    r = scanAndReduce(tree.step, p, r);
    return scanAndReduce(tree.body, p, r);
  }

  @Override
  public final R visitEnhancedForLoop(EnhancedForLoopTree tree, P p) {
    return visitForeachLoop((JCEnhancedForLoop) tree, p);
  }

  public R visitForeachLoop(JCEnhancedForLoop tree, P p) {
    R r = scan(shims.getForLoopVar(tree), p);
    r = scanAndReduce(tree.expr, p, r);
    return scanAndReduce(tree.body, p, r);
  }

  @Override
  public final R visitLabeledStatement(LabeledStatementTree tree, P p) {
    return visitLabelled((JCLabeledStatement) tree, p);
  }

  public R visitLabelled(JCLabeledStatement tree, P p) {
    return scan(tree.body, p);
  }

  @Override
  public final R visitSwitch(SwitchTree tree, P p) {
    return visitSwitch((JCSwitch) tree, p);
  }

  public R visitSwitch(JCSwitch tree, P p) {
    R r = scan(tree.selector, p);
    return scanAndReduce(tree.cases, p, r);
  }

  @Override
  public final R visitCase(CaseTree tree, P p) {
    return visitCase((JCCase) tree, p);
  }

  public R visitCase(JCCase tree, P p) {
    R r = scan(shims.getCaseExpressions(tree), p);
    return scanAndReduce(tree.stats, p, r);
  }

  @Override
  public final R visitSynchronized(SynchronizedTree tree, P p) {
    return visitSynchronized((JCSynchronized) tree, p);
  }

  public R visitSynchronized(JCSynchronized tree, P p) {
    R r = scan(tree.lock, p);
    return scanAndReduce(tree.body, p, r);
  }

  @Override
  public final R visitTry(TryTree tree, P p) {
    return visitTry((JCTry) tree, p);
  }

  public R visitTry(JCTry tree, P p) {
    R r = scan(tree.resources, p);
    r = scanAndReduce(tree.body, p, r);
    r = scanAndReduce(tree.catchers, p, r);
    return scanAndReduce(tree.finalizer, p, r);
  }

  @Override
  public final R visitCatch(CatchTree tree, P p) {
    return visitCatch((JCCatch) tree, p);
  }

  public R visitCatch(JCCatch tree, P p) {
    R r = scan(tree.param, p);
    return scanAndReduce(tree.body, p, r);
  }

  @Override
  public final R visitConditionalExpression(ConditionalExpressionTree tree, P p) {
    return visitConditional((JCConditional) tree, p);
  }

  public R visitConditional(JCConditional tree, P p) {
    R r = scan(tree.cond, p);
    r = scanAndReduce(tree.truepart, p, r);
    return scanAndReduce(tree.falsepart, p, r);
  }

  @Override
  public final R visitIf(IfTree tree, P p) {
    return visitIf((JCIf) tree, p);
  }

  public R visitIf(JCIf tree, P p) {
    R r = scan(tree.cond, p);
    r = scanAndReduce(tree.thenpart, p, r);
    return scanAndReduce(tree.elsepart, p, r);
  }

  @Override
  public final R visitExpressionStatement(ExpressionStatementTree tree, P p) {
    return visitExec((JCExpressionStatement) tree, p);
  }

  public R visitExec(JCExpressionStatement tree, P p) {
    return scan(tree.expr, p);
  }

  @Override
  public final R visitBreak(BreakTree tree, P p) {
    return visitBreak((JCBreak) tree, p);
  }

  public R visitBreak(JCBreak tree, P p) {
    return null;
  }

  @Override
  public final R visitContinue(ContinueTree tree, P p) {
    return visitContinue((JCContinue) tree, p);
  }

  public R visitContinue(JCContinue tree, P p) {
    return null;
  }

  @Override
  public final R visitReturn(ReturnTree tree, P p) {
    return visitReturn((JCReturn) tree, p);
  }

  public R visitReturn(JCReturn tree, P p) {
    return scan(tree.expr, p);
  }

  @Override
  public final R visitThrow(ThrowTree tree, P p) {
    return visitThrow((JCThrow) tree, p);
  }

  public R visitThrow(JCThrow tree, P p) {
    return scan(tree.expr, p);
  }

  @Override
  public final R visitAssert(AssertTree tree, P p) {
    return visitAssert((JCAssert) tree, p);
  }

  public R visitAssert(JCAssert tree, P p) {
    R r = scan(tree.cond, p);
    return scanAndReduce(tree.detail, p, r);
  }

  @Override
  public final R visitMethodInvocation(MethodInvocationTree tree, P p) {
    return visitApply((JCMethodInvocation) tree, p);
  }

  public R visitApply(JCMethodInvocation tree, P p) {
    R r = scan(tree.typeargs, p);
    r = scanAndReduce(tree.meth, p, r);
    return scanAndReduce(tree.args, p, r);
  }

  @Override
  public final R visitNewClass(NewClassTree tree, P p) {
    return visitNewClass((JCNewClass) tree, p);
  }

  public R visitNewClass(JCNewClass tree, P p) {
    R r = scan(tree.encl, p);
    r = scanAndReduce(tree.clazz, p, r);
    r = scanAndReduce(tree.typeargs, p, r);
    r = scanAndReduce(tree.args, p, r);
    return scanAndReduce(tree.def, p, r);
  }

  @Override
  public final R visitNewArray(NewArrayTree tree, P p) {
    return visitNewArray((JCNewArray) tree, p);
  }

  public R visitNewArray(JCNewArray tree, P p) {
    R r = scan(tree.elemtype, p);
    r = scanAndReduce(tree.dims, p, r);
    return scanAndReduce(tree.elems, p, r);
  }

  @Override
  public final R visitParenthesized(ParenthesizedTree tree, P p) {
    return visitParens((JCParens) tree, p);
  }

  public R visitParens(JCParens tree, P p) {
    return scan(tree.expr, p);
  }

  @Override
  public final R visitAssignment(AssignmentTree tree, P p) {
    return visitAssign((JCAssign) tree, p);
  }

  public R visitAssign(JCAssign tree, P p) {
    R r = scan(tree.lhs, p);
    return scanAndReduce(tree.rhs, p, r);
  }

  @Override
  public final R visitCompoundAssignment(CompoundAssignmentTree tree, P p) {
    return visitAssignOp((JCAssignOp) tree, p);
  }

  public R visitAssignOp(JCAssignOp tree, P p) {
    R r = scan(tree.lhs, p);
    return scanAndReduce(tree.rhs, p, r);
  }

  @Override
  public R visitUnary(UnaryTree tree, P p) {
    return visitUnary((JCUnary) tree, p);
  }

  public R visitUnary(JCUnary tree, P p) {
    return scan(tree.arg, p);
  }

  @Override
  public final R visitBinary(BinaryTree tree, P p) {
    return visitBinary((JCBinary) tree, p);
  }

  public R visitBinary(JCBinary tree, P p) {
    R r = scan(tree.lhs, p);
    return scanAndReduce(tree.rhs, p, r);
  }

  @Override
  public final R visitTypeCast(TypeCastTree tree, P p) {
    return visitTypeCast((JCTypeCast) tree, p);
  }

  public R visitTypeCast(JCTypeCast tree, P p) {
    R r = scan(tree.clazz, p);
    return scanAndReduce(tree.expr, p, r);
  }

  @Override
  public final R visitInstanceOf(InstanceOfTree tree, P p) {
    return visitTypeTest((JCInstanceOf) tree, p);
  }

  public R visitTypeTest(JCInstanceOf tree, P p) {
    R r = scan(tree.expr, p);
    return scanAndReduce(tree.getType(), p, r);
  }

  @Override
  public final R visitArrayAccess(ArrayAccessTree tree, P p) {
    return visitIndexed((JCArrayAccess) tree, p);
  }

  public R visitIndexed(JCArrayAccess tree, P p) {
    R r = scan(tree.indexed, p);
    return scanAndReduce(tree.index, p, r);
  }

  @Override
  public final R visitMemberSelect(MemberSelectTree tree, P p) {
    return visitSelect((JCFieldAccess) tree, p);
  }

  public R visitSelect(JCFieldAccess tree, P p) {
    return scan(tree.selected, p);
  }

  @Override
  public final R visitIdentifier(IdentifierTree tree, P p) {
    return visitIdent((JCIdent) tree, p);
  }

  public R visitIdent(JCIdent tree, P p) {
    return null;
  }

  @Override
  public final R visitLiteral(LiteralTree tree, P p) {
    return visitLiteral((JCLiteral) tree, p);
  }

  public R visitLiteral(JCLiteral tree, P p) {
    return null;
  }

  @Override
  public final R visitPrimitiveType(PrimitiveTypeTree tree, P p) {
    return visitTypeIdent((JCPrimitiveTypeTree) tree, p);
  }

  public R visitTypeIdent(JCPrimitiveTypeTree tree, P p) {
    return null;
  }

  @Override
  public final R visitArrayType(ArrayTypeTree tree, P p) {
    return visitTypeArray((JCArrayTypeTree) tree, p);
  }

  public R visitTypeArray(JCArrayTypeTree tree, P p) {
    return scan(tree.elemtype, p);
  }

  @Override
  public final R visitParameterizedType(ParameterizedTypeTree tree, P p) {
    return visitTypeApply((JCTypeApply) tree, p);
  }

  public R visitTypeApply(JCTypeApply tree, P p) {
    R r = scan(tree.clazz, p);
    return scanAndReduce(tree.arguments, p, r);
  }

  @Override
  public final R visitUnionType(UnionTypeTree tree, P p) {
    return visitTypeUnion((JCTypeUnion) tree, p);
  }

  public R visitTypeUnion(JCTypeUnion tree, P p) {
    return scan(tree.alternatives, p);
  }

  @Override
  public final R visitTypeParameter(TypeParameterTree tree, P p) {
    return visitTypeParameter((JCTypeParameter) tree, p);
  }

  public R visitTypeParameter(JCTypeParameter tree, P p) {
    return scan(tree.bounds, p);
  }

  @Override
  public final R visitWildcard(WildcardTree node, P p) {
    return visitWildcard((JCWildcard) node, p);
  }

  public R visitWildcard(JCWildcard tree, P p) {
    R r = scan(tree.kind, p);
    return tree.inner == null ? r : scanAndReduce(tree.inner, p, r);
  }

  public R visitTypeBoundKind(TypeBoundKind tree, P p) {
    return null;
  }

  @Override
  public final R visitModifiers(ModifiersTree tree, P p) {
    return visitModifiers((JCModifiers) tree, p);
  }

  public R visitModifiers(JCModifiers tree, P p) {
    return scan(tree.annotations, p);
  }

  @Override
  public final R visitAnnotation(AnnotationTree tree, P p) {
    return visitAnnotation((JCAnnotation) tree, p);
  }

  public R visitAnnotation(JCAnnotation tree, P p) {
    R r = scan(tree.annotationType, p);
    return scanAndReduce(tree.args, p, r);
  }

  @Override
  public final R visitOther(Tree tree, P p) {
    if (tree instanceof TypeBoundKind) {
      return visitTypeBoundKind((TypeBoundKind) tree, p);
    }
    throw new IllegalStateException("Unknown Tree kind: " + tree.getClass());
  }

  @Override
  public final R visitIntersectionType(IntersectionTypeTree tree, P p) {
    return visitTypeIntersection((JCTypeIntersection) tree, p);
  }

  public R visitTypeIntersection(JCTypeIntersection tree, P p) {
    return scan(tree.bounds, p);
  }

  @Override
  public final R visitMemberReference(MemberReferenceTree tree, P p) {
    return visitReference((JCMemberReference) tree, p);
  }

  public R visitReference(JCMemberReference tree, P p) {
    R r = scan(tree.expr, p);
    return scanAndReduce(tree.typeargs, p, r);
  }

  @Override
  public final R visitErroneous(ErroneousTree tree, P p) {
    return visitErroneous((JCErroneous) tree, p);
  }

  public R visitErroneous(JCErroneous tree, P p) {
    return null;
  }

  @Override
  public final R visitLambdaExpression(LambdaExpressionTree tree, P p) {
    return visitLambda((JCLambda) tree, p);
  }

  public R visitLambda(JCLambda tree, P p) {
    R r = scan(tree.body, p);
    return scanAndReduce(tree.params, p, r);
  }

  @Override
  public final R visitAnnotatedType(AnnotatedTypeTree tree, P p) {
    return visitAnnotatedType((JCAnnotatedType) tree, p);
  }

  public R visitAnnotatedType(JCAnnotatedType tree, P p) {
    R r = scan(tree.annotations, p);
    return scanAndReduce(tree.underlyingType, p, r);
  }

  @Override
  public final R visitPackage(PackageTree tree, P p) {
    return visitPackage((JCPackageDecl) tree, p);
  }

  public R visitPackage(JCPackageDecl tree, P p) {
    R r = scan(tree.annotations, p);
    return scanAndReduce(tree.pid, p, r);
  }

  @Override
  public R visitModule(ModuleTree tree, P p) {
    return visitModule((JCModuleDecl) tree, p);
  }

  public R visitModule(JCModuleDecl tree, P p) {
    R r = scan(tree.mods.annotations, p);
    r = scanAndReduce(tree.getName(), p, r);
    r = scanAndReduce(tree.getDirectives(), p, r);
    return r;
  }

  @Override
  public R visitExports(ExportsTree tree, P p) {
    return visitExports((JCExports) tree, p);
  }

  public R visitExports(JCExports tree, P p) {
    R r = scan(tree.getPackageName(), p);
    r = scanAndReduce(tree.getModuleNames(), p, r);
    return r;
  }

  @Override
  public R visitOpens(OpensTree tree, P p) {
    return visitOpens((JCOpens) tree, p);
  }

  public R visitOpens(JCOpens tree, P p) {
    R r = scan(tree.getPackageName(), p);
    r = scanAndReduce(tree.getModuleNames(), p, r);
    return r;
  }

  @Override
  public R visitProvides(ProvidesTree tree, P p) {
    return visitProvides((JCProvides) tree, p);
  }

  public R visitProvides(JCProvides tree, P p) {
    R r = scan(tree.serviceName, p);
    r = scanAndReduce(tree.implNames, p, r);
    return r;
  }

  @Override
  public R visitRequires(RequiresTree tree, P p) {
    return visitRequires((JCRequires) tree, p);
  }

  public R visitRequires(JCRequires tree, P p) {
    return scan(tree.getModuleName(), p);
  }

  @Override
  public final R visitUses(UsesTree tree, P p) {
    return visitUses((JCUses) tree, p);
  }

  public final R visitUses(JCUses tree, P p) {
    return scan(tree.qualid, p);
  }
}
