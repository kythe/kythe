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

package com.google.devtools.kythe.analyzers.base;

import com.google.devtools.kythe.proto.Storage.VName;

/** Path within a particular corpus and corpus root. */
public final class CorpusPath {
  private final String corpus, root, path;

  public CorpusPath(String corpus, String root, String path) {
    this.corpus = corpus;
    this.root = root;
    this.path = path;
  }

  /**
   * Returns a new {@link CorpusPath} equivalent to the corpus/path/root subset of the given {@link
   * VName}.
   */
  public static CorpusPath fromVName(VName vname) {
    return new CorpusPath(vname.getCorpus(), vname.getRoot(), vname.getPath());
  }

  public String getCorpus() {
    return corpus;
  }

  public String getRoot() {
    return root;
  }

  public String getPath() {
    return path;
  }
}
