/*
 * Copyright 2018 The Kythe Authors. All rights reserved.
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

#include <vector>

#include "absl/strings/match.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "absl/strings/str_split.h"
#include "kythe/cxx/common/path_utils.h"

namespace kythe {
namespace {

// Predicate used in CleanPath for skipping empty components
// and components consistening of a single '.'.
struct SkipEmptyDot {
  bool operator()(absl::string_view sp) { return !(sp.empty() || sp == "."); }
};

// Deal with relative paths as well as '/' and '//'.
absl::string_view PathPrefix(absl::string_view path) {
  int slash_count = 0;
  for (char ch : path) {
    if (ch == '/' && ++slash_count <= 2) continue;
    break;
  }
  switch (slash_count) {
    case 0:
      return "";
    case 2:
      return "//";
    default:
      return "/";
  }
}

}  // namespace

std::string JoinPath(absl::string_view a, absl::string_view b) {
  return absl::StrCat(absl::StripSuffix(a, "/"), "/", absl::StripPrefix(b, "/"));
}

std::string CleanPath(absl::string_view input) {
  const bool is_absolute_path = absl::StartsWith(input, "/");
  std::vector<absl::string_view> parts;
  for (absl::string_view comp : absl::StrSplit(input, '/', SkipEmptyDot{})) {
    if (comp == "..") {
      if (!parts.empty() && parts.back() != "..") {
        parts.pop_back();
        continue;
      }
      if (is_absolute_path) continue;
    }
    parts.push_back(comp);
  }
  // Deal with leading '//' as well as '/'.
  return absl::StrCat(PathPrefix(input), absl::StrJoin(parts, "/"));
}

}  // namespace kythe
