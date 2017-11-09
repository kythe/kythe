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

import java.io.IOException;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.nio.charset.Charset;

/** Provides a mapping of file character offsets to byte offsets and line numbers. */
public class PositionMappings {
  private final int[] byteOffsets;
  private final int[] lineNumbers;

  /**
   * Constructs a new {@link PositionMappings} instance.
   *
   * @param encoding The encoding of the text to be mapped.
   * @param text The source text to be mapped.
   * @throws IllegalStateException If an error was encountered while attempting to read the source
   *     text.
   */
  public PositionMappings(Charset encoding, CharSequence text) {
    byteOffsets = new int[text.length() + 1];
    lineNumbers = new int[text.length() + 1];

    CountingOutputStream counter = new CountingOutputStream();
    OutputStreamWriter writer = new OutputStreamWriter(counter, encoding);
    for (int i = 0; i < text.length(); i++) {
      byteOffsets[i] = counter.getCount();
      lineNumbers[i] = counter.getLines() + 1;
      try {
        writer.append(text.charAt(i));
        writer.flush();
      } catch (IOException ioe) {
        throw new IllegalStateException(ioe);
      }
    }
    byteOffsets[text.length()] = counter.getCount();
    lineNumbers[text.length()] = counter.getLines() + 1;
  }

  /**
   * Returns the line number corresponding to the specified char offset.
   *
   * @param charOffset The char offset of the requested line.
   * @return The line number for the specified offset. -1 if the specified offset was out of bounds.
   */
  public int charToLine(int charOffset) {
    if (charOffset < 0) {
      return -1;
    } else if (charOffset >= lineNumbers.length) {
      System.err.printf(
          "WARNING: offset past end of source: %d >= %d\n", charOffset, lineNumbers.length);
      return -1;
    }
    return lineNumbers[charOffset];
  }

  /**
   * Returns the byte offset corresponding to the specified char offset.
   *
   * @param charOffset The char offset of the requested byte offset.
   * @return The byte offset for the specified char offset. -1 if the specified offset was out of
   *     bounds.
   */
  public int charToByteOffset(int charOffset) {
    if (charOffset < 0) {
      return -1;
    } else if (charOffset >= byteOffsets.length) {
      System.err.printf(
          "WARNING: offset past end of source: %d >= %d\n", charOffset, byteOffsets.length);
      return -1;
    }
    return byteOffsets[charOffset];
  }

  /** {@link OutputStream} that only counts each {@code byte} that should be written. */
  private static class CountingOutputStream extends OutputStream {
    private int count;
    private int lines;

    /** Returns the count of bytes that have been requested to be written. */
    public int getCount() {
      return count;
    }

    /** Returns the number of full lines that have been requested to be written. */
    public int getLines() {
      return lines;
    }

    @Override
    public void write(int b) {
      count++;
      if (b == '\n') {
        lines++;
      }
    }
  }
}
