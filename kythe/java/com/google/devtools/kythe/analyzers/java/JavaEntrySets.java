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

import com.google.devtools.kythe.analyzers.base.CorpusPath;
import com.google.devtools.kythe.analyzers.base.EdgeKind;
import com.google.devtools.kythe.analyzers.base.EntrySet;
import com.google.devtools.kythe.analyzers.base.FactEmitter;
import com.google.devtools.kythe.analyzers.base.KytheEntrySets;
import com.google.devtools.kythe.analyzers.base.NodeKind;
import com.google.devtools.kythe.proto.Analysis.CompilationUnit.FileInput;
import com.google.devtools.kythe.proto.Storage.VName;
import com.google.devtools.kythe.util.Span;

import com.sun.tools.javac.code.Symbol;
import com.sun.tools.javac.code.Symbol.ClassSymbol;
import com.sun.tools.javac.code.Symbol.PackageSymbol;
import com.sun.tools.javac.tree.JCTree;
import com.sun.tools.javac.util.Name;

import java.util.HashMap;
import java.util.List;
import java.util.Map;

import javax.lang.model.element.ElementKind;
import javax.tools.JavaFileObject;

/** Specialization of {@link KytheEntrySets} for Java. */
public class JavaEntrySets extends KytheEntrySets {
  private final Map<Symbol, EntrySet> symbolNodes = new HashMap<>();

  public JavaEntrySets(FactEmitter emitter, VName compilationVName,
      List<FileInput> requiredInputs) {
    super(emitter, compilationVName, requiredInputs);
  }

  /** Emits a NAME node and its associated edge to the given {@code node}. */
  public void emitName(EntrySet node, String name) {
    emitEdge(node, EdgeKind.NAMED, getName(name));
  }

  /**
   * Returns a node for the given {@link Symbol} and its signature. A new node is created and
   * emitted if necessary.
   */
  public EntrySet getNode(Symbol sym, String signature) {
    EntrySet node;
    if ((node = symbolNodes.get(sym)) != null) {
      return node;
    }

    VName v = lookupVName(sym.enclClass());
    if (v == null) {
      node = getName(signature);
      // NAME node was already be emitted
    } else {
      NodeKind kind = elementNodeKind(sym.getKind());
      NodeBuilder builder = kind != null
          ? newNode(kind)
          : newNode(sym.getKind().toString());
      node = builder
          .setCorpusPath(CorpusPath.fromVName(v))
          .addSignatureSalt(signature)
          .setProperty("identifier", sym.getSimpleName().toString())
          .build();
      emitName(node, signature);
      node.emit(getEmitter());
    }

    symbolNodes.put(sym, node);
    return node;
  }

  /** Emits and returns a new {@link EntrySet} representing the Java file. */
  public EntrySet getFileNode(FilePositions file) {
    return getFileNode(
        getDigest(file.getSourceFile()), file.getData(), file.getEncoding().toString());
  }

  /** Emits and returns a new {@link EntrySet} representing a Java package. */
  public EntrySet getPackageNode(PackageSymbol sym) {
    String name = sym.getQualifiedName().toString();
    EntrySet node = emitAndReturn(newNode(NodeKind.PACKAGE).addSignatureSalt(name));
    emitName(node, name);
    return node;
  }

  /** Returns and emits a Java anchor for the given {@link JCTree}. */
  public EntrySet getAnchor(FilePositions filePositions, JCTree tree) {
    return getAnchor(filePositions, filePositions.getStart(tree), filePositions.getEnd(tree));
  }

  /** Returns and emits a Java anchor for the given offset span. */
  public EntrySet getAnchor(FilePositions filePositions, int start, int end) {
    return getAnchor(lookupVName(getDigest(filePositions.getSourceFile())), start, end);
  }

  /** Returns and emits a Java anchor for the given identifier. */
  public EntrySet getAnchor(FilePositions filePositions, Name name, int startOffset) {
    Span span = filePositions.findIdentifier(name, startOffset);
    return span == null
        ? null
        : getAnchor(lookupVName(getDigest(filePositions.getSourceFile())),
            span.getStart(), span.getEnd());
  }

  /** Returns the equivalent {@link NodeKind} for the given {@link ElementKind}. */
  public static NodeKind elementNodeKind(ElementKind kind) {
    switch (kind) {
      case CLASS: return NodeKind.RECORD_CLASS;
      case ENUM: return NodeKind.SUM_ENUM_CLASS;
      case PARAMETER: case LOCAL_VARIABLE: case FIELD:
        return NodeKind.VARIABLE;
      case METHOD: return NodeKind.FUNCTION;
      default:
        // TODO(schroederc): handle all cases, make this exceptional, and remove all null checks
        return null;
    }
  }

  private VName lookupVName(ClassSymbol cls) {
    if (cls == null) {
      return null;
    }
    VName clsVName = lookupVName(getDigest(cls.classfile));
    return clsVName != null
        ? clsVName
        : lookupVName(getDigest(cls.sourcefile));
  }

  private static String getDigest(JavaFileObject sourceFile) {
    if (sourceFile == null) {
      return null;
    }
    // This matches our {@link CustomFileObject#toUri()} logic
    return sourceFile.toUri().getHost();
  }

  /**
   * Returns and emits a placeholder node meant to be <b>soon</b> replaced by a Kythe
   * schema-compliant node.
   */
  @Deprecated
  EntrySet todoNode(String message) {
    return emitAndReturn(newNode("TODO")
        .addSignatureSalt("" + System.nanoTime()) // Ensure unique TODOs
        .setProperty("todo", message));
  }
}
