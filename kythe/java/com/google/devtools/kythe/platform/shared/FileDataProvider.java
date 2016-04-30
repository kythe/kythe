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

import com.google.common.util.concurrent.ListenableFuture;

/**
 * Arbitrary provider of file data that could be backed by local/networked filesystems, cloud
 * storage, SQL database, etc.
 */
public interface FileDataProvider extends AutoCloseable {
  /**
   * Returns a {@link Future<byte[]>} for the contents of the file described by the given path and
   * digest. At least one of path or digest must be specified for each file lookup.
   */
  public ListenableFuture<byte[]> startLookup(String path, String digest);
}
