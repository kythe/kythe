/*
 * Copyright 2019 The Kythe Authors. All rights reserved.
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

import static java.beans.Introspector.decapitalize;

import com.google.auto.service.AutoService;
import com.google.common.collect.ImmutableMap;
import com.google.common.collect.ImmutableSet;
import com.google.common.collect.Iterables;
import com.google.common.collect.Streams;
import com.google.devtools.kythe.analyzers.base.EdgeKind;
import com.google.devtools.kythe.analyzers.base.EntrySet;
import com.google.devtools.kythe.analyzers.java.Plugin.KytheNode;
import com.google.devtools.kythe.analyzers.java.ResolvedAutoValue.GeneratedSymbol;
import com.google.devtools.kythe.analyzers.java.ResolvedAutoValue.Property;
import com.sun.tools.javac.api.JavacTrees;
import com.sun.tools.javac.code.Flags;
import com.sun.tools.javac.code.Scope;
import com.sun.tools.javac.code.Symbol;
import com.sun.tools.javac.code.Symbol.ClassSymbol;
import com.sun.tools.javac.code.Symbol.MethodSymbol;
import com.sun.tools.javac.code.Symtab;
import com.sun.tools.javac.code.Type;
import com.sun.tools.javac.code.Type.ClassType;
import com.sun.tools.javac.code.Type.MethodType;
import com.sun.tools.javac.code.Types;
import com.sun.tools.javac.tree.JCTree;
import com.sun.tools.javac.tree.JCTree.JCAnnotation;
import com.sun.tools.javac.tree.JCTree.JCAssign;
import com.sun.tools.javac.tree.JCTree.JCClassDecl;
import com.sun.tools.javac.tree.JCTree.JCCompilationUnit;
import com.sun.tools.javac.tree.JCTree.JCExpression;
import com.sun.tools.javac.tree.JCTree.JCLiteral;
import com.sun.tools.javac.tree.JCTree.JCMethodDecl;
import com.sun.tools.javac.tree.JCTree.JCPrimitiveTypeTree;
import com.sun.tools.javac.util.Context;
import com.sun.tools.javac.util.Names;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import javax.lang.model.element.Modifier;
import javax.lang.model.type.TypeKind;
import org.checkerframework.checker.nullness.qual.Nullable;

/** Kythe {@link Plugin} that emits property entries for {@link AutoValue} classes. */
@AutoService(Plugin.class)
public class AutoValuePlugin extends Plugin.Scanner<Void, Void> {
  private static final ImmutableSet<String> GENERATED_ANNOTATIONS =
      ImmutableSet.of("javax.annotation.Generated", "javax.annotation.processing.Generated");

  private static final String AUTO_VALUE_PKG = "com.google.auto.value";
  private static final String AUTO_VALUE_ANNOTATION = AUTO_VALUE_PKG + ".AutoValue";
  private static final String BUILDER_ANNOTATION = AUTO_VALUE_ANNOTATION + ".Builder";
  private static final String AUTO_VALUE_PROCESSOR =
      AUTO_VALUE_PKG + ".processor.AutoValueProcessor";

  private JavacTrees javacTrees;
  private Types types;

  public AutoValuePlugin() {}

  @Override
  public Void visitTopLevel(JCCompilationUnit compilation, Void v) {
    Context context = kytheGraph.getJavaContext();
    Symtab symtab = Symtab.instance(context);
    Names names = Names.instance(context);
    if (Iterables.isEmpty(symtab.getClassesForName(names.fromString(AUTO_VALUE_ANNOTATION)))) {
      // No AutoValue class detected in compilation; early exit.
      return null;
    }
    javacTrees = JavacTrees.instance(context);
    types = Types.instance(context);
    return super.visitTopLevel(compilation, v);
  }

  @Override
  public Void visitClassDef(JCClassDecl classDef, Void v) {
    resolveGeneratedAutoValue(classDef).ifPresent(this::emitAutoValue);
    return super.visitClassDef(classDef, v);
  }

  private void emitAutoValue(ResolvedAutoValue autoValue) {
    // Emit `file` -[generates]-> `generated_file`
    Optional<KytheNode> file =
        kytheGraph.getNode(autoValue.symbol().abstractSym().outermostClass().sourcefile);
    Optional<KytheNode> genFile =
        kytheGraph.getNode(autoValue.symbol().generatedSym().outermostClass().sourcefile);
    if (file.isPresent() && genFile.isPresent()) {
      entrySets.emitEdge(file.get().getVName(), EdgeKind.GENERATES, genFile.get().getVName());
    }

    // Emit `abstract` -[generates]-> `generated` edge for each GeneratedSymbol
    autoValue.stream()
        .forEach(
            sym -> {
              Optional<KytheNode> absNode = kytheGraph.getNode(sym.abstractSym());
              Optional<KytheNode> genNode = kytheGraph.getNode(sym.generatedSym());
              if (absNode.isPresent() && genNode.isPresent()) {
                entrySets.emitEdge(
                    absNode.get().getVName(), EdgeKind.GENERATES, genNode.get().getVName());
              }
            });

    for (Property prop : autoValue.properties()) {
      Optional<KytheNode> getterNode = kytheGraph.getNode(prop.getter().abstractSym());
      if (getterNode.isPresent()) {
        // Emit new property node for each known Property
        EntrySet propNode =
            // TODO(schroederc): possibly reuse getter method as "property" node or change node kind
            entrySets.newNode("property").addSignatureSalt(getterNode.get().getVName()).build();
        propNode.emit(entrySets.getEmitter());

        // Emit property/reads edges for getter symbols.
        prop.getter().stream()
            .map(kytheGraph::getNode)
            .flatMap(Streams::stream)
            .map(KytheNode::getVName)
            .forEach(
                getter -> entrySets.emitEdge(getter, EdgeKind.PROPERTY_READS, propNode.getVName()));

        // Mark the "set" semantic of the setter generator symbol.
        prop.setter()
            .map(GeneratedSymbol::abstractSym)
            .flatMap(kytheGraph::getNode)
            .map(KytheNode::getVName)
            .ifPresent(
                setter ->
                    entrySets.getEmitter().emitFact(setter, "/kythe/semantic/generated", "set"));

        // Emit property/writes edges for setter symbols.
        prop.setter().stream()
            .flatMap(GeneratedSymbol::stream)
            .map(kytheGraph::getNode)
            .flatMap(Streams::stream)
            .map(KytheNode::getVName)
            .forEach(
                setter ->
                    entrySets.emitEdge(setter, EdgeKind.PROPERTY_WRITES, propNode.getVName()));
      }
    }
  }

  private @Nullable JCClassDecl findAnnotatedSuperclass(
      ClassType classType, String annotationName) {
    while (true) {
      for (Type i : classType.interfaces_field) {
        JCTree superTypeTree = javacTrees.getTree(i.asElement());
        if (superTypeTree instanceof JCClassDecl) {
          JCClassDecl cls = (JCClassDecl) superTypeTree;
          if (getAnnotation(annotationName, cls.getModifiers().getAnnotations()).isPresent()) {
            return cls;
          }
        }
      }
      if (!(classType.supertype_field instanceof ClassType)) {
        return null;
      }
      classType = (ClassType) classType.supertype_field;
      JCTree superTypeTree = javacTrees.getTree(classType.asElement());
      if (superTypeTree instanceof JCClassDecl) {
        JCClassDecl cls = (JCClassDecl) superTypeTree;
        if (getAnnotation(annotationName, cls.getModifiers().getAnnotations()).isPresent()) {
          return cls;
        }
      }
    }
  }

  private Optional<ResolvedAutoValue> resolveGeneratedAutoValue(JCClassDecl classDef) {
    // Check if class was generated by the AutoValueProcessor
    if (!classDef.getModifiers().getAnnotations().stream()
        .filter(
            // Class decorated by @Generated(value = "...AutoValueProcessor")
            ann ->
                GENERATED_ANNOTATIONS.contains(annotationName(ann))
                    && AUTO_VALUE_PROCESSOR.equals(annotationLiteralValue(ann, "value")))
        .findFirst()
        .isPresent()) {
      return Optional.empty();
    }

    // Find @AutoValue superclass definition.
    JCClassDecl autoValueClass =
        findAnnotatedSuperclass((ClassType) classDef.type, AUTO_VALUE_ANNOTATION);
    if (autoValueClass == null) {
      return Optional.empty();
    }

    // Construct base ResolvedAutoValue.
    ResolvedAutoValue.Builder autoValue =
        ResolvedAutoValue.builder()
            .setSymbol(
                GeneratedSymbol.builder()
                    .setAbstractSym(autoValueClass.sym)
                    .setGeneratedSym(classDef.sym)
                    .build());
    Map<String, GeneratedSymbol> getters = new HashMap<>();
    Map<String, GeneratedSymbol> setters = new HashMap<>();

    // Search generated AutoValue class for getters/builder+setters.
    for (JCTree def : classDef.getMembers()) {
      if (def instanceof JCMethodDecl) {
        JCMethodDecl method = (JCMethodDecl) def;
        if (method.getParameters().isEmpty()
            && !isVoid(method.getReturnType())
            && !types.overridesObjectMethod(classDef.sym, method.sym)) {
          Optional<Symbol> abstractSym =
              findAbstractMethod((ClassSymbol) autoValueClass.sym, method.sym);
          if (abstractSym.isPresent()) {
            getters.put(
                method.sym.getSimpleName().toString(),
                GeneratedSymbol.builder()
                    .setAbstractSym(abstractSym.get())
                    .setGeneratedSym(method.sym)
                    .build());
          }
        }
      } else if (def instanceof JCClassDecl) {
        JCClassDecl subClass = (JCClassDecl) def;
        JCClassDecl builderClass =
            findAnnotatedSuperclass((ClassType) subClass.type, BUILDER_ANNOTATION);
        if (builderClass != null) {
          // Found builder class
          autoValue.setBuilderSymbol(
              GeneratedSymbol.builder()
                  .setAbstractSym(builderClass.sym)
                  .setGeneratedSym(subClass.sym)
                  .build());

          // Search builder for setter methods.
          for (JCTree builderDef : subClass.getMembers()) {
            if (builderDef instanceof JCMethodDecl) {
              JCMethodDecl method = (JCMethodDecl) builderDef;
              Type retType = ((MethodType) method.type).restype;
              if (method.getParameters().size() == 1
                  && types.isSameType(builderClass.type, retType)) {
                Optional<Symbol> abstractSym =
                    findAbstractMethod((ClassSymbol) builderClass.sym, method.sym);
                if (abstractSym.isPresent()) {
                  setters.put(
                      method.sym.getSimpleName().toString(),
                      GeneratedSymbol.builder()
                          .setAbstractSym(abstractSym.get())
                          .setGeneratedSym(method.sym)
                          .build());
                }
              }
            }
          }
        }
      }
    }
    if (getters.isEmpty()) {
      return Optional.empty();
    }

    // Getters must either be all prefixed or not prefixed; trim prefixes, if found.
    if (getters.keySet().stream().allMatch(AutoValuePlugin::isPrefixedGetter)) {
      getters = trimPropertyNames(getters);
    }
    // Separately, setters must either be all prefixed or not prefixed; trim prefixes, if found.
    if (setters.keySet().stream().allMatch(AutoValuePlugin::isPrefixedSetter)) {
      setters = trimPropertyNames(setters);
    }

    // Pair getters and setters by property name.
    for (Map.Entry<String, GeneratedSymbol> getter : getters.entrySet()) {
      String name = getter.getKey();
      Property.Builder prop = Property.builder().setName(name).setGetter(getter.getValue());
      if (setters.containsKey(name)) {
        prop.setSetter(setters.get(name));
      }
      autoValue.addProperty(prop.build());
    }

    return Optional.of(autoValue.build());
  }

  private Optional<Symbol> findAbstractMethod(ClassSymbol abstractClass, MethodSymbol genMethod) {
    Scope scope = abstractClass.members();
    for (Symbol abstractSym : scope.getSymbolsByName(genMethod.name)) {
      if (abstractSym != null
          && Flags.asModifierSet(abstractSym.flags()).contains(Modifier.ABSTRACT)
          && genMethod.overrides(abstractSym, abstractClass, types, true)) {
        return Optional.of(abstractSym);
      }
    }
    return Optional.empty();
  }

  private static Optional<JCAnnotation> getAnnotation(String name, List<JCAnnotation> annotations) {
    return annotations.stream().filter(ann -> annotationName(ann).equals(name)).findFirst();
  }

  private static String annotationName(JCAnnotation ann) {
    return ann.getAnnotationType().type.tsym.toString();
  }

  private static @Nullable Object annotationLiteralValue(JCAnnotation ann, String name) {
    for (JCExpression expr : ann.getArguments()) {
      JCAssign a = (JCAssign) expr;
      if (a.lhs.toString().equals(name) && a.rhs instanceof JCLiteral) {
        return ((JCLiteral) a.rhs).value;
      }
    }
    return null;
  }

  private static ImmutableMap<String, GeneratedSymbol> trimPropertyNames(
      Map<String, GeneratedSymbol> syms) {
    ImmutableMap.Builder<String, GeneratedSymbol> unprefixed = ImmutableMap.builder();
    for (Map.Entry<String, GeneratedSymbol> sym : syms.entrySet()) {
      unprefixed.put(propertyName(sym.getKey()), sym.getValue());
    }
    return unprefixed.buildOrThrow();
  }

  private static boolean isPrefixedGetter(String name) {
    return name.startsWith("get") || name.startsWith("is");
  }

  private static boolean isPrefixedSetter(String name) {
    return name.startsWith("set");
  }

  private static String propertyName(String name) {
    if (name.startsWith("get") || name.startsWith("set")) {
      name = name.substring(3);
    } else if (name.startsWith("is")) {
      name = name.substring(2);
    }
    return decapitalize(name);
  }

  private static boolean isVoid(JCTree tree) {
    return tree instanceof JCPrimitiveTypeTree
        && TypeKind.VOID.equals(((JCPrimitiveTypeTree) tree).getPrimitiveTypeKind());
  }
}
