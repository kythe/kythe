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

import com.google.common.collect.HashMultiset;
import com.google.common.collect.Multiset;
import com.google.common.flogger.FluentLogger;
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
import com.google.devtools.kythe.proto.MarkedSource;
import com.google.devtools.kythe.proto.Storage.VName;
import com.google.devtools.kythe.util.Span;
import com.sun.tools.javac.code.Flags;
import com.sun.tools.javac.code.Symbol;
import com.sun.tools.javac.code.Symbol.ClassSymbol;
import com.sun.tools.javac.code.Symbol.MethodSymbol;
import com.sun.tools.javac.code.Symbol.PackageSymbol;
import com.sun.tools.javac.tree.JCTree;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.Set;
import javax.lang.model.element.ElementKind;
import javax.lang.model.element.Name;
import javax.tools.JavaFileObject;
import org.checkerframework.checker.nullness.qual.Nullable;

/** Specialization of {@link KytheEntrySets} for Java. */
public class JavaEntrySets extends KytheEntrySets {
  private static final FluentLogger logger = FluentLogger.forEnclosingClass();

  private static final String ARRAY_BUILTIN_CLASS = "Array";

  private final Map<Symbol, VName> symbolNodes = new HashMap<>();
  private final Set<Symbol> symbolsDocumented = new HashSet<>();
  private final Map<Symbol, Integer> symbolHashes = new HashMap<>();
  private final JavaIndexerConfig config;
  private final Map<String, Integer> sourceToWildcardCounter = new HashMap<>();

  public JavaEntrySets(
      StatisticsCollector statistics,
      FactEmitter emitter,
      VName compilationVName,
      List<FileInput> requiredInputs,
      JavaIndexerConfig config) {
    super(statistics, emitter, compilationVName, requiredInputs);
    this.config = config;
    setUseCompilationCorpusAsDefault(config.getUseCompilationCorpusAsDefault());
  }

  Map<Symbol, VName> getSymbolNodes() {
    return Collections.unmodifiableMap(symbolNodes);
  }

  /**
   * Returns a node for the given {@link Symbol} and its signature. A new node is created and
   * emitted if necessary. If non-null, msBuilder will be used to generate a signature.
   */
  @Deprecated
  public VName getNode(
      final SignatureGenerator signatureGenerator,
      Symbol sym,
      String signature,
      // TODO(schroederc): separate MarkedSource generation from JavaEntrySets
      MarkedSource.@Nullable Builder msBuilder,
      @Nullable Iterable<MarkedSource> postChildren) {
    return getNode(
        signatureGenerator,
        sym,
        signature,
        MarkedSources.construct(
            signatureGenerator,
            sym,
            msBuilder,
            postChildren,
            // TODO(schroederc): do not construct entire node to just determine its VName
            s ->
                signatureGenerator
                    .getSignature(s)
                    .map(sig -> getNode(signatureGenerator, s, sig, null, null))));
  }

  /**
   * Returns a node for the given {@link Symbol} and its signature. A new node is created and
   * emitted if necessary.
   */
  public VName getNode(
      SignatureGenerator signatureGenerator,
      Symbol sym,
      String signature,
      MarkedSource markedSource) {
    EntrySet node;
    if (symbolNodes.containsKey(sym) && (markedSource == null || symbolsDocumented.contains(sym))) {
      return symbolNodes.get(sym);
    }

    ClassSymbol enclClass = sym.enclClass();
    if (enclClass == null && sym.asType().isPrimitive()) {
      VName v = newBuiltinAndEmit(sym.asType().toString()).getVName();
      symbolNodes.put(sym, v);
      return v;
    }

    // The vname corpus for jdk entities is determined by:
    // * --override_jdk_corpus if present
    // * the compilation's corpus if --use_compilation_corpus_as_default is set
    // * "jdk" if the above two flags are unset
    VName v = lookupVName(enclClass);
    if (fromJDK(sym)) {
      String corpus = "jdk";
      if (config.getOverrideJdkCorpus() != null) {
        corpus = config.getOverrideJdkCorpus();
      } else if (getUseCompilationCorpusAsDefault()) {
        corpus = defaultCorpusPath().getCorpus();
      }
      v = VName.newBuilder().setCorpus(corpus).build();
    }

    if (v == null && sym.owner != null && sym.owner.asType().isPrimitive()) {
      // Handle primitive .class reference
      v = newBuiltinAndEmit(sym.asType().toString()).getVName();
    }

    if (v == null) {
      getStatisticsCollector().incrementCounter("unextracted-input-file");
      String msg =
          String.format(
              "Couldn't generate vname for symbol %s.  Input file for enclosing class %s not seen"
                  + " during extraction.",
              sym, enclClass);
      if (config.getVerboseLogging()) {
        logger.atWarning().log("%s", msg);
      }
      Diagnostic.Builder d = Diagnostic.newBuilder().setMessage(msg);
      return emitDiagnostic(d.build()).getVName();
    } else {
      if (config.getIgnoreVNamePaths()) {
        v = v.toBuilder().setPath(enclClass != null ? enclClass.toString() : "").build();
      }
      if (config.getIgnoreVNameRoots()) {
        v = v.toBuilder().clearRoot().build();
      }

      NodeKind kind = elementNodeKind(sym.getKind());
      NodeBuilder builder = kind != null ? newNode(kind) : newNode(sym.getKind().toString());
      builder.setCorpusPath(CorpusPath.fromVName(v));
      if (markedSource != null) {
        builder.setProperty("code", markedSource);
        symbolsDocumented.add(sym);
      }

      if (signatureGenerator.getUseJvmSignatures()) {
        builder.setSignature(signature);
      } else {
        builder.addSignatureSalt(signature).addSignatureSalt("" + hashSymbol(sym));
      }
      node = builder.build();
      node.emit(getEmitter());
    }

    symbolNodes.put(sym, node.getVName());
    return node.getVName();
  }

  /** Emits and returns a new {@link EntrySet} representing a lambda object. */
  public EntrySet newLambdaAndEmit(Positions filePositions, JCTree.JCLambda lambda) {
    VName fileVName = getFileVName(getDigest(filePositions.getSourceFile()));
    return emitAndReturn(
        newNode(NodeKind.FUNCTION)
            .setCorpusPath(CorpusPath.fromVName(fileVName))
            .addSignatureSalt("" + filePositions.getSpan(lambda)));
  }

  /** Emits and returns a new {@link EntrySet} representing a static class initializer. */
  public EntrySet newClassInitAndEmit(String classSignature, VName classNode) {
    return emitAndReturn(
        newNode(NodeKind.FUNCTION)
            .setCorpusPath(CorpusPath.fromVName(classNode))
            .addSignatureSalt(classNode)
            .addSignatureSalt("clinit")
            .setProperty(
                "code",
                MarkedSource.newBuilder()
                    .addChild(
                        MarkedSource.newBuilder()
                            .setKind(MarkedSource.Kind.CONTEXT)
                            .setPreText(classSignature))
                    .addChild(
                        MarkedSource.newBuilder()
                            .setPreText("<clinit>")
                            .setKind(MarkedSource.Kind.IDENTIFIER))
                    .setPostChildText(".")
                    .build())
            .build());
  }

  /**
   * Emits and returns a new {@link EntrySet} representing a Java comment with an optional subkind.
   */
  public EntrySet newDocAndEmit(
      Optional<String> subkind, Positions filePositions, String text, Iterable<VName> params) {
    VName fileVName = getFileVName(getDigest(filePositions.getSourceFile()));
    NodeBuilder builder =
        newNode(NodeKind.DOC.getKind(), subkind)
            .setCorpusPath(CorpusPath.fromVName(fileVName))
            .setProperty("text", text.getBytes(UTF_8))
            .addSignatureSalt(text);
    params.forEach(builder::addSignatureSalt);
    EntrySet node = emitAndReturn(builder);
    emitOrdinalEdges(node.getVName(), EdgeKind.PARAM, params);
    return node;
  }

  /** Returns the {@link VName} for the given file. */
  public VName getFileVName(JavaFileObject sourceFile) {
    return getFileVName(getDigest(sourceFile));
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
                .setCorpusPath(defaultCorpusPath())
                .setProperty(
                    "code",
                    MarkedSource.newBuilder()
                        .addChild(
                            MarkedSource.newBuilder()
                                .setPostChildText(" ")
                                .setAddFinalListToken(true)
                                .addChild(
                                    MarkedSource.newBuilder()
                                        .setPreText("package")
                                        .setKind(MarkedSource.Kind.MODIFIER)
                                        .build())
                                .build())
                        .addChild(
                            MarkedSource.newBuilder()
                                .setPreText(name)
                                .setKind(MarkedSource.Kind.IDENTIFIER)
                                .build())
                        .build())
                .build());
    return node;
  }

  /** Emits and returns a new {@link EntrySet} for the given wildcard. */
  public EntrySet newWildcardNodeAndEmit(JCTree.JCWildcard wild, String sourcePath) {
    int counter = sourceToWildcardCounter.getOrDefault(sourcePath, 0);
    sourceToWildcardCounter.put(sourcePath, counter + 1);
    return emitAndReturn(
        newNode(NodeKind.TVAR)
            .addSignatureSalt(sourcePath + counter)
            .setCorpusPath(defaultCorpusPath()));
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
  public @Nullable EntrySet newAnchorAndEmit(
      Positions filePositions, Name name, int startOffset, Span snippet) {
    Span span = filePositions.findIdentifier(name, startOffset);
    return span == null
        ? null
        : newAnchorAndEmit(getFileVName(getDigest(filePositions.getSourceFile())), span, snippet);
  }

  /** Emits and returns a DIAGNOSTIC node attached to the given file. */
  public EntrySet emitDiagnostic(Positions filePositions, Diagnostic d) {
    return emitDiagnostic(filePositions.getSourceFile(), d);
  }

  /** Emits and returns a DIAGNOSTIC node attached to the given file. */
  public EntrySet emitDiagnostic(JavaFileObject file, Diagnostic d) {
    return emitDiagnostic(getFileVName(getDigest(file)), d);
  }

  /** Returns the equivalent {@link NodeKind} for the given {@link ElementKind}. */
  private @Nullable NodeKind elementNodeKind(ElementKind kind) {
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
        return NodeKind.TVAR;
      default:
        // TODO(#1845): handle all cases, make this exceptional, and remove all null checks
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
    // XXX: ignore Symbol modifiers since they can be inconsistent between definitions/references.

    int h = hashes.hashCode();
    symbolHashes.put(sym, h);
    return h;
  }

  /** Returns the JVM {@link CorpusPath} for the given {@link Symbol}. */
  public CorpusPath jvmCorpusPath(Symbol sym) {
    if (getUseCompilationCorpusAsDefault()) {
      return defaultCorpusPath();
    }
    return new CorpusPath(
        Optional.ofNullable(lookupVName(sym.enclClass())).map(VName::getCorpus).orElse(""), "", "");
  }

  private @Nullable VName lookupVName(@Nullable ClassSymbol cls) {
    if (cls == null) {
      return null;
    } else if (cls.getQualifiedName().contentEquals(ARRAY_BUILTIN_CLASS)) {
      return newBuiltinAndEmit("array").getVName();
    }
    VName clsVName = lookupVName(getDigest(cls.classfile));
    return clsVName != null ? clsVName : lookupVName(getDigest(cls.sourcefile));
  }

  private static @Nullable String getDigest(@Nullable JavaFileObject sourceFile) {
    if (sourceFile == null) {
      return null;
    }
    // This matches our {@link CustomFileObject#toUri()} logic
    return sourceFile.toUri().getHost();
  }

  static boolean fromJDK(@Nullable Symbol sym) {
    if (sym == null) {
      return false;
    }
    ClassSymbol enclClass = sym.enclClass();
    if (enclClass == null
        || enclClass.classfile == null
        || enclClass.classfile.getKind() == JavaFileObject.Kind.SOURCE) {
      return false;
    }
    String cls = enclClass.className();
    // For performance, first check common package prefixes, then delegate
    // to the slower loadedByJDKClassLoader() method.
    return SignatureGenerator.isArrayHelperClass(enclClass)
        || cls.startsWith("java.")
        || cls.startsWith("javax.")
        || cls.startsWith("com.sun.")
        || cls.startsWith("sun.")
        || loadedByJDKClassLoader(cls);
  }

  /**
   * Detects whether the class with the given name was loaded by the ClassLoader used to load JDK
   * classes.
   *
   * <p>Although this is not required by the JVM spec, standard JVM implementations use separate
   * classloaders for loading JDK classes vs. user classes.
   *
   * <p>Therefore, although not completely bulletproof, this is a good heuristic in practice for
   * detecting whether a class is supplied by the JDK.
   */
  private static boolean loadedByJDKClassLoader(String cls) {
    try {
      return Class.forName(cls).getClassLoader() == String.class.getClassLoader();
    } catch (ClassNotFoundException | SecurityException e) {
      return false;
    }
  }
}
