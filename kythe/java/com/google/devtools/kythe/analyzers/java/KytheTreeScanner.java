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

import com.google.common.base.Joiner;
import com.google.common.base.Optional;
import com.google.common.base.Preconditions;
import com.google.common.collect.Lists;
import com.google.devtools.kythe.analyzers.base.EdgeKind;
import com.google.devtools.kythe.analyzers.base.EntrySet;
import com.google.devtools.kythe.analyzers.java.SourceText.Keyword;
import com.google.devtools.kythe.analyzers.java.SourceText.Positions;
import com.google.devtools.kythe.common.FormattingLogger;
import com.google.devtools.kythe.platform.java.helpers.JCTreeScanner;
import com.google.devtools.kythe.platform.java.helpers.JavacUtil;
import com.google.devtools.kythe.platform.java.helpers.SignatureGenerator;
import com.google.devtools.kythe.platform.shared.StatisticsCollector;
import com.google.devtools.kythe.util.Span;
import com.sun.source.tree.MemberReferenceTree.ReferenceMode;
import com.sun.source.tree.Tree.Kind;
import com.sun.tools.javac.code.Symbol;
import com.sun.tools.javac.code.Symbol.ClassSymbol;
import com.sun.tools.javac.code.Symbol.MethodSymbol;
import com.sun.tools.javac.code.Symbol.PackageSymbol;
import com.sun.tools.javac.code.Symtab;
import com.sun.tools.javac.code.Type;
import com.sun.tools.javac.code.TypeTag;
import com.sun.tools.javac.tree.JCTree;
import com.sun.tools.javac.tree.JCTree.JCAnnotation;
import com.sun.tools.javac.tree.JCTree.JCArrayTypeTree;
import com.sun.tools.javac.tree.JCTree.JCAssert;
import com.sun.tools.javac.tree.JCTree.JCAssign;
import com.sun.tools.javac.tree.JCTree.JCAssignOp;
import com.sun.tools.javac.tree.JCTree.JCClassDecl;
import com.sun.tools.javac.tree.JCTree.JCCompilationUnit;
import com.sun.tools.javac.tree.JCTree.JCExpression;
import com.sun.tools.javac.tree.JCTree.JCExpressionStatement;
import com.sun.tools.javac.tree.JCTree.JCFieldAccess;
import com.sun.tools.javac.tree.JCTree.JCIdent;
import com.sun.tools.javac.tree.JCTree.JCImport;
import com.sun.tools.javac.tree.JCTree.JCMemberReference;
import com.sun.tools.javac.tree.JCTree.JCMethodDecl;
import com.sun.tools.javac.tree.JCTree.JCMethodInvocation;
import com.sun.tools.javac.tree.JCTree.JCNewClass;
import com.sun.tools.javac.tree.JCTree.JCPrimitiveTypeTree;
import com.sun.tools.javac.tree.JCTree.JCReturn;
import com.sun.tools.javac.tree.JCTree.JCThrow;
import com.sun.tools.javac.tree.JCTree.JCTypeApply;
import com.sun.tools.javac.tree.JCTree.JCTypeParameter;
import com.sun.tools.javac.tree.JCTree.JCVariableDecl;
import com.sun.tools.javac.tree.JCTree.JCWildcard;
import com.sun.tools.javac.util.Context;
import java.io.IOException;
import java.nio.charset.Charset;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Set;
import javax.lang.model.element.Element;
import javax.lang.model.element.ElementKind;
import javax.lang.model.element.Name;

/** {@link JCTreeScanner} that emits Kythe nodes and edges. */
public class KytheTreeScanner extends JCTreeScanner<JavaNode, TreeContext> {
  private static final FormattingLogger logger = FormattingLogger.getLogger(KytheTreeScanner.class);
  private final boolean verboseLogging;

  private final JavaEntrySets entrySets;
  private final StatisticsCollector statistics;
  // TODO(schroederc): refactor SignatureGenerator for new schema names
  private final SignatureGenerator signatureGenerator;
  private final Positions filePositions;
  private final Map<Integer, List<SourceText.Comment>> comments = new HashMap<>();
  private final Context javaContext;

  private KytheDocTreeScanner docScanner;

  private KytheTreeScanner(
      JavaEntrySets entrySets,
      StatisticsCollector statistics,
      SignatureGenerator signatureGenerator,
      SourceText src,
      Context javaContext,
      boolean verboseLogging) {
    this.entrySets = entrySets;
    this.statistics = statistics;
    this.signatureGenerator = signatureGenerator;
    this.filePositions = src.getPositions();
    this.javaContext = javaContext;
    this.verboseLogging = verboseLogging;

    for (SourceText.Comment comment : src.getComments()) {
      for (int line = comment.lineSpan.getStart(); line <= comment.lineSpan.getEnd(); line++) {
        if (comments.containsKey(line)) {
          comments.get(line).add(comment);
        } else {
          comments.put(line, Lists.newArrayList(comment));
        }
      }
    }
  }

  public static void emitEntries(
      Context javaContext,
      StatisticsCollector statistics,
      JavaEntrySets entrySets,
      SignatureGenerator signatureGenerator,
      JCCompilationUnit compilation,
      Charset sourceEncoding,
      boolean verboseLogging)
      throws IOException {
    SourceText src = new SourceText(javaContext, compilation, sourceEncoding);
    new KytheTreeScanner(
            entrySets, statistics, signatureGenerator, src, javaContext, verboseLogging)
        .scan(compilation, null);
  }

  /** Returns the {@link Symtab} (symbol table) for the compilation currently being processed. */
  public Symtab getSymbols() {
    return Symtab.instance(javaContext);
  }

  @Override
  public JavaNode visitTopLevel(JCCompilationUnit compilation, TreeContext owner) {
    if (compilation.docComments != null) {
      docScanner = new KytheDocTreeScanner(this, compilation.docComments);
    }
    TreeContext ctx = new TreeContext(filePositions, compilation);

    EntrySet fileNode = entrySets.getFileNode(filePositions);

    List<JavaNode> decls = scanList(compilation.getTypeDecls(), ctx);
    decls.removeAll(Collections.singleton(null));

    if (compilation.getPackageName() != null) {
      EntrySet pkgNode = entrySets.getPackageNode(compilation.packge);
      emitAnchor(ctx.down((JCTree) compilation.getPackageName()), EdgeKind.REF, pkgNode);
      for (JavaNode n : decls) {
        entrySets.emitEdge(n.entries, EdgeKind.CHILDOF, pkgNode);
      }
    }

    scan(compilation.getImports(), ctx);
    scan(compilation.getPackageAnnotations(), ctx);
    return new JavaNode(fileNode, filePositions.getFilename());
  }

  @Override
  public JavaNode visitImport(JCImport imprt, TreeContext owner) {
    TreeContext ctx = owner.downAsSnippet(imprt);

    if (imprt.qualid instanceof JCFieldAccess) {
      JCFieldAccess imprtField = (JCFieldAccess) imprt.qualid;

      if (imprt.staticImport) {
        // In static imports, the "field access" is of the form "import static <class>.<method>;".
        // This branch tries to discover the class symbol for "<class>" and emit a reference for it.

        ClassSymbol cls = JavacUtil.getClassSymbol(javaContext, imprtField.selected.toString());
        if (cls != null) {
          com.sun.tools.javac.util.Name className = cls.fullname;
          if (className != null) {
            int dotIdx = cls.fullname.lastIndexOf((byte) '.');
            if (dotIdx >= 0) {
              className = className.subName(dotIdx + 1, className.length());
            }
            emitNameUsage(ctx.down(imprtField.selected), cls, className);
          }
        }
      } else {
        // In non-static imports, the "field access" is of the form "import <package>.<class>;".
        // This branch emits a node for the referenced package.
        emitAnchor(
            ctx.down(imprtField.selected),
            EdgeKind.REF,
            entrySets.getPackageNode(imprtField.selected.toString()));
      }

      if (imprtField.name.toString().equals("*")) {
        return null;
      }

      Symbol sym = imprtField.sym;
      if (sym == null && imprt.isStatic()) {
        // Static imports don't have their symbol populated so we search for the symbol.

        ClassSymbol cls =
            JavacUtil.getClassSymbol(javaContext, imprtField.selected + "." + imprtField.name);
        if (cls != null) {
          // Import was a inner class import
          sym = cls;
        } else {
          cls = JavacUtil.getClassSymbol(javaContext, imprtField.selected.toString());
          if (cls != null) {
            // Import may be a class member
            sym = cls.members().findFirst(imprtField.name);
          }
        }
      }

      return emitNameUsage(ctx.down(imprtField), sym, imprtField.name, EdgeKind.REF_IMPORTS);
    }
    return scan(imprt.qualid, ctx);
  }

  @Override
  public JavaNode visitIdent(JCIdent ident, TreeContext owner) {
    return emitSymUsage(owner.down(ident), ident.sym);
  }

  @Override
  public JavaNode visitClassDef(JCClassDecl classDef, TreeContext owner) {
    TreeContext ctx = owner.down(classDef);

    Optional<String> signature = signatureGenerator.getSignature(classDef.sym);
    if (signature.isPresent()) {
      EntrySet classNode = entrySets.getNode(classDef.sym, signature.get());
      boolean documented = visitDocComment(classDef, classNode);

      Span classIdent =
          filePositions.findIdentifier(classDef.name, classDef.getPreferredPosition());
      if (classIdent != null) {
        EntrySet anchor = entrySets.getAnchor(filePositions, classIdent, ctx.getSnippet());
        emitAnchor(anchor, EdgeKind.DEFINES_BINDING, classNode);
      }
      emitAnchor(ctx, EdgeKind.DEFINES, classNode);
      if (!documented) {
        emitComment(classDef, classNode);
      }

      EntrySet absNode = defineTypeParameters(ctx, classNode, classDef.getTypeParameters());
      if (absNode != null) {
        List<String> tParamNames = new LinkedList<>();
        for (JCTypeParameter tParam : classDef.getTypeParameters()) {
          tParamNames.add(tParam.getName().toString());
        }
        Span bracketGroup = filePositions.findBracketGroup(classDef.getPreferredPosition());
        if (bracketGroup != null) {
          if (classIdent != null) {
            EntrySet absAnchor =
                entrySets.getAnchor(
                    filePositions,
                    new Span(classIdent.getStart(), bracketGroup.getEnd()),
                    ctx.getSnippet());
            emitAnchor(absAnchor, EdgeKind.DEFINES_BINDING, absNode);
          }
        } else {
          logger.warning("Missing bracket group for generic class definition: " + classDef.sym);
        }
        if (!documented) {
          emitComment(classDef, absNode);
        }
        entrySets.emitName(absNode, signature.get() + "<" + Joiner.on(",").join(tParamNames) + ">");
      }

      visitAnnotations(classNode, classDef.getModifiers().getAnnotations(), ctx);

      JavaNode superClassNode = scan(classDef.getExtendsClause(), ctx);
      emitEdge(
          classNode,
          EdgeKind.EXTENDS,
          superClassNode == null ? getJavaLangObjectNode() : superClassNode);

      for (JCExpression implClass : classDef.getImplementsClause()) {
        JavaNode implNode = scan(implClass, ctx);
        if (implNode == null) {
          statistics.incrementCounter("warning-missing-implements-node");
          logger.warning(
              "Missing 'implements' node for " + implClass.getClass() + ": " + implClass);
          continue;
        }
        emitEdge(classNode, EdgeKind.EXTENDS, implNode);
      }

      for (JCTree member : classDef.getMembers()) {
        JavaNode n = scan(member, ctx);
        if (n != null) {
          entrySets.emitEdge(n.entries, EdgeKind.CHILDOF, classNode);
        }
      }

      return new JavaNode(classNode, signature.get());
    }
    return todoNode(ctx, "JCClass: " + classDef);
  }

  @Override
  public JavaNode visitMethodDef(JCMethodDecl methodDef, TreeContext owner) {
    TreeContext ctx = owner.down(methodDef);

    scan(methodDef.getThrows(), ctx);

    JavaNode returnType = scan(methodDef.getReturnType(), ctx);
    List<JavaNode> params = scanList(methodDef.getParameters(), ctx);
    List<JavaNode> paramTypes = new LinkedList<>();
    List<String> paramTypeNames = new LinkedList<>();
    for (JavaNode n : params) {
      paramTypes.add(n.typeNode);
      paramTypeNames.add(n.typeNode.qualifiedName);
    }

    Optional<String> signature = signatureGenerator.getSignature(methodDef.sym);
    if (signature.isPresent()) {
      EntrySet methodNode = entrySets.getNode(methodDef.sym, signature.get());
      boolean documented = visitDocComment(methodDef, methodNode);
      visitAnnotations(methodNode, methodDef.getModifiers().getAnnotations(), ctx);

      EntrySet absNode = defineTypeParameters(ctx, methodNode, methodDef.getTypeParameters());

      EntrySet ret, bindingAnchor = null;
      String fnTypeName = "(" + Joiner.on(",").join(paramTypeNames) + ")";
      if (methodDef.sym.isConstructor()) {
        // Implicit constructors (those without syntactic definition locations) share the same
        // preferred position as their owned class.  Since implicit constructors don't exist in the
        // file's text, don't generate anchors them by ensuring the constructor's position is ahead
        // of the owner's position.
        if (methodDef.getPreferredPosition() > owner.getTree().getPreferredPosition()) {
          // Use the owner's name (the class name) to find the definition anchor's
          // location because constructors are internally named "<init>".
          bindingAnchor =
              emitAnchor(
                  methodDef.sym.owner.name,
                  methodDef.getPreferredPosition(),
                  EdgeKind.DEFINES_BINDING,
                  methodNode,
                  ctx.getSnippet());
        }
        // Likewise, constructors don't have return types in the Java AST, but
        // Kythe models all functions with return types.  As a solution, we use
        // the class type as the return type for all constructors.
        ret = getNode(methodDef.sym.owner);
      } else {
        bindingAnchor =
            emitAnchor(
                methodDef.name,
                methodDef.getPreferredPosition(),
                EdgeKind.DEFINES_BINDING,
                methodNode,
                ctx.getSnippet());
        ret = returnType.entries;
        fnTypeName = returnType.qualifiedName + fnTypeName;
      }

      if (bindingAnchor != null) {
        if (!documented) {
          emitComment(methodDef, methodNode);
        }
        if (absNode != null) {
          emitAnchor(bindingAnchor, EdgeKind.DEFINES_BINDING, absNode);
          if (!documented) {
            emitComment(methodDef, absNode);
          }
        }
        emitAnchor(ctx, EdgeKind.DEFINES, methodNode);
      }

      emitOrdinalEdges(methodNode, EdgeKind.PARAM, params);
      EntrySet fnTypeNode = entrySets.newFunctionType(ret, toEntries(paramTypes));
      entrySets.emitEdge(methodNode, EdgeKind.TYPED, fnTypeNode);
      entrySets.emitName(fnTypeNode, fnTypeName);

      for (JavaNode n : params) {
        emitEdge(methodNode, EdgeKind.CHILDOF, n);
      }

      ClassSymbol ownerClass = (ClassSymbol) methodDef.sym.owner;
      Set<Element> ownerDirectSupertypes = new HashSet<>();
      ownerDirectSupertypes.add(ownerClass.getSuperclass().asElement());
      for (Type interfaceParent : ownerClass.getInterfaces()) {
        ownerDirectSupertypes.add(interfaceParent.asElement());
      }
      for (MethodSymbol superMethod : JavacUtil.superMethods(javaContext, methodDef.sym)) {
        EntrySet superNode = getNode(superMethod);
        if (ownerDirectSupertypes.contains(superMethod.owner)) {
          entrySets.emitEdge(methodNode, EdgeKind.OVERRIDES, superNode);
        } else {
          entrySets.emitEdge(methodNode, EdgeKind.OVERRIDES_TRANSITIVE, superNode);
        }
      }

      // Set the resulting node for the method and then recurse through its body.  Setting the node
      // first is necessary to correctly add childof edges in the callgraph.
      JavaNode node =
          ctx.setNode(
              new JavaNode(methodNode, signature.get(), new JavaNode(fnTypeNode, fnTypeName)));
      scan(methodDef.getBody(), ctx);

      return node;
    } else {
      // Try to scan method body even if signature could not be generated.
      scan(methodDef.getBody(), ctx);
    }
    return todoNode(ctx, "MethodDef: " + methodDef);
  }

  @Override
  public JavaNode visitVarDef(JCVariableDecl varDef, TreeContext owner) {
    TreeContext ctx = owner.downAsSnippet(varDef);

    Optional<String> signature = signatureGenerator.getSignature(varDef.sym);
    if (signature.isPresent()) {
      EntrySet varNode = entrySets.getNode(varDef.sym, signature.get());
      boolean documented = visitDocComment(varDef, varNode);
      emitAnchor(
          varDef.name,
          varDef.getStartPosition(),
          EdgeKind.DEFINES_BINDING,
          varNode,
          ctx.getSnippet());
      emitAnchor(ctx, EdgeKind.DEFINES, varNode);
      if (varDef.sym.getKind().isField() && !documented) {
        // emit comments for fields and enumeration constants
        emitComment(varDef, varNode);
      }

      visitAnnotations(varNode, varDef.getModifiers().getAnnotations(), ctx);

      JavaNode typeNode = scan(varDef.getType(), ctx);
      if (typeNode != null) {
        emitEdge(varNode, EdgeKind.TYPED, typeNode);
      }

      scan(varDef.getInitializer(), ctx);
      return new JavaNode(varNode, signature.get(), typeNode);
    }
    return todoNode(ctx, "VarDef: " + varDef);
  }

  @Override
  public JavaNode visitTypeApply(JCTypeApply tApply, TreeContext owner) {
    TreeContext ctx = owner.down(tApply);

    JavaNode typeCtorNode = scan(tApply.getType(), ctx);

    List<JavaNode> arguments = scanList(tApply.getTypeArguments(), ctx);
    List<EntrySet> argEntries = new LinkedList<>();
    List<String> argNames = new LinkedList<>();
    for (JavaNode n : arguments) {
      argEntries.add(n.entries);
      argNames.add(n.qualifiedName);
    }

    EntrySet typeNode = entrySets.newTApply(typeCtorNode.entries, argEntries);
    emitAnchor(ctx, EdgeKind.REF, typeNode);

    String qualifiedName = typeCtorNode.qualifiedName + "<" + Joiner.on(',').join(argNames) + ">";
    entrySets.emitName(typeNode, qualifiedName);

    return new JavaNode(typeNode, qualifiedName);
  }

  @Override
  public JavaNode visitSelect(JCFieldAccess field, TreeContext owner) {
    TreeContext ctx = owner.down(field);
    if (field.sym == null) {
      // TODO(schroederc): determine exactly why this occurs
      scan(field.getExpression(), ctx);
      return null;
    } else if (field.sym.getKind() == ElementKind.PACKAGE) {
      EntrySet pkgNode = entrySets.getPackageNode((PackageSymbol) field.sym);
      emitAnchor(ctx, EdgeKind.REF, pkgNode);
      return new JavaNode(pkgNode, field.sym.toString());
    } else {
      scan(field.getExpression(), ctx);
      return emitNameUsage(ctx, field.sym, field.name);
    }
  }

  @Override
  public JavaNode visitReference(JCMemberReference reference, TreeContext owner) {
    TreeContext ctx = owner.down(reference);
    scan(reference.getQualifierExpression(), ctx);
    return emitNameUsage(
        ctx,
        reference.sym,
        reference.getMode() == ReferenceMode.NEW ? Keyword.of("new") : reference.name);
  }

  @Override
  public JavaNode visitApply(JCMethodInvocation invoke, TreeContext owner) {
    TreeContext ctx = owner.down(invoke);
    scan(invoke.getArguments(), ctx);
    scan(invoke.getTypeArguments(), ctx);

    JavaNode method = scan(invoke.getMethodSelect(), ctx);
    if (method != null) {
      EntrySet anchor = emitAnchor(ctx, EdgeKind.REF_CALL, method.entries);
      TreeContext parentContext = owner.getMethodParent();
      if (anchor != null && parentContext != null && parentContext.getNode() != null) {
        emitEdge(anchor, EdgeKind.CHILDOF, parentContext.getNode());
      }
      return method;
    }

    return todoNode(ctx, "Apply: " + invoke);
  }

  @Override
  public JavaNode visitNewClass(JCNewClass newClass, TreeContext owner) {
    TreeContext ctx = owner.down(newClass);

    EntrySet ctorNode = getNode(newClass.constructor);
    if (ctorNode != null) {
      // Span over "new Class"
      EntrySet anchor =
          entrySets.getAnchor(
              filePositions,
              new Span(
                  filePositions.getStart(newClass), filePositions.getEnd(newClass.getIdentifier())),
              ctx.getSnippet());
      emitAnchor(anchor, EdgeKind.REF, ctorNode);

      scanList(newClass.getTypeArguments(), ctx);
      scanList(newClass.getArguments(), ctx);
      scan(newClass.getEnclosingExpression(), ctx);
      scan(newClass.getClassBody(), ctx);
      return scan(newClass.getIdentifier(), ctx);
    }
    return todoNode(ctx, "NewClass: " + newClass);
  }

  @Override
  public JavaNode visitTypeIdent(JCPrimitiveTypeTree primitiveType, TreeContext owner) {
    TreeContext ctx = owner.down(primitiveType);
    if (verboseLogging && primitiveType.typetag == TypeTag.ERROR) {
      logger.warning("found primitive ERROR type: " + ctx);
    }
    String name = primitiveType.typetag.toString().toLowerCase();
    EntrySet node = entrySets.getBuiltin(name);
    emitAnchor(ctx, EdgeKind.REF, node);
    return new JavaNode(node, name);
  }

  @Override
  public JavaNode visitTypeArray(JCArrayTypeTree arrayType, TreeContext owner) {
    TreeContext ctx = owner.down(arrayType);

    JavaNode typeNode = scan(arrayType.getType(), ctx);
    EntrySet node =
        entrySets.newTApply(entrySets.getBuiltin("array"), Arrays.asList(typeNode.entries));
    emitAnchor(ctx, EdgeKind.REF, node);
    JavaNode arrayNode = new JavaNode(node, typeNode.qualifiedName + "[]");
    entrySets.emitName(node, arrayNode.qualifiedName);
    return arrayNode;
  }

  @Override
  public JavaNode visitAnnotation(JCAnnotation annotation, TreeContext owner) {
    TreeContext ctx = owner.down(annotation);

    return scan(annotation.getAnnotationType(), ctx);
  }

  @Override
  public JavaNode visitWildcard(JCWildcard wild, TreeContext owner) {
    TreeContext ctx = owner.down(wild);

    EntrySet node = entrySets.getWildcardNode(wild);
    String signature = wild.kind.kind.toString();
    if (wild.getKind() != Kind.UNBOUNDED_WILDCARD) {
      JavaNode bound = scan(wild.getBound(), ctx);
      signature += bound.qualifiedName;
      emitEdge(
          node,
          wild.getKind() == Kind.EXTENDS_WILDCARD ? EdgeKind.BOUNDED_UPPER : EdgeKind.BOUNDED_LOWER,
          bound);
    }
    return new JavaNode(node, signature);
  }

  @Override
  public JavaNode visitExec(JCExpressionStatement stmt, TreeContext owner) {
    return scan(stmt.expr, owner.downAsSnippet(stmt));
  }

  @Override
  public JavaNode visitReturn(JCReturn ret, TreeContext owner) {
    return scan(ret.expr, owner.downAsSnippet(ret));
  }

  @Override
  public JavaNode visitThrow(JCThrow thr, TreeContext owner) {
    return scan(thr.expr, owner.downAsSnippet(thr));
  }

  @Override
  public JavaNode visitAssert(JCAssert azzert, TreeContext owner) {
    return scanAll(owner.downAsSnippet(azzert), azzert.cond, azzert.detail);
  }

  @Override
  public JavaNode visitAssign(JCAssign assgn, TreeContext owner) {
    return scanAll(owner.downAsSnippet(assgn), assgn.lhs, assgn.rhs);
  }

  @Override
  public JavaNode visitAssignOp(JCAssignOp assgnOp, TreeContext owner) {
    return scanAll(owner.downAsSnippet(assgnOp), assgnOp.lhs, assgnOp.rhs);
  }

  private boolean visitDocComment(JCTree tree, EntrySet node) {
    return docScanner != null && docScanner.visitDocComment(tree, node);
  }

  //// Utility methods ////

  void emitDocReference(Symbol sym, int startChar, int endChar) {
    EntrySet node = getNode(sym);
    if (node == null) {
      if (verboseLogging) {
        logger.warning("failed to emit documentation reference to " + sym);
      }
      return;
    }

    Span loc =
        new Span(
            filePositions.charToByteOffset(startChar), filePositions.charToByteOffset(endChar));
    EntrySet anchor = entrySets.getAnchor(filePositions, loc);
    if (anchor != null) {
      emitAnchor(anchor, EdgeKind.REF_DOC, node);
    }
  }

  int charToLine(int charPosition) {
    return filePositions.charToLine(charPosition);
  }

  boolean emitCommentsOnLine(int line, EntrySet node) {
    List<SourceText.Comment> lst = comments.get(line);
    if (lst != null) {
      for (SourceText.Comment comment : lst) {
        commentAnchor(comment, node);
      }
      return !lst.isEmpty();
    }
    return false;
  }

  private static List<EntrySet> toEntries(Iterable<JavaNode> nodes) {
    List<EntrySet> entries = new LinkedList<>();
    for (JavaNode n : nodes) {
      entries.add(n.entries);
    }
    return entries;
  }

  private EntrySet defineTypeParameters(
      TreeContext ownerContext, EntrySet owner, List<JCTypeParameter> params) {
    if (params.isEmpty()) {
      return null;
    }

    List<EntrySet> typeParams = new LinkedList<>();
    for (JCTypeParameter tParam : params) {
      TreeContext ctx = ownerContext.down(tParam);
      EntrySet node = getNode(tParam.type.asElement());
      emitAnchor(
          tParam.name, tParam.getStartPosition(), EdgeKind.DEFINES_BINDING, node, ctx.getSnippet());
      visitAnnotations(node, tParam.getAnnotations(), ctx);
      typeParams.add(node);

      List<JCExpression> bounds = tParam.getBounds();
      if (bounds.size() == 0) {
        emitEdge(node, EdgeKind.BOUNDED_UPPER, getJavaLangObjectNode());
      } else {
        for (JCExpression expr : bounds) {
          emitEdge(node, EdgeKind.BOUNDED_UPPER, scan(expr, ctx));
        }
      }
    }

    return entrySets.newAbstract(owner, typeParams);
  }

  /** Returns the node associated with a {@link Symbol} or {@code null}. */
  private EntrySet getNode(Symbol sym) {
    JavaNode node = getJavaNode(sym);
    return node == null ? null : node.entries;
  }

  /** Returns the {@link JavaNode} associated with a {@link Symbol} or {@code null}. */
  private JavaNode getJavaNode(Symbol sym) {
    Optional<String> signature = signatureGenerator.getSignature(sym);
    if (!signature.isPresent()) {
      return null;
    }
    return new JavaNode(entrySets.getNode(sym, signature.get()), signature.get());
  }

  private void visitAnnotations(
      EntrySet owner, List<JCAnnotation> annotations, TreeContext ownerContext) {
    for (JavaNode node : scanList(annotations, ownerContext)) {
      entrySets.emitEdge(owner, EdgeKind.ANNOTATED_BY, node.entries);
    }
  }

  // Emits a node for the given sym, an anchor encompassing the TreeContext, and a REF edge
  private JavaNode emitSymUsage(TreeContext ctx, Symbol sym) {
    JavaNode node = getJavaNode(sym);
    if (node == null) {
      return todoNode(ctx, "ExprUsage: " + ctx.getTree());
    }

    emitAnchor(ctx, EdgeKind.REF, node.entries);
    statistics.incrementCounter("symbol-usages-emitted");
    return node;
  }

  // Emits a node for the given sym, an anchor encompassing the name, and a REF edge
  private JavaNode emitNameUsage(TreeContext ctx, Symbol sym, Name name) {
    return emitNameUsage(ctx, sym, name, EdgeKind.REF);
  }

  // Emits a node for the given sym, an anchor encompassing the name, and a given edge kind
  private JavaNode emitNameUsage(TreeContext ctx, Symbol sym, Name name, EdgeKind edgeKind) {
    JavaNode node = getJavaNode(sym);
    if (node == null) {
      return todoNode(ctx, "NameUsage: " + ctx.getTree() + " -- " + name);
    }

    emitAnchor(name, ctx.getTree().getStartPosition(), edgeKind, node.entries, ctx.getSnippet());
    statistics.incrementCounter("name-usages-emitted");
    return node;
  }

  // Returns a JavaNode representing java.lang.Object.
  private JavaNode getJavaLangObjectNode() {
    Symbol javaLangObject = getSymbols().objectType.asElement();
    String javaLangObjectSignature = signatureGenerator.getSignature(javaLangObject).get();
    EntrySet javaLangObjectEntrySet = entrySets.getNode(javaLangObject, javaLangObjectSignature);
    return new JavaNode(javaLangObjectEntrySet, javaLangObjectSignature);
  }

  // Creates/emits an anchor and an associated edge
  private EntrySet emitAnchor(TreeContext anchorContext, EdgeKind kind, EntrySet node) {
    return emitAnchor(
        entrySets.getAnchor(filePositions, anchorContext.getTreeSpan(), anchorContext.getSnippet()),
        kind,
        node);
  }

  // Creates/emits an anchor (for an identifier) and an associated edge
  private EntrySet emitAnchor(
      Name name, int startOffset, EdgeKind kind, EntrySet node, Span snippet) {
    EntrySet anchor = entrySets.getAnchor(filePositions, name, startOffset, snippet);
    if (anchor == null) {
      // TODO(schroederc): Special-case these anchors (most come from visitSelect)
      return null;
    }
    return emitAnchor(anchor, kind, node);
  }

  // Creates/emits an anchor and an associated edge
  private EntrySet emitAnchor(EntrySet anchor, EdgeKind kind, EntrySet node) {
    Preconditions.checkArgument(
        kind.isAnchorEdge(), "EdgeKind was not intended for ANCHORs: " + kind);
    if (anchor == null) {
      return null;
    }
    entrySets.emitEdge(anchor, kind, node);
    return anchor;
  }

  private void emitComment(JCTree defTree, EntrySet node) {
    int defPosition = defTree.getPreferredPosition();
    int defLine = filePositions.charToLine(defPosition);
    emitCommentsOnLine(defLine, node);
    emitCommentsOnLine(defLine - 1, node);
  }

  void emitDoc(String bracketedText, Iterable<Symbol> params, EntrySet node) {
    List<EntrySet> paramNodes = Lists.newArrayList();
    for (Symbol s : params) {
      EntrySet paramNode = getNode(s);
      if (paramNode == null) {
        return;
      }
      paramNodes.add(paramNode);
    }
    EntrySet doc = entrySets.getDoc(filePositions, bracketedText, paramNodes);
    entrySets.emitEdge(doc, EdgeKind.DOCUMENTS, node);
  }

  private EntrySet commentAnchor(SourceText.Comment comment, EntrySet node) {
    return emitAnchor(
        entrySets.getAnchor(filePositions, comment.byteSpan), EdgeKind.DOCUMENTS, node);
  }

  // Unwraps the target EntrySet and emits an edge to it from the sourceNode
  private void emitEdge(EntrySet sourceNode, EdgeKind kind, JavaNode target) {
    entrySets.emitEdge(sourceNode, kind, target.entries);
  }

  // Unwraps each target EntrySet and emits an ordinal edge to each from the given source node
  private void emitOrdinalEdges(EntrySet node, EdgeKind kind, List<JavaNode> targets) {
    List<EntrySet> entries = new LinkedList<>();
    for (JavaNode n : targets) {
      entries.add(n.entries);
    }
    entrySets.emitOrdinalEdges(node, kind, entries);
  }

  @Deprecated
  private JavaNode todoNode(TreeContext ctx, String message) {
    return new JavaNode(
        entrySets.todoNode(ctx.getSourceName(), ctx.getTree(), message),
        "TODO",
        new JavaNode(
            entrySets.todoNode(ctx.getSourceName(), ctx.getTree(), "type:" + message),
            "TODO:type"));
  }

  private <T extends JCTree> List<JavaNode> scanList(List<T> trees, TreeContext owner) {
    List<JavaNode> nodes = new LinkedList<>();
    for (T t : trees) {
      nodes.add(scan(t, owner));
    }
    return nodes;
  }
}

class TreeContext {
  private final Positions filePositions;
  private final TreeContext up;
  private final JCTree tree;
  private final Span snippet;
  private JavaNode node;

  public TreeContext(Positions filePositions, JCCompilationUnit topLevel) {
    this(filePositions, null, topLevel, null);
  }

  private TreeContext(Positions filePositions, TreeContext up, JCTree tree, Span snippet) {
    this.filePositions = filePositions;
    this.up = up;
    this.tree = tree;
    this.snippet = snippet;
  }

  public String getSourceName() {
    return filePositions.getFilename();
  }

  public TreeContext down(JCTree tree) {
    return new TreeContext(filePositions, this, tree, snippet);
  }

  public TreeContext downAsSnippet(JCTree tree) {
    return new TreeContext(filePositions, this, tree, filePositions.getSpan(tree));
  }

  public TreeContext up() {
    return up;
  }

  public JCTree getTree() {
    return tree;
  }

  public JavaNode setNode(JavaNode node) {
    if (this.node != null) {
      throw new IllegalStateException("cannot call setNode() twice on same TreeContext");
    }
    this.node = node;
    return node;
  }

  public JavaNode getNode() {
    return node;
  }

  public TreeContext getMethodParent() {
    TreeContext parent = up();
    while (parent != null && !(parent.getTree() instanceof JCMethodDecl)) {
      parent = parent.up();
    }
    return parent;
  }

  public Span getTreeSpan() {
    return filePositions.getSpan(tree);
  }

  public Span getSnippet() {
    return snippet;
  }

  @Override
  public String toString() {
    List<String> parts = new LinkedList<>();
    parts.add("JCTree = " + tree);
    if (node != null) {
      parts.add("JavaNode = " + node);
    }
    parts.add("TreeSpan = " + getTreeSpan());
    if (snippet != null) {
      parts.add("Snippet span = " + snippet);
    }
    List<String> parents = new LinkedList<>();
    TreeContext cur = up;
    while (cur != null) {
      String parent = "" + cur.getTree().getTag();
      String name = NameVisitor.getName(cur.getTree());
      if (name != null) {
        parent += "[" + name + "]";
      }
      parents.add(parent);
      cur = cur.up();
    }
    if (!parents.isEmpty()) {
      parts.add("JCTree parents = " + parents);
    }
    return String.format("TreeContext{\n  %s,\n}", Joiner.on(",\n  ").join(parts));
  }
}

class JavaNode {
  // TODO(schroederc): clearly separate semantic/type nodes
  final EntrySet entries;
  final JavaNode typeNode;
  final String qualifiedName;

  public JavaNode(EntrySet entries, String qualifiedName) {
    this(entries, qualifiedName, null);
  }

  public JavaNode(EntrySet entries, String qualifiedName, JavaNode typeNode) {
    this.entries = entries;
    this.qualifiedName = qualifiedName;
    this.typeNode = typeNode;
  }

  @Override
  public String toString() {
    return "JavaNode{" + qualifiedName + "}";
  }
}
