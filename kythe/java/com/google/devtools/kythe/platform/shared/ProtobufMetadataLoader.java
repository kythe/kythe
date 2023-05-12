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

package com.google.devtools.kythe.platform.shared;

import com.google.common.flogger.FluentLogger;
import com.google.devtools.kythe.analyzers.base.EdgeKind;
import com.google.devtools.kythe.proto.Analysis.CompilationUnit;
import com.google.devtools.kythe.proto.Storage.VName;
import com.google.protobuf.DescriptorProtos.GeneratedCodeInfo;
import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.nio.file.InvalidPathException;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Set;
import java.util.function.Function;
import org.checkerframework.checker.nullness.qual.Nullable;

/** Loads protobuf metadata (stored as GeneratedCodeInfo messages). */
public class ProtobufMetadataLoader implements MetadataLoader {
  private static final FluentLogger logger = FluentLogger.forEnclosingClass();

  /**
   * Interface to support different ways of extracting GeneratedCodeInfo from a file. For example
   * .pb.meta file contain proto in binary format while in other cases proto is embedded in a
   * comment in base64 format.
   */
  public interface GeneratedCodeInfoExtractor {
    @Nullable GeneratedCodeInfo extract(String fileName, byte[] data);
  }

  /**
   * @param unit used to look up VNames for paths.
   * @param defaultCorpus should vnameLookup return null, this value should be used for the corpus
   *     field; if it is null, then no metadata will be emitted.
   */
  public ProtobufMetadataLoader(CompilationUnit unit, String defaultCorpus) {
    this(unit, defaultCorpus, ProtobufMetadataLoader::extractAnnotationsFromPbMetaFile);
  }

  /**
   * @param unit used to look up VNames for paths.
   * @param defaultCorpus should vnameLookup return null, this value should be used for the corpus
   *     field; if it is null, then no metadata will be emitted.
   * @param extractor extractor to read GeneratedCodeInfoExtractor from input files.
   */
  public ProtobufMetadataLoader(
      CompilationUnit unit, String defaultCorpus, GeneratedCodeInfoExtractor extractor) {
    this.vnameLookup = lookupVNameFromCompilationUnit(unit);
    this.defaultCorpus = defaultCorpus;
    this.extractor = extractor;
  }

  /** This extension signifies a GeneratedCodeInfo metadata file. */
  private static final String META_SUFFIX = ".pb.meta";

  /** The language used for protobuf VNames. */
  private static final String PROTOBUF_LANGUAGE = "protobuf";

  /** vnameLookup is used to map GeneratedCodeInfo filenames to VNames. */
  private final Function<String, VName> vnameLookup;

  /** defaultCorpus is used when no corpus can be found for a given file. */
  private final String defaultCorpus;

  /** extractor is used to extract GeneratedCodeInfo proto from a file. */
  private final GeneratedCodeInfoExtractor extractor;

  /**
   * @return a function that looks up the VName for some filename in the given CompilationUnit.
   */
  private static Function<String, VName> lookupVNameFromCompilationUnit(CompilationUnit unit) {
    HashMap<String, VName> map = new HashMap<>();
    Path root = Paths.get("/", unit.getWorkingDirectory());
    for (CompilationUnit.FileInput input : unit.getRequiredInputList()) {
      try {
        map.put(root.resolve(input.getInfo().getPath()).toString(), input.getVName());
      } catch (InvalidPathException ipe) {
        logger.atWarning().withCause(ipe).log(
            "Found invalid path in CompilationUnit: %s", input.getInfo());
      }
    }
    return p -> map.get(root.resolve(p).toString());
  }

  @Override
  public @Nullable Metadata parseFile(String fileName, byte[] data) {
    GeneratedCodeInfo info = extractor.extract(fileName, data);
    if (info == null) {
      return null;
    }
    VName contextVName = vnameLookup.apply(fileName);
    if (contextVName == null) {
      logger.atWarning().log("Failed getting VName for metadata: %s", fileName);
      if (defaultCorpus == null) {
        return null;
      }
      contextVName = VName.newBuilder().setCorpus(defaultCorpus).build();
    }
    Metadata metadata = new Metadata();
    Set<VName> fileVNames = new HashSet<>();

    for (GeneratedCodeInfo.Annotation annotation : info.getAnnotationList()) {
      Metadata.Rule rule = new Metadata.Rule();
      rule.begin = annotation.getBegin();
      rule.end = annotation.getEnd();
      rule.vname = vnameLookup.apply(annotation.getSourceFile());
      StringBuilder protoPath = new StringBuilder();
      boolean needsDot = false;
      for (int node : annotation.getPathList()) {
        if (needsDot) {
          protoPath.append(".");
        }
        protoPath.append(node);
        needsDot = true;
      }
      if (rule.vname == null) {
        rule.vname =
            VName.newBuilder()
                .setCorpus(contextVName.getCorpus())
                .setPath(annotation.getSourceFile())
                .build();
      }
      fileVNames.add(rule.vname);
      rule.vname =
          rule.vname.toBuilder()
              .setSignature(protoPath.toString())
              .setLanguage(PROTOBUF_LANGUAGE)
              .build();
      rule.edgeOut = EdgeKind.GENERATES;
      rule.reverseEdge = true;
      rule.semantic = annotation.getSemantic();
      metadata.addRule(rule);
    }
    for (VName vname : fileVNames) {
      Metadata.Rule rule = new Metadata.Rule();
      rule.begin = -1;
      rule.end = -1;
      rule.vname = vname;
      rule.reverseEdge = true;
      rule.edgeOut = EdgeKind.GENERATES;
      metadata.addFileScopeRule(rule);
    }
    return metadata;
  }

  private static @Nullable GeneratedCodeInfo extractAnnotationsFromPbMetaFile(
      String fileName, byte[] data) {
    if (!fileName.endsWith(META_SUFFIX)) {
      return null;
    }
    GeneratedCodeInfo info;
    try (ByteArrayInputStream stream = new ByteArrayInputStream(data)) {
      info = GeneratedCodeInfo.parseFrom(stream);
    } catch (IOException ex) {
      logger.atWarning().log("IOException on %s", fileName);
      return null;
    }
    return info;
  }
}
