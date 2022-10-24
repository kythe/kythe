/*
 * Copyright 2018 The Kythe Authors. All rights reserved.
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

package com.google.devtools.kythe.analyzers.jvm;

import static com.google.common.truth.Truth.assertThat;

import com.google.common.collect.ImmutableList;
import com.google.devtools.kythe.analyzers.jvm.JvmGraph.Type;
import junit.framework.TestCase;

/** Tests for the {@link JvmGraph} class. */
@SuppressWarnings("BadImport")
public final class JvmGraphTest extends TestCase {
  public void testTypeDescriptors() {
    Type.ReferenceType objectType = Type.referenceType("java.lang.Object");
    assertThat(objectType.toString()).isEqualTo("Ljava/lang/Object;");
    assertThat(Type.referenceType("java/lang/Object")).isEqualTo(objectType);
    assertThat(Type.referenceType("java.util.Map$Entry").toString())
        .isEqualTo("Ljava/util/Map$Entry;");
    assertThat(Type.booleanType().toString()).isEqualTo("Z");
    assertThat(Type.byteType().toString()).isEqualTo("B");
    assertThat(Type.arrayType(objectType).toString()).isEqualTo("[Ljava/lang/Object;");
    assertThat(Type.methodType(ImmutableList.of(), Type.voidType()).toString()).isEqualTo("()V");
    assertThat(
            Type.methodType(ImmutableList.of(objectType, Type.booleanType()), objectType)
                .toString())
        .isEqualTo("(Ljava/lang/Object;Z)Ljava/lang/Object;");
  }
}
