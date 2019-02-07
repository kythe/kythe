/*
 * Copyright 2015 The Kythe Authors. All rights reserved.
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

import com.google.common.collect.ImmutableList;
import com.sun.tools.javac.code.Scope;
import com.sun.tools.javac.code.Symbol;
import com.sun.tools.javac.code.Symbol.ClassSymbol;
import com.sun.tools.javac.code.Symbol.MethodSymbol;
import com.sun.tools.javac.code.Symbol.TypeSymbol;
import com.sun.tools.javac.code.Symtab;
import com.sun.tools.javac.code.Type;
import com.sun.tools.javac.code.Types;
import com.sun.tools.javac.util.Context;
import com.sun.tools.javac.util.Names;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;
import java.util.function.BiConsumer;

/** Utility methods for javac-based analyses. */
public class JavacUtil {
  private JavacUtil() {}

  /** Kind of method override. */
  public static enum OverrideKind {
    DIRECT,
    INDIRECT;
  }

  /** Visit all super methods for the given {@link MethodSymbol}. */
  public static void visitSuperMethods(
      Context context, MethodSymbol methodSymbol, BiConsumer<MethodSymbol, OverrideKind> consumer) {
    if (methodSymbol.isStatic()) {
      return;
    }
    Types types = Types.instance(context);
    Set<MethodSymbol> directOverrides = new HashSet<>();
    Set<MethodSymbol> indirectOverrides = new HashSet<>();
    TypeSymbol owner = (TypeSymbol) methodSymbol.owner;

    // Follow each direct supertype's inheritance tree, finding each method override.  The first
    // method override in each tree is considered DIRECT with the rest being INDIRECT.
    for (Type directSuperType : types.directSupertypes(owner.type)) {
      boolean direct = true;
      List<Type> superTypes = ImmutableList.of(directSuperType);
      while (!superTypes.isEmpty()) {
        List<Type> nextLevel = new ArrayList<>();
        for (Type superType : superTypes) {
          nextLevel.addAll(types.directSupertypes(superType));
          Scope scope = superType.tsym.members();
          for (Symbol sym : scope.getSymbolsByName(methodSymbol.name)) {
            if (sym != null && !sym.isStatic() && methodSymbol.overrides(sym, owner, types, true)) {
              (direct ? directOverrides : indirectOverrides).add((MethodSymbol) sym);
              direct = false;
              break;
            }
          }
        }
        superTypes = nextLevel;
      }
    }

    // Due to multiple inheritance with interfaces, a method could be found to be both a DIRECT and
    // INDIRECT override.  We choose to consider these methods only DIRECT.
    indirectOverrides.removeAll(directOverrides);
    for (MethodSymbol sym : directOverrides) {
      consumer.accept(sym, OverrideKind.DIRECT);
    }
    for (MethodSymbol sym : indirectOverrides) {
      consumer.accept(sym, OverrideKind.INDIRECT);
    }
  }

  /** Searches in the symbol table to find a class with a particular name. */
  public static ClassSymbol getClassSymbol(Context context, String name) {
    Symtab symtab = Symtab.instance(context);
    Names names = Names.instance(context);

    int len = name.length();
    char[] nameChars = name.toCharArray();
    int dotIndex = len;
    while (true) {
      for (ClassSymbol s : symtab.getClassesForName(names.fromChars(nameChars, 0, len))) {
        // Return first candidate found.
        // TODO(schroederc): filter by module or return all candidates
        return s;
      }
      dotIndex = name.substring(0, dotIndex).lastIndexOf('.');
      if (dotIndex < 0) {
        break;
      }
      nameChars[dotIndex] = '$';
    }
    return null;
  }
}
