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

#include "kythe/cxx/doc/javadoxygen_markup_handler.h"

#include "absl/log/check.h"
#include "absl/log/log.h"

// See
// <http://docs.oracle.com/javase/7/docs/technotes/tools/windows/javadoc.html>
// for a nice spec.

namespace kythe {
namespace {
/// A default value for tags that don't create tag blocks.
#define NOT_TAG_BLOCK Author
/// v is called as (TagEnumerator, "tag-name", is-tag-block, tag-block-id)
// clang-format off
#define JAVADOC_TAGS(v) \
  v(Author, "author", true, Author) \
  v(Code, "code", false, NOT_TAG_BLOCK) \
  v(Deprecated, "deprecated", false, NOT_TAG_BLOCK) \
  v(DocRoot, "docRoot", false, NOT_TAG_BLOCK) \
  v(Exception, "exception", false, NOT_TAG_BLOCK) \
  v(InheritDoc, "inheritDoc", false, NOT_TAG_BLOCK) \
  v(Link, "link", false, NOT_TAG_BLOCK) \
  v(LinkPlain, "linkPlain", false, NOT_TAG_BLOCK) \
  v(Literal, "literal", false, NOT_TAG_BLOCK) \
  v(Param, "param", false, NOT_TAG_BLOCK) \
  v(Return, "return", true, Returns) \
  v(See, "see", true, See) \
  v(Serial, "serial", false, NOT_TAG_BLOCK) \
  v(SerialData, "serialData", false, NOT_TAG_BLOCK) \
  v(SerialField, "serialField", false, NOT_TAG_BLOCK) \
  v(Since, "since", true, Since) \
  v(Throws, "throws", false, NOT_TAG_BLOCK) \
  v(Value, "value", false, NOT_TAG_BLOCK) \
  v(Version, "version", true, Version)
// clang-format on
/// v is called as
///     (TagEnumerator, "tag-name", is-section, is-tag-block, tag-block-id)
/// Sections affect parsing. We don't treat the "\brief" section as a tag block.
// clang-format off
#define DOXYGEN_TAGS(v)                                  \
  v(Brief, "brief", true, false, NOT_TAG_BLOCK)          \
      v(C, "c", false, false, NOT_TAG_BLOCK)             \
          v(Return, "return", true, true, Returns)       \
              v(Returns, "returns", true, true, Returns) \
                  v(Param, "param", true, false, NOT_TAG_BLOCK)
// clang-format on
enum class JavadocTag : int {
#define ENUM_CASE(n, s, b, i) n,
  JAVADOC_TAGS(ENUM_CASE)
#undef ENUM_CASE
};
enum class DoxygenTag : int {
#define ENUM_CASE(n, s, b, tb, i) n,
  DOXYGEN_TAGS(ENUM_CASE)
#undef ENUM_CASE
};
struct JavadocTagInfo {
  size_t name_length;
  const char* name;
  JavadocTag tag;
  bool is_tag_block;
  PrintableSpan::TagBlockId block_id;
};
struct DoxygenTagInfo {
  size_t name_length;
  const char* name;
  DoxygenTag tag;
  bool begins_section;
  bool is_tag_block;
  PrintableSpan::TagBlockId block_id;
};
template <size_t length>
constexpr size_t string_length(char const (&)[length]) {
  return length - 1;
}
constexpr JavadocTagInfo kJavadocTagList[] = {
#define TAG_INFO(n, s, b, i) \
  {string_length(s), s, JavadocTag::n, b, PrintableSpan::TagBlockId::i},
    JAVADOC_TAGS(TAG_INFO)
#undef TAG_INFO
};
constexpr size_t kJavadocTagCount =
    sizeof(kJavadocTagList) / sizeof(JavadocTagInfo);
constexpr DoxygenTagInfo kDoxygenTagList[] = {
#define TAG_INFO(n, s, b, tb, i) \
  {string_length(s), s, DoxygenTag::n, b, tb, PrintableSpan::TagBlockId::i},
    DOXYGEN_TAGS(TAG_INFO)
#undef TAG_INFO
};
constexpr size_t kDoxygenTagCount =
    sizeof(kDoxygenTagList) / sizeof(DoxygenTagInfo);

template <typename TagInfo>
const TagInfo* ParseTag(const TagInfo* tag_list, size_t tag_count,
                        const std::string& buffer, size_t delimiter) {
  // Javadoc tags must start at the beginning of a line (modulo leading spaces
  // or an asterisk); otherwise they are treated like normal text. We strip
  // out comment characters, including leading asterisks, when emitting the
  // raw Printable.
  size_t tag_end = delimiter + 1;
  for (; tag_end < buffer.size(); ++tag_end) {
    char c = buffer[tag_end];
    if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'))) {
      break;
    }
  }
  if (tag_end - delimiter == 1) {
    // Not a Javadoc tag.
    return nullptr;
  }
  for (size_t tag = 0; tag < tag_count; ++tag) {
    const auto& tag_info = tag_list[tag];
    if (tag_info.name_length == tag_end - delimiter - 1 &&
        !memcmp(tag_info.name, &buffer[delimiter + 1], tag_info.name_length)) {
      return &tag_info;
    }
  }
  return nullptr;
}

size_t ParseJavadocBrace(const std::string& buffer, size_t open_brace,
                         size_t limit, PrintableSpans* out_spans);

size_t EvaluateDoxygenTag(const std::string& buffer, size_t slash, size_t limit,
                          const DoxygenTagInfo* info,
                          PrintableSpans* out_spans);

size_t ParseJavadocDescription(const std::string& buffer, size_t begin,
                               size_t limit, PrintableSpans* out_spans) {
  CHECK(limit <= buffer.size());
  bool at_line_start = false;
  for (size_t end = begin; end < limit; ++end) {
    char c = buffer[end];
    switch (c) {
      case '{':
        end = ParseJavadocBrace(buffer, end, limit, out_spans) - 1;
        at_line_start = false;
        break;
      case '\n':
        at_line_start = true;
        // End the description if there's a double newline (for compatibility
        // with Doxygen).
        for (size_t scan = end + 1; scan < limit; ++scan) {
          c = buffer[scan];
          if (c == '\n') {
            // End the description before the first newline.
            return end;
          } else if (c != ' ' && c != '\t') {
            break;
          }
        }
        break;
      case '@':
        if (at_line_start) {
          // Tags must start at the beginning of a line; otherwise they should
          // be treated as normal text. (This discounts inline tags, but those
          // start with {@.)
          return end;
        }
        at_line_start = false;
        break;
      default:
        if (c != ' ' && c != '\t') {
          // TODO(zarko): Unicode whitespace support (probably using
          // std::wstring_convert::from_bytes/isspace to avoid taking on
          // more dependencies).
          at_line_start = false;
        }
        break;
    }
  }
  return limit;
}

size_t EvaluateJavadocTag(const std::string& buffer, size_t at_sign,
                          size_t limit, const JavadocTagInfo* info,
                          PrintableSpans* out_spans) {
  size_t content_start = at_sign + info->name_length + 1;
  if (info->is_tag_block) {
    size_t desc_end =
        ParseJavadocDescription(buffer, content_start, limit, out_spans);
    out_spans->Emplace(content_start, desc_end, info->block_id,
                       out_spans->next_tag_block_id(info->block_id));
    return desc_end;
  }
  switch (info->tag) {
    case JavadocTag::Code: {
      out_spans->Emplace(content_start, limit,
                         PrintableSpan::Semantic::CodeRef);
      return limit;
    } break;
    case JavadocTag::Param:     /* fallthrough */
    case JavadocTag::Exception: /* fallthrough */
    case JavadocTag::Throws: {
      // TODO(zarko): We expect the following to appear (and should annotate
      // as code):
      //   @param java-token desc
      //   @param <java-token> desc
      //   @throws java-qualified-name desc
      size_t desc_end =
          ParseJavadocDescription(buffer, content_start, limit, out_spans);
      auto block_id = info->tag == JavadocTag::Param
                          ? PrintableSpan::TagBlockId::Param
                          : PrintableSpan::TagBlockId::Throws;
      out_spans->Emplace(content_start, desc_end, block_id,
                         out_spans->next_tag_block_id(block_id));
      return desc_end;
    } break;
    default:
      return content_start;
  }
}

size_t ParseDoxygenDescription(const std::string& buffer, size_t begin,
                               size_t limit, PrintableSpans* out_spans) {
  CHECK(limit <= buffer.size());
  for (size_t end = begin; end < limit; ++end) {
    char c = buffer[end];
    if (c == '{') {
      end = ParseJavadocBrace(buffer, end, limit, out_spans) - 1;
    } else if (c == '\\' || c == '@') {
      if (const auto* tag =
              ParseTag(kDoxygenTagList, kDoxygenTagCount, buffer, end)) {
        if (tag->begins_section) {
          return end;
        }
        out_spans->Emplace(end, end + tag->name_length + 1,
                           PrintableSpan::Semantic::Markup);
        end = EvaluateDoxygenTag(buffer, end, limit, tag, out_spans) - 1;
      }
    } else if (c == '\n') {
      // Scan forward to see if the next line is blank.
      // TODO(zarko): Unicode newlines and whitespace.
      for (size_t scan = end + 1; scan < limit; ++scan) {
        c = buffer[scan];
        if (c == '\n') {
          // End the description before the first newline.
          return end;
        } else if (c != ' ' && c != '\t') {
          break;
        }
      }
    }
  }
  return limit;
}

size_t ParseDoxygenWord(const std::string& buffer, size_t begin, size_t limit,
                        PrintableSpans* out_spans) {
  CHECK(limit <= buffer.size());
  // It's not clear what a "word" is; we'll assume it's made of characters
  // that are not whitespace (after 0+ characters of non-endline whitespace).
  size_t end = begin;
  for (; end < limit; ++end) {
    char c = buffer[end];
    if (c != ' ' && c != '\t') {
      break;
    }
  }
  if (end == limit) {
    // This wasn't a word.
    return begin;
  }
  // TODO(zarko): Unicode newlines and whitespace.
  for (; end < limit; ++end) {
    char c = buffer[end];
    if (c == ' ' || c == '\t' || c == '\n') {
      return end;
    }
  }
  return limit;
}

// For Doxygen commands:
//   <>: "a single word"
//   (): extends to the end of the line
//   {}: extends to the next blank line or section indicator
//   []: makes anything optional
// See <https://www.stack.nl/~dimitri/doxygen/manual/commands.html>
size_t EvaluateDoxygenTag(const std::string& buffer, size_t slash, size_t limit,
                          const DoxygenTagInfo* info,
                          PrintableSpans* out_spans) {
  size_t content_start = slash + info->name_length + 1;
  // \return { description of the return value }
  // \returns { description of the return value }
  if (info->is_tag_block) {
    size_t desc_end =
        ParseDoxygenDescription(buffer, content_start, limit, out_spans);
    out_spans->Emplace(content_start, desc_end, info->block_id,
                       out_spans->next_tag_block_id(info->block_id));
    return desc_end;
  }
  switch (info->tag) {
    // \brief { brief description }
    case DoxygenTag::Brief: {
      size_t desc_end =
          ParseDoxygenDescription(buffer, content_start, limit, out_spans);
      out_spans->Emplace(content_start, desc_end,
                         PrintableSpan::Semantic::Brief);
      return desc_end;
    } break;
    // \c <word>
    case DoxygenTag::C: {
      size_t word_end =
          ParseDoxygenWord(buffer, content_start, limit, out_spans);
      out_spans->Emplace(content_start, word_end,
                         PrintableSpan::Semantic::CodeRef);
      return word_end;
    } break;
    default:
      return content_start;
  }
}

// <a href="{@docRoot}/copyright.html"> is supposed to turn into
// <a href="../../copyright.html">, so we don't need to worry about quotes
// as delimiters. Users *do* insert whitespace between the opening { and
// a tag name. Curlies *do* nest arbitrarily; witness:
//   {@code {@literal @}Clazz Clasz<R>}
//   {@code {@link Clazz#Method()}.name}
size_t ParseJavadocBrace(const std::string& buffer, size_t open_brace,
                         size_t limit, PrintableSpans* out_spans) {
  // Try to find the tag for the brace.
  size_t tag_begin = open_brace + 1;
  for (; tag_begin < limit; ++tag_begin) {
    char c = buffer[tag_begin];
    if (c == '@') {
      break;
    } else if (c != ' ') {
      return open_brace + 1;
    }
  }
  const auto* tag =
      ParseTag(kJavadocTagList, kJavadocTagCount, buffer, tag_begin);
  if (tag == nullptr) {
    // Invalid brace block.
    return open_brace + 1;
  }
  // Find the end of this brace block.
  size_t close_brace = tag_begin + tag->name_length + 1;
  size_t brace_stack = 1;
  for (; close_brace < limit; ++close_brace) {
    char c = buffer[close_brace];
    if (c == '}') {
      if (--brace_stack == 0) {
        break;
      }
    } else if (c == '{') {
      ++brace_stack;
    }
  }
  if (brace_stack != 0) {
    // Invalid brace block.
    return open_brace + 1;
  }
  out_spans->Emplace(open_brace, tag_begin + tag->name_length + 1,
                     PrintableSpan::Semantic::Markup);
  out_spans->Emplace(close_brace, close_brace + 1,
                     PrintableSpan::Semantic::Markup);
  EvaluateJavadocTag(buffer, tag_begin, close_brace, tag, out_spans);
  return close_brace + 1;
}
}  // namespace

void ParseJavadoxygen(const Printable& in_message, const PrintableSpans&,
                      PrintableSpans* out_spans) {
  const auto& text = in_message.text();
  // Are we at the start of the line (or the equivalent)?
  bool at_line_start = true;
  for (size_t i = 0; i < text.size(); ++i) {
    char c = text[i];
    switch (c) {
      // NB: Escaping in Javadoc means using HTML entities.
      case '{':
        i = ParseJavadocBrace(text, i, text.size(), out_spans);
        break;
      case '@':
        if (at_line_start) {
          if (const auto* tag =
                  ParseTag(kJavadocTagList, kJavadocTagCount, text, i)) {
            out_spans->Emplace(i, i + tag->name_length + 1,
                               PrintableSpan::Semantic::Markup);
            i = EvaluateJavadocTag(text, i, text.size(), tag, out_spans) - 1;
          } else if (const auto* tag =
                         ParseTag(kDoxygenTagList, kDoxygenTagCount, text, i)) {
            // Fall back to trying to parse as a Doxygen tag.
            out_spans->Emplace(i, i + tag->name_length + 1,
                               PrintableSpan::Semantic::Markup);
            i = EvaluateDoxygenTag(text, i, text.size(), tag, out_spans) - 1;
          }
        } else if (const auto* tag =
                       ParseTag(kDoxygenTagList, kDoxygenTagCount, text, i)) {
          // Fall back to trying to parse as a Doxygen tag.
          out_spans->Emplace(i, i + tag->name_length + 1,
                             PrintableSpan::Semantic::Markup);
          i = EvaluateDoxygenTag(text, i, text.size(), tag, out_spans) - 1;
        }
        break;
      case '\\':
        // Doxygen tags don't appear to care whether they start at the beginning
        // of a line.
        if (const auto* tag =
                ParseTag(kDoxygenTagList, kDoxygenTagCount, text, i)) {
          out_spans->Emplace(i, i + tag->name_length + 1,
                             PrintableSpan::Semantic::Markup);
          i = EvaluateDoxygenTag(text, i, text.size(), tag, out_spans) - 1;
        }
        break;
      case '\n':
        at_line_start = true;
        break;
      default:
        if (c != ' ' && c != '\t') {
          // TODO(zarko): Unicode whitespace support (probably using
          // std::wstring_convert::from_bytes/isspace to avoid taking on
          // more dependencies).
          at_line_start = false;
        }
        break;
    }
  }
}

}  // namespace kythe
