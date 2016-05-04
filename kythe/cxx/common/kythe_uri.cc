/*
 * Copyright 2015 Google Inc. All rights reserved.
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
#include "kythe/cxx/common/path_utils.h"
#include "kythe/cxx/common/proto_conversions.h"

namespace kythe {

/// \brief Returns whether a byte should be escaped.
/// \param mode The escaping mode to use.
/// \param c The byte to examine.
inline bool should_escape(UriEscapeMode mode, char c) {
  return !((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
           (c >= '0' && c <= '9') || c == '-' || c == '.' || c == '_' ||
           c == '~' || (mode == UriEscapeMode::kEscapePaths && c == '/'));
}

/// \brief Returns the value of a hex digit.
/// \param digit The hex digit.
/// \return The value of the digit, or -1 if it was not a hex digit.
inline int value_for_hex_digit(char c) {
  if (c >= '0' && c <= '9') {
    return c - '0';
  } else if (c >= 'a' && c <= 'f') {
    return 10 + (c - 'a');
  } else if (c >= 'A' && c <= 'F') {
    return 10 + (c - 'A');
  }
  return -1;
}

static constexpr char kHexDigits[] = "0123456789ABCDEF";

static std::string UriEscape(UriEscapeMode mode, llvm::StringRef uri) {
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

std::string UriEscape(UriEscapeMode mode, const std::string& uri) {
  return UriEscape(mode, llvm::StringRef(uri));
}

/// \brief URI-unescapes a string.
/// \param string The string to unescape.
/// \return A pair of (success, error-or-unescaped-string).
std::pair<bool, std::string> UriUnescape(const std::string& string) {
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

/// The URI scheme label for Kythe.
static constexpr char kUriScheme[] = "kythe";
static constexpr char kUriPrefix[] = "kythe:";

std::string URI::ToString() const {
  std::string result = kUriPrefix;
  if (vname_.signature().empty() && vname_.path().empty() &&
      vname_.root().empty() && vname_.corpus().empty() &&
      vname_.language().empty()) {
    return result;
  }
  llvm::StringRef signature = ToStringRef(vname_.signature());
  llvm::StringRef path = ToStringRef(vname_.path());
  llvm::StringRef corpus = ToStringRef(vname_.corpus());
  llvm::StringRef language = ToStringRef(vname_.language());
  llvm::StringRef root = ToStringRef(vname_.root());
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
    result.append(UriEscape(UriEscapeMode::kEscapePaths, path));
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
static std::pair<llvm::StringRef, llvm::StringRef> SplitScheme(
    llvm::StringRef uri) {
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
  return std::make_pair(llvm::StringRef(), uri);
}

URI::URI(const kythe::proto::VName& from_vname) : vname_(from_vname) {
  auto corpus = ToStringRef(vname_.corpus());
  vname_.set_corpus(CleanPath(corpus));
}

bool URI::ParseString(const std::string& in_string) {
  llvm::StringRef string(in_string);
  auto head_fragment = string.split('#');
  auto head = head_fragment.first, fragment = head_fragment.second;
  auto scheme_head = SplitScheme(head);
  auto scheme = scheme_head.first;
  head = scheme_head.second;
  if (scheme.empty()) {
    if (head.startswith(":")) {
      return false;
    }
  } else if (scheme != kUriScheme) {
    return false;
  } else if (!head.empty()) {
    head = head.drop_front(1);
  }
  auto head_attrs = head.split('?');
  head = head_attrs.first;
  auto attrs = head_attrs.second;
  std::string corpus;
  if (!head.empty()) {
    if (!head.startswith("//")) {
      return false;
    }
    auto maybe_corpus = UriUnescape(head.drop_front(2));
    if (!maybe_corpus.first) {
      return false;
    }
    corpus = CleanPath(maybe_corpus.second);
  }
  auto maybe_sig = UriUnescape(fragment);
  if (!maybe_sig.first) {
    return false;
  }
  auto signature = maybe_sig.second;
  while (!attrs.empty()) {
    auto attr_rest = attrs.split('?');
    auto attr = attr_rest.first;
    attrs = attr_rest.second;
    auto name_value = attr.split('=');
    auto maybe_value = UriUnescape(name_value.second);
    if (!maybe_value.first || maybe_value.second.empty()) {
      return false;
    }
    if (name_value.first == "lang") {
      vname_.set_language(maybe_value.second);
    } else if (name_value.first == "root") {
      vname_.set_root(maybe_value.second);
    } else if (name_value.first == "path") {
      vname_.set_path(maybe_value.second);
    } else {
      return false;
    }
  }
  vname_.set_signature(signature);
  vname_.set_corpus(corpus);
  return true;
}

}  // namespace kythe
