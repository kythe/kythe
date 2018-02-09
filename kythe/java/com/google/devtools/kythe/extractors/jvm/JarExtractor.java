/*
 * Copyright 2018 Google Inc. All rights reserved.
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

package com.google.devtools.kythe.extractors.jvm;

import com.beust.jcommander.JCommander;
import com.google.common.base.Joiner;
import com.google.common.base.Strings;
import com.google.common.hash.Hashing;
import com.google.devtools.kythe.extractors.shared.CompilationDescription;
import com.google.devtools.kythe.extractors.shared.ExtractionException;
import com.google.devtools.kythe.extractors.shared.IndexInfoUtils;
import com.google.devtools.kythe.platform.indexpack.Archive;
import java.io.IOException;

/**
 * Kythe extractor for Java .jar files.
 *
 * <p>Usage: jar_extractor <.jar file | .class file>*
 */
public class JarExtractor {
  public static void main(String[] args) throws IOException, ExtractionException {
    JvmExtractor.Options options = new JvmExtractor.Options();
    JCommander jc = new JCommander(options, args);
    jc.setProgramName("jar_extractor");
    if (options.help) {
      jc.usage();
      System.exit(2);
    }

    CompilationDescription indexInfo = JvmExtractor.extract(options);

    String outputFile = System.getenv("KYTHE_OUTPUT_FILE");
    if (!Strings.isNullOrEmpty(outputFile)) {
      IndexInfoUtils.writeIndexInfoToFile(indexInfo, outputFile);
    } else {
      String outputDir = System.getenv("KYTHE_OUTPUT_DIRECTORY");
      if (Strings.isNullOrEmpty(outputDir)) {
        throw new IllegalArgumentException(
            "required KYTHE_OUTPUT_FILE or KYTHE_OUTPUT_DIRECTORY environment variable is unset");
      }
      if (Strings.isNullOrEmpty(System.getenv("KYTHE_INDEX_PACK"))) {
        String name = Hashing.sha256().hashUnencodedChars(Joiner.on(" ").join(args)).toString();
        String path = IndexInfoUtils.getIndexPath(outputDir, name).toString();
        IndexInfoUtils.writeIndexInfoToFile(indexInfo, path);
      } else {
        new Archive(outputDir).writeDescription(indexInfo);
      }
    }
  }
}
