/*
 * Copyright 2018 The Kythe Authors. All rights reserved.
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

package com.google.devtools.kythe.analyzers.jvm;

import com.google.common.base.Joiner;
import com.google.common.base.Preconditions;
import com.google.common.base.Splitter;
import com.google.devtools.kythe.analyzers.base.CorpusPath;
import com.google.devtools.kythe.analyzers.base.EntrySet;
import com.google.devtools.kythe.analyzers.base.FactEmitter;
import com.google.devtools.kythe.analyzers.base.KytheEntrySets;
import com.google.devtools.kythe.analyzers.base.KytheEntrySets.NodeBuilder;
import com.google.devtools.kythe.analyzers.base.NodeKind;
import com.google.devtools.kythe.analyzers.jvm.JvmGraph.Type.MethodType;
import com.google.devtools.kythe.analyzers.jvm.JvmGraph.Type.ReferenceType;
import com.google.devtools.kythe.platform.shared.StatisticsCollector;
import com.google.devtools.kythe.proto.MarkedSource;
import com.google.devtools.kythe.proto.Storage.VName;
import java.util.ArrayList;
import java.util.List;
import java.util.Locale;
import java.util.Optional;

/** Kythe JVM language graph. */
public class JvmGraph {
  /** The language component for all JVM Kythe node {@link VName}s. */
  public static final String JVM_LANGUAGE = "jvm";

  /** The language component for all C symbol Kythe node {@link VName}s. */
  public static final String CSYMBOL_LANGUAGE = "csymbol";

  private final KytheEntrySets entrySets;

  /** Constructs a new {@link JvmGraph} that emits Kythe facts to the given {@link FactEmitter}. */
  public JvmGraph(StatisticsCollector statistics, FactEmitter emitter) {
    entrySets =
        new KytheEntrySets(
            statistics,
            emitter,
            VName.newBuilder().setLanguage(JVM_LANGUAGE).build(),
            new ArrayList<>());
  }

  /** Returns the underlying {@link KytheEntrySets} used to construct and emit the Kythe graph. */
  public KytheEntrySets getKytheEntrySets() {
    return entrySets;
  }

  /** Returns the {@link VName} corresponding to the given class/enum/interface type. */
  public static VName getReferenceVName(CorpusPath corpusPath, Type.ReferenceType referenceType) {
    return corpusPath
        .toVNameBuilder()
        .setSignature(referenceType.qualifiedName)
        .setLanguage(JVM_LANGUAGE)
        .build();
  }

  /** Returns the {@link VName} corresponding to the given method type. */
  public static VName getMethodVName(
      CorpusPath corpusPath,
      Type.ReferenceType parentClass,
      String name,
      Type.MethodType methodType) {
    return corpusPath
        .toVNameBuilder()
        .setSignature(methodSignature(parentClass, name, methodType))
        .setLanguage(JVM_LANGUAGE)
        .build();
  }

  /**
   * Returns the {@link VName} corresponding to the given native method type.
   *
   * <p>The jvm provides the option of using the name without the argument signature *if* the method
   * is not overloaded by another native method. This means that methods with no (native) overrides
   * can have two possible associated csymbols.
   */
  public static Optional<VName> getNativeMethodVName(
      CorpusPath corpusPath,
      Type.ReferenceType parentClass,
      String name,
      Type.MethodType methodType,
      boolean hasNativeOverload) {
    Optional<String> signature =
        nativeMethodSignature(parentClass, name, methodType, hasNativeOverload);
    if (signature.isEmpty()) {
      return Optional.empty();
    }
    return Optional.of(
        corpusPath
            .toVNameBuilder()
            .clearPath()
            .clearRoot()
            .setSignature(signature.get())
            .setLanguage(CSYMBOL_LANGUAGE)
            .build());
  }

  /**
   * Returns the {@link VName} corresponding to the given parameter of a method type.
   *
   * <p>Parameter indices are used because names are only optionally retained in class files and not
   * required by the spec.
   */
  public static VName getParameterVName(
      CorpusPath corpusPath,
      Type.ReferenceType parentClass,
      String methodName,
      Type.MethodType methodType,
      int parameterIndex) {
    return corpusPath
        .toVNameBuilder()
        .setSignature(parameterSignature(parentClass, methodName, methodType, parameterIndex))
        .setLanguage(JVM_LANGUAGE)
        .build();
  }

  /** Returns the {@link VName} corresponding to the given field type. */
  public static VName getFieldVName(
      CorpusPath corpusPath, Type.ReferenceType parentClass, String name) {
    return corpusPath
        .toVNameBuilder()
        .setSignature(parentClass.qualifiedName + "." + name)
        .setLanguage(JVM_LANGUAGE)
        .build();
  }

  private static String methodSignature(
      Type.ReferenceType parentClass, String methodName, Type.MethodType methodType) {
    return parentClass.qualifiedName + "." + methodName + methodType;
  }

  private static String jniMangle(String s) {
    StringBuilder sb = new StringBuilder();
    for (int i = 0; i < s.length(); ++i) {
      char c = s.charAt(i);
      if (c == '_') {
        sb.append("_1");
      } else if (c == '.' || c == '/') {
        sb.append("_");
      } else if (c == ';') {
        sb.append("_2");
      } else if (c == '[') {
        sb.append("_3");
      } else if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
        sb.append(c);
      } else {
        sb.append("_0");
        sb.append(String.format("%04x", (int) c).toLowerCase(Locale.ENGLISH));
      }
    }
    return sb.toString();
  }

  private static Optional<String> getNativeMethodParameters(Type.MethodType methodType) {
    String type = methodType.toString();
    if (!type.startsWith("(")) {
      return Optional.empty();
    }
    int cparen = type.indexOf(")", 1);
    if (cparen == -1) {
      return Optional.empty();
    }
    return Optional.of(type.substring(1, cparen));
  }

  private static Optional<String> nativeMethodSignature(
      Type.ReferenceType parentClass,
      String methodName,
      Type.MethodType methodType,
      boolean hasNativeOverload) {
    StringBuilder sb = new StringBuilder();
    sb.append("Java_");
    sb.append(jniMangle(parentClass.qualifiedName));
    sb.append("_");
    sb.append(jniMangle(methodName));
    if (hasNativeOverload) {
      Optional<String> parameters = getNativeMethodParameters(methodType);
      if (!parameters.isPresent()) {
        return Optional.empty();
      }
      sb.append("__");
      sb.append(jniMangle(parameters.get()));
    }
    return Optional.of(sb.toString());
  }

  private static String parameterSignature(
      Type.ReferenceType parentClass,
      String methodName,
      Type.MethodType methodType,
      int parameterIndex) {
    return methodSignature(parentClass, methodName, methodType) + ".param" + parameterIndex;
  }

  /** Emits and returns a Kythe {@code record} node for a JVM class. */
  public VName emitClassNode(CorpusPath corpusPath, Type.ReferenceType refType) {
    return emitNode(
        NodeKind.RECORD_CLASS, getReferenceVName(corpusPath, refType), markedSource(refType));
  }

  /** Emits and returns a Kythe {@code interface} node for a JVM interface. */
  public VName emitInterfaceNode(CorpusPath corpusPath, Type.ReferenceType refType) {
    return emitNode(
        NodeKind.INTERFACE, getReferenceVName(corpusPath, refType), markedSource(refType));
  }

  /** Emits and returns a Kythe {@code sum} node for a JVM enum class. */
  public VName emitEnumNode(CorpusPath corpusPath, Type.ReferenceType refType) {
    return emitNode(
        NodeKind.SUM_ENUM_CLASS, getReferenceVName(corpusPath, refType), markedSource(refType));
  }

  /** Emits and returns a Kythe {@code variable} node for a JVM field. */
  public VName emitFieldNode(CorpusPath corpusPath, Type.ReferenceType parentClass, String name) {
    return emitNode(NodeKind.VARIABLE_FIELD, getFieldVName(corpusPath, parentClass, name));
  }

  /** Emits and returns a Kythe {@code function} node for a JVM method. */
  public VName emitMethodNode(
      CorpusPath corpusPath,
      Type.ReferenceType parentClass,
      String methodName,
      Type.MethodType type) {
    return emitNode(
        methodName.equals("<init>") ? NodeKind.FUNCTION_CONSTRUCTOR : NodeKind.FUNCTION,
        getMethodVName(corpusPath, parentClass, methodName, type));
  }

  /** Emits and returns a Kythe {@code name} node for a native JVM method. */
  public Optional<VName> emitNativeNode(
      CorpusPath corpusPath,
      Type.ReferenceType parentClass,
      String methodName,
      Type.MethodType type,
      boolean hasNativeOverload) {
    Optional<VName> vname =
        getNativeMethodVName(corpusPath, parentClass, methodName, type, hasNativeOverload);
    if (!vname.isPresent()) {
      return Optional.empty();
    }
    return Optional.of(emitNode(NodeKind.NAME, vname.get()));
  }

  /**
   * Emits and returns a Kythe {@code variable/local/parameter} node for a JVM parameter to a
   * method.
   *
   * @see #getParameterVName(Type.ReferenceType, String, Type.MethodType, int)
   */
  public VName emitParameterNode(
      CorpusPath corpusPath,
      Type.ReferenceType parentClass,
      String methodName,
      Type.MethodType methodType,
      int parameterIndex) {
    return emitNode(
        NodeKind.VARIABLE_PARAMETER,
        getParameterVName(corpusPath, parentClass, methodName, methodType, parameterIndex));
  }

  private VName emitNode(NodeKind nodeKind, VName name) {
    return emitNode(nodeKind, name, null);
  }

  private VName emitNode(NodeKind nodeKind, VName name, MarkedSource markedSource) {
    NodeBuilder builder = entrySets.newNode(nodeKind, name);
    if (markedSource != null) {
      builder.setProperty("code", markedSource);
    }
    EntrySet es = builder.build();
    es.emit(entrySets.getEmitter());
    return es.getVName();
  }

  /**
   * JVM {@link Type} descriptor including the {@code void} method return type.
   *
   * @see JvmGraph.Type
   */
  public static class VoidableType {
    private static final VoidableType VOID =
        new VoidableType() {
          @Override
          public String toString() {
            return "V";
          }
        };

    /** Returns the JVM {@code void} type descriptor. */
    public static VoidableType voidType() {
      return VOID;
    }

    @Override
    public boolean equals(Object o) {
      if (this == o) {
        return true;
      } else if (!(o instanceof Type)) {
        return false;
      }
      return this.toString().equals(o.toString());
    }

    @Override
    public int hashCode() {
      return toString().hashCode();
    }
  }

  /**
   * JVM type descriptor.
   *
   * <p>See https://docs.oracle.com/javase/specs/jvms/se9/html/jvms-4.html#jvms-4.3
   */
  public static class Type extends VoidableType {
    private static final PrimitiveType BOOL = new PrimitiveType("Z");
    private static final PrimitiveType BYTE = new PrimitiveType("B");
    private static final PrimitiveType CHAR = new PrimitiveType("C");
    private static final PrimitiveType SHORT = new PrimitiveType("S");
    private static final PrimitiveType INT = new PrimitiveType("I");
    private static final PrimitiveType LONG = new PrimitiveType("J");
    private static final PrimitiveType FLOAT = new PrimitiveType("F");
    private static final PrimitiveType DOUBLE = new PrimitiveType("D");
    private static final Type OBJECT = referenceType("java.lang.Object");

    protected final String signature;

    private Type(String signature) {
      this.signature = signature;
    }

    /** JVM primitive type descriptors. */
    public static final class PrimitiveType extends Type {
      private PrimitiveType(String signature) {
        super(signature);
      }
    }

    /** JVM class/enum/interface type descriptors. */
    public static final class ReferenceType extends Type {
      final String qualifiedName;

      private ReferenceType(String qualifiedName) {
        super("L" + qualifiedName.replace(".", "/") + ";");
        this.qualifiedName = qualifiedName.replace("/", ".");
      }
    }

    /** JVM method type descriptors. */
    public static final class MethodType extends Type {
      private MethodType(String signature) {
        super(signature);
      }
    }

    /** JVM array type descriptors. */
    public static final class ArrayType extends Type {
      private ArrayType(String signature) {
        super(signature);
      }
    }

    /** Returns the JVM {@code boolean} type descriptor. */
    public static PrimitiveType booleanType() {
      return BOOL;
    }

    /** Returns the JVM {@code byte} type descriptor. */
    public static PrimitiveType byteType() {
      return BYTE;
    }

    /** Returns the JVM {@code char} type descriptor. */
    public static PrimitiveType charType() {
      return CHAR;
    }

    /** Returns the JVM {@code int} type descriptor. */
    public static PrimitiveType intType() {
      return INT;
    }

    /** Returns the JVM {@code long} type descriptor. */
    public static PrimitiveType longType() {
      return LONG;
    }

    /** Returns the JVM {@code short} type descriptor. */
    public static PrimitiveType shortType() {
      return SHORT;
    }

    /** Returns the JVM {@code float} type descriptor. */
    public static PrimitiveType floatType() {
      return FLOAT;
    }

    /** Returns the JVM {@code double} type descriptor. */
    public static PrimitiveType doubleType() {
      return DOUBLE;
    }

    /** Returns the type for an Object. */
    public static Type objectType() {
      return OBJECT;
    }

    /** Returns a new JVM class/enum/interface type descriptor. */
    public static ReferenceType referenceType(String qualifiedName) {
      Preconditions.checkNotNull(qualifiedName);
      // Normalized qualified name (e.g. java.util.Map$Entry<K, V> -> java.util.Map$Entry)
      String normalized = qualifiedName.replaceAll("<[^\\]]+>", "");
      return new ReferenceType(normalized);
    }

    /** Returns a new JVM method type descriptor. */
    public static MethodType methodType(List<Type> argTypes, VoidableType retType) {
      Preconditions.checkNotNull(argTypes, "method argument types list must be non-null");
      Preconditions.checkNotNull(retType, "method return type must be non-null");
      return new MethodType("(" + Joiner.on("").join(argTypes) + ")" + retType);
    }

    /** Returns a new JVM array type descriptor. */
    public static ArrayType arrayType(Type elementType) {
      Preconditions.checkNotNull(elementType, "array element type must be non-null");
      return new ArrayType("[" + elementType);
    }

    @Override
    public String toString() {
      return signature;
    }

    static MethodType rawMethodType(String descriptor) {
      return new MethodType(descriptor);
    }

    static Type rawType(String signature) {
      return new Type(signature);
    }
  }

  private static MarkedSource markedSource(Type.ReferenceType referenceType) {
    List<String> parts = Splitter.on('.').splitToList(referenceType.qualifiedName);
    MarkedSource id =
        MarkedSource.newBuilder()
            .setKind(MarkedSource.Kind.IDENTIFIER)
            .setPreText(parts.get(parts.size() - 1))
            .build();
    if (parts.size() == 1) {
      return id;
    }
    MarkedSource.Builder ctx =
        MarkedSource.newBuilder()
            .setKind(MarkedSource.Kind.CONTEXT)
            .setAddFinalListToken(true)
            .setPostChildText(".");
    for (int i = 0; i < parts.size() - 1; i++) {
      ctx.addChildBuilder().setKind(MarkedSource.Kind.IDENTIFIER).setPreText(parts.get(i));
    }
    return MarkedSource.newBuilder().addChild(ctx.build()).addChild(id).build();
  }
}
