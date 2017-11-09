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

package com.google.devtools.kythe.platform.shared;

import com.google.common.collect.ImmutableMap;
import com.google.common.util.concurrent.Futures;
import com.google.common.util.concurrent.ListenableFuture;
import com.google.devtools.kythe.common.FormattingLogger;
import com.google.devtools.kythe.proto.Analysis.FileData;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

/**
 * {@link FileDataProvider} that looks up file data from a given {@link Iterable} of {@link
 * FileData}.
 */
public class FileDataCache implements FileDataProvider {

  private static final FormattingLogger logger = FormattingLogger.getLogger(FileDataProvider.class);

  private final Map<String, byte[]> fileContents;

  public FileDataCache(Iterable<FileData> fileData) {
    ImmutableMap.Builder<String, byte[]> builder = ImmutableMap.builder();
    Set<String> addedFiles = new HashSet<>();
    for (FileData data : fileData) {
      String digest = data.getInfo().getDigest();
      if (addedFiles.contains(digest)) {
        logger.warningfmt(
            "File %s with digest %s occurs multiple times in compilation unit.",
            data.getInfo().getPath(), digest);
      } else {
        addedFiles.add(digest);
        builder.put(digest, data.getContent().toByteArray());
      }
    }
    fileContents = builder.build();
  }

  @Override
  public ListenableFuture<byte[]> startLookup(String path, String digest) {
    if (digest == null) {
      return Futures.immediateFailedFuture(new IllegalArgumentException("digest cannot be null"));
    }
    byte[] content = fileContents.get(digest);
    return content == null
        ? Futures.immediateFailedFuture(
            new RuntimeException("Cache does not contain file for digest: " + digest))
        : Futures.immediateFuture(content);
  }

  @Override
  public void close() {}
}
