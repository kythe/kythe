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

import com.sun.source.tree.CompilationUnitTree;
import com.sun.source.tree.Tree;
import com.sun.source.util.TreePath;
import com.sun.source.util.TreeScanner;
import com.sun.tools.javac.model.JavacElements;
import com.sun.tools.javac.tree.JCTree;
import com.sun.tools.javac.tree.JCTree.JCCompilationUnit;
import com.sun.tools.javac.util.Context;
import com.sun.tools.javac.util.Pair;
import java.util.HashMap;
import java.util.Map;
import javax.lang.model.element.Element;

/**
 * This class memoizes the TreePath(s) in a compilation unit tree. Each node in the tree has one
 * associated tree path. Normally, using the {@code JavacTrees} class, a new TreePath object gets
 * created every time there is a call for getting the tree path to an element. In addition this
 * process is inefficient in the sense that every time it traverses the compilation tree until it
 * reaches the target element.
 *
 * <p>This class, however, traverses the whole tree once and record the tree path associated with
 * each tree element. Clients of this class can get tree paths in O(1) time. The memory used for
 * memoization is O(number of nodes(compilation tree)).
 */
public class MemoizedTreePathScanner extends TreeScanner<Void, TreePath> {
  private final Map<Tree, TreePath> paths = new HashMap<>();
  private final JavacElements elements;

  /** @param unit the compilation unit for which we want to memoize the tree paths */
  public MemoizedTreePathScanner(CompilationUnitTree unit, Context context) {
    elements = JavacElements.instance(context);
    // Constructing the tree path for compilation unit.
    TreePath path = new TreePath(unit);
    paths.put(unit, path);
    // Traversing the whole compilation unit tree.
    unit.accept(this, path);
  }

  /** Scans a single node. The current path is updated for the duration of the scan. */
  @Override
  public Void scan(Tree tree, TreePath parent) {
    if (tree == null) {
      return null;
    }
    TreePath path = new TreePath(parent, tree);
    // Recording the path to the current element in the tree.
    paths.put(tree, path);
    // Traversing the children.
    super.scan(tree, path);
    return null;
  }

  /** Returns the memoized path to the {@code tree} node */
  public TreePath getPath(Tree tree) {
    return paths.get(tree);
  }

  /** Returns the memoized path to the {@code element} */
  public TreePath getPath(Element element) {
    Pair<JCTree, JCCompilationUnit> p = elements.getTreeAndTopLevel(element, null, null);
    if (p != null) {
      return getPath(p.fst);
    } else {
      return null;
    }
  }
}
