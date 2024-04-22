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

import com.google.devtools.kythe.analyzers.base.CorpusPath;
import com.google.devtools.kythe.analyzers.base.EdgeKind;
import com.google.devtools.kythe.analyzers.base.FactEmitter;
import com.google.devtools.kythe.analyzers.base.KytheEntrySets;
import com.google.devtools.kythe.platform.shared.StatisticsCollector;
import com.google.devtools.kythe.proto.Storage.VName;
import java.io.IOException;
import java.io.InputStream;
import java.util.Optional;
import org.objectweb.asm.ClassReader;
import org.objectweb.asm.ClassVisitor;
import org.objectweb.asm.FieldVisitor;
import org.objectweb.asm.MethodVisitor;
import org.objectweb.asm.Opcodes;

/** JVM class visitor emitting Kythe graph facts. */
public final class KytheClassVisitor extends ClassVisitor {
  private static final int ASM_API_LEVEL = Opcodes.ASM7;

  private final JvmGraph jvmGraph;
  private final KytheEntrySets entrySets;
  private final VName enclosingJarFile;

  private CorpusPath corpusPath;
  private JvmGraph.Type.ReferenceType classType;
  private VName classVName;

  public KytheClassVisitor(StatisticsCollector statistics, FactEmitter emitter) {
    this(statistics, emitter, null);
  }

  public KytheClassVisitor(
      StatisticsCollector statistics, FactEmitter emitter, VName enclosingJarFile) {
    this(new JvmGraph(statistics, emitter), enclosingJarFile);
  }

  private KytheClassVisitor(JvmGraph jvmGraph, VName enclosingJarFile) {
    super(ASM_API_LEVEL);
    this.jvmGraph = jvmGraph;
    this.entrySets = jvmGraph.getKytheEntrySets();
    this.enclosingJarFile = enclosingJarFile;
  }

  /** Parse and visit the class file represented by the given {@link InputStream}. */
  public void visitClassFile(InputStream classFile) throws IOException {
    new ClassReader(classFile).accept(this, 0);
  }

  /** Parse and visit the class file represented by the given {@code byte[]}. */
  public void visitClassFile(byte[] b) {
    new ClassReader(b).accept(this, 0);
  }

  /**
   * Returns a new {@link KytheClassVisitor} for classes enclosed within the {@code .jar} file
   * described by the given {@link VName}. If {@code null}, all further classes visited will not be
   * related to a {@code .jar} file.
   */
  public KytheClassVisitor withEnclosingJarFile(VName enclosingJarFile) {
    return new KytheClassVisitor(jvmGraph, enclosingJarFile);
  }

  public KytheEntrySets getEntrySets() {
    return this.entrySets;
  }

  @Override
  public void visit(
      int version,
      int access,
      String name,
      String signature,
      String superName,
      String[] interfaces) {
    corpusPath =
        new CorpusPath(
            Optional.ofNullable(enclosingJarFile).map(VName::getCorpus).orElse(""), "", "");
    classType = JvmGraph.Type.referenceType(name);
    classVName =
        flagSet(access, Opcodes.ACC_ENUM)
            ? jvmGraph.emitEnumNode(corpusPath, classType)
            : flagSet(access, Opcodes.ACC_INTERFACE)
                ? jvmGraph.emitInterfaceNode(corpusPath, classType)
                : jvmGraph.emitClassNode(corpusPath, classType);
    if (enclosingJarFile != null) {
      entrySets.emitEdge(
          entrySets.newImplicitAnchorAndEmit(enclosingJarFile).getVName(),
          EdgeKind.DEFINES,
          classVName);
    }
    if (superName != null) {
      entrySets.emitEdge(
          classVName,
          EdgeKind.EXTENDS,
          // TODO(schroederc): allow indexer to understand references outside of the enclosing jar
          JvmGraph.getReferenceVName(corpusPath, JvmGraph.Type.referenceType(superName)));
    }
    for (String iface : interfaces) {
      entrySets.emitEdge(
          classVName,
          EdgeKind.EXTENDS,
          // TODO(schroederc): allow indexer to understand references outside of the enclosing jar
          jvmGraph.emitInterfaceNode(corpusPath, JvmGraph.Type.referenceType(iface)));
    }
    super.visit(version, access, name, signature, superName, interfaces);
  }

  @Override
  public void visitInnerClass(String name, String outerName, String innerName, int access) {
    if (outerName != null) { // avoid anonymous/local classes
      entrySets.emitEdge(
          JvmGraph.getReferenceVName(corpusPath, JvmGraph.Type.referenceType(name)),
          EdgeKind.CHILDOF,
          JvmGraph.getReferenceVName(corpusPath, JvmGraph.Type.referenceType(outerName)));
    }
    super.visitInnerClass(name, outerName, innerName, access);
  }

  @Override
  public MethodVisitor visitMethod(
      int access, String methodName, String desc, String signature, String[] exceptions) {
    JvmGraph.Type.MethodType methodType = JvmGraph.Type.rawMethodType(desc);
    VName methodVName = jvmGraph.emitMethodNode(corpusPath, classType, methodName, methodType);
    entrySets.emitEdge(methodVName, EdgeKind.CHILDOF, classVName);

    // TODO(zarko): access & Opcodes.ACC_NATIVE -- we need to know if the native method has
    // at least one native overload.

    return new MethodVisitor(ASM_API_LEVEL) {
      private int parameterIndex = 0;

      @Override
      public void visitParameter(String parameterName, int access) {
        VName parameterVName =
            jvmGraph.emitParameterNode(
                corpusPath, classType, methodName, methodType, parameterIndex);
        entrySets.emitEdge(parameterVName, EdgeKind.CHILDOF, methodVName);
        entrySets.emitEdge(methodVName, EdgeKind.PARAM, parameterVName, parameterIndex);

        parameterIndex++;
        super.visitParameter(parameterName, access);
      }
    };
  }

  @Override
  public FieldVisitor visitField(
      int access, String name, String desc, String signature, Object value) {
    VName fieldVName = jvmGraph.emitFieldNode(corpusPath, classType, name);
    entrySets.emitEdge(fieldVName, EdgeKind.CHILDOF, classVName);
    return super.visitField(access, name, desc, signature, value);
  }

  private static boolean flagSet(int flags, int flag) {
    return (flags & flag) == flag;
  }
}
