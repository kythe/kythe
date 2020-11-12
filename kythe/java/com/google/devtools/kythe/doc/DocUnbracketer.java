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

import com.google.common.html.types.SafeHtml;
import com.google.common.html.types.SafeHtmlBuilder;
import com.google.common.html.types.SafeUrl;
import com.google.devtools.kythe.proto.Printable;
import java.util.Stack;
import java.util.function.Function;

/** Unescapes text from Kythe doc nodes into SafeHtml. */
public class DocUnbracketer {
  private static final String ALLOWED_ESCAPES = "[]\\";

  /**
   * Unbracket {@code printable}, following the rules for text in {@link Printable} messages.
   *
   * @param makeLink if provided, this function will be used to generate link URIs from semantic
   *     node tickets. It may return null if there is no available URI.
   * @param printable the document text to unbracket.
   */
  public static SafeHtml unbracket(Function<String, SafeUrl> makeLink, Printable printable) {
    int size = printable.getRawText().length();
    String raw = printable.getRawText();
    SafeHtmlBuilder builder = new SafeHtmlBuilder("pre");
    SafeHtmlBuilder linkBuilder = null; // is non-null if we are inside a link
    boolean linkTextIsEmpty = true;
    int linkNumber = 0; // is equal to the number of link-opening [s we've seen so far
    Stack<SafeUrl> links =
        new Stack<SafeUrl>(); // is always as high as the nesting level, for well-formed documents

    for (int c = 0; c < size; ++c) {
      char ch = raw.charAt(c);
      if (ch == '\\' && c + 1 < size && ALLOWED_ESCAPES.indexOf(raw.charAt(c + 1)) != -1) {
        if (linkBuilder != null) {
          linkBuilder = linkBuilder.escapeAndAppendContent(raw.substring(c + 1, c + 2));
          linkTextIsEmpty = false;
        } else {
          builder = builder.escapeAndAppendContent(raw.substring(c + 1, c + 2));
        }
        ++c;
      } else if (ch == '[') {
        if (linkBuilder != null && !linkTextIsEmpty) {
          builder = builder.appendContent(linkBuilder.build());
        }
        linkBuilder = null;
        linkTextIsEmpty = true;
        SafeUrl link = null;
        if (linkNumber < printable.getLinkCount() && makeLink != null) {
          // Pick the first definition ticket that will provide a location for a link.
          // (We expect to see only one definition ticket per link in general.)
          for (String definition : printable.getLink(linkNumber).getDefinitionList()) {
            link = makeLink.apply(definition);
            if (link != null) {
              break;
            }
          }
          ++linkNumber;
        }
        links.push(link);
        if (link != null) {
          linkBuilder = new SafeHtmlBuilder("a").setHref(link);
          linkTextIsEmpty = true;
        }
      } else if (ch == ']') {
        if (linkBuilder != null) {
          if (!linkTextIsEmpty) {
            builder = builder.appendContent(linkBuilder.build());
          }
          linkBuilder = null;
        }
        if (!links.empty()) {
          links.pop();
          if (!links.empty() && links.peek() != null) {
            linkBuilder = new SafeHtmlBuilder("a").setHref(links.peek());
            linkTextIsEmpty = true;
          }
        }
      } else if (linkBuilder != null) {
        linkBuilder = linkBuilder.escapeAndAppendContent(raw.substring(c, c + 1));
        linkTextIsEmpty = false;
      } else {
        builder = builder.escapeAndAppendContent(raw.substring(c, c + 1));
      }
    }
    if (linkBuilder != null) {
      builder = builder.appendContent(linkBuilder.build());
    }
    return builder.build();
  }
}
