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

import com.google.common.base.Preconditions;
import com.google.common.collect.ImmutableMap;
import com.google.devtools.kythe.platform.shared.StatisticsCollector;
import com.google.devtools.kythe.proto.Analysis.CompilationUnit.FileInput;
import com.google.devtools.kythe.proto.Storage.VName;
import com.google.devtools.kythe.util.KytheURI;

import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;

/**
 * Factory for Kythe-compliant node and edge {@link EntrySet}s. In general, this class provides two
 * sets of methods: low-level node/edge builders and higher-level abstractions for specific kinds of
 * nodes/edges such as ANCHOR nodes and their edges.  Each higher-level abstraction returns a fully
 * realized {@link EntrySet} and automatically emits it (and any associated edges).  The lower-level
 * methods, such as {@link #newNode(String)} return {@link EntrySet.Builder}s and must be emitted by
 * the client.
 *
 * This class is meant to be subclassed to build indexer-specific nodes and edges.
 */
public class KytheEntrySets {
  public static final String NODE_PREFIX = "/kythe/";

  private final Map<String, EntrySet> nameNodes = new HashMap<>();

  private final StatisticsCollector statistics;
  private final FactEmitter emitter;
  private final String language;

  private final VName compilationVName;
  private final Map<String, VName> inputVNames;

  public KytheEntrySets(StatisticsCollector statistics, FactEmitter emitter, VName compilationVName,
      List<FileInput> requiredInputs) {
    this.statistics = statistics;
    this.emitter = emitter;
    this.language = compilationVName.getLanguage();
    this.compilationVName = compilationVName;

    ImmutableMap.Builder<String, VName> inputVNames = ImmutableMap.builder();
    for (FileInput input : requiredInputs) {
      String digest = input.getInfo().getDigest();
      VName.Builder name = input.getVName().toBuilder();
      Preconditions.checkArgument(!name.getPath().isEmpty(),
          "Required input VName must have non-empty path");
      if (name.getSignature().isEmpty()) {
        // Ensure file VName has digest signature
        name = name.setSignature(digest);
      }
      inputVNames.put(digest, name.build());
    }
    this.inputVNames = inputVNames.build();
  }

  /** Returns the {@link FactEmitter} used to emit generated {@link EntrySet}s. */
  public final FactEmitter getEmitter() {
    return emitter;
  }

  /** Return the {@link StatisticsCollector} being used. */
  protected final StatisticsCollector getStatisticsCollector() {
    return statistics;
  }

  /**
   * Returns a new {@link NodeBuilder} with the given node kind set.
   *
   * @deprecated use {@link #newNode(NodeKind)} for schema-defined kinds
   */
  @Deprecated
  public NodeBuilder newNode(String kind) {
    getStatisticsCollector()
        .incrementCounter("deprecated-new-node-" + kind);
    return new NodeBuilder(kind, null, language);
  }

  /** Returns a new {@link NodeBuilder} with the given node kind set. */
  public NodeBuilder newNode(NodeKind kind) {
    getStatisticsCollector()
        .incrementCounter("new-node-" + kind);
    return new NodeBuilder(kind, language);
  }

  /** Returns (and emits) a new builtin node. */
  public EntrySet getBuiltin(String name) {
    return emitAndReturn(newNode(NodeKind.TBUILTIN)
        .setSignature(name + "#builtin"));
  }

  /** Returns (and emits) a new anchor node at the given location in the file. */
  public EntrySet getAnchor(VName fileVName, int start, int end) {
    if (start > end || start < 0) {
      // TODO(schroederc): reduce number of invalid anchors
      return null;
    }
    EntrySet anchor = newNode(NodeKind.ANCHOR)
        .setCorpusPath(CorpusPath.fromVName(fileVName))
        .addSignatureSalt(fileVName)
        .setProperty("loc/start", "" + start)
        .setProperty("loc/end", "" + end)
        .build();
    emitEdge(anchor.getVName(), EdgeKind.CHILDOF, fileVName);
    return emitAndReturn(anchor);
  }

  /** Returns and emits a NAME node. NAME nodes are cached so that they are only emitted once. */
  public EntrySet getName(String name) {
    EntrySet node = nameNodes.get(name);
    if (node == null) {
      node = emitAndReturn(newNode(NodeKind.NAME).setSignature(name));
      nameNodes.put(name, node);
    }
    return node;
  }

  /** Emits and returns a new {@link EntrySet} representing a file. */
  public EntrySet getFileNode(String digest, byte[] contents, String encoding) {
    VName name = lookupVName(digest);
    return emitAndReturn(newNode(NodeKind.FILE)
        .setCorpusPath(CorpusPath.fromVName(name))
        .setSignature(name.getSignature())
        .setProperty("text", contents)
        .setProperty("text/encoding", encoding));
  }

  /**
   * Returns a {@link NodeBuilder} with the given kind and added signature salts for each
   * {@link EntrySet} dependency.
   */
  public NodeBuilder newNode(NodeKind kind, Iterable<EntrySet> dependencies) {
    NodeBuilder builder = newNode(kind);
    for (EntrySet e : dependencies) {
      builder.addSignatureSalt(e.getVName());
    };
    return builder;
  }

  /** Emits an edge of the given kind from {@code source} to {@code target}. */
  public void emitEdge(EntrySet source, EdgeKind kind, EntrySet target) {
    Preconditions.checkNotNull(source, "source EntrySet must be non-null");
    Preconditions.checkNotNull(target, "target EntrySet must be non-null");
    emitEdge(source.getVName(), kind, target.getVName());
  }

  /** Emits an edge of the given kind from {@code source} to {@code target}. */
  public void emitEdge(VName source, EdgeKind kind, VName target) {
    getStatisticsCollector()
        .incrementCounter("emit-edge-" + kind);
    new EdgeBuilder(source, kind, target).build().emit(emitter);
  }

  /** Emits an edge of the given kind and ordinal from {@code source} to {@code target}. */
  public void emitEdge(EntrySet source, EdgeKind kind, EntrySet target, int ordinal) {
    getStatisticsCollector()
        .incrementCounter("emit-edge-" + kind);
    new EdgeBuilder(source.getVName(), kind, target.getVName())
        .setOrdinal(ordinal)
        .build()
        .emit(emitter);
  }

  /**
   * Emits edges of the given kind from {@code source} to each of the target {@link EntrySet}s, with
   * their respective {@link Iterable} order (0-based) as their edge ordinal.
   */
  public void emitOrdinalEdges(EntrySet source, EdgeKind kind, Iterable<EntrySet> targets) {
    emitOrdinalEdges(source, kind, targets, 0);
  }

  /**
   * Emits edges of the given kind from {@code source} to each of the target {@link EntrySet}s, with
   * their respective {@link Iterable} order as their edge ordinal.
   */
  public void emitOrdinalEdges(EntrySet source, EdgeKind kind, Iterable<EntrySet> targets,
      int startingOrdinal) {
    int ordinal = startingOrdinal;
    for (EntrySet target : targets) {
      emitEdge(source, kind, target, ordinal++);
    }
  }

  /** Returns (and emits) a new abstract node over child. */
  public EntrySet newAbstract(EntrySet child, List<EntrySet> params) {
    EntrySet abs = emitAndReturn(newNode(NodeKind.ABS)
        .addSignatureSalt(child.getVName()));
    emitEdge(child, EdgeKind.CHILDOF, abs);
    emitOrdinalEdges(abs, EdgeKind.PARAM, params);
    return abs;
  }

  /** Returns and emits a new {@link NodeKind.TAPPLY} function type node. */
  public EntrySet newFunctionType(EntrySet returnType, List<EntrySet> arguments) {
    List<EntrySet> tArgs = new LinkedList<>(arguments);
    tArgs.add(0, returnType);
    return newTApply(getBuiltin("fn"), tArgs);
  }

  /** Returns and emits a new {@link NodeKind.TAPPLY} node along with its parameter edges. */
  public EntrySet newTApply(EntrySet head, List<EntrySet> arguments) {
    EntrySet node = emitAndReturn(newApplyNode(NodeKind.TAPPLY, head, arguments));
    emitEdge(node, EdgeKind.PARAM, head, 0);
    emitOrdinalEdges(node, EdgeKind.PARAM, arguments, 1);
    return node;
  }

  /**
   * Returns a {@link NodeBuilder} with the given kind and added signature salts for each
   * {@link EntrySet} dependency as well as the "head" node of the application.
   */
  private NodeBuilder newApplyNode(NodeKind kind, EntrySet head, Iterable<EntrySet> dependencies) {
    return newNode(kind, dependencies).addSignatureSalt(head.getVName());
  }

  /**
   * Returns the {@link FileInput}'s {@link VName} with the given digest. If none is found, return
   * {@code null}.
   */
  protected VName lookupVName(String digest) {
    VName inputVName = inputVNames.get(digest);
    return inputVName == null
        ? null
        : EntrySet.extendVName(compilationVName, inputVName);
  }

  protected EntrySet emitAndReturn(EntrySet.Builder b) {
    return emitAndReturn(b.build());
  }

  protected EntrySet emitAndReturn(EntrySet set) {
    set.emit(emitter);
    return set;
  }

  /** {@link EntrySet.Builder} for Kythe nodes. */
  public static class NodeBuilder extends EntrySet.Builder {
    private static final String NODE_KIND_LABEL = "node/kind";
    private static final String NODE_SUBKIND_LABEL = "subkind";

    public NodeBuilder(NodeKind kind, String language) {
      this(kind.getKind(), kind.getSubkind(), language);
    }

    private NodeBuilder(String kind, String subkind, String language) {
      super(VName.newBuilder().setLanguage(language));
      setPropertyPrefix(NODE_PREFIX);
      setProperty(NODE_KIND_LABEL, kind);
      if (subkind != null) {
        setProperty(NODE_SUBKIND_LABEL, subkind);
      }
    }

    public NodeBuilder setSignature(String signature) {
      sourceBuilder.setSignature(signature);
      return this;
    }

    public NodeBuilder addSignatureSalt(String salt) {
      salts.add(salt);
      return this;
    }

    public NodeBuilder addSignatureSalt(VName vname) {
      return addSignatureSalt(new KytheURI(vname).toString());
    }

    public NodeBuilder setCorpusPath(CorpusPath p) {
      sourceBuilder
          .setCorpus(p.getCorpus())
          .setRoot(p.getRoot())
          .setPath(p.getPath());
      return this;
    }

    @Override
    public NodeBuilder setProperty(String name, byte[] value) {
      return (NodeBuilder) super.setProperty(name, value);
    }

    @Override
    public NodeBuilder setProperty(String name, String value) {
      return (NodeBuilder) super.setProperty(name, value);
    }
  }

  /** {@link EntrySet.Builder} for Kythe edges. */
  public static class EdgeBuilder extends EntrySet.Builder {
    private static final String ORDINAL_EDGE_KIND = "/kythe/ordinal";

    public EdgeBuilder(VName source, EdgeKind kind, VName target) {
      super(source, kind.getValue(), target);
    }

    public EdgeBuilder setOrdinal(int ordinal) {
      return setProperty(ORDINAL_EDGE_KIND, "" + ordinal);
    }

    @Override
    public EdgeBuilder setProperty(String name, byte[] value) {
      return (EdgeBuilder) super.setProperty(name, value);
    }

    @Override
    public EdgeBuilder setProperty(String name, String value) {
      return (EdgeBuilder) super.setProperty(name, value);
    }
  }
}
