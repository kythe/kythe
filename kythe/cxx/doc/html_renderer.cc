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
                       const Printable& printable) {
  std::string text_out;
  text_out.reserve(printable.text().size());
  enum class PrintStatus { Raw, UserHtml };
  struct OpenSpan {
    const PrintableSpan* span;
    PrintStatus status;
  };
  std::stack<OpenSpan> open_spans;
  PrintableSpan default_span(0, printable.text().size(),
                             PrintableSpan::Semantic::Raw);
  open_spans.push(OpenSpan{&default_span, PrintStatus::Raw});
  size_t current_span = 0;
  for (size_t i = 0; i <= printable.text().size(); ++i) {
    // Normalized PrintableSpans have all empty or negative-length spans
    // dropped.
    while (!open_spans.empty() && open_spans.top().span->end() == i) {
      switch (open_spans.top().span->semantic()) {
        case PrintableSpan::Semantic::Link:
          if (open_spans.top().span->link().definition_size() != 0) {
            text_out.append("</a>");
          }
          break;
        case PrintableSpan::Semantic::Return:
          text_out.append("</p>");
          break;
        case PrintableSpan::Semantic::CodeRef:
          text_out.append("</tt>");
          break;
        default:
          break;
      }
      open_spans.pop();
    }
    if (open_spans.empty() || i == printable.text().size()) {
      // default_span is first to enter and last to leave; there also may
      // be no empty spans.
      break;
    }
    while (current_span < printable.spans().size() &&
           printable.spans().span(current_span).begin() == i) {
      open_spans.push(
          {&printable.spans().span(current_span), open_spans.top().status});
      ++current_span;
      switch (open_spans.top().span->semantic()) {
        case PrintableSpan::Semantic::Link:
          if (open_spans.top().span->link().definition_size() != 0) {
            const auto& definition =
                open_spans.top().span->link().definition(0);
            text_out.append("<a href=\"");
            auto link_uri = options.make_link_uri(definition);
            // + 2 for the closing ">.
            text_out.reserve(text_out.size() + link_uri.size() + 2);
            for (auto c : link_uri) {
              AppendEscapedHtmlCharacter(&text_out, c);
            }
            text_out.append("\">");
          }
          break;
        case PrintableSpan::Semantic::Html:
          open_spans.top().status = PrintStatus::UserHtml;
          break;
        case PrintableSpan::Semantic::Raw:
          open_spans.top().status = PrintStatus::Raw;
          break;
        case PrintableSpan::Semantic::Return:
          text_out.append("<p><em>Returns: </em>");
          break;
        case PrintableSpan::Semantic::CodeRef:
          text_out.append("<tt>");
          break;
        default:
          break;
      }
    }
    if (open_spans.top().span->semantic() != PrintableSpan::Semantic::Markup) {
      // TODO(zarko): Sanitization for user-provided HTML. Do we need to handle
      // cases where trusted markup is nested inside untrusted markup or can
      // we simply sanitize *all* HTML?
      char c = printable.text()[i];
      switch (open_spans.top().status) {
        case PrintStatus::Raw:
          AppendEscapedHtmlCharacter(&text_out, c);
          break;
        case PrintStatus::UserHtml:
          text_out.push_back(c);
          break;
      }
    }
  }
  return text_out;
}

}  // namespace kythe
