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

#include "kythe/cxx/indexer/proto/comments.h"

#include <vector>

#include "absl/strings/str_join.h"
#include "absl/strings/str_split.h"
#include "absl/strings/strip.h"

namespace kythe {

std::string StripCommentMarkers(const std::string& source) {
  absl::string_view stripped = absl::StripAsciiWhitespace(source);

  // Handle block comments, of the form "/* ... */".
  if (absl::ConsumePrefix(&stripped, "/*")) {
    absl::ConsumeSuffix(&stripped, "*/");
    std::vector<absl::string_view> lines = absl::StrSplit(stripped, '\n');
    for (auto& line : lines) {
      line = absl::StripAsciiWhitespace(line);
      if (!absl::ConsumePrefix(&line, "* ")) {
        absl::ConsumePrefix(&line, "*");  // fallback
      }
    }
    return absl::StrJoin(lines, "\n");
  }

  // Handle line-ending comments, of the form "// ...".
  std::vector<absl::string_view> lines = absl::StrSplit(stripped, '\n');
  for (auto& line : lines) {
    line = absl::StripAsciiWhitespace(line);
    if (!absl::ConsumePrefix(&line, "// ")) {
      absl::ConsumePrefix(&line, "//");  // fallback
    }
  }
  return absl::StrJoin(lines, "\n");
}

}  // namespace kythe
