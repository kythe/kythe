/*
 * Copyright 2015 Google Inc. All rights reserved.
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
import java.util.HashSet;
import java.util.Set;

/** Utility methods for javac-based analyses. */
public class JavacUtil {
  private JavacUtil() {}

  /** Returns the transitive set of super methods for {@code methodSymbol}. */
  public static Set<MethodSymbol> superMethods(Context context, MethodSymbol methodSymbol) {
    Types types = Types.instance(context);

    Set<MethodSymbol> supers = new HashSet<>();
    if (!methodSymbol.isStatic()) {
      TypeSymbol owner = (TypeSymbol) methodSymbol.owner;
      // Iterates over the list of all super classes and interfaces
      for (Type sup : types.closure(owner.type)) {
        if (sup == owner.type) {
          continue; // Skip the owner of the method
        }

        Scope scope = sup.tsym.members();
        for (Symbol sym : scope.getSymbolsByName(methodSymbol.name)) {
          if (sym != null && !sym.isStatic() && methodSymbol.overrides(sym, owner, types, true)) {
            supers.add((MethodSymbol) sym);
            break;
          }
        }
      }
    }
    return supers;
  }

  /** Searches in the symbol table to find a class with a particular name. */
  public static ClassSymbol getClassSymbol(Context context, String name) {
    Symtab symtab = Symtab.instance(context);
    Names names = Names.instance(context);

    int len = name.length();
    char[] nameChars = name.toCharArray();
    int dotIndex = len;
    while (true) {
      ClassSymbol s = symtab.classes.get(names.fromChars(nameChars, 0, len));
      if (s != null) {
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
