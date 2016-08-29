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

#include <algorithm>
#include <stack>

#include "kythe/cxx/doc/markup_handler.h"

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
            case proto::Link::LIST:
              ++list_count;
              break;
            case proto::Link::IMPORTANT:
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
            case proto::Link::LIST:
              --list_count;
              break;
            case proto::Link::IMPORTANT:
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
  std::vector<PrintableSpans> attempts;
  attempts.resize(handlers.size());
  size_t best_span_count = 0;
  for (size_t h = 0; h < handlers.size(); ++h) {
    handlers[h](printable, &attempts[h]);
    if (attempts[h].size() > attempts[best_span_count].size()) {
      best_span_count = h;
    }
  }
  attempts[best_span_count].Merge(printable.spans());
  return Printable(printable.text(), std::move(attempts[best_span_count]));
}
}
