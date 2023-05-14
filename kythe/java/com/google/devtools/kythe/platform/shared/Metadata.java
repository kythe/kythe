/*
 * Copyright 2016 The Kythe Authors. All rights reserved.
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

package com.google.devtools.kythe.platform.shared;

import com.google.common.base.Preconditions;
import com.google.common.collect.ListMultimap;
import com.google.common.collect.MultimapBuilder;
import com.google.devtools.kythe.analyzers.base.EdgeKind;
import com.google.devtools.kythe.proto.Storage.VName;
import com.google.protobuf.DescriptorProtos.GeneratedCodeInfo.Annotation.Semantic;
import java.util.ArrayList;
import java.util.List;

/**
 * Metadata provides rules for emitting new edges when edges of certain kinds are emitted. An
 * instance only applies to a particular source file. Metadata is loaded by a {@link MetadataLoader}
 * and is associated with a source file through a javax.annotation.Generated annotation. This
 * annotation's "comments" field must be ANNOTATION_COMMENT_PREFIX followed by the .java- relative
 * path to a loadable metadata file.
 */
public class Metadata {
  /**
   * A Rule can generate one additional edge based on input conditions.
   *
   * <p>begin/end are ignored for file-scoped rules.
   */
  public static class Rule {
    /** The starting byte offset in the source file. */
    public int begin;
    /** The ending byte offset in the source file. */
    public int end;
    /** The VName to emit if the rule matches. */
    public VName vname;
    /** The edge kind to emit if the rule matches. */
    public EdgeKind edgeOut;
    /**
     * If false, draw the edge to the VName; if true, draw it from the VName. For example, if we
     * emit an {@code Anchor defines/binding Node} edge, our range matches the anchor's, and
     * reverseEdge is true, then we will emit {@code vname edgeOut Node}. If reverseEdge is false,
     * we will emit {@code Node edgeOut vname}.
     */
    public boolean reverseEdge;

    public Semantic semantic;
  }

  /** Applies a new {@link Rule} to the file to which this metadata pertains. */
  public void addRule(Rule rule) {
    Preconditions.checkNotNull(rule);
    rules.put(rule.begin, rule);
  }

  /** Applies a new file-scoped {@link Rule} to the file to which this metadata pertains. */
  public void addFileScopeRule(Rule rule) {
    Preconditions.checkNotNull(rule);
    fileRules.add(rule);
  }

  /**
   * Returns all rules with spans starting at that offset.
   *
   * @param location the starting byte offset in the source file to check.
   */
  public Iterable<Rule> getRulesForLocation(int location) {
    return rules.get(location);
  }

  /** Returns the file-scope rules */
  public Iterable<Rule> getFileScopeRules() {
    return fileRules;
  }

  /** All of the {@link Rule} instances, keyed on their starting offsets. */
  private final ListMultimap<Integer, Rule> rules =
      MultimapBuilder.treeKeys().arrayListValues().build();

  /** All of the file-scope rules */
  private final List<Rule> fileRules = new ArrayList<>();

  /**
   * A class's javax.annotation.Generated must have a comments field with this string as a prefix to
   * participate in metadata rules.
   */
  public static final String ANNOTATION_COMMENT_PREFIX = "annotations:";

  public static final String ANNOTATION_COMMENT_INLINE_METADATA_PREFIX = "kythe-inline-metadata:";
}
