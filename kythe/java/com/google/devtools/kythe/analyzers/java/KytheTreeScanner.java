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
import com.google.common.base.Preconditions;
import com.google.common.collect.ImmutableList;
import com.google.common.collect.ImmutableList.Builder;
import com.google.common.collect.Lists;
import com.google.common.io.ByteStreams;
import com.google.devtools.kythe.analyzers.base.EdgeKind;
import com.google.devtools.kythe.analyzers.base.EntrySet;
import com.google.devtools.kythe.analyzers.java.SourceText.Comment;
import com.google.devtools.kythe.analyzers.java.SourceText.Keyword;
import com.google.devtools.kythe.analyzers.java.SourceText.Positions;
import com.google.devtools.kythe.common.FormattingLogger;
import com.google.devtools.kythe.platform.java.filemanager.JavaFileStoreBasedFileManager;
import com.google.devtools.kythe.platform.java.helpers.JCTreeScanner;
import com.google.devtools.kythe.platform.java.helpers.JavacUtil;
import com.google.devtools.kythe.platform.java.helpers.SignatureGenerator;
import com.google.devtools.kythe.platform.shared.Metadata;
import com.google.devtools.kythe.platform.shared.MetadataLoaders;
import com.google.devtools.kythe.platform.shared.StatisticsCollector;
import com.google.devtools.kythe.proto.Diagnostic;
import com.google.devtools.kythe.proto.MarkedSource;
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
import com.sun.tools.javac.tree.JCTree.JCLiteral;
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
import java.io.InputStream;
import java.net.URI;
import java.nio.charset.Charset;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.Set;
import java.util.stream.Collectors;
import javax.lang.model.element.Element;
import javax.lang.model.element.ElementKind;
import javax.lang.model.element.Name;
import javax.tools.FileObject;
import javax.tools.JavaFileObject;

/** {@link JCTreeScanner} that emits Kythe nodes and edges. */
public class KytheTreeScanner extends JCTreeScanner<JavaNode, TreeContext> {
  private static final FormattingLogger logger = FormattingLogger.getLogger(KytheTreeScanner.class);

  /** Maximum allowed text size for variable {@link MarkedSource.Kind.INITIALIZER}s */
  private static final int MAX_INITIALIZER_LENGTH = 80;

  private final boolean verboseLogging;

  private final JavaEntrySets entrySets;
  private final StatisticsCollector statistics;
  // TODO(schroederc): refactor SignatureGenerator for new schema names
  private final SignatureGenerator signatureGenerator;
  private final Positions filePositions;
  private final Map<Integer, List<Comment>> comments = new HashMap<>();
  private final Context javaContext;
  private final JavaFileStoreBasedFileManager fileManager;
  private final MetadataLoaders metadataLoaders;
  private List<Metadata> metadata;

  private KytheDocTreeScanner docScanner;

  private KytheTreeScanner(
      JavaEntrySets entrySets,
      StatisticsCollector statistics,
      SignatureGenerator signatureGenerator,
      SourceText src,
      Context javaContext,
      boolean verboseLogging,
      JavaFileStoreBasedFileManager fileManager,
      MetadataLoaders metadataLoaders) {
    this.entrySets = entrySets;
    this.statistics = statistics;
    this.signatureGenerator = signatureGenerator;
    this.filePositions = src.getPositions();
    this.javaContext = javaContext;
    this.verboseLogging = verboseLogging;
    this.fileManager = fileManager;
    this.metadataLoaders = metadataLoaders;

    for (Comment comment : src.getComments()) {
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
      boolean verboseLogging,
      JavaFileStoreBasedFileManager fileManager,
      MetadataLoaders metadataLoaders)
      throws IOException {
    SourceText src = new SourceText(javaContext, compilation, sourceEncoding);
    new KytheTreeScanner(
            entrySets,
            statistics,
            signatureGenerator,
            src,
            javaContext,
            verboseLogging,
            fileManager,
            metadataLoaders)
        .scan(compilation, null);
  }

  /** Returns the {@link Symtab} (symbol table) for the compilation currently being processed. */
  public Symtab getSymbols() {
    return Symtab.instance(javaContext);
  }

  @Override
  public JavaNode visitTopLevel(JCCompilationUnit compilation, TreeContext owner) {
    if (compilation.docComments != null) {
      docScanner = new KytheDocTreeScanner(this, javaContext);
    }
    TreeContext ctx = new TreeContext(filePositions, compilation);
    metadata = Lists.newArrayList();

    EntrySet fileNode = entrySets.newFileNodeAndEmit(filePositions);

    List<JavaNode> decls = scanList(compilation.getTypeDecls(), ctx);
    decls.removeAll(Collections.singleton(null));

    if (compilation.getPackageName() != null) {
      EntrySet pkgNode = entrySets.newPackageNodeAndEmit(compilation.packge);
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
            entrySets.newPackageNodeAndEmit(imprtField.selected.toString()));
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
            // Import is a class member; emit usages for all matching (by name) class members.
            ctx = ctx.down(imprtField);
            JavaNode lastMember = null;
            for (Symbol member : cls.members().getSymbolsByName(imprtField.name)) {
              try {
                // Ensure member symbol's type is complete.  If the extractor finds that a static
                // member isn't used (due to overloads), the symbol's dependent type classes won't
                // be saved in the CompilationUnit and this will throw an exception.
                if (member.type != null) {
                  member.type.tsym.complete();
                }

                lastMember = emitNameUsage(ctx, member, imprtField.name, EdgeKind.REF_IMPORTS);
              } catch (Symbol.CompletionFailure e) {
                // Symbol resolution failed (see above comment).  Ignore and continue with other
                // class members matching static import.
              }
            }
            return lastMember;
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
    loadAnnotationsFromClassDecl(classDef);
    TreeContext ctx = owner.down(classDef);

    Optional<String> signature = signatureGenerator.getSignature(classDef.sym);
    if (!signature.isPresent()) {
      // TODO(schroederc): details
      return emitDiagnostic(ctx, "missing class signature", null, null);
    }

    MarkedSource.Builder markedSource = MarkedSource.newBuilder();
    EntrySet classNode =
        entrySets.getNode(signatureGenerator, classDef.sym, signature.get(), markedSource, null);

    // Find the method or class in which this class is defined, if any.
    TreeContext container = ctx.getClassOrMethodParent();
    // Emit the fact that the class is a child of its containing class or method.
    // Note that for a nested/inner class, we already emitted the fact that it's a
    // child of the containing class when we scanned the containing class's members.
    // However we can't restrict ourselves to just classes contained in methods here,
    // because that would miss the case of local/anonymous classes in static/member
    // initializers. But there's no harm in emitting the same fact twice!
    if (container != null) {
      entrySets.emitEdge(classNode, EdgeKind.CHILDOF, container.getNode().entries);
    }

    Span classIdent = filePositions.findIdentifier(classDef.name, classDef.getPreferredPosition());
    if (!classDef.name.isEmpty() && classIdent == null) {
      logger.warning("Missing span for class identifier: " + classDef.sym);
    }

    // Generic classes record the source range of the class name for the abs node, regular
    // classes record the source range of the class name for the record node.
    EntrySet absNode =
        defineTypeParameters(
            ctx,
            classNode,
            classDef.getTypeParameters(),
            ImmutableList.<EntrySet>of(), /* There are no wildcards in class definitions */
            markedSource.build());

    boolean documented = visitDocComment(classNode, absNode);

    if (absNode != null) {
      List<String> tParamNames = new LinkedList<>();
      for (JCTypeParameter tParam : classDef.getTypeParameters()) {
        tParamNames.add(tParam.getName().toString());
      }
      if (classIdent != null) {
        EntrySet absAnchor =
            entrySets.newAnchorAndEmit(filePositions, classIdent, ctx.getSnippet());
        emitDefinesBindingEdge(classIdent, absAnchor, absNode);
      }
      if (!documented) {
        emitComment(classDef, absNode);
      }
    }
    if (absNode == null && classIdent != null) {
      EntrySet anchor = entrySets.newAnchorAndEmit(filePositions, classIdent, ctx.getSnippet());
      emitDefinesBindingEdge(classIdent, anchor, classNode);
    }
    emitAnchor(ctx, EdgeKind.DEFINES, classNode);
    if (!documented) {
      emitComment(classDef, classNode);
    }

    visitAnnotations(classNode, classDef.getModifiers().getAnnotations(), ctx);

    JavaNode superClassNode = scan(classDef.getExtendsClause(), ctx);
    if (superClassNode == null) {
      // Use the implicit superclass.
      switch (classDef.getKind()) {
        case CLASS:
          superClassNode = getJavaLangObjectNode();
          break;
        case ENUM:
          superClassNode = getJavaLangEnumNode(classNode, signature.get());
          break;
        case ANNOTATION_TYPE:
          // TODO(schroederc): handle annotation superclass
          break;
        case INTERFACE:
          break; // Interfaces have no implicit superclass.
        default:
          logger.warningfmt("Unexpected JCClassDecl kind: %s", classDef.getKind());
          break;
      }
    }

    if (superClassNode != null) {
      emitEdge(classNode, EdgeKind.EXTENDS, superClassNode);
    }

    for (JCExpression implClass : classDef.getImplementsClause()) {
      JavaNode implNode = scan(implClass, ctx);
      if (implNode == null) {
        statistics.incrementCounter("warning-missing-implements-node");
        logger.warning("Missing 'implements' node for " + implClass.getClass() + ": " + implClass);
        continue;
      }
      emitEdge(classNode, EdgeKind.EXTENDS, implNode);
    }

    // Set the resulting node for the class before recursing through its members.  Setting the node
    // first is necessary to correctly add childof edges from local/anonymous classes defined
    // directly in the class body (in static initializers or member initializers).
    JavaNode node = ctx.setNode(new JavaNode(classNode, signature.get()));

    for (JCTree member : classDef.getMembers()) {
      JavaNode n = scan(member, ctx);
      if (n != null) {
        entrySets.emitEdge(n.entries, EdgeKind.CHILDOF, classNode);
      }
    }

    return node;
  }

  @Override
  public JavaNode visitMethodDef(JCMethodDecl methodDef, TreeContext owner) {
    TreeContext ctx = owner.down(methodDef);

    scan(methodDef.getThrows(), ctx);

    JavaNode returnType = scan(methodDef.getReturnType(), ctx);
    List<JavaNode> params = scanList(methodDef.getParameters(), ctx);
    List<JavaNode> paramTypes = new LinkedList<>();
    List<String> paramTypeNames = new LinkedList<>();
    List<EntrySet> wildcards = new LinkedList<>();
    for (int i = 0; i < params.size(); i++) {
      JavaNode n = params.get(i);
      if (n.typeNode == null) {
        logger.warningfmt(
            "Missing parameter type (method: %s; parameter: %s)",
            methodDef.getName(), methodDef.getParameters().get(i));
        continue;
      }
      paramTypes.add(n.typeNode);
      paramTypeNames.add(n.typeNode.qualifiedName);
      wildcards.addAll(n.childWildcards);
    }

    Optional<String> signature = signatureGenerator.getSignature(methodDef.sym);
    if (!signature.isPresent()) {
      // Try to scan method body even if signature could not be generated.
      scan(methodDef.getBody(), ctx);

      // TODO(schroederc): details
      return emitDiagnostic(ctx, "missing method signature", null, null);
    }

    MarkedSource.Builder markedSource = MarkedSource.newBuilder();
    EntrySet methodNode =
        entrySets.getNode(signatureGenerator, methodDef.sym, signature.get(), markedSource, null);
    visitAnnotations(methodNode, methodDef.getModifiers().getAnnotations(), ctx);

    EntrySet absNode =
        defineTypeParameters(
            ctx, methodNode, methodDef.getTypeParameters(), wildcards, markedSource.build());
    boolean documented = visitDocComment(methodNode, absNode);

    EntrySet ret = null;
    EntrySet bindingAnchor = null;
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
            emitDefinesBindingAnchorEdge(
                methodDef.sym.owner.name,
                methodDef.getPreferredPosition(),
                methodNode,
                ctx.getSnippet());
      }
      // Likewise, constructors don't have return types in the Java AST, but
      // Kythe models all functions with return types.  As a solution, we use
      // the class type as the return type for all constructors.
      ret = getNode(methodDef.sym.owner);
    } else {
      bindingAnchor =
          emitDefinesBindingAnchorEdge(
              methodDef.name, methodDef.getPreferredPosition(), methodNode, ctx.getSnippet());
      ret = returnType.entries;
      fnTypeName = returnType.qualifiedName + fnTypeName;
    }

    if (bindingAnchor != null) {
      if (!documented) {
        emitComment(methodDef, methodNode);
      }
      if (absNode != null) {
        emitAnchor(bindingAnchor, EdgeKind.DEFINES_BINDING, absNode);
        Span span = filePositions.findIdentifier(methodDef.name, methodDef.getPreferredPosition());
        if (span != null) {
          emitMetadata(span, absNode);
        }
        if (!documented) {
          emitComment(methodDef, absNode);
        }
      }
      emitAnchor(ctx, EdgeKind.DEFINES, methodNode);
    }

    emitOrdinalEdges(methodNode, EdgeKind.PARAM, params);
    EntrySet fnTypeNode = entrySets.newFunctionTypeAndEmit(ret, toEntries(paramTypes));
    entrySets.emitEdge(methodNode, EdgeKind.TYPED, fnTypeNode);

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

    for (JavaNode param : params) {
      entrySets.emitEdge(param.entries, EdgeKind.CHILDOF, node.entries);
    }

    return node;
  }

  @Override
  public JavaNode visitVarDef(JCVariableDecl varDef, TreeContext owner) {
    TreeContext ctx = owner.downAsSnippet(varDef);

    Optional<String> signature = signatureGenerator.getSignature(varDef.sym);
    if (!signature.isPresent()) {
      // TODO(schroederc): details
      return emitDiagnostic(ctx, "missing variable signature", null, null);
    }

    List<MarkedSource> markedSourceChildren = new ArrayList<>();
    if (varDef.getInitializer() != null) {
      String initializer = varDef.getInitializer().toString();
      if (initializer.length() <= MAX_INITIALIZER_LENGTH) {
        markedSourceChildren.add(
            MarkedSource.newBuilder()
                .setKind(MarkedSource.Kind.INITIALIZER)
                .setPreText(initializer)
                .build());
      }
    }

    EntrySet varNode =
        entrySets.getNode(
            signatureGenerator, varDef.sym, signature.get(), null, markedSourceChildren);
    boolean documented = visitDocComment(varNode, null);
    emitDefinesBindingAnchorEdge(varDef.name, varDef.getStartPosition(), varNode, ctx.getSnippet());
    emitAnchor(ctx, EdgeKind.DEFINES, varNode);
    if (varDef.sym.getKind().isField() && !documented) {
      // emit comments for fields and enumeration constants
      emitComment(varDef, varNode);
    }

    TreeContext parentContext = ctx.getClassOrMethodParent();
    if (parentContext != null && parentContext.getNode() != null) {
      emitEdge(varNode, EdgeKind.CHILDOF, parentContext.getNode());
    }
    visitAnnotations(varNode, varDef.getModifiers().getAnnotations(), ctx);

    JavaNode typeNode = scan(varDef.getType(), ctx);
    if (typeNode != null) {
      emitEdge(varNode, EdgeKind.TYPED, typeNode);
    }

    scan(varDef.getInitializer(), ctx);
    return new JavaNode(varNode, signature.get(), typeNode);
  }

  @Override
  public JavaNode visitTypeApply(JCTypeApply tApply, TreeContext owner) {
    TreeContext ctx = owner.down(tApply);

    JavaNode typeCtorNode = scan(tApply.getType(), ctx);
    if (typeCtorNode == null) {
      logger.warning("Missing type constructor: " + tApply.getType());
      return emitDiagnostic(ctx, "missing type constructor", null, null);
    }

    List<JavaNode> arguments = scanList(tApply.getTypeArguments(), ctx);
    List<EntrySet> argEntries = new LinkedList<>();
    List<String> argNames = new LinkedList<>();
    Builder<EntrySet> childWildcards = ImmutableList.builder();
    for (JavaNode n : arguments) {
      argEntries.add(n.entries);
      argNames.add(n.qualifiedName);
      childWildcards.addAll(n.childWildcards);
    }

    EntrySet typeNode = entrySets.newTApplyAndEmit(typeCtorNode.entries, argEntries);
    // TODO(salguarnieri) Think about removing this since it isn't something that we have a use for.
    emitAnchor(ctx, EdgeKind.REF, typeNode);

    String qualifiedName = typeCtorNode.qualifiedName + "<" + Joiner.on(',').join(argNames) + ">";

    return new JavaNode(typeNode, qualifiedName, childWildcards.build());
  }

  @Override
  public JavaNode visitSelect(JCFieldAccess field, TreeContext owner) {
    TreeContext ctx = owner.down(field);
    if (field.sym == null) {
      // TODO(schroederc): determine exactly why this occurs
      scan(field.getExpression(), ctx);
      return null;
    } else if (field.sym.getKind() == ElementKind.PACKAGE) {
      EntrySet pkgNode = entrySets.newPackageNodeAndEmit((PackageSymbol) field.sym);
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
    if (method == null) {
      // TODO details
      return emitDiagnostic(ctx, "error analyzing method", null, null);
    }

    EntrySet anchor = emitAnchor(ctx, EdgeKind.REF_CALL, method.entries);
    TreeContext parentContext = ctx.getMethodParent();
    if (anchor != null && parentContext != null && parentContext.getNode() != null) {
      emitEdge(anchor, EdgeKind.CHILDOF, parentContext.getNode());
    }
    return method;
  }

  @Override
  public JavaNode visitNewClass(JCNewClass newClass, TreeContext owner) {
    TreeContext ctx = owner.down(newClass);

    EntrySet ctorNode = getNode(newClass.constructor);
    if (ctorNode == null) {
      return emitDiagnostic(ctx, "error analyzing class", null, null);
    }

    // Span over "new Class"
    Span refSpan =
        new Span(filePositions.getStart(newClass), filePositions.getEnd(newClass.getIdentifier()));
    EntrySet anchor = entrySets.newAnchorAndEmit(filePositions, refSpan, ctx.getSnippet());
    emitAnchor(anchor, EdgeKind.REF, ctorNode);

    // Span over "new Class(...)"
    Span callSpan = new Span(refSpan.getStart(), filePositions.getEnd(newClass));
    EntrySet callAnchor = entrySets.newAnchorAndEmit(filePositions, callSpan, ctx.getSnippet());
    emitAnchor(callAnchor, EdgeKind.REF_CALL, ctorNode);
    TreeContext parentContext = owner.getMethodParent();
    if (anchor != null && parentContext != null && parentContext.getNode() != null) {
      emitEdge(callAnchor, EdgeKind.CHILDOF, parentContext.getNode());
    }

    scanList(newClass.getTypeArguments(), ctx);
    scanList(newClass.getArguments(), ctx);
    scan(newClass.getEnclosingExpression(), ctx);
    scan(newClass.getClassBody(), ctx);
    return scan(newClass.getIdentifier(), ctx);
  }

  @Override
  public JavaNode visitTypeIdent(JCPrimitiveTypeTree primitiveType, TreeContext owner) {
    TreeContext ctx = owner.down(primitiveType);
    if (verboseLogging && primitiveType.typetag == TypeTag.ERROR) {
      logger.warning("found primitive ERROR type: " + ctx);
    }
    String name = primitiveType.typetag.toString().toLowerCase();
    EntrySet node = entrySets.newBuiltinAndEmit(name);
    emitAnchor(ctx, EdgeKind.REF, node);
    return new JavaNode(node, name);
  }

  @Override
  public JavaNode visitTypeArray(JCArrayTypeTree arrayType, TreeContext owner) {
    TreeContext ctx = owner.down(arrayType);

    JavaNode typeNode = scan(arrayType.getType(), ctx);
    EntrySet node =
        entrySets.newTApplyAndEmit(
            entrySets.newBuiltinAndEmit("array"), Arrays.asList(typeNode.entries));
    emitAnchor(ctx, EdgeKind.REF, node);
    JavaNode arrayNode = new JavaNode(node, typeNode.qualifiedName + "[]");
    return arrayNode;
  }

  @Override
  public JavaNode visitAnnotation(JCAnnotation annotation, TreeContext owner) {
    TreeContext ctx = owner.down(annotation);
    scanList(annotation.getArguments(), ctx);
    return scan(annotation.getAnnotationType(), ctx);
  }

  @Override
  public JavaNode visitWildcard(JCWildcard wild, TreeContext owner) {
    TreeContext ctx = owner.down(wild);

    EntrySet node = entrySets.newWildcardNodeAndEmit(wild, owner.getSourcePath());
    String signature = wild.kind.kind.toString();
    Builder<EntrySet> wildcards = ImmutableList.builder();
    wildcards.add(node);

    if (wild.getKind() != Kind.UNBOUNDED_WILDCARD) {
      JavaNode bound = scan(wild.getBound(), ctx);
      signature += bound.qualifiedName;
      emitEdge(
          node,
          wild.getKind() == Kind.EXTENDS_WILDCARD ? EdgeKind.BOUNDED_UPPER : EdgeKind.BOUNDED_LOWER,
          bound);
      wildcards.addAll(bound.childWildcards);
    }
    return new JavaNode(node, signature, wildcards.build());
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

  private boolean visitDocComment(EntrySet node, EntrySet absNode) {
    // TODO(https://phabricator-dot-kythe-repo.appspot.com/T185): always use absNode
    return docScanner != null && docScanner.visitDocComment(treePath, node, absNode);
  }

  // // Utility methods ////

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
    EntrySet anchor = entrySets.newAnchorAndEmit(filePositions, loc);
    if (anchor != null) {
      emitAnchor(anchor, EdgeKind.REF_DOC, node);
    }
  }

  int charToLine(int charPosition) {
    return filePositions.charToLine(charPosition);
  }

  boolean emitCommentsOnLine(int line, EntrySet node) {
    List<Comment> lst = comments.get(line);
    if (lst != null) {
      for (Comment comment : lst) {
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

  // TODO When we want to refer to a type or method that is generic, we need to point to the abs
  // node. The code currently does not have an easy way to access that node but this method might
  // offer a way to change that.
  // See https://phabricator-dot-kythe-repo.appspot.com/T185 for more discussion and detail.
  /** Create an abs node if we have type variables or if we have wildcards. */
  private EntrySet defineTypeParameters(
      TreeContext ownerContext,
      EntrySet owner,
      List<JCTypeParameter> params,
      List<EntrySet> wildcards,
      MarkedSource markedSource) {
    if (params.isEmpty() && wildcards.isEmpty()) {
      return null;
    }

    List<EntrySet> typeParams = new LinkedList<>();
    for (JCTypeParameter tParam : params) {
      TreeContext ctx = ownerContext.down(tParam);
      EntrySet node = getNode(tParam.type.asElement());
      emitDefinesBindingAnchorEdge(tParam.name, tParam.getStartPosition(), node, ctx.getSnippet());
      visitAnnotations(node, tParam.getAnnotations(), ctx);
      typeParams.add(node);

      List<JCExpression> bounds = tParam.getBounds();
      List<JavaNode> boundNodes =
          bounds.stream().map(expr -> scan(expr, ctx)).collect(Collectors.toList());
      if (boundNodes.size() == 0) {
        boundNodes.add(getJavaLangObjectNode());
      }
      emitOrdinalEdges(node, EdgeKind.BOUNDED_UPPER, boundNodes);
    }
    // Add all of the wildcards that roll up to this node. For example:
    // public static <T> void foo(Ty<?> a, Obj<?, ?> b, Obj<Ty<?>, Ty<?>> c) should declare an abs
    // node that has 1 named absvar (T) and 5 unnamed absvars.
    typeParams.addAll(wildcards);
    return entrySets.newAbstractAndEmit(owner, typeParams, markedSource);
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
    return new JavaNode(
        entrySets.getNode(signatureGenerator, sym, signature.get(), null, null), signature.get());
  }

  private void visitAnnotations(
      EntrySet owner, List<JCAnnotation> annotations, TreeContext ownerContext) {
    for (JavaNode node : scanList(annotations, ownerContext)) {
      entrySets.emitEdge(owner, EdgeKind.ANNOTATED_BY, node.entries);
    }
  }

  // Emits a node for the given sym, an anchor encompassing the TreeContext, and a REF edge
  private JavaNode emitSymUsage(TreeContext ctx, Symbol sym) {
    JavaNode node = getRefNode(ctx, sym);
    if (node == null) {
      // TODO(schroederc): details
      return emitDiagnostic(ctx, "failed to resolve symbol reference", null, null);
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
    JavaNode node = getRefNode(ctx, sym);
    if (node == null) {
      // TODO(schroederc): details
      return emitDiagnostic(ctx, "failed to resolve symbol name", null, null);
    }

    emitAnchor(name, ctx.getTree().getStartPosition(), edgeKind, node.entries, ctx.getSnippet());
    statistics.incrementCounter("name-usages-emitted");
    return node;
  }

  // Returns the reference node for the given symbol.
  private JavaNode getRefNode(TreeContext ctx, Symbol sym) {
    // If referencing a generic class, distinguish between generic vs. raw use
    // (e.g., `List` is in generic context in `List<String> x` but not in `List x`).
    boolean inGenericContext = ctx.up().getTree() instanceof JCTypeApply;
    try {
      if (sym != null
          && SignatureGenerator.isArrayHelperClass(sym.enclClass())
          && ctx.getTree() instanceof JCFieldAccess) {
        signatureGenerator.setArrayTypeContext(((JCFieldAccess) ctx.getTree()).selected.type);
      }
      JavaNode node = getJavaNode(sym);
      if (node != null
          && sym instanceof ClassSymbol
          && inGenericContext
          && !sym.getTypeParameters().isEmpty()) {
        // Always reference the abs node of a generic class, unless used as a raw type.
        node = new JavaNode(entrySets.newAbstractAndEmit(node.entries), node.qualifiedName);
      }
      return node;
    } finally {
      signatureGenerator.setArrayTypeContext(null);
    }
  }

  // Returns a JavaNode representing java.lang.Object.
  private JavaNode getJavaLangObjectNode() {
    Symbol javaLangObject = getSymbols().objectType.asElement();
    String javaLangObjectSignature = signatureGenerator.getSignature(javaLangObject).get();
    EntrySet javaLangObjectEntrySet =
        entrySets.getNode(signatureGenerator, javaLangObject, javaLangObjectSignature, null, null);
    return new JavaNode(javaLangObjectEntrySet, javaLangObjectSignature);
  }

  // Returns a JavaNode representing java.lang.Enum<E> where E is a given enum type.
  private JavaNode getJavaLangEnumNode(EntrySet enumEntrySet, String enumSignature) {
    Symbol javaLangEnum = getSymbols().enumSym;
    String javaLangEnumSignature = signatureGenerator.getSignature(javaLangEnum).get();
    EntrySet javaLangEnumEntrySet =
        entrySets.newAbstractAndEmit(
            entrySets.getNode(signatureGenerator, javaLangEnum, javaLangEnumSignature, null, null));
    EntrySet typeNode =
        entrySets.newTApplyAndEmit(javaLangEnumEntrySet, Collections.singletonList(enumEntrySet));
    String qualifiedName = javaLangEnumSignature + "<" + enumSignature + ">";
    return new JavaNode(typeNode, qualifiedName);
  }

  // Creates/emits an anchor and an associated edge
  private EntrySet emitAnchor(TreeContext anchorContext, EdgeKind kind, EntrySet node) {
    return emitAnchor(
        entrySets.newAnchorAndEmit(
            filePositions, anchorContext.getTreeSpan(), anchorContext.getSnippet()),
        kind,
        node);
  }

  // Creates/emits an anchor (for an identifier) and an associated edge
  private EntrySet emitAnchor(
      Name name, int startOffset, EdgeKind kind, EntrySet node, Span snippet) {
    EntrySet anchor = entrySets.newAnchorAndEmit(filePositions, name, startOffset, snippet);
    if (anchor == null) {
      // TODO(schroederc): Special-case these anchors (most come from visitSelect)
      return null;
    }
    return emitAnchor(anchor, kind, node);
  }

  private void emitMetadata(Span span, EntrySet node) {
    for (Metadata data : metadata) {
      for (Metadata.Rule rule : data.getRulesForLocation(span.getStart())) {
        if (rule.end == span.getEnd()) {
          if (rule.reverseEdge) {
            entrySets.emitEdge(rule.vname, rule.edgeOut, node.getVName());
          } else {
            entrySets.emitEdge(node.getVName(), rule.edgeOut, rule.vname);
          }
        }
      }
    }
  }

  private EntrySet emitDefinesBindingAnchorEdge(
      Name name, int startOffset, EntrySet node, Span snippet) {
    EntrySet anchor = emitAnchor(name, startOffset, EdgeKind.DEFINES_BINDING, node, snippet);
    Span span = filePositions.findIdentifier(name, startOffset);
    if (span != null) {
      emitMetadata(span, node);
    }
    return anchor;
  }

  private void emitDefinesBindingEdge(Span span, EntrySet anchor, EntrySet node) {
    emitMetadata(span, node);
    emitAnchor(anchor, EdgeKind.DEFINES_BINDING, node);
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

  void emitDoc(String bracketedText, Iterable<Symbol> params, EntrySet node, EntrySet absNode) {
    List<EntrySet> paramNodes = Lists.newArrayList();
    for (Symbol s : params) {
      EntrySet paramNode = getNode(s);
      if (paramNode == null) {
        return;
      }
      paramNodes.add(paramNode);
    }
    EntrySet doc = entrySets.newDocAndEmit(filePositions, bracketedText, paramNodes);
    // TODO(https://phabricator-dot-kythe-repo.appspot.com/T185): always use absNode
    entrySets.emitEdge(doc, EdgeKind.DOCUMENTS, node);
    if (absNode != null) {
      entrySets.emitEdge(doc, EdgeKind.DOCUMENTS, absNode);
    }
  }

  private EntrySet commentAnchor(Comment comment, EntrySet node) {
    return emitAnchor(
        entrySets.newAnchorAndEmit(filePositions, comment.byteSpan), EdgeKind.DOCUMENTS, node);
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

  private JavaNode emitDiagnostic(TreeContext ctx, String message, String details, String context) {
    Diagnostic.Builder d = Diagnostic.newBuilder().setMessage(message);
    if (details != null) {
      d.setDetails(details);
    }
    if (context != null) {
      d.setContextUrl(context);
    }
    Span s = ctx.getTreeSpan();
    if (s.valid()) {
      d.getSpanBuilder().getStartBuilder().setByteOffset(s.getStart());
      d.getSpanBuilder().getEndBuilder().setByteOffset(s.getEnd());
    } else if (s.getStart() >= 0) {
      // If the span isn't valid but we have a valid start, use the start for a zero-width span.
      d.getSpanBuilder().getStartBuilder().setByteOffset(s.getStart());
      d.getSpanBuilder().getEndBuilder().setByteOffset(s.getStart());
    }
    EntrySet node = entrySets.emitDiagnostic(filePositions, d.build());
    // TODO(schroederc): don't allow any edges to a diagnostic node
    return new JavaNode(node, "diagnostic");
  }

  private <T extends JCTree> List<JavaNode> scanList(List<T> trees, TreeContext owner) {
    List<JavaNode> nodes = new LinkedList<>();
    for (T t : trees) {
      nodes.add(scan(t, owner));
    }
    return nodes;
  }

  private void loadAnnotationsFile(String path) {
    URI uri = filePositions.getSourceFile().toUri();
    try {
      String fullPath = uri.resolve(path).getPath();
      if (fullPath.startsWith("/")) {
        fullPath = fullPath.substring(1);
      }
      FileObject file = fileManager.getJavaFileFromPath(fullPath, JavaFileObject.Kind.OTHER);
      if (file == null) {
        logger.warning("Can't find metadata " + path + " for " + uri + " at " + fullPath);
        return;
      }
      InputStream stream = file.openInputStream();
      Metadata newMetadata = metadataLoaders.parseFile(fullPath, ByteStreams.toByteArray(stream));
      if (newMetadata == null) {
        logger.warning("Can't load metadata " + path + " for " + uri);
        return;
      }
      metadata.add(newMetadata);
    } catch (IOException | IllegalArgumentException ex) {
      logger.warning("Can't read metadata " + path + " for " + uri);
    }
  }

  private void loadAnnotationsFromClassDecl(JCClassDecl decl) {
    for (JCAnnotation annotation : decl.getModifiers().getAnnotations()) {
      Symbol annotationSymbol = null;
      if (annotation.getAnnotationType() instanceof JCFieldAccess) {
        annotationSymbol = ((JCFieldAccess) annotation.getAnnotationType()).sym;
      } else if (annotation.getAnnotationType() instanceof JCIdent) {
        annotationSymbol = ((JCIdent) annotation.getAnnotationType()).sym;
      }
      if (annotationSymbol == null
          || !annotationSymbol.toString().equals("javax.annotation.Generated")) {
        continue;
      }
      for (JCExpression arg : annotation.getArguments()) {
        if (!(arg instanceof JCAssign)) {
          continue;
        }
        JCAssign assignArg = (JCAssign) arg;
        if (!(assignArg.lhs instanceof JCIdent) || !(assignArg.rhs instanceof JCLiteral)) {
          continue;
        }
        JCIdent lhs = (JCIdent) assignArg.lhs;
        JCLiteral rhs = (JCLiteral) assignArg.rhs;
        if (!lhs.name.toString().equals("comments") || !(rhs.getValue() instanceof String)) {
          continue;
        }
        String comments = (String) rhs.getValue();
        if (comments.startsWith(Metadata.ANNOTATION_COMMENT_PREFIX)) {
          loadAnnotationsFile(comments.substring(Metadata.ANNOTATION_COMMENT_PREFIX.length()));
        }
      }
    }
  }
}
