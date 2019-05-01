/*
 * Copyright 2016 The Kythe Authors. All rights reserved.
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

#include "kythe/cxx/doc/markup_handler.h"

#include <algorithm>
#include <stack>

namespace kythe {

void PrintableSpans::Merge(const PrintableSpans& o) {
  spans_.insert(spans_.end(), o.spans_.begin(), o.spans_.end());
  spans_.erase(
      std::remove_if(spans_.begin(), spans_.end(),
                     [](const PrintableSpan& s) { return !s.is_valid(); }),
      spans_.end());
  std::sort(spans_.begin(), spans_.end());
}

namespace {
bool ShouldReject(Printable::RejectPolicy filter, size_t important_count,
                  size_t list_count) {
  return !((filter & Printable::IncludeLists) == Printable::IncludeLists &&
           list_count != 0) &&
         ((((filter & Printable::RejectUnimportant) ==
            Printable::RejectUnimportant) &&
           important_count == 0) ||
          (((filter & Printable::RejectLists) == Printable::RejectLists) &&
           list_count != 0));
}
}  // anonymous namespace

Printable::Printable(const proto::Printable& from,
                     Printable::RejectPolicy filter) {
  text_.reserve(from.raw_text().size() - from.link_size() * 2);
  size_t current_link = 0;
  std::stack<size_t> unclosed_links;
  // Number of important blocks we're inside.
  size_t important_count = 0;
  // Number of list blocks we're inside.
  size_t list_count = 0;
  for (size_t i = 0; i < from.raw_text().size(); ++i) {
    char c = from.raw_text()[i], next = (i + 1 == from.raw_text().size())
                                            ? '\0'
                                            : from.raw_text()[i + 1];
    switch (c) {
      case '\\':
        if (!ShouldReject(filter, important_count, list_count)) {
          text_.push_back(next);
        }
        ++i;
        break;
      case '[':
        if (current_link < from.link_size()) {
          unclosed_links.push(spans_.size());
          switch (from.link(current_link).kind()) {
            case proto::common::Link::LIST:
              ++list_count;
              break;
            case proto::common::Link::IMPORTANT:
              ++important_count;
              break;
            default:
              break;
          }
          if (!ShouldReject(filter, important_count, list_count)) {
            spans_.Emplace(text_.size(), text_.size(),
                           from.link(current_link++));
          }
        }
        break;
      case ']':
        if (!unclosed_links.empty()) {
          if (!ShouldReject(filter, important_count, list_count)) {
            spans_.mutable_span(unclosed_links.top())->set_end(text_.size());
          }
          switch (spans_.span(unclosed_links.top()).link().kind()) {
            case proto::common::Link::LIST:
              --list_count;
              break;
            case proto::common::Link::IMPORTANT:
              --important_count;
              break;
            default:
              break;
          }
          unclosed_links.pop();
        }
        break;
      default:
        if (!ShouldReject(filter, important_count, list_count)) {
          text_.push_back(c);
        }
        break;
    }
  }
  while (!unclosed_links.empty()) {
    spans_.mutable_span(unclosed_links.top())->set_end(text_.size());
    unclosed_links.pop();
  }
}

Printable HandleMarkup(const std::vector<MarkupHandler>& handlers,
                       const Printable& printable) {
  if (handlers.empty()) {
    return printable;
  }
  PrintableSpans spans = printable.spans();
  for (const auto& handler : handlers) {
    PrintableSpans next_spans;
    handler(printable, spans, &next_spans);
    spans.Merge(next_spans);
  }
  return Printable(printable.text(), std::move(spans));
}

std::string PrintableSpans::Dump(const std::string& annotated_buffer) const {
  std::string text_out;
  text_out.reserve(annotated_buffer.size());
  std::stack<const PrintableSpan*> open_spans;
  PrintableSpan default_span(0, annotated_buffer.size(),
                             PrintableSpan::Semantic::Raw);
  open_spans.push(&default_span);
  text_out.append("[");
  size_t current_span = 0;
  for (size_t i = 0; i <= annotated_buffer.size(); ++i) {
    while (!open_spans.empty() && open_spans.top()->end() == i) {
      text_out.append("]");
      open_spans.pop();
    }
    if (open_spans.empty() || i == annotated_buffer.size()) {
      // default_span is first to enter and last to leave; there also may
      // be no empty spans.
      break;
    }
    while (current_span < spans_.size() && spans_[current_span].begin() == i) {
      open_spans.push(&spans_[current_span++]);
      switch (open_spans.top()->semantic()) {
        case PrintableSpan::Semantic::Uri:
          text_out.append("[uri ");
          break;
        case PrintableSpan::Semantic::Escaped:
          text_out.append("[esc ");
          break;
        case PrintableSpan::Semantic::Styled: {
          text_out.append("[s");
          switch (open_spans.top()->style()) {
            case PrintableSpan::Style::Bold:
              text_out.append("B ");
              break;
            case PrintableSpan::Style::Italic:
              text_out.append("I ");
              break;
            case PrintableSpan::Style::H1:
              text_out.append("H1 ");
              break;
            case PrintableSpan::Style::H2:
              text_out.append("H2 ");
              break;
            case PrintableSpan::Style::H3:
              text_out.append("H3 ");
              break;
            case PrintableSpan::Style::H4:
              text_out.append("H4 ");
              break;
            case PrintableSpan::Style::H5:
              text_out.append("H5 ");
              break;
            case PrintableSpan::Style::H6:
              text_out.append("H6 ");
              break;
            case PrintableSpan::Style::Big:
              text_out.append("BIG ");
              break;
            case PrintableSpan::Style::Small:
              text_out.append("SMALL ");
              break;
            case PrintableSpan::Style::Blockquote:
              text_out.append("BQ ");
              break;
            case PrintableSpan::Style::Superscript:
              text_out.append("SUP ");
              break;
            case PrintableSpan::Style::Subscript:
              text_out.append("SUB ");
              break;
            case PrintableSpan::Style::Underline:
              text_out.append("UL ");
              break;
          }
        } break;
        case PrintableSpan::Semantic::Link: {
          text_out.append("[link");
          switch (open_spans.top()->link().kind()) {
            case proto::common::Link::DEFINITION:
              text_out.append("D ");
              break;
            case proto::common::Link::LIST:
              text_out.append("L ");
              break;
            case proto::common::Link::LIST_ITEM:
              text_out.append("E ");
              break;
            case proto::common::Link::IMPORTANT:
              text_out.append("I ");
              break;
            default:
              text_out.append("? ");
          }
        } break;
        case PrintableSpan::Semantic::CodeRef:
          text_out.append("[coderef ");
          break;
        case PrintableSpan::Semantic::Paragraph:
          text_out.append("[p ");
          break;
        case PrintableSpan::Semantic::ListItem:
          text_out.append("[li ");
          break;
        case PrintableSpan::Semantic::UnorderedList:
          text_out.append("[ul ");
          break;
        case PrintableSpan::Semantic::Raw:
          text_out.append("[raw ");
          break;
        case PrintableSpan::Semantic::Brief:
          text_out.append("[brief ");
          break;
        case PrintableSpan::Semantic::Markup:
          text_out.append("[^ ");
          break;
        case PrintableSpan::Semantic::CodeBlock:
          text_out.append("[cb ");
          break;
        case PrintableSpan::Semantic::UriLink:
          text_out.append("[uril ");
          break;
        case PrintableSpan::Semantic::TagBlock: {
          text_out.append("[tb");
          switch (open_spans.top()->tag_block().first) {
            case PrintableSpan::TagBlockId::Author:
              text_out.append("Author");
              break;
            case PrintableSpan::TagBlockId::Returns:
              text_out.append("Returns");
              break;
            case PrintableSpan::TagBlockId::Since:
              text_out.append("Since");
              break;
            case PrintableSpan::TagBlockId::Version:
              text_out.append("Version");
              break;
            case PrintableSpan::TagBlockId::Param:
              text_out.append("Param");
              break;
            case PrintableSpan::TagBlockId::Throws:
              text_out.append("Throws");
              break;
            case PrintableSpan::TagBlockId::See:
              text_out.append("See");
              break;
          }
          text_out.append(std::to_string(open_spans.top()->tag_block().second));
          text_out.append(" ");
        } break;
      }
    }
    text_out.push_back(annotated_buffer[i]);
  }
  return text_out;
}
}  // namespace kythe
