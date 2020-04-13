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

package com.google.devtools.kythe.platform.java.helpers;

import com.google.common.flogger.FluentLogger;
import com.sun.source.tree.CompilationUnitTree;
import com.sun.source.util.TreePath;
import com.sun.tools.javac.code.BoundKind;
import com.sun.tools.javac.code.Symbol;
import com.sun.tools.javac.code.Symbol.ClassSymbol;
import com.sun.tools.javac.code.Symbol.MethodSymbol;
import com.sun.tools.javac.code.Symbol.TypeSymbol;
import com.sun.tools.javac.code.Symbol.TypeVariableSymbol;
import com.sun.tools.javac.code.Symbol.VarSymbol;
import com.sun.tools.javac.code.Type;
import com.sun.tools.javac.code.Type.ArrayType;
import com.sun.tools.javac.code.Type.CapturedType;
import com.sun.tools.javac.code.Type.ClassType;
import com.sun.tools.javac.code.Type.ErrorType;
import com.sun.tools.javac.code.Type.ForAll;
import com.sun.tools.javac.code.Type.IntersectionClassType;
import com.sun.tools.javac.code.Type.MethodType;
import com.sun.tools.javac.code.Type.ModuleType;
import com.sun.tools.javac.code.Type.PackageType;
import com.sun.tools.javac.code.Type.TypeVar;
import com.sun.tools.javac.code.Type.UndetVar;
import com.sun.tools.javac.code.Type.Visitor;
import com.sun.tools.javac.code.Type.WildcardType;
import com.sun.tools.javac.code.Types;
import com.sun.tools.javac.util.Context;
import com.sun.tools.javac.util.Name;
import java.nio.charset.StandardCharsets;
import java.util.concurrent.TimeUnit;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.Set;
import java.util.stream.Collectors;
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

  private final boolean useJvmSignatures;

  private final boolean ignoreTypesigExceptions;

  private static boolean isJavaLangObject(Type type) {
    return type.tsym.getQualifiedName().contentEquals(Object.class.getName());
  }

  private static final FluentLogger logger = FluentLogger.forEnclosingClass();

  static final String ANONYMOUS = "/anonymous";

  // Constant used to prepend to the beginning of error type names.
  private static final String ERROR_TYPE = "ERROR-TYPE";

  // The full name of the compiler-generated class that holds array members
  // (e.g., clone(), length) on behalf of the true array types.
  // Note the name has no package prefix.
  private static final String ARRAY_HELPER_CLASS = "Array";

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

  // If we're generating a signature for a member of an array type (e.g., the `length` and `clone()`
  // members), this field references that array type.  Otherwise, it's set to null.
  //
  // This is necessary because javac, as an implementation detail, uses a generated class named
  // `Array` (with no package) as an intermediate holder for the references to those members.
  // Therefore the MethodSymbol for, e.g., `clone()` will have as its owner the ClassSymbol for this
  // `Array` class, instead of a ClassSymbol representing the actual array type, e.g., `int[]`. This
  // will cause us to generate the signature `.Array.clone()` instead of `int[].clone()`.
  //
  // Not only does this expose a compiler implementation detail, but it gives the same signature for
  // all array types, whereas the JLS specifies, in effect, that `int[].clone()`, `float[].clone()`
  // etc.  are different methods (each array type directly extends Object, so, at the language
  // level, each must define its own clone()).  See
  // https://docs.oracle.com/javase/specs/jls/se8/html/jls-10.html#jls-10.8 for details.
  //
  // The original array type is not reachable from the member Symbol, so we use this field to
  // provide that information out-of-band, so that we can generate signatures that conform to the
  // specific array types.
  private Type arrayTypeContext = null;

  public TreePath getPath(Element e) {
    return memoizedTreePathScanner.getPath(e);
  }

  public boolean getUseJvmSignatures() {
    return useJvmSignatures;
  }
  // Do not decrease this number.
  private static final int STRING_BUILDER_INIT_CAPACITY = 512;

  private final SigGen sigGen;

  public StringBuilder getStringBuilder() {
    return new StringBuilder(STRING_BUILDER_INIT_CAPACITY);
  }

  public SignatureGenerator(
      CompilationUnitTree compilationUnit, Context context, boolean useJvmSignatures,
      boolean ignoreTypesigExceptions) {
    this.useJvmSignatures = useJvmSignatures;
    this.ignoreTypesigExceptions = ignoreTypesigExceptions;
    this.memoizedTreePathScanner = new MemoizedTreePathScanner(compilationUnit, context);
    sigGen = new SigGen(Types.instance(context));
  }

  /** Does this symbol represent the compiler-generated Array member helper class? */
  public static boolean isArrayHelperClass(Symbol sym) {
    return (sym instanceof ClassSymbol)
        && ((ClassSymbol) sym).className().equals(ARRAY_HELPER_CLASS);
  }

  /** Call this when generating a signature for an array member, with the array's type. */
  public void setArrayTypeContext(Type arrayTypeContext) {
    this.arrayTypeContext = arrayTypeContext;
  }

  public String getArrayTypeName() {
    if (arrayTypeContext == null) {
      return ARRAY_HELPER_CLASS; // With no context we indicate an array of unknown type.
    }
    return arrayTypeContext.toString();
  }

  /** Returns a Java signature for the specified Symbol. */
  public Optional<String> getSignature(Symbol symbol) {
    if (symbol == null) {
      return Optional.empty();
    }
    if (useJvmSignatures) {
      if (symbol instanceof ClassSymbol) {
        return Optional.of(sigGen.getSignature((ClassSymbol) symbol));
      } else if (symbol instanceof MethodSymbol) {
        return Optional.of(sigGen.getSignature((MethodSymbol) symbol));
      } else if (symbol instanceof VarSymbol) {
        try {
          return Optional.of(sigGen.getSignature((VarSymbol) symbol));
        } catch (Types.SignatureGenerator.InvalidSignatureException e) {
          final String action;
          if (ignoreTypesigExceptions) {
            action = "omitting reference";
          } else {
            action = "consider --ignore_typesig_exceptions";
          }
          logger.atWarning()
            .atMostEvery(5, TimeUnit.SECONDS)
            .log("Likely hitting bug JDK-8212750, %s"
                    + " (VarSymbol [%s] in class [%s], package [%s], type [%s])",
                action,
                symbol, symbol.outermostClass(), symbol.packge(), symbol.type);
          if (ignoreTypesigExceptions) {
            return Optional.empty();
          }
          throw new RuntimeException("Hit JDK-8212750, see logs for details.", e);
        }
      }
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
      logger.atWarning().withCause(e).log("Failure generating signature for %s", symbol);
      return Optional.empty();
    }
  }

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
    if (e.getQualifiedName().contentEquals(ARRAY_HELPER_CLASS)) {
      sbout.append(getArrayTypeName());
    } else {
      if (!visitedElements.containsKey(e)) {
        StringBuilder sb = new StringBuilder();
        if (e.getNestingKind() == NestingKind.ANONYMOUS) {
          sb.append(blockNumber.getAnonymousSignature(e));
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
    }
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

  private void visitTypeParameters(
      List<? extends TypeParameterElement> typeParams, StringBuilder sb) {
    if (!typeParams.isEmpty()) {
      Set<TypeVar> typeVars = new HashSet<>();
      for (TypeParameterElement aType : typeParams) {
        typeVars.add((TypeVar) ((TypeSymbol) aType).type.stripMetadata());
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
        sb.append(tsym.name);
        // Don't use TypeKind to check the upper bound, because java 8 introduces a new kind for
        // intersection types. We can't use TypeKind.INTERSECTION until we're using JDK8, since
        // javax.lang.model.* classes come from the runtime.
        Type upperBound = tsym.type.getUpperBound();
        if (upperBound instanceof IntersectionClassType) {
          IntersectionClassType intersectionType =
              (IntersectionClassType) tsym.type.getUpperBound();
          sb.append(" extends ");
          if (!intersectionType.allInterfaces) {
            intersectionType.supertype_field.accept(this, sb);
            sb.append("&");
          }
          for (Type i : intersectionType.interfaces_field) {
            i.accept(this, sb);
            sb.append("&");
          }
          // Remove the extraneous final '&'.  We know there was at least one.
          // Note that using setLength() to shorten a StringBuilder is efficient,
          // and doesn't trigger allocation or copying.
          sb.setLength(sb.length() - 1);
        } else if ((upperBound instanceof ClassType || upperBound instanceof TypeVar)
            && !isJavaLangObject(upperBound)) {
          sb.append(" extends ");
          upperBound.accept(this, sb);
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

  @Override
  public Void visitArrayType(ArrayType t, StringBuilder sbout) {
    t.getComponentType().accept(this, sbout);
    // TODO(#2232): handle case when ArrayType#isVarargs is incorrect; add "..." for varargs
    sbout.append("[]");
    return null;
  }

  @Override
  public Void visitClassType(ClassType t, StringBuilder sbout) {
    if (!visitedTypes.containsKey(t)) {
      StringBuilder sb = new StringBuilder();
      if (t.getEnclosingType().getKind() != TypeKind.NONE) {
        t.getEnclosingType().accept(this, sb);
      } else {
        sb.append(t.tsym.owner.getQualifiedName());
      }
      sb.append(".");
      sb.append(t.tsym.name);
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
        sb.append(t.tsym.name);
      } else if (t.getKind() == TypeKind.VOID) {
        sb.append(t.tsym.name);
      }
      visitedTypes.put(t, sb.toString());
    }
    sbout.append(visitedTypes.get(t));
    return null;
  }

  @Override
  public Void visitTypeVar(TypeVar t, StringBuilder sbout) {
    if (boundedVars.contains((TypeVar) t.stripMetadata())) {
      sbout.append(t.tsym.name);
    } else {
      if (!visitedTypes.containsKey(t)) {
        boundedVars.add((TypeVar) t.stripMetadata());
        StringBuilder sb = new StringBuilder();
        t.tsym.owner.accept(this, sb);
        sb.append("~").append(t.tsym.name);
        visitedTypes.put(t, sb.toString());
        boundedVars.remove((TypeVar) t.stripMetadata());
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
      sb.append(t.kind);
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
      List<TypeVar> typeVars =
          t.getTypeVariables().stream()
              .map(v -> (TypeVar) v.stripMetadata())
              .collect(Collectors.toList());
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

  @Override
  public Void visitModuleType(ModuleType moduleType, StringBuilder sb) {
    // TODO(#2174): implement this method for full Java 9 support
    throw new UnsupportedOperationException();
  }

  /**
   * Generates signatures that are expansions on the JVM signatures as defined in
   * https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-4.html#jvms-4.3.4
   *
   * <p>The main difference is that jvms-4.3.4 has no notion of full signatures, e.g. for a method
   * we emit type.name.argument/return signature where the spec only defines the argument/return
   * signature.
   */
  private static class SigGen extends com.sun.tools.javac.code.Types.SignatureGenerator {

    /**
     * Generates Type signature according to the "ClassSignature" grammar e.g. "Ljava/lang/String;"
     */
    public String getSignature(ClassSymbol symbol) {
      addSignature(symbol);
      return toString();
    }

    /**
     * Generates full method signature "ClassSignature.name:MethodTypeSignature" e.g.
     * "Ljava/lang/String;.charAt(I)C"
     */
    public String getSignature(MethodSymbol symbol) {
      addSignature(symbol);
      return toString();
    }

    private void addSignature(ClassSymbol symbol) {
      this.assembleSig(symbol.type);
    }

    private void addSignature(MethodSymbol symbol) {
      addSignature((ClassSymbol) symbol.owner);
      append('.');
      append(symbol.name);
      append(':');
      assembleSig(symbol.type);
    }

    /**
     * Generates full variable signature (both fields & variables).
     *
     * <p>Grammar for fields: "ClassSignature.name:ClassSignature" e.g.
     * "Lmy/Type;.myField:Ljava/lang/String;"
     *
     * <p>Grammar for variables: Method signature from {@link #getSignature(MethodSymbol)} +
     * ".name:ClassSignature" e.g.
     * "Lmy/Type;.myMethod(I):Ljava/lang/String;.e:Ljava/lang/Exception;"
     */
    public String getSignature(VarSymbol symbol) {
      Symbol owner = symbol.owner;
      if (owner instanceof ClassSymbol) {
        addSignature((ClassSymbol) owner);
      } else if (owner instanceof MethodSymbol) {
        MethodSymbol method = (MethodSymbol) owner;
        // Javac doesn't assign a type or name to the class initialization block but represents it
        // as a method.
        if (method.type == null) {
          sb.append("<clinit>");
        } else {
          addSignature(method);
        }
      }
      append('.');
      append(symbol.name);
      append(':');
      assembleSig(symbol.type);
      return toString();
    }

    private final StringBuilder sb = new StringBuilder();

    SigGen(Types types) {
      super(types);
    }

    @Override
    protected void append(char ch) {
      sb.append(ch);
    }

    @Override
    protected void append(byte[] ba) {
      sb.append(new String(ba, StandardCharsets.UTF_8));
    }

    @Override
    protected void append(Name name) {
      sb.append(name);
    }

    @Override
    public String toString() {
      String signature = sb.toString();
      sb.setLength(0);
      return signature;
    }
  }
}
