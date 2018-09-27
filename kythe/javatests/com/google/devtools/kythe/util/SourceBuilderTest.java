/*
 * Copyright 2017 The Kythe Authors. All rights reserved.
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

package com.google.devtools.kythe.util;

import static com.google.common.truth.Truth.assertThat;

import com.google.devtools.kythe.proto.Internal.Source;
import com.google.devtools.kythe.proto.Storage.Entry;
import com.google.devtools.kythe.proto.Storage.VName;
import com.google.protobuf.ByteString;
import junit.framework.TestCase;

/** Unit tests for {@link SourceBuilder}. */
public final class SourceBuilderTest extends TestCase {

  public void testEmptySource() {
    assertThat(new SourceBuilder().build()).isEqualTo(Source.getDefaultInstance());
  }

  public void testAddFact() {
    assertThat(new SourceBuilder().addEntry(factEntry("name", "value")).build())
        .isEqualTo(Source.newBuilder().putFacts("name", bs("value")).build());
  }

  public void testAddFact_duplicate() {
    assertThat(
            new SourceBuilder()
                .addEntry(factEntry("name", "badValue"))
                .addEntry(factEntry("name", "value"))
                .build())
        .isEqualTo(Source.newBuilder().putFacts("name", bs("value")).build());
  }

  public void testAddEdge() {
    assertThat(new SourceBuilder().addEntry(edgeEntry("kind", sig("sig"))).build())
        .isEqualTo(
            Source.newBuilder()
                .putEdgeGroups(
                    "kind",
                    Source.EdgeGroup.newBuilder()
                        .addEdges(Source.Edge.newBuilder().setTicket("kythe:#sig").build())
                        .build())
                .build());
  }

  public void testAddEdge_duplicate() {
    Entry edge = edgeEntry("kind", sig("target"));
    assertThat(new SourceBuilder().addEntry(edge).addEntry(edge).build())
        .isEqualTo(
            Source.newBuilder()
                .putEdgeGroups(
                    "kind",
                    Source.EdgeGroup.newBuilder()
                        .addEdges(Source.Edge.newBuilder().setTicket("kythe:#target").build())
                        .build())
                .build());
  }

  public void testAdd() {
    assertThat(
            new SourceBuilder()
                .addEntry(factEntry("fact1", "val"))
                .addEntry(edgeEntry("kind1", sig("target1")))
                .addEntry(factEntry("fact2", "val2"))
                .addEntry(edgeEntry("kind1", sig("target2")))
                .addEntry(edgeEntry("kind2", sig("target1")))
                .build())
        .isEqualTo(
            Source.newBuilder()
                .putFacts("fact1", bs("val"))
                .putFacts("fact2", bs("val2"))
                .putEdgeGroups("kind1", edgeGroup("kythe:#target1", "kythe:#target2"))
                .putEdgeGroups("kind2", edgeGroup("kythe:#target1"))
                .build());
  }

  private static Entry factEntry(String name, String value) {
    return Entry.newBuilder().setFactName(name).setFactValue(bs(value)).build();
  }

  private static Entry edgeEntry(String edgeKind, VName target) {
    return Entry.newBuilder().setEdgeKind(edgeKind).setTarget(target).build();
  }

  private static ByteString bs(String s) {
    return ByteString.copyFromUtf8(s);
  }

  private static VName sig(String sig) {
    return VName.newBuilder().setSignature(sig).build();
  }

  private static Source.EdgeGroup edgeGroup(String... targets) {
    Source.EdgeGroup.Builder group = Source.EdgeGroup.newBuilder();
    for (String target : targets) {
      group.addEdges(Source.Edge.newBuilder().setTicket(target).build());
    }
    return group.build();
  }
}
