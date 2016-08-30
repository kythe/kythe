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
    TagBlock,  ///< Text belongs to a new tag block.
    Brief,     ///< Text is a brief subsection of a main description.
    CodeRef,  ///< Text is a reference to code and should be rendered monospace.
    Markup,   ///< Text used solely to direct the markup processor. May contain
              ///< child spans that are relevant. This text is usually not
              ///< rendered.
    Raw,      ///< Text may be passed through directly (modulo escaping).
    Link      ///< Text is a link to some anchor.
  };
  enum class TagBlockId : int {
    Author,
    Returns,
    Since,
    Version,
    Param,
    Throws
  };
  PrintableSpan(size_t begin, size_t end, const proto::Link& link)
      : begin_(begin), end_(end), link_(link), semantic_(Semantic::Link) {}
  PrintableSpan(size_t begin, size_t end, Semantic sema)
      : begin_(begin), end_(end), semantic_(sema) {}
  PrintableSpan(size_t begin, size_t end, TagBlockId tag_id, size_t tag_ordinal)
      : begin_(begin),
        end_(end),
        semantic_(Semantic::TagBlock),
        tag_block_(tag_id, tag_ordinal) {}
  const bool operator<(const PrintableSpan& o) const {
    int priority = link_priority();
    int opriority = o.link_priority();
    return std::tie(begin_, o.end_, semantic_, priority, tag_block_) <
           std::tie(o.begin_, end_, o.semantic_, opriority, tag_block_);
  }
  bool is_valid() const { return begin_ < end_; }
  const size_t begin() const { return begin_; }
  const size_t end() const { return end_; }
  void set_end(size_t end) { end_ = end; }
  const proto::Link& link() const { return link_; }
  Semantic semantic() const { return semantic_; }
  std::pair<TagBlockId, size_t> tag_block() const { return tag_block_; }

 private:
  int link_priority() const { return -link_.kind(); }
  /// The beginning offset, in bytes, of the span.
  size_t begin_;
  /// The ending offset, in bytes, of the span.
  size_t end_;
  /// The link for the span.
  proto::Link link_;
  /// The semantic for the span.
  Semantic semantic_;
  /// The tag block ID for the span.
  std::pair<TagBlockId, size_t> tag_block_ =
      std::make_pair(TagBlockId::Author, 0);
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
  /// \brief Produces a debug representation of the stored spans.
  std::string Dump(const std::string& annotated_buffer) const;
  /// \brief Returns the index of the next tag block with the given tag block
  /// ID.
  size_t next_tag_block_id(PrintableSpan::TagBlockId block_id) {
    return max_tag_block_[block_id]++;
  }

 private:
  std::vector<PrintableSpan> spans_;
  std::map<PrintableSpan::TagBlockId, size_t> max_tag_block_;
};

class Printable {
 public:
  /// A policy bitmask for filtering spans.
  enum RejectPolicy : unsigned {
    IncludeAll = 0,         ///< Reject no spans.
    RejectLists = 1,        ///< Reject LIST spans.
    RejectUnimportant = 2,  ///< Reject spans not dominated by IMPORTANT spans.
    IncludeLists = 4        ///< Always include LIST spans. Has precedence over
                            ///< `kRejectLists`.
  };

  /// \brief Build a Printable from a protobuf.
  /// \post The internal list of spans is sorted.
  /// \param filter A bitmask of Link spans to reject.
  Printable(const proto::Printable& from, RejectPolicy filter);
  /// \brief Build a Printable from a protobuf.
  /// \post The internal list of spans is sorted.
  explicit Printable(const proto::Printable& from)
      : Printable(from, IncludeAll) {}
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

/// \brief Combines `Printable::RejectPolicy` enumerators.
inline Printable::RejectPolicy operator|(Printable::RejectPolicy lhs,
                                         Printable::RejectPolicy rhs) {
  using impl = std::underlying_type<Printable::RejectPolicy>::type;
  return static_cast<Printable::RejectPolicy>(static_cast<impl>(lhs) |
                                              static_cast<impl>(rhs));
}

/// \brief Appends markup-specific spans to `spans` from `printable`.
using MarkupHandler =
    std::function<void(const Printable& printable, PrintableSpans* spans)>;

/// \brief Marks up `printable` using the series of handlers in `handlers`.
Printable HandleMarkup(const std::vector<MarkupHandler>& handlers,
                       const Printable& printable);

}  // namespace kythe

#endif
