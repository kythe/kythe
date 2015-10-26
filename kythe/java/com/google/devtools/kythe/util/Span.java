/*
 * Copyright 2014 Google Inc. All rights reserved.
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

/** Structure representing some arbitrary offset span. */
public class Span implements Comparable<Span> {
  private final int start, end;

  public Span(int startOffset, int endOffset) {
    this.start = startOffset;
    this.end = endOffset;
  }

  public int getStart() {
    return start;
  }

  public int getEnd() {
    return end;
  }

  public boolean valid() {
    return start <= end && start >= 0;
  }

  /** Determines if the given integer is contained within {@code this} {@link Span}. */
  public boolean contains(int n) {
    return getStart() <= n && n < getEnd();
  }

  @Override
  public int compareTo(Span other) {
    if (other.start == this.start) {
      return this.end - other.end;
    }
    return this.start - other.start;
  }

  @Override
  public String toString() {
    return String.format("Span{%d, %d}", start, end);
  }
}
