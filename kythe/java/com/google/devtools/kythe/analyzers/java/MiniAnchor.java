/*
 * Copyright 2016 Google Inc. All rights reserved.
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

import com.google.common.base.Predicate;
import com.google.common.collect.Iterables;
import java.util.Collections;
import java.util.Comparator;
import java.util.Iterator;
import java.util.List;
import java.util.PriorityQueue;

/**
 * A span of text attached to an arbitrary object.
 *
 * @param <T> Type of object to which this MiniAnchor refers.
 */
public class MiniAnchor<T> {
  private final T anchoredTo;
  private final int begin;
  private final int end;

  /**
   * @param anchoredTo the object to associate with this span
   * @param begin the inclusive beginning of this span's text range
   * @param end the exclusive end of this span's text range
   */
  public MiniAnchor(T anchoredTo, int begin, int end) {
    this.anchoredTo = anchoredTo;
    this.begin = begin;
    this.end = end;
  }

  /** @return the inclusive beginning of the text span */
  public int getBegin() {
    return begin;
  }

  /** @return the non-exclusive end of the text span */
  public int getEnd() {
    return end;
  }

  /** @return the object this span is anchored to */
  public T getAnchoredTo() {
    return anchoredTo;
  }

  /** Used to transform source positions when bracketing documentation text. */
  public interface PositionTransform {
    /** Transforms an integer offset. */
    public int transform(int inputPos);
  }

  /**
   * Returns a new string with brackets added to {@code text} (escaping existing ones) and sorts
   * {@code miniAnchors} such that the ith {@link MiniAnchor} corresponds to the ith opening
   * bracket. Drops empty or negative-length spans.
   *
   * @see <a href="http://www.kythe.io/docs/schema/#doc">the Kythe schema section on doc nodes</a>
   * @param text the text to bracket
   * @param posTransform a function that will be applied to transform offsets in {@code text} to
   *     anchor offsets
   * @param miniAnchors a mutable list of {@link MiniAnchor} instances to sort and filter. This list
   *     must support removal
   * @return {@code text} with brackets added and escaped as necessary.
   */
  public static <T> String bracket(
      String text, PositionTransform posTransform, List<MiniAnchor<T>> miniAnchors) {
    Iterables.removeIf(
        miniAnchors,
        new Predicate<MiniAnchor<T>>() {
          @Override
          public boolean apply(MiniAnchor<T> a) {
            // Drop empty or negative-length spans. These are not useful for presentation or
            // are invalid.
            return a.begin >= a.end;
          }
        });
    Collections.sort(miniAnchors,
        new Comparator<MiniAnchor<T>>() {
          @Override
          public int compare(MiniAnchor<T> l, MiniAnchor<T> r) {
            return l.begin == r.begin ? r.end - l.end : l.begin - r.begin;
          }
        });
    StringBuilder bracketed = new StringBuilder(text.length() + miniAnchors.size() * 2);
    Iterator<MiniAnchor<T>> anchors = miniAnchors.iterator();
    MiniAnchor<T> nextAnchor = anchors.hasNext() ? anchors.next() : null;
    PriorityQueue<Integer> ends = new PriorityQueue<Integer>();
    for (int i = 0; i < text.length(); ++i) {
      int sourcePos = posTransform.transform(i);
      char c = text.charAt(i);
      while (!ends.isEmpty() && ends.peek() == sourcePos) {
        ends.poll();
        bracketed.append(']');
      }
      while (nextAnchor != null && nextAnchor.begin == sourcePos) {
        bracketed.append('[');
        ends.add(nextAnchor.end);
        nextAnchor = anchors.hasNext() ? anchors.next() : null;
      }
      if (c == '[' || c == ']' || c == '\\') {
        bracketed.append('\\');
      }
      bracketed.append(c);
    }
    for (int i = 0; i < ends.size(); ++i) {
      bracketed.append(']');
    }
    return bracketed.toString();
  }
}
