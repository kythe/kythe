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

#include "kythe/cxx/doc/html_renderer.h"

#include <stack>

namespace kythe {

/// \brief Appends a representation of `c` to `buffer`, possibly using an HTML
/// entity instead of a literal character.
void AppendEscapedHtmlCharacter(std::string* buffer, char c) {
  switch (c) {
    case '<':
      buffer->append("&lt;");
      break;
    case '>':
      buffer->append("&gt;");
      break;
    case '&':
      buffer->append("&amp;");
      break;
    default:
      buffer->push_back(c);
  }
}

std::string RenderHtml(const HtmlRendererOptions& options,
                       const proto::DocumentationReply_Document& document) {
  if (!document.has_text()) {
    return "";
  }
  const auto& printable = document.text();
  std::string text_out;
  text_out.reserve(printable.raw_text().size());
  int link_index = 0;
  std::stack<bool> link_valid;
  bool escaping_next = false;
  for (auto c : printable.raw_text()) {
    if (escaping_next) {
      AppendEscapedHtmlCharacter(&text_out, c);
      escaping_next = false;
      continue;
    }
    if (c == '\\') {
      escaping_next = true;
    } else if (c == '[') {
      if (link_index < printable.link_size()) {
        if (printable.link(link_index).definition_size() != 0) {
          const auto& definition = printable.link(link_index++).definition(0);
          text_out.append("<a href=\"");
          text_out.append(options.make_link_uri(definition));
          text_out.append("\">");
          link_valid.push(true);
        } else {
          ++link_index;
          link_valid.push(false);
        }
      } else {
        link_valid.push(false);
      }
    } else if (c == ']') {
      if (!link_valid.empty() && link_valid.top()) {
        text_out.append("</a>");
        link_valid.pop();
      }
    } else {
      AppendEscapedHtmlCharacter(&text_out, c);
    }
  }
  return text_out;
}

}  // namespace kythe
