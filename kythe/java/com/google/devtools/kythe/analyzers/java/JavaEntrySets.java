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

import com.google.common.collect.HashMultiset;
import com.google.common.collect.Lists;
import com.google.common.collect.Multiset;
import com.google.devtools.kythe.analyzers.base.CorpusPath;
import com.google.devtools.kythe.analyzers.base.EdgeKind;
import com.google.devtools.kythe.analyzers.base.EntrySet;
import com.google.devtools.kythe.analyzers.base.FactEmitter;
import com.google.devtools.kythe.analyzers.base.KytheEntrySets;
import com.google.devtools.kythe.analyzers.base.NodeKind;
import com.google.devtools.kythe.analyzers.java.SourceText.Positions;
import com.google.devtools.kythe.platform.java.helpers.SignatureGenerator;
import com.google.devtools.kythe.platform.shared.StatisticsCollector;
import com.google.devtools.kythe.proto.Analysis.CompilationUnit.FileInput;
import com.google.devtools.kythe.proto.Diagnostic;
import com.google.devtools.kythe.proto.Link;
import com.google.devtools.kythe.proto.MarkedSource;
import com.google.devtools.kythe.proto.Storage.VName;
import com.google.devtools.kythe.util.KytheURI;
import com.google.devtools.kythe.util.Span;
import com.sun.tools.javac.code.Flags;
import com.sun.tools.javac.code.Symbol;
import com.sun.tools.javac.code.Symbol.ClassSymbol;
import com.sun.tools.javac.code.Symbol.MethodSymbol;
import com.sun.tools.javac.code.Symbol.PackageSymbol;
import com.sun.tools.javac.code.Type;
import com.sun.tools.javac.code.TypeTag;
import com.sun.tools.javac.tree.JCTree;
import java.io.UnsupportedEncodingException;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import javax.annotation.Nullable;
import javax.lang.model.element.ElementKind;
import javax.lang.model.element.Modifier;
import javax.lang.model.element.Name;
import javax.tools.JavaFileObject;

/** Specialization of {@link KytheEntrySets} for Java. */
public class JavaEntrySets extends KytheEntrySets {
  private final Map<Symbol, EntrySet> symbolNodes = new HashMap<>();
  private final Map<Symbol, Integer> symbolHashes = new HashMap<>();
  private final boolean ignoreVNamePaths;
  private final boolean ignoreVNameRoots;
  private final String overrideJdkCorpus;
  private Map<String, Integer> sourceToWildcardCounter = new HashMap<>();

  public JavaEntrySets(
      StatisticsCollector statistics,
      FactEmitter emitter,
      VName compilationVName,
      List<FileInput> requiredInputs,
      boolean ignoreVNamePaths,
      boolean ignoreVNameRoots,
      String overrideJdkCorpus) {
    super(statistics, emitter, compilationVName, requiredInputs);
    this.ignoreVNamePaths = ignoreVNamePaths;
    this.ignoreVNameRoots = ignoreVNameRoots;
    this.overrideJdkCorpus = overrideJdkCorpus;
  }

  /**
   * The only place the integer index for nested classes/anonymous classes is stored is in the
   * flatname of the symbol. (This index is determined at compile time using linear search; see
   * 'localClassName' in Check.java). The simple name can't be relied on; for nested classes it
   * drops the name of the parent class (so 'pkg.OuterClass$Inner' yields only 'Inner') and for
   * anonymous classes it's blank. For multiply-nested classes, we'll see tokens like
   * 'OuterClass$Inner$1$1'.
   */
  private String getIdentToken(Symbol sym, SignatureGenerator signatureGenerator) {
    // If the symbol represents the generated `Array` class, replace it with the actual
    // array type, if we have it.
    if (SignatureGenerator.isArrayHelperClass(sym) && signatureGenerator != null) {
      return signatureGenerator.getArrayTypeName();
    }
    String flatName = sym.flatName().toString();
    int lastDot = flatName.lastIndexOf('.');
    // A$1 is a valid variable/method name, so make sure we only look at $ in class names.
    int lastCash = (sym instanceof ClassSymbol) ? flatName.lastIndexOf('$') : -1;
    int lastTok = lastDot > lastCash ? lastDot : lastCash;
    String identToken = lastTok < 0 ? flatName : flatName.substring(lastTok + 1);
    if (!identToken.isEmpty() && Character.isDigit(identToken.charAt(0))) {
      identToken = "(anon " + identToken + ")";
    }
    return identToken;
  }

  /**
   * Returns the Symbol for sym's parent in qualified names, assuming that we'll be using
   * getIdentToken() to print nodes.
   *
   * <p>We're going through this extra effort to try and give people unsurprising qualified names.
   * To do that we have to deal with javac's mangling (in {@link #getIdentToken} above), since for
   * anonymous classes javac only stores mangled symbols. The code as written will emit only dotted
   * fully-qualified names, even for inner or anonymous classes, and considers concrete type,
   * package, or method names to be appropriate dot points. (If we weren't careful here we might,
   * for example, observe nodes in a qualified name corresponding to variables that are initialized
   * to anonymous classes.) This reflects the nesting structure from the Java side, not the JVM
   * side.
   */
  @Nullable
  private Symbol getQualifiedNameParent(Symbol sym) {
    sym = sym.owner;
    while (sym != null) {
      switch (sym.kind) {
        case TYP:
          if (!sym.type.hasTag(TypeTag.TYPEVAR)) {
            return sym;
          }
          break;
        case PCK:
        case MTH:
          return sym;
          // TODO(T227): resolve non-exhaustive switch statements w/o defaults
        default:
          break;
      }
      sym = sym.owner;
    }
    return null;
  }

  /**
   * Returns a {@link MarkedSource} instance for sym's type (or its return type, if sym is a
   * method). If there is no appropriate type for sym, returns null. Generates links with
   * signatureGenerator.
   */
  @Nullable
  private MarkedSource markType(SignatureGenerator signatureGenerator, Symbol sym) {
    // TODO(zarko): Mark up any annotations.
    Type type = sym.type;
    if (type == null || sym == type.tsym) {
      return null;
    }
    boolean wasArray = false;
    if (type.getReturnType() != null) {
      type = type.getReturnType();
    }
    if (type.hasTag(TypeTag.ARRAY) && ((Type.ArrayType) type).elemtype != null) {
      wasArray = true;
      type = ((Type.ArrayType) type).elemtype;
    }
    MarkedSource.Builder builder = MarkedSource.newBuilder().setKind(MarkedSource.Kind.TYPE);
    if (type.hasTag(TypeTag.CLASS)) {
      MarkedSource.Builder context = MarkedSource.newBuilder();
      String identToken = buildContext(context, type.tsym, signatureGenerator);
      builder.addChild(context.build());
      builder.addChild(
          MarkedSource.newBuilder()
              .setKind(MarkedSource.Kind.IDENTIFIER)
              .setPreText(identToken + (wasArray ? "[] " : " "))
              .build());
      Optional<String> signature = signatureGenerator.getSignature(type.tsym);
      if (signature.isPresent()) {
        EntrySet node = getNode(signatureGenerator, type.tsym, signature.get(), null, null);
        builder.addLink(Link.newBuilder().addDefinition(new KytheURI(node.getVName()).toString()));
      }
    } else {
      builder.addChild(
          MarkedSource.newBuilder()
              .setKind(MarkedSource.Kind.IDENTIFIER)
              .setPreText(type.toString() + (wasArray ? "[] " : " "))
              .build());
    }
    return builder.build();
  }

  /**
   * Sets the provided {@link MarkedSource.Builder} to a CONTEXT node, populating it with the
   * fully-qualified parent scope for sym. Returns the identifier corresponding to sym.
   */
  private String buildContext(
      MarkedSource.Builder context, Symbol sym, SignatureGenerator signatureGenerator) {
    context.setKind(MarkedSource.Kind.CONTEXT).setPostChildText(".").setAddFinalListToken(true);
    String identToken = getIdentToken(sym, signatureGenerator);
    Symbol parent = getQualifiedNameParent(sym);
    List<MarkedSource> parents = Lists.newArrayList();
    while (parent != null) {
      String parentName = getIdentToken(parent, signatureGenerator);
      if (!parentName.isEmpty()) {
        parents.add(
            MarkedSource.newBuilder()
                .setKind(MarkedSource.Kind.IDENTIFIER)
                .setPreText(parentName)
                .build());
      }
      parent = getQualifiedNameParent(parent);
    }
    for (int i = 0; i < parents.size(); ++i) {
      context.addChild(parents.get(parents.size() - i - 1));
    }
    return identToken;
  }

  /**
   * Returns a node for the given {@link Symbol} and its signature. A new node is created and
   * emitted if necessary. If non-null, msBuilder will be used to generate a signature.
   */
  public EntrySet getNode(
      SignatureGenerator signatureGenerator,
      Symbol sym,
      String signature,
      // TODO(schroederc): separate MarkedSource generation from JavaEntrySets
      @Nullable MarkedSource.Builder msBuilder,
      @Nullable Iterable<MarkedSource> postChildren) {
    EntrySet node;
    if ((node = symbolNodes.get(sym)) != null) {
      return node;
    }

    ClassSymbol enclClass = sym.enclClass();
    VName v = lookupVName(enclClass);
    if ((v == null || overrideJdkCorpus != null) && fromJDK(sym)) {
      v =
          VName.newBuilder()
              .setCorpus(overrideJdkCorpus != null ? overrideJdkCorpus : "jdk")
              .build();
    }

    if (v == null) {
      node = getNameAndEmit(signature);
      // NAME node was already emitted
    } else {
      if (ignoreVNamePaths) {
        v = v.toBuilder().setPath(enclClass != null ? enclClass.toString() : "").build();
      }
      if (ignoreVNameRoots) {
        v = v.toBuilder().clearRoot().build();
      }

      MarkedSource.Builder markedSource = msBuilder == null ? MarkedSource.newBuilder() : msBuilder;
      MarkedSource markedType = markType(signatureGenerator, sym);
      if (markedType != null) {
        markedSource.addChild(markedType);
      }
      MarkedSource.Builder context = MarkedSource.newBuilder();
      String identToken = buildContext(context, sym, signatureGenerator);
      markedSource.addChild(context.build());
      switch (sym.getKind()) {
        case TYPE_PARAMETER:
          markedSource.addChild(
              MarkedSource.newBuilder()
                  .setKind(MarkedSource.Kind.IDENTIFIER)
                  .setPreText("<" + sym.getSimpleName().toString() + ">")
                  .build());
          break;
        case CONSTRUCTOR:
        case METHOD:
          String methodName;
          if (sym.getKind() == ElementKind.CONSTRUCTOR && enclClass != null) {
            methodName = enclClass.getSimpleName().toString();
          } else {
            methodName = sym.getSimpleName().toString();
          }
          markedSource.addChild(
              MarkedSource.newBuilder()
                  .setKind(MarkedSource.Kind.IDENTIFIER)
                  .setPreText(methodName)
                  .build());
          markedSource.addChild(
              MarkedSource.newBuilder()
                  .setKind(MarkedSource.Kind.PARAMETER_LOOKUP_BY_PARAM)
                  .setPreText("(")
                  .setPostChildText(", ")
                  .setPostText(")")
                  .build());
          break;
        default:
          markedSource.addChild(
              MarkedSource.newBuilder()
                  .setKind(MarkedSource.Kind.IDENTIFIER)
                  .setPreText(identToken)
                  .build());
          break;
      }
      if (postChildren != null) {
        postChildren.forEach(markedSource::addChild);
      }

      NodeKind kind = elementNodeKind(sym.getKind());
      NodeBuilder builder = kind != null ? newNode(kind) : newNode(sym.getKind().toString());
      builder.setCorpusPath(CorpusPath.fromVName(v)).setProperty("code", markedSource.build());

      if (signatureGenerator.getUseJvmSignatures()) {
        builder.setSignature(signature);
      } else {
        builder.addSignatureSalt(signature).addSignatureSalt("" + hashSymbol(sym));
      }
      node = builder.build();
      node.emit(getEmitter());
    }

    symbolNodes.put(sym, node);
    return node;
  }

  /** Emits and returns a new {@link EntrySet} representing Javadoc. */
  public EntrySet newDocAndEmit(Positions filePositions, String text, Iterable<EntrySet> params) {
    VName fileVName = getFileVName(getDigest(filePositions.getSourceFile()));
    byte[] encodedText;
    try {
      encodedText = text.getBytes("UTF-8");
    } catch (UnsupportedEncodingException ex) {
      encodedText = new byte[0];
    }
    NodeBuilder builder =
        newNode(NodeKind.DOC)
            .setCorpusPath(CorpusPath.fromVName(fileVName))
            .setProperty("text", encodedText)
            .addSignatureSalt(text);
    params.forEach(param -> builder.addSignatureSalt(param.getVName()));
    EntrySet node = emitAndReturn(builder);
    emitOrdinalEdges(node, EdgeKind.PARAM, params);
    return node;
  }

  /** Emits and returns a new {@link EntrySet} representing the Java file. */
  public EntrySet newFileNodeAndEmit(Positions file) {
    return newFileNodeAndEmit(getDigest(file.getSourceFile()), file.getData(), file.getEncoding());
  }

  /** Emits and returns a new {@link EntrySet} representing a Java package. */
  public EntrySet newPackageNodeAndEmit(PackageSymbol sym) {
    return newPackageNodeAndEmit(sym.getQualifiedName().toString());
  }

  /** Emits and returns a new {@link EntrySet} representing a Java package. */
  public EntrySet newPackageNodeAndEmit(String name) {
    EntrySet node =
        emitAndReturn(
            newNode(NodeKind.PACKAGE)
                .addSignatureSalt(name)
                .setProperty(
                    "code",
                    MarkedSource.newBuilder()
                        .setPreText(name)
                        .setKind(MarkedSource.Kind.IDENTIFIER)
                        .build()));
    return node;
  }

  /** Emits and returns a new {@link EntrySet} for the given wildcard. */
  public EntrySet newWildcardNodeAndEmit(JCTree.JCWildcard wild, String sourcePath) {
    int counter = sourceToWildcardCounter.getOrDefault(sourcePath, 0);
    sourceToWildcardCounter.put(sourcePath, counter + 1);
    return emitAndReturn(newNode(NodeKind.ABS_VAR).addSignatureSalt(sourcePath + counter));
  }

  /** Returns and emits a Java anchor for the given offset span. */
  public EntrySet newAnchorAndEmit(Positions filePositions, Span loc) {
    return newAnchorAndEmit(filePositions, loc, null);
  }

  /** Returns and emits a Java anchor for the given offset span. */
  public EntrySet newAnchorAndEmit(Positions filePositions, Span loc, Span snippet) {
    return newAnchorAndEmit(getFileVName(getDigest(filePositions.getSourceFile())), loc, snippet);
  }

  /** Returns and emits a Java anchor for the given identifier. */
  public EntrySet newAnchorAndEmit(
      Positions filePositions, Name name, int startOffset, Span snippet) {
    Span span = filePositions.findIdentifier(name, startOffset);
    return span == null
        ? null
        : newAnchorAndEmit(getFileVName(getDigest(filePositions.getSourceFile())), span, snippet);
  }

  /** Emits and returns a DIAGNOSTIC node attached to the given file. */
  public EntrySet emitDiagnostic(Positions filePositions, Diagnostic d) {
    return emitDiagnostic(getFileVName(getDigest(filePositions.getSourceFile())), d);
  }

  /** Returns the equivalent {@link NodeKind} for the given {@link ElementKind}. */
  @Nullable
  private static NodeKind elementNodeKind(ElementKind kind) {
    switch (kind) {
      case CLASS:
        return NodeKind.RECORD_CLASS;
      case ENUM:
        return NodeKind.SUM_ENUM_CLASS;
      case ENUM_CONSTANT:
        return NodeKind.CONSTANT;
      case ANNOTATION_TYPE:
      case INTERFACE:
        return NodeKind.INTERFACE;

      case EXCEPTION_PARAMETER:
        return NodeKind.VARIABLE_EXCEPTION;
      case FIELD:
        return NodeKind.VARIABLE_FIELD;
      case LOCAL_VARIABLE:
        return NodeKind.VARIABLE_LOCAL;
      case PARAMETER:
        return NodeKind.VARIABLE_PARAMETER;
      case RESOURCE_VARIABLE:
        return NodeKind.VARIABLE_RESOURCE;

      case CONSTRUCTOR:
        return NodeKind.FUNCTION_CONSTRUCTOR;
      case METHOD:
        return NodeKind.FUNCTION;
      case TYPE_PARAMETER:
        return NodeKind.ABS_VAR;
      default:
        // TODO(T227): handle all cases, make this exceptional, and remove all null checks
        return null;
    }
  }

  // Returns a consistent hash for the given symbol across separate compilations and JVM instances.
  private int hashSymbol(Symbol sym) {
    // This method is necessary because Symbol, and most other javac internals, do not overload the
    // Object#hashCode() method and the default implementation, System#identityHashCode(Object), is
    // practically useless because it can change across JVM instances.  This method instead only
    // uses stable hashing methods such as String#hashCode(), Multiset#hashCode(), and
    // Integer#hashCode().

    if (symbolHashes.containsKey(sym)) {
      return symbolHashes.get(sym);
    }

    Multiset<Integer> hashes = HashMultiset.create();
    if (sym.members() != null) {
      for (Symbol member : sym.members().getSymbols()) {
        if (member.isPrivate()
            || (member instanceof MethodSymbol && ((MethodSymbol) member).isStaticOrInstanceInit())
            || ((member.flags_field & (Flags.BRIDGE | Flags.SYNTHETIC)) != 0)) {
          // Ignore initializers, private members, and synthetic members.  It's possible these do
          // not appear in the symbol's scope outside of its .java source compilation (i.e. they do
          // not appear in dependent compilations for Bazel's java rules).
          continue;
        }
        // We can't recursively get the result of hashSymbol(member) since the extractor removes all
        // .class files not directly used by a compilation meaning that member may not be complete.
        hashes.add(member.getSimpleName().toString().hashCode());
        hashes.add(member.kind.ordinal());
      }
    }

    hashes.add(sym.getQualifiedName().toString().hashCode());
    hashes.add(sym.getKind().ordinal());
    for (Modifier mod : sym.getModifiers()) {
      hashes.add(mod.ordinal());
    }

    int h = hashes.hashCode();
    symbolHashes.put(sym, h);
    return h;
  }

  @Nullable
  private VName lookupVName(@Nullable ClassSymbol cls) {
    if (cls == null) {
      return null;
    }
    VName clsVName = lookupVName(getDigest(cls.classfile));
    return clsVName != null ? clsVName : lookupVName(getDigest(cls.sourcefile));
  }

  @Nullable
  private static String getDigest(@Nullable JavaFileObject sourceFile) {
    if (sourceFile == null) {
      return null;
    }
    // This matches our {@link CustomFileObject#toUri()} logic
    return sourceFile.toUri().getHost();
  }

  private static boolean fromJDK(@Nullable Symbol sym) {
    if (sym == null || sym.enclClass() == null) {
      return false;
    }
    String cls = sym.enclClass().className();
    return SignatureGenerator.isArrayHelperClass(sym.enclClass())
        || cls.startsWith("java.")
        || cls.startsWith("javax.")
        || cls.startsWith("com.sun.")
        || cls.startsWith("sun.");
  }
}
