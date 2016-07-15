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

import static com.google.common.collect.Lists.newArrayList;
import static com.google.common.truth.Truth.assertThat;

import java.util.ArrayList;
import java.util.List;
import junit.framework.TestCase;

/** Tests {@link MiniAnchor} */
public class MiniAnchorTest extends TestCase {
  private static final MiniAnchor.PositionTransform identityTransform =
      new MiniAnchor.PositionTransform() {
        @Override
        public int transform(int pos) {
          return pos;
        }
      };

  private static MiniAnchor<String> makeAnchor(String anchoredTo, int begin, int end) {
    return new MiniAnchor<String>(anchoredTo, begin, end);
  }

  private void singleCase(
      String text,
      List<MiniAnchor<String>> anchors,
      String resultText,
      List<String> resultAnchors) {
    String output = MiniAnchor.bracket(text, identityTransform, anchors);
    assertThat(output).isEqualTo(resultText);
    assertThat(anchors.size()).isEqualTo(resultAnchors.size());
    for (int i = 0; i < resultAnchors.size(); ++i) {
      assertThat(anchors.get(i).getAnchoredTo()).isEqualTo(resultAnchors.get(i));
    }
  }

  public void testEmpty() {
    singleCase("", new ArrayList<MiniAnchor<String>>(), "", new ArrayList<String>());
  }

  public void testEscape() {
    singleCase("\\[]", new ArrayList<MiniAnchor<String>>(), "\\\\\\[\\]", new ArrayList<String>());
  }

  public void testBad() {
    singleCase(
        "ccc",
        newArrayList(makeAnchor("aaa", 3, 0), makeAnchor("bbb", 1, 1)),
        "ccc",
        new ArrayList<String>());
  }

  public void testFullAnchor() {
    singleCase("foo", newArrayList(makeAnchor("bar", 0, 3)), "[foo]", newArrayList("bar"));
  }

  public void testRightPadAnchor() {
    singleCase("foo ", newArrayList(makeAnchor("bar", 0, 3)), "[foo] ", newArrayList("bar"));
  }

  public void testLeftPadAnchor() {
    singleCase(" foo", newArrayList(makeAnchor("bar", 1, 4)), " [foo]", newArrayList("bar"));
  }

  public void testMiddleNest() {
    singleCase(
        "aba",
        newArrayList(makeAnchor("bbb", 1, 2), makeAnchor("aaa", 0, 3)),
        "[a[b]a]",
        newArrayList("aaa", "bbb"));
  }

  public void testInterposedNest() {
    singleCase(
        "ababa",
        newArrayList(makeAnchor("bbb", 1, 2), makeAnchor("aaa", 0, 5), makeAnchor("ccc", 3, 4)),
        "[a[b]a[b]a]",
        newArrayList("aaa", "bbb", "ccc"));
  }

  public void testLeftOverlappingNest() {
    singleCase(
        "aba",
        newArrayList(makeAnchor("bbb", 0, 2), makeAnchor("aaa", 0, 3)),
        "[[ab]a]",
        newArrayList("aaa", "bbb"));
  }

  public void testJuxtaposed() {
    singleCase(
        "ab",
        newArrayList(makeAnchor("bbb", 1, 2), makeAnchor("aaa", 0, 1)),
        "[a][b]",
        newArrayList("aaa", "bbb"));
  }

  public void testOverlappingMiddleNest() {
    List<MiniAnchor<String>> anchors =
        newArrayList(makeAnchor("bbb", 1, 2), makeAnchor("aaa", 0, 3), makeAnchor("ccc", 1, 2));
    String output = MiniAnchor.bracket("aba", identityTransform, anchors);
    assertThat(output).isEqualTo("[a[[b]]a]");
    assertThat(anchors.get(0).getAnchoredTo()).isEqualTo("aaa");
    String second = anchors.get(1).getAnchoredTo();
    String third = anchors.get(2).getAnchoredTo();
    assertThat(
            second.equals("bbb") && third.equals("ccc")
                || second.equals("ccc") && third.equals("bbb"))
        .isTrue();
  }
}
