/*
 * Copyright 2023 The Kythe Authors. All rights reserved.
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

#include "kythe/cxx/indexer/textproto/recordio_textparser.h"

#include <optional>
#include <sstream>

#include "absl/functional/function_ref.h"
#include "absl/log/log.h"
#include "absl/strings/ascii.h"
#include "absl/strings/match.h"
#include "absl/strings/str_split.h"
#include "absl/strings/string_view.h"
#include "absl/strings/strip.h"

namespace kythe {
namespace lang_textproto {

namespace {

// WithChar is a delimiter for absl::StrSplit() that splits on given char but
// also includes the delimiter char.
struct WithChar {
  explicit WithChar(char ch) : delimiter_(ch) {}
  absl::string_view Find(absl::string_view text, size_t pos) const {
    absl::string_view sep = delimiter_.Find(text, pos);
    // Always return a zero-width span after the delimiter, so that it's
    // included if present.
    sep.remove_prefix(sep.size());
    return sep;
  }

 private:
  absl::ByChar delimiter_;
};

class ProtoLineDelimiter {
 public:
  explicit ProtoLineDelimiter(absl::string_view delimiter,
                              int* line_count = nullptr)
      : delimiter_(delimiter), line_count_(line_count), current_line_(0) {}

  /// \brief Finds the next occurrence of the configured delimiter
  /// on a line by itself, after the first non-comment, non-empty line.
  absl::string_view Find(absl::string_view text, size_t pos) {
    // Store the start line of chunk.
    if (line_count_) {
      *line_count_ = current_line_;
    }
    for (absl::string_view line :
         absl::StrSplit(text.substr(pos), WithChar('\n'))) {
      current_line_++;
      // Don't look for the delimiter until we've seen our first non-empty,
      // non-comment line.
      data_seen_ = data_seen_ || !(absl::StartsWith(line, "#") ||
                                   absl::StripPrefix(line, "\n").empty());
      bool is_delimiter =
          // The line consists entirely of the delimiter and delimiter may
          // start with a comment.
          absl::StripPrefix(absl::StripPrefix(line, delimiter_), "\n").empty();
      if (!data_seen_ && is_delimiter) continue;

      if (is_delimiter) {
        return line;
      }
    }
    return text.substr(text.size());
  }

 private:
  std::string delimiter_;
  int* line_count_;
  int current_line_;

  bool data_seen_ = false;
};

}  // namespace

void ParseRecordTextChunks(
    absl::string_view content, absl::string_view record_delimiter,
    absl::FunctionRef<void(absl::string_view chunk, int chunk_start_line)>
        callback) {
  int line_count = 0;
  for (absl::string_view chunk : absl::StrSplit(
           content, ProtoLineDelimiter(record_delimiter, &line_count))) {
    callback(chunk, line_count);
  }
}

}  // namespace lang_textproto
}  // namespace kythe
