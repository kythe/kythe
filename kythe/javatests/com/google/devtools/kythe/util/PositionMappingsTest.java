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
import static com.google.common.truth.Truth.assertWithMessage;
import static java.nio.charset.StandardCharsets.US_ASCII;
import static java.nio.charset.StandardCharsets.UTF_16;
import static java.nio.charset.StandardCharsets.UTF_8;

import java.nio.charset.Charset;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

/** Tests {@link com.google.devtools.kythe.util.PositionMappings} */
@RunWith(JUnit4.class)
public class PositionMappingsTest {
  @Test
  public void testASCII() {
    String text = "Hello\nWorld!";
    Charset charset = US_ASCII;
    PositionMappings mapping = new PositionMappings(charset, text);
    assertOffsetsMatchSubstrings(mapping, text, charset);
    assertLineCountsAreCorrect(mapping, text);
  }

  @Test
  public void testUTF8_japanese() {
    String text = "これは、本当に長い文字列です";
    Charset charset = UTF_8;
    PositionMappings mapping = new PositionMappings(charset, text);
    assertOffsetsMatchSubstrings(mapping, text, charset);
    assertLineCountsAreCorrect(mapping, text);
  }

  @Test
  public void testUTF16_japanese() {
    String text = "何？他の日本語文？";
    Charset charset = UTF_16;
    PositionMappings mapping = new PositionMappings(charset, text);
    assertOffsetsMatchSubstrings(mapping, text, charset);
    assertLineCountsAreCorrect(mapping, text);
  }

  @Test
  public void testUTF8_spanish() {
    String text = "¡Sí, este es el español!";
    Charset charset = UTF_8;
    PositionMappings mapping = new PositionMappings(charset, text);
    assertOffsetsMatchSubstrings(mapping, text, charset);
    assertLineCountsAreCorrect(mapping, text);
  }

  @Test
  public void testCharToByteOffsetHandlesOutOfBounds() {
    String text = "¡Sí, este es el español!";
    PositionMappings mapping = new PositionMappings(UTF_8, text);
    assertThat(mapping.charToByteOffset(-1)).isEqualTo(-1);
    assertThat(mapping.charToByteOffset(text.length() + 1)).isEqualTo(-1);
  }

  @Test
  public void testCharToLineHandlesOutOfBounds() {
    String text = "何？他の日本語文？";
    PositionMappings mapping = new PositionMappings(UTF_16, text);
    assertThat(mapping.charToLine(-1)).isEqualTo(-1);
    assertThat(mapping.charToLine(text.length() + 1)).isEqualTo(-1);
  }

  private static void assertLineCountsAreCorrect(PositionMappings mapping, String text) {
    int lineCount = 1;
    for (int i = 0; i < text.length(); i++) {
      assertThat(mapping.charToLine(i)).isEqualTo(lineCount);

      if (text.charAt(i) == '\n') {
        lineCount++;
      }
    }
  }

  private static void assertOffsetsMatchSubstrings(
      PositionMappings positionMappings, String text, Charset charset) {
    // Check edges first
    assertThat(positionMappings.charToByteOffset(0)).isEqualTo(0);
    assertThat(positionMappings.charToByteOffset(text.length()))
        .isEqualTo(text.getBytes(charset).length);

    // check for correct byte offsets at each index in the text
    for (int i = 0; i < text.length(); i++) {
      String ss = text.substring(0, i);
      assertWithMessage("offsets[" + i + "]")
          .that(positionMappings.charToByteOffset(i))
          .isEqualTo(ss.getBytes(charset).length);
    }
  }
}
