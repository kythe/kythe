/*
 * Copyright 2018 The Kythe Authors. All rights reserved.
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

package com.google.devtools.kythe.extractors.jvm.bazel;

import static com.google.common.io.Files.touch;

import com.google.devtools.build.lib.actions.extra.ExtraActionInfo;
import com.google.devtools.build.lib.actions.extra.ExtraActionsBase;
import com.google.devtools.build.lib.actions.extra.SpawnInfo;
import com.google.devtools.kythe.extractors.jvm.JvmExtractor;
import com.google.devtools.kythe.extractors.jvm.JvmExtractor.Options;
import com.google.devtools.kythe.extractors.shared.CompilationDescription;
import com.google.devtools.kythe.extractors.shared.ExtractionException;
import com.google.devtools.kythe.extractors.shared.IndexInfoUtils;
import com.google.devtools.kythe.util.JsonUtil;
import com.google.protobuf.CodedInputStream;
import com.google.protobuf.ExtensionRegistry;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.List;

/**
 * Kythe extractor for Bazel JavaIjar actions.
 *
 * <p>Usage: bazel_jvm_extractor <extra_action_file> <output_file>
 */
public class BazelJvmExtractor {
  private BazelJvmExtractor() {}

  public static void main(String[] args) throws IOException, ExtractionException {
    JsonUtil.usingTypeRegistry(JvmExtractor.JSON_TYPE_REGISTRY);

    if (args.length != 2 && args.length != 3) {
      System.err.println(
          "Usage: bazel_jvm_extractor extra-action-file output-file [vnames-config]");
      System.exit(1);
    }

    String extraActionPath = args[0];
    String outputPath = args[1];

    ExtensionRegistry registry = ExtensionRegistry.newInstance();
    ExtraActionsBase.registerAllExtensions(registry);

    ExtraActionInfo info;
    try (InputStream stream = Files.newInputStream(Paths.get(extraActionPath))) {
      CodedInputStream coded = CodedInputStream.newInstance(stream);
      info = ExtraActionInfo.parseFrom(coded, registry);
    }

    if (!info.hasExtension(SpawnInfo.spawnInfo)) {
      throw new IllegalArgumentException("Given ExtraActionInfo without SpawnInfo");
    }

    SpawnInfo spawnInfo = info.getExtension(SpawnInfo.spawnInfo);
    if (!Paths.get(spawnInfo.getArgument(0)).endsWith("ijar")) {
      throw new IllegalArgumentException("given non-ijar SpawnInfo");
    }

    List<Path> jarFiles = new ArrayList<>();
    for (String inputFile : spawnInfo.getInputFileList()) {
      if (inputFile.endsWith(JvmExtractor.JAR_FILE_EXT)) {
        jarFiles.add(Paths.get(inputFile));
      }
    }

    Options opts = new Options();
    opts.buildTarget = info.getOwner();
    opts.jarOrClassFiles.addAll(jarFiles);
    if (args.length == 3) {
      opts.vnamesConfigFile = Paths.get(args[2]);
    }

    CompilationDescription indexInfo = JvmExtractor.extract(opts);

    if (indexInfo.getCompilationUnit().getRequiredInputCount() == 0) {
      // Skip empty compilations; there is nothing to analyze.
      touch(new File(outputPath));
      return;
    }

    if (!outputPath.endsWith(IndexInfoUtils.KZIP_FILE_EXT)) {
      throw new IllegalArgumentException("Expected output file to have .kzip extension");
    }
    IndexInfoUtils.writeKzipToFile(indexInfo, outputPath);
  }
}
