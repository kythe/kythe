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

import com.google.common.collect.ImmutableList;
import com.google.devtools.kythe.analyzers.base.EntrySet;
import com.google.devtools.kythe.proto.Storage.VName;
import com.sun.tools.javac.code.Symbol;
import java.util.List;
import java.util.Optional;

/** Kythe graph node representing a Java language construct. */
class JavaNode {
  // TODO(schroederc): handle cases where a single JCTree corresponds to multiple Kythe nodes
  // (i.e. types and abstractions)
  private final VName vName;
  private JavaNode type;
  private Symbol sym;

  private ImmutableList<VName> classConstructors = ImmutableList.of();
  private Optional<VName> classInit = Optional.empty();

  // I think order matters for the wildcards because the abs node will be connected to them with
  // param edges, which are numbered. If order doesn't matter, we should change this to something
  // like bazel's NestedSet.
  /**
   * The full list of wildcards that are parented by this node. This includes all wildcards that
   * directly belong to this node, and all wildcards that belong to children of this node.
   */
  final ImmutableList<VName> childWildcards;

  JavaNode(EntrySet entries) {
    this(entries, null);
  }

  JavaNode(VName vName) {
    this(vName, null);
  }

  JavaNode(EntrySet entries, ImmutableList<VName> childWildcards) {
    this(entries.getVName(), childWildcards);
  }

  JavaNode(VName vName, ImmutableList<VName> childWildcards) {
    this.vName = vName;
    this.childWildcards = childWildcards == null ? ImmutableList.<VName>of() : childWildcards;
  }

  VName getVName() {
    return vName;
  }

  JavaNode setType(JavaNode type) {
    this.type = type;
    return this;
  }

  JavaNode getType() {
    return type;
  }

  JavaNode setSymbol(Symbol sym) {
    this.sym = sym;
    return this;
  }

  Symbol getSymbol() {
    return sym;
  }

  JavaNode setClassConstructors(List<VName> constructors) {
    this.classConstructors = ImmutableList.copyOf(constructors);
    return this;
  }

  ImmutableList<VName> getClassConstructors() {
    return classConstructors;
  }

  JavaNode setClassInit(VName classInit) {
    this.classInit = Optional.ofNullable(classInit);
    return this;
  }

  Optional<VName> getClassInit() {
    return classInit;
  }
}
