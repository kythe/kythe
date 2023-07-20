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

package com.google.devtools.kythe.analyzers.base;

import com.google.devtools.kythe.util.schema.Schema;

/** Schema-defined Kythe edge kinds. */
public enum EdgeKind {
  DEFINES(true, "defines"),
  DEFINES_BINDING(true, "defines/binding"),
  DEFINES_IMPLICIT(true, "defines/implicit"),
  DOCUMENTS(true, "documents"),
  TAGGED(true, "tagged"),
  UNDEFINES(true, "undefines"),

  // Edges from an anchor.
  IMPUTES(true, "imputes"),
  REF(true, "ref"),
  REF_CALL(true, "ref/call"),
  REF_CALL_DIRECT(true, "ref/call/direct"),
  REF_CALL_IMPLICIT(true, "ref/call/implicit"),
  REF_DOC(true, "ref/doc"),
  REF_EXPANDS(true, "ref/expands"),
  REF_EXPANDS_TRANSITIVE(true, "ref/expands/transitive"),
  REF_FILE(true, "ref/file"),
  REF_ID(true, "ref/id"),
  REF_IMPLICIT(true, "ref/implicit"),
  REF_IMPORTS(true, "ref/imports"),
  REF_INCLUDES(true, "ref/includes"),
  REF_INIT(true, "ref/init"),
  REF_INIT_IMPLICIT(true, "ref/init/implicit"),
  REF_QUERIES(true, "ref/queries"),
  REF_WRITES(true, "ref/writes"),

  ALIASES("aliases"),
  ALIASES_ROOT("aliases/root"),
  ANNOTATED_BY("annotatedby"),
  BOUNDED_LOWER("bounded/lower"),
  BOUNDED_UPPER("bounded/upper"),
  CHILDOF("childof"),
  CHILDOF_CONTEXT("childof/context"),
  DEPENDS("depends"),
  EXPORTS("exports"),
  EXTENDS("extends"),
  GENERATES("generates"),
  INSTANTIATES("instantiates"),
  INSTANTIATES_SPECULATIVE("instantiates/speculative"),
  NAMED("named"),
  OVERRIDES("overrides"),
  OVERRIDES_ROOT("overrides/root"),
  OVERRIDES_TRANSITIVE("overrides/transitive"),
  PARAM("param"),
  PROPERTY_READS("property/reads"),
  PROPERTY_WRITES("property/writes"),
  SATISFIES("satisfies"),
  SPECIALIZES("specializes"),
  SPECIALIZES_SPECULATIVE("specializes/speculative"),
  TPARAM("tparam"),
  TYPED("typed");

  private static final String EDGE_PREFIX = "/kythe/edge/";

  private final boolean isAnchorEdge;
  private final String kind;

  EdgeKind(boolean isAnchorEdge, String kind) {
    this.isAnchorEdge = isAnchorEdge;
    this.kind = EDGE_PREFIX + kind;
  }

  EdgeKind(String kind) {
    this(false, kind);
  }

  /** Returns {@code true} if the edge is used for {@link NodeKind.ANCHOR}s. */
  public final boolean isAnchorEdge() {
    return isAnchorEdge;
  }

  /** Returns {@code true} if this edge kind is a variant of {@code k}. */
  public final boolean isVariant(EdgeKind k) {
    return this == k || getValue().startsWith(k.getValue() + "/");
  }

  /** Returns the edge kind's Kythe GraphStore value. */
  public final String getValue() {
    return kind;
  }

  /** Returns the edge kind's proto enum value. */
  public final com.google.devtools.kythe.proto.Schema.EdgeKind getProtoEdgeKind() {
    return Schema.edgeKind(getValue());
  }

  @Override
  public String toString() {
    return kind;
  }
}
