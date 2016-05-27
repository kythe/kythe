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

#ifndef KYTHE_CXX_DOC_MARKUP_HANDLER_H_
#define KYTHE_CXX_DOC_MARKUP_HANDLER_H_

#include "kythe/proto/xref.pb.h"

#include <functional>

namespace kythe {
class PrintableSpan {
 public:
  enum class Semantic : int {
    Return,   ///< Text contains a description of a return value.
    Brief,    ///< Text is a brief subsection of a main description.
    CodeRef,  ///< Text is a reference to code and should be rendered monospace.
    Markup,   ///< Text used solely to direct the markup processor. May contain
              ///< child spans that are relevant. This text is usually not
              ///< rendered.
    Html,     ///< Text underneath this node is user-provided HTML.
    Raw,      ///< Text underneath this node is raw (e.g., with no escaping).
    Link      ///< Text is a link to some anchor.
  };
  PrintableSpan(size_t begin, size_t end, const proto::Link& link)
      : begin_(begin), end_(end), link_(link), semantic_(Semantic::Link) {}
  PrintableSpan(size_t begin, size_t end, Semantic sema)
      : begin_(begin), end_(end), semantic_(sema) {}
  const bool operator<(const PrintableSpan& o) const {
    return std::tie(begin_, o.end_, semantic_) <
           std::tie(o.begin_, end_, o.semantic_);
  }
  bool is_valid() const { return begin_ < end_; }
  const size_t begin() const { return begin_; }
  const size_t end() const { return end_; }
  void set_end(size_t end) { end_ = end; }
  const proto::Link& link() const { return link_; }
  Semantic semantic() const { return semantic_; }

 private:
  /// The beginning offset, in bytes, of the span.
  size_t begin_;
  /// The ending offset, in bytes, of the span.
  size_t end_;
  /// The link for the span.
  proto::Link link_;
  /// The semantic for the span.
  Semantic semantic_;
};

class PrintableSpans {
 public:
  /// \brief Insert all spans from `more`.
  /// Empty or negative-length spans are discarded.
  void Merge(const PrintableSpans& more);
  /// Construct a span and insert it.
  template <typename... T>
  void Emplace(T&&... span_args) {
    spans_.emplace_back(span_args...);
  }
  /// \return the number of spans being stored.
  const size_t size() const { return spans_.size(); }
  const PrintableSpan& span(size_t index) const { return spans_[index]; }
  PrintableSpan* mutable_span(size_t index) { return &spans_[index]; }

 private:
  std::vector<PrintableSpan> spans_;
};

class Printable {
 public:
  /// \brief Build a Printable from a protobuf.
  /// \post The internal list of spans is sorted.
  explicit Printable(const proto::Printable& from);
  /// \pre The list of spans is sorted.
  Printable(const std::string& text, PrintableSpans&& spans)
      : text_(text), spans_(std::move(spans)) {}
  /// \brief Return this Printable's list of spans.
  PrintableSpans* mutable_spans() { return &spans_; }
  /// \brief Return this Printable's list of spans.
  const PrintableSpans& spans() const { return spans_; }
  /// \brief The text of this Printable (unannotated with span markup).
  const std::string& text() const { return text_; }

 private:
  /// \brief The text of the Printable.
  std::string text_;
  /// \brief Interesting spans in `text_`.
  PrintableSpans spans_;
};

/// \brief Appends markup-specific spans to `spans` from `printable`.
using MarkupHandler =
    std::function<void(const Printable& printable, PrintableSpans* spans)>;

/// \brief Marks up `printable` using the best handler in `handlers`.
Printable HandleMarkup(const std::vector<MarkupHandler>& handlers,
                       const Printable& printable);

}  // namespace kythe

#endif
