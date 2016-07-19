/*
 * Copyright 2015 Google Inc. All rights reserved.
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

package com.google.devtools.kythe.analyzers.java;

import static com.google.common.truth.Truth.assertThat;
import static com.google.devtools.kythe.analyzers.java.SourceText.PositionMappings;
import static java.nio.charset.StandardCharsets.US_ASCII;
import static java.nio.charset.StandardCharsets.UTF_16;
import static java.nio.charset.StandardCharsets.UTF_8;

import java.nio.charset.Charset;
import junit.framework.TestCase;

/** Tests {@link SourceText} */
public class SourceTextTest extends TestCase {
  public void testASCII() {
    String text = "Hello\nWorld!";
    assertOffsetsMatchSubstrings(US_ASCII, text);

    int[] ao = new PositionMappings(US_ASCII, text).byteOffsets;
    assertThat(ao).hasLength(text.length() + 1);
    for (int i = 0; i < ao.length; i++) {
      assertThat(ao[i]).named("ao[" + i + "]").isEqualTo(i);
    }

    int[] au = new PositionMappings(UTF_8, text).byteOffsets;
    assertThat(ao).isEqualTo(au);
  }

  public void testUTF8_japanese() {
    String text = "これは、本当に長い文字列です";
    assertOffsetsMatchSubstrings(UTF_8, text);
  }

  public void testUTF16_japanese() {
    String text = "何？他の日本語文？";
    assertOffsetsMatchSubstrings(UTF_16, text);
  }

  public void testUTF8_spanish() {
    String text = "¡Sí, este es el español!";
    assertOffsetsMatchSubstrings(UTF_8, text);
  }

  private static void assertOffsetsMatchSubstrings(Charset charset, String text) {
    int[] offsets = new PositionMappings(charset, text).byteOffsets;

    // Check edges first
    assertThat(offsets[0]).isEqualTo(0);
    assertThat(offsets[text.length()]).isEqualTo(text.getBytes(charset).length);

    for (int i = 0; i < offsets.length; i++) {
      String ss = text.substring(0, i);
      assertThat(offsets[i]).named("offsets[" + i + "]").isEqualTo(ss.getBytes(charset).length);
    }
  }
}
