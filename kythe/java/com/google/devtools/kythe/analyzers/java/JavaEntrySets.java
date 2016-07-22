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
import com.google.common.collect.HashMultiset;
import com.google.common.collect.Multiset;
import com.google.devtools.kythe.analyzers.base.CorpusPath;
import com.google.devtools.kythe.analyzers.base.EdgeKind;
import com.google.devtools.kythe.analyzers.base.EntrySet;
import com.google.devtools.kythe.analyzers.base.FactEmitter;
import com.google.devtools.kythe.analyzers.base.KytheEntrySets;
import com.google.devtools.kythe.analyzers.base.NodeKind;
import com.google.devtools.kythe.analyzers.java.SourceText.Positions;
import com.google.devtools.kythe.platform.shared.StatisticsCollector;
import com.google.devtools.kythe.proto.Analysis.CompilationUnit.FileInput;
import com.google.devtools.kythe.proto.Storage.VName;
import com.google.devtools.kythe.util.Span;
import com.sun.tools.javac.code.Symbol;
import com.sun.tools.javac.code.Symbol.ClassSymbol;
import com.sun.tools.javac.code.Symbol.MethodSymbol;
import com.sun.tools.javac.code.Symbol.PackageSymbol;
import com.sun.tools.javac.tree.JCTree;
import java.io.UnsupportedEncodingException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;
import javax.lang.model.element.ElementKind;
import javax.lang.model.element.Modifier;
import javax.lang.model.element.Name;
import javax.tools.JavaFileObject;

/** Specialization of {@link KytheEntrySets} for Java. */
public class JavaEntrySets extends KytheEntrySets {
  private final Map<Symbol, EntrySet> symbolNodes = new HashMap<>();
  private final Map<Symbol, Integer> symbolHashes = new HashMap<>();
  private final Map<Symbol, Set<String>> symbolSigs = new HashMap<Symbol, Set<String>>();
  private final boolean ignoreVNamePaths;

  public JavaEntrySets(
      StatisticsCollector statistics,
      FactEmitter emitter,
      VName compilationVName,
      List<FileInput> requiredInputs,
      boolean ignoreVNamePaths) {
    super(statistics, emitter, compilationVName, requiredInputs);
    this.ignoreVNamePaths = ignoreVNamePaths;
  }

  /**
   * Returns a node for the given {@link Symbol} and its signature. A new node is created and
   * emitted if necessary.
   */
  public EntrySet getNode(Symbol sym, String signature) {
    checkSignature(sym, signature);

    EntrySet node;
    if ((node = symbolNodes.get(sym)) != null) {
      return node;
    }

    ClassSymbol enclClass = sym.enclClass();
    VName v = lookupVName(enclClass);
    if (v == null && fromJDK(sym)) {
      v = VName.newBuilder().setCorpus("jdk").build();
    }

    if (v == null) {
      node = getName(signature);
      // NAME node was already be emitted
    } else {
      if (ignoreVNamePaths) {
        v = v.toBuilder().setPath(enclClass != null ? enclClass.toString() : "").build();
      }

      String format;
      switch (sym.getKind()) {
        case CONSTRUCTOR:
          format =
              String.format(
                  "%%^.%s", enclClass != null ? enclClass.getSimpleName() : sym.getSimpleName());
          break;
        case TYPE_PARAMETER:
          format = String.format("%%^.<%s>", sym.getSimpleName());
          break;
        case METHOD:
          int numParams = ((MethodSymbol) sym).params().size();
          List<String> params = new ArrayList<>(numParams);
          for (int i = 1; i <= numParams; i++) {
            params.add("%" + (i + 1) + "`");
          }
          format =
              String.format("%%1` %%^.%s(%s)", sym.getSimpleName(), Joiner.on(",").join(params));
          break;
        default:
          format = String.format("%%^.%s", sym.getSimpleName());
          break;
      }

      NodeKind kind = elementNodeKind(sym.getKind());
      NodeBuilder builder = kind != null ? newNode(kind) : newNode(sym.getKind().toString());
      node =
          builder
              .setCorpusPath(CorpusPath.fromVName(v))
              .addSignatureSalt(signature)
              .addSignatureSalt("" + hashSymbol(sym))
              .setProperty("format", format)
              .build();
      emitName(node, signature);
      node.emit(getEmitter());
    }

    symbolNodes.put(sym, node);
    return node;
  }

  public EntrySet getDoc(Positions filePositions, String text, Iterable<EntrySet> params) {
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
    for (EntrySet param : params) {
      builder.addSignatureSalt(param.getVName());
    }
    EntrySet node = emitAndReturn(builder);
    emitOrdinalEdges(node, EdgeKind.PARAM, params);
    return node;
  }

  /** Emits and returns a new {@link EntrySet} representing the Java file. */
  public EntrySet getFileNode(Positions file) {
    return getFileNode(getDigest(file.getSourceFile()), file.getData(), file.getEncoding());
  }

  /** Emits and returns a new {@link EntrySet} representing a Java package. */
  public EntrySet getPackageNode(PackageSymbol sym) {
    return getPackageNode(sym.getQualifiedName().toString());
  }

  /** Emits and returns a new {@link EntrySet} representing a Java package. */
  public EntrySet getPackageNode(String name) {
    EntrySet node =
        emitAndReturn(newNode(NodeKind.PACKAGE).addSignatureSalt(name).setProperty("format", name));
    emitName(node, name);
    return node;
  }

  /** Emits and returns a new {@link EntrySet} for the given wildcard. */
  public EntrySet getWildcardNode(JCTree.JCWildcard wild) {
    return emitAndReturn(newNode(NodeKind.ABS_VAR).addSignatureSalt("" + wild.hashCode()));
  }

  /** Returns and emits a Java anchor for the given {@link JCTree}. */
  public EntrySet getAnchor(Positions filePositions, JCTree tree) {
    return getAnchor(filePositions, filePositions.getSpan(tree));
  }

  /** Returns and emits a Java anchor for the given offset span. */
  public EntrySet getAnchor(Positions filePositions, Span loc) {
    return getAnchor(filePositions, loc, null);
  }

  /** Returns and emits a Java anchor for the given offset span. */
  public EntrySet getAnchor(Positions filePositions, Span loc, Span snippet) {
    return getAnchor(getFileVName(getDigest(filePositions.getSourceFile())), loc, snippet);
  }

  /** Returns and emits a Java anchor for the given identifier. */
  public EntrySet getAnchor(Positions filePositions, Name name, int startOffset, Span snippet) {
    Span span = filePositions.findIdentifier(name, startOffset);
    return span == null
        ? null
        : getAnchor(getFileVName(getDigest(filePositions.getSourceFile())), span, snippet);
  }

  /** Returns the equivalent {@link NodeKind} for the given {@link ElementKind}. */
  public static NodeKind elementNodeKind(ElementKind kind) {
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
        // TODO(schroederc): handle all cases, make this exceptional, and remove all null checks
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
            || member instanceof MethodSymbol && ((MethodSymbol) member).isStaticOrInstanceInit()) {
          // Ignore initializers and private members.  It's possible these do not appear in the
          // symbol's scope outside of its .java source compilation (i.e. they do not appear in
          // dependent compilations for Bazel's java rules).
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

  private VName lookupVName(ClassSymbol cls) {
    if (cls == null) {
      return null;
    }
    VName clsVName = lookupVName(getDigest(cls.classfile));
    return clsVName != null ? clsVName : lookupVName(getDigest(cls.sourcefile));
  }

  private static String getDigest(JavaFileObject sourceFile) {
    if (sourceFile == null) {
      return null;
    }
    // This matches our {@link CustomFileObject#toUri()} logic
    return sourceFile.toUri().getHost();
  }

  /** Ensures that a particular {@link Symbol} is only associated with a single signature. */
  private void checkSignature(Symbol sym, String signature) {
    // TODO(schroederc): remove this check in production releases
    if (!symbolSigs.containsKey(sym)) {
      symbolSigs.put(sym, new HashSet<String>());
    }
    Set<String> signatures = symbolSigs.get(sym);
    signatures.add(signature);
    if (signatures.size() > 1) {
      throw new IllegalStateException("Multiple signatures found for " + sym + ": " + signatures);
    }
  }

  private static boolean fromJDK(Symbol sym) {
    if (sym == null || sym.enclClass() == null) {
      return false;
    }
    String cls = sym.enclClass().className();
    return cls.startsWith("java.")
        || cls.startsWith("javax.")
        || cls.startsWith("com.sun.")
        || cls.startsWith("sun.");
  }

  /**
   * Returns and emits a placeholder node meant to be <b>soon</b> replaced by a Kythe
   * schema-compliant node.
   */
  @Deprecated
  EntrySet todoNode(String sourceName, JCTree tree, String message) {
    return emitAndReturn(
        newNode("TODO")
            .addSignatureSalt("" + System.nanoTime()) // Ensure unique TODOs
            .setProperty("todo", message)
            .setProperty("sourcename", sourceName)
            .setProperty("jctree/class", tree.getClass().toString())
            .setProperty("jctree/tag", "" + tree.getTag())
            .setProperty("jctree/type", "" + tree.type)
            .setProperty("jctree/pos", "" + tree.pos)
            .setProperty("jctree/start", "" + tree.getStartPosition()));
  }
}
