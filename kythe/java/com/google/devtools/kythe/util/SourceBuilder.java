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

package com.google.devtools.kythe.util;

import com.google.common.collect.Sets;
import com.google.devtools.kythe.proto.Internal.Source;
import com.google.devtools.kythe.proto.Storage.Entry;
import com.google.devtools.kythe.proto.Storage.VName;
import com.google.protobuf.ByteString;
import java.util.Comparator;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * Accumulates facts and edges from {@link Entry} messages sharing the same source {@link VName}
 * into a {@link Source} message.
 */
public class SourceBuilder {
  private static final Pattern ORDINAL_EDGE_KIND_PATTERN = Pattern.compile("^(.+)\\.(\\d+)$");
  private static final Comparator<Source.Edge> EDGE_ORDER =
      Comparator.comparing(Source.Edge::getOrdinal).thenComparing(Source.Edge::getTicket);

  private final Map<String, ByteString> facts = new HashMap<>();
  private final Map<String, Set<Source.Edge>> edgeGroups = new HashMap<>();

  /** Constructs an empty {@link SourceBuilder}. */
  public SourceBuilder() {}

  /**
   * Constructs a {@link SourceBuilder} initialized with the edges and facts from the given {@link
   * Source}.
   */
  public SourceBuilder(Source src) {
    src.getFactsMap().forEach(facts::put);
    src.getEdgeGroupsMap()
        .forEach((kind, edges) -> edgeGroups.put(kind, Sets.newHashSet(edges.getEdgesList())));
  }

  /**
   * Add a single fact to {@link Source} being built. If two facts are given with the same {@code
   * name}, only the latter fact value is kept.
   */
  public SourceBuilder addFact(String name, ByteString value) {
    facts.put(name, value);
    return this;
  }

  /**
   * Add a single edge to {@link Source} being built. If the edge has already been added, this is a
   * no-op.
   */
  public SourceBuilder addEdge(String edgeKind, VName target) {
    Source.Edge.Builder edge = Source.Edge.newBuilder().setTicket(KytheURI.asString(target));
    Matcher m = ORDINAL_EDGE_KIND_PATTERN.matcher(edgeKind);
    if (m.matches()) {
      edgeKind = m.group(1);
      try {
        edge.setOrdinal(Integer.parseInt(m.group(2)));
      } catch (NumberFormatException nfe) {
        throw new IllegalStateException(nfe);
      }
    }
    edgeGroups.computeIfAbsent(edgeKind, k -> new HashSet<>()).add(edge.build());
    return this;
  }

  /**
   * Adds the given {@link Entry} fact or edge to the {@link Source} being built. The source {@link
   * VName} of the {@link Entry} is ignored and assumed to be the same amongst all {@link Entry}
   * messages being added.
   *
   * @see addFact(String, ByteString)
   * @see addEdge(String, VName)
   */
  public SourceBuilder addEntry(Entry e) {
    if (e.getEdgeKind().isEmpty()) {
      return addFact(e.getFactName(), e.getFactValue());
    } else {
      return addEdge(e.getEdgeKind(), e.getTarget());
    }
  }

  /** Returns the accumulated {@link Source}. */
  public Source build() {
    Source.Builder builder = Source.newBuilder();
    facts.forEach(builder::putFacts);
    edgeGroups.forEach(
        (kind, edges) -> {
          Source.EdgeGroup.Builder group = Source.EdgeGroup.newBuilder();
          edges.stream().sorted(EDGE_ORDER).forEach(group::addEdges);
          builder.putEdgeGroups(kind, group.build());
        });
    return builder.build();
  }

  /** Adds all facts and edges from the given {@link SourceBuilder} into {@code this}. */
  public void mergeWith(SourceBuilder o) {
    this.facts.putAll(o.facts);
    o.edgeGroups.forEach(
        (kind, edges) -> this.edgeGroups.computeIfAbsent(kind, k -> new HashSet<>()).addAll(edges));
  }
}
