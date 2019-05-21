/*
 * Copyright 2019 The Kythe Authors. All rights reserved.
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

package com.google.devtools.kythe.util.schema;

import static com.google.common.truth.Truth.assertThat;
import static com.google.common.truth.Truth.assertWithMessage;

import com.google.common.collect.ImmutableList;
import com.google.common.collect.Streams;
import com.google.devtools.kythe.proto.Internal.Source;
import com.google.devtools.kythe.proto.Schema.Entry;
import com.google.devtools.kythe.proto.Schema.Node;
import com.google.devtools.kythe.proto.Storage;
import com.google.protobuf.Message;
import com.google.protobuf.TextFormat;
import java.util.function.Function;
import junit.framework.TestCase;

/** Tests for {@link Nodes}. */
public final class NodesTest extends TestCase {
  public void testNormalizeNode() {
    testProtoConversion(
        "normalizeNode",
        Nodes::normalizeNode,
        Node.class,
        Node.class,
        ImmutableList.of(
            "source: {signature: 's' corpus: 'c'}",
            "fact: {kythe_name: NODE_KIND value: 'record'}",
            "fact: {kythe_name: SUBKIND value: 'class'}",
            "fact: {kythe_name: NODE_KIND value: 'some_kind'}",
            "fact: {kythe_name: SUBKIND value: 'some_subkind'}",
            "edge: {generic_kind: '/kythe/edge/param' target: {signature: 'tgt'}}",
            "fact: {generic_name: '/kythe/node/kind' value: 'record'}",
            "fact: {generic_name: '/kythe/subkind' value: 'class'}",
            "fact: {generic_name: '/fact/name' value: 'value'}",
            "fact: {source: {signature: 's'} generic_name: '/fact/name' value: 'value'}",
            "edge: {kythe_kind: TYPED target: {signature: 'tgt'}}",
            "edge: {kythe_kind: PARAM ordinal: 42 target: {signature: 'tgt'}}",
            "edge: {generic_kind: '/edge/kind' target: {signature: 'tgt'}}",
            "edge: {source: {path: 'p'} generic_kind: '/edge/kind' target: {signature: 'tgt'}}",
            "edge: {generic_kind: '/edge/kind' ordinal: 4 target: {signature: 'tgt'}}",
            "generic_kind: 'something'",
            "kythe_kind: RECORD",
            "generic_subkind: 'some_subkind'",
            "kythe_subkind: CLASS",
            "source: {signature: 's' corpus: 'c' path: 'p'}"
                + " kythe_kind: RECORD"
                + " kythe_subkind: CLASS"
                + " fact: {kythe_name: TEXT value: 'txt'}"
                + " fact: {generic_name: '/fact/name' value: 'anything'}"
                + " edge: {kythe_kind: PARAM ordinal: 1 target: {signature: 'tgt2'}}"
                + " edge: {generic_kind: '/edge/kind' target: {signature: 'tgt'}}"
                + " edge: {generic_kind: '/edge/kind' ordinal: 1 target: {signature: 'tgt3'}}"
                + " edge: {kythe_kind: PARAM target: {signature: 'tgt'}}"),
        ImmutableList.of(
            "source: {signature: 's' corpus: 'c'}",
            "kythe_kind: RECORD",
            "kythe_subkind: CLASS",
            "generic_kind: 'some_kind'",
            "generic_subkind: 'some_subkind'",
            "edge: {kythe_kind: PARAM target: {signature: 'tgt'}}",
            "kythe_kind: RECORD",
            "kythe_subkind: CLASS",
            "fact: {generic_name: '/fact/name' value: 'value'}",
            "fact: {generic_name: '/fact/name' value: 'value'}",
            "edge: {kythe_kind: TYPED target: {signature: 'tgt'}}",
            "edge: {kythe_kind: PARAM ordinal: 42 target: {signature: 'tgt'}}",
            "edge: {generic_kind: '/edge/kind' target: {signature: 'tgt'}}",
            "edge: {generic_kind: '/edge/kind' target: {signature: 'tgt'}}",
            "edge: {generic_kind: '/edge/kind' ordinal: 4 target: {signature: 'tgt'}}",
            "generic_kind: 'something'",
            "kythe_kind: RECORD",
            "generic_subkind: 'some_subkind'",
            "kythe_subkind: CLASS",
            "source: {signature: 's' corpus: 'c' path: 'p'}"
                + " kythe_kind: RECORD"
                + " kythe_subkind: CLASS"
                + " fact: {kythe_name: TEXT value: 'txt'}"
                + " fact: {generic_name: '/fact/name' value: 'anything'}"
                + " edge: {kythe_kind: PARAM target: {signature: 'tgt'}}"
                + " edge: {kythe_kind: PARAM ordinal: 1 target: {signature: 'tgt2'}}"
                + " edge: {generic_kind: '/edge/kind' target: {signature: 'tgt'}}"
                + " edge: {generic_kind: '/edge/kind' ordinal: 1 target: {signature: 'tgt3'}}"));
  }

  public void testConvertToSource() {
    testProtoConversion(
        "convertToSource",
        Nodes::convertToSource,
        Node.class,
        Source.class,
        ImmutableList.of(
            "source: {signature: 's' corpus: 'c'}",
            "fact: {kythe_name: NODE_KIND value: 'record'}",
            "fact: {generic_name: '/fact/name' value: 'value'}",
            "edge: {kythe_kind: TYPED target: {signature: 'tgt'}}",
            "edge: {kythe_kind: PARAM ordinal: 42 target: {signature: 'tgt'}}",
            "edge: {generic_kind: '/edge/kind' target: {signature: 'tgt'}}",
            "edge: {generic_kind: '/edge/kind' ordinal: 4 target: {signature: 'tgt'}}",
            "generic_kind: 'something'",
            "kythe_kind: RECORD",
            "generic_subkind: 'some_subkind'",
            "kythe_subkind: CLASS",
            "source: {signature: 's' corpus: 'c' path: 'p'}"
                + " kythe_kind: RECORD"
                + " kythe_subkind: CLASS"
                + " fact: {kythe_name: TEXT value: 'txt'}"
                + " fact: {generic_name: '/fact/name' value: 'anything'}"
                + " edge: {generic_kind: '/edge/kind' ordinal: 1 target: {signature: 'tgt3'}}"
                + " edge: {kythe_kind: PARAM target: {signature: 'tgt'}}"
                + " edge: {generic_kind: '/edge/kind' target: {signature: 'tgt'}}"
                + " edge: {kythe_kind: PARAM ordinal: 1 target: {signature: 'tgt2'}}"),
        ImmutableList.of(
            "ticket: 'kythe://c#s'",
            "ticket: 'kythe:' facts: {key: '/kythe/node/kind' value: 'record'}",
            "ticket: 'kythe:' facts: {key: '/fact/name' value: 'value'}",
            "ticket: 'kythe:' edge_groups: {"
                + "key: '/kythe/edge/typed' value: {edges: {ticket: 'kythe:#tgt'}}}",
            "ticket: 'kythe:' edge_groups: {"
                + "key: '/kythe/edge/param' value: {edges: {ticket: 'kythe:#tgt' ordinal: 42}}}",
            "ticket: 'kythe:' edge_groups: {"
                + "key: '/edge/kind' value: {edges: {ticket: 'kythe:#tgt'}}}",
            "ticket: 'kythe:' edge_groups: {"
                + "key: '/edge/kind' value: {edges: {ticket: 'kythe:#tgt' ordinal: 4}}}",
            "ticket: 'kythe:' facts: {key: '/kythe/node/kind' value: 'something'}",
            "ticket: 'kythe:' facts: {key: '/kythe/node/kind' value: 'record'}",
            "ticket: 'kythe:' facts: {key: '/kythe/subkind' value: 'some_subkind'}",
            "ticket: 'kythe:' facts: {key: '/kythe/subkind' value: 'class'}",
            "ticket: 'kythe://c?path=p#s'"
                + " facts: {key: '/kythe/node/kind' value: 'record'}"
                + " facts: {key: '/kythe/subkind' value: 'class'}"
                + " facts: {key: '/kythe/text' value: 'txt'}"
                + " facts: {key: '/fact/name' value: 'anything'}"
                + " edge_groups: {key: '/edge/kind' value: {"
                + "  edges: {ticket: 'kythe:#tgt'}"
                + "  edges: {ticket: 'kythe:#tgt3' ordinal: 1}}}"
                + " edge_groups: {key: '/kythe/edge/param' value: {"
                + "  edges: {ticket: 'kythe:#tgt'}"
                + "  edges: {ticket: 'kythe:#tgt2' ordinal: 1}}}"));
  }

  public void testFromSource() {
    testProtoConversion(
        "fromSource",
        Nodes::fromSource,
        Source.class,
        Node.class,
        ImmutableList.of(
            "ticket: 'kythe://c#s'",
            "ticket: 'kythe:#node' facts: {key: '/kythe/text/encoding' value: 'second'}"
                + " facts: {key: '/kythe/text' value: 'first'}",
            "ticket: 'kythe:#node' facts: {key: '/not/kythe/fact' value: 'val'}",
            "ticket: 'kythe:#node' edge_groups: {key: '/kythe/edge/param' value: {"
                + " edges: {ticket: 'kythe:#target' ordinal: 1}"
                + " edges: {ticket: 'kythe:#tgt' ordinal: 2}"
                + " edges: {ticket: 'kythe:#target' ordinal: 0}}}",
            "ticket: 'kythe:#node' edge_groups: {key: '/not/kythe/edge' value: {"
                + " edges: {ticket: 'kythe:#target'}}}",
            "ticket: 'kythe:#record' facts: {key: '/kythe/node/kind' value: 'record'}",
            "ticket: 'kythe:#record' facts: {key: '/kythe/node/kind' value: 'record'}"
                + " facts: {key: '/kythe/subkind' value: 'class'}",
            "ticket: 'kythe:#node' facts: {key: '/kythe/node/kind' value: 'something'}",
            "ticket: 'kythe:#node' facts: {key: '/kythe/subkind' value: 'something'}"),
        ImmutableList.of(
            "source: {corpus: 'c' signature: 's'}",
            "source: {signature: 'node'} fact: {kythe_name: TEXT value: 'first'}"
                + " fact: {kythe_name: TEXT_ENCODING value: 'second'}",
            "source: {signature: 'node'} fact: {generic_name: '/not/kythe/fact' value: 'val'}",
            "source: {signature: 'node'}"
                + " edge: {kythe_kind: PARAM ordinal: 0 target: {signature: 'target'}}"
                + " edge: {kythe_kind: PARAM ordinal: 1 target: {signature: 'target'}}"
                + " edge: {kythe_kind: PARAM ordinal: 2 target: {signature: 'tgt'}}",
            "source: {signature: 'node'}"
                + " edge: {generic_kind: '/not/kythe/edge' target: {signature: 'target'}}",
            "source: {signature: 'record'} kythe_kind: RECORD",
            "source: {signature: 'record'} kythe_kind: RECORD kythe_subkind: CLASS",
            "source: {signature: 'node'} generic_kind: 'something'",
            "source: {signature: 'node'} generic_subkind: 'something'"));
  }

  public void testConvertToSchemaEntry() {
    testProtoConversion(
        "convertToSchemaEntry",
        Nodes::convertToSchemaEntry,
        Storage.Entry.class,
        Entry.class,
        ImmutableList.of(
            "source: {signature: 's'} edge_kind: '/some/edge' target: {signature: 't'}",
            "source: {signature: 's'} edge_kind: '/kythe/edge/typed' target: {signature: 't'}",
            "source: {path: 'p'} edge_kind: '/kythe/edge/param.0' target: {path: 'p2'}",
            "source: {path: 'p'} edge_kind: '/kythe/edge/param.42' target: {path: 'p2'}",
            "source: {signature: 's'} fact_name: '/some/fact' fact_value: 'value'",
            "source: {signature: 's'} fact_name: '/kythe/node/kind' fact_value: 'record'"),
        ImmutableList.of(
            "edge: {source: {signature: 's'} generic_kind: '/some/edge' target: {signature: 't'}}",
            "edge: {source: {signature: 's'} kythe_kind: TYPED target: {signature: 't'}}",
            "edge: {source: {path: 'p'} kythe_kind: PARAM ordinal: 0 target: {path: 'p2'}}",
            "edge: {source: {path: 'p'} kythe_kind: PARAM ordinal: 42 target: {path: 'p2'}}",
            "fact: {source: {signature: 's'} generic_name: '/some/fact' value: 'value'}",
            "fact: {source: {signature: 's'} kythe_name: NODE_KIND value: 'record'}"));
  }

  private static final <I extends Message, O extends Message> void testProtoConversion(
      String name,
      Function<I, O> f,
      Class<I> protoInput,
      Class<O> protoOutput,
      ImmutableList<String> input,
      ImmutableList<String> expected) {
    assertThat(input).hasSize(expected.size());
    Streams.forEachPair(
        input.stream().map(protoParser(protoInput)),
        expected.stream().map(protoParser(protoOutput)),
        (in, ex) ->
            assertWithMessage(String.format("%s(`%s`)", name, TextFormat.shortDebugString(in)))
                .that(f.apply(in))
                .isEqualTo(ex));
  }

  private static final <T extends Message> Function<String, T> protoParser(
      final Class<T> protoClass) {
    return s -> {
      try {
        return TextFormat.parse(s, protoClass);
      } catch (TextFormat.ParseException e) {
        throw new IllegalArgumentException(e);
      }
    };
  }
}
