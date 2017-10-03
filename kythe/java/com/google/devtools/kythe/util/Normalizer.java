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

package com.google.devtools.kythe.util;

import com.google.devtools.kythe.proto.Point;
import com.google.devtools.kythe.proto.Serving.ExpandedAnchor;
import com.google.devtools.kythe.proto.Serving.File;
import com.google.devtools.kythe.proto.Serving.RawAnchor;
import com.google.devtools.kythe.proto.Span;
import com.google.protobuf.ByteString;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

/**
 * Utility class to expand byte offsets into full file positions.
 *
 * <p>See https://godoc.org/kythe.io/kythe/go/services/xrefs#Normalizer for the corresponding
 * implementation in the Go services.
 */
public final class Normalizer {
  private static final byte LINE_END = '\n';

  private final File file;

  /** Total length of file text in bytes. */
  private final int textLength;
  /** Whether the file has a trailing newline. */
  private final boolean trailingNewline;
  /** List of ordered line lengths (index N -> line N+1's length). */
  private final List<Integer> lineLength;
  /** List of prefix lengths (index N -> length of file behind line N+1). */
  private final List<Integer> prefixLength;

  public Normalizer(File file) {
    this.file = file;
    ByteString text = file.getText();
    this.textLength = text.size();
    this.trailingNewline = textLength > 0 && text.byteAt(textLength - 1) == LINE_END;
    this.lineLength = new ArrayList<>();
    this.prefixLength = new ArrayList<>();
    prefixLength.add(0);
    for (int i = 0; i < text.size(); i++) {
      byte b = text.byteAt(i);
      if (b == LINE_END) {
        appendLine(i, true);
      }
    }
    if (!trailingNewline) {
      appendLine(text.size(), false);
    }
  }

  private void appendLine(int pos, boolean withEnding) {
    int length = pos - prefixLength.get(prefixLength.size() - 1) + (withEnding ? 1 : 0);
    lineLength.add(length);
    prefixLength.add(prefixLength.get(prefixLength.size() - 1) + length);
  }

  /** Returns a {@link Span} for the line containing the given byte offset. */
  public Span expandLine(int offset) {
    int line = getLineNumber(boundOffset(offset));
    Point start =
        Point.newBuilder().setByteOffset(prefixLength.get(line - 1)).setLineNumber(line).build();
    Point end;
    if (line == lineLength.size() + 1) {
      if (!trailingNewline) {
        throw new IllegalStateException();
      }
      end = start;
    } else {
      int len = lineLength.get(line - 1);
      if (line != lineLength.size() || trailingNewline) {
        len--;
      }
      end =
          Point.newBuilder()
              .setLineNumber(line)
              .setByteOffset(start.getByteOffset() + len)
              .setColumnOffset(len)
              .build();
    }
    return Span.newBuilder().setStart(start).setEnd(end).build();
  }

  /** Returns an expanded {@link Point} with line/column numbers based on a byte offset. */
  public Point expandPoint(int offset) {
    offset = boundOffset(offset);
    Point.Builder p = Point.newBuilder().setByteOffset(offset);

    p.setLineNumber(getLineNumber(offset));
    p.setColumnOffset(offset - prefixLength.get(p.getLineNumber() - 1));

    return p.build();
  }

  /** Returns a {@link Span} with both offsets expanded fully into {@link Point}s. */
  public Span expandPoints(int start, int end) {
    return Span.newBuilder().setStart(expandPoint(start)).setEnd(expandPoint(end)).build();
  }

  private int getLineNumber(int offset) {
    assert offset >= 0 && offset <= textLength;
    int i = Collections.binarySearch(prefixLength, offset, (o1, o2) -> o1 - o2);
    return i >= 0 ? i + (i == lineLength.size() && !trailingNewline ? 0 : 1) : -(i + 1);
  }

  private int boundOffset(int offset) {
    if (offset > textLength) {
      return textLength;
    } else if (offset < 0) {
      return 0;
    }
    return offset;
  }

  public ExpandedAnchorBuilder expandAnchor(RawAnchor raw) {
    return new ExpandedAnchorBuilder(raw);
  }

  public class ExpandedAnchorBuilder {
    private final RawAnchor raw;
    private final ExpandedAnchor.Builder expansion = ExpandedAnchor.newBuilder();

    private ExpandedAnchorBuilder(RawAnchor raw) {
      this.raw = raw;

      expansion
          .setTicket(raw.getTicket())
          .setSpan(expandPoints(raw.getStartOffset(), raw.getEndOffset()));
    }

    public ExpandedAnchorBuilder withText() {
      expansion.setText(getText(expansion.getSpan()));
      return this;
    }

    private String getText(Span span) {
      int spanLength = span.getEnd().getByteOffset() - span.getStart().getByteOffset();
      if (spanLength <= 0) {
        return "";
      }
      return file.getText()
          .substring(span.getStart().getByteOffset(), span.getEnd().getByteOffset())
          .toStringUtf8();
    }

    public ExpandedAnchorBuilder withSnippet(boolean fallbackToLineSnippet) {
      Span snippetSpan = expandPoints(raw.getSnippetStart(), raw.getSnippetEnd());
      String snippetText = getText(snippetSpan);
      if (snippetText.isEmpty() && fallbackToLineSnippet) {
        return withLineSnippet();
      }
      expansion.setSnippet(snippetText);
      return this;
    }

    public ExpandedAnchorBuilder withLineSnippet() {
      Span snippetSpan = expandLine(expansion.getSpan().getStart().getByteOffset());
      expansion.setText(getText(snippetSpan));
      return this;
    }

    public ExpandedAnchor build() {
      return expansion.build();
    }
  }
}
