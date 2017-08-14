/*
 * Copyright 2017 Google Inc. All rights reserved.
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

import com.google.auto.service.AutoService;
import com.google.devtools.kythe.analyzers.base.EntrySet;
import com.google.devtools.kythe.analyzers.java.Plugin.KytheNode;
import com.sun.tools.javac.code.Symbol.ClassSymbol;
import com.sun.tools.javac.code.Symtab;
import com.sun.tools.javac.tree.JCTree.JCAnnotation;
import com.sun.tools.javac.tree.JCTree.JCCompilationUnit;
import com.sun.tools.javac.tree.JCTree.JCMethodDecl;
import com.sun.tools.javac.util.Context;
import com.sun.tools.javac.util.Names;
import java.util.Optional;

@AutoService(Plugin.class)
/**
 * Kythe {@link Plugin} that emits "special" Kythe nodes for Java methods annotated with
 * {@code @pkg.PluginTests.SpecialAnnotation}.
 */
public class TestPlugin extends Plugin.Scanner<Void, Void> {
  public TestPlugin() {}

  private KytheNode fileNode;
  private KytheNode specialAnnotationNode;

  @Override
  public Void visitTopLevel(JCCompilationUnit compilation, Void v) {
    Context context = kytheGraph.getJavaContext();
    Symtab symtab = Symtab.instance(context);
    Names names = Names.instance(context);
    ClassSymbol specialAnnotationSym =
        symtab.getClass(symtab.java_base, names.fromString("pkg.PluginTests$SpecialAnnotation"));
    specialAnnotationNode = kytheGraph.getNode(specialAnnotationSym).orElse(null);
    if (specialAnnotationNode == null) {
      return v;
    }
    fileNode = kytheGraph.getNode(compilation).get();
    return super.visitTopLevel(compilation, v);
  }

  @Override
  public Void visitMethodDef(JCMethodDecl methodDef, Void v) {
    for (JCAnnotation ann : methodDef.getModifiers().getAnnotations()) {
      if (!specialAnnotationNode.equals(kytheGraph.getNode(ann.getAnnotationType()).orElse(null))) {
        continue;
      }
      // We're analyzing a method annotated by SpecialAnnotation.  Now we do special analysis.

      kytheGraph
          .getNode(methodDef)
          .map(KytheNode::getVName)
          .ifPresent(
              method -> {
                // Add an extra fact to the method's Kythe node.
                entrySets.getEmitter().emitFact(method, "/extra/fact", "value");

                // Add a new node to the Kythe graph.
                EntrySet specialNode =
                    entrySets.newNode("function", Optional.of("special")).build();
                specialNode.emit(entrySets.getEmitter());

                // Add an extra edge to the Kythe method node from the special node.
                entrySets
                    .getEmitter()
                    .emitEdge(specialNode.getVName(), "/specializing/edge", method);

                // Add anchor for the special node's definition.
                kytheGraph
                    .findIdentifier(methodDef.name, methodDef.getPreferredPosition())
                    .ifPresent(
                        bindingSpan -> {
                          EntrySet anchor =
                              entrySets.newAnchorAndEmit(fileNode.getVName(), bindingSpan, null);
                          entrySets
                              .getEmitter()
                              .emitEdge(
                                  anchor.getVName(),
                                  "/special/defines/binding",
                                  specialNode.getVName());
                        });
              });

      break;
    }
    return super.visitMethodDef(methodDef, v);
  }
}
