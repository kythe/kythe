/*
 * Copyright 2014 Google Inc. All rights reserved.
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

import static java.util.stream.Collectors.joining;
import static java.util.stream.Collectors.toList;

import com.google.common.base.Optional;
import com.google.common.base.Preconditions;
import com.google.devtools.kythe.analyzers.base.EdgeKind;
import com.google.devtools.kythe.analyzers.base.EntrySet;
import com.google.devtools.kythe.common.FormattingLogger;
import com.google.devtools.kythe.platform.java.helpers.JCTreeScanner;
import com.google.devtools.kythe.platform.java.helpers.JavacUtil;
import com.google.devtools.kythe.platform.java.helpers.SignatureGenerator;

import com.sun.tools.javac.code.Symbol;
import com.sun.tools.javac.code.Symbol.ClassSymbol;
import com.sun.tools.javac.code.Symbol.MethodSymbol;
import com.sun.tools.javac.code.Type;
import com.sun.tools.javac.tree.JCTree;
import com.sun.tools.javac.tree.JCTree.JCAnnotation;
import com.sun.tools.javac.tree.JCTree.JCArrayTypeTree;
import com.sun.tools.javac.tree.JCTree.JCClassDecl;
import com.sun.tools.javac.tree.JCTree.JCCompilationUnit;
import com.sun.tools.javac.tree.JCTree.JCExpression;
import com.sun.tools.javac.tree.JCTree.JCFieldAccess;
import com.sun.tools.javac.tree.JCTree.JCIdent;
import com.sun.tools.javac.tree.JCTree.JCImport;
import com.sun.tools.javac.tree.JCTree.JCMethodDecl;
import com.sun.tools.javac.tree.JCTree.JCNewClass;
import com.sun.tools.javac.tree.JCTree.JCPrimitiveTypeTree;
import com.sun.tools.javac.tree.JCTree.JCTypeApply;
import com.sun.tools.javac.tree.JCTree.JCVariableDecl;
import com.sun.tools.javac.tree.JCTree.JCWildcard;
import com.sun.tools.javac.util.Context;
import com.sun.tools.javac.util.Name;

import java.io.IOException;
import java.nio.charset.Charset;
import java.util.Arrays;
import java.util.HashSet;
import java.util.List;
import java.util.Objects;
import java.util.Set;

/** {@link JCTreeScanner} that emits Kythe nodes and edges. */
public class KytheTreeScanner extends JCTreeScanner<JavaNode, Void> {
  private static final FormattingLogger logger =
      FormattingLogger.getLogger(KytheTreeScanner.class);

  private final JavaEntrySets entrySets;
  // TODO(schroederc): refactor SignatureGenerator for new schema names
  private final SignatureGenerator signatureGenerator;
  private final FilePositions filePositions;
  private final Context context;

  private KytheTreeScanner(JavaEntrySets entrySets, SignatureGenerator signatureGenerator,
      FilePositions filePositions, Context context) {
    this.entrySets = entrySets;
    this.signatureGenerator = signatureGenerator;
    this.filePositions = filePositions;
    this.context = context;
  }

  public static void emitEntries(Context context, JavaEntrySets entrySets,
      SignatureGenerator signatureGenerator, JCCompilationUnit compilations, Charset sourceEncoding)
      throws IOException {
    FilePositions filePositions = new FilePositions(context, compilations, sourceEncoding);
    new KytheTreeScanner(entrySets, signatureGenerator, filePositions, context)
        .scan(compilations, null);
  }

  @Override
  public JavaNode visitTopLevel(JCCompilationUnit compilation, Void v) {
    EntrySet fileNode = entrySets.getFileNode(filePositions);

    List<JavaNode> decls = scanList(compilation.getTypeDecls(), v).stream()
        .filter(Objects::nonNull)
        .collect(toList());
    decls.stream()
        .forEach(n -> entrySets.emitEdge(n.entries, EdgeKind.CHILDOF, fileNode));

    if (compilation.getPackageName() != null) {
      EntrySet pkgNode = entrySets.getPackageNode(compilation.packge);
      emitAnchor(compilation.getPackageName(), EdgeKind.REF, pkgNode);
      decls.stream()
          .forEach(n -> entrySets.emitEdge(n.entries, EdgeKind.CHILDOF, pkgNode));
    }

    scanList(compilation.getImports(), v);
    scanList(compilation.getPackageAnnotations(), v);
    return new JavaNode(fileNode, filePositions.getFilename());
  }

  @Override
  public JavaNode visitImport(JCImport imprt, Void v) {
    if (imprt.qualid instanceof JCFieldAccess) {
      JCFieldAccess imprtField = (JCFieldAccess) imprt.qualid;
      // TODO(schroeder): emit package node for imprtField.selected
      return emitNameUsage(imprtField, imprtField.sym, imprtField.name);
    }
    return scan(imprt.qualid, v);
  }

  @Override
  public JavaNode visitIdent(JCIdent ident, Void v) {
    return emitSymUsage(ident, ident.sym);
  }

  @Override
  public JavaNode visitClassDef(JCClassDecl classDef, Void v) {
    Optional<String> signature = signatureGenerator.getSignature(classDef.sym);
    if (signature.isPresent()) {
      EntrySet classNode = entrySets.getNode(classDef.sym, signature.get());
      emitAnchor(classDef.name, classDef.getStartPosition(), EdgeKind.DEFINES, classNode);

      visitAnnotations(classNode, classDef.getModifiers().getAnnotations(), v);

      JavaNode superClassNode = scan(classDef.getExtendsClause(), v);
      if (superClassNode != null) {
        emitEdge(classNode, EdgeKind.EXTENDS, superClassNode);
      }

      for (JCExpression implClass : classDef.getImplementsClause()) {
        JavaNode implNode = scan(implClass, v);
        if (implNode == null) {
          logger.warning("Missing 'implements' node for " + implClass.getClass() + ": " + implClass);
          continue;
        }
        emitEdge(classNode, EdgeKind.IMPLEMENTS, implNode);
      }

      classDef.getMembers().stream()
          .map(member -> scan(member, v))
          .filter(Objects::nonNull)
          .forEach(member ->
              entrySets.emitEdge(member.entries, EdgeKind.CHILDOF, classNode));

      return new JavaNode(classNode, signature.get());
    }
    return todoNode("JCClass: " + classDef);
  }

  @Override
  public JavaNode visitMethodDef(JCMethodDecl methodDef, Void v) {
    scan(methodDef.getBody(), v);
    JavaNode returnType = scan(methodDef.getReturnType(), v);
    List<JavaNode> params = scanList(methodDef.getParameters(), v);

    Optional<String> signature = signatureGenerator.getSignature(methodDef.sym);
    if (signature.isPresent()) {
      EntrySet methodNode = entrySets.getNode(methodDef.sym, signature.get());
      visitAnnotations(methodNode, methodDef.getModifiers().getAnnotations(), v);

      EntrySet ret;
      if (methodDef.sym.isConstructor()) {
        // Use the owner's name (the class name) to find the definition anchor's
        // location because constructors are internally named "<init>".
        emitAnchor(methodDef.sym.owner.name, methodDef.getPreferredPosition(),
            EdgeKind.DEFINES, methodNode);
        // Likewise, constructors don't have return types in the Java AST, but
        // Kythe models all functions with return types.  As a solution, we use
        // the class type as the return type for all constructors.
        ret = getNode(methodDef.sym.owner);
      } else {
        emitAnchor(methodDef.name, methodDef.getPreferredPosition(), EdgeKind.DEFINES, methodNode);
        ret = returnType.entries;
      }

      emitOrdinalEdges(methodNode, EdgeKind.PARAM, params);
      EntrySet fnTypeNode = entrySets.newFunctionType(ret, params.stream()
          .map(n -> n.typeNode)
          .collect(toList()));
      entrySets.emitEdge(methodNode, EdgeKind.TYPED, fnTypeNode);

      ClassSymbol ownerClass = (ClassSymbol) methodDef.sym.owner;
      Set<Type> ownerDirectSupertypes = new HashSet<>(ownerClass.getInterfaces());
      ownerDirectSupertypes.add(ownerClass.getSuperclass());
      for (MethodSymbol superMethod : JavacUtil.superMethods(context, methodDef.sym)) {
        EntrySet superNode = getNode(superMethod);
        if (ownerDirectSupertypes.contains(superMethod.owner.asType())) {
          entrySets.emitEdge(methodNode, EdgeKind.OVERRIDES, superNode);
        } else {
          entrySets.emitEdge(methodNode, EdgeKind.OVERRIDES_TRANSITIVE, superNode);
        }
      }

      return new JavaNode(methodNode, signature.get(), fnTypeNode);
    }
    return todoNode("MethodDef: " + methodDef);
  }

  @Override
  public JavaNode visitVarDef(JCVariableDecl varDef, Void v) {
    Optional<String> signature = signatureGenerator.getSignature(varDef.sym);
    if (signature.isPresent()) {
      EntrySet varNode = entrySets.getNode(varDef.sym, signature.get());
      emitAnchor(varDef.name, varDef.getStartPosition(), EdgeKind.DEFINES, varNode);

      visitAnnotations(varNode, varDef.getModifiers().getAnnotations(), v);

      JavaNode typeNode = scan(varDef.getType(), v);
      if (typeNode != null) {
        emitEdge(varNode, EdgeKind.TYPED, typeNode);
      }

      scan(varDef.getInitializer(), v);
      return new JavaNode(varNode, signature.get(), typeNode);
    }
    return todoNode("VarDef: " + varDef);
  }

  @Override
  public JavaNode visitTypeApply(JCTypeApply tApply, Void v) {
    JavaNode typeCtorNode = scan(tApply.getType(), v);

    List<JavaNode> arguments = scanList(tApply.getTypeArguments(), v);
    List<EntrySet> argEntries = arguments.stream()
        .map(n -> n.entries)
        .collect(toList());

    EntrySet typeNode = entrySets.newTApply(typeCtorNode.entries, argEntries);
    emitAnchor(tApply, EdgeKind.REF, typeNode);

    String qualifiedName = typeCtorNode.qualifiedName
        + "<" + arguments.stream().map(n -> n.qualifiedName).collect(joining(",")) + ">";
    entrySets.emitName(typeNode, qualifiedName);

    return new JavaNode(typeNode, qualifiedName);
  }

  @Override
  public JavaNode visitSelect(JCFieldAccess field, Void v) {
    scan(field.getExpression(), v);
    return emitNameUsage(field, field.sym, field.name);
  }

  @Override
  public JavaNode visitNewClass(JCNewClass newClass, Void v) {
    EntrySet ctorNode = getNode(newClass.constructor);
    if (ctorNode != null) {
      // Span over "new Class"
      EntrySet anchor = entrySets.getAnchor(filePositions,
          filePositions.getStart(newClass),
          filePositions.getEnd(newClass.getIdentifier()));
      emitAnchor(anchor, EdgeKind.REF, ctorNode);
      return super.visitNewClass(newClass, v);
    }
    return todoNode("NewClass: " + newClass);
  }

  @Override
  public JavaNode visitTypeIdent(JCPrimitiveTypeTree primitiveType, Void v) {
    String name = primitiveType.getPrimitiveTypeKind().toString().toLowerCase();
    EntrySet node = entrySets.getBuiltin(name);
    emitAnchor(primitiveType, EdgeKind.REF, node);
    return new JavaNode(node, name, node);
  }

  @Override
  public JavaNode visitTypeArray(JCArrayTypeTree arrayType, Void v) {
    JavaNode typeNode = scan(arrayType.getType(), v);
    EntrySet node = entrySets
        .newTApply(entrySets.getBuiltin("array"), Arrays.asList(typeNode.entries));
    emitAnchor(arrayType, EdgeKind.REF, node);
    return new JavaNode(node, typeNode.qualifiedName + "[]", node);
  }

  @Override
  public JavaNode visitAnnotation(JCAnnotation annotation, Void v) {
    return scan(annotation.getAnnotationType(), v);
  }

  @Override
  public JavaNode visitWildcard(JCWildcard wild, Void v) {
    return todoNode("WildCard: " + wild);
  }

  //// Utility methods ////

  /** Returns the node associated with a {@link Symbol} or {@link null}. */
  private EntrySet getNode(Symbol sym) {
    JavaNode node = getJavaNode(sym);
    return node == null ? null : node.entries;
  }

  /** Returns the {@link JavaNode} associated with a {@link Symbol} or {@link null}. */
  private JavaNode getJavaNode(Symbol sym) {
    Optional<String> signature = signatureGenerator.getSignature(sym);
    if (!signature.isPresent()) {
      return null;
    }
    return new JavaNode(entrySets.getNode(sym, signature.get()), signature.get());
  }

  private void visitAnnotations(EntrySet owner, List<JCAnnotation> annotations, Void v) {
    // TODO(schroederc): edge from owner to each of its annotations
    scanList(annotations, v);
  }

  // Emits a node for the given sym, an anchor encompassing the tree, and a REF edge
  private JavaNode emitSymUsage(JCTree tree, Symbol sym) {
    JavaNode node = getJavaNode(sym);
    if (node == null) {
      return todoNode("ExprUsage: " + tree);
    }

    emitAnchor(tree, EdgeKind.REF, node.entries);
    return node;
  }

  // Emits a node for the given sym, an anchor encompassing the name, and a REF edge
  private JavaNode emitNameUsage(JCTree tree, Symbol sym, Name name) {
    JavaNode node = getJavaNode(sym);
    if (node == null) {
      return todoNode("NameUsage: " + tree + " -- " + name);
    }

    emitAnchor(name, tree.getStartPosition(), EdgeKind.REF, node.entries);
    return node;
  }

  // Creates/emits an anchor and an associated edge
  private void emitAnchor(JCTree anchorTree, EdgeKind kind, EntrySet node) {
    emitAnchor(entrySets.getAnchor(filePositions, anchorTree), kind, node);
  }

  // Creates/emits an anchor (for an identifier) and an associated edge
  private void emitAnchor(Name name, int startOffset, EdgeKind kind, EntrySet node) {
    EntrySet anchor = entrySets.getAnchor(filePositions, name, startOffset);
    if (anchor == null) {
      // TODO(schroederc): Special-case these anchors (most come from visitSelect)
      return;
    }
    emitAnchor(anchor, kind, node);
  }

  // Creates/emits an anchor and an associated edge
  private void emitAnchor(EntrySet anchor, EdgeKind kind, EntrySet node) {
    Preconditions.checkArgument(kind.isAnchorEdge(),
        "EdgeKind was not intended for ANCHORs: " + kind);
    entrySets.emitEdge(anchor, kind, node);
  }

  // Unwraps the target EntrySet and emits an edge to it from the sourceNode
  private void emitEdge(EntrySet sourceNode, EdgeKind kind, JavaNode target) {
    entrySets.emitEdge(sourceNode, kind, target.entries);
  }

  // Unwraps each target EntrySet and emits an ordinal edge to each from the given source node
  private void emitOrdinalEdges(EntrySet node, EdgeKind kind, List<JavaNode> targets) {
    entrySets.emitOrdinalEdges(node, kind,
        targets.stream().map(n -> n.entries).collect(toList()));
  }

  @Deprecated
  private JavaNode todoNode(String message) {
    EntrySet node = entrySets.todoNode(message);
    return new JavaNode(node, "TODO", node);
  }

  private <T extends JCTree> List<JavaNode> scanList(List<T> trees, Void v) {
    return trees.stream().map(t -> scan(t, v)).collect(toList());
  }
}

class JavaNode {
  // TODO(schroederc): clearly separate semantic/type nodes
  final EntrySet entries;
  final EntrySet typeNode;
  final String qualifiedName;

  public JavaNode(EntrySet entries, String qualifiedName) {
    this(entries, qualifiedName, (EntrySet) null);
  }

  public JavaNode(EntrySet entries, String qualifiedName, JavaNode typeNode) {
    this(entries, qualifiedName, typeNode == null ? null : typeNode.entries);
  }

  public JavaNode(EntrySet entries, String qualifiedName, EntrySet typeNode) {
    this.entries = entries;
    this.qualifiedName = qualifiedName;
    this.typeNode = typeNode;
  }

  @Override
  public String toString() {
    return "JavaNode{" + qualifiedName + "}";
  }
}
