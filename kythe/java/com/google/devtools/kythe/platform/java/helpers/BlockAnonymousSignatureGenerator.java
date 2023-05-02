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

import com.google.common.flogger.FluentLogger;
import com.sun.source.tree.BlockTree;
import com.sun.source.tree.CatchTree;
import com.sun.source.tree.ClassTree;
import com.sun.source.tree.CompilationUnitTree;
import com.sun.source.tree.EnhancedForLoopTree;
import com.sun.source.tree.ForLoopTree;
import com.sun.source.tree.MethodTree;
import com.sun.source.tree.Tree;
import com.sun.source.util.TreePath;
import com.sun.source.util.TreeScanner;
import com.sun.tools.javac.tree.JCTree;
import com.sun.tools.javac.tree.JCTree.JCBlock;
import com.sun.tools.javac.tree.JCTree.JCClassDecl;
import com.sun.tools.javac.tree.JCTree.JCMethodDecl;
import java.util.HashMap;
import java.util.Map;
import javax.lang.model.element.Element;
import org.checkerframework.checker.nullness.qual.Nullable;

/**
 * This class is responsible for generating signatures for blocks and anonymous classes. The maps
 * used for caching in this class are specific through out the life of analyzer and they can be used
 * for one or more compilation units which are part of one compilation context.
 */
public class BlockAnonymousSignatureGenerator
    extends TreeScanner<Void, BlockAnonymousSignatureGenerator.BlockAnonymousData> {
  /*
   * Blocks are numbered sequentially from zero within a class scope or a method scope.
   * Here is an example of block numbering:
   * <code><pre>
   *  class A {// block 0
   *    {// block 1
   *      {// block 2
   *      }
   *    }
   *    {// block 3
   *    }
   *    void method() {// block 0
   *      {// block 1
   *      }
   *      if (true) {// block 2
   *      } else {// block 3
   *      }
   *    }
   *  }
   *  </code></pre>
   */

  static class BlockAnonymousData {
    int blockNumber;
    int anonymousNumber;
    final String topLevelSignature;

    public BlockAnonymousData(int blockNumber, int anonymousNumber, String topLevelSignature) {
      this.blockNumber = blockNumber;
      this.anonymousNumber = anonymousNumber;
      this.topLevelSignature = topLevelSignature;
    }
  }

  private static final FluentLogger logger = FluentLogger.forEnclosingClass();
  private final SignatureGenerator signatureGenerator;

  BlockAnonymousSignatureGenerator(SignatureGenerator signatureGenerator) {
    this.signatureGenerator = signatureGenerator;
  }

  // Indicates whether the compilation unit has already been processed or not.
  private boolean unitProcessed = false;

  // Holds the signature for each block visited.
  // Holds the signature for each anonymous class.
  // Anonymous classes are numbered with respect to their first enclosing class declaration or
  // block. They are numbered from zero.
  private final Map<Tree, String> blockAnonymousMap = new HashMap<>();

  /** Returns the block signature of the corresponding element */
  String getBlockSignature(Element element) {
    return getBlockSignature(signatureGenerator.getPath(element));
  }

  /** Returns the block signature of the corresponding path */
  String getBlockSignature(TreePath path) {
    this.process(path.getCompilationUnit());
    path = path.getParentPath();
    JCTree parent = (JCTree) path.getLeaf();
    while (!(parent.getTag() == JCTree.Tag.BLOCK
        || parent.getTag() == JCTree.Tag.CLASSDEF
        || parent.getTag() == JCTree.Tag.FORLOOP
        || parent.getTag() == JCTree.Tag.FOREACHLOOP
        || parent.getTag() == JCTree.Tag.METHODDEF
        || parent.getTag() == JCTree.Tag.CATCH)) {
      path = path.getParentPath();
      parent = (JCTree) path.getLeaf();
    }
    if (parent.getTag() == JCTree.Tag.BLOCK
        || parent.getTag() == JCTree.Tag.FORLOOP
        || parent.getTag() == JCTree.Tag.FOREACHLOOP
        || parent.getTag() == JCTree.Tag.CATCH) {
      return blockAnonymousMap.get(parent);
    } else if (parent.getTag() == JCTree.Tag.METHODDEF) {
      JCMethodDecl methodDecl = (JCMethodDecl) parent;
      StringBuilder methodSignature = new StringBuilder();
      methodDecl.sym.accept(this.signatureGenerator, methodSignature);
      return methodSignature.toString();
    } else {
      JCClassDecl classDecl = (JCClassDecl) parent;
      StringBuilder classSignature = new StringBuilder();
      classDecl.sym.accept(this.signatureGenerator, classSignature);
      return classSignature.toString();
    }
  }

  boolean isInBlock(Element e) {
    TreePath tp = signatureGenerator.getPath(e);
    JCTree parent = null;
    do {
      if (tp != null) {
        parent = (JCTree) tp.getLeaf();
      } else {
        return false;
      }
      tp = tp.getParentPath();
    } while (parent.getTag() != JCTree.Tag.BLOCK);
    return parent.getTag() == JCTree.Tag.BLOCK;
  }

  // Returns the anonymous signature of the corresponding element. Element is supposed to be an
  // anonymous class definition.
  @Nullable String getAnonymousSignature(Element e) {
    TreePath tp = signatureGenerator.getPath(e);
    if (tp == null) {
      return null;
    }
    this.process(tp.getCompilationUnit());
    return blockAnonymousMap.get(tp.getLeaf());
  }

  private void process(CompilationUnitTree unit) {
    if (!unitProcessed) {
      unitProcessed = true;
      unit.accept(this, new BlockAnonymousData(0, 0, ""));
    }
  }

  @Override
  public Void visitMethod(MethodTree methodTree, BlockAnonymousData blockData) {
    JCMethodDecl methodDecl = (JCMethodDecl) methodTree;
    StringBuilder methodSignature = new StringBuilder();
    if (methodDecl.sym == null) {
      logger.atInfo().log("methodDecl symbol was null");
      return null;
    }
    methodDecl.sym.accept(signatureGenerator, methodSignature);
    return super.visitMethod(methodTree, new BlockAnonymousData(-1, 0, methodSignature.toString()));
  }

  @Override
  public Void visitClass(ClassTree classTree, BlockAnonymousData blockData) {
    JCClassDecl classDecl = (JCClassDecl) classTree;
    if (classTree.getSimpleName().contentEquals("")) {
      StringBuilder anonymousClassSignature = new StringBuilder();
      if (classDecl.sym != null) {
        anonymousClassSignature.append(getBlockSignature(classDecl.sym));
      } else {
        String name = classDecl.name == null ? "" : classDecl.name.toString();
        logger.atWarning().log(
            "BlockAnonymous class symbol was null: %s#%d", name, blockData.anonymousNumber);
      }
      anonymousClassSignature.append(SignatureGenerator.ANONYMOUS);
      anonymousClassSignature.append("#");
      anonymousClassSignature.append(blockData.anonymousNumber);
      blockAnonymousMap.put(classDecl, anonymousClassSignature.toString());
      blockData.anonymousNumber++;
      return super.visitClass(
          classTree, new BlockAnonymousData(0, 0, blockAnonymousMap.get(classDecl)));
    } else {
      StringBuilder classSignature = new StringBuilder();
      classDecl.sym.accept(this.signatureGenerator, classSignature);
      return super.visitClass(classTree, new BlockAnonymousData(0, 0, classSignature.toString()));
    }
  }

  @Override
  public Void visitEnhancedForLoop(
      EnhancedForLoopTree enhancedForLoopTree, BlockAnonymousData blockData) {
    blockAnonymousMap.put(
        enhancedForLoopTree, blockData.topLevelSignature + ".{}" + (blockData.blockNumber + 1));
    if (enhancedForLoopTree.getStatement().getKind() != Tree.Kind.BLOCK) {
      blockData.blockNumber++;
    }
    return super.visitEnhancedForLoop(enhancedForLoopTree, blockData);
  }

  @Override
  public Void visitForLoop(ForLoopTree forLoopTree, BlockAnonymousData blockData) {
    blockAnonymousMap.put(
        forLoopTree, blockData.topLevelSignature + ".{}" + (blockData.blockNumber + 1));
    if (forLoopTree.getStatement().getKind() != Tree.Kind.BLOCK) {
      blockData.blockNumber++;
    }
    return super.visitForLoop(forLoopTree, blockData);
  }

  @Override
  public Void visitCatch(CatchTree catchTree, BlockAnonymousData blockData) {
    blockAnonymousMap.put(
        catchTree, blockData.topLevelSignature + ".{}" + (blockData.blockNumber + 1));
    return super.visitCatch(catchTree, blockData);
  }

  @Override
  public Void visitBlock(BlockTree b, BlockAnonymousData blockData) {
    blockData.blockNumber++;
    JCBlock block = (JCBlock) b;
    blockAnonymousMap.put(block, blockData.topLevelSignature + ".{}" + blockData.blockNumber);
    BlockAnonymousData newBlockAnonymousData =
        new BlockAnonymousData(blockData.blockNumber, 0, blockData.topLevelSignature);
    super.visitBlock(block, newBlockAnonymousData);
    blockData.blockNumber = newBlockAnonymousData.blockNumber;
    return null;
  }
}
