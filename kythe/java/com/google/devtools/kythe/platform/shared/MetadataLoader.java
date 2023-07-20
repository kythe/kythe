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

import org.checkerframework.checker.nullness.qual.Nullable;

/** Attempts to parse metadata from a file. */
public interface MetadataLoader {
  /**
   * Returns a Metadata instance on success; {@code null} on failure.
   *
   * @param fileName The name of the metadata file, used to distinguish between different formats
   *     using file extensions.
   * @param data The raw data that composes the metadata file.
   */
  @Nullable Metadata parseFile(String fileName, byte[] data);
}
