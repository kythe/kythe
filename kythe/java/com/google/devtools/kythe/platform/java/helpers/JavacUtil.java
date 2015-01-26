package com.google.devtools.kythe.platform.java.helpers;

import com.sun.tools.javac.code.Scope;
import com.sun.tools.javac.code.Symbol.MethodSymbol;
import com.sun.tools.javac.code.Symbol.TypeSymbol;
import com.sun.tools.javac.code.Type;
import com.sun.tools.javac.code.Types;
import com.sun.tools.javac.util.Context;

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
        for (Scope.Entry e = scope.lookup(methodSymbol.name); e.scope != null; e = e.next()) {
          if (e.sym != null
              && !e.sym.isStatic() && methodSymbol.overrides(e.sym, owner, types, true)) {
            supers.add((MethodSymbol) e.sym);
            break;
          }
        }
      }
    }
    return supers;
  }
}
