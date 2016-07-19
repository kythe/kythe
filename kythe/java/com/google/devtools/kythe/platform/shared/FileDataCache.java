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
import com.google.devtools.kythe.proto.Analysis.FileData;
import java.util.Map;

/**
 * {@link FileDataProvider} that looks up file data from a given {@link List} of {@link FileData}.
 */
public class FileDataCache implements FileDataProvider {
  private final Map<String, byte[]> fileContents;

  public FileDataCache(Iterable<FileData> fileData) {
    ImmutableMap.Builder<String, byte[]> builder = ImmutableMap.builder();
    for (FileData data : fileData) {
      builder.put(data.getInfo().getDigest(), data.getContent().toByteArray());
    }
    fileContents = builder.build();
  }

  @Override
  public ListenableFuture<byte[]> startLookup(String path, String digest) {
    byte[] content = fileContents.get(digest);
    return content != null
        ? Futures.immediateFuture(content)
        : Futures.<byte[]>immediateFailedFuture(
            new RuntimeException("Cache does not contain file for digest: " + digest));
  }

  @Override
  public void close() {}
}
