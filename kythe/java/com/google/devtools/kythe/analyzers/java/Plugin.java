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

import com.google.devtools.kythe.analyzers.base.KytheEntrySets;
import com.google.devtools.kythe.platform.java.helpers.JCTreeScanner;
import com.google.devtools.kythe.proto.Storage.VName;
import com.sun.tools.javac.tree.JCTree;
import com.sun.tools.javac.tree.JCTree.JCCompilationUnit;
import java.util.Map;
import java.util.Optional;

/** Plugin interface for the Kythe Java analyzer. */
public interface Plugin {
  /** Interface to access the Kythe graph from the Java AST. */
  public static final class KytheGraph {
    private final Map<JCTree, KytheNode> treeNodes;

    KytheGraph(Map<JCTree, KytheNode> treeNodes) {
      this.treeNodes = treeNodes;
    }

    /** Returns the {@link KytheNode} associated with the given {@link JCTree}. */
    public Optional<KytheNode> getNode(JCTree tree) {
      return Optional.ofNullable(treeNodes.get(tree));
    }
  }

  /** Node in the Kythe graph emitted by the Kythe Java analyzer. */
  public static final class KytheNode {
    private final VName vName;

    KytheNode(VName vName) {
      this.vName = vName;
    }

    /** Returns the {@link VName} associated with the given Kythe node. */
    public VName getVName() {
      return vName;
    }
  }

  /** Execute the {@link Plugin}'s analysis over a {@link JCCompilationUnit}. */
  public void run(JCCompilationUnit compilation, KytheEntrySets entrySets, KytheGraph kytheGraph);

  /** {@link Plugin} that scans the entire {@link JCCompilationUnit} AST. */
  public static class Scanner<R, P> extends JCTreeScanner<R, P> implements Plugin {
    protected KytheGraph kytheGraph;

    @Override
    public void run(
        JCCompilationUnit compilation, KytheEntrySets entrySets, KytheGraph kytheGraph) {
      this.kytheGraph = kytheGraph;
      compilation.accept(this, null);
    }
  }

  /**
   * Example {@link Plugin} that prints each {@link JCTree} tag with its associated {@link VName}.
   */
  public static final class PrintKytheNodes extends Scanner<Void, Void> {
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
