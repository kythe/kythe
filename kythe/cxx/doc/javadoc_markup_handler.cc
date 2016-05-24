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

#include "kythe/cxx/doc/javadoc_markup_handler.h"
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
enum class JavadocTag : int {
#define ENUM_CASE(n, s) n,
  JAVADOC_TAGS(ENUM_CASE)
#undef ENUM_CASE
};
struct JavadocTagInfo {
  size_t name_length;
  const char* name;
  JavadocTag tag;
};
template <size_t length>
constexpr size_t string_length(char const (&)[length]) {
  return length - 1;
}
constexpr JavadocTagInfo kTagList[] = {
#define TAG_INFO(n, s) {string_length(s), s, JavadocTag::n},
    JAVADOC_TAGS(TAG_INFO)
#undef TAG_INFO
};

const JavadocTagInfo* ParseJavadocTag(const std::string& buffer,
                                      size_t at_sign) {
  // Javadoc tags must start at the beginning of a line (modulo leading spaces
  // or an asterisk); otherwise they are treated like normal text. We strip
  // out comment characters, including leading asterisks, when emitting the
  // raw Printable.
  size_t tag_end = at_sign + 1;
  for (; tag_end < buffer.size(); ++tag_end) {
    char c = buffer[tag_end];
    if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'))) {
      break;
    }
  }
  if (tag_end - at_sign == 1) {
    // Not a Javadoc tag.
    return nullptr;
  }
  for (const auto& tag_info : kTagList) {
    if (tag_info.name_length == tag_end - at_sign - 1 &&
        !memcmp(tag_info.name, &buffer[at_sign + 1], tag_info.name_length)) {
      return &tag_info;
    }
  }
  return nullptr;
}

size_t ParseJavadocBrace(const std::string& buffer, size_t open_brace,
                         size_t limit, PrintableSpans* out_spans);

size_t FindJavadocDescriptionEnd(const std::string& buffer, size_t begin,
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
          FindJavadocDescriptionEnd(buffer, content_start, limit, out_spans);
      out_spans->Emplace(content_start, desc_end,
                         PrintableSpan::Semantic::Return);
      return desc_end;
    }
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
  const auto* tag = ParseJavadocTag(buffer, tag_begin);
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

void ParseJavadoc(const Printable& in_message, PrintableSpans* out_spans) {
  const auto& text = in_message.text();
  // Are we at the start of the line (or the equivalent)?
  bool at_line_start = true;
  // Are we in the text block up top or in the tag block at the bottom?
  bool in_tag_block = false;
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
          if (const auto* tag = ParseJavadocTag(text, i)) {
            out_spans->Emplace(i, i + tag->name_length + 1,
                               PrintableSpan::Semantic::Markup);
            i = EvaluateJavadocTag(text, i, text.size(), tag, out_spans) - 1;
          }
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
