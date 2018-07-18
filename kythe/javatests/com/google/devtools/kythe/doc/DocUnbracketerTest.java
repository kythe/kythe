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

package com.google.devtools.kythe.doc;

import static com.google.common.truth.Truth.assertThat;

import com.google.common.collect.ImmutableList;
import com.google.common.html.types.SafeUrls;
import com.google.devtools.kythe.proto.Link;
import com.google.devtools.kythe.proto.Printable;
import java.util.List;
import junit.framework.TestCase;

public class DocUnbracketerTest extends TestCase {
  /**
   * Run a test case on {@code DocUnbracketer#unbracket}.
   *
   * @param rawText The escaped text for the document ("[hello]").
   * @param destinations A list of destination tokens. The nth token is provided to the unbracketing code
   *   as the nth link's definition URL. Empty tokens become null URLs; other tokens are turned into sanitized
   *   {@code SafeUrl} instances after being prepended with "http://".
   * @param expectedResult The expected HTML string, sans the outermost <pre> tag.
   */
  private static void makeTestCase(
      String rawText, List<String> destinations, String expectedResult) {
    Printable.Builder builder = Printable.newBuilder().setRawText(rawText);
    for (int i = 0; i < destinations.size(); ++i) {
      builder.addLink(Link.newBuilder().addDefinition(Integer.toString(i)));
    }
    String result =
        DocUnbracketer.unbracket(
                index ->
                    destinations.get(Integer.parseInt(index)).isEmpty()
                        ? null
                        : SafeUrls.sanitize("http://" + destinations.get(Integer.parseInt(index))),
                builder.build())
            .getSafeHtmlString();
    System.err.println("<pre>" + expectedResult + "</pre>");
    System.err.println(result);
    assertThat(result).isEqualTo("<pre>" + expectedResult + "</pre>");
  }

  private static String a(String href, String body) {
    return "<a href=\"http://" + href + "\">" + body + "</a>";
  }

  public void testUnbracket() {
    makeTestCase("hello", ImmutableList.of(), "hello");
    makeTestCase("\\[\\]\\\\", ImmutableList.of(), "[]\\");
    makeTestCase("[hello]", ImmutableList.of("a"), a("a", "hello"));
    makeTestCase("[hello]", ImmutableList.of(""), "hello");
    makeTestCase("[[hello]]", ImmutableList.of("a", "b"), a("b", "hello"));
    makeTestCase("[hello", ImmutableList.of("a"), a("a", "hello"));
    makeTestCase("[a]b[c]", ImmutableList.of("x", "y"), a("x", "a") + "b" + a("y", "c"));
    makeTestCase(
        "[[a]bc[d]e]f",
        ImmutableList.of("bce", "a", "d"),
        a("a", "a") + a("bce", "bc") + a("d", "d") + a("bce", "e") + "f");
    makeTestCase("[]a", ImmutableList.of("x"), "a");
    makeTestCase("[a[]a]", ImmutableList.of("a", "b"), a("a", "a") + a("a", "a"));
    makeTestCase("[a[b]a]", ImmutableList.of("a", ""), a("a", "a") + "b" + a("a", "a"));
    makeTestCase("[a]", ImmutableList.of(), "a");
  }
}
