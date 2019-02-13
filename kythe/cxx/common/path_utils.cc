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

#include <stdlib.h>
#include <unistd.h>
#include <vector>

#include "absl/memory/memory.h"
#include "absl/strings/match.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "absl/strings/str_split.h"
#include "glog/logging.h"
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

bool IsProperPathPrefix(absl::string_view s, absl::string_view prefix) {
  return absl::StartsWith(s, prefix) &&
         (s.size() == prefix.size() || s[prefix.size()] == '/');
}

}  // namespace

std::string JoinPath(absl::string_view a, absl::string_view b) {
  return absl::StrCat(absl::StripSuffix(a, "/"), "/",
                      absl::StripPrefix(b, "/"));
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

bool IsAbsolutePath(absl::string_view path) {
  return absl::StartsWith(path, "/");
}

bool GetCurrentDirectory(std::string* dir) {
  size_t len = 128;
  auto buffer = absl::make_unique<char[]>(len);
  for (;;) {
    char* p = getcwd(buffer.get(), len);
    if (p != nullptr) {
      *dir = p;
      return true;
    } else if (errno == ERANGE) {
      len += len;
      buffer = absl::make_unique<char[]>(len);
    } else {
      return false;
    }
  }
}

std::string MakeCleanAbsolutePath(absl::string_view in_path) {
  std::string abs_path;
  if (IsAbsolutePath(in_path)) {
    abs_path = std::string(in_path);
  } else {
    std::string dir;
    if (!GetCurrentDirectory(&dir)) {
      LOG(ERROR) << "Unable to get current working directory";
      return "";
    }
    abs_path = JoinPath(dir, in_path);
  }
  return CleanPath(abs_path);
}

struct PathParts {
  absl::string_view dir, base;
};

PathParts SplitPath(absl::string_view path) {
  std::string::difference_type pos = path.find_last_of('/');

  // Handle the case with no '/' in 'path'.
  if (pos == absl::string_view::npos) return {path.substr(0, 0), path};

  // Handle the case with a single leading '/' in 'path'.
  if (pos == 0) return {path.substr(0, 1), absl::ClippedSubstr(path, 1)};

  return {path.substr(0, pos), absl::ClippedSubstr(path, pos + 1)};
}

absl::string_view Dirname(absl::string_view path) {
  return SplitPath(path).dir;
}

std::string RelativizePath(absl::string_view to_relativize,
                           absl::string_view relativize_against) {
  std::string to_relativize_abs = MakeCleanAbsolutePath(to_relativize);
  std::string relativize_against_abs =
      MakeCleanAbsolutePath(relativize_against);
  if (relativize_against_abs == "/") {
    // We don't handle a generic case where the absolute path ends with slash,
    // since users can work around by just dropping the slash. Except this case
    // where relativization base is the root.
    return to_relativize_abs.substr(1);
  }
  std::string to_relativize_parent = std::string(Dirname(to_relativize_abs));
  std::string ret =
      IsProperPathPrefix(to_relativize_parent, relativize_against_abs)
          ? to_relativize_abs.substr(relativize_against_abs.size() + 1)
          : to_relativize_abs;
  return ret;
}

}  // namespace kythe
