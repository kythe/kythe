/*
 * Copyright 2017 Google Inc. All rights reserved.
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

import com.google.devtools.kythe.proto.Storage.Entry;
import com.google.devtools.kythe.proto.Storage.VName;
import com.google.protobuf.ByteString;
import java.io.IOException;
import java.io.OutputStream;

/** {@link FactEmitter} directly streaming to an {@link OutputValueStream}. */
public class StreamFactEmitter implements FactEmitter {
  private final OutputStream stream;

  public StreamFactEmitter(OutputStream stream) {
    this.stream = stream;
  }

  @Override
  public void emit(VName source, String edgeKind, VName target, String factName, byte[] factValue) {
    Entry.Builder entry =
        Entry.newBuilder()
            .setSource(source)
            .setFactName(factName)
            .setFactValue(ByteString.copyFrom(factValue));
    if (edgeKind != null) {
      entry.setEdgeKind(edgeKind).setTarget(target);
    }

    try {
      entry.build().writeDelimitedTo(stream);
    } catch (IOException ioe) {
      throw new RuntimeException(ioe);
    }
  }
}
