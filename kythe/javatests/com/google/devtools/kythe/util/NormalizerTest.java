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

import com.google.devtools.kythe.proto.Point;
import com.google.devtools.kythe.proto.Serving.File;
import com.google.devtools.kythe.proto.Span;
import com.google.protobuf.ByteString;
import junit.framework.TestCase;

/** Unit tests for {@link Normalizer}. */
public final class NormalizerTest extends TestCase {

  private static final File TEST_FILE =
      File.newBuilder().setText(ByteString.copyFromUtf8("line\nanother\n\nafter empty")).build();
  private static final File TEST_FILE_WITH_TRAILING_NEWLINE =
      File.newBuilder().setText(ByteString.copyFromUtf8("a\nfile\n")).build();
  private static final File TEST_FILE_EMPTY = File.newBuilder().setText(ByteString.EMPTY).build();

  public void testExpander_expandLine() {
    Normalizer e = new Normalizer(TEST_FILE);
    assertThat(e.expandLine(0)).isEqualTo(span(point(0, 1, 0), point(4, 1, 4)));
    assertThat(e.expandLine(4)).isEqualTo(span(point(0, 1, 0), point(4, 1, 4)));
    assertThat(e.expandLine(5)).isEqualTo(span(point(5, 2, 0), point(12, 2, 7)));
    assertThat(e.expandLine(12)).isEqualTo(span(point(5, 2, 0), point(12, 2, 7)));
    assertThat(e.expandLine(100)).isEqualTo(span(point(14, 4, 0), point(25, 4, 11)));

    e = new Normalizer(TEST_FILE_WITH_TRAILING_NEWLINE);
    assertThat(e.expandLine(0)).isEqualTo(span(point(0, 1, 0), point(1, 1, 1)));
    assertThat(e.expandLine(6)).isEqualTo(span(point(2, 2, 0), point(6, 2, 4)));
    assertThat(e.expandLine(7)).isEqualTo(span(point(7, 3, 0), point(7, 3, 0)));
    assertThat(e.expandLine(70)).isEqualTo(span(point(7, 3, 0), point(7, 3, 0)));
  }

  public void testExpander_expandPoint() {
    Normalizer e = new Normalizer(TEST_FILE);

    assertThat(e.expandPoint(0)).isEqualTo(point(0, 1, 0));
    assertThat(e.expandPoint(1)).isEqualTo(point(1, 1, 1));
    assertThat(e.expandPoint(3)).isEqualTo(point(3, 1, 3));
    assertThat(e.expandPoint(4)).isEqualTo(point(4, 1, 4));
    assertThat(e.expandPoint(5)).isEqualTo(point(5, 2, 0));
    assertThat(e.expandPoint(9)).isEqualTo(point(9, 2, 4));
    assertThat(e.expandPoint(12)).isEqualTo(point(12, 2, 7));
    assertThat(e.expandPoint(13)).isEqualTo(point(13, 3, 0));
    assertThat(e.expandPoint(14)).isEqualTo(point(14, 4, 0));
    assertThat(e.expandPoint(25)).isEqualTo(point(25, 4, 11));

    assertThat(e.expandPoint(-1)).isEqualTo(point(0, 1, 0));
    assertThat(e.expandPoint(26)).isEqualTo(point(25, 4, 11));
    assertThat(e.expandPoint(100)).isEqualTo(point(25, 4, 11));
  }

  public void testExpander_withTrailingNewline() {
    Normalizer e = new Normalizer(TEST_FILE_WITH_TRAILING_NEWLINE);

    assertThat(e.expandPoint(0)).isEqualTo(point(0, 1, 0));
    assertThat(e.expandPoint(1)).isEqualTo(point(1, 1, 1));
    assertThat(e.expandPoint(2)).isEqualTo(point(2, 2, 0));
    assertThat(e.expandPoint(6)).isEqualTo(point(6, 2, 4));
    assertThat(e.expandPoint(7)).isEqualTo(point(7, 3, 0));

    assertThat(e.expandPoint(8)).isEqualTo(point(7, 3, 0));
    assertThat(e.expandPoint(100)).isEqualTo(point(7, 3, 0));
  }

  public void testExpander_empty() {
    Normalizer e = new Normalizer(TEST_FILE_EMPTY);

    assertThat(e.expandPoint(0)).isEqualTo(point(0, 1, 0));
    assertThat(e.expandPoint(1)).isEqualTo(point(0, 1, 0));
    assertThat(e.expandPoint(-1)).isEqualTo(point(0, 1, 0));
    assertThat(e.expandPoint(10)).isEqualTo(point(0, 1, 0));
  }

  private static Span span(Point start, Point end) {
    return Span.newBuilder().setStart(start).setEnd(end).build();
  }

  private static Point point(int offset, int line, int col) {
    return Point.newBuilder()
        .setByteOffset(offset)
        .setLineNumber(line)
        .setColumnOffset(col)
        .build();
  }
}
