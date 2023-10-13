/*
 * Copyright 2015 The Kythe Authors. All rights reserved.
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

#include "kythe/cxx/common/kythe_uri.h"

#include <cstddef>
#include <string>
#include <utility>

#include "absl/strings/match.h"
#include "absl/strings/str_split.h"
#include "absl/strings/string_view.h"
#include "kythe/cxx/common/path_utils.h"
#include "kythe/proto/storage.pb.h"

namespace kythe {
namespace {

constexpr char kHexDigits[] = "0123456789ABCDEF";
/// The URI scheme label for Kythe.
constexpr char kUriScheme[] = "kythe";
constexpr char kUriPrefix[] = "kythe:";

/// \brief Returns whether a byte should be escaped.
/// \param mode The escaping mode to use.
/// \param c The byte to examine.
bool should_escape(UriEscapeMode mode, char c) {
  return !((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
           (c >= '0' && c <= '9') || c == '-' || c == '.' || c == '_' ||
           c == '~' || (mode == UriEscapeMode::kEscapePaths && c == '/'));
}

/// \brief Returns the value of a hex digit.
/// \param digit The hex digit.
/// \return The value of the digit, or -1 if it was not a hex digit.
int value_for_hex_digit(char c) {
  if (c >= '0' && c <= '9') {
    return c - '0';
  } else if (c >= 'a' && c <= 'f') {
    return 10 + (c - 'a');
  } else if (c >= 'A' && c <= 'F') {
    return 10 + (c - 'A');
  }
  return -1;
}

std::pair<absl::string_view, absl::string_view> Split(absl::string_view input,
                                                      char ch) {
  return absl::StrSplit(input, absl::MaxSplits(ch, 1));
}

}  // namespace

std::string UriEscape(UriEscapeMode mode, absl::string_view uri) {
  size_t num_escapes = 0;
  for (char c : uri) {
    if (should_escape(mode, c)) {
      ++num_escapes;
    }
  }
  std::string result;
  result.reserve(num_escapes * 2 + uri.size());
  for (char c : uri) {
    if (should_escape(mode, c)) {
      result.push_back('%');
      result.push_back(kHexDigits[(c >> 4) & 0xF]);
      result.push_back(kHexDigits[c & 0xF]);
    } else {
      result.push_back(c);
    }
  }
  return result;
}

/// \brief URI-unescapes a string.
/// \param string The string to unescape.
/// \return A pair of (success, error-or-unescaped-string).
std::pair<bool, std::string> UriUnescape(absl::string_view string) {
  size_t num_escapes = 0;
  for (size_t i = 0, s = string.size(); i < s; ++i) {
    if (string[i] == '%') {
      ++num_escapes;
      if (i + 3 > string.size()) {
        return std::make_pair(false, "bad escape");
      }
      i += 2;
    }
  }
  std::string result;
  result.reserve(string.size() - num_escapes * 2);
  for (size_t i = 0, s = string.size(); i < s; ++i) {
    char c = string[i];
    if (c != '%') {
      result.push_back(c);
      continue;
    }
    int high = value_for_hex_digit(string[++i]);
    int low = value_for_hex_digit(string[++i]);
    if (high < 0 || low < 0) {
      return std::make_pair(false, "bad hex digit");
    }
    result.push_back((high << 4) | low);
  }
  return std::make_pair(true, result);
}

std::string URI::ToString() const {
  std::string result = kUriPrefix;
  if (vname_.signature().empty() && vname_.path().empty() &&
      vname_.root().empty() && vname_.corpus().empty() &&
      vname_.language().empty()) {
    return result;
  }
  absl::string_view signature = vname_.signature();
  absl::string_view path = vname_.path();
  absl::string_view corpus = vname_.corpus();
  absl::string_view language = vname_.language();
  absl::string_view root = vname_.root();
  if (!corpus.empty()) {
    result.append("//");
    result.append(UriEscape(UriEscapeMode::kEscapePaths, corpus));
  }
  if (!language.empty()) {
    result.append("?lang=");
    result.append(UriEscape(UriEscapeMode::kEscapeAll, language));
  }
  if (!path.empty()) {
    result.append("?path=");
    result.append(UriEscape(UriEscapeMode::kEscapePaths, CleanPath(path)));
  }
  if (!root.empty()) {
    result.append("?root=");
    result.append(UriEscape(UriEscapeMode::kEscapePaths, root));
  }
  if (!signature.empty()) {
    result.push_back('#');
    result.append(UriEscape(UriEscapeMode::kEscapeAll, signature));
  }
  return result;
}

/// \brief Separate out the scheme component of `uri` if one exists.
/// \return (scheme, tail) if there was a scheme; ("", uri) otherwise.
static std::pair<absl::string_view, absl::string_view> SplitScheme(
    absl::string_view uri) {
  for (size_t i = 0, s = uri.size(); i != s; ++i) {
    char c = uri[i];
    if (c == ':') {
      return std::make_pair(uri.substr(0, i), uri.substr(i));
    } else if ((i == 0 &&
                ((c >= '0' && c <= '9') || c == '+' || c == '-' || c == '.')) ||
               !((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))) {
      break;
    }
  }
  return std::make_pair(absl::string_view(), uri);
}

URI::URI(const kythe::proto::VName& from_vname) : vname_(from_vname) {}

bool URI::ParseString(absl::string_view uri) {
  auto head_fragment = Split(uri, '#');
  auto head = head_fragment.first, fragment = head_fragment.second;
  auto scheme_head = SplitScheme(head);
  auto scheme = scheme_head.first;
  head = scheme_head.second;
  if (scheme.empty()) {
    if (absl::StartsWith(head, ":")) {
      return false;
    }
  } else if (scheme != kUriScheme) {
    return false;
  } else if (!head.empty()) {
    head.remove_prefix(1);
  }
  auto head_attrs = Split(head, '?');
  head = head_attrs.first;
  auto attrs = head_attrs.second;
  std::string corpus;
  if (!head.empty()) {
    if (!absl::StartsWith(head, "//")) {
      return false;
    }
    auto maybe_corpus = UriUnescape(head.substr(2));
    if (!maybe_corpus.first) {
      return false;
    }
    corpus = maybe_corpus.second;
  }
  auto maybe_sig = UriUnescape(fragment);
  if (!maybe_sig.first) {
    return false;
  }
  auto signature = maybe_sig.second;
  while (!attrs.empty()) {
    auto attr_rest = Split(attrs, '?');
    auto attr = attr_rest.first;
    attrs = attr_rest.second;
    auto name_value = Split(attr, '=');
    auto maybe_value = UriUnescape(name_value.second);
    if (!maybe_value.first || maybe_value.second.empty()) {
      return false;
    }
    if (name_value.first == "lang") {
      vname_.set_language(maybe_value.second);
    } else if (name_value.first == "root") {
      vname_.set_root(maybe_value.second);
    } else if (name_value.first == "path") {
      vname_.set_path(CleanPath(maybe_value.second));
    } else {
      return false;
    }
  }
  vname_.set_signature(signature);
  vname_.set_corpus(corpus);
  return true;
}

}  // namespace kythe
