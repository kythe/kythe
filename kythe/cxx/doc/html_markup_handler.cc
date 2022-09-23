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

#include "absl/log/check.h"
#include "absl/log/log.h"
#include "kythe/cxx/doc/javadoxygen_markup_handler.h"

namespace kythe {
namespace {
enum class HtmlTag : int {
  A,
  B,
  BIG,
  BLOCKQUOTE,
  CODE,
  EM,
  H1,
  H2,
  H3,
  H4,
  H5,
  H6,
  I,
  LI,
  P,
  PRE,
  SMALL,
  STRONG,
  SUB,
  SUP,
  TT,
  UL,
  // TODO(zarko): OL
  // TODO(zarko): DL, DT, DD
  // TODO(zarko): CAPTION, TABLE, TBODY, TD, TFOOT, TH, THEAD, TR
};
struct HtmlTagInfo {
  size_t name_length;
  const char* name;
  HtmlTag tag;
  /// Tags with need_explicit_close will only be emitted if their closing tag
  /// is found. Tags without it will always be emitted; their closing tags will
  /// be ignored.
  enum CloseKind : bool {
    NoClose = false,
    NeedsClose = true
  } needs_explicit_close;
  /// Some tags are simple style tags (like <i>, <b>, and so on).
  bool is_style;
  /// If this tag is_style, the style of the tag.
  PrintableSpan::Style style;
  bool same_name(size_t length, const char* buffer) const {
    if (length != name_length) {
      return false;
    }
    for (size_t p = 0; p < length; ++p) {
      if (tolower(*(buffer++)) != name[p]) {
        return false;
      }
    }
    return true;
  }
  template <size_t length>
  constexpr HtmlTagInfo(char const (&name)[length], HtmlTag tag,
                        CloseKind needs_explicit_close)
      : name_length(length - 1),
        name(name),
        tag(tag),
        needs_explicit_close(needs_explicit_close),
        is_style(false),
        style(PrintableSpan::Style::Bold) {}
  template <size_t length>
  constexpr HtmlTagInfo(char const (&name)[length], HtmlTag tag,
                        PrintableSpan::Style style)
      : name_length(length - 1),
        name(name),
        tag(tag),
        needs_explicit_close(NeedsClose),
        is_style(true),
        style(style) {}
};

constexpr HtmlTagInfo kHtmlTagList[] = {
    {"p", HtmlTag::P, HtmlTagInfo::NoClose},
    {"li", HtmlTag::LI, HtmlTagInfo::NoClose},
    {"b", HtmlTag::B, PrintableSpan::Style::Bold},
    {"i", HtmlTag::I, PrintableSpan::Style::Italic},
    {"ul", HtmlTag::UL, HtmlTagInfo::NeedsClose},
    {"h1", HtmlTag::H1, PrintableSpan::Style::H1},
    {"h2", HtmlTag::H2, PrintableSpan::Style::H2},
    {"h3", HtmlTag::H3, PrintableSpan::Style::H3},
    {"h4", HtmlTag::H4, PrintableSpan::Style::H4},
    {"h5", HtmlTag::H5, PrintableSpan::Style::H5},
    {"h6", HtmlTag::H6, PrintableSpan::Style::H6},
    {"pre", HtmlTag::PRE, HtmlTagInfo::NeedsClose},
    {"strong", HtmlTag::STRONG, PrintableSpan::Style::Bold},
    {"a", HtmlTag::A, HtmlTagInfo::NeedsClose},
    {"blockquote", HtmlTag::BLOCKQUOTE, PrintableSpan::Style::Blockquote},
    {"big", HtmlTag::BIG, PrintableSpan::Style::Big},
    {"small", HtmlTag::SMALL, PrintableSpan::Style::Small},
    {"sup", HtmlTag::SUP, PrintableSpan::Style::Superscript},
    {"sub", HtmlTag::SUB, PrintableSpan::Style::Subscript},
    {"em", HtmlTag::EM, PrintableSpan::Style::Bold},
    {"ul", HtmlTag::UL, PrintableSpan::Style::Underline},
    {"code", HtmlTag::CODE, HtmlTagInfo::NeedsClose},
    {"tt", HtmlTag::TT, HtmlTagInfo::NeedsClose},
};
constexpr size_t kHtmlTagCount = sizeof(kHtmlTagList) / sizeof(HtmlTagInfo);

struct OpenTag {
  const HtmlTagInfo* tag;
  size_t begin;
  size_t end;
};

class ParseState {
 public:
  ParseState(const std::string& buffer, const PrintableSpans& spans,
             PrintableSpans* out_spans)
      : length_(buffer.size()),
        buffer_(buffer),
        spans_(spans),
        out_spans_(out_spans) {}
  void Parse() {
    while (char c = advance()) {
      switch (c) {
        case '<': {
          ParseState forked = ForkParseState(length_);
          if (forked.ParseTag()) {
            JoinParseState(forked);
          }
        } break;
        case '&': {
          ParseState forked = ForkParseState(length_);
          if (forked.ParseEscape()) {
            JoinParseState(forked);
          }
        } break;
        default:
          break;
      }
    }
    CloseOpenTags(length_);
  }

 private:
  /// \brief Advance to the next character in the source buffer (keeping track
  /// of any previously emitted spans we might enter or exit).
  /// \return The next character in the buffer (or 0, if we're past the end).
  char advance() {
    if (++pos_ >= length_) {
      return 0;
    }
    while (!active_spans_.empty() && active_spans_.back()->end() >= pos_) {
      active_spans_.pop_back();
    }
    while (next_span_ < spans_.size() &&
           spans_.span(next_span_).begin() == pos_) {
      active_spans_.push_back(&spans_.span(next_span_++));
    }
    return buffer_[pos_];
  }

  /// \brief Skips forward until the current character is not (horizontal or
  /// vertical) whitespace.
  /// \return The current character after skipping forward (or doing nothing).
  char SkipWhitespace() {
    // TODO(zarko): Unicode; split this out into a separate utility function
    // to use in other markup handlers.
    for (; pos_ < length_ && std::isspace(buffer_[pos_]); advance())
      ;
    return pos_ < length_ ? buffer_[pos_] : 0;
  }

  /// \brief Tries to parse an HTML open or close tag name at the current
  /// position.
  /// \pre The current character is a <.
  const HtmlTagInfo* ParseTagInfo(bool* is_close_tag) {
    // Advance past the <.
    if (!advance()) {
      return nullptr;
    }
    // Skip whitespace before the label.
    char first_label = SkipWhitespace();
    if (!first_label) {
      return nullptr;
    }
    if (first_label == '/') {
      // Skip the slash.
      if (!advance()) {
        return nullptr;
      }
      // Skip whitespace.
      if (!SkipWhitespace()) {
        return nullptr;
      }
      *is_close_tag = true;
    } else {
      *is_close_tag = false;
    }
    size_t name_start = pos_;
    for (char c = buffer_[pos_]; isalnum(c = advance());)
      ;
    if (name_start == pos_) {
      return nullptr;
    }
    for (size_t tag = 0; tag < kHtmlTagCount; ++tag) {
      const auto& tag_info = kHtmlTagList[tag];
      if (tag_info.same_name(pos_ - name_start, buffer_.c_str() + name_start)) {
        if (*is_close_tag) {
          if (SkipWhitespace() != '>') {
            return nullptr;
          }
        }
        return &tag_info;
      }
    }
    return nullptr;
  }

  /// \brief Tries to parse the contents of an href attribute.
  /// \pre The current character is the " of the attribute.
  /// \return true if we found a value; false if otherwise.
  bool ParseHrefContent() {
    size_t uri_begin = pos_ + 1;
    for (;;) {
      char c = advance();
      if (c == '"') {
        break;
      } else if (c == 0) {
        return false;
      } else if (c == '&') {
        if (!ParseEscape()) {
          return false;
        }
      }
    }
    out_spans_->Emplace(uri_begin, pos_, PrintableSpan::Semantic::Uri);
    advance();
    return true;
  }

  /// \brief Tries to parse attributes of the HTML open tag `for_tag`.
  /// \pre The current character is one past the last character in the tag name.
  /// \return true if all (0+) attributes were OK; false otherwise.
  bool ParseAttributes(const HtmlTagInfo* for_tag) {
    bool found_href = false;
    for (;;) {
      char c = SkipWhitespace();
      if (c == '>') {
        return (for_tag->tag != HtmlTag::A || found_href);
      } else if ((c == 'h' || c == 'H') && for_tag->tag == HtmlTag::A) {
        char r = tolower(advance());
        char e = tolower(advance());
        char f = tolower(advance());
        char eq = advance();
        if (eq != '=') {
          eq = SkipWhitespace();
        }
        char quot = advance();
        if (quot != '"') {
          quot = SkipWhitespace();
        }
        if (r == 'r' && e == 'e' && f == 'f' && eq == '=' && quot == '"') {
          if (!(found_href = ParseHrefContent())) {
            return false;
          }
        } else {
          return false;
        }
      } else {
        return false;
      }
    }
  }

  /// \brief Closes tags that don't require explicit closes, or tags that
  /// authors
  /// might forget to close (like <ul> or <pre> at a <p> boundary).
  void CloseOpenTags(size_t at_pos) {
    for (size_t i = open_tags_.size(); i != 0; --i) {
      const auto* open_tag = &open_tags_[i - 1];
      switch (open_tag->tag->tag) {
        case HtmlTag::P:
          out_spans_->Emplace(open_tag->end, at_pos,
                              PrintableSpan::Semantic::Paragraph);
          break;
        case HtmlTag::LI:
          out_spans_->Emplace(open_tag->end, at_pos,
                              PrintableSpan::Semantic::ListItem);
          break;
        case HtmlTag::UL:
          out_spans_->Emplace(open_tag->end, at_pos,
                              PrintableSpan::Semantic::UnorderedList);
          break;
        case HtmlTag::CODE:
        case HtmlTag::TT: /* fallthrough */
          out_spans_->Emplace(open_tag->end, at_pos,
                              PrintableSpan::Semantic::CodeRef);
          break;
        case HtmlTag::PRE:
          out_spans_->Emplace(open_tag->end, at_pos,
                              PrintableSpan::Semantic::CodeBlock);
          break;
        default:
          break;
      }
    }
    open_tags_.clear();
  }

  /// \brief Parse an HTML escape.
  /// \pre The current character is &.
  /// \return true if this was a cromulent HTML escape.
  bool ParseEscape() {
    size_t amp = pos_;
    char ch = advance();
    if (ch == '#') {
      advance();
      ch = SkipWhitespace();
      if (isdigit(ch)) {
        for (; isdigit(ch); ch = advance())
          ;
      } else if (ch == 'x' || ch == 'X') {
        for (ch = advance(); isxdigit(ch); ch = advance())
          ;
      }
    } else {
      for (; isalnum(ch); ch = advance())
        ;
    }
    if (ch == ';' && pos_ != amp + 1) {
      out_spans_->Emplace(amp, pos_ + 1, PrintableSpan::Semantic::Escaped);
      return true;
    }
    return false;
  }

  /// \brief Parse an HTML tag.
  /// \pre The current character is <.
  /// \return true if this was a proper HTML tag.
  bool ParseTag() {
    size_t tag_start = pos_;
    bool is_close_tag = false;
    const HtmlTagInfo* info = ParseTagInfo(&is_close_tag);
    if (info == nullptr) {
      return false;
    }
    if (is_close_tag) {
      // ParseTagInfo puts us at the > of the closing tag.
      if (info->tag == HtmlTag::LI || info->tag == HtmlTag::P) {
        // Discard </li> and </p> tags.
      } else {
        for (size_t i = open_tags_.size(); i != 0; --i) {
          const auto* open_tag = &open_tags_[i - 1];
          if (open_tag->tag == info) {
            if (open_tag->tag->is_style) {
              out_spans_->Emplace(open_tag->end, tag_start,
                                  open_tag->tag->style);
            } else {
              switch (open_tag->tag->tag) {
                case HtmlTag::UL:
                  out_spans_->Emplace(open_tag->end, tag_start,
                                      PrintableSpan::Semantic::UnorderedList);
                  break;
                case HtmlTag::PRE:
                  out_spans_->Emplace(open_tag->end, tag_start,
                                      PrintableSpan::Semantic::CodeBlock);
                  break;
                case HtmlTag::CODE:
                case HtmlTag::TT:
                  out_spans_->Emplace(open_tag->end, tag_start,
                                      PrintableSpan::Semantic::CodeRef);
                  break;
                // Mark the entire <a href=[uri foo]>text</a> as a link (such
                // that it contains the uri).
                case HtmlTag::A:
                  out_spans_->Emplace(open_tag->begin, pos_ + 1,
                                      PrintableSpan::Semantic::UriLink);
                  break;
                default:
                  break;
              }
            }
            open_tags_.erase(open_tags_.begin() + i - 1);
            break;
          } else if (open_tag->tag->tag == HtmlTag::LI &&
                     info->tag == HtmlTag::UL) {
            // Automatically close list items.
            out_spans_->Emplace(open_tag->end, tag_start,
                                PrintableSpan::Semantic::ListItem);
            open_tags_.erase(open_tags_.begin() + i - 1);
          } else if (open_tag->tag->tag == HtmlTag::P &&
                     info->tag == HtmlTag::UL) {
            // Automatically close captive paragraphs.
            out_spans_->Emplace(open_tag->end, tag_start,
                                PrintableSpan::Semantic::Paragraph);
            open_tags_.erase(open_tags_.begin() + i - 1);
          }
        }
      }
    } else {
      if (!ParseAttributes(info)) {
        return false;
      }
      switch (info->tag) {
        case HtmlTag::P: {
          // Javadoc asks authors to separate paragraphs with <p> tags.
          // Look for the nearest <p> that isn't hidden by a <ul>.
          bool push_new_para = true;
          for (auto i = open_tags_.rbegin(), e = open_tags_.rend(); i != e;
               ++i) {
            if (i->tag->tag == HtmlTag::UL) {
              break;
            } else if (i->tag->tag == HtmlTag::P) {
              // Replace a <p>.
              out_spans_->Emplace(i->end, tag_start,
                                  PrintableSpan::Semantic::Paragraph);
              i->begin = tag_start;
              i->end = pos_ + 1;
              push_new_para = false;
              break;
            }
          }
          if (push_new_para) {
            open_tags_.emplace_back(OpenTag({info, tag_start, pos_ + 1}));
          }
        } break;
        case HtmlTag::LI: {
          // Look for the nearest <li> or <ul>.
          for (auto i = open_tags_.rbegin(), e = open_tags_.rend(); i != e;
               ++i) {
            if (i->tag->tag == HtmlTag::UL) {
              // We require explicit <li> tags after <ul>. Open a new <li>.
              open_tags_.emplace_back(OpenTag{info, tag_start, pos_ + 1});
              break;
            } else if (i->tag->tag == HtmlTag::LI) {
              // Replace an <li>.
              out_spans_->Emplace(i->end, tag_start,
                                  PrintableSpan::Semantic::ListItem);
              i->begin = tag_start;
              i->end = pos_ + 1;
              break;
            }
          }
        } break;
        default:
          break;
      }
      if (info->needs_explicit_close) {
        open_tags_.emplace_back(OpenTag{info, tag_start, pos_ + 1});
      }
    }
    // Always mark open and close tags as markup.
    out_spans_->Emplace(tag_start, pos_ + 1, PrintableSpan::Semantic::Markup);
    return true;
  }

  /// \return a `ParseState` equivalent to this one, except for a different
  /// `length`.
  ParseState ForkParseState(size_t new_length) {
    CHECK(new_length <= length_);
    ParseState new_state(buffer_, spans_, out_spans_);
    new_state.next_span_ = next_span_;
    new_state.pos_ = pos_;
    new_state.length_ = new_length;
    new_state.active_spans_ = active_spans_;
    new_state.open_tags_ = open_tags_;
    return new_state;
  }

  /// \brief Copies the state of `other` to this `ParseState`, but preserves
  /// this state's `length`.
  void JoinParseState(const ParseState& other) {
    next_span_ = other.next_span_;
    pos_ = other.pos_;
    CHECK(pos_ <= length_);
    active_spans_ = other.active_spans_;
    open_tags_ = other.open_tags_;
  }

  /// The next previously-emitted span to enter.
  size_t next_span_ = 0;
  /// Our position in `buffer_`, or `~0` if we've not started parsing.
  size_t pos_ = ~0;
  /// The length of buffer_ we're willing to consider.
  size_t length_;
  /// The source text we're parsing.
  const std::string& buffer_;
  /// Previously-emitted spans we're currently inside.
  std::vector<const PrintableSpan*> active_spans_;
  /// HTML tags that are currently open.
  std::vector<OpenTag> open_tags_;
  /// Previously-emitted spans.
  const PrintableSpans& spans_;
  /// Destination for emitting new spans.
  PrintableSpans* out_spans_;
};
}  // anonymous namespace

void ParseHtml(const Printable& in_message, const PrintableSpans& spans,
               PrintableSpans* out_spans) {
  ParseState state(in_message.text(), spans, out_spans);
  state.Parse();
}

}  // namespace kythe
