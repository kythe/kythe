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

/** Emitter of facts. */
public interface FactEmitter {
  /**
   * Emits a single fact to some data sink. {@link edgeKind} and {@link target} must both be either
   * {@code null} (for a node entry) or non-{@code null} (for an edge entry).
   */
  public void emit(VName source, String edgeKind, VName target, String factName, byte[] factValue);
}
