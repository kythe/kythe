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

#include "kythe/cxx/doc/javadoxygen_markup_handler.h"
#include "glog/logging.h"

// See
// <http://docs.oracle.com/javase/7/docs/technotes/tools/windows/javadoc.html>
// for a nice spec.

namespace kythe {
namespace {
#define JAVADOC_TAGS(v) \
  v(Author, "author") \
  v(Code, "code") \
  v(Deprecated, "deprecated") \
  v(DocRoot, "docRoot") \
  v(Exception, "exception") \
  v(InheritDoc, "inheritDoc") \
  v(Link, "link") \
  v(LinkPlain, "linkPlain") \
  v(Literal, "literal") \
  v(Param, "param") \
  v(Return, "return") \
  v(See, "see") \
  v(Serial, "serial") \
  v(SerialData, "serialData") \
  v(SerialField, "serialField") \
  v(Since, "since") \
  v(Throws, "throws") \
  v(Value, "value") \
  v(Version, "version")
#define DOXYGEN_TAGS(v) \
  v(Brief, "brief", true) \
  v(C, "c", false) \
  v(Return, "return", true) \
  v(Returns, "returns", true)
enum class JavadocTag : int {
#define ENUM_CASE(n, s) n,
  JAVADOC_TAGS(ENUM_CASE)
#undef ENUM_CASE
};
enum class DoxygenTag : int {
#define ENUM_CASE(n, s, b) n,
  DOXYGEN_TAGS(ENUM_CASE)
#undef ENUM_CASE
};
struct JavadocTagInfo {
  size_t name_length;
  const char* name;
  JavadocTag tag;
};
struct DoxygenTagInfo {
  size_t name_length;
  const char* name;
  DoxygenTag tag;
  bool begins_section;
};
template <size_t length>
constexpr size_t string_length(char const (&)[length]) {
  return length - 1;
}
constexpr JavadocTagInfo kJavadocTagList[] = {
#define TAG_INFO(n, s) {string_length(s), s, JavadocTag::n},
    JAVADOC_TAGS(TAG_INFO)
#undef TAG_INFO
};
constexpr size_t kJavadocTagCount =
    sizeof(kJavadocTagList) / sizeof(JavadocTagInfo);
constexpr DoxygenTagInfo kDoxygenTagList[] = {
#define TAG_INFO(n, s, b) {string_length(s), s, DoxygenTag::n, b},
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
  for (size_t end = begin; end < limit; ++end) {
    char c = buffer[end];
    if (c == '{') {
      end = ParseJavadocBrace(buffer, end, limit, out_spans) - 1;
    } else if (c == '@') {
      return end;
    }
  }
  return limit;
}

size_t EvaluateJavadocTag(const std::string& buffer, size_t at_sign,
                          size_t limit, const JavadocTagInfo* info,
                          PrintableSpans* out_spans) {
  size_t content_start = at_sign + info->name_length + 1;
  switch (info->tag) {
    case JavadocTag::Return: {
      size_t desc_end =
          ParseJavadocDescription(buffer, content_start, limit, out_spans);
      out_spans->Emplace(content_start, desc_end,
                         PrintableSpan::Semantic::Return);
      return desc_end;
    }
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
    } else if (c == '\\') {
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
  switch (info->tag) {
    // \brief { brief description }
    case DoxygenTag::Brief: {
      size_t desc_end =
          ParseDoxygenDescription(buffer, content_start, limit, out_spans);
      out_spans->Emplace(content_start, desc_end,
                         PrintableSpan::Semantic::Brief);
      return desc_end;
    } break;
    // \return { description of the return value }
    // \returns { description of the return value }
    case DoxygenTag::Return:
    case DoxygenTag::Returns: {
      size_t desc_end =
          ParseDoxygenDescription(buffer, content_start, limit, out_spans);
      out_spans->Emplace(content_start, desc_end,
                         PrintableSpan::Semantic::Return);
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
  out_spans->Emplace(open_brace, close_brace + 1,
                     PrintableSpan::Semantic::Markup);
  EvaluateJavadocTag(buffer, tag_begin, close_brace, tag, out_spans);
  return close_brace + 1;
}
}

void ParseJavadoxygen(const Printable& in_message, PrintableSpans* out_spans) {
  const auto& text = in_message.text();
  // Are we at the start of the line (or the equivalent)?
  bool at_line_start = true;
  // Javadoc is HTML by default.
  out_spans->Emplace(0, text.size(), PrintableSpan::Semantic::Html);
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
          }
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
