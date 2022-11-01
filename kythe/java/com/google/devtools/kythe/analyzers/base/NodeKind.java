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
import java.util.Optional;
import org.checkerframework.checker.nullness.qual.Nullable;

/** Schema-defined Kythe node kinds. */
public enum NodeKind {
  // Core kinds
  ANCHOR("anchor"),
  CONSTANT("constant"),
  DIAGNOSTIC("diagnostic"),
  DOC("doc"),
  FILE("file"),
  FUNCTION("function"),
  INTERFACE("interface"),
  NAME("name"),
  PACKAGE("package"),
  PROCESS("process"),
  SYMBOL("symbol"),
  TALIAS("talias"),
  TAPPLY("tapp"),
  TBUILTIN("tbuiltin"),
  TVAR("tvar"),

  // Sub-kinds
  ANCHOR_IMPLICIT("anchor", "implicit"),
  FUNCTION_CONSTRUCTOR("function", "constructor"),
  RECORD_CLASS("record", "class"),
  SUM_ENUM_CLASS("sum", "enumClass"),
  VARIABLE_EXCEPTION("variable", "local/exception"),
  VARIABLE_FIELD("variable", "field"),
  VARIABLE_LOCAL("variable", "local"),
  VARIABLE_PARAMETER("variable", "local/parameter"),
  VARIABLE_RESOURCE("variable", "local/resource");

  private final String kind;
  private final @Nullable String subkind;

  NodeKind(String kind) {
    this(kind, null);
  }

  NodeKind(String kind, @Nullable String subkind) {
    this.kind = kind;
    this.subkind = subkind;
  }

  /** Returns the node's kind Kythe GraphStore value. */
  public final String getKind() {
    return kind;
  }

  /** Returns the node's subkind Kythe GraphStore value. */
  public final Optional<String> getSubkind() {
    return Optional.ofNullable(subkind);
  }

  /** Returns the node kind's proto enum value. */
  public final com.google.devtools.kythe.proto.Schema.NodeKind getProtoNodeKind() {
    return Schema.nodeKind(getKind());
  }

  /** Returns the node subkind's proto enum value. */
  public final Optional<com.google.devtools.kythe.proto.Schema.Subkind> getProtoSubkind() {
    return getSubkind().map(Schema::subkind);
  }

  @Override
  public String toString() {
    return kind + (subkind == null ? "" : "/" + subkind);
  }
}
