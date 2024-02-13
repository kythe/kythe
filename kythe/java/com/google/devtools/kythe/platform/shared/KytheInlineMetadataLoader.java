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

import static java.nio.charset.StandardCharsets.UTF_8;

import com.google.common.base.Preconditions;
import com.google.common.base.Splitter;
import com.google.common.flogger.FluentLogger;
import com.google.common.io.BaseEncoding;
import com.google.devtools.kythe.analyzers.base.CorpusPath;
import com.google.devtools.kythe.analyzers.base.EdgeKind;
import com.google.devtools.kythe.analyzers.base.EntrySet;
import com.google.devtools.kythe.analyzers.base.FactEmitter;
import com.google.devtools.kythe.analyzers.base.KytheEntrySets.NodeBuilder;
import com.google.devtools.kythe.analyzers.base.NodeKind;
import com.google.devtools.kythe.proto.Metadata.GeneratedCodeInfo;
import com.google.devtools.kythe.proto.Metadata.MappingRule;
import com.google.devtools.kythe.proto.Storage.VName;
import com.google.devtools.kythe.util.Span;
import com.google.errorprone.annotations.CanIgnoreReturnValue;
import com.google.protobuf.ExtensionRegistry;
import com.google.protobuf.InvalidProtocolBufferException;
import java.util.List;
import org.checkerframework.checker.nullness.qual.Nullable;

/**
 * Loader that loads inline metadata, typically inserted into generated source code.
 *
 * <p>The metadata is typically added as a line comment in the form:
 *
 * <pre>// kythe-inline-metadata:contents</pre>
 *
 * where {@code contents} is a base64 string encoded {@link GeneratedCodeInfo} message.
 */
public final class KytheInlineMetadataLoader implements MetadataLoader {
  private static final String ANNOTATION_PREFIX_STRING = "kythe-inline-metadata:";
  private static final String LINE_COMMENT_PREFIX = "//";
  private static final byte[] ANNOTATION_PREFIX = ANNOTATION_PREFIX_STRING.getBytes(UTF_8);
  private static final FluentLogger logger = FluentLogger.forEnclosingClass();

  /** The loader's parse mode, which determines what data is accepted by {@link #parseFile}. */
  public enum ParseMode {
    /**
     * {@link #parseFile} expects the entire contents of the file. Any line beginning with '//' is
     * treated as a line comment and tested for "kythe-inline-metadata:base64".
     */
    FULL_FILE,

    /** {@link #parseFile} expects just the value "kythe-inline-metadata:base64". */
    METADATA_ONLY,
    ;
  }

  private final @Nullable FactEmitter emitter;
  private final ParseMode mode;

  /**
   * @deprecated A FactEmitter is required for loading IMPUTES edges.
   */
  @Deprecated
  public KytheInlineMetadataLoader() {
    this.emitter = null;
    this.mode = ParseMode.METADATA_ONLY;
  }

  /**
   * Creates a loader with mode {@link ParseMode#METADATA_ONLY}, which is how the Java indexer uses
   * this class.
   */
  public KytheInlineMetadataLoader(FactEmitter emitter) {
    this(emitter, ParseMode.METADATA_ONLY);
  }

  public KytheInlineMetadataLoader(FactEmitter emitter, ParseMode mode) {
    this.emitter = Preconditions.checkNotNull(emitter);
    this.mode = Preconditions.checkNotNull(mode);
  }

  @Override
  public @Nullable Metadata parseFile(String fileName, byte[] data) {
    if (mode == ParseMode.METADATA_ONLY && !isCodegenComment(data)) {
      return null;
    }
    GeneratedCodeInfo javaMetadata;
    try {
      javaMetadata = extractMetadata(data);
    } catch (InvalidProtocolBufferException e) {
      logger.atWarning().withCause(e).log(
          "Error parsing GeneratedCodeInfo from file: %s", fileName);
      return null;
    }
    if (javaMetadata == null) {
      return null;
    }
    return constructMetadata(javaMetadata);
  }

  private static boolean isCodegenComment(byte[] data) {
    if (data == null || data.length < ANNOTATION_PREFIX.length) {
      return false;
    }
    for (int i = 0; i < ANNOTATION_PREFIX.length; i++) {
      if (data[i] != ANNOTATION_PREFIX[i]) {
        return false;
      }
    }
    return true;
  }

  private @Nullable GeneratedCodeInfo extractMetadata(byte[] data)
      throws InvalidProtocolBufferException {
    String metadata = null;
    if (mode == ParseMode.METADATA_ONLY) {
      metadata = new String(data, UTF_8).substring(ANNOTATION_PREFIX_STRING.length());
    } else {
      String code = new String(data, UTF_8);
      List<String> lines = Splitter.on('\n').splitToList(code.trim());
      for (String line : lines) {
        if (!line.startsWith(LINE_COMMENT_PREFIX)) {
          continue;
        }
        int index = line.indexOf(ANNOTATION_PREFIX_STRING);
        if (index >= 0) {
          metadata = line.substring(index + ANNOTATION_PREFIX_STRING.length()).trim();
          break;
        }
      }
    }
    if (metadata == null) {
      return null;
    }
    byte[] protoData = BaseEncoding.base64().decode(metadata);
    return GeneratedCodeInfo.parseFrom(protoData, ExtensionRegistry.getEmptyRegistry());
  }

  private Metadata constructMetadata(GeneratedCodeInfo javaMetadata) {
    Metadata metadata = new Metadata();
    for (MappingRule mapping : javaMetadata.getMetaList()) {
      Metadata.Rule rule = new Metadata.Rule();

      if (mapping.getType() == MappingRule.Type.ANCHOR_DEFINES) {
        rule.begin = mapping.getBegin();
        rule.end = mapping.getEnd();
        rule.vname = mapping.getVname();
        rule.edgeOut = EdgeKind.GENERATES;
        rule.reverseEdge = true;
      } else if (mapping.getType() == MappingRule.Type.ANCHOR_ANCHOR) {
        if (emitter == null) {
          throw new IllegalStateException("Must have a FactEmitter to handle IMPUTES edges");
        }

        VName sourceVName = mapping.getSourceVname();
        EntrySet anchor =
            newAnchorAndEmit(
                sourceVName, new Span(mapping.getSourceBegin(), mapping.getSourceEnd()));
        if (anchor == null) {
          continue;
        }

        rule.begin = mapping.getTargetBegin();
        rule.end = mapping.getTargetEnd();
        rule.edgeOut = EdgeKind.IMPUTES;
        rule.reverseEdge = true;
        rule.vname = anchor.getVName();
      } else {
        // Ignore NOP, fail open.
      }

      metadata.addRule(rule);
    }
    return metadata;
  }

  private @Nullable EntrySet newAnchorAndEmit(VName fileVName, Span loc) {
    if (!loc.isValid()) {
      return null;
    }
    EntrySet.Builder builder =
        new NodeBuilder(
                loc.getStart() == loc.getEnd() ? NodeKind.ANCHOR_IMPLICIT : NodeKind.ANCHOR, "")
            .setCorpusPath(CorpusPath.fromVName(fileVName))
            .addSignatureSalt(fileVName)
            .setProperty("loc/start", "" + loc.getStart())
            .setProperty("loc/end", "" + loc.getEnd());
    EntrySet anchor = builder.build();
    return emitAndReturn(anchor);
  }

  @CanIgnoreReturnValue
  private EntrySet emitAndReturn(EntrySet set) {
    set.emit(emitter);
    return set;
  }
}
