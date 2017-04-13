/*
 * Copyright 2016 Google Inc. All rights reserved.
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

import com.google.common.collect.Lists;
import java.util.List;

/**
 * Tries to use a series of MetadataLoader instances to load files. The first non-null result is
 * used.
 */
public class MetadataLoaders implements MetadataLoader {
  private final List<MetadataLoader> loaders = Lists.newArrayList();

  /**
   * Try using the given MetadataLoader to load data.
   *
   * @param loader the MetadataLoader to try
   */
  public void addLoader(MetadataLoader loader) {
    loaders.add(loader);
  }

  @Override
  public Metadata parseFile(String fileName, byte[] data) {
    Metadata metadata;
    for (MetadataLoader loader : loaders) {
      metadata = loader.parseFile(fileName, data);
      if (metadata != null) {
        return metadata;
      }
    }
    return null;
  }
}
