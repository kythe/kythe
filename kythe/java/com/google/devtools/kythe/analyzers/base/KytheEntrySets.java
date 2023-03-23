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

import com.google.common.base.Preconditions;
import com.google.common.collect.ImmutableList;
import com.google.devtools.kythe.platform.shared.StatisticsCollector;
import com.google.devtools.kythe.proto.Analysis.CompilationUnit.FileInput;
import com.google.devtools.kythe.proto.Diagnostic;
import com.google.devtools.kythe.proto.MarkedSource;
import com.google.devtools.kythe.proto.MarkedSource.Kind;
import com.google.devtools.kythe.proto.Storage.VName;
import com.google.devtools.kythe.util.KytheURI;
import com.google.devtools.kythe.util.Span;
import java.nio.charset.Charset;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import org.checkerframework.checker.nullness.qual.Nullable;

/**
 * Factory for Kythe-compliant node and edge {@link EntrySet}s. In general, this class provides two
 * sets of methods: low-level node/edge builders and higher-level abstractions for specific kinds of
 * nodes/edges such as ANCHOR nodes and their edges. Each higher-level abstraction returns a fully
 * realized {@link EntrySet} and automatically emits it (and any associated edges). The lower-level
 * methods, such as {@link #newNode(String)} return {@link EntrySet.Builder}s and must be emitted by
 * the client.
 *
 * <p>This class is meant to be subclassed to build indexer-specific nodes and edges.
 */
public class KytheEntrySets {
  public static final String NODE_PREFIX = "/kythe/";

  private final StatisticsCollector statistics;
  private final FactEmitter emitter;
  private final String language;

  private final VName compilationVName;
  private final Map<String, VName> inputVNames;
  protected boolean useCompilationCorpusAsDefault;

  public KytheEntrySets(
      StatisticsCollector statistics,
      FactEmitter emitter,
      VName compilationVName,
      List<FileInput> requiredInputs) {
    this.statistics = statistics;
    this.emitter = emitter;
    this.language = compilationVName.getLanguage();
    this.compilationVName = compilationVName;

    inputVNames = new HashMap<>();
    for (FileInput input : requiredInputs) {
      String digest = input.getInfo().getDigest();
      VName.Builder name = input.getVName().toBuilder();
      Preconditions.checkArgument(
          !name.getPath().isEmpty(), "Required input VName must have non-empty path");
      if (name.getSignature().isEmpty()) {
        // Ensure file VName has digest signature
        name = name.setSignature(digest);
      }
      inputVNames.put(digest, name.build());
    }
  }

  public final void setUseCompilationCorpusAsDefault(boolean d) {
    this.useCompilationCorpusAsDefault = d;
  }

  public final boolean getUseCompilationCorpusAsDefault() {
    return useCompilationCorpusAsDefault;
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
   * Returns a {@link NodeBuilder} with the given kind and added signature salts for each {@link
   * EntrySet} dependency.
   */
  public NodeBuilder newNode(NodeKind kind, Iterable<VName> dependencies) {
    NodeBuilder builder = newNode(kind);
    for (VName d : dependencies) {
      builder.addSignatureSalt(d);
    }
    return builder;
  }

  /**
   * Returns a new {@link NodeBuilder} with the given node kind set.
   *
   * <p>Note: use {@link #newNode(NodeKind)} for schema-defined kinds
   */
  public NodeBuilder newNode(String kind) {
    return newNode(kind, Optional.empty());
  }

  /**
   * Returns a new {@link NodeBuilder} with the given node kind set.
   *
   * <p>Note: use {@link #newNode(NodeKind)} for schema-defined kinds
   */
  public NodeBuilder newNode(String kind, Optional<String> subkind) {
    getStatisticsCollector().incrementCounter("string-new-node-" + kind);
    return new NodeBuilder(kind, subkind, language);
  }

  /** Returns a new {@link NodeBuilder} with the given node kind set. */
  public NodeBuilder newNode(NodeKind kind) {
    getStatisticsCollector().incrementCounter("new-node-" + kind);
    return new NodeBuilder(kind, language);
  }

  /** Returns a new {@link NodeBuilder} with the given node kind and vname set. */
  public NodeBuilder newNode(NodeKind kind, VName source) {
    getStatisticsCollector().incrementCounter("new-node-" + kind);
    return new NodeBuilder(kind, source);
  }

  /** Returns (and emits) a new builtin node. */
  public EntrySet newBuiltinAndEmit(String name) {
    return newBuiltinAndEmit(name, Optional.empty());
  }

  /** Returns (and emits) a new builtin node. */
  public EntrySet newBuiltinAndEmit(String name, Optional<String> docUri) {
    EntrySet.Builder nodeBuilder =
        newNode(NodeKind.TBUILTIN)
            .setSignature(getBuiltinSignature(name))
            .setCorpusPath(defaultCorpusPath())
            .setProperty(
                "code",
                MarkedSource.newBuilder().setPreText(name).setKind(Kind.IDENTIFIER).build());
    if (docUri.isPresent()) {
      setDocumentUriProperty(nodeBuilder, docUri.get());
    }
    return emitAndReturn(nodeBuilder);
  }

  /** Returns a VName for the builtin node corresponding to the specified name. */
  public VName getBuiltinVName(String name) {
    return VName.newBuilder()
        .setSignature(getBuiltinSignature(name))
        .setLanguage(this.language)
        .build();
  }

  /** Returns (and emits) a new implicit anchor node in the given file. */
  public EntrySet newImplicitAnchorAndEmit(VName fileVName) {
    return emitAndReturn(
        newNode(NodeKind.ANCHOR_IMPLICIT)
            .setCorpusPath(CorpusPath.fromVName(fileVName))
            .addSignatureSalt(fileVName));
  }

  /**
   * Returns (and emits) a new anchor node at the given location in the file with an optional
   * snippet span.
   */
  public @Nullable EntrySet newAnchorAndEmit(VName fileVName, Span loc, Span snippet) {
    if (loc == null || !loc.isValid()) {
      // TODO(schroederc): reduce number of invalid anchors
      return null;
    }
    EntrySet.Builder builder =
        newNode(loc.getStart() == loc.getEnd() ? NodeKind.ANCHOR_IMPLICIT : NodeKind.ANCHOR)
            .setCorpusPath(CorpusPath.fromVName(fileVName))
            .addSignatureSalt(fileVName)
            .setProperty("loc/start", "" + loc.getStart())
            .setProperty("loc/end", "" + loc.getEnd());
    if (snippet != null && snippet.isValid()) {
      builder
          .setProperty("snippet/start", "" + snippet.getStart())
          .setProperty("snippet/end", "" + snippet.getEnd());
    }
    EntrySet anchor = builder.build();
    return emitAndReturn(anchor);
  }

  /** Emits and returns a DIAGNOSTIC node attached to no file. */
  public EntrySet emitDiagnostic(Diagnostic d) {
    return emitDiagnostic(null, d);
  }

  /**
   * Emits and returns a DIAGNOSTIC node attached to the given file (which may be null if file
   * context unknown).
   */
  public EntrySet emitDiagnostic(VName fileVName, Diagnostic d) {
    NodeBuilder builder =
        newNode(NodeKind.DIAGNOSTIC)
            .addSignatureSalt(d.getMessage())
            .addSignatureSalt(d.getDetails())
            .addSignatureSalt(d.getContextUrl())
            .setProperty("message", d.getMessage());
    if (!d.getDetails().isEmpty()) {
      builder.setProperty("details", d.getDetails());
    }
    if (!d.getContextUrl().isEmpty()) {
      builder.setProperty("context/url", d.getContextUrl());
    }
    if (fileVName == null) {
      if (getUseCompilationCorpusAsDefault()) {
        builder.setCorpusPath(defaultCorpusPath());
      }
    } else {
      builder.setCorpusPath(CorpusPath.fromVName(fileVName));
    }
    EntrySet dn = emitAndReturn(builder);
    if (fileVName == null) {
      return dn;
    } else {
      if (d.hasSpan()) {
        Span s =
            new Span(d.getSpan().getStart().getByteOffset(), d.getSpan().getEnd().getByteOffset());
        EntrySet anchor = newAnchorAndEmit(fileVName, s, null);
        emitEdge(anchor, EdgeKind.TAGGED, dn);
      } else {
        emitEdge(fileVName, EdgeKind.TAGGED, dn.getVName());
      }
    }
    return dn;
  }

  /**
   * Returns the {@link VName} of the {@link NodeKind#FILE} node with the given contents digest. If
   * none is found, returns {@code null}.
   */
  public @Nullable VName getFileVName(String digest) {
    VName name = lookupVName(digest);
    if (name == null) {
      return null;
    }
    // https://www.kythe.io/docs/schema/#file
    return name.toBuilder().setLanguage("").setSignature("").build();
  }

  /** Emits and returns a new {@link EntrySet} representing a file digest. */
  public EntrySet newFileNodeAndEmit(String digest, byte[] contents, Charset encoding) {
    VName name = getFileVName(digest);
    return newFileNodeAndEmit(name, contents, encoding);
  }

  /** Emits and returns a new {@link EntrySet} representing a file {@link VName}. */
  public EntrySet newFileNodeAndEmit(VName name, byte[] contents, Charset encoding) {
    return emitAndReturn(
        new NodeBuilder(NodeKind.FILE, name)
            .setProperty("text", contents)
            .setProperty("text/encoding", encoding.name()));
  }

  /** Emits an edge of the given kind from {@code source} to {@code target}. */
  public void emitEdge(EntrySet source, EdgeKind kind, EntrySet target) {
    Preconditions.checkNotNull(source, "source EntrySet must be non-null");
    Preconditions.checkNotNull(target, "target EntrySet must be non-null");
    emitEdge(source.getVName(), kind, target.getVName());
  }

  /** Emits an edge of the given kind from {@code source} to {@code target}. */
  public void emitEdge(VName source, EdgeKind kind, VName target) {
    getStatisticsCollector().incrementCounter("emit-edge-" + kind);
    new EdgeBuilder(source, kind, target).build().emit(emitter);
  }

  /** Emits an edge of the given kind and ordinal from {@code source} to {@code target}. */
  public void emitEdge(EntrySet source, EdgeKind kind, EntrySet target, int ordinal) {
    getStatisticsCollector().incrementCounter("emit-edge-" + kind);
    new EdgeBuilder(source.getVName(), kind, ordinal, target.getVName()).build().emit(emitter);
  }

  /** Emits an edge of the given kind and ordinal from {@code source} to {@code target}. */
  public void emitEdge(VName source, EdgeKind kind, VName target, int ordinal) {
    getStatisticsCollector().incrementCounter("emit-edge-" + kind);
    new EdgeBuilder(source, kind, ordinal, target).build().emit(emitter);
  }

  /**
   * Emits edges of the given kind from {@code source} to each of the target {@link VName}s, with
   * their respective {@link Iterable} order (0-based) as their edge ordinal.
   */
  public void emitOrdinalEdges(VName source, EdgeKind kind, Iterable<VName> targets) {
    emitOrdinalEdges(source, kind, targets, 0);
  }

  /**
   * Emits edges of the given kind from {@code source} to each of the target {@link VName}s, with
   * their respective {@link Iterable} order as their edge ordinal.
   */
  public void emitOrdinalEdges(
      VName source, EdgeKind kind, Iterable<VName> targets, int startingOrdinal) {
    int ordinal = startingOrdinal;
    for (VName target : targets) {
      emitEdge(source, kind, target, ordinal++);
    }
  }

  /** Returns and emits a new {@link NodeKind#TAPPLY} function type node. */
  public EntrySet newFunctionTypeAndEmit(
      VName returnType, VName receiverType, List<VName> arguments, MarkedSource ms) {
    ImmutableList<VName> tArgs =
        ImmutableList.<VName>builderWithExpectedSize(2 + arguments.size())
            .add(returnType)
            .add(receiverType)
            .addAll(arguments)
            .build();
    return newTApplyAndEmit(newBuiltinAndEmit("fn").getVName(), tArgs, ms);
  }

  /** Returns and emits a new {@link NodeKind#TAPPLY} node along with its parameter edges. */
  public EntrySet newTApplyAndEmit(VName head, List<VName> arguments, MarkedSource ms) {
    NodeBuilder builder = newApplyNode(NodeKind.TAPPLY, head, arguments);
    if (ms != null) {
      builder.setProperty("code", ms);
    }
    builder.setCorpusPath(defaultCorpusPath());
    EntrySet node = emitAndReturn(builder);
    emitEdge(node.getVName(), EdgeKind.PARAM, head, 0);
    emitOrdinalEdges(node.getVName(), EdgeKind.PARAM, arguments, 1);
    return node;
  }

  /**
   * Returns a {@link NodeBuilder} with the given kind and added signature salts for each {@link
   * EntrySet} dependency as well as the "head" node of the application.
   */
  private NodeBuilder newApplyNode(NodeKind kind, VName head, Iterable<VName> dependencies) {
    return newNode(kind, dependencies).addSignatureSalt(head);
  }

  /**
   * Returns the {@link FileInput}'s {@link VName} with the given digest. If none is found, return
   * {@code null}.
   */
  protected @Nullable VName lookupVName(String digest) {
    VName inputVName = inputVNames.get(digest);
    return inputVName == null ? null : EntrySet.extendVName(compilationVName, inputVName);
  }

  /**
   * Returns a CorpusPath containing the default corpus. This will either be empty string or the
   * compilation unit's corpus depending on the --use_compilation_corpus_as_default option.
   */
  public CorpusPath defaultCorpusPath() {
    return new CorpusPath(
        useCompilationCorpusAsDefault ? compilationVName.getCorpus() : "", "", "");
  }

  protected EntrySet emitAndReturn(EntrySet.Builder b) {
    return emitAndReturn(b.build());
  }

  protected EntrySet emitAndReturn(EntrySet set) {
    set.emit(emitter);
    return set;
  }

  /** Gets a builtin signature for the specified name. */
  protected static String getBuiltinSignature(String name) {
    return String.format("%s#builtin", name);
  }

  /** Sets the document URI property for the specified node builder. */
  protected static EntrySet.Builder setDocumentUriProperty(
      EntrySet.Builder builder, String docUri) {
    return builder.setProperty("doc/uri", docUri);
  }

  /** {@link EntrySet.Builder} for Kythe nodes. */
  public static class NodeBuilder extends EntrySet.Builder {
    private static final String NODE_KIND_LABEL = "node/kind";
    private static final String NODE_SUBKIND_LABEL = "subkind";

    public NodeBuilder(NodeKind kind, String language) {
      this(kind.getKind(), kind.getSubkind(), language);
    }

    public NodeBuilder(NodeKind kind, VName name) {
      super(name, null, null);
      setupNode(kind.getKind(), kind.getSubkind());
    }

    private NodeBuilder(String kind, Optional<String> subkind, String language) {
      super(VName.newBuilder().setLanguage(language));
      setupNode(kind, subkind);
    }

    private void setupNode(String kind, Optional<String> subkind) {
      setPropertyPrefix(NODE_PREFIX);
      setProperty(NODE_KIND_LABEL, kind);
      if (subkind.isPresent()) {
        setProperty(NODE_SUBKIND_LABEL, subkind.get());
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
      return addSignatureSalt(KytheURI.asString(vname));
    }

    public NodeBuilder setCorpusPath(CorpusPath p) {
      sourceBuilder.setCorpus(p.getCorpus()).setRoot(p.getRoot()).setPath(p.getPath());
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
    public EdgeBuilder(VName source, EdgeKind kind, VName target) {
      super(source, kind.getValue(), target);
    }

    public EdgeBuilder(VName source, EdgeKind kind, int ordinal, VName target) {
      super(source, kind.getValue(), ordinal, target);
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
