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

package com.google.devtools.kythe.platform.java.helpers;

import com.google.common.base.Optional;
import com.google.devtools.kythe.common.FormattingLogger;
import com.sun.source.tree.CompilationUnitTree;
import com.sun.source.util.TreePath;
import com.sun.tools.javac.code.BoundKind;
import com.sun.tools.javac.code.Symbol;
import com.sun.tools.javac.code.Symbol.MethodSymbol;
import com.sun.tools.javac.code.Symbol.TypeSymbol;
import com.sun.tools.javac.code.Symbol.TypeVariableSymbol;
import com.sun.tools.javac.code.Type;
import com.sun.tools.javac.code.Type.ArrayType;
import com.sun.tools.javac.code.Type.CapturedType;
import com.sun.tools.javac.code.Type.ClassType;
import com.sun.tools.javac.code.Type.ErrorType;
import com.sun.tools.javac.code.Type.ForAll;
import com.sun.tools.javac.code.Type.MethodType;
import com.sun.tools.javac.code.Type.PackageType;
import com.sun.tools.javac.code.Type.TypeVar;
import com.sun.tools.javac.code.Type.UndetVar;
import com.sun.tools.javac.code.Type.Visitor;
import com.sun.tools.javac.code.Type.WildcardType;
import com.sun.tools.javac.util.Context;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;
import javax.lang.model.element.Element;
import javax.lang.model.element.ElementKind;
import javax.lang.model.element.ElementVisitor;
import javax.lang.model.element.ExecutableElement;
import javax.lang.model.element.NestingKind;
import javax.lang.model.element.PackageElement;
import javax.lang.model.element.TypeElement;
import javax.lang.model.element.TypeParameterElement;
import javax.lang.model.element.VariableElement;
import javax.lang.model.type.TypeKind;

/**
 * This class is responsible for generating signatures for elements in a Java program. The class is
 * valid to use for a single compilation context. A compilation context may contain many compilation
 * units (files). Since, this class might keep references to items in compilation units, users of
 * this class must make sure that they use this class local to a compilation task. That is to make
 * sure that after each compilation task is over the memory for the compilation units are garbage
 * collected properly. Here is an example usage of this class:
 *
 * <p>CompilationTask task = compiler.getTask(null, standardFileManager, diagnosticsCollector,
 * options, null, sourceFiles); JavacTaskImpl javacTask = (JavacTaskImpl) task; SignatureGenerator
 * signatureGenerator = new SignatureGenerator(javacTask.getContext());
 *
 * <p>For more information see the corresponding test directory for this class.
 */
public class SignatureGenerator
    implements ElementVisitor<Void, StringBuilder>, Visitor<Void, StringBuilder> {
  private static final FormattingLogger logger =
      FormattingLogger.getLogger(SignatureGenerator.class);

  static final String ANONYMOUS = "/anonymous";

  // Constant used to prepend to the beginning of error type names.
  private static final String ERROR_TYPE = "ERROR-TYPE";

  private final BlockAnonymousSignatureGenerator blockNumber =
      new BlockAnonymousSignatureGenerator(this);

  // This map is used to store the signature of all the visited types.
  private final Map<Type, String> visitedTypes = new HashMap<>();

  // This map is used to store the signature of all the visited elements.
  // It also helps us resolve the curiously-recurring template pattern.
  private final Map<Element, String> visitedElements = new HashMap<>();

  // The set of type variables which are bounded in  method or class declaration.
  // This set is used to avoid infinite recursion when the type parameter is used in the signature
  // of the method or class.
  private final Set<TypeVar> boundedVars = new HashSet<>();

  private final MemoizedTreePathScanner memoizedTreePathScanner;

  public TreePath getPath(Element e) {
    return memoizedTreePathScanner.getPath(e);
  }

  // Do not decrease this number.
  private static final int STRING_BUILDER_INIT_CAPACITY = 512;

  public StringBuilder getStringBuilder() {
    return new StringBuilder(STRING_BUILDER_INIT_CAPACITY);
  }

  public SignatureGenerator(CompilationUnitTree compilationUnit, Context context) {
    this.memoizedTreePathScanner = new MemoizedTreePathScanner(compilationUnit, context);
  }

  /** Returns a Java signature for the specified Symbol. */
  public Optional<String> getSignature(Symbol symbol) {
    if (symbol == null) {
      return Optional.absent();
    }
    try {
      StringBuilder sb = getStringBuilder();
      if (symbol instanceof TypeVariableSymbol) {
        symbol.type.accept(this, sb);
      } else {
        symbol.accept(this, sb);
      }
      return Optional.of(sb.toString());
    } catch (Throwable e) {
      // In case something unexpected happened during signature generation we do not want to fail.
      logger.warning(new RuntimeException("Failure generating signature for " + symbol, e), "");
      return Optional.absent();
    }
  }

  // Declaration Signatures

  @Override
  public Void visit(Element e, StringBuilder sb) {
    return null;
  }

  @Override
  public Void visit(Element e) {
    return null;
  }

  @Override
  public Void visitPackage(PackageElement e, StringBuilder sbout) {
    if (!visitedElements.containsKey(e)) {
      StringBuilder sb = new StringBuilder();
      sb.append(e.getQualifiedName());
      visitedElements.put(e, sb.toString());
    }
    sbout.append(visitedElements.get(e));
    return null;
  }

  @Override
  public Void visitType(TypeElement e, StringBuilder sbout) {
    if (!visitedElements.containsKey(e)) {
      StringBuilder sb = new StringBuilder();
      if (e.getNestingKind() == NestingKind.ANONYMOUS) {
        if (e.getKind() == ElementKind.ENUM) {
          e.getEnclosingElement().accept(this, sb);
        } else {
          sb.append(blockNumber.getAnonymousSignature(e));
        }
      } else if (e.asType() != null
          && (e.asType().getKind().isPrimitive() || e.asType().getKind() == TypeKind.VOID)) {
        sb.append(e.getSimpleName());
      } else {
        visitEnclosingElement(e, sb);
        sb.append(".").append(e.getSimpleName());
      }
      visitedElements.put(e, sb.toString());
    }
    sbout.append(visitedElements.get(e));
    return null;
  }

  @Override
  public Void visitVariable(VariableElement e, StringBuilder sbout) {
    if (!visitedElements.containsKey(e)) {
      StringBuilder sb = new StringBuilder();
      ElementKind elementKind = e.getKind();
      if (elementKind == ElementKind.LOCAL_VARIABLE
          || elementKind == ElementKind.EXCEPTION_PARAMETER
          || elementKind == ElementKind.RESOURCE_VARIABLE) {
        sb.append(blockNumber.getBlockSignature(e));
        sb.append("#").append(e.getSimpleName());
      } else if (elementKind == ElementKind.PARAMETER) {
        sb.append(blockNumber.getBlockSignature(e));
        sb.append("#").append(e.getSimpleName());
      } else if (elementKind == ElementKind.FIELD) {
        e.getEnclosingElement().accept(this, sb);
        sb.append(".").append(e.getSimpleName());
      } else if (elementKind == ElementKind.ENUM_CONSTANT) {
        e.getEnclosingElement().accept(this, sb);
        sb.append(".").append(e.getSimpleName());
      }
      visitedElements.put(e, sb.toString());
    }
    sbout.append(visitedElements.get(e));
    return null;
  }

  public Void visitEnclosingElement(Element e, StringBuilder sb) {
    if (blockNumber.isInBlock(e)) {
      sb.append(blockNumber.getBlockSignature(e));
    } else {
      e.getEnclosingElement().accept(this, sb);
    }
    return null;
  }

  //////////////////////////// helper functions ////////////////////////////
  private void visitTypeParameters(
      List<? extends TypeParameterElement> typeParams, StringBuilder sb) {
    if (!typeParams.isEmpty()) {
      Set<TypeVar> typeVars = new HashSet<>();
      for (TypeParameterElement aType : typeParams) {
        typeVars.add((TypeVar) ((TypeSymbol) aType).type);
      }
      boundedVars.addAll(typeVars);
      sb.append("<");
      boolean first = true;
      for (TypeParameterElement aType : typeParams) {
        if (first) {
          first = false;
        } else {
          sb.append(",");
        }
        aType.accept(this, sb);
      }
      sb.append(">");
      boundedVars.removeAll(typeVars);
    }
  }

  private void visitParameterTypes(List<Type> types, StringBuilder sb) {
    sb.append("(");
    boolean first = true;
    for (Type pt : types) {
      if (first) {
        first = false;
      } else {
        sb.append(",");
      }
      pt.accept(this, sb);
    }
    sb.append(")");
  }

  private void visitTypeArguments(List<Type> typeArguments, StringBuilder sb) {
    if (!typeArguments.isEmpty()) {
      sb.append("<");
      boolean first = true;
      for (Type aType : typeArguments) {
        if (!first) {
          sb.append(",");
        }
        aType.accept(this, sb);
        first = false;
      }
      sb.append(">");
    }
  }
  //////////////////////////////////////////////////////////////////////////

  @Override
  public Void visitExecutable(ExecutableElement e, StringBuilder sbout) {
    if (!visitedElements.containsKey(e)) {
      StringBuilder sb = new StringBuilder();
      e.getEnclosingElement().accept(this, sb);
      sb.append(".");
      visitTypeParameters(e.getTypeParameters(), sb);
      if (e.getKind() == ElementKind.CONSTRUCTOR) {
        sb.append(e.getEnclosingElement().getSimpleName());
      } else {
        sb.append(e.getSimpleName());
      }
      ((MethodSymbol) e).asType().accept(this, sb);
      visitedElements.put(e, sb.toString());
    }
    sbout.append(visitedElements.get(e));
    return null;
  }

  @Override
  public Void visitTypeParameter(TypeParameterElement e, StringBuilder sbout) {
    TypeSymbol tsym = (TypeSymbol) e;
    if (tsym.type.getKind() != TypeKind.NONE) {
      if (!visitedElements.containsKey(e)) {
        StringBuilder sb = new StringBuilder();
        visitedElements.put(e, tsym.name.toString());
        // Don't use TypeKind to check the upper bound, because java 8 introduces a new kind for
        // intersection types. We can't use TypeKind.INTERSECTION until we're using JDK8, since
        // javax.lang.model.* classes come from the runtime.
        if (tsym.type.getUpperBound() instanceof ClassType) {
          sb.append(tsym.name.toString());
          TypeVar t = (TypeVar) tsym.type;
          ClassType extendsType = (ClassType) t.bound;
          if (extendsType.isCompound()) {
            if (extendsType.supertype_field.getKind() != TypeKind.NONE
                && extendsType.interfaces_field.nonEmpty()) {
              sb.append(" extends ");
              extendsType.supertype_field.accept(this, sb);
              for (Type i : extendsType.interfaces_field) {
                sb.append("&");
                i.accept(this, sb);
              }
            }
          } else {
            if (!extendsType.tsym.getQualifiedName().contentEquals(Object.class.getName())) {
              sb.append(" extends ");
              extendsType.accept(this, sb);
            }
          }
        }
        visitedElements.put(e, sb.toString());
      }
      sbout.append(visitedElements.get(e));
    }
    return null;
  }

  @Override
  public Void visitUnknown(Element e, StringBuilder sb) {
    return null;
  }

  // Instantiation Signatures

  @Override
  public Void visitArrayType(ArrayType t, StringBuilder sbout) {
    t.getComponentType().accept(this, sbout);
    if (t.isVarargs()) {
      sbout.append("...");
    } else {
      sbout.append("[]");
    }
    return null;
  }

  @Override
  public Void visitClassType(ClassType t, StringBuilder sbout) {
    if (!visitedTypes.containsKey(t)) {
      StringBuilder sb = new StringBuilder();
      if (t.getEnclosingType().getKind() != TypeKind.NONE) {
        t.getEnclosingType().accept(this, sb);
      } else {
        sb.append(t.tsym.owner.getQualifiedName().toString());
      }
      sb.append(".");
      sb.append(t.tsym.name.toString());
      visitTypeArguments(t.getTypeArguments(), sb);
      visitedTypes.put(t, sb.toString());
    }
    sbout.append(visitedTypes.get(t));
    return null;
  }

  @Override
  public Void visitMethodType(MethodType t, StringBuilder sbout) {
    if (!visitedTypes.containsKey(t)) {
      StringBuilder sb = new StringBuilder();
      visitParameterTypes(t.getParameterTypes(), sb);
      visitedTypes.put(t, sb.toString());
    }
    sbout.append(visitedTypes.get(t));
    return null;
  }

  @Override
  public Void visitType(Type t, StringBuilder sbout) {
    if (!visitedTypes.containsKey(t)) {
      StringBuilder sb = new StringBuilder();
      if (t.isPrimitive()) {
        sb.append(t.tsym.name.toString());
      } else if (t.getKind() == TypeKind.VOID) {
        sb.append(t.tsym.name.toString());
      }
      visitedTypes.put(t, sb.toString());
    }
    sbout.append(visitedTypes.get(t));
    return null;
  }

  @Override
  public Void visitTypeVar(TypeVar t, StringBuilder sbout) {
    if (boundedVars.contains(t)) {
      sbout.append(t.tsym.name.toString());
    } else {
      if (!visitedTypes.containsKey(t)) {
        StringBuilder sb = new StringBuilder();
        t.tsym.owner.accept(this, sb);
        sb.append("~").append(t.tsym.name.toString());
        visitedTypes.put(t, sb.toString());
      }
      sbout.append(visitedTypes.get(t));
    }
    return null;
  }

  @Override
  public Void visitErrorType(ErrorType t, StringBuilder sbout) {
    if (!visitedTypes.containsKey(t)) {
      StringBuilder sb = new StringBuilder();
      if (t.asElement() != null) {
        sb.append(ERROR_TYPE).append(".").append(t.asElement().getQualifiedName());
      }
      visitedTypes.put(t, sb.toString());
    }
    sbout.append(visitedTypes.get(t));
    return null;
  }

  @Override
  public Void visitWildcardType(WildcardType t, StringBuilder sbout) {
    if (!visitedTypes.containsKey(t)) {
      StringBuilder sb = new StringBuilder();
      sb.append(t.kind.toString());
      if (t.kind == BoundKind.EXTENDS) {
        t.getExtendsBound().accept(this, sb);
      } else if (t.kind == BoundKind.SUPER) {
        t.getSuperBound().accept(this, sb);
      }
      visitedTypes.put(t, sb.toString());
    }
    sbout.append(visitedTypes.get(t));
    return null;
  }

  @Override
  public Void visitCapturedType(CapturedType t, StringBuilder sb) {
    return null;
  }

  @Override
  public Void visitForAll(ForAll t, StringBuilder sbout) {
    if (!visitedTypes.containsKey(t)) {
      StringBuilder sb = new StringBuilder();
      List<TypeVar> typeVars = t.getTypeVariables();
      boundedVars.addAll(typeVars);
      visitParameterTypes(t.getParameterTypes(), sb);
      boundedVars.removeAll(typeVars);
      visitedTypes.put(t, sb.toString());
    }
    sbout.append(visitedTypes.get(t));
    return null;
  }

  @Override
  public Void visitPackageType(PackageType t, StringBuilder sbout) {
    if (!visitedTypes.containsKey(t)) {
      StringBuilder sb = new StringBuilder();
      t.tsym.accept(this, sb);
      visitedTypes.put(t, sb.toString());
    }
    sbout.append(visitedTypes.get(t));
    return null;
  }

  @Override
  public Void visitUndetVar(UndetVar t, StringBuilder sb) {
    return null;
  }

  public Void defaultConstructorSignatureOf(ExecutableElement e, StringBuilder sb) {
    e.getEnclosingElement().accept(this, sb);
    sb.append(".").append(e.getEnclosingElement().getSimpleName()).append("()");
    return null;
  }
}
