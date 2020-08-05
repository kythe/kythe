/*
 * Copyright 2019 The Kythe Authors. All rights reserved.
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

package com.google.devtools.kythe.util.schema;

import static com.google.devtools.kythe.util.KytheURI.parseVName;

import com.google.devtools.kythe.proto.Internal.Source;
import com.google.devtools.kythe.proto.Schema.Edge;
import com.google.devtools.kythe.proto.Schema.EdgeKind;
import com.google.devtools.kythe.proto.Schema.Entry;
import com.google.devtools.kythe.proto.Schema.Fact;
import com.google.devtools.kythe.proto.Schema.FactName;
import com.google.devtools.kythe.proto.Schema.Node;
import com.google.devtools.kythe.proto.Schema.NodeKind;
import com.google.devtools.kythe.proto.Schema.Subkind;
import com.google.devtools.kythe.proto.Storage;
import com.google.devtools.kythe.proto.Storage.VName;
import com.google.devtools.kythe.util.KytheURI;
import com.google.protobuf.ByteString;
import java.util.Comparator;
import java.util.HashMap;
import java.util.Map;
import java.util.SortedSet;
import java.util.TreeSet;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/** Utility to convert to/from {@link Node}, {@link Entry}, and {@link Source} protos. */
public final class Nodes {
  private Nodes() {}

  private static final Pattern ORDINAL_EDGE_KIND_PATTERN = Pattern.compile("^(.+)\\.(\\d+)$");

  private static final Comparator<Source.Edge> SOURCE_EDGE_ORDER =
      Comparator.comparing(Source.Edge::getOrdinal).thenComparing(Source.Edge::getTicket);

  private static final Comparator<VName> VNAME_ORDER =
      Comparator.comparing(VName::getCorpus)
          .thenComparing(VName::getLanguage)
          .thenComparing(VName::getPath)
          .thenComparing(VName::getRoot)
          .thenComparing(VName::getSignature);

  /** Standard {@link Fact} ordering within a {@link Node}. */
  public static final Comparator<Fact> FACT_ORDER =
      Comparator.comparing(Fact::getGenericName).thenComparing(Fact::getKytheName);

  /** Standard {@link Edge} ordering within a {@link Node}. */
  public static final Comparator<Edge> EDGE_ORDER =
      Comparator.comparing(Edge::getGenericKind)
          .thenComparing(Edge::getKytheKind)
          .thenComparing(Edge::getOrdinal)
          .thenComparing(Edge::getTarget, VNAME_ORDER);

  /**
   * Normalizes a {@link Node} by converting all known generics to their corresponding {@link
   * NodeKind}, {@link Subkind}, {@link EdgeKind}, or {@link FactName} and ordering all facts/edges.
   *
   * @see FACT_ORDER
   * @see EDGE_ORDER
   */
  public static Node normalizeNode(Node n) {
    final Node.Builder b = n.toBuilder().clearFact().clearEdge();
    n.getEdgeList().stream()
        .map(
            e -> {
              Edge.Builder eb = e.toBuilder();
              EdgeKind k = Schema.edgeKind(e.getGenericKind());
              if (!k.equals(EdgeKind.UNKNOWN_EDGE_KIND)) {
                eb.setKytheKind(k);
              }
              return eb.clearSource().build();
            })
        .sorted(EDGE_ORDER)
        .forEach(b::addEdge);
    n.getFactList().stream()
        .map(
            f -> {
              Fact.Builder fb = f.toBuilder();
              FactName name = Schema.factName(f.getGenericName());
              if (!name.equals(FactName.UNKNOWN_FACT_NAME)) {
                fb.setKytheName(name);
              }
              return fb.clearSource().build();
            })
        .sorted(FACT_ORDER)
        .forEach(
            f -> {
              if (FactName.NODE_KIND.equals(f.getKytheName())) {
                String nodeKind = f.getValue().toStringUtf8();
                NodeKind kytheNodeKind = Schema.nodeKind(nodeKind);
                if (NodeKind.UNKNOWN_NODE_KIND.equals(kytheNodeKind)) {
                  b.setGenericKind(nodeKind);
                } else {
                  b.setKytheKind(kytheNodeKind);
                }
              } else if (FactName.SUBKIND.equals(f.getKytheName())) {
                String subkind = f.getValue().toStringUtf8();
                Subkind kytheSubkind = Schema.subkind(subkind);
                if (Subkind.UNKNOWN_SUBKIND.equals(kytheSubkind)) {
                  b.setGenericSubkind(subkind);
                } else {
                  b.setKytheSubkind(kytheSubkind);
                }
              } else {
                b.addFact(f);
              }
            });
    return b.build();
  }

  /** Returns the kind {@link String} of the given {@link Node}. */
  public static String getNodeKind(Node node) {
    if (!node.getKytheKind().equals(NodeKind.UNKNOWN_NODE_KIND)) {
      return Schema.nodeKindString(node.getKytheKind());
    }
    return node.getGenericKind();
  }

  /** Returns the subkind {@link String} of the given {@link Node}. */
  public static String getSubkind(Node node) {
    if (!node.getKytheSubkind().equals(Subkind.UNKNOWN_SUBKIND)) {
      return Schema.subkindString(node.getKytheSubkind());
    }
    return node.getGenericSubkind();
  }

  /** Converts a {@link Node} to a {@link Source}. */
  public static Source convertToSource(Node node) {
    Source.Builder src = Source.newBuilder().setTicket(KytheURI.asString(node.getSource()));
    String nodeKind = getNodeKind(node);
    if (!nodeKind.isEmpty()) {
      src.putFacts(Schema.factNameString(FactName.NODE_KIND), ByteString.copyFromUtf8(nodeKind));
    }
    String subkind = getSubkind(node);
    if (!subkind.isEmpty()) {
      src.putFacts(Schema.factNameString(FactName.SUBKIND), ByteString.copyFromUtf8(subkind));
    }
    for (Fact f : node.getFactList()) {
      src.putFacts(
          f.getKytheName().equals(FactName.UNKNOWN_FACT_NAME)
              ? f.getGenericName()
              : Schema.factNameString(f.getKytheName()),
          f.getValue());
    }
    Map<String, SortedSet<Source.Edge>> edgeGroups = new HashMap<>();
    for (Edge e : node.getEdgeList()) {
      edgeGroups
          .computeIfAbsent(
              e.getKytheKind().equals(EdgeKind.UNKNOWN_EDGE_KIND)
                  ? e.getGenericKind()
                  : Schema.edgeKindString(e.getKytheKind()),
              k -> new TreeSet<>(SOURCE_EDGE_ORDER))
          .add(
              Source.Edge.newBuilder()
                  .setTicket(KytheURI.asString(e.getTarget()))
                  .setOrdinal(e.getOrdinal())
                  .build());
    }
    edgeGroups.forEach(
        (kind, edges) ->
            src.putEdgeGroups(kind, Source.EdgeGroup.newBuilder().addAllEdges(edges).build()));
    return src.build();
  }

  /** Converts a {@link Source} to a {@link Node}. */
  public static Node fromSource(Source src) {
    Node.Builder node = Node.newBuilder().setSource(parseVName(src.getTicket()));
    for (Map.Entry<String, ByteString> fact : src.getFactsMap().entrySet()) {
      node.addFactBuilder().setGenericName(fact.getKey()).setValue(fact.getValue());
    }
    for (Map.Entry<String, Source.EdgeGroup> group : src.getEdgeGroupsMap().entrySet()) {
      for (Source.Edge edge : group.getValue().getEdgesList()) {
        node.addEdgeBuilder()
            .setGenericKind(group.getKey())
            .setTarget(parseVName(edge.getTicket()))
            .setOrdinal(edge.getOrdinal());
      }
    }
    return normalizeNode(node.build());
  }

  /** Converts a {@link Storage.Entry} to an {@link Entry}. */
  public static Entry convertToSchemaEntry(Storage.Entry entry) {
    Entry.Builder b = Entry.newBuilder();
    if (entry.getEdgeKind().isEmpty()) {
      Fact.Builder f =
          b.getFactBuilder().setSource(entry.getSource()).setValue(entry.getFactValue());
      FactName kytheName = Schema.factName(entry.getFactName());
      if (FactName.UNKNOWN_FACT_NAME.equals(kytheName)) {
        f.setGenericName(entry.getFactName());
      } else {
        f.setKytheName(kytheName);
      }
    } else {
      Edge.Builder e = b.getEdgeBuilder().setSource(entry.getSource()).setTarget(entry.getTarget());
      Matcher m = ORDINAL_EDGE_KIND_PATTERN.matcher(entry.getEdgeKind());
      String kind = m.matches() ? m.group(1) : entry.getEdgeKind();
      if (m.matches()) {
        try {
          e.setOrdinal(Integer.parseInt(m.group(2)));
        } catch (NumberFormatException nfe) {
          throw new IllegalStateException(nfe);
        }
      }
      EdgeKind kytheKind = Schema.edgeKind(kind);
      if (EdgeKind.UNKNOWN_EDGE_KIND.equals(kytheKind)) {
        e.setGenericKind(kind);
      } else {
        e.setKytheKind(kytheKind);
      }
    }
    return b.build();
  }
}
