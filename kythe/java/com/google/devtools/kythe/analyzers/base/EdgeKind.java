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

package com.google.devtools.kythe.analyzers.base;

/** Schema-defined Kythe edge kinds. */
public enum EdgeKind {
  DEFINES(true, "defines"),
  DEFINES_BINDING(true, "defines/binding"),
  DOCUMENTS(true, "documents"),

  REF(true, "ref"),
  REF_CALL(true, "ref/call"),
  REF_DOC(true, "ref/doc"),
  REF_IMPORTS(true, "ref/imports"),

  ANNOTATED_BY("annotatedby"),
  BOUNDED_LOWER("bounded/lower"),
  BOUNDED_UPPER("bounded/upper"),
  CHILDOF("childof"),
  EXTENDS("extends"),
  NAMED("named"),
  OVERRIDES("overrides"),
  OVERRIDES_TRANSITIVE("overrides/transitive"),
  PARAM("param"),
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

  /** Returns the edge kind's Kythe GraphStore value. */
  public final String getValue() {
    return kind;
  }

  @Override
  public String toString() {
    return kind;
  }
}
