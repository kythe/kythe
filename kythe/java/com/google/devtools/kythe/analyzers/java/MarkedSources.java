/*
 * Copyright 2017 The Kythe Authors. All rights reserved.
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

import com.google.common.collect.ImmutableSet;
import com.google.common.collect.ImmutableSortedSet;
import com.google.common.collect.Sets;
import com.google.devtools.kythe.platform.java.helpers.SignatureGenerator;
import com.google.devtools.kythe.proto.MarkedSource;
import com.google.devtools.kythe.proto.Storage.VName;
import com.google.devtools.kythe.util.KytheURI;
import com.sun.tools.javac.code.BoundKind;
import com.sun.tools.javac.code.Symbol;
import com.sun.tools.javac.code.Symbol.ClassSymbol;
import com.sun.tools.javac.code.Type;
import com.sun.tools.javac.code.TypeTag;
import java.util.ArrayList;
import java.util.List;
import java.util.Optional;
import java.util.function.Function;
import javax.lang.model.element.ElementKind;
import javax.lang.model.element.Modifier;
import org.checkerframework.checker.nullness.qual.Nullable;

/** {@link MarkedSource} utility class. */
public final class MarkedSources {

  private MarkedSources() {}

  /** {@link MarkedSource} for Java "tapp" nodes of the form {@code C<T1, T2, T3...>}. */
  public static final MarkedSource GENERIC_TAPP;

  /** {@link MarkedSource} for Java "tapp" nodes of the form {@code C[]}. */
  public static final MarkedSource ARRAY_TAPP;

  /** {@link MarkedSource} for Java "tapp" nodes of the form {@code R(T1, T2, T3...)}. */
  public static final MarkedSource FN_TAPP;

  /** {@link MarkedSource} for Java "tapp" nodes of the form {@code R C::(T1, T2, T3...)}. */
  public static final MarkedSource METHOD_TAPP;

  static {
    MarkedSource.Builder genericTAppBuilder =
        MarkedSource.newBuilder().setKind(MarkedSource.Kind.TYPE);
    genericTAppBuilder.addChildBuilder().setKind(MarkedSource.Kind.LOOKUP_BY_PARAM);
    genericTAppBuilder
        .addChildBuilder()
        .setKind(MarkedSource.Kind.PARAMETER_LOOKUP_BY_PARAM)
        .setLookupIndex(1)
        .setPreText("<")
        .setPostText(">")
        .setPostChildText(", ");
    GENERIC_TAPP = genericTAppBuilder.build();
  }

  static {
    MarkedSource.Builder arrayTAppBuilder =
        MarkedSource.newBuilder().setKind(MarkedSource.Kind.TYPE);
    arrayTAppBuilder
        .addChildBuilder()
        .setKind(MarkedSource.Kind.PARAMETER_LOOKUP_BY_PARAM)
        .setLookupIndex(1)
        .setPostText("[]");
    ARRAY_TAPP = arrayTAppBuilder.build();
  }

  static {
    MarkedSource.Builder fnTAppBuilder = MarkedSource.newBuilder().setKind(MarkedSource.Kind.TYPE);
    fnTAppBuilder.addChildBuilder().setKind(MarkedSource.Kind.LOOKUP_BY_PARAM).setLookupIndex(1);
    fnTAppBuilder
        .addChildBuilder()
        .setKind(MarkedSource.Kind.PARAMETER_LOOKUP_BY_PARAM)
        .setLookupIndex(2)
        .setPreText("(")
        .setPostText(")")
        .setPostChildText(", ");
    FN_TAPP = fnTAppBuilder.build();
  }

  static {
    MarkedSource.Builder methodTAppBuilder =
        MarkedSource.newBuilder().setKind(MarkedSource.Kind.TYPE);
    methodTAppBuilder
        .addChildBuilder()
        .setKind(MarkedSource.Kind.BOX)
        .setPostText(" ")
        .addChildBuilder()
        .setKind(MarkedSource.Kind.LOOKUP_BY_PARAM)
        .setLookupIndex(1);
    methodTAppBuilder
        .addChildBuilder()
        .setKind(MarkedSource.Kind.BOX)
        .setPostText("::")
        .addChildBuilder()
        .setKind(MarkedSource.Kind.LOOKUP_BY_PARAM)
        .setLookupIndex(2);
    methodTAppBuilder
        .addChildBuilder()
        .setKind(MarkedSource.Kind.PARAMETER_LOOKUP_BY_PARAM)
        .setLookupIndex(3)
        .setPreText("(")
        .setPostText(")")
        .setPostChildText(", ");
    METHOD_TAPP = methodTAppBuilder.build();
  }

  /** Returns a {@link MarkedSource} instance for a {@link Symbol}. */
  static MarkedSource construct(
      SignatureGenerator signatureGenerator,
      Symbol sym,
      MarkedSource.@Nullable Builder msBuilder,
      @Nullable Iterable<MarkedSource> postChildren,
      Function<Symbol, Optional<VName>> symNames) {
    MarkedSource markedType = markType(signatureGenerator, sym, symNames);
    return construct(signatureGenerator, sym, msBuilder, postChildren, markedType);
  }

  private static MarkedSource construct(
      SignatureGenerator signatureGenerator,
      Symbol sym,
      MarkedSource.@Nullable Builder msBuilder,
      @Nullable Iterable<MarkedSource> postChildren,
      @Nullable MarkedSource markedType) {
    MarkedSource.Builder markedSource = msBuilder == null ? MarkedSource.newBuilder() : msBuilder;
    ImmutableSortedSet<Modifier> modifiers = getModifiers(sym);
    MarkedSource.Builder mods = null;
    if (!modifiers.isEmpty()
        || ImmutableSet.of(
                ElementKind.CLASS, ElementKind.ENUM, ElementKind.INTERFACE, ElementKind.PACKAGE)
            .contains(sym.getKind())) {
      mods = markedSource.addChildBuilder().setPostChildText(" ").setAddFinalListToken(true);
      for (Modifier m : modifiers) {
        mods.addChild(
            MarkedSource.newBuilder()
                .setKind(MarkedSource.Kind.MODIFIER)
                .setPreText(m.toString())
                .build());
      }
    }
    if (markedType != null && sym.getKind() != ElementKind.CONSTRUCTOR) {
      markedSource.addChild(markedType);
    }
    String identToken = buildContext(markedSource.addChildBuilder(), sym, signatureGenerator);
    switch (sym.getKind()) {
      case TYPE_PARAMETER:
        markedSource
            .addChildBuilder()
            .setKind(MarkedSource.Kind.IDENTIFIER)
            .setPreText("" + sym.getSimpleName());
        break;
      case CONSTRUCTOR:
      case METHOD:
        ClassSymbol enclClass = sym.enclClass();
        String methodName;
        if (sym.getKind() == ElementKind.CONSTRUCTOR && enclClass != null) {
          methodName = enclClass.getSimpleName().toString();
        } else {
          methodName = sym.getSimpleName().toString();
        }
        markedSource.addChildBuilder().setKind(MarkedSource.Kind.IDENTIFIER).setPreText(methodName);
        markedSource
            .addChildBuilder()
            .setKind(MarkedSource.Kind.PARAMETER_LOOKUP_BY_PARAM)
            .setPreText("(")
            .setPostChildText(", ")
            .setPostText(")");
        break;
      case ENUM:
      case CLASS:
      case INTERFACE:
        mods.addChild(
            MarkedSource.newBuilder()
                .setKind(MarkedSource.Kind.MODIFIER)
                .setPreText(sym.getKind().toString().toLowerCase())
                .build());
        markedSource.addChildBuilder().setKind(MarkedSource.Kind.IDENTIFIER).setPreText(identToken);
        if (!sym.getTypeParameters().isEmpty()) {
          markedSource
              .addChildBuilder()
              .setKind(MarkedSource.Kind.PARAMETER_LOOKUP_BY_TPARAM)
              .setPreText("<")
              .setPostText(">")
              .setPostChildText(", ");
        }
        break;
      default:
        markedSource.addChildBuilder().setKind(MarkedSource.Kind.IDENTIFIER).setPreText(identToken);
        break;
    }
    if (postChildren != null) {
      postChildren.forEach(markedSource::addChild);
    }
    return markedSource.build();
  }

  private static ImmutableSortedSet<Modifier> getModifiers(Symbol sym) {
    ImmutableSortedSet<Modifier> modifiers = ImmutableSortedSet.copyOf(sym.getModifiers());
    switch (sym.getKind()) {
      case ENUM:
        // Remove synthesized enum modifiers
        return ImmutableSortedSet.copyOf(
            Sets.difference(modifiers, ImmutableSet.of(Modifier.STATIC, Modifier.FINAL)));
      case INTERFACE:
        // Remove synthesized interface modifiers
        return ImmutableSortedSet.copyOf(
            Sets.difference(modifiers, ImmutableSet.of(Modifier.ABSTRACT, Modifier.STATIC)));
      case ENUM_CONSTANT:
        // Remove synthesized enum constantc modifiers
        return ImmutableSortedSet.copyOf(
            Sets.difference(
                modifiers, ImmutableSet.of(Modifier.PUBLIC, Modifier.STATIC, Modifier.FINAL)));
      default:
        return modifiers;
    }
  }

  /**
   * Sets the provided {@link MarkedSource.Builder} to a CONTEXT node, populating it with the
   * fully-qualified parent scope for sym. Returns the identifier corresponding to sym.
   */
  private static String buildContext(
      MarkedSource.Builder context, Symbol sym, SignatureGenerator signatureGenerator) {
    context.setKind(MarkedSource.Kind.CONTEXT).setPostChildText(".").setAddFinalListToken(true);
    String identToken = getIdentToken(sym, signatureGenerator);
    Symbol parent = getQualifiedNameParent(sym);
    List<MarkedSource> parents = new ArrayList<>();
    while (parent != null) {
      String parentName = getIdentToken(parent, signatureGenerator);
      if (!parentName.isEmpty()) {
        parents.add(
            MarkedSource.newBuilder()
                .setKind(MarkedSource.Kind.IDENTIFIER)
                .setPreText(parentName)
                .build());
      }
      parent = getQualifiedNameParent(parent);
    }
    for (int i = 0; i < parents.size(); ++i) {
      context.addChild(parents.get(parents.size() - i - 1));
    }
    return identToken;
  }

  /**
   * Returns a {@link MarkedSource} instance for sym's type (or its return type, if sym is a
   * method). If there is no appropriate type for sym, returns {@code null}. Generates links with
   * {@code signatureGenerator}.
   */
  private static @Nullable MarkedSource markType(
      SignatureGenerator signatureGenerator,
      Symbol sym,
      Function<Symbol, Optional<VName>> symNames) {
    // TODO(zarko): Mark up any annotations.
    Type type = sym.type;
    if (type == null || sym == type.tsym) {
      return null;
    }
    if (type.getReturnType() != null) {
      type = type.getReturnType();
    }
    String postTypeIdText = "";
    if (type.hasTag(TypeTag.ARRAY) && ((Type.ArrayType) type).elemtype != null) {
      postTypeIdText = "[]";
      type = ((Type.ArrayType) type).elemtype;
    }
    MarkedSource.Builder builder =
        MarkedSource.newBuilder().setKind(MarkedSource.Kind.TYPE).setPostText(" ");
    if (type.hasTag(TypeTag.CLASS)) {
      MarkedSource.Builder classIdentParent = builder;
      if (!postTypeIdText.isEmpty()) {
        classIdentParent = builder.addChildBuilder().setPostText(postTypeIdText);
      }
      addClassIdentifier(type, classIdentParent, signatureGenerator, symNames);
    } else {
      builder
          .addChildBuilder()
          .setKind(MarkedSource.Kind.IDENTIFIER)
          .setPreText(type.toString())
          .setPostText(postTypeIdText);
    }
    return builder.build();
  }

  private static void addClassIdentifier(
      Type type,
      MarkedSource.Builder parent,
      SignatureGenerator signatureGenerator,
      Function<Symbol, Optional<VName>> symNames) {
    // Add the class CONTEXT (i.e. package) and class IDENTIFIER (i.e. simple name).
    // The qualifiedName BOX is used to restrict the Link added below.
    MarkedSource.Builder qualifiedName = parent.addChildBuilder();
    String identToken =
        buildContext(qualifiedName.addChildBuilder(), type.tsym, signatureGenerator);
    qualifiedName.addChildBuilder().setKind(MarkedSource.Kind.IDENTIFIER).setPreText(identToken);

    // Add a link to the Kythe semantic node for the class.
    symNames
        .apply(type.tsym)
        .map(KytheURI::asString)
        .ifPresent(ticket -> qualifiedName.addLinkBuilder().addDefinition(ticket));

    // Possibly add a PARAMETER node for the class type arguments.
    if (!type.getTypeArguments().isEmpty()) {
      MarkedSource.Builder typeArgs =
          parent
              .addChildBuilder()
              .setKind(MarkedSource.Kind.PARAMETER)
              .setPreText("<")
              .setPostChildText(", ")
              .setPostText(">");
      for (Type arg : type.getTypeArguments()) {
        switch (arg.getTag()) {
          case CLASS:
            addClassIdentifier(arg, typeArgs.addChildBuilder(), signatureGenerator, symNames);
            break;
          case WILDCARD:
            Type.WildcardType wild = (Type.WildcardType) arg;
            // JDK19+ changes the isBound() accessor to include
            // extends Object bounds. This causes gratuitous differences
            // in documentation and tests.
            if (wild.kind == BoundKind.UNBOUND) {
              typeArgs.addChildBuilder().setPreText(wild.kind.toString());
            } else {
              MarkedSource.Builder boundedWild = typeArgs.addChildBuilder();
              boundedWild.addChildBuilder().setPreText(wild.kind.toString());
              addClassIdentifier(wild.type, boundedWild, signatureGenerator, symNames);
            }
            break;
          default:
            typeArgs.addChildBuilder().setPreText(arg.toString());
        }
      }
    }
  }

  /**
   * The only place the integer index for nested classes/anonymous classes is stored is in the
   * flatname of the symbol. (This index is determined at compile time using linear search; see
   * 'localClassName' in Check.java). The simple name can't be relied on; for nested classes it
   * drops the name of the parent class (so 'pkg.OuterClass$Inner' yields only 'Inner') and for
   * anonymous classes it's blank. For multiply-nested classes, we'll see tokens like
   * 'OuterClass$Inner$1$1'.
   */
  private static String getIdentToken(Symbol sym, SignatureGenerator signatureGenerator) {
    // If the symbol represents the generated `Array` class, replace it with the actual
    // array type, if we have it.
    if (SignatureGenerator.isArrayHelperClass(sym) && signatureGenerator != null) {
      return signatureGenerator.getArrayTypeName();
    }
    String flatName = sym.flatName().toString();
    int lastDot = flatName.lastIndexOf('.');
    // A$1 is a valid variable/method name, so make sure we only look at $ in class names.
    int lastCash = (sym instanceof ClassSymbol) ? flatName.lastIndexOf('$') : -1;
    int lastTok = Math.max(lastDot, lastCash);
    String identToken = lastTok < 0 ? flatName : flatName.substring(lastTok + 1);
    if (!identToken.isEmpty() && Character.isDigit(identToken.charAt(0))) {
      if (sym.name.isEmpty()) {
        identToken = "(anon " + identToken + ")";
      } else {
        identToken = sym.name.toString();
      }
    }
    return identToken;
  }

  /**
   * Returns the Symbol for sym's parent in qualified names, assuming that we'll be using
   * getIdentToken() to print nodes.
   *
   * <p>We're going through this extra effort to try and give people unsurprising qualified names.
   * To do that we have to deal with javac's mangling (in {@link #getIdentToken} above), since for
   * anonymous classes javac only stores mangled symbols. The code as written will emit only dotted
   * fully-qualified names, even for inner or anonymous classes, and considers concrete type,
   * package, or method names to be appropriate dot points. (If we weren't careful here we might,
   * for example, observe nodes in a qualified name corresponding to variables that are initialized
   * to anonymous classes.) This reflects the nesting structure from the Java side, not the JVM
   * side.
   */
  private static @Nullable Symbol getQualifiedNameParent(Symbol sym) {
    sym = sym.owner;
    while (sym != null) {
      switch (sym.kind) {
        case TYP:
          if (!sym.type.hasTag(TypeTag.TYPEVAR)) {
            return sym;
          }
          break;
        case PCK:
        case MTH:
          return sym;
          // TODO(#1845): resolve non-exhaustive switch statements w/o defaults
        default:
          break;
      }
      sym = sym.owner;
    }
    return null;
  }
}
