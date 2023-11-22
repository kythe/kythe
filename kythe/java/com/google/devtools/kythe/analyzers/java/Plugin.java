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

import com.google.devtools.kythe.analyzers.base.KytheEntrySets;
import com.google.devtools.kythe.platform.java.helpers.JCTreeScanner;
import com.google.devtools.kythe.proto.Storage.VName;
import com.google.devtools.kythe.util.Span;
import com.sun.tools.javac.code.Symbol;
import com.sun.tools.javac.tree.JCTree;
import com.sun.tools.javac.tree.JCTree.JCCompilationUnit;
import com.sun.tools.javac.util.Context;
import java.util.Optional;
import javax.lang.model.element.Name;
import javax.tools.JavaFileObject;

/** Plugin interface for the Kythe Java analyzer. */
public interface Plugin {
  /** Interface to access the Kythe graph from the Java AST. */
  public static interface KytheGraph {
    /** Returns the current Java {@link Context}. */
    public Context getJavaContext();

    /** Returns the {@link KytheNode} associated with the given {@link JavaFileObject}. */
    public Optional<KytheNode> getNode(JavaFileObject file);

    /** Returns the {@link KytheNode} associated with the given {@link JCTree}. */
    public Optional<KytheNode> getNode(JCTree tree);

    /** Returns the {@link KytheNode} associated with the given {@link Symbol}. */
    public Optional<KytheNode> getNode(Symbol sym);

    /** Returns the JVM {@link KytheNode} associated with given {@link Symbol}. */
    public Optional<KytheNode> getJvmNode(Symbol sym);

    // TODO(schroederc): should there be a getNode(Type) method as well?

    /** Returns the {@link Span} for the given {@link JCTree}. */
    public Optional<Span> getSpan(JCTree tree);

    /**
     * Returns the {@link Span} for the first known occurrence of the specified {@link Name}d
     * identifier, starting at or after the specified starting offset.
     */
    public Optional<Span> findIdentifier(Name name, int startOffset);
  }

  /** Node in the Kythe graph emitted by the Kythe Java analyzer. */
  public static interface KytheNode {
    /** Returns the {@link VName} associated with the given Kythe node. */
    public VName getVName();
  }

  /** Return a human-friendly name for this plugin to be used in debug outputs. */
  public String getName();

  /** Execute the {@link Plugin}'s analysis over a {@link JCCompilationUnit}. */
  public void run(JCCompilationUnit compilation, KytheEntrySets entrySets, KytheGraph kytheGraph);

  /** {@link Plugin} that scans the entire {@link JCCompilationUnit} AST. */
  public abstract static class Scanner<R, P> extends JCTreeScanner<R, P> implements Plugin {
    protected KytheGraph kytheGraph;
    protected KytheEntrySets entrySets;

    @Override
    public void run(
        JCCompilationUnit compilation, KytheEntrySets entrySets, KytheGraph kytheGraph) {
      this.kytheGraph = kytheGraph;
      this.entrySets = entrySets;
      compilation.accept(this, null);
    }
  }

  /**
   * Example {@link Plugin} that prints each {@link JCTree} tag with its associated {@link VName}.
   */
  public static final class PrintKytheNodes extends Scanner<Void, Void> {
    @Override
    public String getName() {
      return "print_kythe_nodes";
    }

    @Override
    public Void scan(JCTree tree, Void v) {
      if (tree != null) {
        System.err.println(
            tree.getTag()
                + " ->  "
                + kytheGraph
                    .getNode(tree)
                    .map(k -> "{" + k.getVName().toString().replace("\n", " ").trim() + "}"));
      }
      return super.scan(tree, v);
    }
  }
}
