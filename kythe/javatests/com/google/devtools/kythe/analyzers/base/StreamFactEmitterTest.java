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

package com.google.devtools.kythe.analyzers.base;

import static com.google.common.truth.Truth.assertThat;
import static java.nio.charset.StandardCharsets.UTF_8;

import com.google.devtools.kythe.proto.Storage.Entry;
import com.google.devtools.kythe.proto.Storage.VName;
import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import junit.framework.TestCase;

/** Tests {@link StreamFactEmitter} */
public class StreamFactEmitterTest extends TestCase {
  private StreamFactEmitter emitter;
  private ByteArrayOutputStream outputStream;

  @Override
  public void setUp() {
    this.outputStream = new ByteArrayOutputStream();
    this.emitter = new StreamFactEmitter(this.outputStream);
  }

  public void testEmitEntry() throws Exception {
    VName testVName = VName.newBuilder().setSignature("testSignature").build();
    String testFactName = "testFactName";
    String testFactValue = "testFactValue";
    this.emitter.emit(testVName, null, null, testFactName, testFactValue.getBytes(UTF_8));
    this.outputStream.flush();

    ByteArrayInputStream inputStream = new ByteArrayInputStream(this.outputStream.toByteArray());
    Entry.Builder builder = Entry.newBuilder();
    builder.mergeDelimitedFrom(inputStream);
    Entry entry = builder.build();

    assertThat(entry).isNotNull();
    assertThat(testVName).isEqualTo(entry.getSource());
    assertThat(testFactName).isEqualTo(entry.getFactName());
    assertThat(testFactValue).isEqualTo(entry.getFactValue().toString(UTF_8));
  }
}
