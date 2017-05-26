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

import com.google.common.collect.ImmutableList;
import com.google.devtools.kythe.analyzers.base.EntrySet;

/** Kythe graph node representing a Java language construct. */
class JavaNode {
  // TODO(schroederc): clearly separate semantic/type nodes
  final EntrySet entries;
  final JavaNode typeNode;
  final String qualifiedName;

  // I think order matters for the wildcards because the abs node will be connected to them with
  // param edges, which are numbered. If order doesn't matter, we should change this to something
  // like bazel's NestedSet.
  /**
   * The full list of wildcards that are parented by this node. This includes all wildcards that
   * directly belong to this node, and all wildcards that belong to children of this node.
   */
  final ImmutableList<EntrySet> childWildcards;

  JavaNode(EntrySet entries, String qualifiedName) {
    this(entries, qualifiedName, null, ImmutableList.<EntrySet>of());
  }

  JavaNode(EntrySet entries, String qualifiedName, ImmutableList<EntrySet> childWildcards) {
    this(entries, qualifiedName, null, childWildcards);
  }

  JavaNode(EntrySet entries, String qualifiedName, JavaNode typeNode) {
    this(
        entries,
        qualifiedName,
        typeNode,
        typeNode != null ? typeNode.childWildcards : ImmutableList.<EntrySet>of());
  }

  JavaNode(
      EntrySet entries,
      String qualifiedName,
      JavaNode typeNode,
      ImmutableList<EntrySet> childWildcards) {
    this.entries = entries;
    this.qualifiedName = qualifiedName;
    this.typeNode = typeNode;
    this.childWildcards = childWildcards;
  }

  @Override
  public String toString() {
    return "JavaNode{" + qualifiedName + "}";
  }
}
