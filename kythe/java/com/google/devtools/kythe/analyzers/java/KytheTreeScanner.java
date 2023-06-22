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

package com.google.devtools.kythe.analyzers.java;

import static java.nio.charset.StandardCharsets.UTF_8;

import com.google.common.base.Ascii;
import com.google.common.base.Preconditions;
import com.google.common.collect.ImmutableList;
import com.google.common.collect.ImmutableSet;
import com.google.common.collect.Iterables;
import com.google.common.collect.Lists;
import com.google.common.collect.Streams;
import com.google.common.flogger.FluentLogger;
import com.google.common.io.ByteStreams;
import com.google.devtools.kythe.analyzers.base.CorpusPath;
import com.google.devtools.kythe.analyzers.base.EdgeKind;
import com.google.devtools.kythe.analyzers.base.EntrySet;
import com.google.devtools.kythe.analyzers.java.KytheDocTreeScanner.DocCommentVisitResult;
import com.google.devtools.kythe.analyzers.java.SourceText.Comment;
import com.google.devtools.kythe.analyzers.java.SourceText.Keyword;
import com.google.devtools.kythe.analyzers.java.SourceText.Positions;
import com.google.devtools.kythe.analyzers.jvm.JvmGraph;
import com.google.devtools.kythe.analyzers.jvm.JvmGraph.Type.ReferenceType;
import com.google.devtools.kythe.platform.java.helpers.JCTreeScanner;
import com.google.devtools.kythe.platform.java.helpers.JavacUtil;
import com.google.devtools.kythe.platform.java.helpers.SignatureGenerator;
import com.google.devtools.kythe.platform.shared.Metadata;
import com.google.devtools.kythe.platform.shared.MetadataLoaders;
import com.google.devtools.kythe.platform.shared.StatisticsCollector;
import com.google.devtools.kythe.proto.Diagnostic;
import com.google.devtools.kythe.proto.MarkedSource;
import com.google.devtools.kythe.proto.Storage.VName;
import com.google.devtools.kythe.util.Span;
import com.google.protobuf.DescriptorProtos.GeneratedCodeInfo.Annotation.Semantic;
import com.sun.source.tree.MemberReferenceTree.ReferenceMode;
import com.sun.source.tree.Scope;
import com.sun.source.tree.Tree.Kind;
import com.sun.tools.javac.api.JavacTrees;
import com.sun.tools.javac.code.Symbol;
import com.sun.tools.javac.code.Symbol.ClassSymbol;
import com.sun.tools.javac.code.Symbol.PackageSymbol;
import com.sun.tools.javac.code.Symbol.VarSymbol;
import com.sun.tools.javac.code.Symtab;
import com.sun.tools.javac.code.Type;
import com.sun.tools.javac.code.TypeTag;
import com.sun.tools.javac.code.Types;
import com.sun.tools.javac.tree.JCTree;
import com.sun.tools.javac.tree.JCTree.JCAnnotation;
import com.sun.tools.javac.tree.JCTree.JCArrayTypeTree;
import com.sun.tools.javac.tree.JCTree.JCAssert;
import com.sun.tools.javac.tree.JCTree.JCAssign;
import com.sun.tools.javac.tree.JCTree.JCAssignOp;
import com.sun.tools.javac.tree.JCTree.JCBinary;
import com.sun.tools.javac.tree.JCTree.JCBlock;
import com.sun.tools.javac.tree.JCTree.JCClassDecl;
import com.sun.tools.javac.tree.JCTree.JCCompilationUnit;
import com.sun.tools.javac.tree.JCTree.JCExpression;
import com.sun.tools.javac.tree.JCTree.JCExpressionStatement;
import com.sun.tools.javac.tree.JCTree.JCFieldAccess;
import com.sun.tools.javac.tree.JCTree.JCFunctionalExpression;
import com.sun.tools.javac.tree.JCTree.JCIdent;
import com.sun.tools.javac.tree.JCTree.JCImport;
import com.sun.tools.javac.tree.JCTree.JCLambda;
import com.sun.tools.javac.tree.JCTree.JCLiteral;
import com.sun.tools.javac.tree.JCTree.JCMemberReference;
import com.sun.tools.javac.tree.JCTree.JCMethodDecl;
import com.sun.tools.javac.tree.JCTree.JCMethodInvocation;
import com.sun.tools.javac.tree.JCTree.JCModifiers;
import com.sun.tools.javac.tree.JCTree.JCNewClass;
import com.sun.tools.javac.tree.JCTree.JCPackageDecl;
import com.sun.tools.javac.tree.JCTree.JCParens;
import com.sun.tools.javac.tree.JCTree.JCPrimitiveTypeTree;
import com.sun.tools.javac.tree.JCTree.JCReturn;
import com.sun.tools.javac.tree.JCTree.JCThrow;
import com.sun.tools.javac.tree.JCTree.JCTypeApply;
import com.sun.tools.javac.tree.JCTree.JCTypeParameter;
import com.sun.tools.javac.tree.JCTree.JCUnary;
import com.sun.tools.javac.tree.JCTree.JCVariableDecl;
import com.sun.tools.javac.tree.JCTree.JCWildcard;
import com.sun.tools.javac.util.Context;
import java.io.IOException;
import java.io.InputStream;
import java.net.URI;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.Set;
import java.util.function.BiConsumer;
import java.util.stream.Collectors;
import javax.lang.model.element.ElementKind;
import javax.lang.model.element.Modifier;
import javax.lang.model.element.Name;
import javax.lang.model.element.NestingKind;
import javax.tools.FileObject;
import javax.tools.JavaFileObject;
import javax.tools.StandardJavaFileManager;
import org.checkerframework.checker.nullness.qual.Nullable;

/** {@link JCTreeScanner} that emits Kythe nodes and edges. */
public class KytheTreeScanner extends JCTreeScanner<JavaNode, TreeContext> {
  private static final FluentLogger logger = FluentLogger.forEnclosingClass();

  /** Set of known class names used for annotating generated code. */
  private static final ImmutableSet<String> GENERATED_ANNOTATIONS =
      ImmutableSet.of("javax.annotation.Generated", "javax.annotation.processing.Generated");

  /** Maximum allowed text size for variable {@link MarkedSource.Kind.INITIALIZER}s */
  private static final int MAX_INITIALIZER_LENGTH = 80;

  /** Name for special source file containing package annotations and documentation. */
  private static final String PACKAGE_INFO_NAME = "package-info";

  private final JavaIndexerConfig config;

  private final JavaEntrySets entrySets;
  private final StatisticsCollector statistics;
  // TODO(schroederc): refactor SignatureGenerator for new schema names
  private final SignatureGenerator signatureGenerator;
  private final Positions filePositions;
  private final Map<Integer, List<Comment>> comments = new HashMap<>();
  private final Map<Integer, Integer> commentClaims = new HashMap<>();
  private final BiConsumer<JCTree, VName> nodeConsumer;
  private final Context javaContext;
  private final StandardJavaFileManager fileManager;
  private final MetadataLoaders metadataLoaders;
  private final JvmGraph jvmGraph;
  private final Set<VName> emittedIdentType = new HashSet<>();

  private final Set<String> metadataFilePaths = new HashSet<>();
  private List<Metadata> metadata;

  private KytheDocTreeScanner docScanner;

  private KytheTreeScanner(
      JavaEntrySets entrySets,
      StatisticsCollector statistics,
      SignatureGenerator signatureGenerator,
      SourceText src,
      Context javaContext,
      BiConsumer<JCTree, VName> nodeConsumer,
      StandardJavaFileManager fileManager,
      MetadataLoaders metadataLoaders,
      JvmGraph jvmGraph,
      JavaIndexerConfig config) {
    this.entrySets = entrySets;
    this.statistics = statistics;
    this.signatureGenerator = signatureGenerator;
    this.filePositions = src.getPositions();
    this.javaContext = javaContext;
    this.nodeConsumer = nodeConsumer;
    this.fileManager = fileManager;
    this.metadataLoaders = metadataLoaders;
    this.jvmGraph = jvmGraph;
    this.config = config;

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
      BiConsumer<JCTree, VName> nodeConsumer,
      SourceText src,
      StandardJavaFileManager fileManager,
      MetadataLoaders metadataLoaders,
      JavaIndexerConfig config) {
    new KytheTreeScanner(
            entrySets,
            statistics,
            signatureGenerator,
            src,
            javaContext,
            nodeConsumer,
            fileManager,
            metadataLoaders,
            new JvmGraph(statistics, entrySets.getEmitter()),
            config)
        .scan(compilation, null);
  }

  /** Returns the {@link Symtab} (symbol table) for the compilation currently being processed. */
  public Symtab getSymbols() {
    return Symtab.instance(javaContext);
  }

  @Override
  public JavaNode scan(JCTree tree, TreeContext owner) {
    JavaNode node = super.scan(tree, owner);
    if (node != null && nodeConsumer != null) {
      nodeConsumer.accept(tree, node.getVName());
    }
    return node;
  }

  @Override
  public JavaNode visitTopLevel(JCCompilationUnit compilation, TreeContext owner) {
    if (compilation.docComments != null) {
      docScanner = new KytheDocTreeScanner(this, javaContext);
    }
    TreeContext ctx = new TreeContext(filePositions, compilation);
    metadata = new ArrayList<>();
    loadImplicitAnnotationsFile();

    EntrySet fileNode = entrySets.newFileNodeAndEmit(filePositions);
    JavaNode node = ctx.setNode(new JavaNode(fileNode));

    List<JavaNode> decls = scanList(compilation.getTypeDecls(), ctx);
    decls.removeAll(Collections.singleton(null));

    JavaNode pkgNode = scan(compilation.getPackage(), ctx);
    if (pkgNode != null) {
      for (JavaNode n : decls) {
        entrySets.emitEdge(n.getVName(), EdgeKind.CHILDOF, pkgNode.getVName());
      }
    }

    scan(compilation.getImports(), ctx);

    emitFileScopeMetadata(fileNode.getVName());

    return node;
  }

  private void emitFileScopeMetadata(VName file) {
    for (Metadata data : metadata) {
      for (Metadata.Rule rule : data.getFileScopeRules()) {
        if (rule.reverseEdge) {
          entrySets.emitEdge(rule.vname, rule.edgeOut, file);
        } else {
          entrySets.emitEdge(file, rule.edgeOut, rule.vname);
        }
      }
    }
  }

  @Override
  public JavaNode visitPackage(JCPackageDecl pkg, TreeContext owner) {
    TreeContext ctx = owner.down(pkg.pid);

    VName pkgNode = entrySets.newPackageNodeAndEmit(pkg.packge).getVName();

    boolean isPkgInfo =
        filePositions
            .getSourceFile()
            .isNameCompatible(PACKAGE_INFO_NAME, JavaFileObject.Kind.SOURCE);
    EdgeKind anchorKind = isPkgInfo ? EdgeKind.DEFINES_BINDING : EdgeKind.REF;
    emitAnchor(ctx, anchorKind, pkgNode);

    visitDocComment(pkgNode, null, /* modifiers= */ null);
    visitAnnotations(pkgNode, pkg.getAnnotations(), ctx);

    return new JavaNode(pkgNode);
  }

  @Override
  public JavaNode visitImport(JCImport imprt, TreeContext owner) {
    return scan(shims.getQualifiedIdentifier(imprt), owner.downAsSnippet(imprt));
  }

  @Override
  public JavaNode visitIdent(JCIdent ident, TreeContext owner) {
    TreeContext ctx = owner.down(ident);
    if (ident.sym == null) {
      return emitDiagnostic(ctx, "missing identifier symbol", null, null);
    }

    JavaNode node = null;
    if (ident.sym instanceof ClassSymbol && ident == owner.getNewClassIdentifier()) {
      // Use ref/id edges for the primary identifier to disambiguate from the constructor.
      node = emitSymUsage(ctx, ident.sym, EdgeKind.REF_ID);
    } else {
      JCTree parentTree = owner.up() == null ? null : owner.up().getTree();
      RefType refType = getRefType(ctx, owner.getTree(), parentTree, ident.sym, ident.pos);
      if (refType == RefType.READ || refType == RefType.READ_WRITE) {
        node = emitSymUsage(ctx, ident.sym, EdgeKind.REF);
      }
      if (refType == RefType.WRITE || refType == RefType.READ_WRITE) {
        node = emitSymUsage(ctx, ident.sym, EdgeKind.REF_WRITES);
      }
    }

    if (node != null && ident.sym instanceof VarSymbol) {
      // Emit typed edges for "this"/"super" on reference since there is no definition location.
      // TODO(schroederc): possibly add implicit definition on class declaration
      if ("this".equals(ident.sym.getSimpleName().toString())
          && !emittedIdentType.contains(node.getVName())) {
        JavaNode typeNode = getRefNode(ctx, ident.sym.enclClass());
        if (typeNode == null) {
          return emitDiagnostic(ctx, "failed to resolve symbol reference", null, null);
        }
        entrySets.emitEdge(node.getVName(), EdgeKind.TYPED, typeNode.getVName());
        emittedIdentType.add(node.getVName());
      } else if ("super".equals(ident.sym.getSimpleName().toString())
          && !emittedIdentType.contains(node.getVName())) {
        JavaNode typeNode = getRefNode(ctx, ident.sym.enclClass().getSuperclass().asElement());
        if (typeNode == null) {
          return emitDiagnostic(ctx, "failed to resolve symbol reference", null, null);
        }
        entrySets.emitEdge(node.getVName(), EdgeKind.TYPED, typeNode.getVName());
        emittedIdentType.add(node.getVName());
      }
    }
    return node;
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
    VName classNode =
        entrySets.getNode(signatureGenerator, classDef.sym, signature.get(), markedSource, null);

    // Emit the fact that the class is a child of its containing class or method.
    // Note that for a nested/inner class, we already emitted the fact that it's a
    // child of the containing class when we scanned the containing class's members.
    // However we can't restrict ourselves to just classes contained in methods here,
    // because that would miss the case of local/anonymous classes in static/member
    // initializers. But there's no harm in emitting the same fact twice!
    getScope(ctx).forEach(scope -> entrySets.emitEdge(classNode, EdgeKind.CHILDOF, scope));

    emitModifiers(classNode, classDef.getModifiers());
    emitVisibility(classNode, classDef.getModifiers(), ctx);

    NestingKind nestingKind = classDef.sym.getNestingKind();
    if (nestingKind != NestingKind.LOCAL
        && nestingKind != NestingKind.ANONYMOUS
        && !isErroneous(classDef.sym)) {
      // Emit corresponding JVM node
      JvmGraph.Type.ReferenceType referenceType = referenceType(classDef.sym.type);
      VName jvmNode = jvmGraph.emitClassNode(entrySets.jvmCorpusPath(classDef.sym), referenceType);
      entrySets.emitEdge(classNode, EdgeKind.GENERATES, jvmNode);
      entrySets.emitEdge(classNode, EdgeKind.NAMED, jvmNode);
    }

    Span classIdent = filePositions.findIdentifier(classDef.name, classDef.getPreferredPosition());
    if (!classDef.name.isEmpty() && classIdent == null) {
      logger.atWarning().log("Missing span for class identifier: %s", classDef.sym);
    }

    // Generic classes record the source range of the class name for the abs node, regular
    // classes record the source range of the class name for the record node.
    VName absNode =
        defineTypeParameters(
            ctx,
            classNode,
            classDef.getTypeParameters(),
            /* There are no wildcards in class definitions */
            ImmutableList.<VName>of());

    boolean documented = visitDocComment(classNode, absNode, classDef.getModifiers());

    if (absNode != null) {
      if (classIdent != null) {
        EntrySet absAnchor =
            entrySets.newAnchorAndEmit(filePositions, classIdent, ctx.getSnippet());
        emitDefinesBindingEdge(classIdent, absAnchor, absNode, getScope(ctx));
      }
      if (!documented) {
        emitComment(classDef, absNode);
      }
    }
    if (absNode == null && classIdent != null) {
      EntrySet anchor = entrySets.newAnchorAndEmit(filePositions, classIdent, ctx.getSnippet());
      emitDefinesBindingEdge(classIdent, anchor, classNode, getScope(ctx));
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
          superClassNode = getJavaLangEnumNode(classNode);
          break;
        case ANNOTATION_TYPE:
          // TODO(schroederc): handle annotation superclass
          break;
        case INTERFACE:
          break; // Interfaces have no implicit superclass.
        default:
          logger.atWarning().log("Unexpected JCClassDecl kind: %s", classDef.getKind());
          break;
      }
    }

    if (superClassNode != null) {
      entrySets.emitEdge(classNode, EdgeKind.EXTENDS, superClassNode.getVName());
    }

    for (JCExpression implClass : classDef.getImplementsClause()) {
      JavaNode implNode = scan(implClass, ctx);
      if (implNode == null) {
        statistics.incrementCounter("warning-missing-implements-node");
        logger.atWarning().log(
            "Missing 'implements' node for %s: %s", implClass.getClass(), implClass);
        continue;
      }
      entrySets.emitEdge(classNode, EdgeKind.EXTENDS, implNode.getVName());
    }

    // Set the resulting node for the class before recursing through its members.  Setting the node
    // first is necessary to correctly add childof edges from local/anonymous classes defined
    // directly in the class body (in static initializers or member initializers).
    JavaNode node = ctx.setNode(new JavaNode(classNode));

    List<VName> constructors = new ArrayList<>();
    for (JCTree member : classDef.getMembers()) {
      if (member instanceof JCMethodDecl) {
        JCMethodDecl method = (JCMethodDecl) member;
        if (method.sym == null || !method.sym.isConstructor()) {
          continue;
        }

        JavaNode n = scanChild(member, ctx, classNode);
        if (n != null) {
          constructors.add(n.getVName());
        }
      }
    }
    node.setClassConstructors(constructors);

    VName classInit = entrySets.newClassInitAndEmit(signature.get(), classNode).getVName();
    node.setClassInit(classInit);
    entrySets.emitEdge(classInit, EdgeKind.CHILDOF, classNode);
    if (classIdent != null) {
      // Emit implicit zero-length definition for the class static initializer.
      emitAnchor(
          entrySets.newAnchorAndEmit(
              filePositions,
              new Span(classIdent.getStart(), classIdent.getStart()),
              ctx.getSnippet()),
          EdgeKind.DEFINES,
          classInit,
          ImmutableList.of(classNode));
    }

    for (JCTree member : classDef.getMembers()) {
      if (member instanceof JCMethodDecl) {
        JCMethodDecl method = (JCMethodDecl) member;
        if (method.sym == null) {
          String name = method.name == null ? "" : method.name.toString();
          logger.atWarning().log(
              "Missing method symbol: %s at %s:%s", name, ctx.getSourcePath(), ctx.getTreeSpan());
          var unused = emitDiagnostic(ctx, "missing method symbol: " + name, null, null);
        } else if (method.sym.isConstructor()) {
          // Already handled above.
          continue;
        }
      }
      scanChild(member, ctx, classNode);
    }

    return node;
  }

  private JavaNode scanChild(JCTree child, TreeContext owner, VName parent) {
    JavaNode n = scan(child, owner);
    if (n != null) {
      entrySets.emitEdge(n.getVName(), EdgeKind.CHILDOF, parent);
    }
    return n;
  }

  @Override
  public JavaNode visitBlock(JCBlock block, TreeContext owner) {
    TreeContext ctx = owner;
    if (block.isStatic() && owner.getNode().getClassInit().isPresent()) {
      ctx = owner.down(block);
      ctx.setNode(new JavaNode(owner.getNode().getClassInit().get()));
    }
    return scan(block.getStatements(), ctx);
  }

  @Override
  public JavaNode visitMethodDef(JCMethodDecl methodDef, TreeContext owner) {
    TreeContext ctx = owner.down(methodDef);

    scan(methodDef.getThrows(), ctx);
    scan(methodDef.getDefaultValue(), ctx);
    scan(methodDef.getReceiverParameter(), ctx);

    JavaNode returnType = scan(methodDef.getReturnType(), ctx);
    List<JavaNode> params = new ArrayList<>();
    List<JavaNode> paramTypes = new ArrayList<>();
    List<VName> wildcards = new ArrayList<>();
    for (JCVariableDecl param : methodDef.getParameters()) {
      JavaNode n = scan(param, ctx);
      params.add(n);

      JavaNode typeNode = n.getType();
      if (typeNode == null) {
        logger.atWarning().log(
            "Missing parameter type (method: %s; parameter: %s)", methodDef.getName(), param);
        wildcards.addAll(n.childWildcards);
        continue;
      }
      wildcards.addAll(typeNode.childWildcards);
      paramTypes.add(typeNode);
    }

    Optional<String> signature = signatureGenerator.getSignature(methodDef.sym);
    if (!signature.isPresent()) {
      // Try to scan method body even if signature could not be generated.
      scan(methodDef.getBody(), ctx);

      // TODO(schroederc): details
      return emitDiagnostic(ctx, "missing method signature", null, null);
    }

    MarkedSource.Builder markedSource = MarkedSource.newBuilder();
    VName methodNode =
        entrySets.getNode(signatureGenerator, methodDef.sym, signature.get(), markedSource, null);
    visitAnnotations(methodNode, methodDef.getModifiers().getAnnotations(), ctx);
    emitModifiers(methodNode, methodDef.getModifiers());
    emitVisibility(methodNode, methodDef.getModifiers(), ctx);

    VName absNode = defineTypeParameters(ctx, methodNode, methodDef.getTypeParameters(), wildcards);
    boolean documented = visitDocComment(methodNode, absNode, methodDef.getModifiers());

    if (!isErroneous(methodDef.sym)) {
      // Emit corresponding JVM node
      CorpusPath corpusPath = entrySets.jvmCorpusPath(methodDef.sym);
      JvmGraph.Type.MethodType methodJvmType =
          toMethodJvmType((Type.MethodType) externalType(methodDef.sym));
      ReferenceType parentClass = referenceType(externalType(methodDef.sym.enclClass()));
      String methodName = methodDef.name.toString();
      VName jvmNode = jvmGraph.emitMethodNode(corpusPath, parentClass, methodName, methodJvmType);
      entrySets.emitEdge(methodNode, EdgeKind.GENERATES, jvmNode);
      entrySets.emitEdge(methodNode, EdgeKind.NAMED, jvmNode);

      for (int i = 0; i < params.size(); i++) {
        JavaNode param = params.get(i);
        VName paramJvmNode =
            jvmGraph.emitParameterNode(corpusPath, parentClass, methodName, methodJvmType, i);
        entrySets.emitEdge(param.getVName(), EdgeKind.GENERATES, paramJvmNode);
        entrySets.emitEdge(param.getVName(), EdgeKind.NAMED, paramJvmNode);
        entrySets.emitEdge(jvmNode, EdgeKind.PARAM, paramJvmNode, i);
      }
    }

    VName ret = null;
    EntrySet bindingAnchor = null;
    if (methodDef.sym.isConstructor()) {
      // Implicit constructors (those without syntactic definition locations) share the same
      // preferred position as their owned class.  We can differentiate them from other constructors
      // by checking if its position is ahead of the owner's position.
      if (methodDef.getPreferredPosition() > owner.getTree().getPreferredPosition()) {
        // Explicit constructor: use the owner's name (the class name) to find the definition
        // anchor's location because constructors are internally named "<init>".
        bindingAnchor =
            emitDefinesBindingAnchorEdge(
                ctx, methodDef.sym.owner.name, methodDef.getPreferredPosition(), methodNode);
      } else {
        // Implicit constructor: generate a zero-length implicit anchor
        emitAnchor(ctx, EdgeKind.DEFINES, methodNode);
      }
      // Likewise, constructors don't have return types in the Java AST, but
      // Kythe models all functions with return types.  As a solution, we use
      // the class type as the return type for all constructors.
      ret = getNode(methodDef.sym.owner);
    } else {
      bindingAnchor =
          emitDefinesBindingAnchorEdge(
              ctx, methodDef.name, methodDef.getPreferredPosition(), methodNode);
      ret = returnType.getVName();
    }

    if (bindingAnchor != null) {
      if (!documented) {
        emitComment(methodDef, methodNode);
      }
      if (absNode != null) {
        emitAnchor(bindingAnchor, EdgeKind.DEFINES_BINDING, absNode, getScope(ctx));
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

    if (methodDef.getModifiers().getFlags().contains(Modifier.ABSTRACT)) {
      entrySets.getEmitter().emitFact(methodNode, "/kythe/tag/abstract", "");
    }

    VName recv = null;
    if (!methodDef.getModifiers().getFlags().contains(Modifier.STATIC)) {
      recv = owner.getNode().getVName();
    }
    EntrySet fnTypeNode =
        entrySets.newFunctionTypeAndEmit(
            ret,
            recv == null ? entrySets.newBuiltinAndEmit("void").getVName() : recv,
            toVNames(paramTypes),
            recv == null ? MarkedSources.FN_TAPP : MarkedSources.METHOD_TAPP);
    entrySets.emitEdge(methodNode, EdgeKind.TYPED, fnTypeNode.getVName());

    JavacUtil.visitSuperMethods(
        javaContext,
        methodDef.sym,
        (sym, kind) ->
            entrySets.emitEdge(
                methodNode,
                kind == JavacUtil.OverrideKind.DIRECT
                    ? EdgeKind.OVERRIDES
                    : EdgeKind.OVERRIDES_TRANSITIVE,
                getNode(sym)));

    // Set the resulting node for the method and then recurse through its body.  Setting the node
    // first is necessary to correctly add childof edges in the callgraph.
    JavaNode node = ctx.setNode(new JavaNode(methodNode));
    scan(methodDef.getBody(), ctx);

    for (JavaNode param : params) {
      entrySets.emitEdge(param.getVName(), EdgeKind.CHILDOF, node.getVName());
    }

    return node;
  }

  @Override
  public JavaNode visitLambda(JCLambda lambda, TreeContext owner) {
    TreeContext ctx = owner.down(lambda);
    VName lambdaNode = entrySets.newLambdaAndEmit(filePositions, lambda).getVName();
    emitAnchor(ctx, EdgeKind.DEFINES, lambdaNode);

    for (Type target : getTargets(lambda)) {
      VName targetNode = getNode(target.asElement());
      entrySets.emitEdge(lambdaNode, EdgeKind.EXTENDS, targetNode);
    }

    scan(lambda.body, ctx);
    scanList(lambda.params, ctx);
    return new JavaNode(lambdaNode);
  }

  private static ImmutableList<Type> getTargets(JCFunctionalExpression node) {
    if (node == null || node.target == null) {
      return ImmutableList.of();
    }
    return ImmutableList.of(node.target);
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

    VName varNode =
        entrySets.getNode(
            signatureGenerator, varDef.sym, signature.get(), null, markedSourceChildren);
    boolean documented = visitDocComment(varNode, null, varDef.getModifiers());
    emitDefinesBindingAnchorEdge(ctx, varDef.name, varDef.getPreferredPosition(), varNode);
    emitAnchor(ctx, EdgeKind.DEFINES, varNode);
    if (varDef.sym.getKind().isField() && !documented) {
      // emit comments for fields and enumeration constants
      emitComment(varDef, varNode);
    }

    if (varDef.sym.getKind().isField() && !isErroneous(varDef.sym)) {
      // Emit corresponding JVM node
      VName jvmNode =
          jvmGraph.emitFieldNode(
              entrySets.jvmCorpusPath(varDef.sym),
              referenceType(externalType(varDef.sym.enclClass())),
              varDef.name.toString());
      entrySets.emitEdge(varNode, EdgeKind.GENERATES, jvmNode);
      entrySets.emitEdge(varNode, EdgeKind.NAMED, jvmNode);
    }

    getScope(ctx).forEach(scope -> entrySets.emitEdge(varNode, EdgeKind.CHILDOF, scope));
    visitAnnotations(varNode, varDef.getModifiers().getAnnotations(), ctx);

    emitModifiers(varNode, varDef.getModifiers());
    if (varDef.sym.getKind().isField()) {
      emitVisibility(varNode, varDef.getModifiers(), ctx);
    }
    if (varDef.getModifiers().getFlags().contains(Modifier.STATIC)) {
      if (varDef.sym.getKind().isField() && owner.getNode().getClassInit().isPresent()) {
        ctx.setNode(new JavaNode(owner.getNode().getClassInit().get()));
      }
    }
    scan(varDef.getInitializer(), ctx);

    JavaNode typeNode = scan(varDef.getType(), ctx);
    if (typeNode != null) {
      entrySets.emitEdge(varNode, EdgeKind.TYPED, typeNode.getVName());
      return new JavaNode(varNode, typeNode.childWildcards).setType(typeNode);
    }

    return new JavaNode(varNode);
  }

  @Override
  public JavaNode visitTypeApply(JCTypeApply tApply, TreeContext owner) {
    TreeContext ctx = owner.down(tApply);

    JavaNode typeCtorNode = scan(tApply.getType(), ctx);
    if (typeCtorNode == null) {
      logger.atWarning().log("Missing type constructor: %s", tApply.getType());
      return emitDiagnostic(ctx, "missing type constructor", null, null);
    }

    List<JavaNode> arguments = scanList(tApply.getTypeArguments(), ctx);
    List<VName> argVNames = new ArrayList<>();
    ImmutableList.Builder<VName> childWildcards = ImmutableList.builder();
    for (JavaNode n : arguments) {
      argVNames.add(n.getVName());
      childWildcards.addAll(n.childWildcards);
    }

    EntrySet typeNode =
        entrySets.newTApplyAndEmit(typeCtorNode.getVName(), argVNames, MarkedSources.GENERIC_TAPP);
    // TODO(salguarnieri) Think about removing this since it isn't something that we have a use for.
    emitAnchor(
        ctx,
        (owner.getTree() instanceof JCNewClass) ? EdgeKind.REF_ID : EdgeKind.REF,
        typeNode.getVName());

    return new JavaNode(typeNode, childWildcards.build());
  }

  @Override
  public @Nullable JavaNode visitSelect(JCFieldAccess field, TreeContext owner) {
    TreeContext ctx = owner.down(field);

    JCImport imprt = null;
    if (owner.getTree() instanceof JCImport) {
      imprt = (JCImport) owner.getTree();
    }

    Symbol sym = field.sym;
    if (sym == null && imprt != null && imprt.isStatic()) {
      // Static imports don't have their symbol populated so we search for the symbol.

      ClassSymbol cls = JavacUtil.getClassSymbol(javaContext, field.selected + "." + field.name);
      if (cls != null) {
        // Import was a inner class import
        sym = cls;
      } else {
        cls = JavacUtil.getClassSymbol(javaContext, field.selected.toString());
        if (cls != null) {
          // Import is a class member; emit usages for all matching (by name) class members.
          ctx = ctx.down(field);

          JavacTrees trees = JavacTrees.instance(javaContext);
          Type.ClassType classType = (Type.ClassType) cls.asType();
          Scope scope = trees.getScope(treePath);

          JavaNode lastMember = null;
          for (Symbol member : cls.members().getSymbolsByName(field.name)) {
            try {
              if (!member.isStatic() || !trees.isAccessible(scope, member, classType)) {
                continue;
              }

              // Ensure member symbol's type is complete.  If the extractor finds that a static
              // member isn't used (due to overloads), the symbol's dependent type classes won't
              // be saved in the CompilationUnit and this will throw an exception.
              if (member.type != null) {
                member.type.tsym.complete();
                member.type.getParameterTypes().forEach(t -> t.tsym.complete());
                Type returnType = member.type.getReturnType();
                if (returnType != null) {
                  returnType.tsym.complete();
                }
              }

              lastMember = emitNameUsage(ctx, member, field.name, EdgeKind.REF_IMPORTS);
            } catch (Symbol.CompletionFailure e) {
              // Symbol resolution failed (see above comment).  Ignore and continue with other
              // class members matching static import.
            }
          }
          scan(field.getExpression(), ctx);
          return lastMember;
        }
      }
    }

    if (sym == null) {
      scan(field.getExpression(), ctx);
      if (!field.name.contentEquals("*")) {
        String msg = "Could not determine selected Symbol for " + field;
        if (config.getVerboseLogging()) {
          logger.atWarning().log("%s", msg);
        }
        return emitDiagnostic(ctx, msg, null, null);
      }
      return null;
    } else if (sym.getKind() == ElementKind.PACKAGE) {
      EntrySet pkgNode = entrySets.newPackageNodeAndEmit((PackageSymbol) sym);
      emitAnchor(ctx, EdgeKind.REF, pkgNode.getVName());
      return new JavaNode(pkgNode);
    } else {
      scan(field.getExpression(), ctx);

      Name name = field.name.contentEquals("class") ? Keyword.CLASS : field.name;
      if (imprt != null) {
        return emitNameUsage(ctx, sym, name, EdgeKind.REF_IMPORTS);
      }

      JCTree parentTree = owner.up() == null ? null : owner.up().getTree();
      RefType refType = getRefType(ctx, owner.getTree(), parentTree, sym, field.pos);
      // RefType can only be READ, WRITE, OR READ_WRITE so node will always have a value set by
      // at least one call to emitNameUsage. We just have to initialize to null to keep the compiler
      // happy.
      JavaNode node = null;
      if (refType == RefType.READ || refType == RefType.READ_WRITE) {
        node = emitNameUsage(ctx, sym, name, EdgeKind.REF);
      }
      if (refType == RefType.WRITE || refType == RefType.READ_WRITE) {
        node = emitNameUsage(ctx, sym, name, EdgeKind.REF_WRITES);
      }
      return node;
    }
  }

  @Override
  public JavaNode visitReference(JCMemberReference reference, TreeContext owner) {
    TreeContext ctx = owner.down(reference);
    scan(reference.getQualifierExpression(), ctx);
    return emitNameUsage(
        ctx,
        reference.sym,
        reference.getMode() == ReferenceMode.NEW ? Keyword.NEW : reference.name);
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

    EdgeKind kind =
        Optional.ofNullable(method.getSymbol()).map(Symbol::isConstructor).orElse(false)
            ? EdgeKind.REF_CALL_DIRECT
            : EdgeKind.REF_CALL;

    if (config.getEmitRefCallOverIdentifier()) {
      TreeContext methodCtx = ctx.down(invoke.getMethodSelect());
      if (invoke.getMethodSelect() instanceof JCIdent) {
        emitAnchor(methodCtx, kind, method.getVName());
        return method;
      } else if (invoke.getMethodSelect() instanceof JCFieldAccess) {
        JCFieldAccess field = (JCFieldAccess) invoke.getMethodSelect();
        if (field.sym != null && field.name != null) {
          return emitNameUsage(methodCtx, field.sym, field.name, kind);
        }
      }
      // fall-through to handle non-identifier invocations
    }

    emitAnchor(ctx, kind, method.getVName());
    return method;
  }

  @Override
  public JavaNode visitNewClass(JCNewClass newClass, TreeContext owner) {
    TreeContext ctx = owner.down(newClass);

    if (newClass == null || newClass.constructor == null) {
      logger.atWarning().log("Unexpected null class or constructor: %s", newClass);
      return emitDiagnostic(ctx, "error analyzing class", null, null);
    }
    VName ctorNode = getNode(newClass.constructor);
    if (ctorNode == null) {
      return emitDiagnostic(ctx, "error analyzing class", null, null);
    }

    Span refSpan;
    if (newClass.getIdentifier() instanceof JCTypeApply) {
      JCTree type = ((JCTypeApply) newClass.getIdentifier()).getType();
      refSpan = new Span(filePositions.getStart(type), filePositions.getEnd(type));
    } else {
      refSpan =
          new Span(
              filePositions.getStart(newClass.getIdentifier()),
              filePositions.getEnd(newClass.getIdentifier()));
    }

    // Span over "new Class(...)"
    Span callSpan = new Span(filePositions.getStart(newClass), filePositions.getEnd(newClass));
    if (config.getEmitRefCallOverIdentifier()) {
      callSpan = refSpan;
    }

    if (owner.getTree().getTag() == JCTree.Tag.VARDEF) {
      JCVariableDecl varDef = (JCVariableDecl) owner.getTree();
      if (varDef.sym.getKind() == ElementKind.ENUM_CONSTANT) {
        // Handle enum constructors specially.
        // Span over "EnumValueName"
        refSpan = filePositions.findIdentifier(varDef.name, varDef.getStartPosition());
        // Span over "EnumValueName(...)"
        callSpan = new Span(refSpan.getStart(), filePositions.getEnd(varDef));
      }
    }

    EntrySet anchor = entrySets.newAnchorAndEmit(filePositions, refSpan, ctx.getSnippet());
    emitAnchor(anchor, EdgeKind.REF, ctorNode, getScope(ctx));

    EntrySet callAnchor = entrySets.newAnchorAndEmit(filePositions, callSpan, ctx.getSnippet());
    emitAnchor(callAnchor, EdgeKind.REF_CALL_DIRECT, ctorNode, getCallScope(ctx));

    scanList(newClass.getTypeArguments(), ctx);
    scanList(newClass.getArguments(), ctx);
    scan(newClass.getEnclosingExpression(), ctx);
    scan(newClass.getClassBody(), ctx);

    return scan(newClass.getIdentifier(), ctx);
  }

  @Override
  public JavaNode visitTypeIdent(JCPrimitiveTypeTree primitiveType, TreeContext owner) {
    TreeContext ctx = owner.down(primitiveType);
    if (config.getVerboseLogging() && primitiveType.typetag == TypeTag.ERROR) {
      logger.atWarning().log("found primitive ERROR type: %s", ctx);
    }
    String name = Ascii.toLowerCase(primitiveType.typetag.toString());
    EntrySet node = entrySets.newBuiltinAndEmit(name);
    emitAnchor(ctx, EdgeKind.REF, node.getVName());
    return new JavaNode(node);
  }

  @Override
  public JavaNode visitTypeArray(JCArrayTypeTree arrayType, TreeContext owner) {
    TreeContext ctx = owner.down(arrayType);

    JavaNode typeNode = scan(arrayType.getType(), ctx);
    EntrySet node =
        entrySets.newTApplyAndEmit(
            entrySets.newBuiltinAndEmit("array").getVName(),
            Arrays.asList(typeNode.getVName()),
            MarkedSources.ARRAY_TAPP);
    emitAnchor(ctx, EdgeKind.REF, node.getVName());
    return new JavaNode(node);
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
    ImmutableList.Builder<VName> wildcards = ImmutableList.builder();
    wildcards.add(node.getVName());

    if (wild.getKind() != Kind.UNBOUNDED_WILDCARD) {
      JavaNode bound = scan(wild.getBound(), ctx);
      emitEdge(
          node,
          wild.getKind() == Kind.EXTENDS_WILDCARD ? EdgeKind.BOUNDED_UPPER : EdgeKind.BOUNDED_LOWER,
          bound);
      wildcards.addAll(bound.childWildcards);
    }
    return new JavaNode(node, wildcards.build());
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

  private boolean visitDocComment(VName node, VName absNode, JCModifiers modifiers) {
    Optional<String> deprecation = Optional.empty();
    boolean documented = false;
    if (docScanner != null) {
      DocCommentVisitResult result = docScanner.visitDocComment(treePath, node, absNode);
      documented = result.documented();
      deprecation = result.deprecation();
    }
    if (!deprecation.isPresent() && modifiers != null) {
      // emit tags/deprecated if a @Deprecated annotation is present even if there isn't @deprecated
      // javadoc
      if (modifiers.getAnnotations().stream()
          .map(
              a -> {
                if (a == null) {
                  logger.atWarning().log("annotation was null in %s", node.getPath());
                  return null;
                } else if (a.annotationType == null) {
                  logger.atWarning().log(
                      "%s -- a.annotationType was null in %s", a, node.getPath());
                  return null;
                } else if (a.annotationType.type == null) {
                  logger.atWarning().log(
                      "%s -- a.annotationType.type was null in %s",
                      a.annotationType, node.getPath());
                  return null;
                } else if (a.annotationType.type.tsym == null) {
                  logger.atWarning().log(
                      "%s -- a.annotationType.type.tsym was null in %s",
                      a.annotationType.type, node.getPath());
                  return null;
                }
                return a.annotationType.type.tsym.getQualifiedName();
              })
          .anyMatch(n -> n != null && n.contentEquals("java.lang.Deprecated"))) {
        deprecation = Optional.of("");
      }
    }
    emitDeprecated(deprecation, node);
    if (absNode != null) {
      emitDeprecated(deprecation, absNode);
    }
    return documented;
  }

  // // Utility methods ////

  void emitDocReference(Symbol sym, int startChar, int endChar) {
    VName node = getNode(sym);
    if (node == null) {
      if (config.getVerboseLogging()) {
        logger.atWarning().log("failed to emit documentation reference to %s", sym);
      }
      return;
    }

    Span loc =
        new Span(
            filePositions.charToByteOffset(startChar), filePositions.charToByteOffset(endChar));
    EntrySet anchor = entrySets.newAnchorAndEmit(filePositions, loc);
    if (anchor != null) {
      emitAnchor(anchor, EdgeKind.REF_DOC, node, ImmutableList.of());
    }
  }

  void emitDocDiagnostic(JavaFileObject file, Span span, String message) {
    Diagnostic.Builder d = Diagnostic.newBuilder().setMessage(message);
    if (span.isValid()) {
      d.getSpanBuilder().getStartBuilder().setByteOffset(span.getStart());
      d.getSpanBuilder().getEndBuilder().setByteOffset(span.getEnd());
    } else if (span.getStart() >= 0) {
      // If the span isn't valid but we have a valid start, use the start for a zero-width span.
      d.getSpanBuilder().getStartBuilder().setByteOffset(span.getStart());
      d.getSpanBuilder().getEndBuilder().setByteOffset(span.getStart());
    }
    var unused = entrySets.emitDiagnostic(file, d.build());
  }

  int charToLine(int charPosition) {
    return filePositions.charToLine(charPosition);
  }

  boolean emitCommentsOnLine(int line, VName node, int defLine) {
    if (!config.getEmitDocForNonJavadoc()) {
      return false;
    }
    List<Comment> lst = comments.get(line);
    if (lst == null || commentClaims.computeIfAbsent(line, l -> defLine) != defLine) {
      return false;
    }
    for (Comment comment : lst) {
      String bracketed =
          MiniAnchor.bracket(
              comment.text.replaceFirst("^(//|/\\*) ?", "").replaceFirst(" ?\\*/$", ""),
              pos -> pos,
              new ArrayList<>());
      emitDoc(DocKind.LINE, bracketed, new ArrayList<>(), node, null);
    }
    return !lst.isEmpty();
  }

  private static List<VName> toVNames(Iterable<JavaNode> nodes) {
    return Streams.stream(nodes).map(JavaNode::getVName).collect(Collectors.toList());
  }

  /** Create an abs node if we have type variables or if we have wildcards. */
  private @Nullable VName defineTypeParameters(
      TreeContext ownerContext, VName owner, List<JCTypeParameter> params, List<VName> wildcards) {
    if (params.isEmpty() && wildcards.isEmpty()) {
      return null;
    }

    List<VName> typeParams = new ArrayList<>();
    for (JCTypeParameter tParam : params) {
      TreeContext ctx = ownerContext.down(tParam);
      Symbol sym = tParam.type.asElement();
      VName node =
          signatureGenerator
              .getSignature(sym)
              .map(sig -> entrySets.getNode(signatureGenerator, sym, sig, null, null))
              .orElse(null);
      if (node == null) {
        logger.atWarning().log("Could not get type parameter VName: %s", tParam);
        continue;
      }
      emitDefinesBindingAnchorEdge(ctx, tParam.name, tParam.getStartPosition(), node);
      visitAnnotations(node, tParam.getAnnotations(), ctx);
      typeParams.add(node);

      List<JCExpression> bounds = tParam.getBounds();
      List<JavaNode> boundNodes =
          bounds.stream().map(expr -> scan(expr, ctx)).collect(Collectors.toList());
      if (boundNodes.isEmpty()) {
        boundNodes.add(getJavaLangObjectNode());
      }
      emitOrdinalEdges(node, EdgeKind.BOUNDED_UPPER, boundNodes);
    }
    // Add all of the wildcards that roll up to this node. For example:
    // public static <T> void foo(Ty<?> a, Obj<?, ?> b, Obj<Ty<?>, Ty<?>> c) should declare an abs
    // node that has 1 named tparam (T) and 5 unnamed tparam.
    typeParams.addAll(wildcards);

    entrySets.emitOrdinalEdges(owner, EdgeKind.TPARAM, typeParams);
    return owner;
  }

  /** Returns the node associated with a {@link Symbol} or {@code null}. */
  private @Nullable VName getNode(Symbol sym) {
    JavaNode node = getJavaNode(sym);
    return node == null ? null : node.getVName();
  }

  /** Returns the {@link JavaNode} associated with a {@link Symbol} or {@code null}. */
  private @Nullable JavaNode getJavaNode(Symbol sym) {
    if (sym.getKind() == ElementKind.PACKAGE) {
      return new JavaNode(entrySets.newPackageNodeAndEmit((PackageSymbol) sym).getVName())
          .setSymbol(sym);
    }

    VName jvmNode = getJvmNode(sym);

    JavaNode node =
        signatureGenerator
            .getSignature(sym)
            .map(
                sig ->
                    new JavaNode(entrySets.getNode(signatureGenerator, sym, sig, null))
                        .setSymbol(sym))
            .orElse(null);
    if (node != null && jvmNode != null) {
      entrySets.emitEdge(node.getVName(), EdgeKind.NAMED, jvmNode);
    }

    return node;
  }

  private @Nullable VName getJvmNode(Symbol sym) {
    if (isErroneous(sym)) {
      return null;
    }

    Type type = externalType(sym);
    CorpusPath corpusPath = entrySets.jvmCorpusPath(sym);
    if (sym instanceof Symbol.VarSymbol) {
      if (sym.getKind() == ElementKind.FIELD) {
        ReferenceType parentClass = referenceType(externalType(sym.enclClass()));
        String fieldName = sym.getSimpleName().toString();
        return JvmGraph.getFieldVName(corpusPath, parentClass, fieldName);
      }
    } else if (type instanceof Type.MethodType) {
      JvmGraph.Type.MethodType methodJvmType = toMethodJvmType((Type.MethodType) type);
      ReferenceType parentClass = referenceType(externalType(sym.enclClass()));
      String methodName = sym.getQualifiedName().toString();
      return JvmGraph.getMethodVName(corpusPath, parentClass, methodName, methodJvmType);
    } else if (type instanceof Type.ClassType) {
      return JvmGraph.getReferenceVName(corpusPath, referenceType(sym.type));
    }
    return null;
  }

  private void visitAnnotations(
      VName owner, List<JCAnnotation> annotations, TreeContext ownerContext) {
    for (JCAnnotation annotation : annotations) {
      int defPosition = annotation.getPreferredPosition();
      int defLine = filePositions.charToLine(defPosition);
      // Claim trailing annotation comments, which isn't always right, but
      // avoids some confusing comments for method annotations.
      // TODO(danielmoy): don't do this for inline field annotations.
      commentClaims.put(defLine, defLine);
    }
    for (JavaNode node : scanList(annotations, ownerContext)) {
      entrySets.emitEdge(owner, EdgeKind.ANNOTATED_BY, node.getVName());
    }
  }

  // Emits a node for the given sym, an anchor encompassing the TreeContext, and the given edge.
  private JavaNode emitSymUsage(TreeContext ctx, Symbol sym, EdgeKind edgeKind) {
    JavaNode node = getRefNode(ctx, sym);
    if (node == null) {
      // TODO(schroederc): details
      return emitDiagnostic(ctx, "failed to resolve symbol reference", null, null);
    }
    // TODO(schroederc): emit reference to JVM node if `sym.outermostClass()` is not defined in a
    //                   .java source file
    emitAnchor(ctx, edgeKind, node.getVName());
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

    // Ensure the context has a valid source span before searching for the Name.  Otherwise, anchors
    // may accidentily be emitted for Names that happen to appear after the tree context (e.g.
    // lambdas with type-inferred parameters that use the parameter type in the lambda body).
    if (filePositions.getSpan(ctx.getTree()).isValidAndNonZero()) {
      emitAnchor(
          name,
          ctx.getTree().getPreferredPosition(),
          edgeKind,
          node.getVName(),
          ctx.getSnippet(),
          edgeKind.isVariant(EdgeKind.REF_CALL) ? getCallScope(ctx) : getScope(ctx));
      statistics.incrementCounter("name-usages-emitted");
    }
    return node;
  }

  private static ImmutableList<VName> getCallScope(TreeContext ctx) {
    TreeContext parent = ctx.getScope();
    if (parent.getTree() instanceof JCClassDecl) {
      // Special-case callsites in non-static initializer blocks to scope to all constructors.
      return parent.getNode().getClassConstructors();
    }
    return getScope(ctx);
  }

  static enum Visibility {
    PUBLIC("public"),
    PACKAGE("package"),
    PRIVATE("private"),
    PROTECTED("protected");

    private Visibility(String factValue) {
      this.factValue = factValue;
    }

    final String factValue;

    static Visibility get(JCModifiers modifiers, TreeContext ctx) {
      if (modifiers.getFlags().contains(Modifier.PUBLIC)) {
        return PUBLIC;
      }
      if (modifiers.getFlags().contains(Modifier.PRIVATE)) {
        return PRIVATE;
      }
      if (modifiers.getFlags().contains(Modifier.PROTECTED)) {
        return PROTECTED;
      }
      JCClassDecl parent = ctx.getClassParentDecl();
      if (parent == null) {
        return PACKAGE;
      }
      if (parent.getKind().equals(Kind.INTERFACE)) {
        return PUBLIC;
      }
      return PACKAGE;
    }
  }

  private static ImmutableList<VName> getScope(TreeContext ctx) {
    return Optional.ofNullable(ctx.getScope())
        .map(TreeContext::getNode)
        .map(JavaNode::getVName)
        .map(ImmutableList::of)
        .orElse(ImmutableList.of());
  }

  // Returns the reference node for the given symbol.
  private @Nullable JavaNode getRefNode(TreeContext ctx, Symbol sym) {
    // If referencing a generic class, distinguish between generic vs. raw use
    // (e.g., `List` is in generic context in `List<String> x` but not in `List x`).
    try {
      if (sym == null) {
        logger.atWarning().log("sym was null");
        return null;
      }
      if (SignatureGenerator.isArrayHelperClass(sym.enclClass())
          && ctx.getTree() instanceof JCFieldAccess) {
        signatureGenerator.setArrayTypeContext(((JCFieldAccess) ctx.getTree()).selected.type);
      }
      return getJavaNode(sym);
    } finally {
      signatureGenerator.setArrayTypeContext(null);
    }
  }

  // Cached common java.lang.Object node.
  private JavaNode javaLangObjectNode;
  // Cached common java.lang.Enum node.
  private JavaNode javaLangEnumNode;

  // Returns a JavaNode representing java.lang.Object.
  private JavaNode getJavaLangObjectNode() {
    if (javaLangObjectNode == null) {
      javaLangObjectNode = resolveJavaLangSymbol(getSymbols().objectType.asElement());
    }
    return javaLangObjectNode;
  }

  // Returns a JavaNode representing java.lang.Enum<E> where E is a given enum type.
  private JavaNode getJavaLangEnumNode(VName enumVName) {
    if (javaLangEnumNode == null) {
      VName v = resolveJavaLangSymbol(getSymbols().enumSym).getVName();
      javaLangEnumNode = new JavaNode(v);
    }
    EntrySet typeNode =
        entrySets.newTApplyAndEmit(
            javaLangEnumNode.getVName(),
            Collections.singletonList(enumVName),
            MarkedSources.GENERIC_TAPP);
    return new JavaNode(typeNode);
  }

  private JavaNode resolveJavaLangSymbol(Symbol sym) {
    Optional<String> signature = signatureGenerator.getSignature(sym);
    if (!signature.isPresent()) {
      // This usually indicates a problem with the compilation's bootclasspath.
      return emitDiagnostic(null, "failed to resolve " + sym, null, null);
    }
    return new JavaNode(entrySets.getNode(signatureGenerator, sym, signature.get(), null, null))
        .setSymbol(sym);
  }

  private void emitMetadata(Span span, VName node) {
    for (Metadata data : metadata) {
      for (Metadata.Rule rule : data.getRulesForLocation(span.getStart())) {
        if (rule.end == span.getEnd()) {
          if (rule.reverseEdge) {
            entrySets.emitEdge(rule.vname, rule.edgeOut, node);
          } else {
            entrySets.emitEdge(node, rule.edgeOut, rule.vname);
          }

          if (rule.semantic == Semantic.SET) {
            VName genFuncVName = rule.reverseEdge ? node : rule.vname;
            entrySets.getEmitter().emitFact(genFuncVName, "/kythe/semantic/generated", "set");
          }
        }
      }
    }
  }

  private EntrySet emitDefinesBindingAnchorEdge(
      TreeContext ctx, Name name, int startOffset, VName node) {
    EntrySet anchor =
        emitAnchor(
            name, startOffset, EdgeKind.DEFINES_BINDING, node, ctx.getSnippet(), getScope(ctx));
    Span span = filePositions.findIdentifier(name, startOffset);
    if (span != null) {
      emitMetadata(span, node);
    }
    return anchor;
  }

  private void emitDefinesBindingEdge(Span span, EntrySet anchor, VName node, List<VName> scope) {
    emitMetadata(span, node);
    emitAnchor(anchor, EdgeKind.DEFINES_BINDING, node, scope);
  }

  // Creates/emits an anchor and an associated edge
  private EntrySet emitAnchor(TreeContext anchorContext, EdgeKind kind, VName node) {
    return emitAnchor(
        entrySets.newAnchorAndEmit(
            filePositions, anchorContext.getTreeSpan(), anchorContext.getSnippet()),
        kind,
        node,
        kind.isVariant(EdgeKind.REF_CALL) ? getCallScope(anchorContext) : getScope(anchorContext));
  }

  // Creates/emits an anchor (for an identifier) and an associated edge
  private @Nullable EntrySet emitAnchor(
      Name name, int startOffset, EdgeKind kind, VName node, Span snippet, List<VName> scope) {
    EntrySet anchor = entrySets.newAnchorAndEmit(filePositions, name, startOffset, snippet);
    if (anchor == null) {
      // TODO(schroederc): Special-case these anchors (most come from visitSelect)
      return null;
    }
    return emitAnchor(anchor, kind, node, scope);
  }
  // Creates/emits an anchor and an associated edge
  private @Nullable EntrySet emitAnchor(
      EntrySet anchor, EdgeKind kind, VName node, List<VName> scope) {
    Preconditions.checkArgument(
        kind.isAnchorEdge(), "EdgeKind was not intended for ANCHORs: %s", kind);
    if (anchor == null) {
      return null;
    }
    entrySets.emitEdge(anchor.getVName(), kind, node);
    if (kind.isVariant(EdgeKind.REF_CALL) || config.getEmitAnchorScopes()) {
      scope.forEach(s -> entrySets.emitEdge(anchor.getVName(), EdgeKind.CHILDOF, s));
    }
    return anchor;
  }

  private void emitComment(JCTree defTree, VName node) {
    int defPosition = defTree.getPreferredPosition();
    int defLine = filePositions.charToLine(defPosition);
    emitCommentsOnLine(defLine, node, defLine);
    emitCommentsOnLine(defLine - 1, node, defLine);
  }

  void emitDoc(
      DocKind kind, String bracketedText, Iterable<Symbol> params, VName node, VName absNode) {
    List<VName> paramNodes = new ArrayList<>();
    for (Symbol s : params) {
      VName paramNode = getNode(s);
      if (paramNode == null) {
        return;
      }
      paramNodes.add(paramNode);
    }
    EntrySet doc =
        entrySets.newDocAndEmit(kind.getDocSubkind(), filePositions, bracketedText, paramNodes);
    entrySets.emitEdge(doc.getVName(), EdgeKind.DOCUMENTS, node);
    if (absNode != null) {
      entrySets.emitEdge(doc.getVName(), EdgeKind.DOCUMENTS, absNode);
    }
  }

  private void emitDeprecated(Optional<String> deprecation, VName node) {
    deprecation.ifPresent(d -> entrySets.getEmitter().emitFact(node, "/kythe/tag/deprecated", d));
  }

  private void emitModifiers(VName node, JCModifiers modifiers) {
    if (modifiers.getFlags().contains(Modifier.ABSTRACT)) {
      entrySets.getEmitter().emitFact(node, "/kythe/tag/abstract", "");
    }

    if (modifiers.getFlags().contains(Modifier.STATIC)) {
      entrySets.getEmitter().emitFact(node, "/kythe/tag/static", "");
    }

    if (modifiers.getFlags().contains(Modifier.VOLATILE)) {
      entrySets.getEmitter().emitFact(node, "/kythe/tag/volatile", "");
    }

    if (modifiers.getFlags().contains(Modifier.DEFAULT)) {
      entrySets.getEmitter().emitFact(node, "/kythe/tag/default", "");
    }
  }

  private void emitVisibility(VName node, JCModifiers modifiers, TreeContext ctx) {
    entrySets
        .getEmitter()
        .emitFact(node, "/kythe/visibility", Visibility.get(modifiers, ctx).factValue);
  }

  // Unwraps the target EntrySet and emits an edge to it from the sourceNode
  private void emitEdge(EntrySet sourceNode, EdgeKind kind, JavaNode target) {
    entrySets.emitEdge(sourceNode.getVName(), kind, target.getVName());
  }

  // Unwraps each target JavaNode and emits an ordinal edge to each from the given source node
  private void emitOrdinalEdges(VName node, EdgeKind kind, List<JavaNode> targets) {
    entrySets.emitOrdinalEdges(node, kind, toVNames(targets));
  }

  private JavaNode emitDiagnostic(TreeContext ctx, String message, String details, String context) {
    Diagnostic.Builder d = Diagnostic.newBuilder().setMessage(message);
    if (details != null) {
      d.setDetails(details);
    }
    if (context != null) {
      d.setContextUrl(context);
    }
    if (ctx != null) {
      Span s = ctx.getTreeSpan();
      if (s.isValid()) {
        d.getSpanBuilder().getStartBuilder().setByteOffset(s.getStart());
        d.getSpanBuilder().getEndBuilder().setByteOffset(s.getEnd());
      } else if (s.getStart() >= 0) {
        // If the span isn't valid but we have a valid start, use the start for a zero-width span.
        d.getSpanBuilder().getStartBuilder().setByteOffset(s.getStart());
        d.getSpanBuilder().getEndBuilder().setByteOffset(s.getStart());
      }
    }
    EntrySet node = entrySets.emitDiagnostic(filePositions, d.build());
    // TODO(schroederc): don't allow any edges to a diagnostic node
    return new JavaNode(node);
  }

  private <T extends JCTree> List<JavaNode> scanList(List<T> trees, TreeContext owner) {
    List<JavaNode> nodes = new ArrayList<>();
    for (T t : trees) {
      nodes.add(scan(t, owner));
    }
    return nodes;
  }

  private void loadAnnotationsFile(String path) {
    URI uri = filePositions.getSourceFile().toUri();
    try {
      String fullPath = resolveSourcePath(path);
      if (metadataFilePaths.contains(fullPath)) {
        return;
      }
      FileObject file = Iterables.getOnlyElement(fileManager.getJavaFileObjects(fullPath), null);
      if (file == null) {
        logger.atWarning().log("Can't find metadata %s for %s at %s", path, uri, fullPath);
        return;
      }
      loadAnnotationsFile(fullPath, file);
    } catch (IllegalArgumentException ex) {
      logger.atWarning().withCause(ex).log("Can't read metadata %s for %s", path, uri);
    }
  }

  private void loadAnnotationsFile(String fullPath, FileObject file) {
    try (InputStream stream = file.openInputStream()) {
      Metadata newMetadata = metadataLoaders.parseFile(fullPath, ByteStreams.toByteArray(stream));
      if (newMetadata == null) {
        logger.atWarning().log("Can't load metadata %s", fullPath);
        return;
      }
      metadata.add(newMetadata);
      metadataFilePaths.add(fullPath);
    } catch (IOException ex) {
      logger.atWarning().withCause(ex).log("Can't read metadata for %s", fullPath);
    }
  }

  private void loadImplicitAnnotationsFile() {
    URI uri = filePositions.getSourceFile().toUri();
    String name = Paths.get(uri.getPath()).getFileName().toString();
    try {
      String fullPath = resolveSourcePath(name + ".pb.meta");
      if (metadataFilePaths.contains(fullPath)) {
        return;
      }
      FileObject file = Iterables.getOnlyElement(fileManager.getJavaFileObjects(fullPath));
      // getJavaFileObjects is only guaranteed to check that the path isn't a directory, not whether
      // it exists.
      if (file == null || !isFileReadable(file)) {
        return;
      }
      loadAnnotationsFile(fullPath, file);
    } catch (IllegalArgumentException ex) {
      // However, in practice it will also raise IllegalArgumentException if the file
      // does not exist.
      logger.atFine().withCause(ex).log("Can't read implicit metadata for %s", uri);
    }
  }

  private void loadAnnotationsData(String fullPath, String metadataComment) {
    Metadata newMetadata = metadataLoaders.parseFile(fullPath, metadataComment.getBytes(UTF_8));
    if (newMetadata == null) {
      logger.atWarning().log("Can't load metadata %s", fullPath);
      return;
    }
    metadata.add(newMetadata);
    metadataFilePaths.add(fullPath);
  }

  private void loadInlineMetadata(String metadataPrefix) {
    metadataPrefix += ":";
    String fullPath = filePositions.getSourceFile().toUri().getPath();
    try {
      if (metadataFilePaths.contains(fullPath)) {
        return;
      }
      for (List<Comment> commentList : comments.values()) {
        for (Comment comment : commentList) {
          int index = comment.text.indexOf(metadataPrefix);
          if (index != -1) {
            loadAnnotationsData(
                fullPath,
                Metadata.ANNOTATION_COMMENT_INLINE_METADATA_PREFIX
                    + comment.text.substring(index + metadataPrefix.length()));
            break;
          }
        }
      }
    } catch (IllegalArgumentException ex) {
      logger.atWarning().withCause(ex).log("Can't read metadata at %s", fullPath);
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
          || !GENERATED_ANNOTATIONS.contains(annotationSymbol.toString())) {
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
        if (!lhs.name.contentEquals("comments") || !(rhs.getValue() instanceof String)) {
          continue;
        }
        String comments = (String) rhs.getValue();
        if (comments.startsWith(Metadata.ANNOTATION_COMMENT_PREFIX)) {
          loadAnnotationsFile(comments.substring(Metadata.ANNOTATION_COMMENT_PREFIX.length()));
        } else if (comments.startsWith(Metadata.ANNOTATION_COMMENT_INLINE_METADATA_PREFIX)) {
          loadInlineMetadata(
              comments.substring(Metadata.ANNOTATION_COMMENT_INLINE_METADATA_PREFIX.length()));
        }
      }
    }
  }

  /** Resovles a string as a source-file relative path */
  private String resolveSourcePath(String path) {
    try {
      return fileManager.asPath(filePositions.getSourceFile()).resolveSibling(path).toString();
    } catch (UnsupportedOperationException unused) {
      // Do nothing; perform fallback below
    }
    // Fallback to URI-based path resolution when asPath is unsupported.
    URI uri = filePositions.getSourceFile().toUri();
    String fullPath = uri.resolve(path).getPath();
    if (fullPath.startsWith("/")) {
      fullPath = fullPath.substring(1);
    }
    return fullPath;
  }

  /** Check if a {@link Symbol} is erroneous or produces an exception. */
  private boolean isErroneous(Symbol sym) {
    try {
      return sym.asType().isErroneous() || sym.enclClass().asType().isErroneous();
    } catch (Symbol.CompletionFailure | AssertionError | NullPointerException f) {
      return true;
    }
  }

  private Type externalType(Symbol sym) {
    return sym.externalType(Types.instance(javaContext));
  }

  static JvmGraph.Type toJvmType(Type type) {
    switch (type.getTag()) {
      case ARRAY:
        return JvmGraph.Type.arrayType(toJvmType(((Type.ArrayType) type).getComponentType()));
      case CLASS:
        return referenceType(type);
      case METHOD:
        return toMethodJvmType(type.asMethodType());
      case TYPEVAR:
        return referenceType(type);

      case BOOLEAN:
        return JvmGraph.Type.booleanType();
      case BYTE:
        return JvmGraph.Type.byteType();
      case CHAR:
        return JvmGraph.Type.charType();
      case DOUBLE:
        return JvmGraph.Type.doubleType();
      case FLOAT:
        return JvmGraph.Type.floatType();
      case INT:
        return JvmGraph.Type.intType();
      case LONG:
        return JvmGraph.Type.longType();
      case SHORT:
        return JvmGraph.Type.shortType();

      case ERROR:
        // Assume reference type; avoid crashing
        return referenceType(type);

      default:
        throw new IllegalStateException("unhandled Java Type: " + type.getTag() + " -- " + type);
    }
  }

  /** Returns a new JVM class/enum/interface type descriptor to the specified source type. */
  private static ReferenceType referenceType(Type referent) {
    String qualifiedName = referent.tsym.flatName().toString();
    return JvmGraph.Type.referenceType(qualifiedName);
  }

  /** Returns true if the FileObject exists and is readable */
  private static boolean isFileReadable(FileObject file) {
    try (InputStream stream = file.openInputStream()) {
      return true;
    } catch (IOException unused) {
      return false;
    }
  }

  private static JvmGraph.VoidableType toJvmReturnType(Type type) {
    switch (type.getTag()) {
      case VOID:
        return JvmGraph.Type.voidType();
      default:
        return toJvmType(type);
    }
  }

  static JvmGraph.Type.MethodType toMethodJvmType(Type.MethodType type) {
    return JvmGraph.Type.methodType(
        type.getParameterTypes().stream()
            .map(KytheTreeScanner::toJvmType)
            .collect(Collectors.toList()),
        toJvmReturnType(type.getReturnType()));
  }

  static enum DocKind {
    JAVADOC(Optional.of("javadoc")),
    LINE;

    private final Optional<String> subkind;

    private DocKind(Optional<String> subkind) {
      this.subkind = subkind;
    }

    private DocKind() {
      this(Optional.empty());
    }

    /** Returns the Kythe subkind for this type of document. */
    public Optional<String> getDocSubkind() {
      return subkind;
    }
  }

  /** An enum representing whether a reference reads from a symbol, writes to it, or both. */
  private enum RefType {
    READ,
    WRITE,
    READ_WRITE,
  }

  /**
   * Returns the type of reference for a symbol in a tree at a specific position.
   *
   * @param tree The {@link JCTree} containing the reference
   * @param parent The parent {@link JCTree} of {@code tree}
   * @param sym The {@link Symbol} being referenced
   * @param position The integer position of the {@link Symbol} being referenced
   * @return A {@link RefType} indicating whether the reference reads from the symbol, writes to it,
   *     or does both.
   */
  private RefType getRefType(
      TreeContext ctx, JCTree tree, @Nullable JCTree parent, Symbol sym, int position) {
    // JCAssign looks like "a = 1 + 2"
    // JCAssignOp looks like "a += 3"
    if (tree instanceof JCAssign || tree instanceof JCAssignOp) {
      JCExpression lhs;
      JCExpression rhs;
      if (tree instanceof JCAssign) {
        lhs = ((JCAssign) tree).lhs;
        rhs = ((JCAssign) tree).rhs;
      } else {
        lhs = ((JCAssignOp) tree).lhs;
        rhs = ((JCAssignOp) tree).rhs;
      }

      // Check if the symbol is on the left side of the assignment, which is always at least a WRITE
      // reference, or if the symbol experiences a write on the right side such as "a = b++",
      // which is always a READ_WRITE.
      if (expressionMatchesSymbol(ctx, lhs, sym, position)) {
        // We also need to check if this reference is READ_WRITE. This is true in the following
        // cases:
        //   - The tree is a JCAssignOp such as "b++;" or
        //   - The parent tree is a JCAssign and the current tree is on the RHS of that assignment.
        //     An example of this would be "a = b = 1;", where b is both written to and read from.
        RefType refType = RefType.WRITE;
        if (tree instanceof JCAssignOp) {
          refType = RefType.READ_WRITE;
        } else if (parent instanceof JCAssign && ((JCAssign) parent).rhs.getTree().equals(tree)) {
          refType = RefType.READ_WRITE;
        }
        return refType;
      } else if (symbolWrittenOnRHS(ctx, rhs, sym, position)) {
        return RefType.READ_WRITE;
      }
    } else if (tree instanceof JCExpressionStatement) {
      JCExpressionStatement stmt = (JCExpressionStatement) tree;
      switch (stmt.getExpression().getKind()) {
        case POSTFIX_INCREMENT:
        case POSTFIX_DECREMENT:
        case PREFIX_INCREMENT:
        case PREFIX_DECREMENT:
          return RefType.READ_WRITE;
        default:
          // Some other expression like ! or ~
          return RefType.READ;
      }
    }
    return RefType.READ;
  }

  /**
   * Returns whether a {@link JCExpression}, representing either a {@link JCIdent} or a {@link
   * JCFieldAccess}, matches a {@link Symbol} at a given integer position.
   */
  private boolean expressionMatchesSymbol(
      TreeContext ctx, JCExpression e, Symbol sym, int position) {
    if (e instanceof JCIdent) {
      JCIdent ident = (JCIdent) e;
      if (ident.sym.equals(sym) && ident.pos == position) {
        return true;
      }
    } else if (e instanceof JCFieldAccess) {
      JCFieldAccess fieldAccess = (JCFieldAccess) e;
      if (fieldAccess.sym == null) {
        String name = fieldAccess.name == null ? "" : fieldAccess.name.toString();
        logger.atWarning().log(
            "Missing field access symbol: %s at %s:%s",
            name, ctx.getSourcePath(), ctx.getTreeSpan());
        var unused = emitDiagnostic(ctx, "missing field access symbol: " + name, null, null);
      } else if (fieldAccess.sym.equals(sym) && fieldAccess.pos == position) {
        return true;
      }
    }
    return false;
  }

  /**
   * Returns whether a {@link Symbol} at a given integer position is written to in a {@link
   * JCExpression} from the right-hand side of a {@link JCAssign} or {@link JCAssignOp} expression.
   */
  private boolean symbolWrittenOnRHS(TreeContext ctx, JCExpression e, Symbol sym, int position) {
    if (e instanceof JCParens) {
      return symbolWrittenOnRHS(ctx, ((JCParens) e).expr, sym, position);
    } else if (e instanceof JCUnary) {
      switch (e.getTree().getKind()) {
        case POSTFIX_INCREMENT:
        case POSTFIX_DECREMENT:
        case PREFIX_INCREMENT:
        case PREFIX_DECREMENT:
          return expressionMatchesSymbol(ctx, ((JCUnary) e).arg, sym, position);
        default:
          return false;
      }
    } else if (e instanceof JCBinary) {
      JCExpression lhs = ((JCBinary) e).lhs;
      JCExpression rhs = ((JCBinary) e).rhs;
      // Process the each side recursively because they could be another binary expression
      return symbolWrittenOnRHS(ctx, lhs, sym, position)
          || symbolWrittenOnRHS(ctx, rhs, sym, position);
    }
    return false;
  }
}
