/*
 * Copyright 2018 Google Inc. All rights reserved.
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

import com.google.devtools.kythe.analyzers.base.EdgeKind;
import com.google.devtools.kythe.analyzers.base.FactEmitter;
import com.google.devtools.kythe.analyzers.base.KytheEntrySets;
import com.google.devtools.kythe.platform.shared.StatisticsCollector;
import com.google.devtools.kythe.proto.Storage.VName;
import org.objectweb.asm.ClassVisitor;
import org.objectweb.asm.FieldVisitor;
import org.objectweb.asm.MethodVisitor;
import org.objectweb.asm.Opcodes;

/** JVM class visitor emitting Kythe graph facts. */
public final class KytheClassVisitor extends ClassVisitor {
  private final JvmGraph jvmGraph;
  private final KytheEntrySets entrySets;

  private JvmGraph.Type.ReferenceType classType;
  private VName classVName;

  public KytheClassVisitor(StatisticsCollector statistics, FactEmitter emitter) {
    super(Opcodes.ASM6);
    jvmGraph = new JvmGraph(statistics, emitter);
    entrySets = jvmGraph.getKytheEntrySets();
  }

  @Override
  public void visitSource(String source, String debug) {
    super.visitSource(source, debug);
  }

  @Override
  public void visit(
      int version,
      int access,
      String name,
      String signature,
      String superName,
      String[] interfaces) {
    classType = JvmGraph.Type.referenceType(name);
    classVName =
        flagSet(access, Opcodes.ACC_ENUM)
            ? jvmGraph.emitEnumNode(classType)
            : flagSet(access, Opcodes.ACC_INTERFACE)
                ? jvmGraph.emitInterfaceNode(classType)
                : jvmGraph.emitClassNode(classType);
    if (superName != null) {
      entrySets.emitEdge(
          classVName,
          EdgeKind.EXTENDS,
          jvmGraph.getReferenceVName(JvmGraph.Type.referenceType(superName)));
    }
    for (String iface : interfaces) {
      entrySets.emitEdge(
          classVName,
          EdgeKind.EXTENDS,
          jvmGraph.emitInterfaceNode(JvmGraph.Type.referenceType(iface)));
    }
    super.visit(version, access, name, signature, superName, interfaces);
  }

  @Override
  public MethodVisitor visitMethod(
      int access, String name, String desc, String signature, String[] exceptions) {
    JvmGraph.Type.MethodType t = JvmGraph.Type.rawMethodType(desc);
    VName methodVName = jvmGraph.emitMethodNode(classType, name, t);
    entrySets.emitEdge(methodVName, EdgeKind.CHILDOF, classVName);
    return super.visitMethod(access, name, desc, signature, exceptions);
  }

  @Override
  public FieldVisitor visitField(
      int access, String name, String desc, String signature, Object value) {
    VName fieldVName = jvmGraph.emitFieldNode(classType, name);
    entrySets.emitEdge(fieldVName, EdgeKind.CHILDOF, classVName);
    return super.visitField(access, name, desc, signature, value);
  }

  private static boolean flagSet(int flags, int flag) {
    return (flags & flag) == flag;
  }
}
